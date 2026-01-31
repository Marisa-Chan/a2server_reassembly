#ifndef INN_H
#define INN_H

#include <cstdint>

#include "building.h"
#include "mfc_templ.h"
#include "unit_list.h"


class Inventory;
class Quest;
class QuestMap;


class Inn : public Building {
public:
    CMap<uint32_t, uint32_t, QuestMap*, QuestMap*> quest_map_per_player;
    CMap<uint32_t, uint32_t, Inventory*, Inventory*> rewards_per_player;
    UnitList unit_list; // Maybe current visitors of the inn?
    uint32_t has_kill_all_humans;
    uint32_t has_kill_all_monsters;
    uint32_t has_kill_all_undead;
    uint32_t has_raise_dead;
    uint16_t delivery_item_id;
    uint8_t gap_0xd6[2];
    uint32_t quest_roll_counter; // 120 in vanilla.
    CMap<uint32_t, uint32_t, QuestMap*, QuestMap*>::CAssoc* selected_quest;
    QuestMap* quest_map;
    Quest* active_quest;
    uint8_t gap_0xe8[4];
};
static_assert(offsetof(Inn, delivery_item_id) == 0xd4, "Inn::delivery_item_id offset mismatch");
static_assert(offsetof(Inn, quest_map) == 0xe0, "Inn::quest_map offset mismatch");
static_assert(sizeof(Inn) == 0xec, "Inn size mismatch");


#endif
