#ifndef INN_H
#define INN_H

#include <cstdint>

#include "building.h"
#include "assert_offset.h"
#include <afxtempl.h>
#include "unit_list.h"


class Humanoid;
struct Inventory;
class Quest;
class QuestMap;


class Inn : public Building {
public:
    enum ExitCode : int32_t {
        EXIT_LEAVE_NONE   = -1,
        EXIT_CANCEL_QUEST = static_cast<int32_t>(0xAAAAAAAAu),
    };

    enum : int32_t {
        REWARD_SLOT_LIMIT     = 0x100,
        REWARD_GOLD_UNIT      = 250,
        MIN_QUEST_REWARD      = 250,
        MAX_QUEST_REWARD      = 16383000,
        DEFAULT_ROLL_INTERVAL = 120,
    };

    DECLARE_SERIAL(Inn); // Runtime class definition at 637330.

public:
    Inn(); // 56046b (client: 539266)
    Inn(uint8_t type_id, TokenPos* pos); // 5605cc (client: 539332)
    
public: // VTable at 60fa30.
    virtual ~Inn(); // 56073e (client: 53940c)

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
    POSITION selected_quest;
    QuestMap* quest_map;
    Quest* active_quest;
    uint8_t gap_0xe8[4];

public:
    virtual void RegenTick() override; // 567d74 (client: 53df77)

    void RemoveUnit(Humanoid* humanoid, int32_t id); // 560dc2 (client: 5395df)
    void UpdateQuests(Unit* inn); // 560c67 (client: 539510)
    void InnCreateQuests(Player* player); // 562148 (client: 53a15e)
    void InnReward(Player* player); // 565307 (client: 53c371) - Generate reward items for completed quest
    bool HasQuest(Player* player); // 567b4d (client: 53de79) - Check if this player has a quest taken in this inn.
    Quest* FindActiveQuest(Player* player); // 567a25 (client: 53ddf5) - Find active quest for player at this inn.
    void DeleteCompletedQuests(Player* player); // 567c21 (client: 53dec1)
};
ASSERT_OFFSET(Inn, unit_list, 0xa4);
ASSERT_OFFSET(Inn, delivery_item_id, 0xd4);
ASSERT_OFFSET(Inn, quest_map, 0xe0);
ASSERT_SIZE(Inn, 0xec);


#endif
