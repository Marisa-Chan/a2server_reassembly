#ifndef GROUP_H
#define GROUP_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


struct GroupSub {
public:
    GroupSub();  // client: 5730de
    ~GroupSub();  // client: 57317c

    void Serialize(CArchive& ar);  // client: 57e48b

public:
    uint16_t gap_0x0;
    uint8_t gap_0x2[6];
    uint8_t gap_0x8;
    uint8_t gap_0x9;
    uint16_t order_pos;
    uint8_t gap_0xc[9];
    uint8_t order_age;
    uint8_t gap_0x16[10];
    uint8_t order;
    uint8_t gap_0x21[3];
    uint32_t avg_pos_yyxx;
    uint16_t avg_pos;
    uint8_t max_unit_dist;
    uint8_t max_scan_range;
    uint8_t max_dist_plus_scan;
    uint8_t guard_range;
    uint8_t gap_0x2e[2];
    uint32_t was_empty;
    uint32_t phase_ticks;
    uint8_t guard_range_base;
    uint8_t gap_0x39;
    uint8_t gap_0x3a[10];
    uint8_t speed_override; // Group movement speed?
    uint8_t presence_detected;
    uint8_t gap_0x46[2];
    uint32_t active; // Modified by the "Suspend group" trigger action.
    CList<uint16_t>* list;
};
ASSERT_OFFSET(GroupSub, active, 0x48);
ASSERT_SIZE(GroupSub, 0x50);

class Player;
class Unit;

class Group {
public:
    CList<Unit*> unit_list;
    uint32_t group_id;
    CList<uint16_t> some_list;
    GroupSub* group_sub;
    uint32_t saved_ptr;
    Player* owner;
    uint32_t has_quest_kill;
    uint32_t has_quest_intercept;

public:
    Group();  // Constructor - 554D9E (client: 534b65)
    ~Group();

    void Serialize(CArchive& ar);  // client: 537654

    // ASM member functions
    void RemoveUnit(Unit* unit);  // Remove unit from group (client: 534cf4)
    void AddUnit(Unit* unit);  // Add unit to group (client: 534cae)
    Unit* GetHeadUnit();  // 599F00 (client: 569210). Avoid it, use `unit_list.GetHead()` instead.
    Unit* GetNextUnit(Unit* unit);  // 599F20 (client: 547970). Avoid it, use `unit_list.GetNext()` instead (this method is O(N) for no reason).
};

ASSERT_OFFSET(Group, owner, 0x44);
ASSERT_SIZE(Group, 0x50);


struct GroupList {
    CList<Group*> groups;

    GroupList();  // client: 534d7b
    ~GroupList();
    Group* AllocGroup();
    void Serialize(CArchive& ar);  // client: 536115
};


#endif
