# Authorization (v20) protocol migration — spec & port plan

STATUS: **PORTED** (2026-11-13):
- `d8947292` — v20 HAT authorization protocol, `key_20` crypt, extended `0x80100000` TCP framing, and `RunModalLoopClient` `-hat` fix (`src/patch/hat_client.{h,cpp}`, `src/packet.cpp`, `src/net.cpp`, `src/util.cpp`, `src/patch/hat_net.{h,cpp}`, `src/patch/player_info.{h,cpp}`, `src/main_window.cpp`)
- `08196993` — HAT-triggered screenshot capture (`0x80100000` opcode `0x01`), `imp_MakeScreenshotNet` (`UnlockSurface1`), PNG upload (`hat:8000` protover 24 + `api.allods2.eu:8080/posts.php` via vendored `happyhttp`), server `src/patch/screenshots.{h,cpp}`, and `#screenshot <nick>` GM command
- `948326b3` — HTTP auto-updater (`NETPROTO_updater` `/allods/version_id` + `/allods/version` with graceful pass-through) and `-delay <ms>` startup flag (`src/patch/hat_client.{h,cpp}`, `src/game_app.cpp`, `src/main_window.cpp`)

USER DECISIONS (in force):
- Patched client talks ONLY to patched server; non-patched only to non-patched.
  REPLACE the old auth on both ends, do NOT dual-accept.
- The HTTP auto-updater and HAT screenshot capture+upload are ported alongside this cluster.

## 1. Scope — the client hooks (allods2.mp)

| Hook VA | Export | Kind | What it is |
|---|---|---|---|
| `50E48B` | NETPROTO_createAuthorizationPacket (ord15) | entry takeover | builds the v20 auth packet (our `NetStru1::SendHatCredentials` twin, net.cpp:4956) |
| `515D77` | NETPROTO_sendAuthorizationPacket (ord16) | entry takeover | auth-size-aware `PacketJoin::VMethod3` (write/send) |
| `589260` | recv_ (ord97) | entry on `WSOCK32.DLL::Ordinal_16` | framed reader replacing raw `recv` in `RecvThreadTcp` |
| `5122D1` / `512314` | NETPROTO_enlargeBuffer / 2 (ord100/101) | entry + inline | swap RecvThreadTcp's 0x2000 stack buffer for the 51200-byte `newNetBuffer` |
| `50FEED` | NET_cryptPacket (ord18) | entry on `Game::XorData` | version-keyed XOR (see §6) |
| `4949CD` | NETPROTO_loginToHat (ord84) | entry on `RunModalLoopClient` | login wrapper + SHA1 CRC anticheat |
| `457FF4` | imp_MakeScreenshotNet (ord109) | entry on `UnlockSurface1` region | HAT-triggered screenshot (0x64/0x01 packet — rides with this cluster) |

Server counterparts live in `my-srvmgr/protolayer_hat.*` — mostly ported already
as `src/patch/hat_net.{h,cpp}` + `src/patch/hat_packet.{h,cpp}` (control
connection to the HAT master). The missing piece is the game-socket auth shape
below on BOTH ends.

## 2. Baseline flow in OUR tree (what gets replaced)

1. Client: `MainWindow` connect path → `MainWindow::RunModalLoopClient`
   (main_window.cpp:5556, client twin 4949cd) →
2. `NetStru1::SendHatCredentials(login, password, flags)` — net.cpp:4956
   (client twin 51d8d6):
   - builds `PacketJoin` with `id = HAT_CREDENTIALS (0xC9)`,
     `name = login + password` (plain concat), `player_id = login.GetLength()`,
     `color = flags`
   - baseline packet header: size word @+0x0E, alloc 0x10 (see §4 for the
     "auth" variants +0x4E/+0x50)
3. Server: routes 0xC9 in the packet-id switch (net.cpp:1073) together with
   MAP_INFO/HAT_LOGIN/... as a `PacketJoin`; hat_id handshake follows
   (client waits on `net_session.hat_id`).

## 3. v20 auth packet format (createAuthorizationPacket decode)

Built at client `50E48B` (takes login/color args like SendHatCredentials;
`0x401860` = the CString-format helper producing `"login:color-or-password"`
into `packet+0x50`).

Layout (offsets from packet start; the baseline packet object is the
`PacketJoin`-shaped buffer at client global `0x699FC8` in the patch):

| Offset | Size | Meaning |
|---|---|---|
| +0x07 | 2 | **set to 0** — the "this is an auth packet" marker (normal packets have nonzero here; this is how the sender/receiver branches) |
| +0x09 | 1 | 0 = auth packet marker byte (checked by sendAuthorization: `byte[+9]==0` → auth sizing) |
| +0x0A | 0x30 | anticheat fingerprint (crcmain, §5) |
| +0x4A | 4 | `crcmain(packet+0x0A)` (low 16) \| loginlen<<16 \| `PROTO_VER=20`<<24 |
| +0x4E | 2 | total packet size word (auth variant; normal packets use +0x0E) |
| +0x50 | var | `"login:color"` (or password) string |

Send path (`sendAuthorizationPacket`, client 515D77 = `PacketJoin::VMethod3`
twin): if `byte[+9]==0` → size word at +0x4E, total = `strlen@0x50 + 0x50 + 8`;
else normal → size word at +0x0E, total = `+0x10 + 8`. Payload sent from
`packet+9` (so the marker byte travels first).

## 4. recv_ framing (client 589260, replaces raw recv in RecvThreadTcp)

State machine (module statics `recv_lastCustom/recv_lastIncomplete/
recv_lastRemaining`):

1. If a previous payload recv was incomplete → `recv(s, buf, remaining)`
   repeatedly until complete, then return.
2. Read 4B `pkt_size`, then 4B `pkt_flags`.
3. **Special HAT packet**: `(pkt_flags & 0x80100000) == 0x80100000` →
   read `pkt_size` payload bytes into a `Packet`, hand to
   `Cl_ProcessServerPacket` (network_protocol_ext.cpp:825; currently the
   0x01 = screenshot-request handler), then SYNTHESIZE a normal-looking
   return: header `{5, 0x01000000}` + payload `0x64 ^ key_20[0]`,
   `1 ^ *(uint32_t*)(key_20+1)` → i.e. the game sees an encrypted 0x64
   (HAT_SCORE_LIST-ish) packet. On handler failure return -1 (drops the link).
4. **Normal game packet**: read `pkt_size` payload into `buf+8`, return
   `size + 8`. Partial recv → set the incomplete state and continue next call.

Note the payload is still XOR-crypted with key_20 by the upper layers — the
commented-out debug loop in recv_ shows `buf[8+i] ^ key_20[i%0x50]`.

`NETPROTO_fixRecv` (40DB2C, in ProcessPackets) adjusts the client's receive
accounting for the 8-byte framing header (`len - 3` patch on the copied length).

enlargeBuffer x2 (5122D1/512314): RecvThreadTcp's stack buffer becomes the
static `char newNetBuffer[51200]` (our `CLlDriver::RecvThreadTcp` is
net.cpp:3124 (VA comment 522b95) — currently retail 0x2000 stack buffer + own copy_num framing).

## 5. crcmain — the fingerprint + the gamemode side effect

`crcmain(char* data)` (my-a2mgr network_protocol.cpp:21-78) fills the 0x30-byte
fingerprint at `packet+0x0A` (called as `crcmain(pkt+0x0A)`; `data+0x40` below
therefore ALIASES `packet+0x4A`, the crc/len/ver dword!):

- `net_key = 0x6CDB248D` (constant; the `_neI_SessionKey` variant is commented out)
- `crc32_X = CRC32(SHA1(file), 20)` for allods2.exe, a2mgr.dll, patch.res, world.res
- dword table:
  - `+0x00 = crc32_allods2 ^ net_key`
  - `+0x04 = (crc32_a2mgr ^ net_key) + crc32_allods2`
  - `+0x08 = crc32_patch ^ crc32_a2mgr`
  - `+0x0C = (crc32_world ^ net_key) - crc32_patch`
  - `+0x10 = net_key`
  - `+0x14 = sessionID ^ 0xDEADBEEF`
  - `+0x18..0x2B` = 20-byte sha1_uuid, each dword then XOR net_key
  - `+0x2C = CRC32(data+0x18, 20) ^ net_key` (computed BEFORE the uuid dwords
    are XORed — order matters)
- **gamemode side effect**: `uoth = *(dword*)(data+0x40)` = the packet's
  `+0x4A` dword; `gamemode = (uoth & 0x00FF0000) >> 16` = the loginlen<<16 field
  reused... in practice the caller's 3rd arg lands there: gamemode 4 (sandbox)
  → client resets `MainWindow+0x3C8` (hat_settings.deathmatch) to 0 and
  `z_softcore = 2`; gamemode 2 (softcore) → same reset, `z_softcore = 1`;
  else `z_softcore = 0`. I.e. the auth send is also where the patched client
  applies the forced game mode (ties into the GUI_softcoreSaveSwitch port —
  the z_softcore override paths we shipped dormant).

For OUR builds the file-hash content is decorative (both ends are ours; the
patched client verified the *patched* binaries). Plan: fill with real
`CRC32(SHA1(rea2c.exe))` + zeros, or skip verification entirely — decide at
port time; skip-verify is the documented favorite.

## 6. Crypt (NET_cryptPacket, client 50FEED on Game::XorData)

`cryptver(p, ver, len)` picks the XOR table by protocol version:
`<10 → cr08`, `==10 → cr10`, `11..19 → cr11`, `==20 → cr20`; XOR with
`key[(i % 0x8E) % 0x50]`. `PROTO_VER = 20` (my-a2mgr config.h:64).

**Already in our tree**: `src/patch/hat_packet.cpp` carries `key_20` byte-for-
byte identical to the manager's `cr20`, plus `PACKET_GetKey(ver)` and the
Packet reader/writer class (ported from lib/packet.hpp). The other three
tables (cr08/cr10/cr11) exist in my-a2mgr/network_crypt.cpp if ever needed.

## 7. Port plan onto our code

1. `patch/hat_client.{h,cpp}` (new, A2CLIENT_SOURCES):
   - `BuildAuthorizationPacket(PacketJoin* pkt, login, color)` — §3 layout;
     fingerprint per §5 decision.
   - crcmain equivalent + the gamemode/z_softcore side effect (our
     `MainWindow::hat_settings.deathmatch` + `tangar_cfg::z_softcore`).
   - `Cl_ProcessServerPacket` (screenshot request 0x01; log-only until the
     screenshot upload subsystem lands — see hat_net.h precedent).
2. `NetStru1::SendHatCredentials` (net.cpp:4956): A2CLIENT_PATCH arm replaces
   the baseline PacketJoin fill with §3; retail in #else.
3. `PacketJoin::VMethod3`/write path (client twin 515D77): auth-size branch
   (+0x4E/+0x50 vs +0x0E/+0x10).
4. `CLlDriver::RecvThreadTcp` (net.cpp:522b95): A2CLIENT_PATCH arm with the
   §4 framing state machine + 51200 static buffer; retail in #else.
5. Server: replace the 0xC9 routing (net.cpp:1073) to parse the v20 shape
   (login/password from the +0x50 string, split on ':'), skip CRC verify.
   (Replace, not dual-accept — user decision.)
6. `NET_cryptPacket` (Game::XorData entry): route through
   `tangar_hat::PACKET_GetKey(PROTO_VER)`.
7. loginToHat (4949cd = our RunModalLoopClient, already touched by the HATENT
   port): add the SHA1-CRC anticheat gather around the connect (decorative for
   our builds — same decision as §5).
8. Then (separate commits): auto-updater (HTTP, /allods/version_id + file
   download — happyhttp in my-a2mgr/lib/), screenshot capture+upload
   (Cl_ScreenshotThread → hat.rom2.ru:8000; manual PNG screenshots already
   ported via lodepng), imp_MakeScreenshotNet.

## 8. Resolved implementation findings (verified against disasm & my-srvmgr)

1. **`0x50A218` send target**: `NetStru1::QueuePacketSend` (`51A0EE`, client `50A218`).
   `SendHatCredentials` hands the `PacketJoin` to `QueuePacketSend(0, &pkt, 1)`, which
   serializes via `PacketJoin::WriteTo` (`id == 0` writes `+0x09..+0x4F` + `name` string)
   and XOR-encrypts with `XorData` (`key_20` under patch `#if`).
2. **`+0x4A` dword layout & credentials**: Low 16 bits of `+0x4A` (`PacketJoin::player_id`)
   are `login.GetLength()` (not a CRC16!), bits 16..23 are `(gamemode & 0xFF)` (read by
   `crcmain` to reset `hat_settings.deathmatch = 0` and set `tangar_cfg::z_softcore` = 1 or 2,
   then cleared from `+0x4A` before send), and bits 24..31 are `PROTO_VER = 20`. The string
   at `+0x50` (`PacketJoin::name`) is plain `login + password` (concatenated without `':'`),
   sliced by `login.GetLength()` on the HAT (`a2hat` `Parse_TryLogin` reads `player_id & 0xFFFF`
   for `login_len` and `player_id >> 24` for `proto_ver == 20`).
3. **Server-side auth & framing**: `DecodePacket` (`src/net.cpp`) replaces `case HAT_CREDENTIALS:`
   with `case 0:` under `#ifdef A2SERVER_PATCH`, instantiating `PacketJoin` whose `ReadFrom`
   branches on `this->id == 0` (`+0x4E` size word, `+0x50` string). On the TCP game socket,
   `CLlDriver::RecvThreadTcp` routes through `RecvServerFramed` (`src/patch/hat_net.cpp`,
   `my-srvmgr/srvmgr_new.cpp:365` `recv_`) with a 51200B buffer, and `CLlDriver::SendDataTcp`
   flushes `PlayerInfo::EnqueuedPackets` as `0x80100000`-flagged frames before game packets.
4. **Synthetic `0x64` frame in client `RecvClientFramed`**: `0x64` is `PacketWord`
   `SERVER_TICK` (`value = 1`), encrypted with `key_20` (5 bytes total + 8-byte `{5, 0x01000000}`
   header), giving `RecvThreadTcp` a valid non-empty game packet after consuming an out-of-band
   `0x80100000` manager frame.

## 9. Source references (ground truth)

- my-a2mgr/network_protocol.cpp — createAuthorizationPacket (76-218),
  sendAuthorizationPacket (76-118), recv_ (222-309), NETPROTO_fixRecv (311-326),
  newNetBuffer/enlargeBuffer (330+), crcmain (21-78)
- my-a2mgr/network_protocol_ext.cpp (902 lines) — Cl_ProcessServerPacket (:825,
  0x01 screenshot request → ScreenshotInfo + ProcLastSI reading the screen
  surface @0x62571C/pitch @0x625708), HTTP auto-updater, hat-enter glue
- my-a2mgr/network_crypt.cpp — cryptver + cr08/cr10/cr11/cr20 tables
- my-a2mgr/config.h:64 — `#define PROTO_VER 20`
- my-srvmgr/protolayer_hat.* — server HAT control connection (ported as
  src/patch/hat_net.{h,cpp}); server-side auth accept lives around the 0xC9
  routing
- Our tree: src/net.cpp:4956 (SendHatCredentials), src/net.cpp:1073 (0xC9
  routing), src/net.cpp:3124 (CLlDriver::RecvThreadTcp),
  src/patch/hat_packet.{h,cpp} (Packet + key_20 + PACKET_GetKey),
  src/main_window.cpp:5556 (RunModalLoopClient; HATENT port cdef809c touches
  the PrepareForConnect call there)

Decoded across sessions 29-33b (memory: tangar-patch-porting.md); written to
this file 2026-11-13. The 4 open questions in §8 are the only unknowns.
