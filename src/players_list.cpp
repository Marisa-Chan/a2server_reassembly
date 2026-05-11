#include "players_list.h"
#include "player.h"

#include "game_app.h"
#include "group.h"
#include "map_stuff.h"
#include "net.h"
#include "quest_map.h"
#include "server.h"
#include "unit.h"
#include "unit_list.h"


int PlayersList::CountHumanPlayers()
{
	//53618f

	if (list.IsEmpty())
		return 0;

	int count = 0;

	for (POSITION it = list.GetHeadPosition(); it != nullptr;)
	{
		if (list.GetNext(it)->is_ai == 0)
			count++;
	}
	return count;
}

int PlayersList::CountCD()
{
	//536092
	int count = 0;

	for (POSITION pos = list.GetHeadPosition(); pos != nullptr;)
	{
		if (list.GetNext(pos)->field_0xa45 != 0)
			count++;
	}
	return count;
}

// 534DDD
void PlayersList::sub_534DDD() {
	POSITION pos = list.GetHeadPosition();
	Player* player = nullptr;

	if (pos != nullptr) {
		player = list.GetNext(pos);
	}

	while (player != nullptr) {
		// Inactivity timer check.
		if (player->field_0xa90 != 0) {
			uint32_t now = GetTickCount();
			if (now > player->field_0xa90) {
				// Timer expired.
				player->FUN_00534778();
				player->field_0xa50--;
				player->field_0xa90 = 0;
			} else if (now - player->field_0xa94 >= 5000) {
				// Send countdown notification every 5 seconds.
				g_NetStru1_main.FUN_0051ce86(0xE, (player->field_0xa90 - now) / 1000, player);
				player->field_0xa94 += 5000;
			}
		}

		// Decrement field_0xa68 if positive.
		if (player->field_0xa68 > 0) {
			player->field_0xa68--;
		}

		// Check if player has timed out.
		if (0 < player->field_0xa50 && player->field_0xa50 < g_Server->tick16) {
			if (player->field_0x41 != 0) {
				// Tournament mode: save player's main unit.
				if (g_Server->field4_0x74 != 0 && player->main_unit != nullptr) {
					g_Server->sub_4EE028(player->main_unit);
				}

				g_QuestMap.sub_55ECFE(player->player_id);

				// Remove all units from map and move to dead pool.
				CList<Unit*>* unit_list = nullptr;
				if (player->unit_list != nullptr) {
					unit_list = &player->unit_list->unit_list;
				}

				if (unit_list != nullptr) {
					POSITION upos = unit_list->GetHeadPosition();
					Unit* unit = nullptr;
					if (upos != nullptr) {
						unit = unit_list->GetNext(upos);
					}

					while (unit != nullptr) {
						MapStuff_Instance->sub_58E3D1(unit);

						// Remove from global active unit list (dword_6CDB3C).
						POSITION found = dword_6CDB3C->unit_list.Find(unit);
						if (found != nullptr) {
							dword_6CDB3C->unit_list.RemoveAt(found);
						}

						unit->pOwner = nullptr;
						unit->hp = -600;
						unit->decay = 5;
						dword_6B37C4->AddTail(unit);

						// Advance to next unit.
						if (upos != nullptr) {
							unit = unit_list->GetNext(upos);
						} else {
							unit = nullptr;
						}
					}
				}

				// Clear all groups.
				player->group_list->groups.RemoveAll();

				// Remove main_unit from server's units_list.
				if (player->main_unit != nullptr) {
					UnitList* srv_units = g_Server->srv_stru1->units_list;
					POSITION found = srv_units->unit_list.Find(player->main_unit);
					if (found != nullptr) {
						srv_units->unit_list.RemoveAt(found);
					}
				}
			}

			// Team mode: handle rune holder leaving.
			if (g_ServerConfig.gameType == 2) {
				int32_t opposite_team = 1 - player->field_0xa70;
				int32_t rune_holder = g_Server->ctf_carrying[opposite_team];
				if (rune_holder == player->player_id) {
					g_Server->sub_4F8FBF(opposite_team, 0);
					g_NetStru1_main.FUN_0051d6b4(0);
					g_NetStru1_main.FUN_0051ce86(0x101 - player->field_0xa70, 0, nullptr);
				}
			}

			// Add login to FileList if not already present.
			uint32_t already_in_list = 0;
			for (int32_t i = 0; i < g_Server->FileList.GetSize(); i++) {
				if (strcmp(g_Server->FileList[i], player->login) == 0) {
					already_in_list = 1;
				}
			}
			if (!already_in_list) {
				g_Server->FileList.SetAtGrow(g_Server->FileList.GetSize(), player->login);
			}

			g_Server->sub_4F0ECF();
			g_NetStru1_main.FUN_0051cefb(0x97, player->player_id, 0, nullptr);
			LogMessage("Player " + player->name + " has left the game");

			// Decrement next_player_id if this was the last assigned.
			if (player->player_id == this->next_player_id - 1) {
				this->next_player_id--;
			}

			// Remove and delete player.
			POSITION found = this->list.Find(player);
			if (found != nullptr) {
				this->list.RemoveAt(found);
			}
			delete player;

			g_Server->sub_4F4570();

			// Team mode: check if either team is now empty.
			if (g_ServerConfig.gameType == 2) {
				int32_t team_counts[2] = {0, 0};
				for (POSITION pp = this->list.GetHeadPosition(); pp != nullptr;) {
					Player* p = this->list.GetNext(pp);
					if (!p->is_ai) {
						team_counts[p->field_0xa70]++;
					}
				}

				if (team_counts[0] == 0) {
					g_Server->FUN_004f94c0(3);
				} else if (team_counts[1] == 0) {
					g_Server->FUN_004f94c0(4);
				}
			}
		}

		// Advance to next player.
		if (pos != nullptr) {
			player = this->list.GetNext(pos);
		} else {
			player = nullptr;
		}
	}
}

PlayersList::~PlayersList()
{ //534c02
	for (POSITION it = list.GetHeadPosition(); it != nullptr;)
	{
		Player* pl = list.GetNext(it);
		if (pl)
			delete pl;
	}
	list.RemoveAll();
}