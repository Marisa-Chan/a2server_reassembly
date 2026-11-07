// A2 network protocol for the browser prototype.
//
// Wire format (CLlDriver TCP provider 4 — same on the HAT link and the game
// link, see src/net.cpp NetStru3 + util.cpp XorData):
//   frame = [pos:u16][pad:u16][cmode:u8][csize:u16][pkt_num:u8] + xor(payload)
//   payload chunks are <=142 bytes; xor key is 80 bytes, index resets per frame.
//   A "stream" = game packets back-to-back ending with the 5-byte seal
//   [64 01 00 00 00] (PacketDword 0x64, value 1 — see NetStru1::SendPacket_64).
//   pkt_num on a frame counts the seals carried inside it; the receiver
//   concatenates payloads until a frame with pkt_num > 0 completes the stream.

const XORKEY = new Uint8Array([
    0x2e, 0xc7, 0xc4, 0x8d, 0xff, 0xe5, 0x5d, 0x0b,
    0xd6, 0xff, 0x7f, 0xff, 0xd7, 0x34, 0xd2, 0x02,
    0xe2, 0x6d, 0x9e, 0x48, 0x7b, 0xc6, 0x6a, 0xf1,
    0x97, 0x73, 0x56, 0x77, 0xfa, 0x9d, 0x80, 0x00,
    0x11, 0x04, 0x29, 0x08, 0xa6, 0x8b, 0x2a, 0x47,
    0x64, 0x00, 0x01, 0x38, 0x84, 0xa0, 0x40, 0x69,
    0x01, 0xf9, 0xfa, 0xbe, 0xea, 0xff, 0x97, 0x7b,
    0xa7, 0x26, 0xed, 0xf7, 0x6b, 0x7b, 0x3b, 0x4f,
    0x44, 0x74, 0xa3, 0x09, 0x79, 0x40, 0x38, 0x3a,
    0x20, 0x5d, 0xa3, 0x40, 0xc3, 0xe8, 0x7f, 0x3b,
]);

function xorInPlace(buf, off, len) {
    for (let i = 0; i < len; i++) {
        buf[off + i] ^= XORKEY[i % 80];
    }
}

// ---------------------------------------------------------------- packets ---

function u16le(v) { return [v & 0xff, (v >> 8) & 0xff]; }
function u32le(v) { return [v & 0xff, (v >>> 8) & 0xff, (v >>> 16) & 0xff, (v >>> 24) & 0xff]; }
function strBytes(s) { const a = new Uint8Array(s.length); for (let i = 0; i < s.length; i++) a[i] = s.charCodeAt(i) & 0xff; return a; }

// [id] — bare packet
function pkt1(id) { return new Uint8Array([id]); }
// [id][u32]
function pktDword(id, v) { return new Uint8Array([id, ...u32le(v)]); }
// [id][u32][u32] — Packet2Dwords
function pkt2dw(id, p1, p2) { return new Uint8Array([id, ...u32le(p1), ...u32le(p2)]); }
// [id][u32 count][data] — PacketData
function pktData(id, data) { return new Uint8Array([id, ...u32le(data.length), ...data]); }
// [id][player_id][token][color][flags][u16 len][name][0] — PacketJoin
function pktJoin(id, playerId, token, color, flags, name) {
    const nb = strBytes(name);
    return new Uint8Array([id, playerId, token, color, flags, ...u16le(nb.length), ...nb, 0]);
}

const SEAL = new Uint8Array([0x64, 0x01, 0x00, 0x00, 0x00]);

// ------------------------------------------------------------- connection ---

class A2Conn {
    constructor(ws, label) {
        this.ws = ws;
        this.label = label;
        this.rxRaw = new Uint8Array(0);
        this.pending = [];
        this.onPacket = null;   // (id, bytes) => void
        this.onClose = null;
        this.onDesync = null;
    }

    sendStream(packets) {
        const parts = packets.map(p => (p instanceof Uint8Array ? p : new Uint8Array(p)));
        let total = SEAL.length;
        for (const p of parts) total += p.length;
        const stream = new Uint8Array(total);
        let off = 0;
        for (const p of parts) { stream.set(p, off); off += p.length; }
        stream.set(SEAL, off);

        let sent = 0;
        while (true) {
            const chunk = stream.subarray(sent, sent + 142);
            sent += chunk.length;
            const last = sent >= stream.length ? 1 : 0;
            const frame = new Uint8Array(8 + chunk.length);
            const dv = new DataView(frame.buffer);
            dv.setUint16(0, chunk.length, true);
            dv.setUint16(2, 0, true);      // pad
            frame[4] = 0;                  // cmode
            dv.setUint16(5, 0, true);      // csize
            frame[7] = last;               // pkt_num = seals in this frame
            frame.set(chunk, 8);
            xorInPlace(frame, 8, chunk.length);
            this.ws.send(frame);
            if (last) return;
        }
    }

    feed(data) {
        const incoming = new Uint8Array(data);
        const merged = new Uint8Array(this.rxRaw.length + incoming.length);
        merged.set(this.rxRaw);
        merged.set(incoming, this.rxRaw.length);

        let off = 0;
        while (merged.length - off >= 8) {
            const dv = new DataView(merged.buffer, merged.byteOffset + off);
            const pos = dv.getUint16(0, true);
            const pktNum = merged[off + 7];
            if (pos === 0 || pos >= 143) {
                if (this.onDesync) this.onDesync(`invalid frame pos=${pos} at rx offset ${off}`);
                this.rxRaw = new Uint8Array(0);
                return;
            }
            if (merged.length - off < 8 + pos) break;
            const payload = merged.slice(off + 8, off + 8 + pos);
            xorInPlace(payload, 0, pos);
            off += 8 + pos;

            this.pending.push(payload);
            if (pktNum > 0) {
                let total = 0;
                for (const p of this.pending) total += p.length;
                const stream = new Uint8Array(total);
                let soff = 0;
                for (const p of this.pending) { stream.set(p, soff); soff += p.length; }
                this.pending = [];
                for (const pkt of parseStream(stream)) {
                    if (this.onPacket) this.onPacket(pkt.id, pkt.bytes, stream);
                }
            }
        }
        this.rxRaw = merged.slice(off);
    }
}

// ------------------------------------------------------------ stream parse --

const L_1 = new Set([3, 5, 9, 0xa, 0xaf, 0xb7, 0xb8, 0x4a, 0x4b, 0x4c, 0x5a, 0xc8, 0xd6, 0xdb, 0xdc]);
const L_9 = new Set([4, 0xb, 0x39, 0x3b, 0x5c, 0xca, 0xcc, 0xd0, 0xd8, 0xe1, 0x23, 0x3a, 0x3d, 0x3e, 0x3f,
    0x40, 0x46, 0x48, 0x83, 0x84, 0x92, 0x94, 0xaa, 0xc3, 0x65, 0x67, 0x97, 0xab, 0xac, 0xad, 0xb4, 0xb5, 0xb6]);
const L_5 = new Set([0xe, 0x10, 0xb3, 0xc1, 0xdf, 0x64, 0x6b, 0x6d, 0x71, 0x72, 0x73, 0x82]);
const L_6 = new Set([0x0b]); // hat kick: [0B][reason:u8][u32 0]
const L_4 = new Set([0x88, 0x89]);
const L_3 = new Set([0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x69, 0x6a, 0x74]);
const L_JOIN = new Set([6, 7, 8, 0x4e, 0x5b, 0x93, 0xbf, 0xc9, 0xd1, 0xd2, 0xd3, 0xf, 0x49, 0x91, 0x96, 0xae]);
const L_GRP = new Set([0x12, 0x13, 0x14, 0x17, 0x18, 0x4d, 0x16, 0x1a, 0x1c, 0x1d, 0x1f, 0x21, 0x24, 0x26,
    0x19, 0x1b, 0x1e, 0x25]);
const L_TER = new Set([0x45, 0x8a, 0x8c, 0x9b, 0xb9, 0xbb, 0xbc]);
const L_UUPD_FLAGSIZE = { 0x6c: 4, 0x6e: 1, 0x6f: 2, 0x70: 3 };
const L_DATA = new Set([0xba, 0xc2, 0xcb, 0xcd, 0xce, 0xcf, 0xd4, 0xd9, 0xda, 0xdd, 0xde, 0xe0, 0xbe]);

// Field sizes by flag bit index (bit i => UNITUPD_OFFS[i] bytes).
const UNITUPD_OFFS = [2, 2, 5, 4, 3, 2, 1, 2, 4, 4, 4, 4, 4, 2, 2, 1, 2, 2, 2, 1, 5, 0, 0, 0, 5, 0x10, 0, 0, 0, 8, 0, 0x18];

function unitupdDataLen(flags) {
    let n = 0;
    for (let i = 0; i < 32; i++) {
        if (flags & (1 << i)) n += UNITUPD_OFFS[i];
    }
    return n;
}

function rd16(b, o) { return b[o] | (b[o + 1] << 8); }
function rd32(b, o) { return (b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)) >>> 0; }

function packetLen(id, b, i) {
    try {
        if (id in L_UUPD_FLAGSIZE) {
            const fs = L_UUPD_FLAGSIZE[id];
            const flags = readFlags(b, i + 3, fs);
            return 3 + fs + unitupdDataLen(flags);
        }
        if (L_1.has(id)) return 1;
        if (L_9.has(id)) return 9;
        if (L_5.has(id)) return 5;
        if (L_6.has(id)) return 6;
        if (L_4.has(id)) return 4;
        if (L_3.has(id)) return 3;
        if (L_JOIN.has(id)) return 8 + rd16(b, i + 5);
        if (L_GRP.has(id)) return 10 + 2 * b[i + 9];
        if (L_TER.has(id)) return 5 + 2 * rd32(b, i + 1);
        if (L_DATA.has(id)) return 5 + rd32(b, i + 1);
        if (id === 0x22) return 11;
        if (id === 0x7a) return 9;
        if (id === 0x86 || id === 0x8b) return 8;
        if (id === 0x87) return 19;
        if (id === 0x9c) {
            let pop = 0, fl = rd16(b, i + 3);
            for (let k = 0; k < 16; k++) if (fl & (1 << k)) pop++;
            return 5 + 2 * pop;
        }
        if (id === 0xc0) return 45 + rd32(b, i + 41);
        if (id === 0x76) return 10 + rd16(b, i + 8);
        if (id === 0x02) return 45; // enter game: id + 3*u32 + 32 zero bytes
    } catch (e) { /* fallthrough */ }
    return null;
}

function readFlags(b, off, size) {
    let v = 0;
    for (let i = 0; i < size; i++) v |= b[off + i] << (8 * i);
    return v >>> 0;
}

function parseStream(stream) {
    const out = [];
    let i = 0;
    const n = stream.length;
    while (i < n) {
        const id = stream[i];
        const isTrailingSeal = id === 0x64 && i === n - 1;
        if (isTrailingSeal) break;
        const len = packetLen(id, stream, i);
        if (len === null || i + len > n) {
            out.push({ id, bytes: stream.slice(i), truncated: true });
            break;
        }
        out.push({ id, bytes: stream.slice(i, i + len) });
        i += len;
    }
    return out;
}

// ----------------------------------------------------- unit update decoder --

// Field order mirrors NetStru1::SendUnitUpdate (src/net.cpp) exactly — the
// receiver must walk in pack order, not bit order.
const UUPD_FIELDS = [
    { flag: 0x1, name: 'hp', size: 2, kind: 'u16' },
    { flag: 0x2, name: 'mp', size: 2, kind: 'u16' },
    { flag: 0x4, name: 'skills', size: 5, kind: 'bytes' },
    { flag: 0x8, name: 'hp_max,mp_max', size: 4, kind: 'u16x2' },
    { flag: 0x10, name: 'pos', size: 3, kind: 'pos' },
    { flag: 0x20, name: 'type,face', size: 2, kind: 'bytes' },
    { flag: 0x40, name: 'owner', size: 1, kind: 'u8' },
    { flag: 0x80, name: 'server_id', size: 2, kind: 'u16' },
    { flag: 0x10000, name: 'attack', size: 2, kind: 'u16' },
    { flag: 0x20000, name: 'dmg_min,spread', size: 2, kind: 'bytes' },
    { flag: 0x4000, name: 'defense', size: 2, kind: 'u16' },
    { flag: 0x8000, name: 'absorb', size: 1, kind: 'u8' },
    { flag: 0x40000, name: 'scan_range', size: 2, kind: 'u16' },
    { flag: 0x2000, name: 'weight', size: 2, kind: 'u16' },
    { flag: 0x1000000, name: 'magic_prot', size: 5, kind: 'bytes' },
    { flag: 0x2000000, name: 'battle_stats', size: 16, kind: 'bytes' },
    { flag: 0x80000, name: 'speed', size: 1, kind: 'u8' },
    { flag: 0x100000, name: 'spells', size: 5, kind: 'bytes' },
    { flag: 0x100, name: 'exp_fire', size: 4, kind: 'u32' },
    { flag: 0x200, name: 'exp_water', size: 4, kind: 'u32' },
    { flag: 0x400, name: 'exp_air', size: 4, kind: 'u32' },
    { flag: 0x800, name: 'exp_earth', size: 4, kind: 'u32' },
    { flag: 0x1000, name: 'exp_astral', size: 4, kind: 'u32' },
    { flag: 0x20000000, name: 'base_stats', size: 8, kind: 'bytes' },
    { flag: 0x80000000, name: 'name', size: 24, kind: 'name' },
];

function decodeUnitUpdate(b) {
    const id = b[0];
    const fs = L_UUPD_FLAGSIZE[id];
    const unitId = rd16(b, 1);
    const flags = readFlags(b, 3, fs);
    let off = 3 + fs;
    const fields = {};
    for (const f of UUPD_FIELDS) {
        if ((flags & f.flag) === 0) continue;
        if (off + f.size > b.length) break;
        const seg = b.subarray(off, off + f.size);
        switch (f.kind) {
            case 'u8': fields[f.name] = seg[0]; break;
            case 'u16': fields[f.name] = rd16(seg, 0); break;
            case 'u32': fields[f.name] = rd32(seg, 0); break;
            case 'u16x2': fields[f.name] = [rd16(seg, 0), rd16(seg, 2)]; break;
            case 'pos':
                fields.facing = seg[0];
                fields.sub = [seg[1], seg[2]]; // subcell u16 LE, or cell x,y when at center
                break;
            case 'name': {
                let end = seg.indexOf(0);
                if (end < 0) end = seg.length;
                fields[f.name] = new TextDecoder('latin1').decode(seg.subarray(0, end));
                break;
            }
            default: fields[f.name] = Array.from(seg);
        }
        off += f.size;
    }
    return { unitId, flags, fields };
}

// ------------------------------------------------------------ packet names --

const PKT_NAMES = {
    0x02: 'EnterGame', 0x03: 'LeaveGame?', 0x04: 'MissionReady', 0x05: 'Start?',
    0x0b: 'Kick', 0x0e: 'Word2?', 0x10: 'Dword?',
    0x12: 'GroupOrder', 0x13: 'GroupOrder', 0x14: 'GroupOrder', 0x16: 'GroupMove',
    0x17: 'GroupOrder', 0x18: 'GroupOrder', 0x19: 'GroupOrder', 0x1a: 'GroupOrder',
    0x22: 'Ping?', 0x23: 'Dword2', 0x32: 'Word', 0x33: 'Word', 0x34: 'Word',
    0x35: 'Word', 0x36: 'Word', 0x38: 'Word', 0x39: 'MapInfoReq',
    0x3b: 'MapChunkReq', 0x45: 'Terrain', 0x46: 'Dword2',
    0x48: 'JoinStats', 0x49: 'JoinInfo', 0x4e: 'NickCheck',
    0x64: 'Tick', 0x65: 'Dword2', 0x67: 'Dword2',
    0x69: 'Word', 0x6a: 'Word', 0x6b: 'UnitState', 0x6c: 'UnitUpdate',
    0x6d: 'UnitState2', 0x6e: 'UnitUpdate1', 0x6f: 'UnitUpdate2', 0x70: 'UnitUpdate3',
    0x71: 'UnitState3', 0x72: 'UnitState4', 0x73: 'UnitState5', 0x74: 'Word',
    0x76: 'Inventory', 0x79: 'WearItems', 0x7a: 'Sack',
    0x82: 'State5', 0x86: 'Effect', 0x87: 'AreaEffect', 0x88: 'EffectHang',
    0x89: 'EffectGone', 0x8a: 'EffectMulti', 0x8b: 'EffectFrom', 0x8c: 'EffectMultiFrom',
    0x8d: 'Message', 0x91: 'JoinInfo2', 0x92: 'TeamNotify', 0x93: 'JoinInfo3',
    0x94: 'Dword2', 0x96: 'PlayerJoin', 0x97: 'PlayerLeft', 0x98: 'AshMap',
    0x9b: 'Terrain2', 0x9c: 'Mask2', 0xaa: 'Dword2', 0xab: 'Dword2', 0xac: 'Dword2',
    0xad: 'Dword2', 0xae: 'JoinInfo4', 0xaf: 'Bare', 0xb3: 'Dword', 0xb4: 'Dword2',
    0xb5: 'Dword2', 0xb6: 'Dword2', 0xb7: 'Bare', 0xb8: 'Bare', 0xb9: 'Diplomacy',
    0xba: 'CreatureKnow', 0xbb: 'PlayerQuests', 0xbc: 'InnQuests',
    0xbe: 'CharFile', 0xbf: 'MapChunkQ2', 0xc0: 'MapChunkData', 0xc1: 'Latency',
    0xc2: 'Data2', 0xc3: 'Dword2', 0xc8: 'SrvListReq', 0xc9: 'Login',
    0xca: 'CharReq', 0xcb: 'EnterServer', 0xcc: 'CharDelete', 0xcd: 'ServerList',
    0xce: 'CharList', 0xcf: 'CharData', 0xd0: 'HatTicket', 0xd1: 'SrvLogin',
    0xd4: 'RawString', 0xd5: 'LoginOk', 0xd6: 'Bare', 0xd8: 'JoinOk',
    0xd9: 'JoinFail', 0xda: 'Reconnect', 0xdb: 'Bare', 0xdc: 'Bare',
    0xdd: 'JoinChar', 0xde: 'Data2', 0xdf: 'NickResult', 0xe0: 'HatMessage',
    0xe1: 'PlayerCheck',
};

function pktName(id) {
    return PKT_NAMES[id] || '?';
}
