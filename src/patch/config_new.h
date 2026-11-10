#pragma once

// Tangar server configuration surface, ported from my-srvmgr/config_new.h.
// The runtime knobs behind most A2SERVER_TANGAR_HAT features; parsed from
// server.cfg by ReadConfig (config_new.cpp), which in the patched binary
// replaces the stock config parser (imp_ParseConfig takeover @ 0x4F7188).

#include <stdint.h>
#include <string>
#include <vector>

extern unsigned long MAX_SKILL;
extern unsigned long MAX_EXP_ON_SKILL;

struct MainCharacterParameters {
    uint8_t Body, Reaction, Mind, Spirit;
    uint8_t ResistFire, ResistWater, ResistAir, ResistEarth, ResistAstral;
};

enum ServerIDType {
    UNDEFINED = 0,
    EASY = 1,
    KIDS = 2,
    NIVAL = 3,
    MEDIUM = 4,
    HARD = 5,
    NIGHTMARE = 6,
    QUEST_T1 = 7,
    QUEST_T2 = 8,
    QUEST_T3 = 9,
    QUEST_T4 = 10,
};

namespace Config
{
    extern uint32_t ExceptionCount;

    extern uint32_t LogMode;
    extern std::string LogFile;

    extern std::string ChrBase;

    extern ServerIDType ServerID;

    extern bool ServerStarted;

    extern std::string CurrentMapName;
    extern std::string CurrentMapTitle;

    extern uint32_t ServerFlags;
    extern bool MapLoaded;
    extern uint32_t ProtocolVersion;

    extern uint32_t MaxPaletteAllowed;

    extern std::string SqlAddress;
    extern uint16_t SqlPort;
    extern std::string SqlLogin;
    extern std::string SqlPassword;
    extern std::string SqlDatabase;

    extern uint32_t ServerCaps;
    extern uint32_t GameMode;

    extern bool Suspended;
    extern uint32_t OriginalTime;
    extern uint32_t MaxPlayers;

    extern std::vector<std::string> Includes;

    extern std::string ControlDirectory;

    extern bool ExitingCleanly;

    extern double DeathExpMult;
    extern double KilledExpMult;
    extern double PKExpMult;
    extern double RespawnExpMult2;

    extern float InventoryDropProbability;
    extern float WearDropProbability;

    extern MainCharacterParameters WarriorMaleMaxParameters;
    extern MainCharacterParameters WarriorFemaleMaxParameters;
    extern MainCharacterParameters MageMaleMaxParameters;
    extern MainCharacterParameters MageFemaleMaxParameters;

    extern uint32_t MinQuestReward;
    extern uint32_t MaxQuestReward;

    extern uint32_t QuestRollInterval;
    extern float mage_pvp_dmg_factor;
    extern int16_t max_pvp_dmg;
    extern float shop_potions_factor;
    extern bool server_rotate_maps;
    extern bool shuffle_maps;
    extern bool server_restart_on_map_change;

    extern bool AllowOnlyOneQuest_KillNMonsters;
    extern bool AllowOnlyOneQuest_KillTheMonster;
    extern bool AllowOnlyOneQuest_KillTheGroup;
    extern bool AllowQuestFilters;

    extern std::string IPAddress;
    extern uint16_t IPAddressP;

    extern std::string IPAddress2;
    extern uint16_t IPAddress2P;

    extern std::string HatAddress;
    extern uint16_t HatAddressP;

    extern uint32_t CheatsAdditionalRights;
}

int ReadConfig(const char* filename);
uint32_t ParseFlags(std::string flags_str);
