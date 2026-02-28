#include "group.h"
#include "unit.h"
#include "game_app.h"
#include "server.h"
#include "util.h"
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

void GroupSub::Serialize(CArchive& ar)
{
    //5b0429
    if (ar.IsLoading())
    {
        ar.Read(this, 0x50);
        list = new CList<uint16_t>();

        list->Serialize(ar);
    }
    else
    {
        ar.Write(this, 0x50);

        list->Serialize(ar);
    }
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


void Group::Serialize(CArchive& ar)
{
    //55c94d
    some_list.Serialize(ar);
    group_sub->Serialize(ar);

    if (ar.IsLoading())
    {
        unit_list.RemoveAll();
        uint32_t tmp;
        ar >> tmp;

        for (uint32_t i = 0; i < tmp; i++)
        {
            Unit* unit;
            ar >> unit;
            AddUnit(unit);
        }

        ar >> group_id;

        ar >> tmp;
        if (!g_Server->field23_0xdc.Lookup((void *)tmp, *(void**)&field_0x40))
            field_0x40 = 0;

        ar >> tmp;
        if (!g_Server->field23_0xdc.Lookup((void*)tmp, *(void**)&owner))
            owner = nullptr;
    }
    else
    {
        ar << (uint32_t)unit_list.GetCount();

        for (POSITION pos = unit_list.GetHeadPosition(); pos != NULL;)
        {
            Unit* obj = unit_list.GetNext(pos);
            ar << obj;
        }

        ar << group_id;

        ar << (uint32_t)field_0x40;
        ar << (uint32_t)owner;
    }
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





GroupList::GroupList()
{
    //5555f2
}

GroupList::~GroupList()
{
    //555874
    while (groups.GetCount())
    {
        Group* grp = groups.RemoveHead();
        delete grp;
    }
}

Group* GroupList::AllocGroup()
{
    //55564e
    
    POSITION pos = groups.GetHeadPosition();
    while (pos != NULL)
    {
        POSITION curpos = pos; //save current position for RemoveAt
        Group* grp = groups.GetNext(pos); //get data and iterate to next

        if (grp && grp->unit_list.GetCount() == 0)
        {
            groups.RemoveAt(curpos);
            delete grp;
        }
    }

    //VERY STRANGE LOGIC
    return new Group();
}




// 558BE1
void GroupList::Serialize(CArchive& ar)
{
    if (ar.IsStoring()) {
        int32_t count = groups.GetCount();
        ar.Write(&count, sizeof(count));
        for (POSITION pos = groups.GetHeadPosition(); pos != nullptr;) {
            Group* grp = groups.GetNext(pos);
            grp->Serialize(ar);
        }
    } else {
        int32_t count;
        ar.Read(&count, sizeof(count));
        for (int32_t i = 0; i < count; ++i) {
            Group* grp = new Group();
            grp->Serialize(ar);
            groups.AddTail(grp);
        }
    }
}

Group group_instantiation_check;
