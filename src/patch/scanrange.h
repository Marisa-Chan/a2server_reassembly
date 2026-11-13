#pragma once

// Server-side scan-range subsystem, ported from my-srvmgr scanrange.cpp
// (SR_Step called from OnServerTic; #scan dumps via CheatCommand). Every
// tick each unit's terrain-aware vision (see scanrange_calc.h) is stamped
// with GetTickCount() into the owning player's Vision table — and the
// tables of players the owner shares vision with (diplomacy bit 0x10).

#include <stdint.h>

class Player;

namespace tangar_scanrange {

// Recompute all vision tables; call once per server tick.
void SR_Step();

// True when any of the 3x3 cells around (check_x, check_y) was seen by the
// player within the last 3000 ms. (Unused by the manager too; kept for
// parity — the natural gate for future server-side visibility checks.)
bool SR_CheckVision(Player* player, uint8_t check_x, uint8_t check_y);

// Dump the player's current vision grid into the log (the #scan command).
void SR_DumpToFile(Player* player);

} // namespace tangar_scanrange
