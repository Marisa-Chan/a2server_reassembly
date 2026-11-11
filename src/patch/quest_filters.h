#pragma once

// Tangar quest filters, ported from my-srvmgr/quests.cpp + inn.cpp
// (InitializeMobNames + the filter_out_existing_* family). Spliced into
// Inn::InnCreateQuests candidate counting, they prevent duplicate quests
// (AllowOnlyOneQuest_*) and let players filter quests by mob name
// (AllowQuestFilters + player_settings quest_filter).

#include <cstdint>
#include <string>

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

} // namespace tangar_quests
