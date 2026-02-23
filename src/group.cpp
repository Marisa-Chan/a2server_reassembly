#include "group.h"
#include <cstring>

// 5A4F6E - GroupSub constructor
GroupSub::GroupSub()
{
    // Clear the entire structure to zeroes.
    memset(this, 0, sizeof(GroupSub));
    
    // Set specific fields (most are already 0 from memset).
    field_0x45 = 1;
    
    // Create a new CList with initial capacity hint of 10.
    list = new CList<uint16_t>(10);
}

// 554D9E - Group constructor
Group::Group() : unit_list(10), some_list(10)
{
    field_0x40 = 0;
    owner = nullptr;
    has_quest_kill = 0;
    has_quest_intercept = 0;
    
    group_sub = new GroupSub();
}

Group group_instantiation_check;
