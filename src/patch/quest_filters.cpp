// Ported from my-srvmgr/quests.cpp (filters + quest lookups) and inn.cpp
// (InitializeMobNames + NormalizeMobName).

#include "quest_filters.h"

#include <unordered_map>

#include "config_new.h"
#include "player_settings.h"

#include "../game_app.h"
#include "../player.h"
#include "../quest.h"
#include "../quest_map.h"
#include "../table.h"
#include "../unit.h"

namespace tangar_quests {

namespace {

std::string NormalizeMobName(const char* name) {
    std::string result = name;

    for (auto it = result.begin(); it != result.end(); ++it) {
        if (*it == '-') {
            *it = '_';
        } else {
            *it = tolower(*it);
        }
    }

    return result;
}

std::unordered_map<int, std::string> mob_names;                  // (face<<8|typeId) -> normalized name
std::unordered_map<int, std::string> mob_names_by_server_id_normed; // server_id -> normalized name

void InitializeMobNames() {
    if (!mob_names.empty()) {
        return;
    }

    std::unordered_map<int, std::string> mob_names_by_server_id;

    const CArray<MonsterInfo>& monsters = g_GameDataRes.monsters;
    for (int i = 0; i < monsters.GetSize(); i++) {
        const MonsterInfo& m = monsters[i];
        if (m.Values().GetSize() == 0) {
            continue;
        }
        const MonsterInfoData& d = m.Values()[0];

        // Add necros share the same type_id=99. Same with druids (100) and
        // catapults (101).
        std::string name = (const char*)m.name;
        if (d.type_id == 99) {
            name = "Necro";
        } else if (d.type_id == 100) {
            name = "Druid";
        } else if (d.type_id == 101) {
            name = "Catapult";
        }

        mob_names[d.face << 8 | d.type_id] = NormalizeMobName(name.c_str());
        mob_names_by_server_id[d.server_id] = (const char*)m.name;
    }

    // Human mobs don't have MonsterInfoData attached to them. Hardcode these
    // two for #reborn.
    mob_names_by_server_id[2130] = "2H_Knight4";
    mob_names_by_server_id[2132] = "2F_KnightLeader4";

    for (auto it = mob_names_by_server_id.begin(); it != mob_names_by_server_id.end(); ++it) {
        mob_names_by_server_id_normed[it->first] = NormalizeMobName(it->second.c_str());
    }

    // Most mobs of the first level don't have a suffix, which makes it
    // impossible to choose only them with filters. For example, the ghost is
    // `Ghost`, but a filter `Ghost` would also match `Ghost.2`. Append `.1`
    // to each mob name that doesn't have a level.
    for (auto it = mob_names_by_server_id_normed.begin(); it != mob_names_by_server_id_normed.end(); ++it) {
        const char mob_name_end = it->second.back();
        if (mob_name_end < '0' || mob_name_end > '9') {
            it->second += ".1";
        }
    }
    for (auto it = mob_names.begin(); it != mob_names.end(); ++it) {
        const char mob_name_end = it->second.back();
        if (mob_name_end < '0' || mob_name_end > '9') {
            it->second += ".1";
        }
    }
}

// True if the player has a live quest of this Kind with this obj.
bool PlayerHasQuest(int32_t kind, uint32_t player_id, uint32_t obj, bool obj_low16) {
    POSITION pos = g_QuestMap.quests_map.GetStartPosition();
    Quest* quest = nullptr;
    uint32_t key = 0;
    while (pos != nullptr) {
        g_QuestMap.quests_map.GetNextAssoc(pos, key, quest);
        if (quest == nullptr || quest->Kind() != kind) {
            continue;
        }
        if (quest->quest_data.player_id != player_id) {
            continue;
        }
        if (obj_low16) {
            if ((int16_t)(quest->quest_data.obj & 0xFFFF) == (int16_t)obj) {
                return true;
            }
        } else if (quest->quest_data.obj == obj) {
            return true;
        }
    }
    return false;
}

} // namespace

bool FilterKillNCandidate(Player* player, int32_t quest_monster_type) {
    // Is this unit type already in the player's quests?
    if (Config::AllowOnlyOneQuest_KillNMonsters && PlayerHasQuest(2, player->player_id, (uint32_t)quest_monster_type, false)) {
        return false;
    }

    if (Config::AllowQuestFilters) {
        InitializeMobNames();

        settings::PlayerSettings* player_settings = settings::Find((const char*)player->name);
        if (player_settings != nullptr) {
            const std::string& name_filter = player_settings->quest_filter;
            if (mob_names[quest_monster_type].find(name_filter) == std::string::npos) {
                return false;
            }
        }
    }

    return true;
}

bool FilterKillMobCandidate(Player* player, Unit* candidate) {
    // Is this unit already in the player's quests?
    if (Config::AllowOnlyOneQuest_KillTheMonster && PlayerHasQuest(1, player->player_id, candidate->server_id, true)) {
        return false;
    }

    if (Config::AllowQuestFilters) {
        InitializeMobNames();

        settings::PlayerSettings* player_settings = settings::Find((const char*)player->name);
        if (player_settings != nullptr) {
            const std::string& name_filter = player_settings->quest_filter;
            if (mob_names_by_server_id_normed[candidate->server_id].find(name_filter) == std::string::npos) {
                return false;
            }
        }
    }

    return true;
}

bool FilterKillGroupCandidateUnit(Player* player, Unit* unit) {
    if (Config::AllowQuestFilters) {
        InitializeMobNames();

        settings::PlayerSettings* player_settings = settings::Find((const char*)player->name);
        if (player_settings != nullptr) {
            const std::string& name_filter = player_settings->quest_filter;
            if (mob_names_by_server_id_normed[unit->server_id].find(name_filter) == std::string::npos) {
                return false; // Retry with the next unit in group.
            }
        }
    }
    return true;
}

bool HasQuestForMonsterGroup(Player* player, int32_t group_id) {
    return PlayerHasQuest(3, player->player_id, (uint32_t)group_id, false);
}

// 565B27
uint32_t InnScrollsReward(Player* player, uint32_t reward) {
    if (player != nullptr && player->main_unit != nullptr && (player->main_unit->unit_attrs & 4) == 0) {
        // Note: max reward is 16383000, so this doesn't overflow.
        return static_cast<uint32_t>(reward * 1.5);
    }

    // Mages don't get scrolls.
    return 0;
}

// 5660C8
bool SkipPotionReward() {
    return Config::ServerID >= NIGHTMARE;
}

// 5666C1
bool SkipExperienceReward() {
    return true;
}

} // namespace tangar_quests
