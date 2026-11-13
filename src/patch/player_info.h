#pragma once

// Tangar per-player info, ported from my-srvmgr/player_info.{h,cpp}.
// Backs the timed mute system (UnmuteDate) and GM state (god-mode setters,
// spell-set restore); created on join (imp_PIadd), deleted on leave
// (imp_PIdel), rebuilt on map load (PI_Reset, imp_LoadedMap).
//
// Renames/omissions vs the original: the struct Player -> PlayerInfo (the
// game already has a Player class), Players[32] -> g_PlayerInfo; the
// SavedItems (forbidden items), CastSpell[0x14] and EnqueuedPackets (HAT
// net) fields return together with those features.

#include <cstdint>

#include "../player.h"

struct PlayerInfo {
    bool Exists;
    Player* Class;

    bool Casted;
    bool ShouldReturn;
    uint32_t LastReturn;
    bool GodMode;
    Player* GodSetter;

    int8_t SetSpells;
    uint32_t LastSpells;
    Player* SpellSetter;

    uint32_t UnmuteDate;

    // used in scanrange (patch/scanrange.cpp): GetTickCount() stamps of the
    // cells the player currently sees
    uint32_t Vision[256][256];
};

extern PlayerInfo g_PlayerInfo[32];

PlayerInfo* PI_Get(Player* player);
void PI_Reset();
void PI_Clear(PlayerInfo& struc);
void PI_Create(Player* player);
void PI_Delete(Player* player);
