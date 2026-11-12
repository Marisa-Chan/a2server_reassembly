#pragma once

// Tangar reborn readiness, ported from my-srvmgr reborn_readiness.cpp. Reports
// whether a character meets the reborn / reclass / ascend / hell-circle
// requirements from thresholds.cfg (#reborn and #hell commands).

#include "config_new.h"

class Player;

namespace reborn {

void RebornReadinessInfo(ServerIDType server_id, const Player* player, bool hell);

} // namespace reborn
