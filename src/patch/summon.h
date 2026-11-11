#pragma once

// Tangar summoned-unit rebalance, ported from my-srvmgr/summon.cpp.
// The DLL hook (summon @ 53AC32) replaces the plain `summoned = 1` store in
// the summon spell with a full stat override per ServerID and summon tier.

class Unit;

void TangarFixSummonedUnit(Unit* unit, int level);
