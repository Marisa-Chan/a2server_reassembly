#include "group.h"
#include "unit.h"
#include "game_app.h"
#include "server.h"
#include "util.h"
#include <cstring>

// 5A4F6E - GroupOrderState constructor
// 5a4f6e (client: 5730de)
GroupOrderState::GroupOrderState() {
    // Clear the entire structure to zeroes.
    memset(this, 0, sizeof(GroupOrderState));
    
    // Set specific fields (most are already 0 from memset).
    this->presence_detected = 1;
    
    this->list = new CList<uint16_t>();
}

// 5a500c (client: 57317c)
GroupOrderState::~GroupOrderState() {
    if (this->list) {
        delete this->list;
    }
}

// 5b0429 (client: 57e599)
void GroupOrderState::Serialize(CArchive& ar) {
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
// 554d9e (client: 534b65) - Constructor
Group::Group() {
    this->saved_ptr = 0;
    this->owner = nullptr;
    this->has_quest_kill = 0;
    this->has_quest_intercept = 0;
    
    this->group_sub = new GroupOrderState();
}

// 554eea (client: 534c21)
Group::~Group() {
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


// 55c94d (client: 537654)
void Group::Serialize(CArchive& ar) {
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
        if (!g_Server->saved_ptr_map.Lookup((void *)tmp, *(void**)&this->saved_ptr)) {
            this->saved_ptr = 0;
        }

        ar >> tmp;
        if (!g_Server->saved_ptr_map.Lookup((void*)tmp, *(void**)&this->owner)) {
            this->owner = nullptr;
        }
    } else {
        ar << (uint32_t)this->unit_list.GetCount();

        for (POSITION pos = this->unit_list.GetHeadPosition(); pos != NULL;) {
            Unit* obj = this->unit_list.GetNext(pos);
            ar << obj;
        }

        ar << this->group_id;

        ar << (uint32_t)this->saved_ptr;
        ar << (uint32_t)this->owner;
    }
}


// 5552e6 (client: 534cf4) - Remove unit from group
void Group::RemoveUnit(Unit* unit) {
    POSITION pos = this->unit_list.Find(unit);
    if (pos != NULL) {
        this->unit_list.RemoveAt(pos);
    }

    unit->group = nullptr;
}

// 555176 (client: 534cae) - Add unit to group
void Group::AddUnit(Unit* unit) {
    if (unit->group) {
        unit->group->RemoveUnit(unit);
    }

    this->unit_list.AddTail(unit);

    unit->group = this;
    this->owner = unit->pOwner;
}


// 599f00 (client: 569210) - Avoid it, use `unit_list.GetHead()` instead.
Unit* Group::GetHeadUnit() {
    if (this->unit_list.IsEmpty()) {
        return nullptr;
    }
    return this->unit_list.GetHead();
}


// 599f20 (client: 547970) - Avoid it, use `unit_list.GetNext()` instead (this method is O(N) for no reason).
Unit* Group::GetNextUnit(Unit* unit) {
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





// 5555f2 (client: 534d7b)
GroupList::GroupList() {
}

// 555874 (client: 534e74)
GroupList::~GroupList() {
    while (this->groups.GetCount()) {
        Group* grp = this->groups.RemoveHead();
        delete grp;
    }
}

// 55564e (client: 534d91)
Group* GroupList::AllocGroup() {
    
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




// 558be1 (client: 536115)
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
