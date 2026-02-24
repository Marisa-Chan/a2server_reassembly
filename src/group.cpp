#include "group.h"
#include "unit.h"
#include <cstring>

// 5A4F6E - GroupSub constructor
GroupSub::GroupSub()
{
    // Clear the entire structure to zeroes.
    memset(this, 0, sizeof(GroupSub));
    
    // Set specific fields (most are already 0 from memset).
    field_0x45 = 1;
    
    list = new CList<uint16_t>();
}

GroupSub::~GroupSub()
{
    //5a500c
    if (list)
        delete list;
}

// 554D9E - Group constructor
Group::Group()
{
    field_0x40 = 0;
    owner = nullptr;
    has_quest_kill = 0;
    has_quest_intercept = 0;
    
    group_sub = new GroupSub();
}

Group::~Group()
{
    //554eea
    while (!unit_list.IsEmpty())
    {
        Unit* unit = unit_list.RemoveHead();
        if (unit)
            delete unit;
    }

    if (group_sub)
        delete group_sub;
}


void Group::RemoveUnit(Unit* unit)
{
    //5552e6
    POSITION pos = unit_list.Find(unit);
    if (pos != NULL)
        unit_list.RemoveAt(pos);

    unit->group = nullptr;
}

void Group::AddUnit(Unit* unit)
{
    //555176
    if (unit->group)
        unit->group->RemoveUnit(unit);

    unit_list.AddTail(unit);

    unit->group = this;
    owner = unit->pOwner;
}


Group group_instantiation_check;
