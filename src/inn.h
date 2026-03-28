#ifndef INN_H
#define INN_H

#include <cstdint>

#include "building.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "unit_list.h"


class Humanoid;
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

public:
    virtual void VMethod1() override;

    void sub_560DC2(Humanoid* humanoid, uint16_t id);
    void sub_560C67(Unit* inn);
    void InnCreateQuests(Player* player);
    bool sub_567B4D(Player* player); // Check if this player has a quest taken in this inn.
};
ASSERT_OFFSET(Inn, delivery_item_id, 0xd4);
ASSERT_OFFSET(Inn, quest_map, 0xe0);
ASSERT_SIZE(Inn, 0xec);


#endif
