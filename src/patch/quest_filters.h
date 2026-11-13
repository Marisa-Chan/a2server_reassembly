#pragma once

// Tangar quest filters, ported from my-srvmgr/quests.cpp + inn.cpp
// (InitializeMobNames + the filter_out_existing_* family). Spliced into
// Inn::InnCreateQuests candidate counting, they prevent duplicate quests
// (AllowOnlyOneQuest_*) and let players filter quests by mob name
// (AllowQuestFilters + player_settings quest_filter).

#include <cstdint>
#include <string>
#include <vector>

class Player;
class Unit;

namespace tangar_quests {

// Count this kill-N candidate (true) or filter it out (false).
bool FilterKillNCandidate(Player* player, int32_t quest_monster_type);
// Count this kill-the-monster candidate (true) or filter it out (false).
bool FilterKillMobCandidate(Player* player, Unit* candidate);
// Count this kill-the-group candidate unit (true) or retry the next unit
// (false); the group itself is filtered via HasQuestForMonsterGroup.
bool FilterKillGroupCandidateUnit(Player* player, Unit* unit);
// True if the player already has a kill-the-group quest for this group.
bool HasQuestForMonsterGroup(Player* player, int32_t group_id);

// Count monsters in mob_names_by_server_id_normed matching a normalized filter.
int CountMatchingMobs(const std::string& filter);
// Formatted status lines ("Quest to kill %s: %d out of %d done") for #quest_state/#qs.
std::vector<std::string> QuestStateNMonsters(Player* player);

// Inn quest reward hooks (565B27, 5660C8, 5666C1).
uint32_t InnScrollsReward(Player* player, uint32_t reward);
bool SkipPotionReward();
bool SkipExperienceReward();

} // namespace tangar_quests
