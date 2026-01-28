#ifndef GROUP_H
#define GROUP_H

#include <cstdint>

#include "mfc_templ.h"


struct GroupSub {
    uint8_t gap_0x0[7];
    uint8_t field_0x8;
    uint8_t field_0x9;
    uint16_t field_0xa;
    uint8_t gap_0xc[9];
    uint8_t field_0x15;
    uint8_t gap_0x16[10];
    uint8_t field_0x20;
    uint8_t gap_0x21[3];
    uint32_t field_0x24;
    uint16_t field_0x28;
    uint8_t field_0x2a;
    uint8_t field_0x2b;
    uint8_t gap_0x2c;
    uint8_t field_0x2d;
    uint8_t gap_0x2e[2];
    uint32_t field_0x30;
    uint32_t field_0x34;
    uint8_t field_0x38;
    uint8_t gap_0x39[11];
    uint8_t field_0x44;
    uint8_t field_0x45;
    uint8_t gap_0x46[2];
    uint32_t active; // Modified by the "Suspend group" trigger action.
    CList<uint16_t>* list;
};
static_assert(offsetof(GroupSub, active) == 0x48, "GroupSub::active offset mismatch");
static_assert(sizeof(GroupSub) == 0x50, "GroupSub size mismatch");

class Player;
class Unit;

class Group {
public:
    CList<Unit*> unit_list;
    uint32_t group_id;
    CList<void*> some_list;
    GroupSub* group_sub;
    int field_0x40;
    Player* owner;
    uint32_t has_quest_kill;
    uint32_t has_quest_intercept;
};

static_assert(offsetof(Group, owner) == 0x44, "Group::owner offset mismatch");
static_assert(sizeof(Group) == 0x50, "Group size mismatch");


#endif
