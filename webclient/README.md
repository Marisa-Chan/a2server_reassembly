# A2 browser client — prototype

A browser client that logs in to the running HAT (rom_hat), lists the games
(maps), joins one, and logs the incoming game packets (ticks, unit updates,
effects…). No rendering yet — this is the transport/protocol layer plus a
packet console.

## Run

```
python webclient/bridge.py          # serves the page + WS<->TCP bridge on 127.0.0.1:8765
```

Then open http://127.0.0.1:8765/, fill in the HAT login (default
`testlogin` / `12345` against rom_hat on 127.0.0.1:8000), pick a game, pick or
create a character, and join. The right panel shows every packet both ways;
the left panel tracks the unit registry built from 0x6c-family updates.

Requires a running rom_hat (asmland, clients port 8000) and a registered game
server (e.g. `asmland/Server1/rea2s.exe -window`, game port 9991).

## Architecture

- `bridge.py` — asyncio HTTP static server + WebSocket-to-raw-TCP pipe at
  `/ws?host=&port=`. Browsers cannot open raw sockets; this is the only
  server-side shim. Later it can be replaced by a native WebSocket listener
  inside rea2s.
- `public/a2net.js` — the A2 wire protocol: CLlDriver frame codec, packet
  builders, the packet-length table, and the unit-update field decoder
  (pack order mirrors `NetStru1::SendUnitUpdate` in `src/net.cpp`).
- `public/app.js` — the HAT login → server list → join → packet-log flow.

## Wire protocol (cheat sheet)

Both the HAT link and the game link use CLlDriver TCP provider 4:

```
frame  = [pos:u16][pad:u16][cmode:u8][csize:u16][pkt_num:u8] + xor(payload)
stream = packets back-to-back + seal [64 01 00 00 00]   (PacketDword 0x64, value 1)
```

- xor = 80-byte key from `util.cpp XorData`, index resets each frame.
- payload chunk ≤ 142 bytes; `pkt_num` on a frame counts the seals inside it;
  the receiver concatenates payloads until a frame with `pkt_num > 0` arrives.

### HAT flow (rom_hat, protocol v10-era retail client)

| dir | packet | layout |
|-----|--------|--------|
| → | 0xC9 login | `[C9][loginlen][0][mode][7][u16 len][login+password][0]` (PacketJoin) |
| ← | 0xCE char list | `[CE][count][hat_id:u32][id1:u32 id2:u32]…` |
| → | 0xC8 list request | `[C8]` |
| ← | 0xCD server list | text: `CURRENTCOUNT\|%2u$BREAK…` then `\|name\|1.02\|map\|WxH\|diff\|players\|addr:port` per game |
| → | 0xCB enter | `[CB][count][id1][id2][body][react][mind][spirit][sphere][face][sex][nicklen][nick][srvaddr]` |
| ← | 0xD0 ticket | `[D0][id1][id2]` — use as claim on the game connection |
| ← | 0xDA reconnect | `[DA][count][hat_id][id1][id2][key1][key2][addr]` |

### Game join (proven sequence, matches a2probe_scratch/a2join.py)

```
connect → 0xD0(id1,id2) claim → 0xC1(100) → 0x02 enter(45B) → 0x39(0,0)
        → 0x48(body|react|mind|spirit, sphere|face|sex) → 0x04(0,0)
```

The hero then arrives as 0x96 (player registry) + 0x6c with create bits
(0x20|0x80); positions ride in unit-update flag 0x10 (facing byte + subcell
u16 or cell x,y).

## Known gaps (iteration 2 material)

- position decode shown raw (`pos=[b0,b1]`), subcell-vs-cell not resolved
- `0x0` packets show up rarely (unmapped id, logged as truncated)
- no keepalive handling on long sessions, no ping graph
- unit 0x6b/0x6d "UnitState" not decoded (looks like anim/state per unit)
