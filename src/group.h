#ifndef GROUP_H
#define GROUP_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


struct GroupSub {
public:
    GroupSub();
    ~GroupSub();

    void Serialize(CArchive& ar);

public:
    uint16_t field_0x0;
    uint8_t gap_0x2[6];
    uint8_t field_0x8;
    uint8_t field_0x9;
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
    uint8_t field_0x39;
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
    uint32_t field_0x40;
    Player* owner;
    uint32_t has_quest_kill;
    uint32_t has_quest_intercept;

public:
    Group();  // Constructor - 554D9E
    ~Group();

    void Serialize(CArchive& ar);

    // ASM member functions
    void RemoveUnit(Unit* unit);  // Remove unit from group
    void AddUnit(Unit* unit);  // Add unit to group
    Unit* GetHeadUnit();  // 599F00. Avoid it, use `unit_list.GetHead()` instead.
    Unit* sub_599F20(Unit* unit);  // 599F20. Avoid it, use `unit_list.GetNext()` instead (this method is O(N) for no reason).
};

ASSERT_OFFSET(Group, owner, 0x44);
ASSERT_SIZE(Group, 0x50);


struct GroupList {
    CList<Group*> groups;

    GroupList();
    ~GroupList();
    Group* AllocGroup();
    void Serialize(CArchive& ar);
};


#endif
