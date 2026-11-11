#pragma once

#include <cstdint>

// Tangar damage filter, ported from my-srvmgr/cheat_codes_new.cpp OnDamage +
// pvm2.cpp VerifyDamage2 + GetDamageBonus. In the patched binary this runs on
// the GM kill-command damage (the three verify_damage cave sites inside
// CheatCommand): damage bonuses from gear, PvP mage factor and cap, PvM /
// advanced-PvM rules, GM maxdamage and god modes.

class Unit;

int32_t TangarOnDamage(Unit* attacker, Unit* victim, int16_t damage);
