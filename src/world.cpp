#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "world.h"

#include "building.h"
#include "buildings_list.h"
#include "constants.h"
#include "effect.h"
#include "eye.h"
#include "file.h"
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
#include "resource.h"
#include "server.h"
#include "spell.h"
#include "token.h"
#include "unit.h"
#include "virtual_caster.h"
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
#include "patch/config_new.h"
#include "patch/flags.h"
#include "patch/player_settings.h"
#include "patch/solo.h"
#include "patch/tangar_log.h"
#endif

World* g_World; // 6a8b8c

// 5b5855 (client: 5839c5)
Diplomacy::Diplomacy() {
    std::memset(this->diplomacy, 0, sizeof(this->diplomacy));
    this->flags[0] = 1;
    this->flags[1] = 1;
    this->flags[2] = 0;
    this->flags[3] = 0;
}

// 5b545f (client: 5835cf) - Register player in the diplomacy matrix.
void Diplomacy::RegisterPlayer(Player* player) {
    int32_t player_id = player->player_id;
    
    this->diplomacy[player_id][0] = (player->is_ai == 2) ? 0 : static_cast<uint8_t>(player->is_ai);
    
    this->diplomacy[0][player_id] = 1;

    for (int32_t i = 1; i < 70; ++i) {
        if (this->diplomacy[0][i] != 0) {
            if (this->diplomacy[player_id][0] == 0) {
                if (this->diplomacy[i][0] == 0) {
                    this->diplomacy[player_id][i] = this->flags[3];
                    this->diplomacy[i][player_id] = this->flags[3];
                } else {
                    this->diplomacy[player_id][i] = this->flags[1];
                    this->diplomacy[i][player_id] = this->flags[0];
                }
            } else if (this->diplomacy[i][0] == 0) {
                this->diplomacy[player_id][i] = this->flags[0];
                this->diplomacy[i][player_id] = this->flags[1];
            } else {
                this->diplomacy[player_id][i] = this->flags[2];
                this->diplomacy[i][player_id] = this->flags[2];
            }
        }
    }
    
    this->diplomacy[player_id][player_id] = (DIPLO_VISION_SHARE | DIPLO_ALLIANCE);
}

// 5a4b1a (client: 572c8a)
World::World(MapStuff* map_stuff, PlayersList* players_list) {
    this->Init();
    this->ResetMatchState();
    std::memset(this->gap_0xc554, 0, sizeof(this->gap_0xc554));
    this->map_stuff = map_stuff;
    this->map_stuff->world = this;
    this->players_list = players_list;

    // Vanilla here does `this->skip_presence_grid = (map_stuff->GetWidth() < 120) ? 1 : 0;` but then overwrites it with 0 anyway.
    this->skip_presence_grid = 0;
}

// 5a4d38 (client: 572ea8)
World::~World() {
    delete this->trigger_checks;
    delete this->trigger_actions;
    delete this->triggers;

    this->Destroy();
}

// 5a4757 (client: 5728c7)
void World::ResetMatchState() {
    this->duration4 = 0;
    this->duration4_low = 0;
    this->saved_0xa48 = 0;
    this->saved_0xa49 = 0;
    this->counter = 0;

    static constexpr uint8_t bdec_init[8] = {0, 0, 75, 65, 85, 65, 95, 65};
    std::memcpy(this->waypoint_table, bdec_init, sizeof(bdec_init));

    this->patrol_counter = 0;
    this->gap_0xbe04 = 0;
    this->gap_0xbe08 = 0;
    this->mission_complete = 0;
    this->saved_0xbe10 = 0;
    this->mission_fail = 0;
    this->mission_state = 0;
    this->joined_count = 0;
    this->patrol_unit = nullptr;
    this->map_stuff = nullptr;

    std::memset(this->gap_0xc52c, 0, sizeof(this->gap_0xc52c));
    std::memset(this->trigger_variables, 0, sizeof(this->trigger_variables));
    std::memset(this->references, 0, sizeof(this->references));
    std::memset(this->trigger_results, 0, sizeof(this->trigger_results));
    std::memset(this->saved_0xa728, 0, sizeof(this->saved_0xa728));
    std::memset(this->gap_0xbc08, 0, sizeof(this->gap_0xbc08));

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

// 5a44c5 (client: 572635)
void World::ReinitState() {
    this->InitState();
}

// 5a44db (client: 57264b)
void World::BeginDestroy() {
    this->DestroyState();
}

// 5a3f3a (client: 5720aa)
void World::InitState() {
    this->ConstructMembers();
    this->duration4 = 0;
    this->duration4_low = 0;
    this->saved_0xa48 = 0;
    this->saved_0xa49 = 0;
    this->counter = 0;
}

// 5a3f88 (client: 5720f8)
void World::DestroyState() {
    this->DestroyMembers();
}

// (Re)initialize the world's runtime state and load AI settings from ai.reg.
// 5a3f9b (client: 57210b)
void World::ConstructMembers() {
    this->gap_0x0 = 0x8000;
    std::srand(static_cast<uint32_t>(std::time(nullptr)));

    // Reconstruct members in-place.
    new (&this->perf_tick) Perf();
    new (&this->perf_presence) Perf();
    new (&this->perf_triggers) Perf();
    new (&this->perf_ai) Perf();
    new (&this->gap_0xc8) FieldBlock5859c();
    new (&this->nearby_list) UnitList();
    new (&this->unconscious_list) UnitList();
    new (&this->non_targets) UnitList();
    new (&this->targets) UnitList();
    new (&this->diplomacy) Diplomacy();
    this->action_list.RemoveAll();
    new (&this->scrape_action) Action();

    this->duration4 = 0;
    this->duration4_low = 0;
    this->saved_0xa48 = 0;
    this->saved_0xa49 = 0;
    this->counter = 0;
    this->relative_move_max_dist = 2;
    this->skip_presence_grid = 0;

    // TODO: extract [0x8b8--0xa48] into a struct?
    std::memset(this->gap_0x8b8, 0, 400);

    static constexpr uint8_t a54_init[16] = {2, 1, 2, 4, 4, 2, 1, 0, 2, 1, 4, 0, 2, 1, 2, 4};
    std::memcpy(this->attack_decision_table, a54_init, sizeof(a54_init));

    File2 file2;
    file2.Open("World\\Data\\ai.reg", 0, nullptr);

    RegFile* reg = new RegFile();
    try {
        reg->ReadFromFile(&file2, -1);
    } catch (CException* e) {
        e->Delete();
    }

    this->MinimalGuardRange = reg->GetInt("Scanning", "MinimalGuardRange", 10);
    delete reg;
}

// 5a4284 (client: 5723f4)
void World::DestroyMembers() {
}

// Populate the attack-target list (targets) and non-attack-target list
// (non_targets) for `unit` by scanning all units in `pList`.
// Invisible enemies that no group member can see are excluded.
// 5a3ad6 (client: 571c46) - Populate attack/non-attack target lists for unit from pList.
void World::SplitTargetLists(Unit* unit, UnitList* pList) {
    this->targets.unit_list.RemoveAll();
    this->non_targets.unit_list.RemoveAll();

    CList<Unit*>* src = pList ? &pList->unit_list : nullptr;
    for (POSITION node = src ? src->GetHeadPosition() : nullptr; node != nullptr; ) {
        Unit* u = src->GetNext(node);

        // Skip dead/decaying units.
        if (u->some_state == Unit::EXEC_DEAD) {
            continue;
        }

        // If u has has invisibility and is not allied with `unit`, verify that at least one
        // unit in `unit`'s group can see it before including it.
        if ((u->enchantments & (1 << spell::invisibility)) != 0) {
#ifdef A2SERVER_PATCH
            if (u->pOwner == nullptr) {
                return;
            }
#endif
            if (this->diplomacy.diplomacy[unit->pOwner->player_id][u->pOwner->player_id] != DIPLO_ALLIANCE) {
                bool can_see = false;
                for (POSITION gn = unit->group->unit_list.GetHeadPosition(); gn != nullptr;) {
                    Unit* gu = unit->group->unit_list.GetNext(gn);
                    int32_t range = this->map_stuff->GetRangeTo(u, gu);
                    if ((range & 0xFF) <= gu->eye2->seeInvisible) {
                        can_see = true;
                    }
                }
                if (!can_see) {
                    continue;
                }
            }
        }

        // Classify: enemy -> attack list; others -> non-attack list.
        if (this->AreAtWar(unit, u)) {
            this->targets.unit_list.AddTail(u);
        } else {
            this->non_targets.unit_list.AddTail(u);
        }
    }
}

// Filter a UnitList by enemy/allied relationship and visibility.
// Removes units where diplomacy&1 == flag, and invisible units
// that no member of unit's group can see.
// 5a3896 (client: 571a06) - filter a UnitList by enemy/allied relationship.
void World::FilterByRelation(Unit* unit, UnitList* list, int32_t flag) {
    if (list == nullptr) {
        return;
    }

    POSITION it = list->unit_list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current = it;
        Unit* other = list->unit_list.GetNext(it);

        bool should_remove = false;
        if ((int)this->AreAtWar(unit, other) == flag) {
            should_remove = true;
        } else if ((other->enchantments & (1u << spell::invisibility)) != 0) {
            bool can_see = false;
            POSITION group_it = unit->group->unit_list.GetHeadPosition();
            while (group_it != nullptr) {
                Unit* group_unit = unit->group->unit_list.GetNext(group_it);
                int32_t range = this->map_stuff->GetRangeTo(other, group_unit);
                if ((range & 0xFF) <= group_unit->eye2->seeInvisible) {
                    can_see = true;
                    break;
                }
            }
            if (!can_see) {
                should_remove = true;
            }
        }

        if (should_remove) {
            list->unit_list.RemoveAt(current);
        }
    }
}

// Filter a UnitList to keep only allied units that are visible to `unit`'s group.
// Removes non-allies (diplomacy bit 2 not set) and invisible allies that no group
// member can see.
// 5a39ad (client: 571b1d) - filter UnitList for CastSelfHealOrBuff.
void World::FilterBuffTargets(Unit* unit, UnitList* list) {
    if (list == nullptr) {
        return;
    }

    POSITION it = list->unit_list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current = it;
        Unit* other = list->unit_list.GetNext(it);

        bool should_remove = false;
        if ((this->diplomacy.diplomacy[unit->pOwner->player_id][other->pOwner->player_id] & DIPLO_ALLIANCE) == 0) {
            should_remove = true;
        } else if ((other->enchantments & (1u << spell::invisibility)) != 0) {
            bool can_see = false;
            POSITION group_it = unit->group->unit_list.GetHeadPosition();
            while (group_it != nullptr) {
                Unit* group_unit = unit->group->unit_list.GetNext(group_it);
                int32_t range = this->map_stuff->GetRangeTo(other, group_unit);
                if (range <= group_unit->eye2->seeInvisible) {
                    can_see = true;
                    break;
                }
            }
            if (!can_see) {
                should_remove = true;
            }
        }

        if (should_remove) {
            list->unit_list.RemoveAt(current);
        }
    }
}

// Build a temporary UnitList of enemy units near `yx` that `caster` can see.
// 5a3808 (client: 571978) - Get nearby enemy UnitList for caster at yx.
UnitList* World::GetNearbyEnemies(Unit* caster, PosYX yx) {
    uint8_t range = caster->eye->scan_radius_base;
    UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, range);
    this->FilterByRelation(caster, list, 0);
    return list;
}

// Build a temporary UnitList of allied units near `yx` that `caster` can see.
// 5a384f (client: 5719bf) - Get nearby ally UnitList for caster at yx.
UnitList* World::GetNearbyAllies(Unit* caster, PosYX yx) {
    uint8_t range = caster->eye->scan_radius_base;
    UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, range);
    this->FilterByRelation(caster, list, 1);
    return list;
}

// compute an attack preference score for `unit` against `target`.
// Returns 0xFFFFFF when the target should be ignored.
// 5ad0b3 (client: 57b223)
int32_t World::GetAttackDecision(Unit* unit, Unit* target) {
    PosYX target_yx = target->position->CompatGetYX();
    PosYX unit_yx = unit->position->CompatGetYX();
    uint8_t distance = this->map_stuff->ChebyshevDistance(unit_yx, target_yx);

    uint8_t target_movement = target->MovementType();
    if (target_movement == 1 && this->UnitMaxRange(target) > 1) {
        target_movement = 0;
    }

    uint8_t lookup_value;
    if (this->UnitMaxRange(unit) <= 1) {
        uint8_t unit_movement = unit->MovementType();
        lookup_value = this->attack_decision_table[target_movement + unit_movement * 4];
    } else {
        lookup_value = this->attack_decision_table[target_movement];
        uint8_t unit_max_range = this->UnitMaxRange(unit);
        if (unit_max_range < distance) {
            distance = distance - unit_max_range + 1;
        } else {
            distance = 1;
        }
    }

    uint8_t facing = this->map_stuff->GetFacingTo(unit, target);
    uint8_t angle_diff = this->map_stuff->FacingDiff(unit->eye->facing, facing);
    int32_t score = distance * 256 + angle_diff;

    if (lookup_value == 0) {
        score = 0xFFFFFF;
    } else {
        if (lookup_value == 1) {
            if (unit->GetTokenSize() < 2 && unit->mind >= 15) {
                score = score + (score >> 1);
            }
        } else if (lookup_value == 4) {
            if (unit->GetTokenSize() < 2 && unit->mind >= 15) {
                score = score - (score >> 2);
            }
        }

#ifdef A2SERVER_PATCH
        if (target->IsUnit() && (target->enchantments & (1 << spell::stone_curse)) != 0) {
#else
        if ((target->enchantments & (1 << spell::stone_curse)) != 0) {
#endif
            score += 127;
            if (score < 1) {
                score = 1;
            }
        }
    }

    return score;
}

// 5add64 (client: 57bed4) - Build nearby friendly unit list from `group`. - build a nearby-friendly-unit list from `group` into nearby_list.
void World::BuildAllyList(Group* group) {
    this->nearby_list.unit_list.RemoveAll();

    if (group == nullptr) {
        return;
    }

    POSITION it = group->unit_list.GetHeadPosition();
    if (it == nullptr) {
        return;
    }

    Visibility* visibility = &this->map_stuff->visibility;
    visibility->ClearVisScratch();
    uint8_t* scratch = visibility->vis_scratch;

    for (; it != nullptr;) {
        Unit* u = group->unit_list.GetNext(it);
        uint16_t yx = u->position->CompatGetYX();
        visibility->RefreshUnitVisibility(u, yx);

        if (u->pOwner->is_ai != 0 && u->eye2->position3 != 0) {
            scratch[u->eye2->position3] += 1;
            u->eye2->counter2 += 1;
            if (u->eye2->counter2 > 20) {
                u->eye2->position3 = 0;
            }
        }
    }

    UnitList* all = this->map_stuff->all_units;
    CList<Unit*>* src = all != nullptr ? &all->unit_list : nullptr;
    if (src != nullptr) {
        for (POSITION it = src->GetHeadPosition(); it != nullptr;) {
            Unit* u = src->GetNext(it);
            uint16_t yx = u->position->CompatGetYX();
            if (scratch[yx] != 0) {
                this->nearby_list.unit_list.AddTail(u);
            }
        }
    }

    this->PruneUnconsciousTargets();

    if (this->nearby_list.unit_list.GetCount() == 0 && this->unconscious_list.unit_list.GetCount() != 0) {
        this->nearby_list.unit_list.RemoveAll();
        for (POSITION it = this->unconscious_list.unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* u = this->unconscious_list.unit_list.GetNext(it);
            this->nearby_list.unit_list.AddTail(u);
        }
        this->unconscious_list.unit_list.RemoveAll();
    }
}

// 5adb16 (client: 57bc86) - build a nearby-friendly-unit list from `group`, then filter by relationship to the group's first unit.
void World::ScanNearbyUnits(Group* group) {
    this->nearby_list.unit_list.RemoveAll();

    if (group == nullptr) {
        return;
    }

    POSITION it = group->unit_list.GetHeadPosition();
    if (it == nullptr) {
        return;
    }

    Visibility* visibility = &this->map_stuff->visibility;
    visibility->ClearVisScratch();
    uint8_t* scratch = visibility->vis_scratch;

    for (; it != nullptr;) {
        Unit* u = group->unit_list.GetNext(it);
        uint16_t yx = u->position->CompatGetYX();
        visibility->RefreshUnitVisibility(u, yx);

        if (u->pOwner->is_ai && u->eye2->position3 != 0) {
            scratch[u->eye2->position3] += 1;
            u->eye2->counter2 += 1;
            if (u->eye2->counter2 > 20) {
                u->eye2->position3 = 0;
            }
        }
    }

    UnitList* all = this->map_stuff->all_units;
    CList<Unit*>* src = all != nullptr ? &all->unit_list : nullptr;
    if (src != nullptr) {
        for (POSITION src_it = src->GetHeadPosition(); src_it != nullptr;) {
            Unit* u = src->GetNext(src_it);
            uint16_t yx = u->position->CompatGetYX();
            if (scratch[yx] != 0) {
                this->nearby_list.unit_list.AddTail(u);
            }
        }
    }

    Unit* head = group->unit_list.GetHead();
    this->FilterByRelation(head, &this->nearby_list, 0);
    this->PruneUnconsciousTargets();

    if (this->nearby_list.unit_list.GetCount() == 0 && this->unconscious_list.unit_list.GetCount() != 0) {
        this->nearby_list.unit_list.RemoveAll();
        for (POSITION other_it = this->unconscious_list.unit_list.GetHeadPosition(); other_it != nullptr;) {
            Unit* u = this->unconscious_list.unit_list.GetNext(other_it);
            this->nearby_list.unit_list.AddTail(u);
        }
        this->unconscious_list.unit_list.RemoveAll();
    }
}

// 5aef66 (client: 57d0d6)
void World::AiAggroBrain(Group* group) {
    this->ScanNearbyUnits(group);
    this->AssignBestTargets(group);
    POSITION it = group->unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = group->unit_list.GetNext(it);
        if (unit->eye2->unit4 == nullptr) {
            this->DoRetreat(unit);
        } else {
            this->DoAttackTarget(unit, unit->eye2->unit4);
        }
    }
}

// 5af805 (client: 57d975)
void World::SetWithdrawThresholds(int32_t a, int32_t b, int32_t c, Player* player) {
    UnitList* unit_list = this->map_stuff->all_units;
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

        uint8_t mode = unit->eye2->wimpy_level;
        if (mode == 0) {
            unit->eye2->withdraw = (unit->hp_max * a) / 100;
        } else if (mode == 1) {
            unit->eye2->withdraw = (unit->hp_max * b) / 100;
        } else if (mode == 2) {
            unit->eye2->withdraw = (unit->hp_max * c) / 100;
        } else {
            unit->eye2->wimpy_level = 0;
        }
    }
}

// 5afa01 (client: 57db71)
void World::SetWimpyThresholds(int32_t a, int32_t b, int32_t c, Player* player) {
    UnitList* unit_list = this->map_stuff->all_units;
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

        uint8_t mode = unit->eye2->wimpy_level;
        if (mode == 0) {
            unit->eye2->wimpy = (unit->hp_max * a) / 100;
        } else if (mode == 1) {
            unit->eye2->wimpy = (unit->hp_max * b) / 100;
        } else if (mode == 2) {
            unit->eye2->wimpy = (unit->hp_max * c) / 100;
        } else {
            unit->eye2->wimpy_level = 0;
        }
    }
}

// Record a PvP hit: set attacker->target war, conditionally set reverse war,
// and increment target's hit counter when hit_flag is nonzero.
// 5b5643 (client: 5837b3)
void Diplomacy::RecordHostility(Unit* attacker, Unit* target, int32_t hit_flag) {
    if (attacker->pOwner == nullptr || target->pOwner == nullptr) {
        return;
    }

    uint8_t attacker_pid = attacker->pOwner->player_id;
    uint8_t target_pid = target->pOwner->player_id;

    // Set attacker -> target war if not already at war.
    if ((this->diplomacy[attacker_pid][target_pid] & (DIPLO_WAR | DIPLO_ALLIANCE)) == 0) {
        this->diplomacy[attacker_pid][target_pid] |= DIPLO_WAR;
        if (!attacker->pOwner->is_ai) {
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
            // auto_war_1 hook (5B56EF)
            if (Config::LogMode & SVL_DIPLOMACY) {
                Printf("Player %s (%s) set auto-war to player %s", (const char*)attacker->pOwner->name, "attacker", (const char*)target->pOwner->name);
            }
#endif
            g_NetStru1_main.SendVisibilityState(attacker->pOwner);
            g_NetStru1_main.SendVisibilityState(target->pOwner);
        }
    }

    // Set target -> attacker war (reverse) conditionally.
    if ((this->diplomacy[target_pid][attacker_pid] & (DIPLO_WAR | DIPLO_ALLIANCE)) != 0) {
        return;
    }

    if (hit_flag == 0 || target->eye2->hits_taken > 10 || target->hp < target->hp_max / 2 || target->some_state == Unit::EXEC_DEAD) {
        this->diplomacy[target_pid][attacker_pid] |= DIPLO_WAR;
        if (!target->pOwner->is_ai) {
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
            // auto_war_2 hook (5B57C6)
            if (Config::LogMode & SVL_DIPLOMACY) {
                Printf("Player %s (%s) set auto-war to player %s", (const char*)target->pOwner->name, "victim", (const char*)attacker->pOwner->name);
            }
#endif
            g_NetStru1_main.SendVisibilityState(attacker->pOwner);
            g_NetStru1_main.SendVisibilityState(target->pOwner);
        }
    }

    if (hit_flag != 0) {
        target->eye2->hits_taken++;
    }
}

// Check if two units are at war.
// 5b5816 (client: 583986) - Check if unit1's owner is at war with unit2's owner.
bool World::AreAtWar(Unit* unit1, Unit* unit2) {
#ifdef A2SERVER_PATCH
    if (unit2->pOwner == nullptr || unit1->pOwner == nullptr) {
        return false;
    }
#endif
    return (this->diplomacy.diplomacy[unit1->pOwner->player_id][unit2->pOwner->player_id] & DIPLO_WAR) != 0;
}

// Packed distance/facing metric between two units:
// high byte = distance, low byte = rotation for unit1 to unit2.
// 5b5c2b (client: 583d9b) - packed distance/facing metric between two units
uint16_t World::PackDistFacing(Unit* unit1, Unit* unit2) {
    uint16_t result = (uint8_t)this->map_stuff->GetRangeTo(unit1, unit2);
    result = result << 8;
    uint8_t facing = this->map_stuff->GetFacingTo(unit1, unit2);
    result += this->map_stuff->FacingDiff(unit1->eye->facing, facing);
    return result;
}

// Collect up to `rays` units hit by a multi-ray spell into `units`.
// Fills the two scratch lists via ScanNearbyUnits, keeps the `rays` closest
// candidates (list-27 distances are biased by <<8 so list-26 units win ties),
// adds `target` first, then trims the list back down to `rays` entries.
// 5b5ca8 (client: 583e18) - collect units hit by a multi-ray spell
void World::CollectRayTargets(Unit* caster, Unit* target, CList<Unit*>* units, uint32_t rays, uint32_t max_range) {
    units->RemoveAll();
    this->RecordUnitHit(caster, target, 0);
    this->ScanNearbyUnits(caster->group);

    if (this->nearby_list.unit_list.GetCount() == 0) {
        units->AddTail(target);
        return;
    }

    uint16_t best_dist[10];
    int32_t best_idx[10];
    for (int32_t i = 0; i < 10; i++) {
        best_idx[i] = 0xFFFA;
        best_dist[i] = 0xFFFA;
    }

    int32_t max_shifted = (max_range & 0xFF) << 8;
    uint32_t* dist_arr = &this->dist_scratch[100]; // Scratch distance array at this+0xC74.
    Unit* near_units[100];
    int32_t count = 0;

    for (POSITION it = this->nearby_list.unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = this->nearby_list.unit_list.GetNext(it);
        uint16_t dist = this->PackDistFacing(caster, unit);
        if (dist <= max_shifted) {
            dist_arr[count] = dist;
            near_units[count] = unit;
            count++;
        }
    }

    for (POSITION it = this->unconscious_list.unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = this->unconscious_list.unit_list.GetNext(it);
        uint16_t dist = this->PackDistFacing(caster, unit);
        if (dist <= max_shifted) {
            dist_arr[count] = dist << 8;
            near_units[count] = unit;
            count++;
        }
    }

    // For each ray, pick the closest not-yet-picked candidate and mark it used.
    int32_t ray_count = rays & 0xFF;
    for (int32_t i = 0; i < ray_count; i++) {
        for (int32_t j = 0; j < count; j++) {
            if (dist_arr[j] < best_dist[i]) {
                best_dist[i] = (uint16_t)dist_arr[j]; // Low word only, matches the original.
                best_idx[i] = j;
            }
        }
        if (best_idx[i] < 60000) {
            dist_arr[best_idx[i]] = 0xFFDC;
        }
    }

    units->AddTail(target);
    for (int32_t i = 0; i < ray_count; i++) {
        if (best_idx[i] < 65000 && best_dist[i] < 65000 && near_units[best_idx[i]] != target) {
            units->AddTail(near_units[best_idx[i]]);
        }
    }

    while (units->GetCount() > ray_count) {
        units->RemoveTail();
    }
}

// 5abb32 (client: 579ca2)
uint8_t World::CanClaimMoveCell(Unit* unit, PosYX yx) {
    Unit* unit_at_yx = this->map_stuff->GetUnitAt(yx);

    if (unit->GetTokenSize() != 1) {
        return 0;
    }
    if (unit->pOwner->is_ai) {
        return 0;
    }
    if (unit->eye->guard_cell == yx) {
        return 0;
    }

    if (unit_at_yx == nullptr) {
        UnitList* nearby_list = this->map_stuff->BuildNearbyUnitList(yx, 1);
        CList<Unit*>* list = nearby_list != nullptr ? &nearby_list->unit_list : nullptr;
        if (list != nullptr) {
            POSITION unit_it = list->GetHeadPosition();
            while (unit_it != nullptr) {
                Unit* nearby_unit = list->GetNext(unit_it);
                if (nearby_unit->eye->reserved_cell == yx.val && nearby_unit->position->IsAtCellCenter() == 0) {
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

    if (unit_at_yx->position->IsAtCellCenter() == 0) {
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

// 5ab62e (client: 57979e)
uint32_t World::ComputeListAvgPos(UnitList* list) {
    int count = list->unit_list.GetCount();
    if (count == 0) {
        return 0;
    }

    uint32_t sum_xx = 0;
    uint32_t sum_yy = 0;

    POSITION it = list->unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = list->unit_list.GetNext(it);
        sum_xx += unit->position->GetXx();
        sum_yy += unit->position->GetYy();
    }

    uint32_t avg_xx = sum_xx / count;
    uint32_t avg_yy = sum_yy / count;

    return (avg_yy << 16) + avg_xx;
}

// 5aec20 (client: 57cd90)
void World::AiAttackBrain(Group* group, uint16_t target_yx) {
    this->ScanNearbyUnits(group);
    this->AssignNearbyTargets(group);

    POSITION it = group->unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* unit = group->unit_list.GetNext(it);
        if (unit->eye2->unit4 != nullptr) {
            this->DoAttackTarget(unit, unit->eye2->unit4);
        } else {
            if (unit->position->CompatGetYX() == target_yx && unit->position->IsAtCellCenter()) {
                this->SetIdleOrRetreat(unit, 1);
            } else {
                this->DoMoveStep(unit, target_yx);
            }
        }
    }
}

// 5aece7 (client: 57ce57)
void World::AiSwarmBrain(Group* group, uint16_t target_yx) {
    this->ScanNearbyUnits(group);
    if (this->nearby_list.unit_list.GetCount() == 0) {
        this->AiMoveBrain(group, target_yx);
    } else {
        this->AssignNearbyTargets(group);
        POSITION it = group->unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* unit = group->unit_list.GetNext(it);
            if (unit->eye2->unit4 == nullptr) {
                this->SetIdleOrRetreat(unit, 1);
            } else {
                this->DoAttackTarget(unit, unit->eye2->unit4);
            }
        }
    }
}

// 5ab719 (client: 579889) - Compute average position of units in a group.
uint32_t World::ComputeGroupAvgPos(Group* group) {
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

    group->group_sub->avg_pos_yyxx = position_yyxx;
    group->group_sub->avg_pos = position_yx.val;

    uint8_t max_distance = 0;
    uint8_t max_scan_range = 0;
    uint8_t max_sum = 0;

    unit_it = group->unit_list.GetHeadPosition();
    while (unit_it != nullptr) {
        Unit* unit = group->unit_list.GetNext(unit_it);
        uint8_t distance = this->map_stuff->ChebyshevDistance(unit->position->GetYX(), position_yx);
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

    group->group_sub->max_unit_dist = max_distance;
    group->group_sub->max_scan_range = max_scan_range;
    group->group_sub->max_dist_plus_scan = max_sum;

    return position_yyxx;
}

// Server turn tick: rebuild presence grid, run world updates, process each
// player's groups, and log turn timing statistics when tracing is enabled.
// 5abd16 (client: 579e86)
void World::ProcessTick(PlayersList* players) {
    this->perf_tick.Begin();
    if (this->skip_presence_grid == 0) {
        this->perf_presence.Begin();
        this->map_stuff->scan_presence_grid.RebuildPresenceGrid();
        this->perf_presence.End();
        this->duration1 = (int32_t)this->perf_presence.period64;
        this->total1 += (int32_t)this->perf_presence.period64;
    }

    this->perf_triggers.Begin();
    this->ProcessTriggers();
    this->perf_triggers.End();
    this->duration2 = (int32_t)this->perf_triggers.period64;
    this->total2 += (int32_t)this->perf_triggers.period64;

    this->perf_ai.Begin();
    POSITION players_iter = players->list.GetHeadPosition();
    while (players_iter != nullptr) {
        Player* player = players->list.GetNext(players_iter);
        this->ProcessPlayerGroupAi(player);
    }
    this->ProcessVirtualCasters();
    this->perf_ai.End();
    this->duration3 = (int32_t)this->perf_ai.period64;
    this->total3 += (int32_t)this->perf_ai.period64;

    this->counter += 1;
    this->perf_tick.End();
    this->duration4 = (int32_t)this->perf_tick.period64;
    this->duration4_low = (int32_t)(this->perf_tick.period64 >> 32);
    this->duration5 = (int32_t)this->perf_tick.period64;
    this->total5 += (int32_t)this->perf_tick.period64;

    if (g_Server->script_settings->turn_tracing != 0) {
        CString str;
        str.Format("Stats: last turn - %d, average: %d.\n", this->duration5, this->total5 / this->counter);
        g_NetStru1_main.AnnouncePlayerName(str, nullptr);
    }
}

// 5b0724 (client: 57e894)
void World::ProcessTriggers() {
    this->trigger_variables[999] += 1;
    this->EvaluateTriggerChecks();
    this->ExecuteTriggerActions();
    this->action_list.RemoveAll();
}

// Evaluate map triggers and run their actions when all conditions pass.
// 5b0a6a (client: 57ebda)
void World::ExecuteTriggerActions() {
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
                case Check::CMP_EQ: all_pass = all_pass && (left == right); break;
                case Check::CMP_NE: all_pass = all_pass && (left != right); break;
                case Check::CMP_GT: all_pass = all_pass && (left > right); break;
                case Check::CMP_LT: all_pass = all_pass && (left < right); break;
                case Check::CMP_GE: all_pass = all_pass && (left >= right); break;
                case Check::CMP_LE: all_pass = all_pass && (left <= right); break;
                default: all_pass = false; break;
            }
        }

        if (all_pass) {
            this->trigger_results[trigger.trigger_id] = 1;
            if (trigger.once != 0 && g_Server->script_settings->script_tracing != 0) {
                CString msg;
                msg.Format("Script: Trigger %d ( %d ifs, %d instants ).\n", trigger.trigger_id, trigger.checks->GetCount(), trigger.actions->GetCount());
                g_NetStru1_main.AnnouncePlayerName(msg, nullptr);
            }
            POSITION action_it = trigger.actions->GetHeadPosition();
            while (action_it != nullptr) {
                int32_t script_id = trigger.actions->GetNext(action_it);
                this->DoScriptInstID(script_id);
            }
        }
    }
}

// Try to cast a healing/buff spell on the caster or the most-damaged nearby ally.
// 5a845b (client: 5765cb) - AI mage retreat helper: cast healing/buff on self or most-damaged ally.
void World::CastSelfHealOrBuff(Unit* unit) {
    if (unit->spell_book == nullptr) {
        return;
    }
    if (unit->mp <= 10) {
        return;
    }

    Spell* spell = unit->spell_book->GetSpell(spell::heal);
    if (spell == nullptr) {
        return;
    }

    if (unit->hp != unit->hp_max) {
        unit->eye2->action_done = 1;
        this->BeginAutobuffCast(unit, unit, spell);
        return;
    }

    PosYX yx = unit->position->GetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, spell->max_range);
    this->FilterBuffTargets(unit, list);

    double best_ratio = 2.0;
    Unit* best_unit = nullptr;
    if (list != nullptr) {
        POSITION it = list->unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = list->unit_list.GetNext(it);
            // WAT: that's integer division in vanilla, so `ratio` is likely 0 or 1.
            double ratio = static_cast<double>(other->hp / other->hp_max);
            if (ratio < best_ratio) {
                best_ratio = ratio;
                best_unit = other;
            }
        }
    }

    if (best_ratio >= 1.0) {
        this->PickFallbackBuff(unit, list);
    } else {
        unit->eye2->action_done = 1;
        this->BeginAutobuffCast(unit, best_unit, spell);
    }
}

// Set up an autobuff cast action on caster targeting target (or the nearest
// eligible unit when target is null). If no target is found the caster idles.
// 5a85f4 (client: 576764) - Set up autobuff cast action for caster.
void World::BeginAutobuffCast(Unit* caster, Unit* target, Spell* spell) {
    #ifdef A2SERVER_PATCH
    // bug_001A: target->eye2 may be null (or target may be a Building) - treat as unavailable when invalid
    if (target != nullptr) {
        if (!target->IsUnit()) {
            if (spell != nullptr && spell->is_defensive != 0) {
                target = nullptr;
            }
        } else if (target->eye2 == nullptr) {
            target = nullptr;
        }
    }
    #endif
    if (target == nullptr) {
        PosYX yx = caster->position->GetYX();
        UnitList* list = (spell->is_defensive != 0) ? this->GetNearbyAllies(caster, yx) : this->GetNearbyEnemies(caster, yx);
        if (list->unit_list.GetCount() != 0) {
            target = list->unit_list.GetHead();
        }
    }

    if (target == nullptr) {
        this->SetIdleOrRetreat(caster, 0);
    } else {
        caster->eye2->cast_action = 8;
        caster->eye2->unit5 = target;
        caster->eye2->spell = spell;
        caster->eye2->max_range = this->UnitMaxRange(caster);
#ifdef A2SERVER_PATCH
        if (target->IsUnit() && target->eye2 != nullptr) {
            target->eye2->autobuff_spell_id = spell->spell_id;
            target->eye2->autobuff_tick = g_Server->tick;
        }
#else
        target->eye2->autobuff_spell_id = spell->spell_id;
        target->eye2->autobuff_tick = g_Server->tick;
#endif
    }

    caster->eye2->max_range = spell->max_range;
}

// Fallback buff selection for CastSelfHealOrBuff: pick a random allied unit from `list`
// and attempt to cast heal/buff spells on it.
// 5b6862 (client: 5849d2) - fallback buff selection for CastSelfHealOrBuff.
void World::PickFallbackBuff(Unit* unit, UnitList* list) {
    // Heal self.
    if (unit->hp != unit->hp_max) {
        Spell* heal = unit->spell_book->GetSpell(spell::heal);
        if (heal != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, unit, heal);
            return;
        }
    }

    // Shield self, unless already shielded.
    Spell* shield = unit->spell_book->GetSpell(spell::shield);
    if (shield != nullptr && (unit->enchantments & (1 << shield->spell_id)) == 0) {
        unit->eye2->action_done = 1;
        this->BeginAutobuffCast(unit, unit, shield);
        return;
    }

    // Pick a random same-player target from the provided list.
    Unit* target = nullptr;
    if (list != nullptr) {
        int32_t count = list->unit_list.GetCount();
        int32_t index = this->RandomScaled(count);
        POSITION it = list->unit_list.FindIndex(index);
        if (it != nullptr) {
            target = list->unit_list.GetAt(it);
        }
    }

    if (target == nullptr || target->pOwner != unit->pOwner) {
        return;
    }

    // Heal.
    if (target->hp != target->hp_max) {
        Spell* heal = unit->spell_book->GetSpell(spell::heal);
        if (heal != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, target, heal);
            return;
        }
    }

    // Only consider buffs when caster has full mana.
    if (unit->mp != unit->mp_max) {
        return;
    }

    // Build a list of affordable defensive buff spells excluding heal/shield.
    CList<Spell*> buffs;
    for (int32_t spell_id = 1; spell_id < spell::max_spell_id; spell_id++) {
        Spell* spell = unit->spell_book->GetSpell(spell_id);
        if (spell != nullptr && spell->mana_cost <= unit->mp && spell->is_defensive) {
            if (spell->spell_id != spell::heal && spell->spell_id != spell::shield) {
                buffs.AddTail(spell);
            }
        }
    }

    int32_t index = this->RandomScaled(buffs.GetCount());
    POSITION buff_pos = buffs.FindIndex(index);
    Spell* buff = (buff_pos != nullptr) ? buffs.GetAt(buff_pos) : nullptr;
    if (buff == nullptr || (target->enchantments & (1 << buff->spell_id)) != 0) {
        return;
    }

    unit->eye2->action_done = 1;
    this->BeginAutobuffCast(unit, target, buff);
}

// Pick a spell from the caster's spellbook and start casting it at the
// appropriate target (self, target unit, or target position).
// 5a6801 (client: 574971) - pick and start casting a known spell.
void World::CastKnownSpell(Unit* caster, Unit* target, int32_t spell_id) {
    Spell* spell = caster->spell_book->GetSpell(spell_id);
    if (spell == nullptr) {
        return;
    }

    switch (spell_id) {
        case spell::fire_arrow:
        case spell::ice_missile:
        case spell::lightning:
        case spell::prismatic_spray:
        case spell::stone_missile:
        case spell::stone_curse:
        case spell::drain_life:
        case spell::curse:
        case spell::slow:
            this->BeginAutobuffCast(caster, target, spell);
            break;

        case spell::protection_from_fire:
        case spell::protection_from_water:
        case spell::invisibility:
        case spell::protection_from_air:
        case spell::protection_from_earth:
        case spell::bless:
        case spell::haste:
        case spell::heal:
        case spell::summon:
        case spell::shield:
            this->BeginAutobuffCast(caster, caster, spell);
            break;

        case spell::fire_ball:
        case spell::wall_of_fire:
        case spell::poison_cloud:
        case spell::blizzard:
        case spell::darkness:
        case spell::light:
        case spell::wall_of_earth:
            this->BeginPositionCast(caster, target->position->CompatGetYX(), spell);
            break;

        case spell::acid_stream: {
            uint8_t facing = this->map_stuff->GetFacingTo(caster, target);
            uint8_t new_x = caster->position->GetX() + this->map_stuff->neighbor_dx[facing >> 5];
            uint8_t new_y = caster->position->GetY() + this->map_stuff->neighbor_dy[facing >> 5];
            uint16_t pos_yx = PosYX(new_x, new_y);
            this->BeginPositionCast(caster, pos_yx, spell);
            break;
        }

        case spell::teleport: {
            uint16_t target_yx = target->position->CompatGetYX();
            uint16_t caster_yx = caster->position->CompatGetYX();
            uint8_t distance = this->map_stuff->ChebyshevDistance(caster_yx, target_yx);
            if (distance > 2) {
                uint16_t found_yx = 0;
                for (uint8_t dy = 0; dy <= 2; dy++) {
                    for (uint8_t dx = 0; dx <= 2; dx++) {
                        uint16_t check_yx = target_yx + PosYX{dx, dy}.val - PosYX{1, 1}.val;
                        if (this->map_stuff->IsCellPassableDynamic(caster, check_yx) != 0) {
                            found_yx = check_yx;
                            break;
                        }
                    }
                    if (found_yx != 0) {
                        break;
                    }
                }
                if (found_yx != 0) {
                    this->BeginPositionCast(caster, found_yx, spell);
                }
            }
            break;
        }
    }

    caster->eye2->action_done = 1;
}

// Pick a random unit near pos_yx that is an enemy (is_defensive=0) or ally
// (is_defensive=1) of owner, skipping invisible units.
// 5b61d0 (client: 584340)
Unit* World::PickRelationUnitNear(uint16_t pos_yx, uint8_t byte_0x3c, Player* owner, int32_t is_defensive) {
    this->map_stuff->GatherUnitsNear(pos_yx, byte_0x3c, owner, &this->nearby_list);

    CList<Unit*>& list = this->nearby_list.unit_list;
    POSITION it = list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current = it;
        Unit* unit = list.GetNext(it);
        Player* unit_owner = unit->pOwner;
        bool remove = false;
        if ((this->diplomacy.diplomacy[owner->player_id][unit_owner->player_id] & DIPLO_WAR) == is_defensive) {
            remove = true;
        } else if ((unit->enchantments & (1 << spell::invisibility)) != 0) {
            remove = true;
        }
        if (remove) {
            list.RemoveAt(current);
        }
    }

    int32_t count = list.GetCount();
    int32_t idx = this->RandomRange(1, count);
    POSITION selected = list.FindIndex(idx - 1);
    if (selected != nullptr) {
        return list.GetAt(selected);
    }
    return nullptr;
}

// Process pending virtual casters and fire their spells into the world.
// 5b6346 (client: 5844b6)
void World::ProcessVirtualCasters() {
    CList<VirtualCaster*>& vc_list = g_Server->srv_stru1->virtual_casters_list;
    POSITION it = vc_list.GetHeadPosition();
    while (it != nullptr) {
        VirtualCaster* vc = vc_list.GetNext(it);
        Spell* spell = g_Server->spells[vc->properties[0]];
        uint16_t pos_yx = PosYX{vc->properties[2], vc->properties[3]}.val;
        Unit* unit = this->PickRelationUnitNear(pos_yx, vc->byte_0x3c, vc->pOwner, spell->is_defensive);
        if (unit != nullptr) {
            g_Server->srv_stru1->MakeTargetSpellEffect(vc->properties[2], vc->properties[3], unit, vc->properties[0], vc->properties[1]);
        }
    }
}

// 5a9961 (client: 577ad1)
void World::OrderSwarmTo(Unit* unit, uint8_t x, uint8_t y) {
    this->BeginUnitOrder(unit);
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
    // solo_pickup_sack hook (5A9977)
    SoloPickup(unit, y, x);
#else
    unit->state = Unit::ORDER_PICKUP;
#endif
    unit->eye2->command_to = ((uint16_t)y << 8) + x;
    unit->eye->target_reached = 0;
    unit->eye2->cast_action = 0;
}

// 5a99c7 (client: 577b37)
void World::OrderGatherLoot(Unit* unit) {
    this->BeginUnitOrder(unit);
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
    // solo_pick_all_sacks hook (5A99DD)
    SoloPickupAll(unit);
#else
    unit->state = Unit::ORDER_LOOT_NEARBY;
#endif
    unit->eye->target_reached = 0;
    unit->eye2->cast_action = 0;
}

// Record a PvP hit: mark the attacker in UnitEye2, update diplomacy, push a combat
// action onto the world's action list, and store the retaliation cell.
// 5aa581 (client: 5786f1) - Record PvP hit and update combat log
void World::RecordUnitHit(Unit* target, Unit* attacker, int32_t hit_flag) {
    attacker->eye2->engaged = 1;
    attacker->eye2->position3 = target->position->CompatGetYX();
    attacker->eye2->counter2 = 0;

    this->diplomacy.RecordHostility(target, attacker, hit_flag);

    this->scrape_action.action_kind = 1;
    this->scrape_action.actor_unit = target;
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
        attacker->eye2->attack_move_pos = PosYX{new_x, new_y}.val;
    } else {
        // This wasn't in vanilla, but otherwise we divide by zero.
        attacker->eye2->attack_move_pos = attacker->position->GetYX().val;
    }
}

// 5aaa89 (client: 578bf9)
void World::ChaseOrCast(Unit* unit) {
#ifdef A2TANGAR_HAT
    // imp_distance_bug_fix hook (5AAA8D): state 13 is "cast point spell from
    // the book onto an enemy that is too far" - if the queued spell is not
    // defensive, stop instead of casting
    if (unit->state == Unit::ORDER_CAST_UNIT) {
        if (unit->eye2 == nullptr || unit->eye2->spell == nullptr || unit->eye2->spell->is_defensive == 0) {
            unit->state = Unit::ORDER_HOLD_POSITION;
        }
    }
#endif
    if (unit != unit->eye2->unit5 && this->map_stuff->FacesTargetInRange(unit, unit->eye2->unit5, unit->eye2->max_range) == 0) {
        this->StepTowardUnit(unit, unit->eye2->unit5);
        unit->some_state = Unit::EXEC_STEP;
    } else {
        this->StartCastAtUnit(unit);
    }
}

// 5a9b6b (client: 577cdb) - AI tick for a monster unit?
void World::MonsterAiTick(Unit* unit) {
    if (this->skip_presence_grid == 0 && unit->group->group_sub->presence_detected == 0 && unit->eye2->ai_state == 0) {
        unit->some_state = Unit::EXEC_SPECIAL;
        return;
    }

    if (unit->some_state != Unit::EXEC_PICKUP && unit->some_state != Unit::EXEC_INTERACT_BUILDING) {
        unit->some_state = Unit::EXEC_IDLE;
    }

    if (unit->enchantments != 0 && (unit->enchantments & (1 << spell::stone_curse)) != 0 && unit->eye2->ai_state == 0) {
        unit->eye2->ai_state = 4;
    }

    if (unit->eye2->ai_state == 0) {
        if (unit->eye2->teleport_pos.val != 0 && unit->position->IsAtCellCenter()) {
            this->map_stuff->TeleportUnit(unit, unit->eye2->teleport_pos.x, unit->eye2->teleport_pos.y);
            unit->eye2->teleport_pos = PosYX{0};
        }

        switch (unit->eye2->cast_action) {
        case 1:
            this->DoMoveStepTo(unit, unit->eye2->command_to, 0);
            break;
        case 2:
            this->StartAttack(unit);
            break;
        case 4:
            this->StepTowardUnit(unit, unit->eye2->unit3);
            break;
        case 5:
            this->ChaseOrAttack(unit);
            break;
        case 6:
            this->TickRangedApproach(unit);
            break;
        case 7:
            this->FinishSwarmArrival(unit);
            break;
        case 8:
            this->ChaseOrCast(unit);
            break;
        case 9:
            this->ChaseOrAreaCast(unit);
            break;
        case 10:
            this->DoTurnTo(unit, unit->eye->target_facing);
            break;
        case 11:
            this->DoIdleFidget(unit);
            break;
        case 12:
            this->CheckMoveArrival(unit);
            break;
        case 15:
            this->ApproachBuilding(unit, unit);
            break;
        default:
            break;
        }
    } else {
        switch (unit->eye2->ai_state) {
        case 1:
            unit->some_state = Unit::EXEC_ATTACK;
            unit->eye2->counter++;
            if (unit->eye2->counter > 2 && unit->attack_ready != 0) {
                unit->eye2->ai_state = 0;
                unit->some_state = Unit::EXEC_IDLE;
            }
            break;
        case 2:
            if (unit->eye2->cast_is_area == 0) {
                unit->some_state = Unit::EXEC_CAST_POS;
            } else {
                unit->some_state = Unit::EXEC_CAST_UNIT;
            }
            unit->eye2->counter++;
            if (unit->eye2->counter > 2) {
                if (unit->eye2->counter == 3) {
                    if (unit->eye2->cast_is_area == 0) {
                        this->scrape_action.action_kind = 3;
                        this->scrape_action.actor_unit = unit;
                        this->scrape_action.payload = unit->some_spell;
                        this->scrape_action.x = unit->area_cast_x;
                        this->scrape_action.y = unit->area_cast_y;
                    } else {
                        this->scrape_action.action_kind = 2;
                        this->scrape_action.actor_unit = unit;
                        this->scrape_action.payload = unit->some_spell;
                        this->scrape_action.target = unit->cast_target;
                    }
                    this->action_list.AddTail(this->scrape_action);
                }
                if (unit->attack_ready != 0) {
                    unit->eye2->ai_state = 0;
                    unit->some_state = Unit::EXEC_IDLE;
                }
            }
            break;
        case 3:
            this->map_stuff->UpdateEyeMoveTime(unit);
            unit->some_state = Unit::EXEC_STEP;
            if (unit->position->IsAtCellCenter()) {
                unit->eye2->ai_state = 0;
                unit->some_state = Unit::EXEC_IDLE;
            }
            break;
        case 4:
            unit->some_state = Unit::EXEC_SPECIAL;
            if ((unit->enchantments & (1 << spell::stone_curse)) == 0) {
                unit->eye2->ai_state = 0;
            }
            break;
        case 0xff:
            unit->some_state = Unit::EXEC_SPECIAL;
            break;
        default:
            unit->eye2->ai_state = 0;
            unit->some_state = Unit::EXEC_SPECIAL;
            break;
        }
    }

    if (unit->eye->order_arrived != 0) {
        unit->eye->order_arrived = 0;
        if (unit->state == Unit::ORDER_MOVE) {
            this->PrepareUnitOrder(unit);
            this->ResetUnitToHold(unit);
        } else if (unit->state == Unit::ORDER_PATROL_STEP) {
            unit->eye2->patrol_reanchor = 1;
            unit->eye2->cast_action = 0;
            uint8_t x2 = unit->eye2->position2.x;
            uint8_t y2 = unit->eye2->position2.y;
            uint8_t x1 = unit->position->GetX();
            uint8_t y1 = unit->position->GetY();
            if (this->map_stuff->ChebyshevDistanceXY(x1, y1, x2, y2) < 3) {
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
        } else if (unit->state == Unit::ORDER_SCRIPT_PATROL) {
            unit->eye2->ai_state = 0xFF;
        } else {
            unit->eye2->cast_action = 0;
            this->AcquireNearestTarget(unit);
        }
    }
}

// 5aac17 (client: 578d87) - pick a melee/cast target for a monster unit.
void World::AcquireNearestTarget(Unit* unit) {
    this->nearby_list.unit_list.RemoveAll();

    uint32_t best_distance = 0xff;
    CList<Unit*>* all = nullptr;
    if (this->map_stuff->all_units != nullptr) {
        all = &this->map_stuff->all_units->unit_list;
    }

    if (all != nullptr) {
        POSITION unit_it = all->GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = all->GetNext(unit_it);
            uint8_t dist = other->position->Distance(unit->position);
            uint8_t max_range = this->UnitMaxRange(unit);
            if (dist <= max_range && dist <= best_distance && other != unit) {
                this->nearby_list.AddTail(other);
                best_distance = dist;
            }
        }
    }

    this->FilterByRelation(unit, &this->nearby_list, 0);
    this->PruneUnconsciousTargets();

    if (this->nearby_list.unit_list.GetCount() == 0 && this->unconscious_list.unit_list.GetCount() != 0) {
        CList<Unit*>* alt = &this->unconscious_list.unit_list;
        POSITION unit_it = alt->GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = alt->GetNext(unit_it);
            this->nearby_list.AddTail(other);
        }
        alt->RemoveAll();
    }

    Unit* best_target = nullptr;
    uint32_t best_diff = 300;
    if (this->nearby_list.unit_list.GetCount() != 0) {
        POSITION unit_it = this->nearby_list.unit_list.GetHeadPosition();
        while (unit_it != nullptr) {
            Unit* other = this->nearby_list.unit_list.GetNext(unit_it);
            uint8_t facing = this->map_stuff->GetFacingTo(unit, other);
            uint8_t diff = this->map_stuff->FacingDiff(unit->eye->facing, facing);
            if (diff <= best_diff) {
                best_target = other;
                best_diff = diff;
            }
        }
    }

    if (best_target != nullptr) {
        unit->eye2->unit = best_target;
        unit->eye2->cast_action = 6;
        this->TickRangedApproach(unit);
    } else {
        if (!unit->pOwner->is_ai) {
            unit->eye2->cast_action = 0;
        } else {
            unit->eye2->cast_action = 0xb;
            this->DoIdleFidget(unit);
        }
    }
}

// Target-pursuit spell selection for AI-controlled mages.
// Returns 1 if a cast action was set up, 0 otherwise.
// 5a6ed9 (client: 575049) - target-pursuit check.
int32_t World::ShouldPursueTarget(Unit* caster, Unit* target) {
    if (caster->eye2->spell_id == 0) {
        return 0;
    }

    Spell* spell = this->GetCastableSpell(caster, caster->eye2->spell_id, 0);
    if (spell == nullptr) {
        return 0;
    }

    switch (caster->eye2->spell_id) {
    case spell::fire_arrow:
    case spell::ice_missile:
    case spell::lightning:
    case spell::prismatic_spray:
    case spell::stone_missile:
    case spell::drain_life:
        caster->eye2->action_done = 1;
        this->BeginAutobuffCast(caster, target, spell);
        return 1;

    case spell::fire_ball:
    case spell::wall_of_fire:
    case spell::poison_cloud:
    case spell::blizzard: {
        this->BuildAllyList(caster->group);
        this->nearby_list.unit_list.AddTail(target);
        this->SplitTargetLists(caster, &this->nearby_list);

        bool adjust_pos = (caster->eye2->spell_id == spell::fire_ball) || (caster->eye2->spell_id == spell::wall_of_fire);
        uint8_t range = (caster->eye2->spell_id == spell::fire_ball) ? 1 : 2;

        int32_t best_score = -1000;
        POSITION it = this->targets.unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* nearby = this->targets.unit_list.GetNext(it);
            PosYX yx = nearby->position->CompatGetYX();
            if (adjust_pos) {
                if (nearby->position->IsAtCellCenter() == 0) {
                    uint8_t dir = nearby->eye->facing >> 5;
                    yx.x = yx.x + this->map_stuff->neighbor_dx[dir];
                    yx.y = yx.y + this->map_stuff->neighbor_dy[dir];
                }
            }
            UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, range);
            int32_t enemy_count = 0;
            int32_t friendly_count = 0;
            POSITION it2 = (list != nullptr) ? list->unit_list.GetHeadPosition() : nullptr;
            while (it2 != nullptr) {
                Unit* other = list->unit_list.GetNext(it2);
                if (this->AreAtWar(caster, other)) {
                    enemy_count++;
                } else if (other == caster) {
                    friendly_count += 100;
                } else {
                    friendly_count++;
                }
            }
            int32_t score = enemy_count - friendly_count;
            if (score > best_score) {
                best_score = score;
            }
        }

        if (best_score < 1) {
            return 0;
        }
        caster->eye2->action_done = 1;
        this->BeginAutobuffCast(caster, target, spell);
        return 1;
    }

    case spell::stone_curse:
    case spell::curse: {
        uint32_t mask = 1 << caster->eye2->spell_id;
        if ((target->enchantments & mask) == 0) {
            this->BuildAllyList(caster->group);
            this->SplitTargetLists(caster, &this->nearby_list);
            POSITION it = this->targets.unit_list.GetHeadPosition();
            while (it != nullptr) {
                Unit* other = this->targets.unit_list.GetNext(it);
                if ((other->enchantments & mask) == 0) {
                    caster->eye2->action_done = 1;
                    this->BeginAutobuffCast(caster, target, spell);
                    return 1;
                }
            }
            return 0;
        }
        caster->eye2->action_done = 1;
        this->BeginAutobuffCast(caster, target, spell);
        return 1;
    }

    case spell::animate_dead:
        return 0;

    case spell::heal: {
        this->BuildAllyList(caster->group);
        this->SplitTargetLists(caster, &this->nearby_list);

        Unit* best_unit = nullptr;
        double best_ratio = 1.0;
        POSITION it = this->non_targets.unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = this->non_targets.unit_list.GetNext(it);
            if (this->ShouldAutobuff(caster, other) == 0) {
                continue;
            }
            // WAT: this integer division is weird.
            double ratio = (double)(other->hp / other->hp_max);
            if (ratio < best_ratio) {
                best_ratio = ratio;
                best_unit = other;
            }
        }
        if (best_ratio >= 1.0) {
            caster->eye2->cast_action = 0;
            return 1;
        }
        caster->eye2->action_done = 1;
        this->BeginAutobuffCast(caster, best_unit, spell);
        return 1;
    }

    case spell::summon:
        this->BeginAutobuffCast(caster, caster, spell);
        return 1;

    case spell::shield:
        if ((caster->enchantments & (1 << spell::shield)) == 0) {
            caster->eye2->action_done = 1;
            this->BeginAutobuffCast(caster, caster, spell);
            return 1;
        }
        return 0;

    default:
        return 0;
    }
}

// Check if `caster` should autobuff `target` based on diplomacy and settings.
// 5a7a1c (client: 575b8c) - Check if `unit` should autobuff `other` (diplomacy/settings check).
int32_t World::ShouldAutobuff(Unit* caster, Unit* target) {
    uint8_t relation = this->diplomacy.diplomacy[caster->pOwner->player_id][target->pOwner->player_id] & (DIPLO_WAR | DIPLO_ALLIANCE);

    // Neutral.
    if (relation == 0 && caster->pOwner->settings->autobuff_mask.neutral_units != 0) {
        return 1;
    }

    // Friend.
    if ((relation & DIPLO_ALLIANCE) != 0 && caster->pOwner->settings->autobuff_mask.allied_units != 0) {
        return 1;
    }

    // Own units.
    if (caster->pOwner == target->pOwner && caster->pOwner->settings->autobuff_mask.own_units != 0) {
        return 1;
    }

    // Self.
    if (caster == target) {
        return 1;
    }

    return 0;
}

// Autobuff handler: pick and cast healing/buff spells on self or allies.
// 5a7b44 (client: 575cb4) - Autobuff handler: pick and cast buff spells on self or allies.
void World::DoAutobuff(Unit* unit) {
    if (unit->mp < 5) {
        unit->eye2->cast_action = 0;
        return;
    }

    if (unit->pOwner->settings->autobuff_mask.tier_low == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Self-heal.
    if (unit->hp != unit->hp_max) {
        Spell* spell = this->GetAutobuffSpell(unit, spell::heal);
        if (spell != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, unit, spell);
            return;
        }
    }

    // Self-shield.
    if ((unit->enchantments & (1 << spell::shield)) == 0) {
        Spell* spell = this->GetAutobuffSpell(unit, spell::shield);
        if (spell != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, unit, spell);
            return;
        }
    }

    // Build nearby friendly unit list for the group.
    this->BuildAllyList(unit->group);
    this->SplitTargetLists(unit, &this->nearby_list);

    // Ally-heal.
    Spell* spell = this->GetAutobuffSpell(unit, spell::heal);
    if (spell != nullptr && this->non_targets.unit_list.GetCount() > 1) {
        double best_ratio = 2.0;
        Unit* best_unit = nullptr;

        POSITION pos = this->non_targets.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->non_targets.unit_list.GetNext(pos);
            if (!this->ShouldAutobuff(unit, other) || !this->IsSpellInRange(unit, other, spell)) {
                continue;
            }

            // WAT: I guess the value should be cast before division? But original does it like this.
            double ratio = other->hp / other->hp_max;
            if (ratio < best_ratio) {
                best_ratio = ratio;
                best_unit = other;
            }
        }

        if (best_ratio < 1.0) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, best_unit, spell);
            return;
        }
    }

    // Check autobuff_mask bit for medium-level buffs.
    if (unit->pOwner->settings->autobuff_mask.tier_medium == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    if (unit->mp < 10) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Ally medium-level buffs.
    {
        POSITION pos = this->non_targets.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->non_targets.unit_list.GetNext(pos);
            if (this->ShouldAutobuff(unit, other) == 0) {
                continue;
            }

            int32_t autobuff_backoff = abs((int32_t)other->eye2->autobuff_tick - g_Server->tick);

            // Bless: requires fighter, no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::bless);
            if (spell != nullptr && (other->unit_attrs & Unit::ATTR_MAGE) == 0 && (other->enchantments & (1 << spell::bless)) == 0) {
                if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                    if (this->IsSpellInRange(unit, other, spell) != 0) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }

            // Haste: no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::haste);
            if (spell != nullptr && (other->enchantments & (1 << spell::haste)) == 0) {
                if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                    if (this->IsSpellInRange(unit, other, spell) != 0) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }
        }
    }

    // Check autobuff_mask bit for high-level buffs.
    if (unit->pOwner->settings->autobuff_mask.tier_high == 0) {
        unit->eye2->cast_action = 0;
        return;
    }

    if (unit->mp < 40) {
        unit->eye2->cast_action = 0;
        return;
    }

    // Ally high-level buffs.
    {
        POSITION pos = this->non_targets.unit_list.GetHeadPosition();
        while (pos) {
            Unit* other = this->non_targets.unit_list.GetNext(pos);
            if (this->ShouldAutobuff(unit, other) == 0) {
                continue;
            }

            int32_t autobuff_backoff = abs((int32_t)other->eye2->autobuff_tick - g_Server->tick);

            // Protection from air: no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::protection_from_air);
            if (spell != nullptr && this->IsSpellInRange(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_air)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from earth: no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::protection_from_earth);
            if (spell != nullptr && this->IsSpellInRange(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_earth)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from fire: no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::protection_from_fire);
            if (spell != nullptr && this->IsSpellInRange(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_fire)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }

            // Protection from water: no matching enchantment.
            spell = this->GetAutobuffSpell(unit, spell::protection_from_water);
            if (spell != nullptr && this->IsSpellInRange(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::protection_from_water)) == 0) {
                    if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                        unit->eye2->action_done = 1;
                        this->BeginAutobuffCast(unit, other, spell);
                        return;
                    }
                }
            }

            // Invisibility: no matching enchantment, not self, diplomacy check.
            spell = this->GetAutobuffSpell(unit, spell::invisibility);
            if (spell != nullptr && this->IsSpellInRange(unit, other, spell) != 0) {
                if ((other->enchantments & (1 << spell::invisibility)) == 0 && unit != other) {
                    if ((this->diplomacy.diplomacy[unit->pOwner->player_id][other->pOwner->player_id] & (DIPLO_WAR | DIPLO_ALLIANCE)) != 0) {
                        if (other->eye2->autobuff_spell_id != 20 || autobuff_backoff > 20) {
                            unit->eye2->action_done = 1;
                            this->BeginAutobuffCast(unit, other, spell);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Self-invisibility --- last.
    spell = this->GetAutobuffSpell(unit, spell::invisibility);
    if (spell != nullptr && (unit->enchantments & (1 << spell::invisibility)) == 0) {
        unit->eye2->action_done = 1;
        this->BeginAutobuffCast(unit, unit, spell);
        return;
    }

    unit->eye2->cast_action = 0;
}

// Run trigger action.
// 5b0e08 (client: 57ef78) - Run script by instance ID
void World::DoScriptInstID(int32_t script_id) {
    TriggerAction action = (*this->trigger_actions)[script_id];

    if (g_Server->script_settings->script_tracing != 0) {
        char buf[100];
        sprintf(buf, "Script: Instant %d ( type: %d ).\n", script_id, (int)action.type);
        CString msg(buf);
        g_NetStru1_main.AnnouncePlayerName(msg, nullptr);
    }

    if ((action.ref_count & 0xFF) != 0) {
        int count_groups = 0;
        int count_players = 0;
        int count_units = 0;
        for (int i = 0; i < 10; i++) {
            if (action.ref_scope[i] == 0) {
                continue;
            }
            uint8_t entry_type = action.ref_kind[i];
            void* ref = this->references[action.ref_index[i]];
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
        case script_action_op::increment_mission_stage: this->ScriptOP_0x01(&action); break;
        case script_action_op::send_game_event:         this->ScriptOP_0x02(&action); break;
        case script_action_op::set_variable:            this->ScriptOP_0x03(&action); break;
        case script_action_op::mission_complete:        this->ScriptOP_0x04(&action); break;
        case script_action_op::mission_fail:            this->ScriptOP_0x05(&action); break;
        case script_action_op::group_command:           this->ScriptOP_0x06(&action); break;
        case script_action_op::set_formation:           this->ScriptOP_0x07(&action); break;
        case script_action_op::increment_variable:      this->ScriptOP_0x08(&action); break;
        case script_action_op::noop_09:                 break;  // No-op.
        case script_action_op::change_diplomacy:        this->ScriptOP_0x0a(&action); break;
        case script_action_op::transfer_item:           this->ScriptOP_0x0b(&action); break;
        case script_action_op::give_item:               this->ScriptOP_0x0c(&action); break;
        case script_action_op::remove_item:             this->ScriptOP_0x0d(&action); break;
        case script_action_op::kill_unit:               this->ScriptOP_0x0e(&action); break;
        case script_action_op::kill_unit_alt:           this->ScriptOP_0x0f(&action); break;
        case script_action_op::phase_out_unit:          this->ScriptOP_0x10(&action); break;
        case script_action_op::phase_in_unit:           this->ScriptOP_0x11(&action); break;
        case script_action_op::polymorph_unit:          this->ScriptOP_0x12(&action); break;
        case script_action_op::change_unit_owner:       this->ScriptOP_0x13(&action); break;
        case script_action_op::drop_all_items:          this->ScriptOP_0x14(&action); break;
        case script_action_op::cast_area_spell:         this->ScriptOP_0x15(&action); break;
        case script_action_op::change_group_owner:      this->ScriptOP_0x16(&action); break;
        case script_action_op::give_money:              this->ScriptOP_0x17(&action); break;
        case script_action_op::cast_unit_spell:         this->ScriptOP_0x18(&action); break;
        case script_action_op::set_trap:                this->ScriptOP_0x19(&action); break;
        case script_action_op::set_building_health:     this->ScriptOP_0x1a(&action); break;
        case script_action_op::teleport_unit:           this->ScriptOP_0x1b(&action); break;
        case script_action_op::transfer_all_items:      this->ScriptOP_0x1c(&action); break;
        case script_action_op::set_tile_spell_duration: this->ScriptOP_0x1d(&action); break;
        case script_action_op::set_unit_spell_duration: this->ScriptOP_0x1e(&action); break;
        case script_action_op::set_repop_delay:         this->ScriptOP_0x1f(&action); break;
        case script_action_op::phase_out_group:         this->ScriptOP_0x20(&action); break;
        case script_action_op::phase_in_group:          this->ScriptOP_0x21(&action); break;
        case script_action_op::set_unit_param:          this->ScriptOP_0x22(&action); break;
        case script_action_op::set_scenario_variable:   this->ScriptOP_0x23(&action); break;
        case script_action_op::set_subobjective:        this->ScriptOP_0x24(&action); break;
        // no 0x25
        case script_action_op::remove_item_from_all:    this->ScriptOP_0x26(&action); break;
        case script_action_op::suspend_group:           this->ScriptOP_0x27(&action); break;
        default: {
            if (g_Server->script_settings->turn_tracing != 0) {
                char buf[100];
                sprintf(buf, "Script: Bad instant %d.\n", (int)action.type);
                CString msg(buf);
                g_NetStru1_main.AnnouncePlayerName(msg, nullptr);
            }
        }
    }
}

// 5b37f3 (client: 581963) - Increment mission stage
void World::ScriptOP_0x01(TriggerAction*) {
    this->mission_state++;
    this->trigger_variables[902]++;
}

// 5b3b11 (client: 581c81) - Send message
void World::ScriptOP_0x02(TriggerAction* action) {
    g_NetStru1_main.SendGameEvent(nullptr, action->data[0], 0);
}

// 5b3b33 (client: 581ca3) - Set value to variable
void World::ScriptOP_0x03(TriggerAction* action) {
    this->trigger_variables[action->data[0]] = action->data[1];
}

// 5b3b79 (client: 581ce9) - Force "Mission Complete" state
void World::ScriptOP_0x04(TriggerAction*) {
    this->mission_complete++;
}

// 5b3b9b (client: 581d0b) - Force "Mission Fail" state
void World::ScriptOP_0x05(TriggerAction* action) {
    this->mission_fail = action->data[0];
}

// 5b3bb7 (client: 581d27) - Group Command
void World::ScriptOP_0x06(TriggerAction* action) {
    Group* group = action->group;
    switch (action->data[0]) {
        case 1: // Guard (guard_range=data[1])
            this->GroupGuardCommand(group, action->data[1]);
            break;
        case 2: // Swarm (x=data[1], y=data[2])
            this->GroupAttackMoveCommand(group, action->data[1], action->data[2]);
            break;
        case 3: // Stand Ground
            this->GroupAggressiveStance(group);
            break;
        case 4: // Move (x=data[1], y=data[2])
            this->GroupMoveCommand(group, action->data[1], action->data[2]);
            break;
        case 5: // Swarm 2 (x=data[1], y=data[2])
            this->GroupSwarmCommand(group, action->data[1], action->data[2]);
            break;
        case 10: // Attack (victim=action->unit)
            this->GroupAttackCommand(group, action->unit);
            break;
        case 11: // Defend (unit=action->unit, follow_range=data[1])
            this->GroupAttackTargetCommand(group, action->unit, action->data[1]);
            break;
        case 14: // Patrol (x=data[1], y=data[2])
            this->GroupPatrolCommand(group, action->data[1], action->data[2]);
            break;
        case 15: // Follow (unit=action->unit, follow_range=data[1])
            this->GroupFollowCommand(group, action->unit, action->data[1]);
            break;
        case 17: // Roam
            this->GroupRoamCommand(group);
            break;
        case 18: // Dwell (guard_range=data[1])
            this->PlaceRespawnedGroup(group, action->data[1]);
            break;
    }
}

// 5b3b56 (client: 581cc6) - Set formation
void World::ScriptOP_0x07(TriggerAction* action) {
    this->SetFormation(action->player, action->data[0]);
}

// 5b3ae1 (client: 581c51) - Increment variable
void World::ScriptOP_0x08(TriggerAction* action) {
    this->trigger_variables[action->data[0]]++;
}

// 5b3d75 (client: 581ee5) - Change diplomacy
void World::ScriptOP_0x0a(TriggerAction* action) {
    uint32_t player1_id = action->data[0];
    uint32_t player2_id = action->data[1];
    uint32_t set_diplomacy = action->data[2];
    this->diplomacy.diplomacy[player1_id][player2_id] = (this->diplomacy.diplomacy[player1_id][player2_id] & ~(DIPLO_WAR | DIPLO_ALLIANCE)) + set_diplomacy;
    for (POSITION pos = this->players_list->list.GetHeadPosition(); pos != nullptr; ) {
        Player* p = this->players_list->list.GetNext(pos);
        if (p->player_id == player1_id || p->player_id == player2_id) {
            g_NetStru1_main.SendVisibilityState(p);
        }
    }
}

// 5b3e56 (client: 581fc6) - Give item from one unit (action->unit) to another (action->multi)
void World::ScriptOP_0x0b(TriggerAction* action) {
    #ifdef A2SERVER_PATCH
    // bug_001B: action->unit can be null - skip the script op
    if (action->unit == nullptr) {
        return;
    }
    #endif
    Item* item = action->unit->inventory->TakeItemById(action->item_id);
    g_NetStru1_main.SendMonsterInventory(action->unit, action->unit->pOwner, 0, 0);
    if (item) {
        Unit* recipient = (Unit*)action->multi;
        recipient->inventory->PutItemIntoBagAtDefault(item);
        g_NetStru1_main.SendMonsterInventory(recipient, recipient->pOwner, 0, 0);
    }
}

// 5b3ed4 (client: 582044) - Add item to unit's inventory
void World::ScriptOP_0x0c(TriggerAction* action) {
    Item* item = g_GameDataRes.CreateItemById(action->item_id);
    if (item) {
        action->unit->inventory->PutItemIntoBagAtDefault(item);
    }
    g_NetStru1_main.SendMonsterInventory(action->unit, action->unit->pOwner, 0, 0);
}

// 5b3f2f (client: 58209f) - Remove item from unit's inventory
void World::ScriptOP_0x0d(TriggerAction* action) {
    Item* item = action->unit->inventory->TakeItemById(action->item_id);
    if (item) {
        delete item;
    }
    g_NetStru1_main.SendMonsterInventory(action->unit, action->unit->pOwner, 0, 0);
}

// 5b408d (client: 5821fd) - kill unit
void World::ScriptOP_0x0e(TriggerAction* action) {
    #ifdef A2SERVER_PATCH
    // bug_001C: action->unit can be null - skip the script op
    if (action->unit == nullptr) {
        return;
    }
    #endif
    g_NetStru1_main.SendHumanoidProps(action->unit, action->unit->pOwner, EQUIP_SLOTS_ALL);
}

// 5b40ba (client: 58222a) - kill unit (same as 0x0e)
void World::ScriptOP_0x0f(TriggerAction* action) {
    #ifdef A2SERVER_PATCH
    // bug_001D: action->unit can be null - skip the script op
    if (action->unit == nullptr) {
        return;
    }
    #endif
    g_NetStru1_main.SendHumanoidProps(action->unit, action->unit->pOwner, EQUIP_SLOTS_ALL);
}

// 5b40e7 (client: 582257) - Phase out unit
void World::ScriptOP_0x10(TriggerAction* action) {
    action->unit->RemoveFromMap();
}

// 5b40ff (client: 58226f) - Phase in unit
void World::ScriptOP_0x11(TriggerAction* action) {
    action->unit->ReturnToMap();
}

// 5b4117 (client: 582287) - Polymorph unit: phase out true form (action->unit), place polymorphed form (action->multi) at same position
void World::ScriptOP_0x12(TriggerAction* action) {
    action->unit->RemoveFromMap();
    Unit* polymorh_as = (Unit*)action->multi;
    polymorh_as->AddToMap(action->unit->position->GetX(), action->unit->position->GetY(), 3);
}

// 5b415a (client: 5822ca) - Change unit's owner
void World::ScriptOP_0x13(TriggerAction* action) {
    g_Server->srv_stru1->TransferUnitOwnership(action->unit, action->player);
}

// 5b41f9 (client: 582369) - Drop all
void World::ScriptOP_0x14(TriggerAction* action) {
    action->unit->CreateSackFromInventory(action->unit->inventory, 0, 0);
    action->unit->inventory = new Inventory();
}

// 5b4287 (client: 5823f7) - Make spell effect: from=(data[0], data[1]), to=(data[2], data[3]), spell=data[4], power=data[5]
void World::ScriptOP_0x15(TriggerAction* action) {
    uint8_t from_x = action->data[0], from_y = action->data[1];
    uint8_t to_x = action->data[2], to_y = action->data[3];
    uint32_t spell_id = action->data[4];

    uint32_t power = action->data[5] ? action->data[5] : 99;

    g_Server->srv_stru1->MakeAreaSpellEffect(from_x, from_y, to_x, to_y, spell_id, power);
}

// 5b4183 (client: 5822f3) - Change group's owner
void World::ScriptOP_0x16(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        g_Server->srv_stru1->TransferUnitOwnership(unit, action->player);
    }
}

// 5b41d8 (client: 582348) - Give money to player
void World::ScriptOP_0x17(TriggerAction* action) {
    action->player->AddMoney(action->data[0], 0);
}

// 5b4309 (client: 582479) - Cast spell to unit: from=(data[0], data[1]), spell=data[2], power=data[3]
void World::ScriptOP_0x18(TriggerAction* action) {
    uint8_t from_x = action->data[0], from_y = action->data[1];
    uint32_t spell_id = action->data[2];
    uint32_t power = action->data[3];
    g_Server->srv_stru1->MakeTargetSpellEffect(from_x, from_y, action->unit, spell_id, power);
}

// 5b4347 (client: 5824b7) - Set trap: spell=data[0], power=data[1], trigger=(data[2],data[3]), source=(data[4],data[5]), target=(gap[0],gap[4])
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
    this->map_stuff->ApplyTerrainMod(trigger_yx, src);
}

// 5b4428 (client: 582598) - Set building's health
void World::ScriptOP_0x1a(TriggerAction* action) {
    action->building->hp = action->data[0];
}

// 5b4446 (client: 5825b6) - Teleport unit
void World::ScriptOP_0x1b(TriggerAction* action) {
    this->map_stuff->TeleportUnit(action->unit, action->data[0], action->data[1]);
    if (action->unit->pOwner->is_ai) {
        this->OrderGuardPosition(action->unit);
    }
}

// 5b4494 (client: 582604) - Give All: transfer all items from `action->unit` to `action->multi`
void World::ScriptOP_0x1c(TriggerAction* action) {
    Unit* target = (Unit*)action->multi;
    target->inventory->TransferFrom(action->unit->inventory);
    action->unit->inventory = new Inventory();
    g_NetStru1_main.SendMonsterInventory(action->unit, action->unit->pOwner, 0, 0);
    g_NetStru1_main.SendMonsterInventory(target, target->pOwner, 0, 0);
}

// 5b455f (client: 5826cf) - Tile Spell Duration: set duration of spell (data[2]) at tile (data[0], data[1])
void World::ScriptOP_0x1d(TriggerAction* action) {
    uint16_t yx = (action->data[1] << 8) | action->data[0];
    AreaEffect** slots = this->map_stuff->GetAreaEffectsAt(yx);
    if (!slots) {
        return;
    }
    for (int i = 0; i < AreaEffect::LAYER_COUNT; i++) {
        if (slots[i] && slots[i]->itemDataID == action->data[2]) {
            slots[i]->duration = action->data[3];
            break;
        }
    }
}

// 5b45e8 (client: 582758) - Unit Spell Duration: update spell_value of effect matching spell ID (data[0]) on unit
void World::ScriptOP_0x1e(TriggerAction* action) {
    for (POSITION pos = action->unit->_effects.GetHeadPosition(); pos != nullptr; ) {
        Effect* effect = action->unit->_effects.GetNext(pos);
        if (effect->itemDataID == action->data[0]) {
            effect->spell_value = action->data[1];
        }
    }
}

// 5b4646 (client: 5827b6) - Repop Delay
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

// 5b46bb (client: 58282b) - Phase Out Group
void World::ScriptOP_0x20(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        unit->RemoveFromMap();
    }
}

// 5b46ff (client: 58286f) - Phase In Group
void World::ScriptOP_0x21(TriggerAction* action) {
    for (POSITION pos = action->group->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = action->group->unit_list.GetNext(pos);
        unit->ReturnToMap();
    }
}

// 5b4743 (client: 5828b3) - Set Unit Parameter
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
    g_NetStru1_main.BroadcastTokenState(action->unit, action->unit->pOwner, 0);
}

// 5b382a (client: 58199a) - Set Scenario Variable
void World::ScriptOP_0x23(TriggerAction* action) {
    ScenarioSetVar(action->data[0], action->data[1]);
}

// 5b39df (client: 581b4f) - Set Subobjective
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
                    g_NetStru1_main.SendGameEvent(nullptr, 0xFD, 0);
                }
                ScenarioSetVar(subobjective_key, 3);
            }
            break;
        case 4:
            g_NetStru1_main.SendGameEvent(nullptr, 0xFE, 0);
            ScenarioSetVar(subobjective_key, 5);
            break;
        default:
            ScenarioSetVar(subobjective_key, action->data[1]);
            break;
    }
}

// 5b3fa7 (client: 582117) - Remove item from everybody's inventory
void World::ScriptOP_0x26(TriggerAction* action) {
    UnitList* list = this->map_stuff->all_units;
    if (!list) {
        return;
    }
    for (POSITION pos = list->unit_list.GetHeadPosition(); pos != nullptr; ) {
        Unit* unit = list->unit_list.GetNext(pos);
        Item* item = unit->inventory->TakeItemById(action->item_id);
        if (item) {
            delete item;
            g_NetStru1_main.SendMonsterInventory(unit, unit->pOwner, 0, 0);
        }
    }
}

// 5b4070 (client: 5821e0) - Suspend group
void World::ScriptOP_0x27(TriggerAction* action) {
    action->group->group_sub->active = 0;
}


// 5a6adb (client: 574c4b) - Get effective max cast range for unit.
uint8_t World::UnitMaxRange(Unit* unit) {
#ifdef A2SERVER_PATCH
    if (unit->eye2->spell_id != 0 && unit->spell_book != nullptr) {
#else
    if (unit->eye2->spell_id != 0) {
#endif
        Spell* spell = unit->spell_book->GetSpell(unit->eye2->spell_id);
        if (spell && spell->mana_cost <= unit->mp) {
            return spell->max_range;
        }
    }
    return unit->max_range;
}

// 5a5e54 (client: 573fc4) - build active/alternate target lists around unit.
void World::BuildTargetLists(Unit* unit) {
    Visibility* visibility = &this->map_stuff->visibility;
    visibility->ClearVisScratch();
    uint8_t* scratch = visibility->vis_scratch;

    uint16_t yx = unit->position->CompatGetYX();
    visibility->RefreshUnitVisibility(unit, yx);

    if (unit->pOwner->is_ai && unit->eye2->position3 != 0) {
        scratch[unit->eye2->position3] += 1;
        unit->eye2->counter2++;
        if (unit->eye2->counter2 > 20) {
            unit->eye2->position3 = 0;
        }
    }

    this->nearby_list.unit_list.RemoveAll();

    UnitList* all = this->map_stuff->all_units;
    if (all != nullptr) {
        POSITION it = all->unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = all->unit_list.GetNext(it);
            uint16_t other_yx = other->position->CompatGetYX();
            if (scratch[other_yx] != 0) {
                this->nearby_list.unit_list.AddTail(other);
            }
        }
    }

    this->FilterByRelation(unit, &this->nearby_list, 0);
    this->PruneUnconsciousTargets();

    if (this->nearby_list.unit_list.GetCount() == 0 && this->unconscious_list.unit_list.GetCount() != 0) {
        this->nearby_list.unit_list.RemoveAll();
        POSITION it = this->unconscious_list.unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = this->unconscious_list.unit_list.GetNext(it);
            this->nearby_list.unit_list.AddTail(other);
        }
        this->unconscious_list.unit_list.RemoveAll();
    }
}

// 5a607b (client: 5741eb) - idle/wander helper. - idle/wander helper: choose the best target from the active/alternate lists.
void World::DoGuardScan(Unit* unit) {
    uint8_t max_range = this->UnitMaxRange(unit);
    uint8_t best_range = max_range + 1;
    uint8_t best_score_diff = 0xFF;
    Unit* best_target = nullptr;

    this->BuildTargetLists(unit);

    POSITION it = this->nearby_list.unit_list.GetHeadPosition();
    while (it != nullptr) {
        Unit* other = this->nearby_list.unit_list.GetNext(it);
        int32_t filter_result = this->GetAttackDecision(unit, other);
        if (filter_result == -1) {
            continue;
        }
        int32_t range = this->map_stuff->GetRangeTo(unit, other);
        if (other->MovementType() == 3 && unit->MovementType() != 3) {
            range += 1;
        }
        uint8_t facing = this->map_stuff->GetFacingTo(unit, other);
        uint8_t score = this->map_stuff->FacingDiff(facing, unit->eye->facing);
        if (range < best_range) {
            best_target = other;
            best_score_diff = score;
            best_range = static_cast<uint8_t>(range);
        } else if (range == best_range && range <= max_range && score <= best_score_diff) {
            best_target = other;
            best_score_diff = score;
        }
    }

    if (max_range < best_range) {
        best_target = nullptr;
    }

    if (best_target == nullptr) {
        best_range = max_range + 1;
        best_score_diff = 0xFF;

        it = this->unconscious_list.unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = this->unconscious_list.unit_list.GetNext(it);
            int32_t range = this->map_stuff->GetRangeTo(unit, other);
            uint8_t facing = this->map_stuff->GetFacingTo(unit, other);
            uint8_t score = this->map_stuff->FacingDiff(facing, unit->eye->facing);
            if (range < best_range) {
                best_target = other;
                best_score_diff = score;
                best_range = static_cast<uint8_t>(range);
            } else if (range == best_range && range <= max_range && score <= best_score_diff) {
                best_target = other;
                best_score_diff = score;
            }
        }
    }

    if (max_range < best_range) {
        best_target = nullptr;
    }

    if (best_target != nullptr) {
        this->PursueTarget(unit, best_target);
    } else {
        this->DoRetreat(unit);
    }

    if (unit->position->IsAtCellCenter() && unit->eye->guard_cell.val != 0) {
        PosYX current_yx = unit->position->CompatGetYX();
        if (unit->eye->guard_cell != current_yx) {
            this->map_stuff->MoveUnitBlockers(unit, PosYX(0), 1);
        }
    }
}

// 5a6b48 (client: 574cb8)
void World::DoAttackTarget(Unit* unit, Unit* target) {
    Unit* chosen_target = target;
    int32_t chosen_spell = 0;

#ifdef A2SERVER_PATCH
    if (target == nullptr || (target->IsUnit() && (target->unit_attrs & Unit::ATTR_PHASED_OUT) != 0)) {
#else
    if ((target->unit_attrs & Unit::ATTR_PHASED_OUT) != 0) {
#endif
        this->SetIdleOrRetreat(unit, 0);
        return;
    }

    for (int32_t i = 0; i < 3; i++) {
        if (unit->eye2->known_spells[i] != 0 && RandInt() < (int32_t)unit->eye2->known_spell_probabilities[i]) {
            chosen_spell = unit->eye2->known_spells[i];
        }
    }

    if (chosen_spell != 0) {
        if (chosen_spell != spell::teleport) {
            this->CastKnownSpell(unit, target, chosen_spell);
            return;
        }

        uint16_t target_yx = target->position->CompatGetYX();
        uint16_t unit_yx = unit->position->CompatGetYX();
        uint8_t distance = this->map_stuff->ChebyshevDistance(unit_yx, target_yx);
        if (distance > 2) {
            this->CastKnownSpell(unit, target, chosen_spell);
            return;
        }
    }

    if (unit == chosen_target) {
        chosen_target = nullptr;
    }

    if (chosen_target == nullptr) {
        uint16_t unit_yx = unit->position->CompatGetYX();
        UnitList* nearby_list = this->GetNearbyEnemies(unit, unit_yx);
        if (nearby_list->unit_list.GetCount() != 0) {
            chosen_target = nearby_list->unit_list.GetHead();
        }
    }

    if (chosen_target == nullptr) {
        this->SetIdleOrRetreat(unit, 0);
    } else if (!unit->pOwner->is_ai) {
        if (!unit->IsMage() || this->ShouldPursueTarget(unit, chosen_target) == 0) {
            unit->eye2->cast_action = 5;
            unit->eye2->unit = chosen_target;
            unit->eye2->max_range = this->UnitMaxRange(unit);
            if (unit->IsMage() && unit->eye2->max_range < 2) {
                this->DoAutobuff(unit);
            }
        }
    } else {
        if (!unit->IsMage() || this->PickAttackSpell(unit, chosen_target) == 0) {
            unit->eye2->cast_action = 5;
            unit->eye2->unit = chosen_target;
            unit->eye2->max_range = this->UnitMaxRange(unit);
        }
    }
}

// Pick a random affordable non-defensive spell from the caster's spellbook and
// start casting it at the provided target. Falls back to DoAttackTarget for units
// with high mind when the random roll is low.
// 5b6632 (client: 5847a2) - AI mage spell selection: pick a random affordable non-defensive spell.
int32_t World::PickAttackSpell(Unit* unit, Unit* target) {
    if (unit->mind >= 60 && this->RandomScaled(100) < 30) {
        this->DoAttackTarget(unit, target);
        return 0;
    }

    CList<Spell*> spells;
    for (int32_t spell_id = 1; spell_id < spell::max_spell_id; spell_id++) {
        Spell* spell = unit->spell_book->GetSpell(spell_id);
        if (spell != nullptr && spell->mana_cost <= unit->mp && !spell->is_defensive) {
            spells.AddTail(spell);
        }
    }

    int32_t index = this->RandomScaled(spells.GetCount());
    POSITION pos = spells.FindIndex(index);
    Spell* chosen = (pos != nullptr) ? spells.GetAt(pos) : nullptr;
    if (chosen == nullptr) {
        return 0;
    }

    unit->eye2->action_done = 1;
    this->BeginAutobuffCast(unit, target, chosen);
    return 1;
}

// Set unit to idle/wander (param=0) or retreat (param!=0).
// 5a6e2c (client: 574f9c) - Set unit to idle/wander (retreat=0) or retreat (retreat!=0).
void World::SetIdleOrRetreat(Unit* unit, int retreat) {
    if (retreat == 0) {
        this->DoGuardScan(unit);
    } else {
        this->DoRetreat(unit);
    }
}

// 5a6e59 (client: 574fc9) - retreat helper.
void World::DoRetreat(Unit* unit) {
    if (unit->pOwner->is_ai) {
        unit->eye2->cast_action = 0x0B;
        if (unit->IsMage()) {
            this->CastSelfHealOrBuff(unit);
        }
    } else {
        this->IdleSelfBuff(unit);
    }
}

// 5a8709 (client: 576879) - set unit to pursue the chosen target.
void World::PursueTarget(Unit* unit, Unit* target) {
    if (unit->IsMage() && !unit->pOwner->is_ai && this->ShouldPursueTarget(unit, target)) {
        return;
    }

    unit->eye2->cast_action = 6;
    unit->eye2->unit = target;
    unit->eye2->max_range = this->UnitMaxRange(unit);
}

// Pick spell by number if the unit knows it and has enough MP.
// 5a79d6 (client: 575b46) - Pick spell by number if unit has enough MP.
Spell* World::GetCastableSpell(Unit* unit, int32_t spell_number, int32_t /*flag*/) {
    Spell* spell = unit->spell_book->GetSpell(spell_number);
    if (spell == nullptr || unit->mp < spell->mana_cost) {
        return nullptr;
    }
    return spell;
}

// autobuff-family hooks (5A7BBD..5A83FE, 11 call sites inside DoAutobuff):
// wraps GetCastableSpell for the autobuff paths only - a bit set in the
// player's persisted autobuff_mask suppresses that spell (AI is exempt).
Spell* World::GetAutobuffSpell(Unit* unit, int32_t spell_number) {
#if defined(A2TANGAR_HAT) && !defined(A2CLIENT)
    if (!unit->pOwner->is_ai) {
        settings::PlayerSettings* ps = settings::Find((const char*)unit->pOwner->name);
        if (ps != nullptr && (ps->autobuff_mask & (1u << spell_number)) != 0) {
            return nullptr;
        }
    }
#endif
    return this->GetCastableSpell(unit, spell_number, 0);
}

// Check whether `other` is within `spell`'s max range from `unit`.
// 5a7af7 (client: 575c67) - Check if `other` is in range of spell from `unit`.
int32_t World::IsSpellInRange(Unit* unit, Unit* other, Spell* spell) {
    PosYX other_yx = other->position->GetYX();
    PosYX unit_yx = unit->position->GetYX();
    uint8_t distance = this->map_stuff->ChebyshevDistance(unit_yx, other_yx);
    return distance <= spell->max_range ? 1 : 0;
}

// Set up a single-target cast action for `unit` on `target` using `spell`.
// 5a92af (client: 57741f)
void World::OrderCastAtUnit(Unit* unit, Unit* target, Spell* spell) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_CAST_UNIT;
    unit->eye2->unit5 = target;
    unit->eye2->spell = spell;
    unit->eye2->max_range = spell->max_range;
    unit->eye2->cast_action = 0;
}

// 5a9832 (client: 5779a2)
void World::PrepareUnitOrder(Unit* unit) {
    if (unit->eye->facing != unit->eye->target_facing) {
        unit->eye->target_facing = unit->eye->facing;
    }

    if (unit->eye->guard_cell != PosYX()) {
        if (unit->position->IsAtCellCenter()) {
            if (unit->position->GetYX() != unit->eye->guard_cell) {
                this->map_stuff->MoveUnitBlockers(unit, PosYX(), 1);
            }
        }
    }
    unit->eye2->max_range = this->UnitMaxRange(unit);
    unit->eye2->action_done = 0;
    unit->eye->target_unit = 0;
    unit->some_state = Unit::EXEC_IDLE;
}

// 5a9ac4 (client: 577c34)
void World::BeginUnitOrder(Unit* unit) {
    this->PrepareUnitOrder(unit);
    unit->eye2->has_command = 0;
}

// 5a93f4 (client: 577564)
void World::OrderHoldPosition(Unit* unit) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_HOLD_POSITION;
    unit->eye2->position1 = unit->position->GetYX();
    unit->eye2->cast_action = 0;
}

// 5a9a6a (client: 577bda) - World callback: register newly summoned/placed unit
void World::ResetUnitToHold(Unit* unit) {
    this->BeginUnitOrder(unit);
    this->OrderHoldPosition(unit);
}

// Reset unit to idle and register it in the world.
// 5a9a8f (client: 577bff) - reset unit to idle.
void World::ResetUnitToIdle(Unit* unit) {
    this->PrepareUnitOrder(unit);
    this->ResetUnitToHold(unit);
    unit->eye2->has_command = 1;
}

// Toggle a unit between state 2 and a full idle reset.
// 5aa8ed (client: 578a5d)
void World::FinishSwarmArrival(Unit* unit) {
    if (unit->some_state == Unit::EXEC_PICKUP) {
        this->ResetUnitToIdle(unit);
    } else {
        unit->some_state = Unit::EXEC_PICKUP;
    }
}

// Set unit to pursue `target`; if not centered in a cell, mark it for movement.
// 5aa91b (client: 578a8b)
void World::StepTowardUnit(Unit* unit, Unit* target) {
    this->map_stuff->MoveStepTowardUnit(unit, target, unit->eye2->max_range);
    if (!unit->position->IsAtCellCenter()) {
        unit->eye2->ai_state = 3;
        unit->eye2->counter = 0;
    }
    unit->some_state = Unit::EXEC_STEP;
}

// Turn unit to face `facing`; if the facing changed, mark state as 1.
// 5aaf84 (client: 5790f4) - turn unit to face target.
void World::TurnToFacing(Unit* unit, uint8_t facing) {
    uint8_t old_facing = unit->eye->facing;
    this->map_stuff->SetFacing(unit, facing);
    if (unit->eye->facing != old_facing) {
        unit->some_state = Unit::EXEC_STEP;
    }
}

// 5acd4c (client: 57aebc)
void World::ClearGroupOrders(Group* grp) {
    for (POSITION it = grp->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = grp->unit_list.GetNext(it);
        this->BeginUnitOrder(unit);
        unit->eye2->gap_0x38 = 0;
    }
    grp->group_sub->order = GroupSub::ORDER_DEFAULT;
}

// 5aca54 (client: 57abc4)
void World::GroupAggressiveStance(Group* grp) {
    this->ClearGroupOrders(grp);
    for (POSITION it = grp->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = grp->unit_list.GetNext(it);
        this->OrderHoldPosition(unit);
    }
    grp->group_sub->order = GroupSub::ORDER_AGGRESSIVE;
}

// 5acdf4 (client: 57af64) - World callback for animate-dead group
void World::ProcessAnimateDeadGroup(Group* group) {
    this->ClearGroupOrders(group);
    this->GroupAggressiveStance(group);
}

// Group move command.
// 5ac289 (client: 57a3f9) - Group move command.
void World::GroupMoveCommand(Group* group, uint8_t x, uint8_t y) {
    this->ClearGroupOrders(group);

    bool use_relative = true;
    int32_t min_speed = 250;
    uint8_t origin_x = 0;
    uint8_t origin_y = 0;

    if (group->owner->settings->formation == 2) {
        uint32_t pos = this->ComputeGroupAvgPos(group);
        origin_x = static_cast<uint8_t>((pos >> 8) & 0xFF);
        origin_y = static_cast<uint8_t>((pos >> 24) & 0xFF);

        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            uint8_t dist = this->map_stuff->ChebyshevDistance(PosYX(unit->position->CompatGetYX()), PosYX(origin_x, origin_y));
            if (dist > this->relative_move_max_dist) {
                use_relative = false;
            }
        }
    } else {
        use_relative = group->owner->settings->formation != 0;
        if (use_relative) {
            uint32_t pos = this->ComputeGroupAvgPos(group);
            origin_x = static_cast<uint8_t>((pos >> 8) & 0xFF);
            origin_y = static_cast<uint8_t>((pos >> 24) & 0xFF);
        }
    }

    this->ClearGroupOrders(group);
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        if (!use_relative) {
            this->OrderMoveTo(unit, x, y);
        } else {
            unit->eye2->x = unit->position->GetX() - origin_x;
            unit->eye2->y = unit->position->GetY() - origin_y;
            this->OrderMoveTo(unit, x + unit->eye2->x, y + unit->eye2->y);
            if (unit->speed < min_speed) {
                min_speed = unit->speed;
            }
        }
    }

    if (use_relative) {
        group->group_sub->speed_override = min_speed;
    }
    group->group_sub->order = GroupSub::ORDER_MOVE;
    group->group_sub->order_pos = PosYX(x, y).val;
}

// Group swarm command. Same as move, but mode is 5.
// 5ac507 (client: 57a677) - Group swarm command.
void World::GroupSwarmCommand(Group* group, uint8_t x, uint8_t y) {
    this->GroupMoveCommand(group, x, y);
    group->group_sub->order = GroupSub::ORDER_SWARM;
}

// 5a457a (client: 5726ea)
void World::Init() {
    this->ReinitState();
}

// 5a4590 (client: 572700)
void World::Destroy() {
    this->BeginDestroy();
}

// 5aa78c (client: 5788fc)
void World::StartAttack(Unit* unit) {
    unit->eye2->ai_state = 1;
    unit->eye2->counter = 0;
    unit->some_state = Unit::EXEC_ATTACK;
    unit->cast_target = unit->eye2->unit;
}

// Refresh group_sub position/range fields from ComputeGroupAvgPos results.
// 5ac137 (client: 57a2a7) - Refresh group_sub position/range fields.
void World::RefreshGroupGuardAnchor(Group* group) {
    this->ComputeGroupAvgPos(group);
    group->group_sub->gap_0x0 = group->group_sub->avg_pos;
    group->group_sub->guard_range = group->group_sub->max_dist_plus_scan;
    group->group_sub->guard_range_base = group->group_sub->max_dist_plus_scan;
}

// Issue a move-to-point command to `group`.
// 5ac785 (client: 57a8f5)
void World::GroupAttackMoveCommand(Group* group, uint8_t x, uint8_t y) {
    this->ClearGroupOrders(group);
    group->group_sub->order = GroupSub::ORDER_ATTACK_MOVE;
    group->group_sub->order_pos = PosYX{x, y}.val;
}

// Issue a respawn/return command to `group` using the lead unit's position.
// 5ac7c8 (client: 57a938)
void World::GroupRoamCommand(Group* group) {
    this->ClearGroupOrders(group);
    group->group_sub->order = GroupSub::ORDER_ROAM;
    Unit* head = group->unit_list.GetHead();
    group->group_sub->order_pos = head->position->CompatGetYX();
    group->group_sub->order_age = 0;
}

// Issue a guard/hold command to `group`.
// range=0 means use the world's MinimalGuardRange.
// 5ac8a2 (client: 57aa12) - Group guard command, range=0 means use world's MinimalGuardRange.
void World::GroupGuardCommand(Group* group, uint8_t range) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderGuardPosition(unit);
    }

    group->group_sub->order = GroupSub::ORDER_GUARD;
    this->RefreshGroupGuardAnchor(group);

    if (range == 0) {
        range = this->MinimalGuardRange;
    }

    if (group->group_sub->guard_range < range) {
        group->group_sub->guard_range = range;
        group->group_sub->guard_range_base = range;
    }

    group->group_sub->gap_0x39 = 0;
}

// 5a9087 (client: 5771f7) - order `unit` to attack `target`.
void World::OrderEngageTarget(Unit* unit, Unit* target) {
    if (unit == target) {
        this->ResetUnitToHold(unit);
    } else {
        this->BeginUnitOrder(unit);
        unit->state = Unit::ORDER_ATTACK_UNIT;
        unit->eye2->unit = target;
        unit->eye2->max_range = this->UnitMaxRange(unit);
        unit->eye2->cast_action = 0;
    }
}

// 5a930f (client: 57747f) - group area-cast setup. - order `unit` to area-cast `spell` at (x, y).
void World::OrderAreaCast(Unit* unit, uint8_t x, uint8_t y, Spell* spell) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_CAST_POS;
    unit->eye2->cast_pos = PosYX(x, y).val;
    unit->eye2->spell = spell;
    unit->eye2->max_range = spell->max_range;
    unit->eye2->cast_action = 0;
}

// 5a943b (client: 5775ab) - helper for GroupStopCommand. - helper used by GroupStopCommand: set unit to retreat/evade state.
void World::OrderStandGround(Unit* unit) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_STAND_GROUND;
    unit->eye2->position1 = unit->position->CompatGetYX();
    unit->eye2->cast_action = 0;
}

// 5a9482 (client: 5775f2) - order `unit` to follow/guard `target` (state 8).
void World::OrderAttackTarget(Unit* unit, Unit* target, uint8_t range) {
    this->BeginUnitOrder(unit);
    if (unit == target) {
        this->ResetUnitToHold(unit);
    } else {
        unit->state = Unit::ORDER_DEFEND_TARGET;
        unit->eye2->unit2 = target;
        unit->eye2->cast_action = 0;
        unit->eye2->range = (range != 0) ? range : 3;
    }
}

// 5a9501 (client: 577671) - order `unit` to attack `target` (state 0x11).
void World::OrderFollowTarget(Unit* unit, Unit* target, uint8_t range) {
    this->BeginUnitOrder(unit);
    if (unit == target) {
        this->ResetUnitToHold(unit);
    } else {
        unit->state = Unit::ORDER_FOLLOW;
        unit->eye2->unit2 = target;
        unit->eye2->cast_action = 0;
        unit->eye2->range = (range != 0) ? range : 3;
    }
}

// 5a9580 (client: 5776f0) - order `unit` to move to (x, y) using path-list.
void World::OrderPatrolTo(Unit* unit, uint8_t x, uint8_t y) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_PATROL_STEP;
    unit->eye2->position1 = unit->position->CompatGetYX();
    unit->eye2->positions_list->RemoveAll();
    unit->eye2->positions_list->AddHead(unit->position->CompatGetYX());
    unit->eye2->positions_list->AddTail(PosYX(x, y).val);
    unit->eye2->position2 = unit->eye2->positions_list->GetTail();
    unit->eye2->patrol_reanchor = 0;
    unit->eye2->cast_action = 0;
    this->map_stuff->visibility.ScanUnitVisibility(unit, unit->position->CompatGetYX());
}

// 5a9918 (client: 577a88) - helper for GroupGoHomeCommand. - helper used by GroupGoHomeCommand: set unit to idle/wander state 0x16.
void World::OrderTeleportHome(Unit* unit) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_RETURN_HOME;
    unit->eye2->cast_action = 0;
}

// 5aa426 (client: 578596) - turn unit to face `facing`; if already facing, clear cast action.
void World::DoTurnTo(Unit* unit, uint8_t facing) {
    if (unit->eye->facing != facing) {
        this->map_stuff->SetFacing(unit, facing);
        unit->some_state = Unit::EXEC_STEP;
    } else {
        unit->eye2->cast_action = 0;
        unit->some_state = Unit::EXEC_IDLE;
    }
}

// 5aa7cf (client: 57893f) - commit a targeted spell cast from eye2 setup.
void World::StartCastAtUnit(Unit* unit) {
    unit->eye2->ai_state = 2;
    unit->eye2->counter = 0;
    unit->some_state = Unit::EXEC_CAST_UNIT;
    unit->cast_target = unit->eye2->unit5;
    unit->some_spell = unit->eye2->spell;
    unit->eye2->cast_is_area = 1;
    if (unit->eye2->action_done == 0) {
        this->ResetUnitToIdle(unit);
    }
}

// 5aa84f (client: 5789bf) - commit an area spell cast from eye2 setup.
void World::StartAreaCast(Unit* unit) {
    unit->eye2->ai_state = 2;
    unit->eye2->counter = 0;
    unit->some_state = Unit::EXEC_CAST_POS;
    unit->area_cast_x = PosYX(unit->eye2->cast_pos).x;
    unit->area_cast_y = PosYX(unit->eye2->cast_pos).y;
    unit->some_spell = unit->eye2->spell;
    unit->eye2->cast_is_area = 0;
    if (unit->eye2->action_done == 0) {
        this->ResetUnitToIdle(unit);
    }
}

// Order a unit to interact with/enter a building.
// 5a90f4 (client: 577264)
void World::OrderEnterBuilding(Unit* unit, Building* building) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_ENTER_BUILDING;
    unit->eye2->enter_building = building;
    unit->eye2->max_range = 1;
    unit->eye2->cast_action = 0;

    PosYX command_to = building->position->CompatGetYX();
    unit->eye2->command_to = command_to.val;

    switch (building->width) {
    case 1: case 2:
        break;
    case 3: case 4:
        unit->eye2->max_range = 2;
        unit->eye2->command_to = PosYX(command_to.x + 1, command_to.y + 1).val;
        break;
    default:
        unit->eye2->max_range = 3;
        unit->eye2->command_to = PosYX(command_to.x + 2, command_to.y + 2).val;
    }
}

// 5ac881 (client: 57a9f1)
void World::GroupAttackMove(Group* group, uint8_t x, uint8_t y) {
    this->GroupSwarmCommand(group, x, y);
}

// 5af683 (client: 57d7f3)
void World::SetFormation(Player* player, uint8_t formation) {
    player->settings->formation = formation;
}

// 5afbfd (client: 57dd6d) - increments joined_count
void World::IncrementJoinedCount() {
    this->joined_count++;
}

// 5b2e7a (client: 580fea) - Post-load world relink helper
void World::RelinkPointers() {
    this->diplomacy.world = this;
}

// Serialize/Deserialize world state.
// 5b0556 (client: 57e6c6) - Serialize
void World::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar.Write(this->trigger_variables, sizeof(this->trigger_variables));
        ar.Write(this->trigger_results, sizeof(this->trigger_results));
        ar.Write(&this->perf_tick, sizeof(this->perf_tick));
        ar.Write(this->saved_0xa728, sizeof(this->saved_0xa728));
        ar.Write(&this->diplomacy, sizeof(this->diplomacy));
        ar << this->saved_0xa48;
        ar << this->saved_0xa49;
        ar << this->counter;
        ar << this->mission_complete;
        ar << this->saved_0xbe10;
        ar << this->mission_fail;
    } else {
        ar.Read(this->trigger_variables, sizeof(this->trigger_variables));
        ar.Read(this->trigger_results, sizeof(this->trigger_results));
        ar.Read(&this->perf_tick, sizeof(this->perf_tick));
        ar.Read(this->saved_0xa728, sizeof(this->saved_0xa728));
        ar.Read(&this->diplomacy, sizeof(this->diplomacy));
        ar >> this->saved_0xa48;
        ar >> this->saved_0xa49;
        ar >> this->counter;
        ar >> this->mission_complete;
        ar >> this->saved_0xbe10;
        ar >> this->mission_fail;
    }
}

// 5b6f30 (client: 584ff0) - rand() wrapper.
int32_t RandInt() {
    return std::rand();
}

// 5b6f40 (client: 585000)
int32_t World::RandomScaled(int32_t multiplier) {
    return (RandInt() * multiplier) / this->gap_0x0;
}

// 5b6f60 (client: 585020)
int32_t World::RandomRange(int32_t min, int32_t max) {
    return (RandInt() * ((max - min) + 1)) / this->gap_0x0 + min;
}

// 5a8efb (client: 57706b) - Set unit movement target, clamped to map bounds.
void World::OrderMoveTo(Unit* unit, uint8_t x, uint8_t y) {
    this->BeginUnitOrder(unit);

    if (x < this->map_stuff->map_min_x) {
        x = this->map_stuff->map_min_x;
    }
    if (x > this->map_stuff->map_max_x) {
        x = this->map_stuff->map_max_x;
    }
    if (y < this->map_stuff->map_min_y) {
        y = this->map_stuff->map_min_y;
    }
    if (y > this->map_stuff->map_max_y) {
        y = this->map_stuff->map_max_y;
    }

    unit->state = Unit::ORDER_MOVE;
    unit->eye2->command_to = PosYX(x, y).val;
    unit->eye->target_reached = 0;
    unit->eye2->cast_action = 1;
}

// 5aa375 (client: 5784e5) - set a unit to move/cast toward `yx`.
void World::DoMoveStepTo(Unit* unit, PosYX yx, uint8_t max_range) {
    unit->eye->moving_to = 1;

    if (unit->eye->static_path_cell.val != yx.val) {
        unit->eye->static_age = 0xFF;
    }

    this->map_stuff->MoveStepTowardPos(unit, yx, max_range);

    if (unit->position->IsAtCellCenter() == 0) {
        unit->eye2->ai_state = 3;
        unit->eye2->counter = 0;
    }

    unit->some_state = Unit::EXEC_STEP;
}

// 5aa485 (client: 5785f5) - pick a new random wander facing for a monster unit.
void World::DoIdleFidget(Unit* unit) {
    if (unit->eye->facing == unit->eye->target_facing) {
        if (unit->eye2->engaged == 0 && RandInt() >= 205) {
            unit->some_state = Unit::EXEC_IDLE;
            return;
        }

        uint8_t new_facing = unit->eye->facing + 33 + this->RandomScaled(190);
        unit->eye->target_facing = new_facing;
        unit->eye2->engaged = 0;
    }

    this->map_stuff->SetFacing(unit, unit->eye->target_facing);
    unit->some_state = Unit::EXEC_STEP;
}

// 5aab85 (client: 578cf5) - resume moving toward the unit's current command target.
void World::ApproachBuilding(Unit* unit_same, Unit* unit) {
    int result = this->map_stuff->FacesPosInRange(unit, unit->eye2->command_to, unit->eye2->max_range);
    if (result != 0) {
        if (unit->some_state == Unit::EXEC_INTERACT_BUILDING) {
            this->ResetUnitToIdle(unit);
        } else {
            unit->some_state = Unit::EXEC_INTERACT_BUILDING;
        }
    } else {
        this->DoMoveStepTo(unit, unit->eye2->command_to, unit->eye2->max_range);
        unit->some_state = Unit::EXEC_STEP;
    }
}

// 5aaebc (client: 57902c) - start a melee/cast action against the unit's current target.
void World::TickRangedApproach(Unit* unit) {
    if (unit->IsMage()) {
        uint8_t max_range = this->UnitMaxRange(unit);
        if (max_range < 2 && !unit->pOwner->is_ai) {
            unit->eye2->cast_action = 0;
            return;
        }
    }

    if (this->map_stuff->FacesTargetInRange(unit, unit->eye2->unit, this->UnitMaxRange(unit))) {
        this->StartAttack(unit);
        unit->some_state = Unit::EXEC_ATTACK;
    } else {
        uint8_t facing = this->map_stuff->GetFacingTo(unit, unit->eye2->unit);
        this->TurnToFacing(unit, facing);
        unit->some_state = Unit::EXEC_STEP;
    }
}

// 5a3c5f (client: 571dcf) - move unconscious targets out of nearby_list into unconscious_list. - move unconscious targets out of the active target list into the alternate list.
void World::PruneUnconsciousTargets() {
    this->unconscious_list.unit_list.RemoveAll();

    POSITION it = this->nearby_list.unit_list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current = it;
        Unit* unit = this->nearby_list.unit_list.GetNext(it);
        if (unit->hp < 1) {
            this->nearby_list.unit_list.RemoveAt(current);
            this->unconscious_list.AddTail(unit);
        }
    }
}

// Filter a UnitList by movement type for short-range units.
// Removes units whose movement type is 3 (flyers?) when `unit` has a max range below 2.
// 5a3dc9 (client: 571f39) - helper for GetNearbyUnits.
void World::RemoveAirborneTargets(Unit* unit, UnitList* list) {
    if (list == nullptr) {
        return;
    }

    uint8_t max_range = this->UnitMaxRange(unit);
    POSITION it = list->unit_list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current = it;
        Unit* other = list->unit_list.GetNext(it);

        if (max_range < 2 && other->MovementType() == 3) {
            list->unit_list.RemoveAt(current);
        }
    }
}

// 5ac206 (client: 57a376) - order a group to area-cast a spell at (x, y).
void World::GroupCastAtPositionCommand(Group* group, uint8_t x, uint8_t y, Spell* spell) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        if (unit->spell == nullptr) {
            this->ResetUnitToHold(unit);
        } else {
            this->OrderAreaCast(unit, x, y, spell);
        }
        unit->spell = nullptr;
    }

    group->group_sub->order = GroupSub::ORDER_DEFAULT;
}

// 5ac983 (client: 57aaf3) - Place a newly respawned group into the world.
void World::PlaceRespawnedGroup(Group* group, int guard_range) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderGuardPosition(unit);
    }

    group->group_sub->order = GroupSub::ORDER_DEFEND;
    this->RefreshGroupGuardAnchor(group);

    if (guard_range == 0) {
        if (group->group_sub->guard_range < this->MinimalGuardRange) {
            group->group_sub->guard_range = 20;
            group->group_sub->guard_range_base = 20;
        }
    } else {
        if (group->group_sub->guard_range < guard_range) {
            group->group_sub->guard_range = static_cast<uint8_t>(guard_range);
            group->group_sub->guard_range_base = static_cast<uint8_t>(guard_range);
        }
    }

    group->group_sub->gap_0x39 = 0;
}

// 5af6f5 (client: 57d865) - initialize combat withdrawal thresholds for a unit.
void World::InitUnitCombatProfile(Unit* unit) {
    if (unit->IsHumanoid() == 0) {
        return;
    }

    if (unit->spell_book != nullptr) {
        unit->eye2->wimpy_level = 2;
        unit->eye2->withdraw = unit->hp_max;
        unit->eye2->wimpy = unit->hp_max / 4;
    } else {
        uint8_t max_range = this->UnitMaxRange(unit);
        if (max_range < 2 || unit->MovementType() == 3) {
            unit->eye2->wimpy_level = 0;
            unit->eye2->withdraw = 0;
            unit->eye2->wimpy = 0;
        } else {
            unit->eye2->wimpy_level = 1;
            unit->eye2->withdraw = unit->hp_max / 2;
            unit->eye2->wimpy = 0;
        }
    }
}

// 5a9383 (client: 5774f3)
void World::OrderGuardPosition(Unit* unit) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_GUARD_POSITION;
    if (unit->position->IsAtCellCenter() == 0) {
        unit->eye2->position1 = unit->eye->position1;
    } else {
        unit->eye2->position1 = unit->position->GetYX();
    }
    unit->eye2->cast_action = 0;
}

// 5aa97e (client: 578aee)
void World::ChaseOrAttack(Unit* unit) {
    if (this->map_stuff->FacesTargetInRange(unit, unit->eye2->unit, this->UnitMaxRange(unit)) == 0) {
        this->StepTowardUnit(unit, unit->eye2->unit);
        unit->some_state = Unit::EXEC_STEP;
    } else {
        this->StartAttack(unit);
        unit->some_state = Unit::EXEC_ATTACK;
    }
}

// 5aa9f6 (client: 578b66)
void World::CheckMoveArrival(Unit* unit) {
    if (unit->position->CompatGetYX() == unit->eye2->command_to && unit->position->IsAtCellCenter() != 0) {
        if (unit->some_state == Unit::EXEC_PICKUP) {
            unit->some_state = Unit::EXEC_IDLE;
            unit->eye2->cast_action = 0;
        } else {
            unit->some_state = Unit::EXEC_PICKUP;
        }
    } else {
        this->DoMoveStepTo(unit, unit->eye2->command_to, 0);
        unit->some_state = Unit::EXEC_STEP;
    }
}

// 5aab08 (client: 578c78)
void World::ChaseOrAreaCast(Unit* unit) {
    if (this->map_stuff->FacesPosInRange(unit, PosYX(unit->eye2->cast_pos), unit->eye2->max_range) == 0) {
        this->DoMoveStepTo(unit, PosYX(unit->eye2->cast_pos), unit->eye2->max_range);
        unit->some_state = Unit::EXEC_STEP;
    } else {
        this->StartAreaCast(unit);
    }
}

// 5abf50 (client: 57a0c0)
void World::ProcessPlayerGroupAi(Player* player) {
    POSITION it = player->group_list->groups.GetHeadPosition();
    while (it != nullptr) {
        Group* group = player->group_list->groups.GetNext(it);
        if (this->skip_presence_grid != 0) {
            this->ProcessGroupAi(group);
        } else if (group->group_sub->presence_detected != 0) {
            this->ProcessGroupAi(group);
        }
    }
}

// 5abfbc (client: 57a12c)
void World::ProcessGroupAi(Group* group) {
    this->FlagEmptyGroup(group);
    uint8_t cmd = group->group_sub->order;
    if (cmd == GroupSub::ORDER_IDLE_ALL) {
        this->IdleGroupUnits(group);
    } else {
        switch (cmd) {
        case GroupSub::ORDER_DEFAULT:
            for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
                Unit* unit = group->unit_list.GetNext(it);
                this->RunUnitAi(unit);
            }
            break;
        case GroupSub::ORDER_GUARD:
            this->AiGuardBrain(group);
            break;
        case GroupSub::ORDER_ATTACK_MOVE:
            this->AiAttackBrain(group, group->group_sub->order_pos);
            break;
        case GroupSub::ORDER_AGGRESSIVE:
            this->AiAggroBrain(group);
            break;
        case GroupSub::ORDER_MOVE:
            this->AiMoveBrain(group, group->group_sub->order_pos);
            break;
        case GroupSub::ORDER_SWARM:
            this->AiSwarmBrain(group, group->group_sub->order_pos);
            break;
        case GroupSub::ORDER_ROAM:
            this->AiRoamBrain(group);
            break;
        case GroupSub::ORDER_DEFEND:
            this->AiDefendBrain(group);
            break;
        case GroupSub::ORDER_STAND:
            this->AiStandBrain(group);
            break;
        }
    }
    this->ProcessGroupRecall(group);
}

// 5aed8d (client: 57cefd)
void World::AiRoamBrain(Group* group) {
    uint8_t max_distance = 0;
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        uint8_t distance = this->map_stuff->ChebyshevDistance(unit->position->GetYX(), PosYX(group->group_sub->order_pos));
        if (distance > max_distance) {
            max_distance = distance;
        }
    }

    if (max_distance < 10 || group->group_sub->order_age > 50) {
        bool retry = true;
        uint8_t new_x = 0;
        uint8_t new_y = 0;
        while (retry) {
            int32_t idx = this->RandomRange(0, 7);
            int32_t dx = this->map_stuff->neighbor_dx[idx] * 20;
            int32_t dy = this->map_stuff->neighbor_dy[idx] * 20;
            new_x = group->group_sub->order_pos + dx;
            new_y = (group->group_sub->order_pos >> 8) + dy;
            retry = (new_x < this->map_stuff->map_min_x || new_x > this->map_stuff->map_max_x || new_y < this->map_stuff->map_min_y || new_y > this->map_stuff->map_max_y);
        }
        group->group_sub->order_pos = PosYX{new_x, new_y}.val;
        group->group_sub->order_age = 0;
    }

    this->AiSwarmBrain(group, group->group_sub->order_pos);
    group->group_sub->order_age++;
}

// 5a6ea5 (client: 575015) - idle/wander helper.
void World::IdleSelfBuff(Unit* unit) {
    if (unit->IsMage()) {
        this->DoAutobuff(unit);
    } else {
        unit->eye2->cast_action = 0;
    }
}

// 5ab35b (client: 5794cb)
void World::DoScriptedPatrol(Unit* unit) {
    uint8_t v14 = this->waypoint_table[1];
    bool nearby = false;

    if (unit->typeId == unit_type::npc_druid) {
        this->patrol_unit = unit;
        unit->scan_range = (unit->scan_range & 0x00FF) | 0x0500;
        unit->max_range = 5;
        this->DoGuardScan(unit);
        if (unit->eye2->cast_action == 6) {
            this->patrol_counter++;
        }
        if (this->patrol_counter != 0) {
            this->patrol_counter++;
        }
    } else if (this->patrol_counter == 0) {
        if (v14 != 0) {
            uint8_t y2 = this->waypoint_table[v14 * 2 + 1];
            uint8_t x2 = this->waypoint_table[v14 * 2];
            uint8_t y1 = unit->position->GetY();
            uint8_t x1 = unit->position->GetX();
            uint8_t dist = this->map_stuff->ChebyshevDistanceXY(x1, y1, x2, y2);
            if (dist < 5) {
                nearby = true;
                if (unit->eye2->ai_state != 0xFF) {
                    uint16_t pos = PosYX{x2, y2}.val;
                    this->DoMoveStep(unit, pos);
                }
            }
        }
        if (!nearby) {
            if (unit->eye2->ai_state == 0xFF) {
                unit->eye2->ai_state = 0;
            }
            if (unit->eye2->position1.val == 0) {
                unit->eye2->position1 = unit->position->GetYX();
            }
            this->AdvancePatrol(unit);
        }
    } else {
        if (unit->eye2->ai_state == 0xFF) {
            unit->eye2->ai_state = 0;
        }
        if (unit->eye2->positions_list->IsEmpty()) {
            uint8_t x = this->RandomRange(77, 87);
            uint8_t y = this->RandomRange(45, 55);
            PosYX pos = PosYX{x, y};
            unit->eye2->position1 = pos;
            unit->eye2->positions_list->AddTail(pos.val);
        }
        this->AdvancePatrol(unit);
    }
}

// 5a8778 (client: 5768e8)
void World::DoMoveStep(Unit* unit, uint16_t target_yx) {
    unit->eye2->cast_action = 1;
    unit->eye2->command_to = target_yx;
}

// 5a57e8 (client: 573958)
void World::PickHealSpell(Unit* unit, Unit* target) {
    int32_t spell_id = 0;
    bool did_cast = false;

    if (unit->mp2 == 0) {
        if (target->hp < target->hp_max) {
            spell_id = spell::heal;
        }
    } else if (unit->mp2 < unit->mp_max + 3 && target->hp < target->hp_max / 2) {
        spell_id = spell::heal;
    }

    if (spell_id != 0) {
        Spell* spell = unit->spell_book->GetSpell(spell_id);
        if (spell != nullptr && spell->mana_cost <= unit->mp) {
            unit->eye2->action_done = 1;
            this->BeginAutobuffCast(unit, target, spell);
            did_cast = true;
        }
    }

    if (!did_cast) {
        this->PickAdjacentEnemy(unit, target);
    }
}

// 5a65bb (client: 57472b) - pick a nearby target for `unit` from units around `target`.
void World::PickAdjacentEnemy(Unit* unit, Unit* target) {
    UnitList* list = this->GetNearbyUnits(target);
    if (list == nullptr || list->unit_list.GetCount() == 0) {
        this->SetIdleOrRetreat(unit, 0);
        return;
    }

    Unit* best_type3 = nullptr;
    uint8_t best_distance = 0;

    POSITION pos = list->unit_list.GetHeadPosition();
    while (pos != nullptr) {
        Unit* other = list->unit_list.GetNext(pos);
        if (other->MovementType() == 3) {
            if (best_type3 == nullptr) {
                best_type3 = other;
                best_distance = this->map_stuff->GetRangeTo(unit, other);
            } else {
                uint8_t distance = this->map_stuff->GetRangeTo(unit, other);
                if (distance < best_distance) {
                    best_type3 = other;
                    best_distance = distance;
                }
            }
        }
    }

    if (best_type3 != nullptr) {
        this->DoAttackTarget(unit, best_type3);
        return;
    }

    Unit* best_any = nullptr;
    best_distance = 0xFF;

    pos = list->unit_list.GetHeadPosition();
    while (pos != nullptr) {
        Unit* other = list->unit_list.GetNext(pos);
        uint8_t distance = this->map_stuff->GetRangeTo(unit, other);
        if (distance < best_distance) {
            best_any = other;
            best_distance = distance;
        }
    }

    if (best_any != nullptr) {
        this->DoAttackTarget(unit, best_any);
    }
}

// 5a5568 (client: 5736d8)
void World::TickAttackTarget(Unit* unit, Unit* target) {
    uint8_t range = unit->eye2->range;
    uint8_t distance = this->map_stuff->ChebyshevDistanceXY(
        unit->position->GetX(),
        unit->position->GetY(),
        target->position->GetX(),
        target->position->GetY()
    );

    if (range < distance) {
        this->BeginChase(unit, target, range);
    } else {
        if (unit->spell_book != nullptr) {
            this->PickHealSpell(unit, target);
        } else {
            this->PickAdjacentEnemy(unit, target);
        }

        uint8_t cast_action = unit->eye2->cast_action;
        if (cast_action != 5 && cast_action != 6 && cast_action != 8 && cast_action != 9) {
            if (distance < 2) {
                uint32_t packed_yyxx = (target->position->GetY() << 24) | (target->position->GetX() << 8);
                int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, range);
                this->DoMoveStep(unit, result);
            }
        }
    }
}

// 5a56d7 (client: 573847)
void World::TickFollowTarget(Unit* unit, Unit* target) {
    uint8_t range = unit->eye2->range;
    uint8_t distance = this->map_stuff->ChebyshevDistanceXY(
        unit->position->GetX(),
        unit->position->GetY(),
        target->position->GetX(),
        target->position->GetY()
    );

    if (range < distance) {
        this->BeginChase(unit, target, range);
    } else if (distance < 2) {
        uint32_t packed_yyxx = (target->position->GetY() << 24) | (target->position->GetX() << 8);
        int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, range);
        this->DoMoveStep(unit, result);
    } else {
        this->SetIdleOrRetreat(unit, 0);
    }
}

// 5a8885 (client: 5769f5)
void World::DoTeleportHome(Unit* unit) {
    if (unit->spell_book != nullptr) {
        if (this->GetCastableSpell(unit, spell::teleport, 0) != nullptr) {
            this->TeleportToSafety(unit);
            return;
        }
    }

    uint8_t range = unit->eye->scan_radius_base + 5;
    PosYX pos = unit->position->GetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(pos, range);
    this->FilterByRelation(unit, list, 0);

    uint8_t count = 0;
    if (list != nullptr) {
        POSITION it = list->unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = list->unit_list.GetNext(it);
            if (other->hp > 0) {
                count++;
            }
        }
    }

    if (count == 0) {
        this->SetIdleOrRetreat(unit, 0);
    } else {
        uint32_t packed_yyxx = this->ComputeListAvgPos(list);
        int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, 3);
        this->DoMoveStep(unit, result);
    }
}

// 5a8a55 (client: 576bc5)
void World::TeleportToSafety(Unit* unit) {
    uint8_t range = (unit->scan_range >> 8) + 1;
    PosYX pos = unit->position->GetYX();
    UnitList* list = this->GetNearbyEnemies(unit, pos);

    uint8_t count = 0;
    if (list != nullptr) {
        POSITION it = list->unit_list.GetHeadPosition();
        while (it != nullptr) {
            Unit* other = list->unit_list.GetNext(it);
            if (other->hp > 0) {
                count++;
            }
        }
    }

    if (count == 0) {
        this->SetIdleOrRetreat(unit, 0);
    } else {
        uint32_t packed_yyxx = this->ComputeListAvgPos(list);
        int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, range);

        Spell* spell = this->GetCastableSpell(unit, spell::teleport, 0);
        if (spell != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginPositionCast(unit, result, spell);
        } else {
            spell = this->GetCastableSpell(unit, spell::invisibility, 0);
            // WAT: enchantments & 0xC is a check for whether the unit under a "fireball" and "fire wall" spells, which are not enchantments.
            // Should this be "1 << 0xc" instead?
            if (spell != nullptr && (unit->enchantments & 0xC) == 0) {
                unit->eye2->action_done = 1;
                this->BeginAutobuffCast(unit, unit, spell);
            } else {
                this->DoMoveStep(unit, result);
            }
        }
    }
}

// 5a8dbc (client: 576f2c) - helper for FleeFromEnemies.
void World::TeleportAwayFromEnemies(Unit* unit) {
    uint8_t range = (unit->scan_range >> 8) - 1;
    PosYX pos = unit->position->GetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(pos, 2);
    this->FilterByRelation(unit, list, 0);
    if (list->unit_list.GetCount() == 0) {
        this->DoGuardScan(unit);
    } else {
        uint32_t packed_yyxx = this->ComputeListAvgPos(list);
        int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, range);
        Spell* spell = this->GetCastableSpell(unit, spell::teleport, 0);
        if (spell != nullptr) {
            unit->eye2->action_done = 1;
            this->BeginPositionCast(unit, result, spell);
        } else {
            spell = this->GetCastableSpell(unit, spell::invisibility, 0);
            // WAT: enchantments & 0xC makes no sense. Was it supposed to be (1 << 0xC)?
            if (spell != nullptr && (unit->enchantments & 0xC) == 0) {
                unit->eye2->action_done = 1;
                this->BeginAutobuffCast(unit, unit, spell);
            } else {
                this->DoMoveStep(unit, result);
            }
        }
    }
}

// Check whether the unit is below its wimpy threshold and has nearby enemies.
// Returns 1 if the unit should flee, 0 otherwise.
// 5a89d5 (client: 576b45) - helper for ProcessUnitRecall.
int32_t World::WantsRetreat(Unit* unit) {
    if (unit->hp > unit->eye2->wimpy) {
        return 0;
    }

    uint8_t range = unit->eye->scan_radius_base + 5;
    PosYX yx = unit->position->CompatGetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, range);
    this->FilterByRelation(unit, list, 0);
    return list->unit_list.GetCount() != 0 ? 1 : 0;
}

// Check whether the unit is below its withdraw threshold and has nearby enemies.
// Returns 1 if the unit should teleport away, 0 otherwise.
// 5a8bda (client: 576d4a) - helper for ProcessUnitRecall.
int32_t World::MustFlee(Unit* unit) {
    if (unit->hp > unit->eye2->withdraw) {
        return 0;
    }

    PosYX yx = unit->position->CompatGetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(yx, 2);
    this->FilterByRelation(unit, list, 0);
    return list->unit_list.GetCount() != 0 ? 1 : 0;
}

// 5a8cd8 (client: 576e48) - helper for ProcessUnitRecall.
void World::FleeFromEnemies(Unit* unit) {
    if (unit->spell_book != nullptr) {
        if (this->GetCastableSpell(unit, spell::teleport, 0) != nullptr) {
            this->TeleportAwayFromEnemies(unit);
            return;
        }
    }

    PosYX pos = unit->position->GetYX();
    UnitList* list = this->map_stuff->BuildNearbyUnitList(pos, 2);
    this->FilterByRelation(unit, list, 0);
    if (list->unit_list.GetCount() == 0) {
        this->DoGuardScan(unit);
    } else {
        uint32_t packed_yyxx = this->ComputeListAvgPos(list);
        int16_t result = this->map_stuff->ClampPosToMap(unit, packed_yyxx, 3);
        this->DoMoveStep(unit, result);
    }
}

// 5a59e2 (client: 573b52)
void World::DoPatrolTick(Unit* unit) {
    unit->eye2->cast_action = 0;
    if (unit->eye2->patrol_reanchor != 0) {
        unit->eye2->position1 = unit->position->CompatGetYX();
        unit->eye2->patrol_reanchor = 0;
    }
    this->AdvancePatrol(unit);
    if (unit->eye2->cast_action == 0 || unit->eye2->cast_action == 0xB) {
        unit->eye2->patrol_reanchor = 1;
        uint16_t pos = unit->eye2->position2.val;
        if (unit->position->CompatGetYX() == pos) {
            CList<uint16_t>* positions = unit->eye2->positions_list;
            POSITION found = positions->Find(pos, NULL);
            if (found == NULL) {
                pos = positions->GetHead();
            } else {
                positions->GetNext(found);
                if (found == NULL) {
                    pos = positions->GetHead();
                } else {
                    pos = positions->GetAt(found);
                }
            }
        }
        unit->eye2->position2 = pos;
        this->DoMoveStep(unit, pos);
    }
}

// 5a5b49 (client: 573cb9)
void World::AdvancePatrolStep(Unit* unit) {
    this->AdvancePatrol(unit);
}

// 5a647c (client: 5745ec) - helper for state 10/24 unit actions.
void World::AdvancePatrol(Unit* unit) {
    if (unit->eye2->position1.val == 0) {
        unit->eye2->position1 = unit->position->GetYX();
    }

    uint8_t y2 = unit->eye2->position1.y;
    uint8_t x2 = unit->eye2->position1.x;
    uint8_t y1 = unit->position->GetY();
    uint8_t x1 = unit->position->GetX();
    uint8_t dist = this->map_stuff->ChebyshevDistanceXY(x1, y1, x2, y2);
    if (dist >= (unit->scan_range >> 8)) {
        this->DoMoveStep(unit, unit->eye2->position1.val);
    }

    UnitList* list = this->GetNearbyEnemies(unit, unit->eye2->position1);
    if (list->unit_list.GetCount() != 0) {
        this->DoAttackTarget(unit, list->unit_list.GetHead());
    } else {
        PosYX current_pos = unit->position->GetYX();
        if (current_pos.val == unit->eye2->position1.val) {
            this->SetIdleOrRetreat(unit, 0);
        } else {
            this->DoMoveStep(unit, unit->eye2->position1.val);
        }
    }
}

// 5a6da3 (client: 574f13)
void World::BeginEnterBuilding(Unit* unit, Building* building) {
    unit->eye2->cast_action = 0xF;
    unit->eye2->enter_building = building;
}

// 5a551c (client: 57368c) - set up a position-targeted cast action.
void World::BeginPositionCast(Unit* unit, uint16_t pos_yx, Spell* spell) {
    unit->eye2->cast_action = 9;
    unit->eye2->cast_pos = pos_yx;
    unit->eye2->spell = spell;
    unit->eye2->max_range = spell->max_range;
}

// Build a temporary UnitList of units near `unit` using the unit's eye range.
// 5a3791 (client: 571901) - helper for GetNearbyUnits.
UnitList* World::BuildNearbyList(Unit* unit) {
    uint8_t range = unit->eye->scan_radius_base;
    PosYX yx = unit->position->CompatGetYX();
    return this->map_stuff->BuildNearbyUnitList(yx, range);
}

// 5a37c5 (client: 571935) - get nearby UnitList around unit.
UnitList* World::GetNearbyUnits(Unit* unit) {
    UnitList* list = this->BuildNearbyList(unit);
    this->FilterByRelation(unit, list, 0);
    this->RemoveAirborneTargets(unit, list);
    return list;
}

// 5a54b8 (client: 573628)
void World::DoAttackMove(Unit* unit, uint16_t command_to) {
    UnitList* list = this->GetNearbyEnemies(unit, unit->position->GetYX());
    if (list != nullptr && list->unit_list.GetCount() != 0) {
        this->DoAttackTarget(unit, list->unit_list.GetHead());
    } else {
        this->DoMoveStep(unit, command_to);
    }
}

// 5a87a3 (client: 576913)
void World::DoSwarmMoveStep(Unit* unit, uint16_t pos_yx) {
    if (pos_yx != unit->position->CompatGetYX()) {
        this->DoMoveStep(unit, pos_yx);
    } else {
        unit->eye2->cast_action = 7;
        unit->eye2->command_to = pos_yx;
    }
}

// 5a8829 (client: 576999)
void World::BeginChase(Unit* unit, Unit* target, uint8_t range) {
    unit->eye2->cast_action = 4;
    unit->eye2->unit3 = target;
    if (range != 0) {
        unit->eye2->max_range = range;
    } else {
        unit->eye2->max_range = unit->max_range;
    }
}

// 5a6789 (client: 5748f9)
void World::DoGatherLootStep(Unit* unit) {
    this->map_stuff->CollectSackCells(unit, 1);
    if (this->map_stuff->sack_cells.GetCount() == 0) {
        this->OrderHoldPosition(unit);
    } else {
        unit->eye2->command_to = this->map_stuff->sack_cells.GetHead();
        unit->eye2->cast_action = 0xC;
    }
}

// Reset unit state and set a movement target from two packed bytes.
// 5a925c (client: 5773cc) - helper for DoScriptedWalk.
void World::OrderWalkTo(Unit* unit, uint8_t a, uint8_t b) {
    this->BeginUnitOrder(unit);
    unit->state = Unit::ORDER_ATTACK_MOVE;
    unit->eye2->command_to = PosYX{a, b}.val;
    unit->eye2->cast_action = 0;
}

// 5ab5e3 (client: 579753)
void World::DoScriptedWalk(Unit* unit) {
    this->DoMoveStep(unit, 0x4F5B);
    if (unit->position->GetX() > 0x58) {
        this->OrderWalkTo(unit, 0x52, 0x2F);
    }
}

// 5af5dc (client: 57d74c) - helper for ProcessGroupRecall.
void World::ProcessUnitRecall(Unit* unit) {
    if (this->WantsRetreat(unit) != 0) {
        this->DoTeleportHome(unit);
    } else if (this->MustFlee(unit) != 0) {
        this->FleeFromEnemies(unit);
    }
}

// 5abcfd (client: 579e6d)
void World::RunUnitAi(Unit* unit) {
    this->ProcessUnitAi(unit);
}

// 5a5155 (client: 5732c5) - helper for RunUnitAi.
void World::ProcessUnitAi(Unit* unit) {
    if (!unit->pOwner->is_ai) {
        this->ProcessUnitState(unit);
    } else {
        this->RunAiUnitState(unit);
    }
}

// 5a5188 (client: 5732f8) - AI wrapper for ProcessUnitState.
void World::RunAiUnitState(Unit* unit) {
    this->ProcessUnitState(unit);
}

// 5a51a1 (client: 573311) - per-state unit action dispatcher.
void World::ProcessUnitState(Unit* unit) {
#ifdef A2SERVER_PATCH
    if (unit->state == Unit::ORDER_ATTACK_UNIT && unit->eye2->unit != nullptr && unit->eye2->unit->IsUnit() && unit->eye2->unit->some_state == Unit::EXEC_DEAD) {
        this->ResetUnitToIdle(unit);
    }
    if ((unit->state == Unit::ORDER_DEFEND_TARGET || unit->state == Unit::ORDER_FOLLOW) && unit->eye2->unit2 != nullptr && unit->eye2->unit2->IsUnit() && unit->eye2->unit2->some_state == Unit::EXEC_DEAD) {
        this->ResetUnitToIdle(unit);
    }
#else
    if (unit->state == Unit::ORDER_ATTACK_UNIT && unit->eye2->unit != nullptr && unit->eye2->unit->some_state == Unit::EXEC_DEAD) {
        this->ResetUnitToIdle(unit);
    }
    if ((unit->state == Unit::ORDER_DEFEND_TARGET || unit->state == Unit::ORDER_FOLLOW) && unit->eye2->unit2 != nullptr && unit->eye2->unit2->some_state == Unit::EXEC_DEAD) {
        this->ResetUnitToIdle(unit);
    }
#endif
    if (unit->state == Unit::ORDER_MOVE && unit->eye->target_reached != 0 && unit->eye->move_target.val == unit->position->CompatGetYX()) {
        this->ResetUnitToIdle(unit);
    }

    switch (unit->state) {
    case Unit::ORDER_IDLE:
        unit->some_state = Unit::EXEC_IDLE;
        break;
    case Unit::ORDER_MOVE:
        this->DoMoveStep(unit, unit->eye2->command_to);
        break;
    case Unit::ORDER_PICKUP:
        this->DoSwarmMoveStep(unit, unit->eye2->command_to);
        break;
    case Unit::ORDER_ATTACK_UNIT:
        this->DoAttackTarget(unit, unit->eye2->unit);
        break;
    case Unit::ORDER_ATTACK_MOVE:
        this->DoAttackMove(unit, unit->eye2->command_to);
        break;
    case Unit::ORDER_DEFEND_TARGET:
        this->TickAttackTarget(unit, unit->eye2->unit2);
        break;
    case Unit::ORDER_PATROL_STEP:
        this->DoPatrolTick(unit);
        break;
    case Unit::ORDER_GUARD_POSITION:
        this->AdvancePatrolStep(unit);
        break;
    case Unit::ORDER_HOLD_POSITION:
        this->DoGuardScan(unit);
        break;
    case Unit::ORDER_CAST_UNIT:
        this->BeginAutobuffCast(unit, unit->eye2->unit5, unit->eye2->spell);
        break;
    case Unit::ORDER_CAST_POS:
        this->BeginPositionCast(unit, unit->eye2->cast_pos, unit->eye2->spell);
        break;
    case Unit::ORDER_ENTER_BUILDING:
        this->BeginEnterBuilding(unit, unit->eye2->enter_building);
        break;
    case Unit::ORDER_FOLLOW:
        this->TickFollowTarget(unit, unit->eye2->unit2);
        break;
    case Unit::ORDER_RETURN_HOME:
        this->DoTeleportHome(unit);
        break;
    case Unit::ORDER_SCRIPT_PATROL:
        this->DoScriptedPatrol(unit);
        break;
    case Unit::ORDER_SCRIPT_WALK:
        this->DoScriptedWalk(unit);
        break;
    case Unit::ORDER_LOOT_NEARBY:
        this->DoGatherLootStep(unit);
        break;
    case Unit::ORDER_SCRIPT_SPECIAL:
        unit->some_state = Unit::EXEC_SPECIAL;
        break;
    default:
        unit->some_state = Unit::EXEC_IDLE;
        this->SetIdleOrRetreat(unit, 0);
        break;
    }
}

// 5ae5c0 (client: 57c730)
void World::JitterGuardRangeWide(Group* group) {
    uint8_t base = group->group_sub->guard_range_base;
    int32_t r = this->RandomRange(-1, 1);
    group->group_sub->guard_range = base + r + 4;
}

// 5ae5f5 (client: 57c765)
void World::JitterGuardRange(Group* group) {
    uint8_t base = group->group_sub->guard_range_base;
    int32_t r = this->RandomRange(-1, 1);
    group->group_sub->guard_range = base + r;
}

// 5af623 (client: 57d793)
void World::FlagEmptyGroup(Group* group) {
    if (group->unit_list.GetCount() == 0) {
        group->group_sub->order = GroupSub::ORDER_IDLE_ALL;
    }
}

// 5aefe5 (client: 57d155)
void World::AiStandBrain(Group* group) {
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->DoRetreat(unit);
    }
}

// 5af022 (client: 57d192)
void World::IdleGroupUnits(Group* group) {
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->SetIdleOrRetreat(unit, 0);
    }
}

// 5af061 (client: 57d1d1)
void World::AiMoveBrain(Group* group, uint16_t yx) {
    this->ComputeGroupAvgPos(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);

        if (unit->position->CompatGetYX() == unit->eye2->command_to && unit->position->IsAtCellCenter() && unit->eye2->has_command == 0) {
            this->ResetUnitToIdle(unit);
            unit->eye->target_reached = 0;
        } else {
            if (unit->eye->target_reached != 0 && unit->eye->move_target.val == unit->position->CompatGetYX()) {
                this->ResetUnitToIdle(unit);
                unit->eye->target_reached = 0;
            }
        }

        if (unit->eye2->has_command != 0) {
            this->SetIdleOrRetreat(unit, 0);
        } else {
            if (group->group_sub->speed_override != 0) {
                unit->eye2->gap_0x6c = 1;
            }

            this->DoMoveStep(unit, unit->eye2->command_to);

            if (!this->map_stuff->IsCellPassableDynamic(unit, yx) && unit->eye2->command_to == yx) {
                uint8_t distance = this->map_stuff->ChebyshevDistance(yx, unit->position->CompatGetYX());
                if (distance < 2) {
                    this->DoGuardScan(unit);
                }
            }
        }
    }
}

// 5af646 (client: 57d7b6)
void World::ProcessGroupRecall(Group* group) {
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->ProcessUnitRecall(unit);
    }
}

// 5ab92c (client: 579a9c)
void World::UpdateGroupCenter(Group* group) {
    int32_t count = group->unit_list.GetCount();
    if (count == 0) {
        return;
    }

    uint16_t sum_x = 0;
    uint16_t sum_y = 0;
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        sum_x += unit->position->GetX();
        sum_y += unit->position->GetY();
    }

    uint8_t avg_x = static_cast<uint8_t>(sum_x / count);
    uint8_t avg_y = static_cast<uint8_t>(sum_y / count);
    PosYX avg_yx(avg_x, avg_y);

    group->group_sub->avg_pos_yyxx = (avg_y << 16) | avg_x;
    group->group_sub->avg_pos = avg_yx.val;

    uint8_t max_distance = 0;
    uint8_t max_scan_range = 0;
    uint8_t max_sum = 0;
    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        uint8_t distance = this->map_stuff->ChebyshevDistance(unit->position->GetYX(), avg_yx);
        uint8_t scan_range = unit->scan_range >> 8;
        if (distance > max_distance) {
            max_distance = distance;
        }
        if (scan_range > max_scan_range) {
            max_scan_range = scan_range;
        }
        if (distance + scan_range > max_sum) {
            max_sum = distance + scan_range;
        }
    }

    group->group_sub->max_unit_dist = max_distance;
    group->group_sub->max_scan_range = max_scan_range;
    group->group_sub->max_dist_plus_scan = max_sum;
}

// 5ad2c1 (client: 57b431)
int32_t World::GetTargetScore(Unit* unit, Unit* target) {
    PosYX unit_yx = unit->position->GetYX();
    PosYX target_yx = target->position->GetYX();
    uint8_t distance = this->map_stuff->ChebyshevDistance(unit_yx, target_yx);

    uint8_t target_movement = 0;
    uint8_t target_range = this->UnitMaxRange(target);
    if (target_range > 1 && target->MovementType() == 1) {
        target_movement = 0;
    } else {
        target_movement = target->MovementType();
    }

    uint8_t unit_range = this->UnitMaxRange(unit);
    uint8_t cost = 0;
    if (unit_range <= 1) {
        int32_t movement_type = unit->MovementType();
        cost = this->attack_decision_table[target_movement + movement_type * 4];
    } else {
        cost = this->attack_decision_table[target_movement];
        if (distance > unit_range) {
            distance = distance - unit_range + 1;
        } else {
            distance = 1;
        }
    }

    uint8_t facing = this->map_stuff->GetFacingTo(unit, target);
    uint8_t facing_diff = this->map_stuff->FacingDiff(unit->eye->facing, facing);

    if (distance > 1) {
        return 0xFFFFFF;
    }

    int32_t score = (distance << 8) + facing_diff;
    if (cost == 0) {
        return 0xFFFFFF;
    } else if (cost == 1) {
        score *= 2;
    } else if (cost == 4) {
        score /= 2;
    }
    return score;
}

// 5ae471 (client: 57c5e1)
void World::AssignBestTargets(Group* group) {
    if (group->unit_list.GetCount() == 0 || this->nearby_list.unit_list.GetCount() == 0) {
        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            unit->eye2->unit4 = nullptr;
        }
    } else {
        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            Unit* best_target = nullptr;
            int32_t best_score = 0xFFFFFF;

            for (POSITION target_it = this->nearby_list.unit_list.GetHeadPosition(); target_it != nullptr;) {
                Unit* target = this->nearby_list.unit_list.GetNext(target_it);
                int32_t score = this->GetTargetScore(unit, target);
                if (score < best_score) {
                    best_target = target;
                    best_score = score;
                }
            }

            if (best_score == 0xFFFFFF) {
                unit->eye2->unit4 = nullptr;
            } else {
                unit->eye2->unit4 = best_target;
            }
        }
    }
}

// 5a3d12 (client: 571e82)
void World::PruneNearbyUnits(uint16_t pos_yx, uint8_t max_distance) {
    CList<Unit*>& unit_list = this->nearby_list.unit_list;
    POSITION it = unit_list.GetHeadPosition();
    while (it != nullptr) {
        POSITION current_it = it;
        Unit* unit = unit_list.GetNext(it);
        uint16_t unit_pos = unit->position->CompatGetYX();
        uint8_t distance = this->map_stuff->ChebyshevDistance(pos_yx, unit_pos);
        if (distance > max_distance) {
            unit_list.RemoveAt(current_it);
        }
    }
}

// 5ae2d4 (client: 57c444)
void World::AssignNearbyTargets(Group* group) {
    if (group->unit_list.GetCount() == 0 || this->nearby_list.unit_list.GetCount() == 0) {
        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            unit->eye2->unit4 = nullptr;
        }
    } else {
        this->scrape_action.action_kind = 4;
        this->scrape_action.actor_group = group;
        this->action_list.AddTail(this->scrape_action);

        for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(it);
            Unit* best_target = nullptr;
            int32_t best_score = 0xFFFFFF;

            for (POSITION target_it = this->nearby_list.unit_list.GetHeadPosition(); target_it != nullptr;) {
                Unit* target = this->nearby_list.unit_list.GetNext(target_it);
                int32_t score = this->GetAttackDecision(unit, target);
                if (score < best_score) {
                    best_target = target;
                    best_score = score;
                }
            }

            if (best_score == 0xFFFFFF) {
                unit->eye2->unit4 = nullptr;
            } else {
                unit->eye2->unit4 = best_target;
            }
        }
    }
}

// 5ae628 (client: 57c798)
void World::AiGuardBrain(Group* group) {
    this->UpdateGroupCenter(group);
    this->ScanNearbyUnits(group);
    this->PruneNearbyUnits(group->group_sub->avg_pos, group->group_sub->guard_range);
    this->AssignNearbyTargets(group);

    POSITION it = group->unit_list.GetHeadPosition();
    if (it == nullptr) {
        if (group->group_sub->was_empty == 0) {
            group->group_sub->phase_ticks++;
        } else {
            this->JitterGuardRange(group);
            group->group_sub->phase_ticks = 0;
        }
        group->group_sub->was_empty = 0;
    } else {
        if (group->group_sub->was_empty == 0) {
            this->JitterGuardRangeWide(group);
            group->group_sub->phase_ticks = 0;
        } else {
            group->group_sub->phase_ticks++;
        }
        group->group_sub->was_empty = 1;
    }

    while (it != nullptr) {
        Unit* unit = group->unit_list.GetNext(it);
        if (unit->eye2->unit4 == nullptr) {
            if (unit->position->GetYX() == unit->eye2->position1 && unit->position->IsAtCellCenter()) {
                this->SetIdleOrRetreat(unit, 1);
            } else {
                this->DoMoveStep(unit, unit->eye2->position1);
            }
        } else {
            this->DoAttackTarget(unit, unit->eye2->unit4);
        }
    }
}

// 5ae7c3 (client: 57c933)
void World::AiDefendBrain(Group* group) {
    this->UpdateGroupCenter(group);
    this->ScanNearbyUnits(group);
    this->PruneNearbyUnits(group->group_sub->avg_pos, group->group_sub->guard_range);
    this->AssignNearbyTargets(group);

    POSITION it = group->unit_list.GetHeadPosition();
    if (it == nullptr) {
        if (group->group_sub->was_empty == 0) {
            group->group_sub->phase_ticks++;
        } else {
            this->JitterGuardRange(group);
            group->group_sub->phase_ticks = 0;
        }
        group->group_sub->was_empty = 0;
    } else {
        if (group->group_sub->was_empty == 0) {
            this->JitterGuardRangeWide(group);
            group->group_sub->phase_ticks = 0;
        } else {
            group->group_sub->phase_ticks++;
        }
        group->group_sub->was_empty = 1;
    }

    while (it != nullptr) {
        Unit* unit = group->unit_list.GetNext(it);
        int32_t tick16_diff = std::abs(unit->eye2->tick16 - g_Server->tick16);
        if (unit->hp != unit->hp_max && tick16_diff < 30) {
            this->DoMoveStep(unit, unit->eye2->attack_move_pos);
        } else {
            if (tick16_diff < 10) {
                this->DoMoveStep(unit, unit->eye2->attack_move_pos);
            } else if (unit->eye2->unit4 != nullptr) {
                this->DoAttackTarget(unit, unit->eye2->unit4);
            } else {
                int32_t tick_diff = std::abs((int32_t)unit->eye2->tick - g_Server->tick);
                if (unit->hp == unit->hp_max || tick_diff > 9) {
                    if (unit->eye2->counter3 == 0) {
                        if (unit->position->GetYX() == unit->eye2->position1 && unit->position->IsAtCellCenter()) {
                            this->SetIdleOrRetreat(unit, 1);
                        } else {
                            this->DoMoveStep(unit, unit->eye2->position1);
                        }
                    } else {
                        unit->eye2->counter3 += 1;
                        this->DoMoveStep(unit, unit->eye2->gap_0xa0);
                        if (unit->eye2->counter3 > 10) {
                            unit->eye2->counter3 = 0;
                        }
                    }
                } else {
                    this->DoMoveStep(unit, unit->eye2->attack_move_pos);
                }
            }
        }
    }
}

// 5ac80f (client: 57a97f)
void World::GroupAttackCommand(Group* group, Unit* target) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        if (this->GetAttackDecision(unit, target) == 0xFFFFFF) {
            this->ResetUnitToHold(unit);
        } else {
            this->OrderEngageTarget(unit, target);
        }
    }
}

// 5acb4d (client: 57acbd)
void World::GroupAttackTargetCommand(Group* group, Unit* target, uint8_t range) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderAttackTarget(unit, target, range);
    }
}

// 5acb9e (client: 57ad0e)
void World::GroupFollowCommand(Group* group, Unit* target, uint8_t range) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderFollowTarget(unit, target, range);
    }
}

// 5acbef (client: 57ad5f)
void World::GroupPatrolCommand(Group* group, uint8_t x, uint8_t y) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderPatrolTo(unit, x, y);
    }
}

// 5ac187 (client: 57a2f7)
void World::GroupCastAtUnitCommand(Group* group, Unit* target, Spell*) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        if (unit->spell == nullptr) {
            this->ResetUnitToHold(unit);
        } else {
            this->OrderCastAtUnit(unit, target, unit->spell);
        }
        unit->spell = nullptr;
    }

    group->group_sub->order = GroupSub::ORDER_DEFAULT;
}

// 5acaa7 (client: 57ac17) - order group to retreat/evade (state 0x13).
void World::GroupStopCommand(Group* group) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderStandGround(unit);
    }

    group->group_sub->order = GroupSub::ORDER_STAND;
}

// 5acafa (client: 57ac6a) - order group to idle/wander (state 0).
void World::GroupGoHomeCommand(Group* group) {
    this->ClearGroupOrders(group);

    for (POSITION it = group->unit_list.GetHeadPosition(); it != nullptr;) {
        Unit* unit = group->unit_list.GetNext(it);
        this->OrderTeleportHome(unit);
    }

    group->group_sub->order = GroupSub::ORDER_DEFAULT;
}

// 5b0762 (client: 57e8d2) - evaluate pending trigger checks.
void World::EvaluateTriggerChecks() {
    TriggerCheck local;

    for (POSITION it = this->trigger_checks->GetHeadPosition(); it != nullptr;) {
        TriggerCheck trigger_check = this->trigger_checks->GetNext(it); // Copies the value.
        if (trigger_check.ref_count == 0) {
            this->DispatchTriggerCheck(&trigger_check);
        }
    }
}

// 5b081e (client: 57e98e) - trigger-check dispatcher.
void World::DispatchTriggerCheck(TriggerCheck* check) {
    switch (check->type) {
    case trigger_check_op::group_unit_count:           this->CheckOP_1(check); break;
    case trigger_check_op::unit_in_box:                this->CheckOP_2(check); break;
    case trigger_check_op::unit_in_circle:             this->CheckOP_3(check); break;
    case trigger_check_op::unit_param:                 this->CheckOP_4(check); break;
    case trigger_check_op::unit_alive:                 this->CheckOP_5(check); break;
    case trigger_check_op::unit_distance:              this->CheckOP_6(check); break;
    case trigger_check_op::point_to_unit_dist:         this->CheckOP_7(check); break;
    case trigger_check_op::player_unit_count:          this->CheckOP_8(check); break;
    case trigger_check_op::unit_attack_target:         this->CheckOP_9(check); break;
    case trigger_check_op::diplomacy:                  this->CheckOP_10(check); break;
    case trigger_check_op::noop_11:                    break; // No-op.
    case trigger_check_op::item_in_inventory:          this->CheckOP_12(check); break;
    case trigger_check_op::noop_13:                    break; // No-op.
    case trigger_check_op::sack_at_point:              this->CheckOP_14(check); break;
    case trigger_check_op::nearest_player_unit_dist:   this->CheckOP_15(check); break;
    case trigger_check_op::item_carrier_to_point_dist: this->CheckOP_16(check); break;
    case trigger_check_op::item_in_inventory_alt:      this->CheckOP_17(check); break;
    case trigger_check_op::vip_unit_alive:             this->CheckOP_18(check); break;
    case trigger_check_op::variable_value:             this->CheckOP_19(check); break;
    case trigger_check_op::player_building_count:      this->CheckOP_20(check); break;
    case trigger_check_op::building_health:            this->CheckOP_21(check); break;
    case trigger_check_op::teleport_unit_on_tile:      this->CheckOP_22(check); break;
    case trigger_check_op::scenario_variable:          this->CheckOP_23(check); break;
    case trigger_check_op::subobjective:               this->CheckOP_24(check); break;
    case trigger_check_op::spell_on_tile:              this->CheckOP_25(check); break;
    case trigger_check_op::spell_on_unit:              this->CheckOP_26(check); break;
    case trigger_check_op::unit_centered_on_tile:      this->CheckOP_27(check); break;
    }
}

// 5b2e91 (client: 581001) - How many units contains this group
void World::CheckOP_1(TriggerCheck* check) {
    this->trigger_variables[check->id] = check->group->unit_list.GetCount();
}

// 5b2ef9 (client: 581069) - Is unit in a box
void World::CheckOP_2(TriggerCheck* check) {
    uint8_t x = check->unit->position->GetX();
    uint8_t y = check->unit->position->GetY();
    if (check->data[0] <= x && x <= check->data[2] && check->data[1] <= y && y <= check->data[3]) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b2f9a (client: 58110a) - Is unit in a circle
void World::CheckOP_3(TriggerCheck* check) {
    TokenPos* pos = check->unit->position;
    int32_t distance = this->map_stuff->ChebyshevDistanceXY(pos->GetX(), pos->GetY(), check->data[0], check->data[1]);
    if (distance > check->data[2]) {
        this->trigger_variables[check->id] = 0;
    } else {
        this->trigger_variables[check->id] = 1;
    }
}

// 5b3071 (client: 5811e1) - Get unit parameter
void World::CheckOP_4(TriggerCheck* check) {
    if (check->data[0] == modifier::health) {
        this->trigger_variables[check->id] = check->unit->hp;
    }
}

// 5b2eb8 (client: 581028) - Is unit alive?
void World::CheckOP_5(TriggerCheck* check) {
    if (check->unit->some_state == Unit::EXEC_DEAD) {
        this->trigger_variables[check->id] = 0;
    } else {
        this->trigger_variables[check->id] = 1;
    }
}

// 5b3131 (client: 5812a1) - Distance between units
void World::CheckOP_6(TriggerCheck* check) {
    Unit* other = static_cast<Unit*>(check->multi);

    if (check->unit->some_state == Unit::EXEC_DEAD || check->multi == nullptr || other->some_state == Unit::EXEC_DEAD) {
        this->trigger_variables[check->id] = 0xFF;
    } else {
        TokenPos* pos = check->unit->position;
        TokenPos* other_pos = other->position;
        this->trigger_variables[check->id] = this->map_stuff->ChebyshevDistanceXY(pos->GetX(), pos->GetY(), other_pos->GetX(), other_pos->GetY());
    }
}

// 5b3016 (client: 581186) - Distance from point to unit
void World::CheckOP_7(TriggerCheck* check) {
    TokenPos* pos = check->unit->position;
    this->trigger_variables[check->id] = this->map_stuff->ChebyshevDistanceXY(pos->GetX(), pos->GetY(), check->data[0], check->data[1]);
}

// 5b30ad (client: 58121d) - How many units this player have
void World::CheckOP_8(TriggerCheck* check) {
    int32_t count = 0;
    CList<Group*>& groups = check->player->group_list->groups;
    for (POSITION it = groups.GetHeadPosition(); it != nullptr;) {
        Group* group = groups.GetNext(it);
        count += group->unit_list.GetCount();
    }
    this->trigger_variables[check->id] = count;
}

// 5b31c9 (client: 581339) - Is unit make attack
void World::CheckOP_9(TriggerCheck* check) {
    if (check->unit->eye2->cast_action == 5) {
        this->trigger_variables[check->id] = check->unit->eye2->unit->TokenID;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b3222 (client: 581392) - Get diplomacy
void World::CheckOP_10(TriggerCheck* check) {
    int other_id = static_cast<Player*>(check->multi)->player_id;
    this->trigger_variables[check->id] = this->diplomacy.diplomacy[check->player->player_id][other_id] & (DIPLO_WAR | DIPLO_ALLIANCE);
}

// 5b3273 (client: 5813e3) - Item in inventory, same as check 17.
void World::CheckOP_12(TriggerCheck* check) {
    if (check->unit->inventory->FindItemById(check->item_id) != nullptr) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b32cf (client: 58143f) - Get sack
void World::CheckOP_14(TriggerCheck* check) {
    if (this->map_stuff->GetSackAt(check->data[0], check->data[1]) != nullptr) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b3324 (client: 581494) - Distance to nearest units of player
void World::CheckOP_15(TriggerCheck* check) {
    int32_t best = 0xFF;
    CList<Group*>& groups = check->player->group_list->groups;
    for (POSITION it = groups.GetHeadPosition(); it != nullptr;) {
        Group* group = groups.GetNext(it);
        for (POSITION unit_it = group->unit_list.GetHeadPosition(); unit_it != nullptr;) {
            Unit* unit = group->unit_list.GetNext(unit_it);
            int32_t distance = this->map_stuff->ChebyshevDistanceXY(unit->position->GetX(), unit->position->GetY(), check->data[0], check->data[1]);
            if (distance < best) {
                best = distance;
            }
        }
    }
    this->trigger_variables[check->id] = best;
}

// 5b33f0 (client: 581560) - Item to point
void World::CheckOP_16(TriggerCheck* check) {
    if (check->unit->inventory->FindItemById(check->item_id) != nullptr) {
        this->trigger_variables[check->id] = this->map_stuff->ChebyshevDistanceXY(check->unit->position->GetX(), check->unit->position->GetY(), check->data[0], check->data[1]);
    } else {
        this->trigger_variables[check->id] = 0xFF;
    }
}

// 5b347a (client: 5815ea) - Item in inventory
void World::CheckOP_17(TriggerCheck* check) {
    if (check->unit->inventory->FindItemById(check->item_id) != nullptr) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b34c9 (client: 581639) - VIP
void World::CheckOP_18(TriggerCheck* check) {
    if (check->unit->some_state == Unit::EXEC_DEAD) {
        this->mission_fail += 1;
    }
}

// 5b34f7 (client: 581667) - Return value of variable
void World::CheckOP_19(TriggerCheck* check) {
    this->trigger_variables[check->id] = this->trigger_variables[check->data[0]];
}

// 5b3525 (client: 581695) - How many buildings this player have
void World::CheckOP_20(TriggerCheck* check) {
    int32_t count = 0;
    BuildingsList* buildings = g_Server->srv_stru1->building_list;
    for (POSITION it = buildings->GetHeadPosition(); it != nullptr;) {
        Building* building = buildings->GetNext(it);
        if (building->pOwner == check->player && building->hp > 0) {
            count += 1;
        }
    }
    this->trigger_variables[check->id] = count;
}

// 5b35a0 (client: 581710) - Health of building
void World::CheckOP_21(TriggerCheck* check) {
    this->trigger_variables[check->id] = check->building->hp;
}

// 5b35c6 (client: 581736) - Teleportation
void World::CheckOP_22(TriggerCheck* check) {
    PosYX yx(check->data[0], check->data[1]);
    Unit* unit = this->map_stuff->GetSmallUnitAt(yx.val);
    if (unit != nullptr) {
        this->map_stuff->TeleportUnit(unit, check->data[2], check->data[3]);
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b3644 (client: 5817b4) - Get Scenario Variable
void World::CheckOP_23(TriggerCheck* check) {
    this->trigger_variables[check->id] = ScenarioGetVar(check->data[0]);
}

// 5b366d (client: 5817dd) - Get Subobjective
void World::CheckOP_24(TriggerCheck* check) {
    this->trigger_variables[check->id] = ScenarioGetVar(check->data[0] + 752);
}

// 5b369c (client: 58180c) - Spell on Tile
void World::CheckOP_25(TriggerCheck* check) {
    PosYX yx(check->data[0], check->data[1]);
    AreaEffect** effects = this->map_stuff->GetAreaEffectsAt(yx.val);
    if (effects == nullptr) {
        this->trigger_variables[check->id] = 0;
    } else if (effects[check->data[2]] != nullptr) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}

// 5b3720 (client: 581890) - Spell on Unit
void World::CheckOP_26(TriggerCheck* check) {
    if ((check->unit->enchantments & (1 << check->data[0])) == 0) {
        this->trigger_variables[check->id] = 0;
    } else {
        this->trigger_variables[check->id] = 1;
    }
}

// 5b3776 (client: 5818e6) - Centered on Tile
void World::CheckOP_27(TriggerCheck* check) {
    TokenPos* pos = check->unit->position;
    if (pos->GetX() == check->data[1] && pos->GetY() == check->data[2] && pos->IsAtCellCenter() != 0) {
        this->trigger_variables[check->id] = 1;
    } else {
        this->trigger_variables[check->id] = 0;
    }
}
