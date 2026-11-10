// Tangar server configuration (config v2.0), ported from
// my-srvmgr/config_new.cpp. In the patched binary this replaces the stock
// config parser (imp_ParseConfig @ 0x4F7188).
//
// The original wrote parsed values straight into the server binary's config
// block at hardcoded addresses; those writes are re-targeted here at the
// named g_ServerConfig fields (ServerConfig @ 0x6D15A0, see server.h), each
// carrying its original address in a comment.

#include "config_new.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "flags.h"
#include "server_state.h"
#include "tangar_log.h"
#include "tangar_utils.h"

#include "../game_app.h"

unsigned long MAX_SKILL = 110;              // Vanilla (V): 100; we use SOFTCORE values by default
unsigned long MAX_EXP_ON_SKILL = 35742360;  // V: 13779612;

uint32_t ParseLogFlags(std::string flags_line)
{
    if (CheckHex(flags_line)) {
        return HexToInt(flags_line);
    }

    uint32_t flags = 0;
    std::vector<std::string> v = Explode(flags_line, "|");
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
        std::string par = ToLower(Trim((*it)));
        if (!par.length()) {
            continue;
        }
        uint32_t flag = 0;
        bool erase = false;
        if (par[0] == '-') {
            erase = true;
            par.erase(0, 1);
        }

        if (par == "camping") {
            flag = SVL_CAMPING;
        } else if (par == "buildings") {
            flag = SVL_BUILDINGS;
        } else if (par == "saves") {
            flag = SVL_SAVES;
        } else if (par == "diplomacy") {
            flag = SVL_DIPLOMACY;
        } else if (par == "all") {
            flag = SVL_ALL;
        }

        if (!flag) {
            continue;
        }
        if (!erase) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }

    return flags;
}

// Ported from my-srvmgr/cheat_codes_new.cpp (used for the serverflags key).
uint32_t ParseFlags(std::string flags_str)
{
    if (CheckHex(flags_str)) {
        return HexToInt(flags_str);
    }

    uint32_t flags = 0;
    std::vector<std::string> v = Explode(flags_str, "|");
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
        std::string par = ToLower(Trim((*it)));
        if (!par.length()) {
            continue;
        }
        uint32_t flag = 0;
        bool erase = false;
        if (par[0] == '-') {
            erase = true;
            par.erase(0, 1);
        }

        if (par == "pvm") {
            flag = SVF_PVM;
        } else if (par == "muted") {
            flag = SVF_MUTED;
        } else if (par == "closed") {
            flag = SVF_CLOSED;
        } else if (par == "advanced_pvm") {
            flag = SVF_ADVPVM;
        } else if (par == "nohealing") {
            flag = SVF_NOHEALING;
        } else if (par == "noobsrv") {
            flag = SVF_NOOBSRV;
        } else if (par == "softcore") {
            flag = SVF_SOFTCORE;
        } else if (par == "nodrop") {
            flag = SVF_NODROP;
        } else if (par == "fnodrop") {
            flag = SVF_FNODROP;
        } else if (par == "nosaving") {
            flag = SVF_NOSAVING;
        } else if (par == "sandbox") {
            flag = SVF_SANDBOX;
        } else if (par == "entermage") {
            flag = SVF_ENTERMAGE;
        } else if (par == "enterwarrior") {
            flag = SVF_ENTERWARRIOR;
        }

        if (!flag) {
            continue;
        }
        if (!erase) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }

    return flags;
}

namespace Config
{
    uint32_t ExceptionCount = 0;

    std::string ChrBase = "chr";

    uint32_t LogMode = SVL_ALL;
    std::string LogFile = "server.log";

    ServerIDType ServerID = UNDEFINED;
    bool ServerStarted = false;
    std::string CurrentMapName = "N/A";
    std::string CurrentMapTitle = "N/A";

    uint32_t ServerFlags = 0;
    float InventoryDropProbability = 1;
    float WearDropProbability = 1;
    bool MapLoaded = false;
    uint32_t ProtocolVersion = 7;

    std::string IPAddress = "0.0.0.0";
    uint16_t IPAddressP = 8001;

    std::string IPAddress2 = "0.0.0.0";
    uint16_t IPAddress2P = 8002;

    std::string HatAddress = "127.0.0.1";
    uint16_t HatAddressP = 7999;

    uint32_t MaxPaletteAllowed = 16;

    std::string SqlAddress = "127.0.0.1";
    uint16_t SqlPort = 3306;
    std::string SqlLogin = "root";
    std::string SqlPassword = "";
    std::string SqlDatabase = "logins";

    uint32_t ServerCaps = 0;
    uint32_t GameMode = 0;

    bool Suspended = false;
    uint32_t OriginalTime = 0;

    std::vector<std::string> Includes;

    std::string ControlDirectory = "ctl";

    uint32_t MaxPlayers = 16;

    bool ExitingCleanly = false;

    double DeathExpMult     = 1;
    double KilledExpMult    = 1;
    double PKExpMult        = 1;
    double RespawnExpMult2  = 0.9;

    MainCharacterParameters WarriorMaleMaxParameters = { 52, 50, 48, 46, 100, 100, 100, 100, 100};
    MainCharacterParameters WarriorFemaleMaxParameters = { 50, 52, 46, 48, 100, 100, 100, 100, 100};
    MainCharacterParameters MageMaleMaxParameters = { 48, 46, 52, 50, 100, 100, 100, 100, 100};
    MainCharacterParameters MageFemaleMaxParameters = { 46, 48, 50, 52, 100, 100, 100, 100, 100};

    uint32_t MinQuestReward = 250;
    uint32_t MaxQuestReward = 16383000;

    uint32_t QuestRollInterval  = 0x78;

    float mage_pvp_dmg_factor = 1;
    int16_t max_pvp_dmg = 100;
    float shop_potions_factor = 1;
    bool server_rotate_maps = true;
    bool shuffle_maps = true;

    // Server will restart right after changing the map. Enabling together with
    // `shuffle_maps` doesn't make much sense.
    bool server_restart_on_map_change = true;

    // If these settings are set to true, it will forbid a player from taking
    // several quests for the same monster, group or monster type.
    bool AllowOnlyOneQuest_KillNMonsters = false;
    bool AllowOnlyOneQuest_KillTheMonster = false;
    bool AllowOnlyOneQuest_KillTheGroup = false;
    bool AllowQuestFilters = false;

    // Extra flags OR-ed to each player's flags when checking cheats
    // permissions. Useful for local debugging. Example: set to 0x3F000080
    // (GMF_CMD_SET) to allow everyone to use map control commands (#nextmap,
    // #speed and so on). Since this is a bitmask, accepts only hex values in
    // config.
    uint32_t CheatsAdditionalRights = 0;
}

int32_t ReadIntegerParameter(std::string value, int32_t MinValue, int32_t MaxValue)
{
    int32_t val = StrToUInt(value);
    if (val < MinValue) {
        val = MinValue;
    }
    if (val > MaxValue) {
        val = MaxValue;
    }
    return val;
}

float ReadFloatParameter(std::string value, float MinValue, float MaxValue)
{
    float val = StrToFloat(value);
    if (val < MinValue) {
        val = MinValue;
    }
    if (val > MaxValue) {
        val = MaxValue;
    }
    return val;
}

int ReadConfig(const char* filename)
{
    if (!Config::Includes.size()) { // root config
        // set defaults
        g_ServerConfig.ip_address = "127.0.0.1:8001";   // 0x6D15B0
        g_ServerConfig.ip_address2 = "127.0.0.1:8002";  // 0x6D15B4
        g_ServerConfig.hat_address = "127.0.0.1:7999";  // 0x6D15B8
        g_ServerConfig.chr_base = "chr";                // 0x6D15BC
    }

    std::ifstream f_cfg;
    f_cfg.open(filename, std::ios::in);
    if (!f_cfg.is_open()) {
        return -1;
    }

    std::string line;
    int32_t lnid = 0;
    std::string section = "root";

    std::vector<std::pair<std::string, uint32_t>> maps;

    while (std::getline(f_cfg, line)) {
        lnid++;
        bool enc = false;
        for (size_t i = 0; i < line.length(); i++) {
            if (line[i] == '"') {
                enc = !enc;
            }
            if (enc) {
                continue;
            }
            if (line[i] == '#' || line[i] == ';' ||
                (line[i] == '/' && i != line.length() - 1 && line[i + 1] == '/')) {
                line.erase(i);
                break;
            }
        }

        line = Trim(line);
        if (!line.length()) {
            continue;
        }

        if (line[0] == '!') {
            if (line.find("!include ") == 0) {
                line.erase(0, 9);
                line = Trim(line);
                if (line[0] == '"') {
                    line.erase(0, 1);
                    line.erase(line.length() - 1, 1);
                }

                if (std::find(Config::Includes.begin(), Config::Includes.end(), line) == Config::Includes.end()) {
                    Config::Includes.push_back(line);
                    int32_t result = ReadConfig(line.c_str());
                    if (result == -1) {
                        Printf("ReadConfig: file not found in \"%s\"!", line.c_str());
                    } else if (result > 0) {
                        Printf("ReadConfig: syntax error at \"%s\":%u!", line.c_str(), result);
                    }
                } else {
                    Printf("ReadConfig: recursive inclusion detected in \"%s\":%u.", filename, lnid);
                }
            }
            continue;
        }

        if (line[0] == '[') {
            line.erase(0, 1);
            size_t whs = line.find_first_of(']');
            if (whs == std::string::npos) {
                return lnid;
            }
            line.erase(whs);
            section = ToLower(Trim(line));

            if (section == "bannedips" ||
                section == "bannedplayers" ||
                section == "reporttowww") {
                Printf("ReadConfig: obsolete section (%s)", section.c_str());
            } else if (section != "settings" &&
                       section != "maps" &&
                       section != "network" &&
                       section != "mysql") {
                return lnid;
            }
        } else {
            std::string parameter, value;
            size_t whd = line.find_first_of('=');
            if (whd != std::string::npos) {
                parameter = line;
                parameter.erase(whd);
                value = line;
                value.erase(0, whd + 1);
            } else {
                value = "";
                parameter = line;
            }

            parameter = ToLower(Trim(parameter));
            value = Trim(value);
            if (value[0] == '"') {
                if (value[value.length() - 1] != '"') {
                    return lnid;
                }

                value.erase(0, 1);
                value.erase(value.length() - 1, 1);
            }

            if (section == "settings") {
                if (parameter == "chrbase") {
                    if (value[value.length() - 1] != '\\') {
                        value += '\\';
                    }
                    Config::ChrBase = value;
                    g_ServerConfig.chr_base = value.c_str(); // 0x6D15BC
                } else if (parameter == ToLower("QuestRollInterval")) {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    Config::QuestRollInterval = ReadIntegerParameter(value, 1, 0xFF);
                } else if (parameter == ToLower("MagePvpDmgFactor")) {
                    if (!CheckFloat(value)) {
                        return lnid;
                    }
                    Config::mage_pvp_dmg_factor = ReadFloatParameter(value, 0, 100);
                } else if (parameter == ToLower("MaxPvpDmg")) {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    // value, int32_t MinValue, int32_t MaxValue
                    Config::max_pvp_dmg = ReadIntegerParameter(value, 1, 500);
                } else if (parameter == ToLower("ShopPotionsFactor")) {
                    if (!CheckFloat(value)) {
                        return lnid;
                    }
                    Config::shop_potions_factor = ReadFloatParameter(value, 0, 100);
                } else if (parameter == ToLower("ServerRotateMaps")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::server_rotate_maps = StrToBool(value);
                } else if (parameter == ToLower("ShuffleMaps")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::shuffle_maps = StrToBool(value);
                } else if (parameter == "server_restart_on_map_change") {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::server_restart_on_map_change = StrToBool(value);
                } else if (parameter == "description") {
                    g_ServerConfig.server_name = value.c_str(); // 0x6D15C0
                } else if (parameter == "repopdelay") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 20) {
                        val = 20;
                    }
                    if (val > 500) {
                        val = 500;
                    }
                    g_ServerConfig.repop_delay = val; // 0x6D15A0
                } else if (parameter == "gamespeed") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 0 || val > 8) {
                        val = 4;
                    }
                    g_ServerConfig.game_speed = val; // 0x6D15A8
                } else if (parameter == "logfile") {
                    Config::LogFile = value;
                } else if (parameter == "logmode") {
                    Config::LogMode = ParseLogFlags(value);
                } else if (parameter == "serverid") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    uint32_t val = StrToUInt(value);
                    Config::ServerID = static_cast<ServerIDType>(val);
                    g_ServerConfig.server_id = val; // 0x6D15C4
                } else if (parameter == "sayrange") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    uint32_t val = StrToUInt(value);
                    if (val > 255 || !val) {
                        val = 255;
                    }
                    g_ServerConfig.chat_range = val; // 0x6D162C
                } else if (parameter == "shoutdelay") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 0) {
                        val = 0;
                    }
                    g_ServerConfig.shout_delay = val; // 0x6D1630
                } else if (parameter == "maxplayers") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    uint32_t val = StrToUInt(value);
                    if (val > 32) {
                        val = 32;
                    }
                    Config::MaxPlayers = val;
                    g_ServerConfig.max_players = val; // 0x6D163C
                } else if (parameter == "gametype" ||
                           parameter == "gamemode") {
                    uint32_t gmode = 0;
                    if (CheckInt(value)) {
                        uint32_t val = StrToUInt(value);
                        if (val > 3) {
                            return lnid;
                        }
                        gmode = val;
                    } else {
                        value = ToLower(value);
                        if (value == "cooperative") {
                            gmode = GAMEMODE_COOPERATIVE;
                        } else if (value == "deathmatch") {
                            gmode = GAMEMODE_DEATHMATCH;
                        } else if (value == "softcore") {
                            gmode = GAMEMODE_SOFTCORE;
                        } else if (value == "arena") {
                            gmode = GAMEMODE_ARENA;
                        } else if (value == "sandbox") {
                            gmode = GAMEMODE_SANDBOX;
                        } else {
                            return lnid;
                        }
                    }

                    Config::GameMode = gmode;
                    if (gmode == GAMEMODE_SANDBOX || gmode == GAMEMODE_SOFTCORE) {
                        g_ServerConfig.gameType = GAME_TYPE_COOPERATIVE; // 0x6D1648; для сервера это кооп
                    } else {
                        g_ServerConfig.gameType = static_cast<GameType>(gmode); // 0x6D1648
                    }
                } else if (parameter == "logintimeout") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 10) {
                        val = 10;
                    }
                    if (val > 300) {
                        val = 300;
                    }

                    g_ServerConfig.login_timeout = val; // 0x6D1640
                } else if (parameter == "reconnectdelay") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 1) {
                        val = 0;
                    }
                    g_ServerConfig.reconnect_delay = val; // 0x6D1644
                } else if (parameter == "shutdowndelay") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 1 || val > 60) {
                        val = 5;
                    }
                    g_ServerConfig.shutdown_delay = val; // 0x6D1658
                } else if (parameter == "fraglimit") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 1) {
                        val = 2147483647;
                    }
                    g_ServerConfig.frag_limit = val; // 0x6D164C
                } else if (parameter == "arenatimelimit" ||
                           parameter == "timelimit") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 1) {
                        val = 2147483647;
                    }
                    g_ServerConfig.arena_time_limit = val; // 0x6D1660
                } else if (parameter == "scaledmaps") {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    bool bv = StrToBool(value);
                    if (bv) {
                        g_ServerConfig.map_range_check = 1; // 0x6D1654
                    }
                } else if (parameter == "treasureprobability") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 0) {
                        val = 0;
                    }
                    if (val > 100) {
                        val = 100;
                    }
                    g_ServerConfig.treasure_probability = val; // 0x6D1664
                } else if (parameter == "serverflags") {
                    Config::ServerFlags = ParseFlags(value);
                    if (Config::ServerFlags & SVF_SOFTCORE) {
                        MAX_SKILL = 110;
                        MAX_EXP_ON_SKILL = 35742360;
                        Config::ServerCaps |= SVC_SOFTCORE;
                    }
                } else if (parameter == "maxpaletteallowed") {
                    int32_t val = StrToUInt(value);
                    if (val < 0) {
                        val = 0;
                    }
                    Config::MaxPaletteAllowed = val;
                } else if (parameter == "deathexpmult") {
                    float val = StrToFloat(value);
                    if (val < 0) {
                        val = 0;
                    }
                    if (val > 1) {
                        val = 1;
                    }
                    Config::DeathExpMult = val;
                } else if (parameter == "killedexpmult") {
                    float val = StrToFloat(value);
                    if (val < 0) {
                        val = 0;
                    }
                    if (val > 1) {
                        val = 1;
                    }
                    Config::KilledExpMult = val;
                } else if (parameter == "pkexpmult") {
                    float val = StrToFloat(value);
                    if (val < 0) {
                        val = 0;
                    }
                    if (val > 1) {
                        val = 1;
                    }
                    Config::PKExpMult = val;
                } else if (parameter == "respawnexpmult2") {
                    float val = StrToFloat(value);
                    if (val < 0) {
                        val = 0;
                    }
                    if (val > 1) {
                        val = 1;
                    }
                    Config::RespawnExpMult2 = val;
                } else if (parameter.rfind(ToLower("CharParams."), 0) == 0) {
                    // string.rfind is used as .startsWith()
                    const char delim = '.';

                    std::vector<std::string> out;
                    tokenize(parameter, delim, out);
                    int statValue = ReadIntegerParameter(value, 1, 200);
                    if (out.size() == 3) {
                        std::string clazz = out[1];
                        std::string stat = out[2];
                        MainCharacterParameters* params = NULL;
                        if (clazz == ToLower("WarriorMale")) {
                            params = &Config::WarriorMaleMaxParameters;
                        } else if (clazz == ToLower("WarriorFemale")) {
                            params = &Config::WarriorFemaleMaxParameters;
                        } else if (clazz == ToLower("MageMale")) {
                            params = &Config::MageMaleMaxParameters;
                        } else if (clazz == ToLower("MageFemale")) {
                            params = &Config::MageFemaleMaxParameters;
                        } else {
                            Printf("Error while reading '%s' part of '%s' config", clazz.c_str(), parameter.c_str());
                        }

                        if (stat == ToLower("MaxBody")) {
                            params->Body = statValue;
                        } else if (stat == ToLower("MaxReaction")) {
                            params->Reaction = statValue;
                        } else if (stat == ToLower("MaxMind")) {
                            params->Mind = statValue;
                        } else if (stat == ToLower("MaxSpirit")) {
                            params->Spirit = statValue;
                        } else if (stat == ToLower("MaxResistFire")) {
                            params->ResistFire = statValue;
                        } else if (stat == ToLower("MaxResistWater")) {
                            params->ResistWater = statValue;
                        } else if (stat == ToLower("MaxResistAir")) {
                            params->ResistAir = statValue;
                        } else if (stat == ToLower("MaxResistEarth")) {
                            params->ResistEarth = statValue;
                        } else if (stat == ToLower("MaxResistAstral")) {
                            params->ResistAstral = statValue;
                        } else {
                            Printf("Error while reading '%s' part of '%s' config", stat.c_str(), parameter.c_str());
                        }
                    }
                } else if (parameter == ToLower("MinQuestReward")) {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    Config::MinQuestReward = ReadIntegerParameter(value, 250, 16383000);
                } else if (parameter == ToLower("MaxQuestReward")) {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    Config::MaxQuestReward = ReadIntegerParameter(value, Config::MinQuestReward, 16383000);
                } else if (parameter == ToLower("InventoryDropProbability")) {
                    if (!CheckFloat(value)) {
                        return lnid;
                    }
                    Config::InventoryDropProbability = ReadFloatParameter(value, 0, 1);
                } else if (parameter == ToLower("WearDropProbability")) {
                    if (!CheckFloat(value)) {
                        return lnid;
                    }
                    Config::WearDropProbability = ReadFloatParameter(value, 0, 1);
                } else if (parameter == ToLower("AllowOnlyOneQuest_KillNMonsters")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::AllowOnlyOneQuest_KillNMonsters = StrToBool(value);
                } else if (parameter == ToLower("AllowOnlyOneQuest_KillTheMonster")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::AllowOnlyOneQuest_KillTheMonster = StrToBool(value);
                } else if (parameter == ToLower("AllowOnlyOneQuest_KillTheGroup")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::AllowOnlyOneQuest_KillTheGroup = StrToBool(value);
                } else if (parameter == ToLower("AllowQuestFilters")) {
                    if (!CheckBool(value)) {
                        return lnid;
                    }
                    Config::AllowQuestFilters = StrToBool(value);
                } else if (parameter == "cheats_additional_rights") {
                    try {
                        Config::CheatsAdditionalRights = std::stoi(value, nullptr, 16);
                    } catch (const std::exception& e) {
                        Log() << "ReadConfig: ignored invalid hex value for cheats_additional_rights: " << value << " (" << e.what() << ")";
                    }
                } else if (parameter == "servercaps") {
                    value = Trim(ToLower(value));
                    std::vector<std::string> svcap = Explode(value, "|");
                    uint32_t caps = 0;
                    for (std::vector<std::string>::iterator it = svcap.begin(); it != svcap.end(); ++it) {
                        std::string& cap = (*it);
                        if (!cap.length()) {
                            continue;
                        }
                        uint32_t flag = 0;
                        bool erase = false;
                        if (cap[0] == '-') {
                            erase = true;
                            cap.erase(0, 1);
                        }

                        if (cap == "all") {
                            flag = SVC_ALL;
                        } else if (cap == "detailed_info") {
                            flag = SVC_DETAILED_INFO;
                        } else if (cap == "fixed_maplist") {
                            flag = SVC_FIXED_MAPLIST;
                        } else if (cap == "save_database") {
                            flag = SVC_SAVE_DATABASE;
                        }

                        if (!flag) {
                            return lnid;
                        }
                        if (!erase) {
                            caps |= flag;
                        } else {
                            caps &= ~flag;
                        }
                    }

                    Config::ServerCaps &= SVC_SOFTCORE;
                    Config::ServerCaps |= caps & ~SVC_SOFTCORE;
                } else if (parameter == "controldirectory") {
                    Config::ControlDirectory = value;
                } else if (parameter == "flagscore" ||
                           parameter == "protocol" ||
                           parameter == "ipaddress" ||
                           parameter == "ipaddress2" ||
                           parameter == "save" ||
                           parameter == "hataddress") {
                    Printf("ReadConfig: obsolete parameter (settings/%s)", parameter.c_str());
                }
            } else if (section == "network") {
                if (parameter == "protocolversion") {
                    if (!CheckInt(value)) {
                        return lnid;
                    }
                    int32_t val = StrToUInt(value);
                    if (val < 8) {
                        val = 8;
                    }
                    if (val > 20) {
                        val = 20;
                    }
                    Config::ProtocolVersion = val;
                } else if (parameter == "ipaddress" ||
                           parameter == "ipaddress2" ||
                           parameter == "hataddress") {
                    std::vector<std::string> ipd = Explode(value, ":");
                    if (ipd.size() > 2) {
                        return lnid;
                    }
                    if (!CheckIP(ipd[0])) {
                        return lnid;
                    }
                    uint32_t port = 8001;
                    if (ipd.size() == 2) {
                        if (!CheckInt(ipd[1])) {
                            return lnid;
                        }
                        port = StrToUInt(ipd[1]);
                        if (port > 65535) {
                            return lnid;
                        }
                    }

                    if (parameter == "ipaddress") {
                        g_ServerConfig.ip_address = value.c_str(); // 0x6D15B0
                        Config::IPAddress = ipd[0];
                        Config::IPAddressP = port;
                    } else if (parameter == "ipaddress2") {
                        g_ServerConfig.ip_address2 = value.c_str(); // 0x6D15B4
                        Config::IPAddress2 = ipd[0];
                        Config::IPAddress2P = port;
                    } else {
                        g_ServerConfig.hat_address = value.c_str(); // 0x6D15B8
                        Config::HatAddress = ipd[0];
                        Config::HatAddressP = port;
                    }
                }
            } else if (section == "mysql") {
                if (parameter == "server") {
                    std::vector<std::string> ipd = Explode(value, ":");
                    if (ipd.size() > 2) {
                        return lnid;
                    }
                    if (!CheckIP(ipd[0])) {
                        return lnid;
                    }
                    uint32_t port = 3306;
                    Config::SqlAddress = ipd[0];
                    Config::SqlPort = port;
                } else if (parameter == "login") {
                    Config::SqlLogin = value;
                } else if (parameter == "password") {
                    Config::SqlPassword = value;
                } else if (parameter == "database") {
                    Config::SqlDatabase = value;
                } else {
                    return lnid;
                }
            } else if (section == "maps") {
                std::string m_filename = parameter;
                uint32_t m_time = 2147483647;
                if (value.length()) {
                    m_time = static_cast<uint32_t>(StrToFloat(value) * 60.0);
                }
                maps.emplace_back(std::move(m_filename), m_time);
            } else {
                Printf("ReadConfig: unknown parameter (%s/%s)", section.c_str(), parameter.c_str());
            }
        }
    }

    if (Config::GameMode == GAMEMODE_SOFTCORE) {
        Config::ServerFlags = Config::ServerFlags & ~(SVF_SANDBOX) | SVF_SOFTCORE;
    } else if (Config::GameMode == GAMEMODE_SANDBOX) {
        Config::ServerFlags = Config::ServerFlags & ~(SVF_SOFTCORE) | SVF_SANDBOX;
    }

    f_cfg.close();

    server_state.Load();

    if (Config::server_restart_on_map_change) {
        if (server_state.map_index < static_cast<int>(maps.size())) {
            Log() << "Setting map index to " << server_state.map_index << " from server state";
            g_ServerConfig.current_map_index = server_state.map_index; // 0x6D1634
        } else {
            Log() << "Saved map index " << server_state.map_index << " is out of range, ignoring";
        }
    }

    if (Config::shuffle_maps && g_ServerConfig.current_map_index == 0) { // 0x6D1634
        Log() << "Randomizing map order";
        auto device = std::random_device{};
        std::mt19937 generator(device());
        std::shuffle(maps.begin(), maps.end(), generator);
    } else if (!server_state.map_order.empty()) {
        Log() << "Restoring map order from server state with " << server_state.map_order.size() << " maps";

        std::unordered_map<std::string, std::size_t> order;
        order.reserve(server_state.map_order.size());
        for (std::size_t i = 0; i < server_state.map_order.size(); ++i) {
            order[server_state.map_order[i]] = i + 1;
        }

        std::sort(maps.begin(), maps.end(),
            [&](const auto& a, const auto& b) {
                auto order_a = order[a.first];
                auto order_b = order[b.first];
                if (!order_a) {
                    Log() << "Map " << a.first << " not found in saved map order";
                }
                if (!order_b) {
                    Log() << "Map " << b.first << " not found in saved map order";
                }
                return (order_a && order_b) ? order_a < order_b : order_a > 0;
            }
        );
    }

    for (auto& map_and_time : maps) {
        // 0x6D15F0 map_names / 0x6D1618 map_durations appends (AppendMaplist)
        g_ServerConfig.map_names.SetAtGrow(g_ServerConfig.map_names.GetSize(), map_and_time.first.c_str());
        g_ServerConfig.map_durations.SetAtGrow(g_ServerConfig.map_durations.GetSize(), map_and_time.second);
    }

    return 0;
}
