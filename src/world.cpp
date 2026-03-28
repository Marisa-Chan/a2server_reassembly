#include "world.h"

#include "constants.h"
#include "eye.h"
#include "group.h"
#include "map_stuff.h"
#include "player.h"
#include "unit.h"

// Populate the attack-target list (field29_0xac4) and non-attack-target list
// (field28_0xaa4) for `unit` by scanning all units in `pList`.
// Invisible enemies that no group member can see are excluded.
// 5A3AD6
void World::sub_5A3AD6(Unit* unit, UnitList* pList) {
    this->field29_0xac4.unit_list.RemoveAll();
    this->field28_0xaa4.unit_list.RemoveAll();

    CList<Unit*>* src = pList ? &pList->unit_list : nullptr;
    for (auto* node = src ? src->m_pNodeHead : nullptr; node != nullptr; node = node->pNext) {
        Unit* u = node->data;

        // Skip dead/decaying units.
        if (u->some_state == 0x10) {
            continue;
        }

        // If u has has invisibility and is not allied with `unit`, verify that at least one
        // unit in `unit`'s group can see it before including it.
        if ((u->enchantments & (1 << spell::invisibility)) != 0 && this->diplomacy[unit->pOwner->player_id][u->pOwner->player_id] != 2) {
            bool can_see = false;
            for (auto* gn = unit->group->unit_list.m_pNodeHead; gn != nullptr; gn = gn->pNext) {
                Unit* gu = gn->data;
                int32_t range = this->field24_0xa50->sub_59190D(u, gu);
                if ((range & 0xFF) <= gu->eye2->seeInvisible) {
                    can_see = true;
                }
            }
            if (!can_see) {
                continue;
            }
        }

        // Classify: enemy -> attack list; others -> non-attack list.
        if ((this->diplomacy[unit->pOwner->player_id][u->pOwner->player_id] & 1) != 0) {
            this->field29_0xac4.unit_list.AddTail(u);
        } else {
            this->field28_0xaa4.unit_list.AddTail(u);
        }
    }
}
