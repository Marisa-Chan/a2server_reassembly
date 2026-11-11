#pragma once

// Tangar "advanced PvM" strong/weak detection, ported from my-srvmgr/pvm2.cpp
// (vd2_CheckStrong and its spell/scroll/item queries). A "strong" character
// has overcapped stats, tier-3 spells, or too many high-tier scrolls/items;
// under SVF_ADVPVM strong and weak players cannot damage each other.

class Unit;

namespace tangar_pvm2 {

// True if the unit qualifies as "strong".
bool CheckStrong(Unit* unit);

// VerifyDamage2: true when the damage between the two main units must be
// blocked (strong vs weak in either direction).
bool VerifyDamage2(Unit* attacker, Unit* victim);

} // namespace tangar_pvm2
