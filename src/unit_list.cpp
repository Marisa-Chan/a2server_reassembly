#include "unit_list.h"
#include "unit.h"
#include "player.h"
#include "effect.h"
#include "group.h"
#include "item.h"
#include "game_app.h"
#include "players_list.h"
#include "quest_map.h"

//555a1d
UnitList::UnitList() = default;

//555a8d
UnitList::~UnitList() = default;

void UnitList::Serialize(CArchive& ar)
{
	//558ef8
	if (ar.IsStoring())
	{
		ar.WriteCount(unit_list.GetCount());

		POSITION pos = unit_list.GetHeadPosition();
		while (pos != NULL)
		{
			Unit *unit = unit_list.GetNext(pos);
			ar.WriteObject(unit);
		}
	}
	else
	{
		DWORD nNewCount = ar.ReadCount();
		Unit* unit;
		while (nNewCount--)
		{
			ar >> unit;
			unit_list.AddTail(unit);
		}
	}
}

void UnitList::CallMethod5()
{
	//5591da
	POSITION pos = unit_list.GetHeadPosition();
	while (pos != NULL)
	{
		Unit* unit = unit_list.GetNext(pos);
		unit->VMethod5();
	}
}

void UnitList::AddTail(Unit* unit)
{
	//555c1b
	unit_list.AddTail(unit);
}

void UnitList::AddTailAllocId(Unit* unit)
{
	//555d67
	unit_list.AddTail(unit);
	unit->building_id = g_buildingIdSet.AllocBit();
}

void UnitList::AddTailId6xxx(Unit* unit)
{
	//555f1c
	unit_list.AddTail(unit);
	unit->building_id = 0x6000 + unit->TokenID;
}

// 556345
void UnitList::ProcessTick()
{
	POSITION pos = unit_list.GetHeadPosition();
	while (pos != NULL) {
		POSITION cur_pos = pos;
		Unit* unit = unit_list.GetNext(pos);

		// Validate last_hit_by: clear if killer has no owner
		if (unit->last_hit_by != NULL && unit->last_hit_by->pOwner == NULL) {
			unit->last_hit_by = NULL;
		}

		// Send knowledge flag updates to players when visibility tracking diverges
		if (unit->field_0x1a4 != unit->field_0x1a6 && g_Server->field4_0x74 != 0) {
			for (int i = 0; i < 16; i++) {
				if ((unit->field_0x1a4 & (1 << i)) != 0 && (unit->field_0x1a6 & (1 << i)) == 0) {
					Player* player = g_PlayersList->sub_535B50(i + 16);
					if (player != NULL) {
						g_NetStru1_main.sub_519221(unit, player, unit->something_per_player[i], 0xffb, 0, 0);
					}
				}
			}
			unit->field_0x1a6 = unit->field_0x1a4;
		}

		// Tick alive unit
		if (unit->some_state != 0x10) {
			unit->VMethod2();
		}

		// Process dead unit
		if (unit->some_state == 0x10) {
			// Capture group quest ID before removal
			int32_t group_id = 0;
			if (unit->group != NULL && unit->group->unit_list.GetCount() == 1 && (unit->group->has_quest_intercept != 0 || unit->group->has_quest_kill != 0)) {
				group_id = unit->group->group_id;
			}

			// Notify owner player and remove unit from player's list
			if (unit->pOwner != NULL) {
				unit->pOwner->sub_5347A1(unit);
			}

			// Remove from this list and add to dead units pool
			unit_list.RemoveAt(cur_pos);
			g_Server->srv_stru1->units_list->AddTail(unit);

			// Clean up effects
			POSITION epos = unit->_effects.GetHeadPosition();
			while (epos != NULL) {
				POSITION ecur_pos = epos;
				Effect* effect = unit->_effects.GetNext(epos);
				effect->spell_value = 1;
				effect->caster = NULL;
				effect->VMethod10(unit);
				if (effect->usage_type & 0x80) {
					unit->_effects.RemoveAt(ecur_pos);
					delete effect;
				}
			}

			// Kill attribution
			if (unit->last_hit_by != NULL) {
				if (unit->last_hit_by->typeId < 0x21 || unit->last_hit_by->typeId > 0x3f) {
					// Killer is NOT a monster type
					if (0x20 < unit->typeId && unit->typeId < 0x40 && unit->last_hit_by->VMethod7() != 0 && unit->last_hit_by->monster_info != NULL) {
						CString msg;
						uint8_t y = unit->position->GetY();
						uint8_t x = unit->position->GetX();
						msg.Format("Player %s killed by %s ID=%d at %d,%d", (const char*)unit->pOwner->name, (const char*)unit->last_hit_by->monster_info->name, unit->last_hit_by->building_id, x, y);
						LogMessage(msg);
						if (g_ServerConfig.gameType != 0) {
							if (g_ServerConfig.gameType == 2 && unit->last_hit_by->pOwner != nullptr) {
								if (unit->last_hit_by->pOwner->name == "Red" && unit->pOwner->field_0xa70 == 1) {
									g_Server->field57_0x200++;
								} else if (unit->last_hit_by->pOwner->name == "Blue" && unit->pOwner->field_0xa70 == 0) {
									g_Server->field58_0x204++;
								} else {
									(&g_Server->field57_0x200)[unit->pOwner->field_0xa70]--;
									unit->pOwner->frags--;
								}
							} else {
								(&g_Server->field57_0x200)[unit->pOwner->field_0xa70]--;
								unit->pOwner->frags--;
							}
							if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
								g_NetStru1_main.FUN_0051cefb(0x94, unit->pOwner->player_id, 0xb0000, nullptr);
								g_NetStru1_main.FUN_0051d6b4(0);
							} else if (g_ServerConfig.gameType == 3 && g_Server->tick > 50) {
								g_NetStru1_main.FUN_0051cefb(0x94, unit->pOwner->player_id, 0xb0000, nullptr);
								g_Server->sub_4FA348(&unit->pOwner->name, -1);
							}
						}
					}
				} else if (unit->last_hit_by->hp < 0) {
					// Killer (a monster) is also dead — mutual kill
					if (unit->pOwner->is_ai == 0) {
						CString msg;
						msg.Format("Player %s has died.", (const char*)unit->pOwner->name);
						LogMessage(msg);
					}
					unit->last_hit_by = nullptr;
					if (g_ServerConfig.gameType != 0 && unit->pOwner->is_ai == 0 && g_Server->tick > 50) {
						unit->pOwner->frags--;
						(&g_Server->field57_0x200)[unit->pOwner->field_0xa70]--;
						g_NetStru1_main.FUN_0051cefb(0x94, unit->pOwner->player_id, 0xc0000, nullptr);
						if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
							g_NetStru1_main.FUN_0051d6b4(0);
						} else {
							g_Server->sub_4FA348(&unit->pOwner->name, -1);
						}
					}
				} else {
					// Normal pkill --- killer (a monster) is alive
					unit->last_hit_by->VMethod16(unit);
					unit->last_hit_by->VMethod22(unit, unit->last_hit_spell_id);
					if (unit->typeId < 0x21 || unit->typeId > 0x3f) {
						// Victim is a monster
						unit->last_hit_by->pOwner->monster_kills++;
					} else {
						// Victim is a human
						CString msg;
						uint8_t y = unit->position->GetY();
						uint8_t x = unit->position->GetX();
						msg.Format("Player %s pkilled by %s at %d,%d", (const char*)unit->pOwner->name, (const char*)unit->last_hit_by->pOwner->name, x, y);
						LogMessage(msg);
						uint32_t kill_sphere = 0;
						if (g_ServerConfig.gameType != 0) {
							if ((unit->last_hit_by->unit_attrs & 4) == 0) {
								kill_sphere = unit->last_hit_by->hit_values.physical_damage_type;
							} else if (unit->last_hit_spell_id != 0) {
								kill_sphere = g_GameDataRes.spells.ElementAt(unit->last_hit_spell_id).values.GetData()->sphere + 5;
							}
						}
						if (g_Server->field4_0x74 != 0 && kill_sphere != 0) {
							g_NetStru1_main.FUN_0051cefb(0x94, unit->pOwner->player_id, unit->last_hit_by->pOwner->player_id | (kill_sphere << 16), nullptr);
						}
						if (g_ServerConfig.gameType == 0) {
							unit->last_hit_by->pOwner->player_kills++;
						} else {
							(&g_Server->field57_0x200)[unit->last_hit_by->pOwner->field_0xa70]++;
							unit->last_hit_by->pOwner->frags++;
							if (g_ServerConfig.gameType == 1) {
								if (unit->last_hit_by->pOwner->frags < g_ServerConfig.frag_limit) {
									g_NetStru1_main.FUN_0051d6b4(0);
								} else { 
									g_Server->FUN_004f94c0(1);
								}
							} else if (g_ServerConfig.gameType == 2) {
								if ((&g_Server->field57_0x200)[unit->last_hit_by->pOwner->field_0xa70] < g_ServerConfig.frag_limit) {
									g_NetStru1_main.FUN_0051d6b4(0);
								} else {
									g_Server->FUN_004f94c0(1);
								}
							} else if (g_ServerConfig.gameType == 3 && g_Server->tick > 50) {
								g_Server->sub_4FA348(&unit->last_hit_by->pOwner->name, 1);
								if (g_ServerConfig.frag_limit <= unit->last_hit_by->pOwner->frags) {
									g_Server->sub_4FA78E(1);
								}
							}
						}
					}
					// Knowledge update for killer
					g_NetStru1_main.sub_519221(unit->last_hit_by, unit->last_hit_by->pOwner, 0x2000000, 0xffb, 0, 0);
					// Update kill stats
					if (unit->server_id != 0 && unit->server_id < 0xa00) {
						uint8_t* kills = &unit->last_hit_by->pOwner->kill_stats[unit->server_id];
						if (*kills < 14) {
							(*kills)++;
							if ((*kills & 1) == 0) { // Opens a new monster stat every 2 kills.
								g_NetStru1_main.sub_51D1A8(unit->server_id, unit->last_hit_by->pOwner);
							}
						}
					}
				}
			}

			// Compute killer player ID for quest events
			uint32_t killer_pid = 0;
			if (unit->last_hit_by != NULL && unit->last_hit_by->pOwner != NULL && 0x21 <= unit->last_hit_by->typeId && unit->last_hit_by->typeId <= 0x3f) {
				killer_pid = unit->last_hit_by->pOwner->player_id;
			}

			// Quest event: unit death
			if (unit->summon_id == 0)
				g_QuestMap.sub_55DD10(4, unit->building_id, unit->typeId << 16 | unit->face << 24 | killer_pid);

			// Quest event: group death
			if (group_id != 0) {
				g_QuestMap.sub_55DD10(5, group_id, killer_pid);
			}

			// Quest event: player has no more units alive
			if (unit->pOwner != NULL && unit->pOwner->field_0xa60 != 0 && unit->pOwner->unit_list->unit_list.GetCount() == 0)
				g_QuestMap.sub_55DD10(15, unit->pOwner->player_id, killer_pid);

			// Clear pathfinding tracking
			if (unit->building_id != 0 && unit->movement_type > 1) {
				g_buildingIdSet.Unset(unit->building_id);
				unit->building_id = 0;
			}
		}
	}
}

// 5574B5
void UnitList::sub_5574B5() {
	// Tick all units in this list (call VMethod1 on each)
	POSITION pos = this->unit_list.GetHeadPosition();
	while (pos != NULL) {
		Unit* unit = this->unit_list.GetNext(pos);
		unit->VMethod1();
	}

	// Process corpse decay on dead units
	CList<Unit*>& dead_list = g_Server->srv_stru1->units_list->unit_list;
	pos = dead_list.GetHeadPosition();
	while (pos != NULL) {
		Unit* unit = dead_list.GetNext(pos);
		unit->sub_52E7FA();
	}

	// Count summons per summon_id in the summon pool
	CList<Unit*>& summon_pool = dword_6B37C4->unit_list;
	pos = summon_pool.GetHeadPosition();

	if (pos == NULL) {
		return;
	}

	Unit* unit = summon_pool.GetNext(pos);

	CMap<uint32_t, uint32_t, int32_t, int32_t> map;
	while (unit != NULL) {
		if (unit->summon_id != 0) {
			map[unit->summon_id]++;
		}
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
		if (unit->summon_id != 0) {
			if (map[unit->summon_id] == -1) {
				map[unit->summon_id] = -2;
				unit->sub_52D94E();
			}
		}
		if (pos == NULL) {
			unit = NULL;
		} else {
			unit = summon_pool.GetNext(pos);
		}
	}
}
