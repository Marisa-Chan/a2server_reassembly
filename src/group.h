#ifndef GROUP_H
#define GROUP_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


struct GroupSub {
public:
    enum Order : uint8_t {
        ORDER_DEFAULT     = 0,
        ORDER_GUARD       = 1,
        ORDER_ATTACK_MOVE = 2,
        ORDER_AGGRESSIVE  = 3,
        ORDER_MOVE        = 4,
        ORDER_SWARM       = 5,
        ORDER_ROAM        = 0x11,
        ORDER_DEFEND      = 0x12,
        ORDER_STAND       = 0x13,
        ORDER_IDLE_ALL    = 0xFF,
    };

    GroupSub(); // 5a4f6e (client: 5730de)
    ~GroupSub(); // 5a500c (client: 57317c)

    void Serialize(CArchive& ar); // 5b0429 (client: 57e599)

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
    Group(); // 554d9e (client: 534b65) - Constructor
    ~Group(); // 554eea (client: 534c21)

    void Serialize(CArchive& ar); // 55c94d (client: 537654)

    // Member functions
    void RemoveUnit(Unit* unit); // 5552e6 (client: 534cf4) - Remove unit from group
    void AddUnit(Unit* unit); // 555176 (client: 534cae) - Add unit to group
    Unit* GetHeadUnit(); // 599f00 (client: 569210) - Avoid it, use `unit_list.GetHead()` instead.
    Unit* GetNextUnit(Unit* unit); // 599f20 (client: 547970) - Avoid it, use `unit_list.GetNext()` instead (this method is O(N) for no reason).
};

ASSERT_OFFSET(Group, owner, 0x44);
ASSERT_SIZE(Group, 0x50);


struct GroupList {
    CList<Group*> groups;

    GroupList(); // 5555f2 (client: 534d7b)
    ~GroupList(); // 555874 (client: 534e74)
    Group* AllocGroup(); // 55564e (client: 534d91)
    void Serialize(CArchive& ar); // 558be1 (client: 536115)
};


#endif
