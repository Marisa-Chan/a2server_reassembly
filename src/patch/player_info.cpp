// Ported from my-srvmgr/player_info.cpp. zxmgr::GetPlayers() iteration is
// g_PlayersList here.

#include "player_info.h"
#include "gm_commands.h"
#include "screenshots.h"

#include "../game_app.h"
#include "../net.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

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

    uint16_t p_id = player->player_id;
    for (int i = 0; i < 32; i++) {
        if (!g_PlayerInfo[i].Exists || g_PlayerInfo[i].Class == nullptr) {
            continue;
        }
        if (g_PlayerInfo[i].GodMode && g_PlayerInfo[i].GodSetter == player) {
            g_PlayerInfo[i].GodMode = false;
            g_PlayerInfo[i].GodSetter = nullptr;
        }

        Human* other_unit = g_PlayerInfo[i].Class->main_unit;
        if (other_unit != nullptr && g_PlayerInfo[i].SetSpells != 0 &&
            g_PlayerInfo[i].SpellSetter == player && i != static_cast<int>(p_id) - 1) {
            tangar_gm::SetSpells(other_unit, g_PlayerInfo[i].LastSpells);
            g_PlayerInfo[i].SetSpells = 0;
            g_PlayerInfo[i].LastSpells = 0;
            g_PlayerInfo[i].SpellSetter = nullptr;
            g_NetStru1_main.SendUnitUpdate(other_unit, nullptr, 0xFFFFFFFFu, 0xFFB, 0, 0);
        }
    }

    Human* unit = player->main_unit;
    if (unit != nullptr && g_PlayerInfo[p_id - 1].SetSpells != 0) {
        tangar_gm::SetSpells(unit, g_PlayerInfo[p_id - 1].LastSpells);
        g_PlayerInfo[p_id - 1].SetSpells = 0;
        g_PlayerInfo[p_id - 1].LastSpells = 0;
        g_PlayerInfo[p_id - 1].SpellSetter = nullptr;
    }

    PI_Clear(g_PlayerInfo[p_id - 1]);
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
