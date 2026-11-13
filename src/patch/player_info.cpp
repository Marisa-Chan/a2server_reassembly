// Ported from my-srvmgr/player_info.cpp. zxmgr::GetPlayers() iteration is
// g_PlayersList here; the SetSpells restore (needs the #setspells spellbook
// rebuild) and ClientScreenshot_DropPlayer return with those features.

#include "player_info.h"
#include "screenshots.h"

#include "../game_app.h"
#include "../players_list.h"
#include "../server.h"

PlayerInfo g_PlayerInfo[32];

void PI_Clear(PlayerInfo& struc) {
    struc.Exists = false;
    struc.Class = nullptr;
    struc.Casted = false;
    struc.ShouldReturn = false;
    struc.LastReturn = 0;
    struc.GodMode = false;
    struc.GodSetter = nullptr;
    struc.SetSpells = 0;
    struc.LastSpells = 0;
    struc.SpellSetter = nullptr;
    struc.UnmuteDate = 0;
    struc.EnqueuedPackets.clear();
    memset(struc.Vision, 0, sizeof(struc.Vision));
}

void PI_Reset() {
    for (int i = 0; i < 32; i++) {
        PI_Clear(g_PlayerInfo[i]);
    }

    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (player == nullptr) {
            continue;
        }
        if (player->player_id < 1 || player->player_id > 32) {
            continue;
        }

        PI_Create(player);
    }
}

void PI_Create(Player* player) {
    if (player == nullptr) {
        return;
    }
    if (player->player_id < 1 || player->player_id > 32) {
        return;
    }

    PlayerInfo& struc = g_PlayerInfo[player->player_id - 1];
    PI_Clear(struc);
    struc.Exists = true;
    struc.Class = player;
}

void PI_Delete(Player* player) {
    if (player == nullptr) {
        return;
    }
    if (player->player_id < 1 || player->player_id > 32) {
        return;
    }

    for (int i = 0; i < 32; i++) {
        if (!g_PlayerInfo[i].Exists || g_PlayerInfo[i].Class == nullptr) {
            continue;
        }
        if (g_PlayerInfo[i].GodMode && g_PlayerInfo[i].GodSetter == player) {
            g_PlayerInfo[i].GodMode = false;
            g_PlayerInfo[i].GodSetter = nullptr;
        }

        // The SetSpells restore for other players (zxmgr::SetSpells) returns
        // with the #setspells feature.
    }

    // The own-unit SetSpells restore returns with the #setspells feature.

    PI_Clear(g_PlayerInfo[player->player_id - 1]);
    ClientScreenshot_DropPlayer(player);
}

PlayerInfo* PI_Get(Player* player) {
    if (player == nullptr) {
        return nullptr;
    }
    if (player->player_id < 1 || player->player_id > 32) {
        return nullptr;
    }
    if (!g_PlayerInfo[player->player_id - 1].Exists) {
        return nullptr;
    }
    return &g_PlayerInfo[player->player_id - 1];
}
