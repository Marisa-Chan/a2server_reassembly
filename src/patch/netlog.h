// Centralized network debug logging (both rea2s and rea2c builds).
// Opt-in via the command line: --net-log=DIRECTORY writes
// DIRECTORY\server.log (rea2s) or DIRECTORY\client.log (rea2c); without the
// flag nothing is logged and every call is a no-op.
//
// Everything is best-effort: if the directory cannot be created/opened the
// calls degrade to no-ops so logging can never take the game down.

#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>

#ifdef _WINSOCK2API_
#include <winsock2.h>
#endif

namespace netlog {

// One formatted line: "[HH:MM:SS.mmm] <dir> <message>". dir examples:
// "S->C", "C->S", "SRV->HAT", "HAT->SRV", "EVT".
void logf(const char* dir, const char* fmt, ...);

// Same as logf, plus a hex dump of data (capped).
void hex(const char* dir, const void* data, size_t len, const char* fmt, ...);

// Log a CLlDriver TCP frame (8-byte header + XORed payload): de-XORs the
// payload, parses the game packet id stream and prints ids + hex.
// `framedata` points at the full frame ([pos:u16][pad:u16][cmode:u8]
// [csize:u16][pkt_num:u8] + payload), `len` = total frame bytes.
// `peer` identifies the remote end (used for direction labels).
void frame(const char* dir, long peer_port, const void* framedata, size_t len, const char* fmt, ...);

// Connection event: "EVT <what> sock=0x.. peer=ip:port".
void conn(const char* what, uint64_t sock, const char* peer_ip, int peer_port);

// Best-effort readable name for a game packet id (or "0xXX").
const char* pkt_name(uint8_t id);

} // namespace netlog
