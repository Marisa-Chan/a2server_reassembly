#include "unit_list.h"
#include "unit.h"
#include "player.h"
#include "effect.h"
#include "group.h"
#include "item.h"
#include "game_app.h"
#include "players_list.h"
#include "quest.h"
#include "quest_map.h"
#ifdef A2TANGAR_HAT
#include "patch/config_new.h"
#endif

#ifdef A2TANGAR_HAT
// death_update_exp from the manager (player_died/player_killed/
// player_pkilled hooks): scale each sphere's experience by the multiplier
// and recompute the total.
static void TangarApplyDeathExpPenalty(Humanoid* unit, double mult) {
    unit->experience = 0;
    for (int32_t i = 0; i < 5; i++) {
        unit->experience_per_sphere[i] = (uint32_t)(unit->experience_per_sphere[i] * mult);
        unit->experience += unit->experience_per_sphere[i];
    }
}
#endif

// 555a1d (client: 534ebc)
UnitList::UnitList() = default;

// 555a8d (client: 534ede)
UnitList::~UnitList() = default;

// 558ef8 (client: 536222)
void UnitList::Serialize(CArchive& ar) {
    //558ef8 - both binaries write the count as a raw 4-byte int (not WriteCount)
    if (ar.IsStoring()) {
        uint32_t count = this->unit_list.GetCount();
        ar << count;

        POSITION pos = this->unit_list.GetHeadPosition();
        while (pos != NULL) {
            Unit *unit = this->unit_list.GetNext(pos);
            ar.WriteObject(unit);
        }
    } else {
        uint32_t count = 0;
        ar >> count;
        Unit* unit;
        while (count--) {
            ar >> unit;
            this->unit_list.AddTail(unit);
        }
    }
}

// 5591da (client: 53623e)
void UnitList::FixupPointers() {
    POSITION pos = this->unit_list.GetHeadPosition();
    while (pos != NULL) {
        Unit* unit = this->unit_list.GetNext(pos);
        unit->FixupPointers();
    }
}

// 555c1b (client: 534f4c)
void UnitList::AddTail(Unit* unit) {
    this->unit_list.AddTail(unit);
}

// 555d67 (client: 534f68)
void UnitList::AddTailAllocId(Unit* unit) {
    this->unit_list.AddTail(unit);
    unit->building_id = g_buildingIdSet.AllocBit();
}

// 555f1c (client: 534fa3)
void UnitList::AddTailTokenId(Unit* unit) {
    this->unit_list.AddTail(unit);
    unit->building_id = 0x6000 + unit->TokenID;
}

// 575aa0 (client: 546940) - Remove unit from list and in any do delete for this unit
void UnitList::Remove(Unit* unit) {
    POSITION pos = this->unit_list.Find(unit);
    if (pos) {
        this->unit_list.RemoveAt(pos);
    }

    delete unit;
}

// 5560d2 (client: 534fdf) - Look up unit in list by ID
Unit* UnitList::FindById(uint16_t id) {
    POSITION it = this->unit_list.GetHeadPosition();
    while (it) {
        Unit* unit = this->unit_list.GetNext(it);
        if (unit->building_id == (int16_t)id) {
            return unit;
        }
    }
    return nullptr;
}

// 556345 (client: 53517b) - Tick alive units and reap dead ones
void UnitList::ProcessTick() {
    POSITION pos = this->unit_list.GetHeadPosition();
    while (pos != NULL) {
        POSITION cur_pos = pos;
        Unit* unit = this->unit_list.GetNext(pos);

        // Validate last_hit_by: clear if killer has no owner
        if (unit->last_hit_by != NULL && unit->last_hit_by->pOwner == NULL) {
            unit->last_hit_by = NULL;
        }

        // Send knowledge flag updates to players when visibility tracking diverges
        if (unit->seen_by_mask != unit->seen_by_mask_prev && g_Server->multiplayer != 0) {
            for (int i = 0; i < 16; i++) {
                if ((unit->seen_by_mask & (1 << i)) != 0 && (unit->seen_by_mask_prev & (1 << i)) == 0) {
                    Player* player = g_PlayersList->FindById(i + 16);
                    if (player != NULL) {
                        g_NetStru1_main.SendUnitUpdate(unit, player, unit->something_per_player[i], EQUIP_SLOTS_ALL, 0, 0);
                    }
                }
            }
            unit->seen_by_mask_prev = unit->seen_by_mask;
        }

        // Tick alive unit
        if (unit->some_state != Unit::EXEC_DEAD) {
            #ifdef A2SERVER_PATCH
            // bug_0014: skip the virtual call for zombie units (zeroed vtable)
            if (*reinterpret_cast<void**>(unit) != nullptr) {
                unit->ProcessEffects();
            }
            #else
            unit->ProcessEffects();
            #endif
        }

        // Process dead unit
        if (unit->some_state == Unit::EXEC_DEAD) {
            // Capture group quest ID before removal
            int32_t group_id = 0;
            if (unit->group != NULL && unit->group->unit_list.GetCount() == 1 && (unit->group->has_quest_intercept != 0 || unit->group->has_quest_kill != 0)) {
                group_id = unit->group->group_id;
            }

            // Notify owner player and remove unit from player's list
            if (unit->pOwner != NULL) {
                unit->pOwner->RemoveUnitFromGroup(unit);
            }

            // Remove from this list and add to dead units pool
            this->unit_list.RemoveAt(cur_pos);
            g_Server->srv_stru1->units_list->AddTail(unit);

            // Clean up effects
            POSITION epos = unit->_effects.GetHeadPosition();
            while (epos != NULL) {
                POSITION ecur_pos = epos;
                Effect* effect = unit->_effects.GetNext(epos);
                effect->spell_value = 1;
                effect->caster = NULL;
                effect->TickOnUnit(unit);
                if (effect->usage_type & Effect::USAGE_EXPIRED) {
                    unit->_effects.RemoveAt(ecur_pos);
                    delete effect;
                }
            }

            // Kill attribution
            #ifdef A2SERVER_PATCH
            // part_of_kill hook (5569EF): units with hp_max <= 0 must not
            // attribute kills; their last_hit_by is cleared. bug_0015:
            // unit->pOwner may be null - skip kill attribution.
            if (unit->hp_max <= 0) {
                unit->last_hit_by = nullptr;
            }
            if (unit->last_hit_by != NULL && unit->pOwner != NULL) {
            #else
            if (unit->last_hit_by != NULL) {
            #endif
                if (unit->last_hit_by->typeId < unit_type::hero_fighter_male || unit->last_hit_by->typeId > unit_type::humanoid_last) {
                    // Killer is NOT a monster type
                    if (unit_type::hero_sprite_base < unit->typeId && unit->typeId < unit_type::monster_first && unit->last_hit_by->IsUnit() != 0 && unit->last_hit_by->monster_info != NULL) {
                        #ifdef A2TANGAR_HAT
                        // player_killed hook (557005)
                        TangarApplyDeathExpPenalty(static_cast<Humanoid*>(unit), Config::KilledExpMult);
                        #endif
                        CString msg;
                        uint8_t y = unit->position->GetY();
                        uint8_t x = unit->position->GetX();
                        msg.Format("Player %s killed by %s ID=%d at %d,%d", (const char*)unit->pOwner->name, (const char*)unit->last_hit_by->monster_info->name, unit->last_hit_by->building_id, x, y);
                        LogMessage(msg);
                        if (g_ServerConfig.gameType != GAME_TYPE_COOPERATIVE) {
                            if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY && unit->last_hit_by->pOwner != nullptr) {
                                if (unit->last_hit_by->pOwner->name == "Red" && unit->pOwner->team == 1) {
                                    g_Server->ctf_score[0]++;
                                } else if (unit->last_hit_by->pOwner->name == "Blue" && unit->pOwner->team == 0) {
                                    g_Server->ctf_score[1]++;
                                } else {
                                    g_Server->ctf_score[unit->pOwner->team]--;
                                    unit->pOwner->frags--;
                                }
                            } else {
                                g_Server->ctf_score[unit->pOwner->team]--;
                                unit->pOwner->frags--;
                            }
                            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                                g_NetStru1_main.SendInfoPacket(KILL_NOTICE, unit->pOwner->player_id, 0xb0000, nullptr);
                                g_NetStru1_main.SendTeamState(0);
                            } else if (g_ServerConfig.gameType == GAME_TYPE_ARENA && g_Server->tick > 50) {
                                g_NetStru1_main.SendInfoPacket(KILL_NOTICE, unit->pOwner->player_id, 0xb0000, nullptr);
                                g_Server->AddArenaFrags(&unit->pOwner->name, -1);
                            }
                        }
                    }
                } else if (unit->last_hit_by->hp < 0) {
                    // Killer (a monster) is also dead — mutual kill
                    if (unit->pOwner->is_ai == 0) {
                        #ifdef A2TANGAR_HAT
                        // player_died hook (556A56)
                        TangarApplyDeathExpPenalty(static_cast<Humanoid*>(unit), Config::DeathExpMult);
                        #endif

                        CString msg;
                        msg.Format("Player %s has died.", (const char*)unit->pOwner->name);
                        LogMessage(msg);
                    }
                    unit->last_hit_by = nullptr;
                    if (g_ServerConfig.gameType != GAME_TYPE_COOPERATIVE && unit->pOwner->is_ai == 0 && g_Server->tick > 50) {
                        unit->pOwner->frags--;
                        g_Server->ctf_score[unit->pOwner->team]--;
                        g_NetStru1_main.SendInfoPacket(KILL_NOTICE, unit->pOwner->player_id, 0xc0000, nullptr);
                        if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                            g_NetStru1_main.SendTeamState(0);
                        } else {
                            g_Server->AddArenaFrags(&unit->pOwner->name, -1);
                        }
                    }
                } else {
                    // Normal pkill --- killer (a monster) is alive
                    unit->last_hit_by->AwardKillMoney(unit);
                    unit->last_hit_by->AwardKillExperience(unit, unit->last_hit_spell_id);
                    if (unit->typeId < unit_type::hero_fighter_male || unit->typeId > unit_type::humanoid_last) {
                        // Victim is a monster
                        unit->last_hit_by->pOwner->monster_kills++;
                    } else {
                        // Victim is a human

                        #ifdef A2TANGAR_HAT
                        // player_pkilled hook (556BE6)
                        TangarApplyDeathExpPenalty(static_cast<Humanoid*>(unit), Config::PKExpMult);
                        #endif

                        CString msg;
                        uint8_t y = unit->position->GetY();
                        uint8_t x = unit->position->GetX();
                        // pkill_msg hook (556C0F): pushes killer owner name (unit->last_hit_by->pOwner->name).
                        msg.Format("Player %s pkilled by %s at %d,%d", (const char*)unit->pOwner->name, (const char*)unit->last_hit_by->pOwner->name, x, y);
                        LogMessage(msg);
                        uint32_t kill_sphere = 0;
                        if (g_ServerConfig.gameType != GAME_TYPE_COOPERATIVE) {
                            if ((unit->last_hit_by->unit_attrs & Unit::ATTR_MAGE) == 0) {
                                kill_sphere = unit->last_hit_by->hit_values.physical_damage_type;
                            } else if (unit->last_hit_spell_id != 0) {
                                kill_sphere = g_GameDataRes.spells.ElementAt(unit->last_hit_spell_id).Values().GetData()->sphere + 5;
                            }
                        }
                        if (g_Server->multiplayer != 0 && kill_sphere != 0) {
                            g_NetStru1_main.SendInfoPacket(KILL_NOTICE, unit->pOwner->player_id, unit->last_hit_by->pOwner->player_id | (kill_sphere << 16), nullptr);
                        }
                        if (g_ServerConfig.gameType == GAME_TYPE_COOPERATIVE) {
                            unit->last_hit_by->pOwner->player_kills++;
                        } else {
                            g_Server->ctf_score[unit->last_hit_by->pOwner->team]++;
                            unit->last_hit_by->pOwner->frags++;
                            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH) {
                                if (unit->last_hit_by->pOwner->frags < g_ServerConfig.frag_limit) {
                                    g_NetStru1_main.SendTeamState(0);
                                } else { 
                                    g_Server->SignalGameStart(1);
                                }
                            } else if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                                if (g_Server->ctf_score[unit->last_hit_by->pOwner->team] < g_ServerConfig.frag_limit) {
                                    g_NetStru1_main.SendTeamState(0);
                                } else {
                                    g_Server->SignalGameStart(1);
                                }
                            } else if (g_ServerConfig.gameType == GAME_TYPE_ARENA && g_Server->tick > 50) {
                                g_Server->AddArenaFrags(&unit->last_hit_by->pOwner->name, 1);
                                if (g_ServerConfig.frag_limit <= unit->last_hit_by->pOwner->frags) {
                                    g_Server->ResetArenaRound(1);
                                }
                            }
                        }
                    }
                    // Knowledge update for killer
                    g_NetStru1_main.SendUnitUpdate(unit->last_hit_by, unit->last_hit_by->pOwner, UNIT_UPD_KILL_STATS, EQUIP_SLOTS_ALL, 0, 0);
                    // Update kill stats
                    if (unit->server_id != 0 && unit->server_id < 0xa00) {
                        uint8_t* kills = &unit->last_hit_by->pOwner->kill_stats[unit->server_id];
                        if (*kills < 14) {
                            (*kills)++;
                            if ((*kills & 1) == 0) { // Opens a new monster stat every 2 kills.
                                g_NetStru1_main.SendKillStats(unit->server_id, unit->last_hit_by->pOwner);
                            }
                        }
                    }
                }
            }

            // Compute killer player ID for quest events
            uint32_t killer_pid = 0;
            if (unit->last_hit_by != NULL && unit->last_hit_by->pOwner != NULL && unit_type::hero_fighter_male <= unit->last_hit_by->typeId && unit->last_hit_by->typeId <= unit_type::humanoid_last) {
                killer_pid = unit->last_hit_by->pOwner->player_id;
            }

            // Quest event: unit death
            if (unit->summon_id == 0) {
                g_QuestMap.QueueEvent(Quest::EVENT_UNIT_KILLED, unit->building_id, (static_cast<int32_t>(MonsterTypeFace::Make(unit->typeId, unit->face).raw) << 16) | killer_pid);

            // Quest event: group death
            }
            if (group_id != 0) {
                g_QuestMap.QueueEvent(Quest::EVENT_GROUP_WIPED, group_id, killer_pid);
            }

            // Quest event: player has no more units alive
            if (unit->pOwner != NULL && unit->pOwner->quest_ai != 0 && unit->pOwner->unit_list->unit_list.GetCount() == 0) {
                g_QuestMap.QueueEvent(Quest::EVENT_INN_DEFENDERS_WIPED, unit->pOwner->player_id, killer_pid);

            // Clear pathfinding tracking
            }
            if (unit->building_id != 0 && unit->movement_type > 1) {
                g_buildingIdSet.Unset(unit->building_id);
                unit->building_id = 0;
            }
        }
    }
}

// 5561a1 (client: 535043)
void UnitList::SendVisibilityUpdates() {
    POSITION it = this->unit_list.GetHeadPosition();
    while (it != NULL) {
        Unit* unit = this->unit_list.GetNext(it);

        // Update the unit for a player if visibility tracking diverges
        if (unit->seen_by_mask != unit->seen_by_mask_prev && g_Server->multiplayer != 0) {
            for (int32_t i = 0; i < 16; i++) {
                if ((unit->seen_by_mask & (1 << i)) != 0 && (unit->seen_by_mask_prev & (1 << i)) == 0) {
                    Player* player = g_PlayersList->FindById(i + 16);
                    if (player != NULL) {
                        g_NetStru1_main.SendUnitUpdate(unit, player, unit->something_per_player[i], EQUIP_SLOTS_ALL, 0, 0);
                    }
                }
            }
            unit->seen_by_mask_prev = unit->seen_by_mask;
        }
    }
}

// Clear this player's vision-sharing bit on every unit in the list.
// 5579d8 (client: 535b23) - Clear unit vision mask bits for player across this list
void UnitList::ClearSentToMask(Player* player) {
    POSITION it = this->unit_list.GetHeadPosition();
    while (it) {
        Unit* unit = this->unit_list.GetNext(it);
        unit->sent_to_mask &= ~player->vision_sharing_id;
    }
}

// 5574b5 (client: 5358d9)
void UnitList::RegenTick() {
    // Tick all units in this list (call RegenTick on each)
    POSITION pos = this->unit_list.GetHeadPosition();
    while (pos != NULL) {
        Unit* unit = this->unit_list.GetNext(pos);
        unit->RegenTick();
    }

    // Process corpse decay on dead units
    CList<Unit*>& dead_list = g_Server->srv_stru1->units_list->unit_list;
    pos = dead_list.GetHeadPosition();
    while (pos != NULL) {
        Unit* unit = dead_list.GetNext(pos);
        unit->DecayTick();
    }

    // Count summons per summon_id in the summon pool
    CList<Unit*>& summon_pool = g_UnitPool->unit_list;
    pos = summon_pool.GetHeadPosition();

    if (pos == NULL) {
        return;
    }

    Unit* unit = summon_pool.GetNext(pos);

    CMap<uint32_t, uint32_t, int32_t, int32_t> map;
    while (unit != NULL) {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        if (unit->summon_id != 0 && unit->decay <= Unit::DECAY_UNCONSCIOUS && unit->some_state != Unit::EXEC_DEAD) {
            map[unit->summon_id]++;
        }
#else
        if (unit->summon_id != 0) {
            map[unit->summon_id]++;
        }
#endif
        if (pos == NULL) {
            unit = NULL;
        } else {
            unit = summon_pool.GetNext(pos);
        }
    }

    // Probabilistically mark summon groups for culling
    POSITION map_pos = map.GetStartPosition();
    while (map_pos != NULL) {
        uint32_t summon_id;
        int32_t count;
        map.GetNextAssoc(map_pos, summon_id, count);
        if (Random1N(100) < count) {
            map[summon_id] = -1;
        }
    }

    // Kill one unit per marked summon group
    pos = summon_pool.GetHeadPosition();
    unit = NULL;
    if (pos != NULL) {
        unit = summon_pool.GetNext(pos);
    }
    while (unit != NULL) {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        if (unit->summon_id != 0 && unit->decay <= Unit::DECAY_UNCONSCIOUS && unit->some_state != Unit::EXEC_DEAD) {
#else
        if (unit->summon_id != 0) {
#endif
            if (map[unit->summon_id] == -1) {
                map[unit->summon_id] = -2;
                unit->Die();
            }
        }
        if (pos == NULL) {
            unit = NULL;
        } else {
            unit = summon_pool.GetNext(pos);
        }
    }
}

// 557ab0 (client: 535b90) - Check if player has a rewarded unit
int UnitList::CountKillableMonsters() {
    int32_t count = 0;
    for (POSITION it = this->unit_list.GetHeadPosition(); it != NULL; ) {
        Unit* unit = this->unit_list.GetNext(it);
        if (unit->summon_id == 0 && unit->hp > -10 && unit_type::monster_first <= unit->typeId && unit->typeId <= unit_type::skeleton_mage_b) {
            // Ghost.1, F_Zombie.1 and F_Skeleton.1 are excluded. They can be "summoned" via "raise dead".
            if ((unit->typeId != unit_type::ghost || unit->face != 1) && unit->typeId != unit_type::zombie && unit->typeId != unit_type::skeleton) {
                count++;
            }
        }
    }
    return count;
}
