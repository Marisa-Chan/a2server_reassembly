#pragma once

#include <string>

#include "../player.h"

void KickPlayer(Player* player, bool silent = false);
void TangarKickAll(Player* except_player, bool silent);
void DisconnectPlayer(Player* player);
Player* FindPlayerByNickname(const char* nickname);
