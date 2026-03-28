#include "world.h"

#include "constants.h"
#include "eye.h"
#include "game_app.h"
#include "group.h"
#include "logic.h"
#include "map_stuff.h"
#include "net.h"
#include "player.h"
#include "server.h"
#include "spell.h"
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

// Set up an autobuff cast action on caster targeting target (or the nearest
// eligible unit when target is null). If no target is found the caster idles.
// 5A85F4
void World::sub_5A85F4(Unit* caster, Unit* target, Spell* spell) {
    if (target == nullptr) {
        uint16_t yx = caster->position->GetYX();
        UnitList* list = (spell->is_defensive != 0) ? this->sub_5A384F(caster, yx) : this->sub_5A3808(caster, yx);
        if (list->unit_list.GetCount() != 0) {
            target = list->unit_list.GetHead();
        }
    }

    if (target == nullptr) {
        this->sub_5A6E2C(caster, 0);
    } else {
        caster->eye2->cast_action = 8;
        caster->eye2->unit5 = target;
        caster->eye2->spell = spell;
        caster->eye2->max_range = this->sub_5A6ADB(caster);
        target->eye2->autobuff_spell_id = spell->spell_id;
        target->eye2->autobuff_tick = g_Server->tick;
    }

    caster->eye2->max_range = spell->max_range;
}

// Run trigger action.
// 5B0E08
void World::DoScriptInstID(int32_t script_id) {
    TriggerAction action = (*this->trigger_actions)[script_id];

    if (g_Server->script_settings->script_tracing != 0) {
        char buf[100];
        sprintf(buf, "Script: Instant %d ( type: %d ).\n", script_id, (int)action.type);
        CString msg(buf);
        g_NetStru1_main.FUN_0051cd89(msg, nullptr);
    }

    if ((action.field_0x58 & 0xFF) != 0) {
        int count_groups = 0;
        int count_players = 0;
        int count_units = 0;
        for (int i = 0; i < 10; i++) {
            if (action.gap_0x3a[i] == 0) {
                continue;
            }
            uint8_t entry_type = action.field_0x30[i];
            void* ref = this->references[action.field_0x4e[i]];
            switch (entry_type) {
                case 2:
                    if (count_groups > 0) {
                        action.multi = ref;
                    } else {
                        action.group = (Group*)ref;
                    }
                    count_groups++;
                    break;
                case 3:
                    if (count_players > 0) {
                        action.multi = ref;
                    } else {
                        action.player = (Player*)ref;
                    }
                    count_players++;
                    break;
                case 4:
                    if (count_units > 0) {
                        action.multi = ref;
                    } else {
                        action.unit = (Unit*)ref;
                    }
                    count_units++;
                    break;
            }
        }
    }

    switch (action.type) {
        case 0x01: this->ScriptOP_0x01(&action); break;
        case 0x02: this->ScriptOP_0x02(&action); break;
        case 0x03: this->ScriptOP_0x03(&action); break;
        case 0x04: this->ScriptOP_0x04(&action); break;
        case 0x05: this->ScriptOP_0x05(&action); break;
        case 0x06: this->ScriptOP_0x06(&action); break;
        case 0x07: this->ScriptOP_0x07(&action); break;
        case 0x08: this->ScriptOP_0x08(&action); break;
        case 0x09: break;  // No-op.
        case 0x0a: this->ScriptOP_0x0a(&action); break;
        case 0x0b: this->ScriptOP_0x0b(&action); break;
        case 0x0c: this->ScriptOP_0x0c(&action); break;
        case 0x0d: this->ScriptOP_0x0d(&action); break;
        case 0x0e: this->ScriptOP_0x0e(&action); break;
        case 0x0f: this->ScriptOP_0x0f(&action); break;
        case 0x10: this->ScriptOP_0x10(&action); break;
        case 0x11: this->ScriptOP_0x11(&action); break;
        case 0x12: this->ScriptOP_0x12(&action); break;
        case 0x13: this->ScriptOP_0x13(&action); break;
        case 0x14: this->ScriptOP_0x14(&action); break;
        case 0x15: this->ScriptOP_0x15(&action); break;
        case 0x16: this->ScriptOP_0x16(&action); break;
        case 0x17: this->ScriptOP_0x17(&action); break;
        case 0x18: this->ScriptOP_0x18(&action); break;
        case 0x19: this->ScriptOP_0x19(&action); break;
        case 0x1a: this->ScriptOP_0x1a(&action); break;
        case 0x1b: this->ScriptOP_0x1b(&action); break;
        case 0x1c: this->ScriptOP_0x1c(&action); break;
        case 0x1d: this->ScriptOP_0x1d(&action); break;
        case 0x1e: this->ScriptOP_0x1e(&action); break;
        case 0x1f: this->ScriptOP_0x1f(&action); break;
        case 0x20: this->ScriptOP_0x20(&action); break;
        case 0x21: this->ScriptOP_0x21(&action); break;
        case 0x22: this->ScriptOP_0x22(&action); break;
        case 0x23: this->ScriptOP_0x23(&action); break;
        case 0x24: this->ScriptOP_0x24(&action); break;
        // no 0x25
        case 0x26: this->ScriptOP_0x26(&action); break;
        case 0x27: this->ScriptOP_0x27(&action); break;
        default: {
            if (g_Server->script_settings->turn_tracing != 0) {
                char buf[100];
                sprintf(buf, "Script: Bad instant %d.\n", (int)action.type);
                CString msg(buf);
                g_NetStru1_main.FUN_0051cd89(msg, nullptr);
            }
        }
    }
}
