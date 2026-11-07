// Browser A2 client prototype: HAT login -> server (map) list -> join -> log.

let hatConn = null;      // A2Conn on the HAT link
let gameConn = null;     // A2Conn on the game link
let hatWs = null;
let gameWs = null;

const state = {
    hatId: null,
    chars: [],           // {id1, id2}
    servers: [],         // parsed 0xCD entries
    selectedServer: null,
    ticket: null,        // {id1, id2}
    players: new Map(),  // player_id -> name (0x96)
    units: new Map(),    // unitId -> {fields, count, verdict}
    counters: new Map(), // pkt id -> count
    logPaused: false,
    hideTicks: false,
};

// ------------------------------------------------------------------- utils --

function $(id) { return document.getElementById(id); }
function ts() { const d = new Date(); return d.toTimeString().slice(0, 8) + '.' + String(d.getMilliseconds()).padStart(3, '0'); }
function hex(b, max = 48) {
    let s = '';
    const n = Math.min(b.length, max);
    for (let i = 0; i < n; i++) s += b[i].toString(16).padStart(2, '0') + ' ';
    if (b.length > n) s += `… (+${b.length - n}B)`;
    return s.trim();
}

function log(connLabel, dir, id, bytes, detail) {
    if (state.logPaused) return;
    if (state.hideTicks && id === 0x64 && dir === 'RX') return;
    const table = $('pktlog');
    const row = document.createElement('tr');
    row.className = dir === 'TX' ? 'tx' : 'rx';
    const name = pktName(id);
    row.innerHTML =
        `<td>${ts()}</td><td>${connLabel}</td><td>${dir}</td>` +
        `<td class="pid">0x${id.toString(16).padStart(2, '0')}</td>` +
        `<td>${name}</td><td>${bytes.length}</td>` +
        `<td class="detail">${detail || ''}</td>` +
        `<td class="hexcell" title="${hex(bytes)}">${hex(bytes)}</td>`;
    table.appendChild(row);
    while (table.rows.length > 4000) table.deleteRow(0);
    if ($('autoscroll').checked) $('logwrap').scrollTop = $('logwrap').scrollHeight;
}

function logInfo(msg) {
    const el = $('statusline');
    el.textContent = msg;
    console.log('[a2] ' + msg);
}

function bump(id) {
    state.counters.set(id, (state.counters.get(id) || 0) + 1);
    if (id === 0x64) return;
    const el = $('counters');
    const parts = [...state.counters.entries()]
        .sort((a, b) => b[1] - a[1])
        .slice(0, 14)
        .map(([k, v]) => `0x${k.toString(16)}:${v}`);
    el.textContent = parts.join('  ');
}

// ------------------------------------------------------------ HAT handling --

function openBridge(host, port, label) {
    const ws = new WebSocket(`ws://${location.host}/ws?host=${encodeURIComponent(host)}&port=${port}`);
    ws.binaryType = 'arraybuffer';
    const conn = new A2Conn(ws, label);
    conn.onDesync = (msg) => log(label, '!!', 0, new Uint8Array(), msg);
    ws.onmessage = (ev) => conn.feed(ev.data);
    return { ws, conn };
}

function hatConnect() {
    const host = $('hat-host').value.trim() || '127.0.0.1';
    const port = parseInt($('hat-port').value, 10) || 8000;
    const login = $('login').value.trim();
    const password = $('password').value;
    const mode = parseInt($('mode').value, 10) || 0;

    if (!login) { alert('login required'); return; }
    if (hatWs) { hatWs.close(); hatWs = null; }

    logInfo(`connecting to HAT ${host}:${port}…`);
    const { ws, conn } = openBridge(host, port, 'HAT');
    hatWs = ws;
    hatConn = conn;
    state.pendingLogin = { login, password, mode };

    ws.onopen = () => {
        logInfo('HAT connected, sending login 0xC9…');
        conn.onPacket = hatPacket;
        conn.sendStream([pktJoin(0xC9, login.length, 0, mode, 7, login + password)]);
        log('HAT', 'TX', 0xC9, new Uint8Array([0xC9]), `login="${login}" mode=${mode}`);
    };
    ws.onclose = () => {
        logInfo('HAT connection closed');
        $('hat-status').textContent = 'disconnected';
    };
    ws.onerror = () => logInfo('HAT websocket error');
}

function parseServerList(text) {
    // "CURRENTCOUNT|%2u$BREAK\nTOTALSERVERS|%2u$BREAK\n\n" + per server
    // "|srvname|1.02|mapname|WxH|difficulty|players|addr:port\n"
    const out = [];
    const lines = text.split('\n');
    for (const line of lines) {
        if (!line.startsWith('|')) continue;
        const parts = line.split('|');
        if (parts.length < 7) continue;
        out.push({
            name: parts[1], ver: parts[2], map: parts[3], size: parts[4],
            difficulty: parts[5], players: parts[6], addr: parts[7] || parts.slice(7).join('|'),
        });
    }
    return out;
}

function hatPacket(id, b) {
    bump(id);
    switch (id) {
        case 0xCE: {
            const data = b.subarray(5);
            state.hatId = rd32(data, 0);
            state.chars = [];
            for (let off = 4; off + 8 <= data.length; off += 8) {
                state.chars.push({ id1: rd32(data, off), id2: rd32(data, off + 4) });
            }
            log('HAT', 'RX', id, b, `hat_id=${state.hatId} chars=${state.chars.length}`);
            renderChars();
            logInfo(`logged in (hat_id=${state.hatId}), requesting server list 0xC8…`);
            $('hat-status').textContent = 'logged in';
            hatConn.sendStream([pkt1(0xC8)]);
            log('HAT', 'TX', 0xC8, new Uint8Array([0xC8]), 'server list request');
            break;
        }
        case 0xCD: {
            let text = new TextDecoder('latin1').decode(b.subarray(5));
            if (text.charCodeAt(text.length - 1) === 0) text = text.slice(0, -1);
            state.servers = parseServerList(text);
            log('HAT', 'RX', id, b, `${state.servers.length} servers`);
            renderServers();
            logInfo(`server list: ${state.servers.length} game(s)`);
            break;
        }
        case 0xD0: {
            state.ticket = { id1: rd32(b, 1), id2: rd32(b, 5) };
            log('HAT', 'RX', id, b, `ticket id1=${state.ticket.id1} id2=${state.ticket.id2}`);
            logInfo('join approved (0xD0 ticket) — connecting to game…');
            gameConnect();
            break;
        }
        case 0xDA: {
            const data = b.subarray(5);
            state.hatId = rd32(data, 0);
            state.ticket = { id1: rd32(data, 4), id2: rd32(data, 8) };
            const addr = new TextDecoder('latin1').decode(data.subarray(0x14)).replace(/\0.*$/, '');
            log('HAT', 'RX', id, b, `reconnect to ${addr}`);
            logInfo(`reconnect ticket for ${addr} — connecting…`);
            state.selectedServer = { addr, name: '(reconnect)' };
            gameConnect();
            break;
        }
        case 0x0B: {
            const reason = b[1];
            log('HAT', 'RX', id, b, `KICKED reason=${reason}`);
            logInfo(`HAT kicked us: reason code ${reason}`);
            break;
        }
        default:
            log('HAT', 'RX', id, b, '');
    }
}

function renderChars() {
    const box = $('chars');
    box.innerHTML = '';
    if (state.chars.length === 0) {
        box.innerHTML = '<div class="dim">no characters on this login — a new one will be created on join</div>';
        $('nick').disabled = false;
        return;
    }
    for (const c of state.chars) {
        const label = document.createElement('label');
        const radio = document.createElement('input');
        radio.type = 'radio';
        radio.name = 'char';
        radio.value = `${c.id1}:${c.id2}`;
        label.appendChild(radio);
        label.appendChild(document.createTextNode(` char ${c.id1}/${c.id2}`));
        box.appendChild(label);
    }
    const fresh = document.createElement('label');
    const radio = document.createElement('input');
    radio.type = 'radio';
    radio.name = 'char';
    radio.value = 'new';
    radio.checked = true;
    fresh.appendChild(radio);
    fresh.appendChild(document.createTextNode(' create new character'));
    box.appendChild(fresh);
}

function renderServers() {
    const list = $('servers');
    list.innerHTML = '';
    state.servers.forEach((srv, idx) => {
        const row = document.createElement('div');
        row.className = 'srvrow';
        row.innerHTML =
            `<b>${srv.name}</b> — ${srv.map} ${srv.size}, diff ${srv.difficulty}, ` +
            `${srv.players} pl, <code>${srv.addr}</code>`;
        row.onclick = () => {
            state.selectedServer = srv;
            [...list.children].forEach(c => c.classList.remove('sel'));
            row.classList.add('sel');
            $('join').disabled = false;
        };
        list.appendChild(row);
    });
    if (state.servers.length === 0) {
        list.innerHTML = '<div class="dim">no games visible — start the game server (rea2s)</div>';
    }
}

// ------------------------------------------------------------ join (0xCB) --

function joinSelected() {
    if (!state.selectedServer || !hatConn) return;
    const nick = $('nick').value.trim();
    const picked = document.querySelector('input[name="char"]:checked');
    const isNew = !picked || picked.value === 'new';
    let id1, id2;

    if (isNew) {
        if (nick.length < 3) { alert('nickname must be at least 3 chars'); return; }
        id1 = (Math.random() * 0xFFFFFFF) >>> 0;
        id2 = (Math.random() * 0xFFFFFF) >>> 0; // keep clear of the 0x3F000000 GM mask
    } else {
        [id1, id2] = picked.value.split(':').map(Number);
    }

    const body = 15, reaction = 15, mind = 15, spirit = 15, sphere = 1, face = 0x20, color = 0;
    state.joinStats = { body, reaction, mind, spirit, sphere, face, color };

    const nickB = strBytes(nick || 'Guest');
    const srvB = strBytes(state.selectedServer.addr);
    const data = new Uint8Array(16 + nickB.length + srvB.length);
    const dv = new DataView(data.buffer);
    dv.setUint32(0, id1, true);
    dv.setUint32(4, id2, true);
    data[8] = body; data[9] = reaction; data[10] = mind; data[11] = spirit;
    data[12] = sphere; data[13] = face; data[14] = color;
    data[15] = nickB.length;
    data.set(nickB, 16);
    data.set(srvB, 16 + nickB.length);

    logInfo(`entering server ${state.selectedServer.addr} (0xCB) as ${isNew ? 'new char "' + nick + '"' : 'char ' + id1 + '/' + id2}…`);
    hatConn.sendStream([pktData(0xCB, data)]);
    log('HAT', 'TX', 0xCB, new Uint8Array([0xCB, ...u32le(data.length)]),
        `nick="${nick}" srv=${state.selectedServer.addr} id1=${id1} id2=${id2}`);
}

// ----------------------------------------------------------- game handling --

function gameConnect() {
    if (!state.selectedServer || !state.ticket) return;
    const [ghost, gportStr] = state.selectedServer.addr.split(':');
    const gport = parseInt(gportStr, 10) || 9992;

    const { ws, conn } = openBridge(ghost, gport, 'GAME');
    gameWs = ws;
    gameConn = conn;

    ws.onopen = () => {
        logInfo(`game connected ${ghost}:${gport}, claiming hat ticket…`);
        conn.onPacket = gamePacket;
        const t = state.ticket;
        const s = state.joinStats || { body: 15, reaction: 15, mind: 15, spirit: 15, sphere: 1, face: 0x20, color: 0 };

        const enter = new Uint8Array(45); // 0x02: id + 3*u32 + 32 zeros
        enter[0] = 0x02;
        const p48 = pkt2dw(0x48,
            (s.body) | (s.reaction << 8) | (s.mind << 16) | (s.spirit << 24),
            (s.sphere) | (s.face << 8) | (s.color << 16));

        const steps = [
            [0, () => conn.sendStream([pkt2dw(0xD0, t.id1, t.id2)])],
            [500, () => conn.sendStream([pktDword(0xC1, 100)])],
            [500, () => conn.sendStream([enter])],
            [700, () => conn.sendStream([pkt2dw(0x39, 0, 0)])],
            [500, () => conn.sendStream([p48])],
            [700, () => conn.sendStream([pkt2dw(0x04, 0, 0)])],
        ];
        for (const [delay, fn] of steps) setTimeout(fn, delay);
        setTimeout(() => logInfo('join sequence sent — logging incoming packets…'), 1600);
        log('GAME', 'TX', 0xD0, new Uint8Array([0xD0]), `claim ticket id1=${t.id1} id2=${t.id2}`);
    };
    ws.onclose = () => logInfo('game connection closed');
}

function gamePacket(id, b) {
    bump(id);
    if (id === 0x64) { log('GAME', 'RX', id, b, `tick=${rd32(b, 1)}`); return; }

    switch (id) {
        case 0x96: {
            const pid = b[1], tok = b[2], color = b[3], flags = b[4];
            const namelen = rd16(b, 5);
            const name = new TextDecoder('latin1').decode(b.subarray(7, 7 + namelen));
            state.players.set(pid, name);
            log('GAME', 'RX', id, b, `player id=${pid} name="${name}" color=${color} flags=${flags} tok=${tok}`);
            break;
        }
        case 0x6C: case 0x6E: case 0x6F: case 0x70: {
            const u = decodeUnitUpdate(b);
            const known = state.units.has(u.unitId);
            let verdict = '';
            if (!known) {
                const size = state.players.size ? Math.max(...state.players.keys()) + 1 : 0;
                const owner = u.fields.owner;
                if (u.unitId === 0) verdict = 'DROP: bad id';
                else if (owner !== undefined && owner >= size) verdict = `DROP: owner ${owner} >= ${size}`;
                else if ((u.flags & 0xa0) === 0) verdict = 'DROP: no create bits';
                else { verdict = 'CREATE'; state.units.set(u.unitId, { fields: {}, count: 0 }); }
            }
            const entry = state.units.get(u.unitId);
            if (entry) {
                entry.count++;
                Object.assign(entry.fields, u.fields);
                if (u.fields.name) entry.name = u.fields.name;
            }
            const f = u.fields;
            const posBits = f.pos || f.sub;
            log('GAME', 'RX', id, b,
                `unit 0x${u.unitId.toString(16)} ${verdict ? verdict + ' ' : ''}` +
                (f.owner !== undefined ? `owner=${f.owner}(${state.players.get(f.owner) || '?'}) ` : '') +
                (f['type,face'] ? `type=${f['type,face'][0]} face=${f['type,face'][1]} ` : '') +
                (f.hp !== undefined ? `hp=${f.hp} ` : '') +
                (posBits ? `pos=[${posBits}] ` : '') +
                (f.name ? `"${f.name}"` : ''));
            renderUnits();
            break;
        }
        case 0xC0: {
            const chunkNo = rd32(b, 1);
            log('GAME', 'RX', id, b, `map data chunk #${chunkNo} (${b.length}B)`);
            break;
        }
        case 0x97: {
            const pid = rd16(b, 1);
            log('GAME', 'RX', id, b, `player left id=${pid}`);
            break;
        }
        case 0x0B: {
            log('GAME', 'RX', id, b, `KICKED reason=${b[1]}`);
            logInfo(`game server kicked us: reason ${b[1]}`);
            break;
        }
        default:
            log('GAME', 'RX', id, b, '');
    }
}

function renderUnits() {
    const t = $('units');
    const rows = [...state.units.entries()].map(([uid, u]) => {
        const f = u.fields;
        return `<tr><td>0x${uid.toString(16)}</td><td>${u.name || (f['type,face'] ? 'type ' + f['type,face'][0] : '')}</td>` +
            `<td>${f.owner !== undefined ? f.owner : ''}</td><td>${f.hp !== undefined ? f.hp : ''}</td>` +
            `<td>${f.pos ? f.pos.join(',') : (f.sub ? f.sub.join(',') : '')}</td><td>${u.count}</td></tr>`;
    });
    t.innerHTML = rows.join('');
}

// ------------------------------------------------------------------- wiring --

$('hat-connect').onclick = hatConnect;
$('refresh').onclick = () => {
    if (hatConn) {
        hatConn.sendStream([pkt1(0xC8)]);
        log('HAT', 'TX', 0xC8, new Uint8Array([0xC8]), 'server list request');
    }
};
$('join').onclick = joinSelected;
$('join').disabled = true;
$('clear-log').onclick = () => { $('pktlog').innerHTML = ''; };
$('pause').onclick = (ev) => { state.logPaused = ev.target.checked; };
$('hide-ticks').onchange = (ev) => { state.hideTicks = ev.target.checked; };
