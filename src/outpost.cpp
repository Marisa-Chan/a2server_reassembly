#include "outpost.h"

#include "group.h"
#include "player.h"
#include "unit.h"

IMPLEMENT_SERIAL(Outpost, Building, 1);

// 54326A
Outpost::Outpost() {
    this->sub_54335a();
}

// 5432DF
Outpost::Outpost(TokenPos* pos) : Building(pos) {
    this->sub_54335a();
}

void Outpost::sub_54335a() {
    this->tiles_mask = 0;
    this->group_id = 0;
    this->repop_delay = 120;
    this->repop_countdown = 0;
    this->script_id = -1;
    this->spread = 0;
    this->is_awaiting_repop = 0;
    this->has_quest_kill = 0;
    this->has_quest_intercept = 0;
}

// 5433CA
Outpost::~Outpost() {
    POSITION pos = this->units_copy.unit_list.GetHeadPosition();
    while (pos != nullptr) {
        Unit* u = this->units_copy.unit_list.GetNext(pos);
        delete u;
    }
}

// 5438ED
void Outpost::VMethod1() {
    Building::VMethod1();

    Group* found_group = nullptr;

    if (!this->is_awaiting_repop) {
        // Scan units_original: wait until all units have decayed to bones (>= 3).
        // If any unit has an AI-override owner with magic marker, postpone repop.
        POSITION pos = this->units_original.unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* u = this->units_original.unit_list.GetNext(pos);
            if (u->decay < 3) {
                return; // At least one unit has not yet decayed; wait longer.
            }

            Player* p = u->pOwner;
            if (p != nullptr && p->is_ai == 0 && p->hat_player_id == 0xF6D04773u && p->flags == 4) {
                this->repop_countdown = 32000; // 32000 tick delay
                return;
            }
        }

        this->is_awaiting_repop = 1;
    }

    // Decrement timer; only continue when countdown hits zero.
    if (this->repop_countdown-- >= 1) {
        return;
    }

    // Countdown reached zero: finalize corpse decay for remaining units.
    {
        POSITION pos = this->units_original.unit_list.GetHeadPosition();
        while (pos != nullptr) {
            Unit* u = this->units_original.unit_list.GetNext(pos);
            if (u->decay < 5) {
                this->hp = -601;
                u->sub_52E7FA();
            }
        }
    }

    // Find the group associated with this outpost in the owner's group list.
    {
        POSITION pos = this->pOwner->group_list->groups.GetHeadPosition();
        while (pos != nullptr) {
            Group* g = this->pOwner->group_list->groups.GetNext(pos);
            if (g->group_id == this->group_id) {
                found_group = g;
                if (g->unit_list.GetCount() > 0) {
                    return; // Group still has live units; don't respawn yet.
                }
                break;
            }
        }
    }

    // Create a fresh group if none was found.
    if (found_group == nullptr) {
        found_group = new Group();
        found_group->group_id = this->group_id;
        this->pOwner->group_list->groups.AddTail(found_group);
    }

    // Spawn units into the group and place them on the map.
    this->sub_543EC3(found_group);
}

int32_t Outpost::IsOutpost() {
    return 1;
}
