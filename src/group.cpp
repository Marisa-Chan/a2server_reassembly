#include "group.h"
#include "unit.h"
#include "game_app.h"
#include "server.h"
#include "util.h"
#include <cstring>

// 5A4F6E - GroupSub constructor
GroupSub::GroupSub() {
    // Clear the entire structure to zeroes.
    memset(this, 0, sizeof(GroupSub));
    
    // Set specific fields (most are already 0 from memset).
    this->field_0x45 = 1;
    
    this->list = new CList<uint16_t>();
}

GroupSub::~GroupSub() {
    //5a500c
    if (this->list) {
        delete this->list;
    }
}

void GroupSub::Serialize(CArchive& ar) {
    //5b0429
    if (ar.IsLoading()) {
        ar.Read(this, 0x50);
        this->list = new CList<uint16_t>();

        this->list->Serialize(ar);
    } else {
        ar.Write(this, 0x50);

        this->list->Serialize(ar);
    }
}


// 554D9E - Group constructor
Group::Group() {
    this->field_0x40 = 0;
    this->owner = nullptr;
    this->has_quest_kill = 0;
    this->has_quest_intercept = 0;
    
    this->group_sub = new GroupSub();
}

Group::~Group() {
    //554eea
    while (!this->unit_list.IsEmpty()) {
        Unit* unit = this->unit_list.RemoveHead();
        if (unit) {
            delete unit;
        }
    }

    if (this->group_sub) {
        delete this->group_sub;
    }
}


void Group::Serialize(CArchive& ar) {
    //55c94d
    this->some_list.Serialize(ar);
    this->group_sub->Serialize(ar);

    if (ar.IsLoading()) {
        this->unit_list.RemoveAll();
        uint32_t tmp;
        ar >> tmp;

        for (uint32_t i = 0; i < tmp; i++) {
            Unit* unit;
            ar >> unit;
            this->AddUnit(unit);
        }

        ar >> this->group_id;

        ar >> tmp;
        if (!g_Server->field23_0xdc.Lookup((void *)tmp, *(void**)&this->field_0x40)) {
            this->field_0x40 = 0;
        }

        ar >> tmp;
        if (!g_Server->field23_0xdc.Lookup((void*)tmp, *(void**)&this->owner)) {
            this->owner = nullptr;
        }
    } else {
        ar << (uint32_t)this->unit_list.GetCount();

        for (POSITION pos = this->unit_list.GetHeadPosition(); pos != NULL;) {
            Unit* obj = this->unit_list.GetNext(pos);
            ar << obj;
        }

        ar << this->group_id;

        ar << (uint32_t)this->field_0x40;
        ar << (uint32_t)this->owner;
    }
}


void Group::RemoveUnit(Unit* unit) {
    //5552e6
    POSITION pos = this->unit_list.Find(unit);
    if (pos != NULL) {
        this->unit_list.RemoveAt(pos);
    }

    unit->group = nullptr;
}

void Group::AddUnit(Unit* unit) {
    //555176
    if (unit->group) {
        unit->group->RemoveUnit(unit);
    }

    this->unit_list.AddTail(unit);

    unit->group = this;
    this->owner = unit->pOwner;
}


// 599F00
Unit* Group::sub_599F00() {
    if (this->unit_list.IsEmpty()) {
        return nullptr;
    }
    return this->unit_list.GetHead();
}


// 599F20
Unit* Group::sub_599F20(Unit* unit) {
    POSITION it = this->unit_list.Find(unit);
    if (it == nullptr) {
        return nullptr;
    }
    this->unit_list.GetNext(it);
    if (it == nullptr) {
        return nullptr;
    }
    return this->unit_list.GetAt(it);
}





GroupList::GroupList() {
    //5555f2
}

GroupList::~GroupList() {
    //555874
    while (this->groups.GetCount()) {
        Group* grp = this->groups.RemoveHead();
        delete grp;
    }
}

Group* GroupList::AllocGroup() {
    //55564e
    
    POSITION pos = this->groups.GetHeadPosition();
    while (pos != NULL) {
        POSITION curpos = pos; //save current position for RemoveAt
        Group* grp = this->groups.GetNext(pos); //get data and iterate to next

        if (grp && grp->unit_list.GetCount() == 0) {
            this->groups.RemoveAt(curpos);
            delete grp;
        }
    }

    //VERY STRANGE LOGIC
    return new Group();
}




// 558BE1
void GroupList::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        int32_t count = this->groups.GetCount();
        ar.Write(&count, sizeof(count));
        for (POSITION pos = this->groups.GetHeadPosition(); pos != nullptr;) {
            Group* grp = this->groups.GetNext(pos);
            grp->Serialize(ar);
        }
    } else {
        int32_t count;
        ar.Read(&count, sizeof(count));
        for (int32_t i = 0; i < count; ++i) {
            Group* grp = new Group();
            grp->Serialize(ar);
            this->groups.AddTail(grp);
        }
    }
}

Group group_instantiation_check;
