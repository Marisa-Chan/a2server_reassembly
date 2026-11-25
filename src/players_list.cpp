#include "players_list.h"
#include "player.h"

#include "building.h"
#include "buildings_list.h"
#include "effect.h"
#include "game_app.h"
#include "group.h"
#include "map_stuff.h"
#include "net.h"
#include "quest_map.h"
#include "sack.h"
#include "server.h"
#include "token.h"
#include "unit.h"
#include "unit_list.h"
#include "world.h"
#if defined(A2SERVER_PATCH) || defined(A2TANGAR_HAT)
#include "patch/player_info.h"
#endif

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
// Clear any remaining pOwner references on dead corpses, pooled summons,
// active units, buildings, or sacks before freeing `player` (dead pets and
// mercenaries were already detached from player->unit_list when they died).
static void DetachPlayerOwnedTokens(Player* player) {
    auto clear_unit_owner = [player](UnitList* ul) {
        if (ul == nullptr) {
            return;
        }
        for (POSITION unit_it = ul->unit_list.GetHeadPosition(); unit_it != nullptr;) {
            Unit* u = ul->unit_list.GetNext(unit_it);
            if (u != nullptr && u->pOwner == player) {
                u->pOwner = nullptr;
            }
        }
    };
    clear_unit_owner(g_ServerUnits);
    clear_unit_owner(g_UnitPool);
    if (g_Server != nullptr && g_Server->srv_stru1 != nullptr) {
        clear_unit_owner(g_Server->srv_stru1->units_list);
        if (g_Server->srv_stru1->building_list != nullptr) {
            for (POSITION building_it = g_Server->srv_stru1->building_list->GetHeadPosition(); building_it != nullptr;) {
                Building* bld = g_Server->srv_stru1->building_list->GetNext(building_it);
                if (bld != nullptr && bld->pOwner == player) {
                    bld->pOwner = nullptr;
                }
            }
        }
        if (g_Server->srv_stru1->sack_list != nullptr) {
            for (POSITION sack_it = g_Server->srv_stru1->sack_list->list.GetHeadPosition(); sack_it != nullptr;) {
                Sack* sack = g_Server->srv_stru1->sack_list->list.GetNext(sack_it);
                if (sack != nullptr && sack->pOwner == player) {
                    sack->pOwner = nullptr;
                }
            }
        }
    }
}
#endif


// 5357c6 (client: 520219) - AddTail: append player to list
void PlayersList::AddPlayer(Player* player) {
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
    if (!player->is_ai && g_Server->multiplayer != 0 && player->name != "Self") {
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
        g_World->diplomacy.RegisterPlayer(player);
    }
}

// Find player by numeric id.
// 535b50 (client: 520402) - find player by numeric id
Player* PlayersList::FindById(uint16_t player_id) {
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
// 535c46 (client: 520449) - GetByTokenID
Player* PlayersList::FindByTokenId(uint16_t token_id) {
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
// 536286 (client: none) - Reset all players' frags to 0
void PlayersList::ResetFrags() {
    for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
        Player* player = this->list.GetNext(it);
        player->frags = 0;
    }
}

// Returns max player_id in list.
// 535f97 (client: 52052f) - returns max player_id in list
int32_t PlayersList::GetMaxPlayerId() {
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
// 535d39 (client: 52048d) - find player by name (name compared against player->name)
Player* PlayersList::FindByName(CString name) {
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

// 535e94 (client: none) - find player by HAT character id (hat_char_id1==a && hat_char_id2==b)
Player* PlayersList::FindByHatCharId(int32_t a, int32_t b) {
    POSITION it = this->list.GetHeadPosition();
    Player* player = it ? this->list.GetNext(it) : nullptr;
    while (player != nullptr) {
        if (player->hat_char_id1 == a && player->hat_char_id2 == b) {
            return player;
        }

        player = it ? this->list.GetNext(it) : nullptr;
    }

    return nullptr;
}

// 53618f (client: 5205cf) - I guess it counts current human players?
int PlayersList::CountHumanPlayers() {

    if (this->list.IsEmpty()) {
        return 0;
    }

    int count = 0;

    for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
        if (this->list.GetNext(it)->is_ai == 0) {
            count++;
        }
    }
    return count;
}

// 536092 (client: 52057e)
int PlayersList::CountCD() {
    int count = 0;

    for (POSITION pos = this->list.GetHeadPosition(); pos != nullptr;) {
        if (this->list.GetNext(pos)->has_cd != 0) {
            count++;
        }
    }
    return count;
}

// 53636e (client: none) - checks if team play is already started
int PlayersList::AreTeamsStarted() {
    if (g_ServerConfig.gameType != GAME_TYPE_TEAMPLAY) {
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
        if (player->team_ready == 0) {
            return 0;
        }
        if (player->team == 0) {
            has_blue = 1;
        } else {
            has_red = 1;
        }
        has_players = 1;
    }

    return (has_players && has_blue && has_red) ? 1 : 0;
}

#ifndef A2CLIENT
// 534ddd (client: 51ff32)
void PlayersList::ProcessPlayerTimeouts() {
    POSITION pos = this->list.GetHeadPosition();
    Player* player = nullptr;

    if (pos != nullptr) {
        player = this->list.GetNext(pos);
    }

    while (player != nullptr) {
        // Inactivity timer check.
        if (player->afk_expire_tick != 0) {
            uint32_t now = GetTickCount();
            if (now > player->afk_expire_tick) {
                // Timer expired.
#ifdef A2SERVER_PATCH
                // part_of_camp hook (534E87): forced save upon AFK
                // deactivation ("autosave upon camping")
                g_Server->SaveCharacter(player->main_unit);
#endif
                player->Deactivate();
                player->drop_deadline_tick16--;
                player->afk_expire_tick = 0;
            } else if (now - player->afk_notify_tick >= 5000) {
                // Send countdown notification every 5 seconds.
                g_NetStru1_main.SendTeamNotify(0xE, (player->afk_expire_tick - now) / 1000, player);
                player->afk_notify_tick += 5000;
            }
        }

        // Decrement field_0xa68 if positive.
        if (player->shout_cooldown > 0) {
            player->shout_cooldown--;
        }

        // Check if player has timed out.
        if (0 < player->drop_deadline_tick16 && player->drop_deadline_tick16 < g_Server->tick16) {
#ifdef A2SERVER_PATCH
            // part_of_disconnect hook (534F4C): always save the character on
            // the 5-minute disconnect timeout (anti-dupe); the vanilla save
            // below is skipped (the hook jumps past it).
            if (player->main_unit != nullptr) {
                g_Server->SaveCharacter(player->main_unit);
            }
#endif
            if (player->is_in_world != 0) {
#ifndef A2SERVER_PATCH
                // Tournament mode: save player's main unit.
                if (g_Server->multiplayer != 0 && player->main_unit != nullptr) {
                    g_Server->SaveCharacter(player->main_unit);
                }
#endif
                g_QuestMap.RemovePlayerQuests(player->player_id);

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
                        MapStuff_Instance->AddUnitToMap(unit);

                        // Remove from global active unit list (g_ServerUnits).
                        POSITION found = g_ServerUnits->unit_list.Find(unit);
                        if (found != nullptr) {
                            g_ServerUnits->unit_list.RemoveAt(found);
                        }

                        unit->pOwner = nullptr;
                        unit->hp = -600;
                        unit->decay = Unit::DECAY_REMOVED;
#ifdef A2SERVER_PATCH
                        unit->summon_id = 0;
                        unit->some_state = Unit::EXEC_DEAD;
                        unit->state = Unit::ORDER_DEAD;
                        unit->enchantments = 0;
                        while (!unit->_effects.IsEmpty()) {
                            delete unit->_effects.RemoveHead();
                        }
                        if (unit->building_id != 0) {
                            BldIdSet_Unset(unit->building_id);
                            unit->building_id = 0;
                        }
                        if (g_UnitPool->unit_list.Find(unit) == nullptr) {
                            g_UnitPool->AddTail(unit);
                        }
#else
                        g_UnitPool->AddTail(unit);
#endif

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
#ifdef A2SERVER_PATCH
                        if (player->main_unit->building_id != 0) {
                            BldIdSet_Unset(player->main_unit->building_id);
                            player->main_unit->building_id = 0;
                        }
                        if (g_UnitPool->unit_list.Find(player->main_unit) == nullptr) {
                            g_UnitPool->AddTail(player->main_unit);
                        }
#endif
                    }
                }
            }

            // Team mode: handle rune holder leaving.
            if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                int32_t opposite_team = 1 - player->team;
                int32_t rune_holder = g_Server->ctf_carrying[opposite_team];
                if (rune_holder == player->player_id) {
                    g_Server->UpdateQuestRune(opposite_team, 0);
                    g_NetStru1_main.SendTeamState(0);
                    g_NetStru1_main.SendTeamNotify(0x101 - player->team, 0, nullptr);
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

            g_Server->HatKeepaliveTick();
            g_NetStru1_main.SendInfoPacket(PLAYER_LEFT, player->player_id, 0, nullptr);
#if defined(A2SERVER_PATCH) || defined(A2TANGAR_HAT)
            // imp_PIdel hook (535439): drop the per-player info, so no record keeps
            // a dangling Player* (ServerTic walks the records every tick).
            PI_Delete(player);
#endif
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
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            DetachPlayerOwnedTokens(player);
#endif
            delete player;

            g_Server->UpdateHatAnnounce();

            // Team mode: check if either team is now empty.
            if (g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                int32_t team_counts[2] = {0, 0};
                for (POSITION pp = this->list.GetHeadPosition(); pp != nullptr;) {
                    Player* p = this->list.GetNext(pp);
                    if (!p->is_ai) {
                        team_counts[p->team]++;
                    }
                }

                if (team_counts[0] == 0) {
                    g_Server->SignalGameStart(3);
                } else if (team_counts[1] == 0) {
                    g_Server->SignalGameStart(4);
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
#else
// 534ddd (client: 51ff32)
void PlayersList::ProcessPlayerTimeouts() {
    for (POSITION pos = list.GetHeadPosition(); pos; ) {
        Player* player = this->list.GetNext(pos);

        // Decrement field_0xa68 if positive.
        if (player->shout_cooldown > 0) {
            player->shout_cooldown--;
        }

        // Check if player has timed out.
        if (0 < player->drop_deadline_tick16 && player->drop_deadline_tick16 < g_Server->tick16) {
            if (player->is_in_world != 0) {
                // Tournament mode: save player's main unit.
                if (g_Server->multiplayer != 0 && player->main_unit != nullptr) {
                    g_Server->SaveCharacter(player->main_unit);
                }

                g_QuestMap.RemovePlayerQuests(player->player_id);

                for (POSITION upos = player->unit_list->unit_list.GetHeadPosition(); upos; ) {
                    Unit* unit = player->unit_list->unit_list.GetNext(upos);
                    MapStuff_Instance->AddUnitToMap(unit);

                    g_ServerUnits->Remove(unit);

                    unit->pOwner = nullptr;
                    unit->hp = -600;
                    unit->decay = Unit::DECAY_REMOVED;
                    g_UnitPool->AddTail(unit);
                }

                // Clear all groups.
                player->group_list->groups.RemoveAll();
            }

            if (!player->login.IsEmpty()) {
                char buf[260];
                sprintf(buf, "%schr\\%s\\lockflag", (const char*)g_ServerConfig.chr_base, (const char*)player->login);

                CFileStatus st;
                if (CFile::GetStatus(buf, st))
                    CFile::Remove(buf);
            }

            g_NetStru1_main.SendInfoPacket(PLAYER_LEFT, player->player_id, 0, nullptr);

            LogMessage("Player " + player->name + " disconnected.");

            // Decrement next_player_id if this was the last assigned.
            if (player->player_id == this->next_player_id - 1) {
                this->next_player_id--;
            }

            // Remove and delete player.
            POSITION found = this->list.Find(player);
            if (found != nullptr) {
                this->list.RemoveAt(found);
            }
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            DetachPlayerOwnedTokens(player);
#endif
            delete player;

            g_Server->UpdateHatAnnounce();
        }
    }
}
#endif

// 5b70f0 (client: 585150)
Player* PlayersList::GetHead() {
    if (this->list.IsEmpty()) {
        return nullptr;
    }
    return this->list.GetHead();
}

// 5B7110. Avoid this O(N^2) crap, use normal iteration over `list`.
// 5b7110 (client: 547970) - Avoid this O(N^2) crap, use normal iteration over `list`.
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

// 534c02 (client: 51feea)
PlayersList::~PlayersList() {
    for (POSITION it = this->list.GetHeadPosition(); it != nullptr;) {
        Player* pl = this->list.GetNext(it);
        if (pl) {
            delete pl;
        }
    }
    this->list.RemoveAll();
}

// 55afb3 (client: 536e8d) - Serialize
void PlayersList::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->next_player_id;
    } else {
        ar >> this->next_player_id;
    }

    g_Server->saved_ptr_map[nullptr] = nullptr;

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
