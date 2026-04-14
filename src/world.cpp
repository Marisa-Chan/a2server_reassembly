#include <cstdlib>

#include "world.h"

#include "building.h"
#include "buildings_list.h"
#include "constants.h"
#include "effect.h"
#include "eye.h"
#include "game_app.h"
#include "group.h"
#include "inventory.h"
#include "logic.h"
#include "map_stuff.h"
#include "net.h"
#include "spell_effect.h"
#include "outpost.h"
#include "player.h"
#include "players_list.h"
#include "server.h"
#include "spell.h"
#include "token.h"
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

// Check if two units are at war.
// 5B5816
bool World::sub_5B5816(Unit* unit1, Unit* unit2) {
    return (this->diplomacy[unit1->pOwner->player_id][unit2->pOwner->player_id] & 1) != 0;
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

// 5A9961
void World::sub_5A9961(Unit* unit, uint8_t x, uint8_t y) {
    this->sub_5A9AC4(unit);
    unit->state = 2;
    unit->eye2->command_to = ((uint16_t)y << 8) + x;
    unit->eye->field136_0x90 = 0;
    unit->eye2->cast_action = 0;
}

// 5A99C7
void World::sub_5A99C7(Unit* unit) {
    this->sub_5A9AC4(unit);
    unit->state = 26;
    unit->eye->field136_0x90 = 0;
    unit->eye2->cast_action = 0;
}

// 5AAA89
void World::sub_5AAA89(Unit* unit) {
    if (unit != unit->eye2->unit5 && this->field24_0xa50->sub_58FE6D(unit, unit->eye2->unit5, unit->eye2->max_range) == 0) {
        this->sub_5AA91B(unit, unit->eye2->unit5);
        unit->some_state = 1;
    } else {
        this->sub_5AA7CF(unit);
    }
}

// Autobuff handler: pick and cast healing/buff spells on self or allies.
// 5A7B44
void World::sub_5A7B44(Unit* unit) {
    if (unit->mp < 5) {
        unit->eye2->cast_action = 0;
        return;
    }

    if ((unit->pOwner->settings->autobuff_mask & 8) == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Self-heal.
    if (unit->hp != unit->hp_max) {
        Spell* spell = this->sub_5A79D6(unit, spell::heal, 0);
        if (spell != nullptr) {
            unit->eye2->field49_0x60 = 1;
            this->sub_5A85F4(unit, unit, spell);
            return;
        }
    }

    // Self-shield.
    if ((unit->enchantments & (1 << spell::shield)) == 0) {
        Spell* spell = this->sub_5A79D6(unit, spell::shield, 0);
        if (spell != nullptr) {
            unit->eye2->field49_0x60 = 1;
            this->sub_5A85F4(unit, unit, spell);
            return;
        }
    }

    // Build nearby friendly unit list for the group.
    this->sub_5ADD64(unit->group);
    this->sub_5A3AD6(unit, &this->field26_0xa64);

    // Ally-heal.
    Spell* spell = this->sub_5A79D6(unit, spell::heal, 0);
    if (spell != nullptr && this->field28_0xaa4.unit_list.GetCount() > 1) {
        double best_ratio = 2.0;
        Unit* best_unit = nullptr;

        POSITION pos = this->field28_0xaa4.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->field28_0xaa4.unit_list.GetNext(pos);
            if (!this->sub_5A7A1C(unit, other) || !this->sub_5A7AF7(unit, other, spell)) {
                continue;
            }

            // WAT: I guess the value should be cast before division? But ASM does it like this.
            double ratio = other->hp / other->hp_max;
            if (ratio < best_ratio) {
                best_ratio = ratio;
                best_unit = other;
            }
        }

        if (best_ratio < 1.0) {
            unit->eye2->field49_0x60 = 1;
            this->sub_5A85F4(unit, best_unit, spell);
            return;
        }
    }

    // Check autobuff_mask bit for medium-level buffs.
    if ((unit->pOwner->settings->autobuff_mask & 0x10) == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    if (unit->mp < 10) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Ally medium-level buffs.
    {
        POSITION pos = this->field28_0xaa4.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->field28_0xaa4.unit_list.GetNext(pos);
            if (this->sub_5A7A1C(unit, other) == 0) {
                continue;
            }

            int32_t autobuff_backoff = abs((int32_t)other->eye2->autobuff_tick - g_Server->tick);

            // Bless: requires fighter, no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::bless, 0);
            if (spell != nullptr && (other->unit_attrs & 4) == 0 && (other->enchantments & (1 << spell::bless)) == 0) {
                if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                    if (this->sub_5A7AF7(unit, other, spell) != 0) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }

            // Haste: no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::haste, 0);
            if (spell != nullptr && (other->enchantments & (1 << spell::haste)) == 0) {
                if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                    if (this->sub_5A7AF7(unit, other, spell) != 0) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }
        }
    }

    // Check autobuff_mask bit for high-level buffs.
    if ((unit->pOwner->settings->autobuff_mask & 0x20) == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    if (unit->mp < 40) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Ally high-level buffs.
    {
        POSITION pos = this->field28_0xaa4.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->field28_0xaa4.unit_list.GetNext(pos);
            if (this->sub_5A7A1C(unit, other) == 0) {
                continue;
            }

            int32_t autobuff_backoff = abs((int32_t)other->eye2->autobuff_tick - g_Server->tick);

            // Protection from air: no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::protection_from_air, 0);
            if (spell != nullptr && this->sub_5A7AF7(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_air)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from earth: no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::protection_from_earth, 0);
            if (spell != nullptr && this->sub_5A7AF7(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_earth)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from fire: no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::protection_from_fire, 0);
            if (spell != nullptr && this->sub_5A7AF7(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_fire)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from water: no matching enchantment.
            spell = this->sub_5A79D6(unit, spell::protection_from_water, 0);
            if (spell != nullptr && this->sub_5A7AF7(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_water)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->field49_0x60 = 1;
                        this->sub_5A85F4(unit, other, spell);
                        return;
                    }
                }
            }

            // Invisibility: no matching enchantment, not self, diplomacy check.
            spell = this->sub_5A79D6(unit, spell::invisibility, 0);
            if (spell != nullptr && this->sub_5A7AF7(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::invisibility)) == 0 && unit != other) {
                    if ((this->diplomacy[unit->pOwner->player_id][other->pOwner->player_id] & 3) != 0) {
                        if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                            unit->eye2->field49_0x60 = 1;
                            this->sub_5A85F4(unit, other, spell);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Self-invisibility --- last.
    spell = this->sub_5A79D6(unit, spell::invisibility, 0);
    if (spell != nullptr && (unit->enchantments & (1 << spell::invisibility)) == 0) {
        unit->eye2->field49_0x60 = 1;
        this->sub_5A85F4(unit, unit, spell);
        return;
    }

    unit->eye2->cast_action = 0;
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

// 5b37f3 — Increment mission stage
void World::ScriptOP_0x01(TriggerAction*) {
    this->mission_state++;
    this->trigger_variables[902]++;
}

// 5b3b11 — Send message
void World::ScriptOP_0x02(TriggerAction* action) {
    g_NetStru1_main.sub_51CD2A(nullptr, action->data[0], 0);
}

// 5b3b33 — Set value to variable
void World::ScriptOP_0x03(TriggerAction* action) {
    this->trigger_variables[action->data[0]] = action->data[1];
}

// 5b3b79 — Force "Mission Complete" state
void World::ScriptOP_0x04(TriggerAction*) {
    this->mission_complete++;
}

// 5b3b9b — Force "Mission Fail" state
void World::ScriptOP_0x05(TriggerAction* action) {
    this->mission_fail = action->data[0];
}

// 5b3bb7 — Group Command
void World::ScriptOP_0x06(TriggerAction* action) {
    Group* group = action->group;
    switch (action->data[0]) {
        case 1: // Guard (guard_range=data[1])
            this->sub_5AC8A2(group, action->data[1]);
            break;
        case 2: // Swarm (x=data[1], y=data[2])
            this->sub_5AC785(group, action->data[1], action->data[2]);
            break;
        case 3: // Stand Ground
            this->sub_5ACA54(group);
            break;
        case 4: // Move (x=data[1], y=data[2])
            this->sub_5AC289(group, action->data[1], action->data[2]);
            break;
        case 5: // Swarm 2 (x=data[1], y=data[2])
            this->sub_5AC507(group, action->data[1], action->data[2]);
            break;
        case 10: // Attack (victim=action->unit)
            this->sub_5AC80F(group, action->unit);
            break;
        case 11: // Defend (unit=action->unit, follow_range=data[1])
            this->sub_5ACB4D(group, action->unit, action->data[1]);
            break;
        case 14: // Patrol (x=data[1], y=data[2])
            this->sub_5ACBEF(group, action->data[1], action->data[2]);
            break;
        case 15: // Follow (unit=action->unit, follow_range=data[1])
            this->sub_5ACB9E(group, action->unit, action->data[1]);
            break;
        case 17: // Roam
            this->sub_5AC7C8(group);
            break;
        case 18: // Dwell (guard_range=data[1])
            this->sub_5AC983(group, action->data[1]);
            break;
    }
}

// 5b3b56 — Set formation
void World::ScriptOP_0x07(TriggerAction* action) {
    this->sub_5AF683(action->player, action->data[0]);
}

// 5b3ae1 — Increment variable
void World::ScriptOP_0x08(TriggerAction* action) {
    this->trigger_variables[action->data[0]]++;
}

// 5b3d75 — Change diplomacy
void World::ScriptOP_0x0a(TriggerAction* action) {
    uint32_t player1_id = action->data[0];
    uint32_t player2_id = action->data[1];
    uint32_t set_diplomacy = action->data[2];
    this->diplomacy[player1_id][player2_id] = (this->diplomacy[player1_id][player2_id] & ~0x03) + set_diplomacy;
    for (POSITION pos = this->players_list->GetHeadPosition(); pos != nullptr; ) {
        Player* p = this->players_list->GetNext(pos);
        if (p->player_id == player1_id || p->player_id == player2_id) {
            g_NetStru1_main.sub_51CB21(p);
        }
    }
}

// 5b3e56 — Give item from one unit (action->unit) to another (action->multi)
void World::ScriptOP_0x0b(TriggerAction* action) {
    Item* item = action->unit->inventory->sub_552C37(action->item_id);
    g_NetStru1_main.sub_51A6D5(action->unit, action->unit->pOwner, 0, 0);
    if (item) {
        Unit* recipient = (Unit*)action->multi;
        recipient->inventory->PutItemIntoBagAtDefault(item);
        g_NetStru1_main.sub_51A6D5(recipient, recipient->pOwner, 0, 0);
    }
}

// 5b3ed4 — Add item to unit's inventory
void World::ScriptOP_0x0c(TriggerAction* action) {
    Item* item = g_GameDataRes.sub_510EE6(action->item_id);
    if (item) {
        action->unit->inventory->PutItemIntoBagAtDefault(item);
    }
    g_NetStru1_main.sub_51A6D5(action->unit, action->unit->pOwner, 0, 0);
}

// 5b3f2f — Remove item from unit's inventory
void World::ScriptOP_0x0d(TriggerAction* action) {
    Item* item = action->unit->inventory->sub_552C37(action->item_id);
    if (item) {
        delete item;
    }
    g_NetStru1_main.sub_51A6D5(action->unit, action->unit->pOwner, 0, 0);
}

// 5b408d — kill unit
void World::ScriptOP_0x0e(TriggerAction* action) {
    g_NetStru1_main.sub_51A0EF(action->unit, action->unit->pOwner, 0xffb);
}

// 5b40ba — kill unit (same as 0x0e)
void World::ScriptOP_0x0f(TriggerAction* action) {
    g_NetStru1_main.sub_51A0EF(action->unit, action->unit->pOwner, 0xffb);
}

// 5b40e7 — Phase out unit
void World::ScriptOP_0x10(TriggerAction* action) {
    action->unit->sub_52C163();
}

// 5b40ff — Phase in unit
void World::ScriptOP_0x11(TriggerAction* action) {
    action->unit->sub_52C409();
}

// 5b4117 — Polymorph unit: phase out true form (action->unit), place polymorphed form (action->multi) at same position
void World::ScriptOP_0x12(TriggerAction* action) {
    action->unit->sub_52C163();
    Unit* polymorh_as = (Unit*)action->multi;
    polymorh_as->sub_52C36D(action->unit->position->GetX(), action->unit->position->GetY(), 3);
}

// 5b415a — Change unit's owner
void World::ScriptOP_0x13(TriggerAction* action) {
    g_NetStru1_main.FUN_004fb4ca(action->unit, action->player);
}

// 5b41f9 — Drop all: destroy all items in unit's inventory
void World::ScriptOP_0x14(TriggerAction* action) {
    action->unit->sub_52D8D3(action->unit->inventory, 0, 0);
    action->unit->inventory = new Inventory();
}

// 5b4287 — Make spell effect: from=(data[0], data[1]), to=(data[2], data[3]), spell=data[4], power=data[5]
void World::ScriptOP_0x15(TriggerAction* action) {
    uint8_t from_x = action->data[0], from_y = action->data[1];
    uint8_t to_x = action->data[2], to_y = action->data[3];
    uint32_t spell_id = action->data[4];

    uint32_t power = action->data[5] ? action->data[5] : 99;

    g_Server->srv_stru1->sub_4FBB24(from_x, from_y, to_x, to_y, spell_id, power);
}

// 5b4183 — Change group's owner
void World::ScriptOP_0x16(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        g_NetStru1_main.FUN_004fb4ca(unit, action->player);
    }
}

// 5b41d8 — Give money to player
void World::ScriptOP_0x17(TriggerAction* action) {
    action->player->sub_534AC1(action->data[0], 0);
}

// 5b4309 — Cast spell to unit: from=(data[0], data[1]), spell=data[2], power=data[3]
void World::ScriptOP_0x18(TriggerAction* action) {
    uint8_t from_x = action->data[0], from_y = action->data[1];
    uint32_t spell_id = action->data[2];
    uint32_t power = action->data[3];
    g_Server->srv_stru1->sub_4FBAE3(from_x, from_y, action->unit, spell_id, power);
}

// 5b4347 — Set trap: spell=data[0], power=data[1], trigger=(data[2],data[3]), source=(data[4],data[5]), target=(gap[0],gap[4])
void World::ScriptOP_0x19(TriggerAction* action) {
    uint8_t spell_id = action->data[0];
    uint8_t power = action->data[1];
    uint8_t trigger_x = action->data[2], trigger_y = action->data[3];
    uint8_t source_x = action->data[4], source_y = action->data[5];
    uint8_t target_x = action->data[6], target_y = action->data[7];

    uint8_t src[6];
    src[0] = spell_id;
    src[1] = power; // power/skill
    if (action->data[4] == 0) {        // no separate source/target: use trigger position for all
        src[2] = src[4] = trigger_x; // trigger_x = source_x = target_x
        src[3] = src[5] = trigger_y; // trigger_y = source_y = target_y
    } else {
        src[2] = source_x;
        src[3] = source_y;
        src[4] = target_x;
        src[5] = target_y;
    }
    uint16_t trigger_yx = (trigger_y << 8) | trigger_x;
    this->field24_0xa50->sub_596576(trigger_yx, src);
}

// 5b4428 — Set building's health
void World::ScriptOP_0x1a(TriggerAction* action) {
    action->building->hp = action->data[0];
}

// 5b4446 — Teleport unit
void World::ScriptOP_0x1b(TriggerAction* action) {
    this->field24_0xa50->sub_5954AC(action->unit, action->data[0], action->data[1]);
    if (action->unit->pOwner->is_ai) {
        this->sub_5A9383(action->unit);
    }
}

// 5b4494 — Give All: transfer all items from `action->unit` to `action->multi`
void World::ScriptOP_0x1c(TriggerAction* action) {
    Unit* target = (Unit*)action->multi;
    target->inventory->sub_552A42(action->unit->inventory);
    action->unit->inventory = new Inventory();
    g_NetStru1_main.sub_51A6D5(action->unit, action->unit->pOwner, 0, 0);
    g_NetStru1_main.sub_51A6D5(target, target->pOwner, 0, 0);
}

// 5b455f — Tile Spell Duration: set duration of spell (data[2]) at tile (data[0], data[1])
void World::ScriptOP_0x1d(TriggerAction* action) {
    uint16_t yx = (action->data[1] << 8) | action->data[0];
    AreaEffect** slots = this->field24_0xa50->sub_59536C(yx);
    if (!slots) {
        return;
    }
    for (int i = 0; i < 6; i++) {
        if (slots[i] && slots[i]->itemDataID == action->data[2]) {
            slots[i]->duration = action->data[3];
            break;
        }
    }
}

// 5b45e8 — Unit Spell Duration: update spell_value of effect matching spell ID (data[0]) on unit
void World::ScriptOP_0x1e(TriggerAction* action) {
    for (POSITION pos = action->unit->_effects.GetHeadPosition(); pos != nullptr; ) {
        Effect* effect = action->unit->_effects.GetNext(pos);
        if (effect->itemDataID == action->data[0]) {
            effect->spell_value = action->data[1];
        }
    }
}

// 5b4646 — Repop Delay
void World::ScriptOP_0x1f(TriggerAction* action) {
    BuildingsList* buildings = g_Server->srv_stru1->building_list;
    for (POSITION pos = buildings->GetHeadPosition(); pos != nullptr; ) {
        Building* b = buildings->GetNext(pos);
        if (b->IsOutpost()) {
            Outpost* outpost = static_cast<Outpost*>(b);
            if (outpost->group_id == action->data[0]) {
                outpost->repop_countdown = (int32_t)action->data[1];
            }
        }
    }
}

// 5b46bb — Phase Out Group
void World::ScriptOP_0x20(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        unit->sub_52C163();
    }
}

// 5b46ff — Phase In Group
void World::ScriptOP_0x21(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        unit->sub_52C409();
    }
}

// 5b4743 — Set Unit Parameter
void World::ScriptOP_0x22(TriggerAction* action) {
    switch (action->data[0]) { // parameter
        case 6: // hp
            action->unit->hp = action->data[1];
            break;
        case 0xf: // defense
            action->unit->protections.defense = action->data[1];
            break;
        case 0x10: // absorption
            action->unit->protections.absorption = action->data[1];
            break;
    }
    g_NetStru1_main.sub_51AC77(action->unit, action->unit->pOwner, 0);
}

// 5b382a — Set Scenario Variable
void World::ScriptOP_0x23(TriggerAction* action) {
    ScenarioSetVar(action->data[0], action->data[1]);
}

// 5b39df — Set Subobjective
void World::ScriptOP_0x24(TriggerAction* action) {
    int32_t subobjective_key = action->data[0] + 0x2f0;
    switch (action->data[1]) {
        case 1:
            if (!ScenarioGetVar(subobjective_key)) {
                ScenarioSetVar(subobjective_key, 1);
            }
            break;
        case 2:
            if (ScenarioGetVar(subobjective_key) != 4) {
                if (!(ScenarioGetVar(subobjective_key) & 2)) {
                    g_NetStru1_main.sub_51CD2A(nullptr, 0xFD, 0);
                }
                ScenarioSetVar(subobjective_key, 3);
            }
            break;
        case 4:
            g_NetStru1_main.sub_51CD2A(nullptr, 0xFE, 0);
            ScenarioSetVar(subobjective_key, 5);
            break;
        default:
            ScenarioSetVar(subobjective_key, action->data[1]);
            break;
    }
}

// 5b3fa7 — Remove item from everybody's inventory
void World::ScriptOP_0x26(TriggerAction* action) {
    UnitList* list = this->field24_0xa50->field69_0xa456c;
    if (!list) {
        return;
    }
    for (POSITION pos = list->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = list->unit_list.GetNext(pos);
        Item* item = unit->inventory->sub_552C37(action->item_id);
        if (item) {
            delete item;
            g_NetStru1_main.sub_51A6D5(unit, unit->pOwner, 0, 0);
        }
    }
}

// 5b4070 — Suspend group
void World::ScriptOP_0x27(TriggerAction* action) {
    action->group->group_sub->active = 0;
}
