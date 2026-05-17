#ifndef INN_H
#define INN_H

#include <cstdint>

#include "building.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "unit_list.h"


class Humanoid;
struct Inventory;
class Quest;
class QuestMap;


class Inn : public Building {
public:
    DECLARE_SERIAL(Inn); // Runtime class definition at 637330.

public:
    Inn(); // 56046B
    Inn(uint8_t type_id, TokenPos* pos); // 5605CC
    
public: // VTable at 60fa30.
    virtual ~Inn();

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

    void sub_560DC2(Humanoid* humanoid, int32_t id);
    void sub_560C67(Unit* inn);
    void InnCreateQuests(Player* player);
    void InnReward(Player* player); // sub_565307: Generate reward items for completed quest
    bool sub_567B4D(Player* player); // Check if this player has a quest taken in this inn.
    Quest* sub_567A25(Player* player); // Find active quest for player at this inn.
};
ASSERT_OFFSET(Inn, unit_list, 0xa4);
ASSERT_OFFSET(Inn, delivery_item_id, 0xd4);
ASSERT_OFFSET(Inn, quest_map, 0xe0);
ASSERT_SIZE(Inn, 0xec);


#endif
