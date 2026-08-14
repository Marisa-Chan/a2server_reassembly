#include <algorithm>
#include <cstdlib>
#include <cstring>

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

// 5B5855
Diplomacy::Diplomacy() {
    std::memset(this->diplomacy, 0, sizeof(this->diplomacy));
    this->flags[0] = 1;
    this->flags[1] = 1;
    this->flags[2] = 0;
    this->flags[3] = 0;
}

// 5A4B1A
World::World(MapStuff* map_stuff, PlayersList* players_list) {
    this->sub_5A457A();
    this->sub_5A4757();
    std::memset(this->field57_0xc554, 0, sizeof(this->field57_0xc554));
    this->field24_0xa50 = map_stuff;
    this->field24_0xa50->field41_0x58d80 = this;
    this->players_list = players_list;

    // Vanilla here does `this->field37_0xbbe8 = (map_stuff->GetWidth() < 120) ? 1 : 0;` but then overwrites it with 0 anyway.
    this->field37_0xbbe8 = 0;
}

// 5A4D38
World::~World() {
    delete this->trigger_checks;
    delete this->trigger_actions;
    delete this->triggers;

    this->sub_5A4590();
}

// 5A4757
void World::sub_5A4757() {
    this->duration4 = 0;
    this->duration4_low = 0;
    this->field20_0xa48 = 0;
    this->field21_0xa49 = 0;
    this->counter = 0;

    static constexpr uint8_t bdec_init[8] = {0, 0, 75, 65, 85, 65, 95, 65};
    std::memcpy(this->field_0xbdec, bdec_init, sizeof(bdec_init));

    this->field_0xbdfc = 0;
    this->field49_0xbe04 = 0;
    this->field50_0xbe08 = 0;
    this->mission_complete = 0;
    this->field52_0xbe10 = 0;
    this->mission_fail = 0;
    this->mission_state = 0;
    this->field65_0xc780 = 0;
    this->field48_0xbe00 = 0;
    this->field24_0xa50 = nullptr;

    std::memset(this->field56_0xc52c, 0, sizeof(this->field56_0xc52c));
    std::memset(this->trigger_variables, 0, sizeof(this->trigger_variables));
    std::memset(this->references, 0, sizeof(this->references));
    std::memset(this->trigger_results, 0, sizeof(this->trigger_results));
    std::memset(this->field32_0xa728, 0, sizeof(this->field32_0xa728));
    std::memset(this->field39_0xbc08, 0, sizeof(this->field39_0xbc08));

    this->name[0] = '\0';
    this->players_list = nullptr;
    this->diplomacy.world = this;

    this->trigger_variables[900] = 0;
    this->trigger_variables[901] = 1;
    this->trigger_variables[902] = 0;
    this->trigger_variables[999] = 0;

    this->trigger_checks = new CList<TriggerCheck>();
    this->trigger_actions = new CArray<TriggerAction>();
    this->triggers = new CList<Trigger>();
}

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
        if ((u->enchantments & (1 << spell::invisibility)) != 0 && this->diplomacy.diplomacy[unit->pOwner->player_id][u->pOwner->player_id] != 2) {
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
        if ((this->diplomacy.diplomacy[unit->pOwner->player_id][u->pOwner->player_id] & 1) != 0) {
            this->field29_0xac4.unit_list.AddTail(u);
        } else {
            this->field28_0xaa4.unit_list.AddTail(u);
        }
    }
}

// 5ADD64 — build a nearby-friendly-unit list from `group` into field26_0xa64.
void World::sub_5ADD64(Group* group) {
    this->field26_0xa64.unit_list.RemoveAll();

    if (group == nullptr) {
        return;
    }

    POSITION it = group->unit_list.GetHeadPosition();
    if (it == nullptr) {
        return;
    }

    Visibility* visibility = &this->field24_0xa50->visibility;
    visibility->sub_58E1C1();
    uint8_t* scratch = visibility->field_0x2a008;

    for (; it != nullptr;) {
        Unit* u = group->unit_list.GetNext(it);
        uint16_t yx = u->position->CompatGetYX();
        visibility->sub_58DE69(u, yx);

        if (u->pOwner->is_ai != 0 && u->eye2->position3 != 0) {
            scratch[u->eye2->position3] += 1;
            u->eye2->counter2 += 1;
            if (u->eye2->counter2 > 20) {
                u->eye2->position3 = 0;
            }
        }
    }

    UnitList* all = this->field24_0xa50->field69_0xa456c;
    CList<Unit*>* src = all != nullptr ? &all->unit_list : nullptr;
    if (src != nullptr) {
        for (POSITION it = src->GetHeadPosition(); it != nullptr;) {
            Unit* u = src->GetNext(it);
            uint16_t yx = u->position->CompatGetYX();
            if (scratch[yx] != 0) {
                this->field26_0xa64.unit_list.AddTail(u);
            }
        }
    }

    this->sub_5A3C5F();

    if (this->field26_0xa64.unit_list.GetCount() == 0 && this->field27_0xa84.unit_list.GetCount() != 0) {
        this->field26_0xa64.unit_list.RemoveAll();
        for (POSITION it = this->field27_0xa84.unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* u = this->field27_0xa84.unit_list.GetNext(it);
            this->field26_0xa64.unit_list.AddTail(u);
        }
        this->field27_0xa84.unit_list.RemoveAll();
    }
}

// 5AF805
void World::sub_5AF805(int32_t a, int32_t b, int32_t c, Player* player) {
    UnitList* unit_list = this->field24_0xa50->field69_0xa456c;
    if (unit_list == nullptr) {
        return;
    }

    for (POSITION unit_it = unit_list->unit_list.GetHeadPosition(); unit_it != nullptr;) {
        Unit* unit = unit_list->unit_list.GetNext(unit_it);
        if (player == nullptr) {
            if (unit->pOwner->is_ai != 0) {
                continue;
            }
        } else if (unit->pOwner != player) {
            continue;
        }

        uint8_t mode = unit->eye2->field39_0x4c;
        if (mode == 0) {
            unit->eye2->withdraw = (unit->hp_max * a) / 100;
        } else if (mode == 1) {
            unit->eye2->withdraw = (unit->hp_max * b) / 100;
        } else if (mode == 2) {
            unit->eye2->withdraw = (unit->hp_max * c) / 100;
        } else {
            unit->eye2->field39_0x4c = 0;
        }
    }
}

// 5AFA01
void World::sub_5AFA01(int32_t a, int32_t b, int32_t c, Player* player) {
    UnitList* unit_list = this->field24_0xa50->field69_0xa456c;
    if (unit_list == nullptr) {
        return;
    }

    for (POSITION unit_it = unit_list->unit_list.GetHeadPosition(); unit_it != nullptr;) {
        Unit* unit = unit_list->unit_list.GetNext(unit_it);
        if (player == nullptr) {
            if (unit->pOwner->is_ai != 0) {
                continue;
            }
        } else if (unit->pOwner != player) {
            continue;
        }

        uint8_t mode = unit->eye2->field39_0x4c;
        if (mode == 0) {
            unit->eye2->wimpy = (unit->hp_max * a) / 100;
        } else if (mode == 1) {
            unit->eye2->wimpy = (unit->hp_max * b) / 100;
        } else if (mode == 2) {
            unit->eye2->wimpy = (unit->hp_max * c) / 100;
        } else {
            unit->eye2->field39_0x4c = 0;
        }
    }
}

// Record a PvP hit: set attacker->target war, conditionally set reverse war,
// and increment target's hit counter when hit_flag is nonzero.
// 5B5643
void Diplomacy::sub_5B5643(Unit* attacker, Unit* target, int32_t hit_flag) {
    if (attacker->pOwner == nullptr || target->pOwner == nullptr) {
        return;
    }

    uint8_t attacker_pid = attacker->pOwner->player_id;
    uint8_t target_pid = target->pOwner->player_id;

    // Set attacker -> target war if not already at war.
    if ((this->diplomacy[attacker_pid][target_pid] & 3) == 0) {
        this->diplomacy[attacker_pid][target_pid] |= 1;
        if (!attacker->pOwner->is_ai) {
            g_NetStru1_main.sub_51CB21(attacker->pOwner);
            g_NetStru1_main.sub_51CB21(target->pOwner);
        }
    }

    // Set target -> attacker war (reverse) conditionally.
    if ((this->diplomacy[target_pid][attacker_pid] & 3) != 0) {
        return;
    }

    if (hit_flag == 0 || target->eye2->field77_0xac > 10 || target->hp < target->hp_max / 2 || target->some_state == 0x10) {
        this->diplomacy[target_pid][attacker_pid] |= 1;
        if (!target->pOwner->is_ai) {
            g_NetStru1_main.sub_51CB21(attacker->pOwner);
            g_NetStru1_main.sub_51CB21(target->pOwner);
        }
    }

    if (hit_flag != 0) {
        target->eye2->field77_0xac++;
    }
}

// Check if two units are at war.
// 5B5816
bool World::sub_5B5816(Unit* unit1, Unit* unit2) {
    return (this->diplomacy.diplomacy[unit1->pOwner->player_id][unit2->pOwner->player_id] & 1) != 0;
}

// 5ABB32
uint8_t World::sub_5ABB32(Unit* unit, PosYX yx) {
    Unit* unit_at_yx = this->field24_0xa50->sub_58CA1B(yx);

    if (unit->VMethod3() != 1) {
        return 0;
    }
    if (unit->pOwner->is_ai) {
        return 0;
    }
    if (unit->eye->field121_0x80 == yx) {
        return 0;
    }

    if (unit_at_yx == nullptr) {
        UnitList* nearby_list = this->field24_0xa50->sub_5897AA(yx, 1);
        CList<Unit*>* list = nearby_list != nullptr ? &nearby_list->unit_list : nullptr;
        if (list != nullptr) {
            POSITION unit_it = list->GetHeadPosition();
            while (unit_it != nullptr) {
                Unit* nearby_unit = list->GetNext(unit_it);
                if (nearby_unit->eye->field111_0x70 == yx.val && nearby_unit->position->sub_58bec3() == 0) {
                    return 2;
                }
            }
        }
        if (unit_at_yx == nullptr) {
            return 0;
        }
    }

    if (unit_at_yx->hp < 1) {
        return 0;
    }

    if (unit_at_yx->position->sub_58bec3() == 0) {
        if (unit->eye->position1 == yx.val) {
            return 0;
        }
        return 1;
    }

    if (unit_at_yx->eye2->cast_action == 1 || unit_at_yx->eye2->cast_action == 4) {
        if (unit->eye2->command_to == yx.val) {
            return 0;
        }
        return 2;
    }

    return 0;
}

// 5AB719
uint32_t World::sub_5AB719(Group* group) {
    int count = group->unit_list.GetCount();
    if (count == 0) {
        return 0;
    }

    uint32_t sum_xx = 0;
    uint32_t sum_yy = 0;

    POSITION unit_it = group->unit_list.GetHeadPosition();
    while (unit_it != nullptr) {
        Unit* unit = group->unit_list.GetNext(unit_it);
        sum_xx += unit->position->GetXx();
        sum_yy += unit->position->GetYy();
    }

    uint32_t avg_xx = sum_xx / count;
    uint32_t avg_yy = sum_yy / count;

    uint32_t position_yyxx = (avg_yy << 16) + avg_xx;
    PosYX position_yx = PosYX(avg_xx >> 8, avg_yy >> 8);

    group->group_sub->field_0x24 = position_yyxx;
    group->group_sub->field_0x28 = position_yx.val;

    uint8_t max_distance = 0;
    uint8_t max_scan_range = 0;
    uint8_t max_sum = 0;

    unit_it = group->unit_list.GetHeadPosition();
    while (unit_it != nullptr) {
        Unit* unit = group->unit_list.GetNext(unit_it);
        uint8_t distance = this->field24_0xa50->sub_593B29(unit->position->GetYX(), position_yx);
        if (max_distance < distance) {
            max_distance = distance;
        }

        uint8_t scan_range = unit->scan_range >> 8;
        if (max_scan_range < scan_range) {
            max_scan_range = scan_range;
        }

        uint8_t combined = distance + scan_range;
        if (max_sum < combined) {
            max_sum = combined;
        }
    }

    group->group_sub->field_0x2a = max_distance;
    group->group_sub->field_0x2b = max_scan_range;
    group->group_sub->field_0x2c = max_sum;

    return position_yyxx;
}

// Server turn tick: rebuild presence grid, run world updates, process each
// player's groups, and log turn timing statistics when tracing is enabled.
// 5ABD16
void World::sub_5ABD16(PlayersList* players) {
    this->field2_0x8.Begin();
    if (this->field37_0xbbe8 == 0) {
        this->field3_0x38.Begin();
        this->field24_0xa50->scan_presence_grid.sub_596131();
        this->field3_0x38.End();
        this->duration1 = (int32_t)this->field3_0x38.period64;
        this->total1 += (int32_t)this->field3_0x38.period64;
    }

    this->field4_0x68.Begin();
    this->sub_5B0724();
    this->field4_0x68.End();
    this->duration2 = (int32_t)this->field4_0x68.period64;
    this->total2 += (int32_t)this->field4_0x68.period64;

    this->field5_0x98.Begin();
    POSITION players_iter = players->list.GetHeadPosition();
    while (players_iter != nullptr) {
        Player* player = players->list.GetNext(players_iter);
        this->sub_5ABF50(player);
    }
    this->sub_5B6346();
    this->field5_0x98.End();
    this->duration3 = (int32_t)this->field5_0x98.period64;
    this->total3 += (int32_t)this->field5_0x98.period64;

    this->counter += 1;
    this->field2_0x8.End();
    this->duration4 = (int32_t)this->field2_0x8.period64;
    this->duration4_low = (int32_t)(this->field2_0x8.period64 >> 32);
    this->duration5 = (int32_t)this->field2_0x8.period64;
    this->total5 += (int32_t)this->field2_0x8.period64;

    if (g_Server->script_settings->turn_tracing != 0) {
        CString str;
        str.Format("Stats: last turn - %d, average: %d.\n", this->duration5, this->total5 / this->counter);
        g_NetStru1_main.FUN_0051cd89(str, nullptr);
    }
}

void World::sub_5B0724() {
    this->trigger_variables[999] += 1;
    this->sub_5B0762();
    this->sub_5B0A6A();
    this->action_list.RemoveAll();
}

// Evaluate map triggers and run their actions when all conditions pass.
// 5B0A6A
void World::sub_5B0A6A() {
    POSITION trigger_it = this->triggers->GetHeadPosition();
    while (trigger_it != nullptr) {
        Trigger& trigger = this->triggers->GetNext(trigger_it);
        if (trigger.once != 0 && this->trigger_results[trigger.trigger_id] != 0) {
            continue;
        }
        this->trigger_results[trigger.trigger_id] = 0;

        bool all_pass = true;
        POSITION check_it = trigger.checks->GetHeadPosition();
        while (check_it != nullptr && all_pass) {
            Check& check = trigger.checks->GetNext(check_it);
            int32_t left = this->trigger_variables[check.arg1];
            int32_t right = this->trigger_variables[check.arg2];
            switch (check.compare) {
                case 0: all_pass = all_pass && (left == right); break;
                case 1: all_pass = all_pass && (left != right); break;
                case 2: all_pass = all_pass && (left > right); break;
                case 3: all_pass = all_pass && (left < right); break;
                case 4: all_pass = all_pass && (left >= right); break;
                case 5: all_pass = all_pass && (left <= right); break;
                default: all_pass = false; break;
            }
        }

        if (all_pass) {
            this->trigger_results[trigger.trigger_id] = 1;
            if (trigger.once != 0 && g_Server->script_settings->script_tracing != 0) {
                CString msg;
                msg.Format("Script: Trigger %d ( %d ifs, %d instants ).\n", trigger.trigger_id, trigger.checks->GetCount(), trigger.actions->GetCount());
                g_NetStru1_main.FUN_0051cd89(msg, nullptr);
            }
            POSITION action_it = trigger.actions->GetHeadPosition();
            while (action_it != nullptr) {
                int32_t script_id = trigger.actions->GetNext(action_it);
                this->DoScriptInstID(script_id);
            }
        }
    }
}

// Set up an autobuff cast action on caster targeting target (or the nearest
// eligible unit when target is null). If no target is found the caster idles.
// 5A85F4
void World::sub_5A85F4(Unit* caster, Unit* target, Spell* spell) {
    if (target == nullptr) {
        PosYX yx = caster->position->GetYX();
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
        caster->eye2->max_range = this->UnitMaxRange(caster);
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

// Record a PvP hit: mark the attacker in UnitEye2, update diplomacy, push a combat
// action onto the world's action list, and store the retaliation cell.
// 5AA581
void World::sub_5AA581(Unit* target, Unit* attacker, int32_t param_4) {
    attacker->eye2->field44_0x54 = 1;
    attacker->eye2->position3 = target->position->CompatGetYX();
    attacker->eye2->counter2 = 0;

    this->diplomacy.sub_5B5643(target, attacker, param_4);

    this->scrape_action.action_kind = 1;
    this->scrape_action.unit = target;
    this->scrape_action.payload = attacker;
    this->action_list.AddTail(this->scrape_action);

    attacker->eye2->unit6 = target;
    attacker->eye2->tick = g_Server->tick;

    int32_t dxx = attacker->position->GetXx() - target->position->GetXx();
    int32_t dyy = attacker->position->GetYy() - target->position->GetYy();
    int32_t max_delta = (std::max)(std::abs(dxx), std::abs(dyy));

    if (max_delta != 0) {
        double scale = 10.0 / max_delta;
        uint8_t new_x = attacker->position->GetX() + dxx * scale;
        uint8_t new_y = attacker->position->GetY() + dyy * scale;
        attacker->eye2->field69_0x9c = PosYX{new_x, new_y}.val;
    } else {
        // This wasn't in vanilla, but otherwise we divide by zero.
        attacker->eye2->field69_0x9c = attacker->position->GetYX().val;
    }
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

// 5A9B6B — AI tick for a monster unit.
void World::sub_5A9B6B(Unit* unit) {
    if (this->field37_0xbbe8 == 0 && unit->group->group_sub->field_0x45 == 0 && unit->eye2->field4_0x9 == 0) {
        unit->some_state = 0x1B;
        return;
    }

    if (unit->some_state != 2 && unit->some_state != 0xF) {
        unit->some_state = 0;
    }

    if (unit->enchantments != 0 && (unit->enchantments & (1 << spell::stone_curse)) != 0 && unit->eye2->field4_0x9 == 0) {
        unit->eye2->field4_0x9 = 4;
    }

    if (unit->eye2->field4_0x9 == 0) {
        if (unit->eye2->field81_0xb0.val != 0 && unit->position->sub_58bec3()) {
            this->field24_0xa50->sub_5954AC(unit, unit->eye2->field81_0xb0.x, unit->eye2->field81_0xb0.y);
            unit->eye2->field81_0xb0 = PosYX{0};
        }

        switch (unit->eye2->cast_action) {
        case 1:
            this->sub_5AA375(unit, unit->eye2->command_to, 0);
            break;
        case 2:
            this->sub_5AA78C(unit);
            break;
        case 4:
            this->sub_5AA91B(unit, unit->eye2->unit3);
            break;
        case 5:
            this->sub_5AA97E(unit);
            break;
        case 6:
            this->sub_5AAEBC(unit);
            break;
        case 7:
            this->sub_5AA8ED(unit);
            break;
        case 8:
            this->sub_5AAA89(unit);
            break;
        case 9:
            this->sub_5AAB08(unit);
            break;
        case 10:
            this->sub_5AA426(unit, unit->eye->field1_0x1);
            break;
        case 11:
            this->sub_5AA485(unit);
            break;
        case 12:
            this->sub_5AA9F6(unit);
            break;
        case 15:
            this->sub_5AAB85(unit, unit);
            break;
        default:
            break;
        }
    } else {
        switch (unit->eye2->field4_0x9) {
        case 1:
            unit->some_state = 3;
            unit->eye2->counter++;
            if (unit->eye2->counter > 2 && unit->field_0x136 != 0) {
                unit->eye2->field4_0x9 = 0;
                unit->some_state = 0;
            }
            break;
        case 2:
            if (unit->eye2->field48_0x5c == 0) {
                unit->some_state = 0xE;
            } else {
                unit->some_state = 0xD;
            }
            unit->eye2->counter++;
            if (unit->eye2->counter > 2) {
                if (unit->eye2->counter == 3) {
                    if (unit->eye2->field48_0x5c == 0) {
                        this->scrape_action.action_kind = 3;
                        this->scrape_action.unit = unit;
                        this->scrape_action.payload = unit->some_spell;
                        this->scrape_action.x = unit->area_cast_x;
                        this->scrape_action.y = unit->area_cast_y;
                    } else {
                        this->scrape_action.action_kind = 2;
                        this->scrape_action.unit = unit;
                        this->scrape_action.payload = unit->some_spell;
                        this->scrape_action.target = unit->cast_target;
                    }
                    this->action_list.AddTail(this->scrape_action);
                }
                if (unit->field_0x136 != 0) {
                    unit->eye2->field4_0x9 = 0;
                    unit->some_state = 0;
                }
            }
            break;
        case 3:
            this->field24_0xa50->sub_590678(unit);
            unit->some_state = 1;
            if (unit->position->sub_58bec3()) {
                unit->eye2->field4_0x9 = 0;
                unit->some_state = 0;
            }
            break;
        case 4:
            unit->some_state = 0x1B;
            if ((unit->enchantments & (1 << spell::stone_curse)) == 0) {
                unit->eye2->field4_0x9 = 0;
            }
            break;
        case 0xff:
            unit->some_state = 0x1B;
            break;
        default:
            unit->eye2->field4_0x9 = 0;
            unit->some_state = 0x1B;
            break;
        }
    }

    if (unit->eye->field139_0x98 != 0) {
        unit->eye->field139_0x98 = 0;
        if (unit->state == 1) {
            this->FUN_005a9832(unit);
            this->sub_5A9A6A(unit);
        } else if (unit->state == 0xA) {
            unit->eye2->field2_0x4 = 1;
            unit->eye2->cast_action = 0;
            uint8_t x2 = unit->eye2->position2.x;
            uint8_t y2 = unit->eye2->position2.y;
            uint8_t x1 = unit->position->GetX();
            uint8_t y1 = unit->position->GetY();
            if (this->field24_0xa50->sub_58BFA3(x1, y1, x2, y2) < 3) {
                CList<uint16_t>* positions = unit->eye2->positions_list;
                if (positions != nullptr) {
                    POSITION pos = positions->Find(unit->eye2->position2);
                    if (pos != nullptr) {
                        unit->eye2->position2 = positions->GetNext(pos);
                        if (pos != nullptr) {
                            unit->eye2->position2 = positions->GetAt(pos);
                        }
                    } else {
                        unit->eye2->position2 = positions->GetHead();
                    }
                }
            }
        } else if (unit->state == 0x18) {
            unit->eye2->field4_0x9 = 0xFF;
        } else {
            unit->eye2->cast_action = 0;
            this->sub_5AAC17(unit);
        }
    }
}

// 5AAC17 — pick a melee/cast target for a monster unit.
void World::sub_5AAC17(Unit* unit) {
    this->field26_0xa64.unit_list.RemoveAll();

    uint32_t best_distance = 0xff;
    CList<Unit*>* all = nullptr;
    if (this->field24_0xa50->field69_0xa456c != nullptr) {
        all = &this->field24_0xa50->field69_0xa456c->unit_list;
    }

    if (all != nullptr) {
        POSITION unit_it = all->GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = all->GetNext(unit_it);
            uint8_t dist = other->position->Distance(unit->position);
            uint8_t max_range = this->UnitMaxRange(unit);
            if (dist <= max_range && dist <= best_distance && other != unit) {
                this->field26_0xa64.AddTail(other);
                best_distance = dist;
            }
        }
    }

    this->sub_5A3896(unit, &this->field26_0xa64, 0);
    this->sub_5A3C5F();

    if (this->field26_0xa64.unit_list.GetCount() == 0 && this->field27_0xa84.unit_list.GetCount() != 0) {
        CList<Unit*>* alt = &this->field27_0xa84.unit_list;
        POSITION unit_it = alt->GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = alt->GetNext(unit_it);
            this->field26_0xa64.AddTail(other);
        }
        alt->RemoveAll();
    }

    Unit* best_target = nullptr;
    uint32_t best_diff = 300;
    if (this->field26_0xa64.unit_list.GetCount() != 0) {
        POSITION unit_it = this->field26_0xa64.unit_list.GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = this->field26_0xa64.unit_list.GetNext(unit_it);
            uint8_t facing = this->field24_0xa50->sub_591424(unit, other);
            uint8_t diff = sub_595561(unit->eye->field0_0x0, facing);
            if (diff <= best_diff) {
                best_target = other;
                best_diff = diff;
            }
        }
    }

    if (best_target != nullptr) {
        unit->eye2->unit = best_target;
        unit->eye2->cast_action = 6;
        this->sub_5AAEBC(unit);
    } else {
        if (!unit->pOwner->is_ai) {
            unit->eye2->cast_action = 0;
        } else {
            unit->eye2->cast_action = 0xb;
            this->sub_5AA485(unit);
        }
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
                    if ((this->diplomacy.diplomacy[unit->pOwner->player_id][other->pOwner->player_id] & 3) != 0) {
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
            if (action.field_0x3a[i] == 0) {
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
    this->diplomacy.diplomacy[player1_id][player2_id] = (this->diplomacy.diplomacy[player1_id][player2_id] & ~0x03) + set_diplomacy;
    for (POSITION pos = this->players_list->list.GetHeadPosition(); pos != nullptr; ) {
        Player* p = this->players_list->list.GetNext(pos);
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


uint8_t World::UnitMaxRange(Unit* unit)
{ //5a6adb
    if (unit->eye2->spell_id != 0)
    {
        Spell* spell = unit->spell_book->sub_53DB79(unit->eye2->spell_id);
        if (spell && spell->mana_cost <= unit->mp)
            return spell->max_range;
    }
    return unit->max_range;
}

void World::FUN_005a9832(Unit* unit)
{ //5a9832
    if (unit->eye->field0_0x0 != unit->eye->field1_0x1)
        unit->eye->field1_0x1 = unit->eye->field0_0x0;

    if (unit->eye->field121_0x80 != PosYX())
    {
        if (unit->position->sub_58bec3())
        {
            if (unit->position->GetYX() != unit->eye->field121_0x80)
                field24_0xa50->FUN_005969c6(unit, PosYX(), 1);
        }
    }
    unit->eye2->max_range = UnitMaxRange(unit);
    unit->eye2->field49_0x60 = 0;
    unit->eye->field120_0x7c = 0;
    unit->some_state = 0;
}

void World::sub_5A9AC4(Unit* unit)
{ //5a9ac4
    FUN_005a9832(unit);
    unit->eye2->field43_0x50 = 0;
}

void World::FUN_005a93f4(Unit* unit)
{ //5a93f4
    sub_5A9AC4(unit);
    unit->state = 0xc;
    unit->eye2->position1 = unit->position->GetYX();
    unit->eye2->cast_action = 0;
}

void World::sub_5A9A6A(Unit* unit)
{ //5a9a6a
    sub_5A9AC4(unit);
    FUN_005a93f4(unit);
}

void World::FUN_005acd4c(Group* grp)
{ //5acd4c
    for (POSITION it = grp->unit_list.GetHeadPosition(); it != nullptr;)
    {
        Unit* unit = grp->unit_list.GetNext(it);
        sub_5A9AC4(unit);
        unit->eye2->field29_0x38 = 0;
    }
    grp->group_sub->field_0x20 = 0;
}

void World::sub_5ACA54(Group* grp)
{ //5aca54
    FUN_005acd4c(grp);
    for (POSITION it = grp->unit_list.GetHeadPosition(); it != nullptr;)
    {
        Unit* unit = grp->unit_list.GetNext(it);
        FUN_005a93f4(unit);
    }
    grp->group_sub->field_0x20 = 3;
}

void World::sub_5ACDF4(Group* group)
{ // 5acdf4
    FUN_005acd4c(group);
    sub_5ACA54(group);
}

// Group move command.
// 5AC289
void World::sub_5AC289(Group* group, uint8_t x, uint8_t y)
{
    this->FUN_005acd4c(group);

    bool use_relative = true;
    int8_t min_speed = 250;
    uint8_t origin_x = 0;
    uint8_t origin_y = 0;

    if (group->owner->settings->formation == 2) {
        uint32_t pos = this->sub_5AB719(group);
        origin_x = static_cast<uint8_t>((pos >> 8) & 0xFF);
        origin_y = static_cast<uint8_t>((pos >> 24) & 0xFF);

        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            uint8_t dist = this->field24_0xa50->sub_593B29(PosYX(unit->position->CompatGetYX()), PosYX(origin_x, origin_y));
            if (dist > this->gap_0xa724[0]) {
                use_relative = false;
            }
        }
    } else {
        use_relative = group->owner->settings->formation != 0;
        if (use_relative) {
            uint32_t pos = this->sub_5AB719(group);
            origin_x = static_cast<uint8_t>((pos >> 8) & 0xFF);
            origin_y = static_cast<uint8_t>((pos >> 24) & 0xFF);
        }
    }

    this->FUN_005acd4c(group);
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        if (!use_relative) {
            this->sub_5A8EFB(unit, x, y);
        } else {
            unit->eye2->x = unit->position->GetX() - origin_x;
            unit->eye2->y = unit->position->GetY() - origin_y;
            this->sub_5A8EFB(unit, x + unit->eye2->x, y + unit->eye2->y);
            if (unit->speed < min_speed) {
                min_speed = unit->speed;
            }
        }
    }

    if (use_relative) {
        group->group_sub->field_0x44 = min_speed;
    }
    group->group_sub->field_0x20 = 4;
    group->group_sub->field_0xa = PosYX(x, y).val;
}

// Group swarm command. Same as move, but mode is 5.
// 5AC507
void World::sub_5AC507(Group* group, uint8_t x, uint8_t y)
{ 
    this->sub_5AC289(group, x, y);
    group->group_sub->field_0x20 = 5;
}
