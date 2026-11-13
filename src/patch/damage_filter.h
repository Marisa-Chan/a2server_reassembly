#pragma once

#include <cstdint>

// Tangar damage filter, ported from my-srvmgr/cheat_codes_new.cpp OnDamage +
// pvm2.cpp VerifyDamage2 + GetDamageBonus. Invoked via the three srv_pvm.xck
// caves (504033/50403F in Unit::RollDamage, 5040D2/5040DA in Spell::ExecuteAtArea,
// and 5048D3/5048E6 in Effect::ApplyToUnit): damage bonuses from gear, PvP mage
// factor and cap, PvM / advanced-PvM rules, GM maxdamage and god modes.

class Unit;

int32_t TangarOnDamage(Unit* attacker, Unit* victim, int16_t damage);
