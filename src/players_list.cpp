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
#include "world.h"


// 5357C6
void PlayersList::sub_5357C6(Player* player) {
	// Build a bitmask of player_id values (1..31) already in use.
	uint32_t used_mask = 0;
	for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
		Player* other = this->list.GetNext(it);
		if (other->player_id < 32) {
			used_mask |= 1 << (other->player_id - 1);
		}
	}

	// AI players start at 1, human players start at 16.
	int32_t new_id = 1;
	if (!player->is_ai && g_Server->field4_0x74 != 0 && player->name != "Self") {
		new_id = 16;
	}

	while (new_id < 32) {
		if ((used_mask & (1u << (new_id - 1))) == 0) {
			player->player_id = new_id;
			if (this->next_player_id <= new_id) {
				this->next_player_id = new_id + 1;
			}
			break;
		}
		new_id++;
	}

	if (player->is_ai) {
		player->vision_sharing_id = 0;
	} else {
		if (player->player_id == 0) {
			LogMessage("Warning - no more free scanMask bit's");
			player->player_id = this->next_player_id;
			this->next_player_id = this->next_player_id + 1;
		}
		player->vision_sharing_id = 1 << (player->player_id % 16);
		player->vision_sharing_mask = player->vision_sharing_id;
		if (player->player_id > 32) {
			LogMessage("Warning - player number overflow 32");
		}
	}

	this->list.AddTail(player);

	if (g_World != nullptr) {
		g_World->diplomacy.sub_5B545F(player);
	}
}

// Find player by numeric id.
// 535B50
Player* PlayersList::sub_535B50(uint16_t player_id) {
	POSITION it = this->list.GetHeadPosition();
	Player* player = it ? this->list.GetNext(it) : nullptr;
	while (player != nullptr) {
		if (player->player_id == player_id) {
			return player;
		}

		player = it ? this->list.GetNext(it) : nullptr;
	}

	return nullptr;
}

// Find player by token id.
// 535C46
Player* PlayersList::sub_535C46(uint16_t token_id) {
	POSITION it = this->list.GetHeadPosition();
	Player* player = it ? this->list.GetNext(it) : nullptr;
	while (player != nullptr) {
		if (player->token_id == token_id) {
			return player;
		}

		player = it ? this->list.GetNext(it) : nullptr;
	}

	return nullptr;
}

// Reset all players' frags to 0.
// 536286
void PlayersList::sub_536286() {
	for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
		Player* player = this->list.GetNext(it);
		player->frags = 0;
	}
}

// Returns max player_id in list.
// 535F97
int32_t PlayersList::sub_535F97() {
	int32_t max_id = 0;
	for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
		Player* player = this->list.GetNext(it);
		if (player->player_id > max_id) {
			max_id = player->player_id;
		}
	}
	return max_id;
}

// Find player by exact name match.
// 535D39
Player* PlayersList::sub_535D39(CString name) {
	POSITION it = this->list.GetHeadPosition();
	Player* player = it ? this->list.GetNext(it) : nullptr;
	while (player != nullptr) {
		if (player->name == name) {
			return player;
		}

		player = it ? this->list.GetNext(it) : nullptr;
	}

	return nullptr;
}

// 535E94
Player* PlayersList::sub_535E94(int32_t a, int32_t b) {
	POSITION it = this->list.GetHeadPosition();
	Player* player = it ? this->list.GetNext(it) : nullptr;
	while (player != nullptr) {
		if (player->field_0xa80 == a && player->field_0xa84 == b) {
			return player;
		}

		player = it ? this->list.GetNext(it) : nullptr;
	}

	return nullptr;
}

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

// 53636E
int PlayersList::sub_53636E() {
	if (g_ServerConfig.gameType != 2) {
		return 1;
	}

	int32_t has_players = 0;
	int32_t has_blue = 0;
	int32_t has_red = 0;

	for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
		Player* player = this->list.GetNext(it);
		if (player->is_ai) {
			continue;
		}
		if (player->field_0xa6c == 0) {
			return 0;
		}
		if (player->field_0xa70 == 0) {
			has_blue = 1;
		} else {
			has_red = 1;
		}
		has_players = 1;
	}

	return (has_players && has_blue && has_red) ? 1 : 0;
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

// 5B70F0
Player* PlayersList::GetHead() {
	if (this->list.IsEmpty()) {
		return nullptr;
	}
	return this->list.GetHead();
}

// 5B7110. Avoid this O(N^2) crap, use normal iteration over `list`.
Player* PlayersList::GetNext(Player* player) {
	POSITION it = this->list.Find(player);
	if (it == nullptr) {
		return nullptr;
	}
	this->list.GetNext(it);
	if (it == nullptr) {
		return nullptr;
	}
	return this->list.GetAt(it);
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

// 55AFB3
void PlayersList::sub_55AFB3(CArchive& ar) {
	if (ar.IsStoring()) {
		ar << this->next_player_id;
	} else {
		ar >> this->next_player_id;
	}

	g_Server->field23_0xdc[nullptr] = nullptr;

	if (ar.IsStoring()) {
		ar << this->list.GetCount();
		for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
			Player* player = this->list.GetNext(it);
			ar.WriteObject(player);
		}
	} else {
		this->list.RemoveAll();
		int32_t count;
		ar >> count;
		for (int32_t i = 0; i < count; i++) {
			Player* player = nullptr;
			ar >> player;
			this->list.AddTail(player);
		}
	}
}
