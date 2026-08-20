#include "outpost.h"

#include "game_app.h"
#include "group.h"
#include "net.h"
#include "player.h"
#include "server.h"
#include "unit.h"
#include "world.h"

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

// 54361E
void Outpost::FillUnits(Group* group) {
    this->units_original.unit_list.RemoveAll();
    this->units_copy.unit_list.RemoveAll();

    POSITION it = group->unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = group->unit_list.GetNext(it);
        this->units_original.AddTail(unit);
        if (unit->VMethod8()) { // IsHumanoid
            Human* human = new Human();
            human->sub_530231((Humanoid*)unit);
            this->units_copy.AddTail(human);
        } else {
            Unit* unit_copy = new Unit();
            unit_copy->VMethod11(unit);
            this->units_copy.AddTail(unit_copy);
        }
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

// 543EC3
void Outpost::sub_543EC3(Group* group) {
    POSITION original_it = this->units_original.unit_list.GetHeadPosition();
    Unit* unit_original = nullptr;
    if (original_it != nullptr) {
        unit_original = this->units_original.unit_list.GetNext(original_it);
    }
    POSITION copy_it = this->units_copy.unit_list.GetHeadPosition();
    Unit* unit_copy = nullptr;
    if (copy_it != nullptr) {
        unit_copy = this->units_copy.unit_list.GetNext(copy_it);
    }

    while (unit_original != nullptr) {
        // Detach the original unit from the server's global unit list.
        CList<Unit*>& server_units = g_Server->srv_stru1->units_list->unit_list;
        POSITION found = server_units.Find(unit_original);
        if (found != nullptr) {
            server_units.RemoveAt(found);
        }

        if (unit_copy->VMethod8()) {
            ((Humanoid*)unit_original)->sub_530231((Humanoid*)unit_copy);
        } else {
            unit_original->VMethod11(unit_copy);
        }
        unit_original->field_x18 = 0;

        int32_t placed;
        if (this->spread != 0) {
            placed = unit_original->sub_52BF3D(this->position->GetX(), this->position->GetY(), this->spread);
        } else {
            placed = unit_original->sub_52BF3D(unit_original->position->GetX(), unit_original->position->GetY(), 0);
        }

        if (placed != 0) {
            this->is_awaiting_repop = 0;
            this->repop_countdown = this->repop_delay;
            dword_6CDB3C->AddTailId6xxx(unit_original);
            group->AddUnit(unit_original);
            this->pOwner->unit_list->AddTail(unit_original);
            g_NetStru1_main.sub_519221(unit_original, nullptr, 0x800000fb, 0xffb, 0, 0);
        }

        unit_original = original_it ? this->units_original.unit_list.GetNext(original_it) : nullptr;
        unit_copy = copy_it ? this->units_copy.unit_list.GetNext(copy_it) : nullptr;
    }

    group->has_quest_kill = this->has_quest_kill;
    group->has_quest_intercept = this->has_quest_intercept;
    g_World->sub_5AC983(group, 0);
    if (this->script_id >= 0) {
        g_World->DoScriptInstID(this->script_id);
    }
}
