#pragma once

// Tangar GM noclip, ported from my-srvmgr (zxmgr::MakeUnitNoClip +
// CheckPlayerNoClip, hook imp_GMNoClip @ 5070EC). A GM with GMF_NOCLIP gets
// their unit's physical footprint cleared on return-to-map so they can walk
// through everything.

class Player;

void TangarCheckPlayerNoClip(Player* player);
