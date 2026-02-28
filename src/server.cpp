#include "server.h"

#include <cstring>

#include "game_app.h"
#include "world.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "players_list.h"
#include "net.h"
#include "map_stuff.h"
#include "spell_effect.h"
#include "mfc_plex.h"
#include "quest_map.h"


class QuestMap;

// ---- Global variables used by sub_4FC644 ----
extern "C" UnitList* dword_6CDB3C;  // pending-unit list

// ---- ASM subroutines called by sub_4FC644 ----
extern "C" void __fastcall sub_596131(ScanPresenceGrid* scan_presence_grid);

// ---- Variables used by sub_4FF937 ----
extern "C" QuestMap unk_6CE4D8; // Global quest map instance?
extern "C" PacketInfo unk_6D0788; // position-entry packet buffer.
extern "C" PacketInfo unk_6E9DB0;

// CRuntimeClass for AreaEffect (stru_6364B8 in Main.asm).
extern "C" CRuntimeClass stru_6364B8;

// Called when a player enters a map; streams the current game state to them.
// 4FF937
void Server::sub_4FF937(Player* player, int32_t bool_arg4)
{
    // Send existing players' PacketJoin data to the new player.
    g_NetStru1_main.sub_51C8B1(player);

    // Reconnect check?
    if (player->hat_player_id == 0xF6D04773 && player->flags == 4) {
        CString player_nick(player->name);

        int pipe_pos = player_nick.Find('|');
        if (pipe_pos != -1) {
            player_nick = player_nick.Mid(pipe_pos + 1);
        }

        Player* found_player = g_PlayersList->sub_535D39(player_nick);
        if (found_player != nullptr && dword_6CDB3C != nullptr) {
            // Transfer any unit from the original player to the re-connecting player.
            auto* node = dword_6CDB3C->unit_list.m_pNodeHead;
            while (node != nullptr) {
                auto* next = node->pNext;
                Unit* unit = node->data;
                if (unit != nullptr && unit->pOwner == found_player) {
                    g_NetStru1_main.FUN_004fb4ca(unit, player);
                }
                node = next;
            }
        }
    }

    // Arena mode: run the arena entry handler for this player.
    if (g_ServerConfig.gameType == 3) {
        sub_4FA551(player);
    }

    // Broadcast a PacketJoin for the new player to all existing players.
    {
        PacketJoin* pkt = new PacketJoin(current_map_name);
        pkt->id = 6;
        // [pkt+0xA] = Server::field21_0xd4.  Offset 0xA within PacketJoin is
        // the first uint32_t after the base Packet (Packet = 0xA bytes, pkt+0xA =
        // player_id of PacketJoin, but the ASM stores field21_0xd4 there as a dword).
        *reinterpret_cast<uint32_t*>(&pkt->player_id) = field21_0xd4;
        pkt->to_player_id = player->player_id;
        g_NetStru1_main.FUN_005186cd(pkt);
        delete pkt;
    }

    // Require the player to have a main unit.
    if (player->main_unit == nullptr) {
        LogMessage("Client " + player->name + " tries to enter mission without Hero. Return");
        g_NetStru1_main.FUN_0051cd89("You can't enter mission without Hero", player);
        return;
    }

    // Reset the rejoining flag.
    player->field_0x40 = 0;

    // Early-out for some game types.
    if (field4_0x74 != 0 && bool_arg4 != 0) {
        if (g_ServerConfig.gameType != 0 && g_ServerConfig.gameType != 3) {
            return;
        }
    }

    // Diplomacy and vision-mask setup with the "Self" map player.
    {
        Player* self_player = g_PlayersList->sub_535D39("Self");
        if (self_player != nullptr) {
            uint16_t self_id = self_player->player_id;
            uint16_t player_id = player->player_id;

            // Copy Self's diplomacy column into the new player's row (slots 2..15).
            for (int slot = 2; slot <= 15; ++slot) {
                g_World->diplomacy[slot][player_id]   = g_World->diplomacy[slot][self_id];
                g_World->diplomacy[self_id][slot] = g_World->diplomacy[player_id][slot];
            }
            // Mark Self as allied with the new player.
            g_World->diplomacy[self_id][player_id] = 0x12;

            // Walk all non-AI players and update alliance / vision masks.
            for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
                Player* p = node->data;
                auto p_id = p->player_id;
                if (p->is_ai != 0 || p_id == player_id) {
                    continue;
                }

                // Co-op: clear mutual alliance bytes.
                if (g_ServerConfig.gameType == 0) {
                    g_World->diplomacy[p_id][player_id] = 0;
                    g_World->diplomacy[player_id][p_id] = 0;
                }

                // Clear each player's vision bits for the other.
                player->vision_sharing_mask &= ~p->vision_sharing_id;
                p->vision_sharing_mask &= ~player->vision_sharing_id;
            }
        }
    }

    // Softcore (gameType == 2) team-alliance setup.
    if (g_ServerConfig.gameType == 2) {
        Player* red  = g_PlayersList->sub_535D39("Red");
        Player* blue = g_PlayersList->sub_535D39("Blue");

        uint16_t red_id = red->player_id;
        uint16_t blue_id = blue->player_id;
        uint16_t my_id = player->player_id;

        if (red != nullptr && blue != nullptr) {
            if (player->field_0xa70 == 0) {
                // Allied with Red (0x12), hostile to Blue (1).
                g_World->diplomacy[red_id][my_id] = 0x12;
                g_World->diplomacy[my_id][red_id] = 0x12;
                g_World->diplomacy[blue_id][my_id] = 1;
                g_World->diplomacy[my_id][blue_id] = 1;

                // Share vision with Red, clear from Blue.
                player->vision_sharing_mask |= red->vision_sharing_id;
                red->vision_sharing_mask |= player->vision_sharing_id;
                player->vision_sharing_mask &= ~blue->vision_sharing_id;
                blue->vision_sharing_mask &= ~player->vision_sharing_id;
            } else {
                // Allied with Blue (0x12), hostile to Red (1).
                g_World->diplomacy[blue_id][my_id] = 0x12;
                g_World->diplomacy[my_id][blue_id] = 0x12;
                g_World->diplomacy[red_id][my_id] = 1;
                g_World->diplomacy[my_id][red_id] = 1;

                // Share vision with Blue, clear from Red.
                player->vision_sharing_mask |= blue->vision_sharing_id;
                blue->vision_sharing_mask |= player->vision_sharing_id;

                player->vision_sharing_mask &= ~red->vision_sharing_id;
                red->vision_sharing_mask &= ~player->vision_sharing_id;
            }
        }
    }

    // Send updated terrain/diplomacy state to every non-AI player.
    for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
        Player* p = node->data;
        if (p != nullptr && p->is_ai == 0) {
            g_NetStru1_main.sub_51CB21(p);
        }
    }

    // If `bool_arg4` is set and field_0x41 is not yet set, put the unit on the map (sub_5013D4).
    if (bool_arg4 != 0 && player->field_0x41 == 0) {
        sub_5013D4(player);
    }

    // Mark the player as fully active and refresh world / map state.
    player->field_0x41 = 1;
    player->field_0x43 = 1;
    g_World->sub_5AFBFD(); // increment some counter (number of joined players?).
    sub_596131(&MapStuff_Instance->scan_presence_grid);  // refresh scan grid

    // Clear vision mask bits for this player across every unit list.
    // Three separate lists are processed:
    //   a) The global pending unit list (dword_6CDB3C).
    //   b) srv_stru1->sack_list — clears Token::field_x18 word bits (sub_554B03).
    //   c) srv_stru1->units_list.
    // Then for each player in g_PlayersList, also clear their per-player unit list.
    dword_6CDB3C->sub_5579D8(player);

    this->srv_stru1->sack_list->sub_554B03(player);
    this->srv_stru1->units_list->sub_5579D8(player);

    // Per-player unit lists.
    for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
        Player* p = node->data;
        if (p != nullptr) {
            p->unit_list->sub_5579D8(player);
        }
    }

    // Send player's main unit's position.
    unk_6D0788.packet.id = 0xABu;
    unk_6D0788.packet.to_player_id = player->player_id;
    unk_6D0788.field_0xa = player->main_unit->position->GetX() & 0xFF;
    unk_6D0788.field_0xe = player->main_unit->position->GetY() & 0xFF;

    g_NetStru1_main.FUN_005186cd(&unk_6D0788.packet);

    // Send all data for the main unit.
    g_NetStru1_main.sub_519221(player->main_unit, 0, -1, 0xFFB, 0, 0);

    // Stream remaining server state to the joining player.
    g_NetStru1_main.sub_51C0F7(player);          // send units from dword_6CDB3C
    g_NetStru1_main.sub_51CA5D(player);          // send server state
    g_NetStru1_main.sub_51D1A8(0, player);       // send kill stats
    g_NetStru1_main.sub_51CF5C(player->main_unit, 0, nullptr); // hero visibility packet

    // Send any active area effects to the player.
    if (this->srv_stru1->effects_list != nullptr) {
        CList<SpellEffect*>& effect_list = this->srv_stru1->effects_list->list;
        for (auto* node = effect_list.m_pNodeHead; node != nullptr; node = node->pNext) {
            SpellEffect* effect = node->data;
            if (effect == nullptr) {
                continue;
            }

            // Check runtime class: is this an AreaEffect?
            // TODO: also check `!effect->IsKindOf(AreaEffect::GetRuntimeClass())`.
            if (!effect->IsKindOf(&stru_6364B8)) {
                continue;
            }
            AreaEffect* ae = reinterpret_cast<AreaEffect*>(effect);
            if (ae->field_0x4c != 0) {
                g_NetStru1_main.sub_51BE8F(ae, 1);
            }
        }
    }

    // If field4_0x74 is set, send the SrvStru1 state list from the opaque
    // unk_6CE4D8 state buffer.  sub_51D4F6 uses unk_6CE4D8 as an internal
    // iterator struct (fields at offsets 0x74/0x78/0x80) and produces a
    // terrain-list packet that it sends to player.
    if (g_Server->field4_0x74 != 0) {
        g_NetStru1_main.sub_51D4F6(&unk_6CE4D8, player, 0);
    }

    // Encode and send the full map-terrain packet to the player.
    // encode_list is a stack-local CArray<uint16_t> (= CWordArray, var_88 in ASM).
    // MapStuff_Instance->sub_5948B0 fills it with run-length encoded terrain data.
    // The result is copied into unk_6E9DB0 (the large static packet buffer)
    // beginning at offset 0xE, then the packet is sent.
    {
        // Set up terrain packet header.
        unk_6E9DB0.packet.id = 0x9Bu;
        unk_6E9DB0.packet.to_player_id = player->player_id;

        CArray<uint16_t> encode_list;
        MapStuff_Instance->sub_5948B0(&encode_list);

        int32_t count = encode_list.GetSize();
        uint16_t* src_ptr = encode_list.GetData();
        std::memcpy(&unk_6E9DB0.field_0xe, src_ptr, count * 2);
        unk_6E9DB0.field_0xa = count;

        g_NetStru1_main.FUN_005186cd(&unk_6E9DB0.packet);
    }

    // Finalise the player's session state.
    player->FUN_00534AC1(0, 1);              // signal mission entry with arg=1
    g_NetStru1_main.FUN_0051ceac(3, player); // send "mission entered" event
    this->sub_4F4570();                      // server-state refresh
}


void Server::FUN_004ff439(Player* player, int32_t arg4)
{
    //4FF439
    if (player->main_unit == nullptr) {
        return;
    }

    int slot = (arg4 >> 8) & 0xFF;
    player->field_0xa50 = 0;
    player->field_0x42 = 1;

    auto main_unit = player->main_unit;
    int phased_in = ((main_unit->unit_attrs & 8) != 0) ? 1 : 0;
    if (slot == 0 && phased_in != 0) {
        slot = 1;
    }

    while (slot < 0x10) {
        bool found = false;
        auto* node = g_PlayersList->m_pNodeHead;
        while (node != nullptr) {
            Player* candidate = node->data;
            if (candidate != nullptr) {
                if (candidate->field_0xa44 == static_cast<uint8_t>(slot) && candidate->is_ai == 0) {
                    found = true;
                    break;
                }
            }
            node = node->pNext;
        }

        if (!found) {
            break;
        }
        ++slot;
    }

    if (slot != 0) {
        player->field_0xa44 = static_cast<uint8_t>(slot);
    }

    PacketJoin& packet = PacketJoin::Inst;
    auto* node = g_PlayersList->m_pNodeHead;
    while (node != nullptr) {
        Player* target = node->data;
        if (target != nullptr) {
            if (target->is_ai == 0 && target->field_0x42 != 0) {
                bool should_send = true;
                if (target != player) {
                    if (player->hat_player_id == 0xF6D04773 && player->flags == 4) {
                        should_send = false;
                    }
                }

                if (should_send) {
                    std::strcpy(packet.name, (LPCTSTR)player->name);
                    packet.id = 0x96;
                    packet.player_id = player->player_id;
                    packet.token_id = player->token_id;
                    packet.field_0xc = player->field_0xa44 - 1;
                    packet.flags = 0;

                    if (phased_in != 0)
                        packet.flags |= 2;
                    
                    if (player->is_ai != 0)
                        packet.flags |= 1;
                    
                    packet.to_player_id = target->player_id;

                    g_NetStru1_main.FUN_005186cd(&packet);

                    int msg_type = (phased_in != 0) ? 3 : 4;
                    g_NetStru1_main.FUN_0051ce86(msg_type, player->player_id, target);
                }
            }
        }
        node = node->pNext;
    }

    LogMessage("Player " + player->name + " has joined the game");

    player->FUN_00534AC1(0, 1);

    if (field18_0x94 != 0)
        g_NetStru1_main.FUN_0051ceac(0xB7, player);

    g_NetStru1_main.FUN_0051c748(nullptr);
    if (field3_0x70 != 0) {
        g_NetStru1_main.FUN_005188db();
    }
}

// sub_4FC644 — validate and process a player join request.
// Returns: 0=success, 1=server full, 2=duplicate name, 3=banned name,
//          4=name too short, 5=invalid char data, 6=too strong, 7=too weak,
//          8=team play started, 9=shutdown initiated.
// Calling convention: __thiscall (ecx=this=Server*), args are stdcall-pushed.
// `name` / `login` are CStrings passed by value (caller owns them; function may assign into them).
int Server::sub_4FC644(uint32_t pkt_word0, uint32_t pkt_word1,
                       CString name,          // player display name (in/out)
                       CString login,         // player login credential
                       void* Block,           // raw network packet (heap-allocated, ownership transferred here)
                       int32_t block_size,
                       int32_t team_id)
{
    Player* player = nullptr;

    // 1. Server full?
    if (g_PlayersList->CountHumanPlayers() >= 16) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Server full)");

        // ASM also checks `if (player != 0) {delete player}`, but player is always null at this point, so we skip that.

        if (Block != nullptr) {
            operator delete(Block);
        }
        return 1;
    }

    // 2. Team play already started?
    if (g_ServerConfig.gameType == 2 && g_PlayersList->sub_53636E()) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Team play already started)");
        if (Block != nullptr) {
            operator delete(Block);
        }
        return 8;
    }

    // 3. Shutdown initiated?
    if (!(g_ShutdownIn >= 0x7FFFFFFF || this->field59_0x208 != 0)) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Shutdown initiated)");
        if (Block != nullptr) {
            operator delete(Block);
        }
        return 9;
    }

    // 4. Duplicate name check
    for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
        Player* existing = node->data;
        if (existing == nullptr) {
            continue;
        }

        if (std::strcmp(name, existing->name) == 0) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Duplicated name)");
            if (Block != nullptr) {
                operator delete(Block);
            }
            return 2;
        }
    }

    // 5. Build working name (nickname) and password suffix (var_38)
    CString nickname(name);
    CString var_38;

    int pipe_pos = nickname.Find('|');
    if (pipe_pos != -1) {
        var_38 = nickname.Mid(pipe_pos + 1);
        nickname = nickname.Left(pipe_pos);
    }

    // 6. Banned name check
    for (int i = 0; i < g_ServerConfig.banned_names.GetSize(); ++i) {
        if (std::strcmp(nickname, g_ServerConfig.banned_names[i]) == 0) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Banned name)");
            if (Block != nullptr) {
                operator delete(Block);
            }
            return 3;
        }
    }

    // 7. Name starts with space -> banned
    if (nickname != nullptr && nickname[0] == ' ') {
        LogMessage("Player " + name + " login " + login + " has been rejected (Banned name)");
        if (Block != nullptr) {
            operator delete(Block);
        }
        return 3;
    }

    // --- 8. Name too short ---
    if (nickname.GetLength() <= 2) {
        LogMessage("Player " + name + " login " + login + " has been rejected (varName too short)");
        if (Block != nullptr) {
            operator delete(Block);
        }
        return 4;
    }

    // This is `player = new Player()`, but we haven't moved the constructor yet.
    player = new Player();

    Human* unit = nullptr;

    // Process network packet.
    // TODO: create the Block struct as a real struct.
    if (*reinterpret_cast<uint32_t*>(Block) != 0xFFDDAA11u) {
        // Normal packet path
        unit = sub_4EF4E7(Block, block_size, player);
        Block = nullptr;
    } else {
        // Magic packet 0xFFDDAA11
        uint8_t* pkt = reinterpret_cast<uint8_t*>(Block);
        unit = sub_500907(player, pkt[5], pkt[6], pkt[7], pkt[8], pkt[9], static_cast<uint32_t>(pkt[10]));

        // Copy 8 bytes from Block+0xC into player+0x10 (token_id / hat_player_id area)
        std::memcpy(reinterpret_cast<uint8_t*>(player) + 0x10, pkt + 0x0C, 8);

        player->name = name;
        player->money = 1000;

        operator delete(Block);
        Block = nullptr;
    }

    // Invalid character data?
    if (unit == nullptr) {
        LogMessage("Player " + name + " login " + login + " has been rejected (Invalid character data)");
        if (player != nullptr) {
            void** vtbl = *reinterpret_cast<void***>(player);
            typedef int(__thiscall* DtorFn)(Player*, int);
            reinterpret_cast<DtorFn>(vtbl[1])(player, 1);
        }
        if (Block != nullptr) {
            operator delete(Block);
        }
        return 5;
    }

    // Map level range checks.
    if (g_ServerConfig.gameType == 0 && g_ServerConfig.map_range_check != 0) {
        // [player+0xA88] = min_server_level; [this+0x1CC] = MapLevel
        if (this->MapLevel < player->min_server_level) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Too strong for this map)");
            if (player != nullptr) {
                void** vtbl = *reinterpret_cast<void***>(player);
                typedef int(__thiscall* DtorFn)(Player*, int);
                reinterpret_cast<DtorFn>(vtbl[1])(player, 1);
            }
            if (Block != nullptr) {
                operator delete(Block);
            }
            return 6;
        }
        // [player+0xA8C] = max_server_level
        if (this->MapLevel > player->max_server_level) {
            LogMessage("Player " + name + " login " + login + " has been rejected (Too weak for this map)");
            if (player != nullptr) {
                void** vtbl = *reinterpret_cast<void***>(player);
                typedef int(__thiscall* DtorFn)(Player*, int);
                reinterpret_cast<DtorFn>(vtbl[1])(player, 1);
            }
            if (Block != nullptr) {
                operator delete(Block);
            }
            return 7;
        }
    }

    // Normalize the player's stored name (var_34 / var_14 split).
    // Re-read name from the character data (player->name at +0x18, which was set by packet processing)
    CString var_34(player->name);
    CString var_14;

    int pipe_pos2 = var_34.Find('|');
    if (pipe_pos2 != -1) {
        var_14 = var_34.Mid(pipe_pos2 + 1);
        var_34 = var_34.Left(pipe_pos2);
    }

    // Replace leading space with '-'.
    if (var_34[0] == ' ') {
        var_34.SetAt(0, '-');
    }

    // Pad to at least length 3 with '-'.
    while (var_34.GetLength() < 3) {
        var_34 += '-';
    }

    // Rebuild name: if var_38 is non-empty, include suffix; otherwise just var_34.
    if (var_38.GetLength() != 0) {
        name = var_34 + '|' + var_38;
    } else {
        name = var_34;
    }

    // Set player fields.
    player->main_unit = unit;
    player->login = login;
    player->field_0xa80 = pkt_word0;
    player->field_0xa84 = pkt_word1;
    player->name = name;
    unit->name = name;
    player->is_ai = 0;
    player->field_0xa50 = this->tick16 + 300;
    player->field_0x42 = 0;

    // Team game flags.
    if (g_ServerConfig.gameType == 2) {
        player->field_0xa6c = 0;
        player->field_0xa70 = (team_id == 1) ? 1 : 0;
    }

    // Add player to global list.
    g_PlayersList->sub_5357C6(player);

    // Notify existing players (send join packets).
    sub_4EE028(unit);

    // Clear state fields.
    player->building_entered_from_yx = 0;
    player->field_0x40 = 0;

    // Initialize diplomacy.
    Player* self = g_PlayersList->sub_535D39("Self");
    if (self != nullptr) {
        int16_t partner_id = (int16_t)self->player_id;
        int16_t my_id      = (int16_t)player->player_id;

        for (int i = 2; i <= 15; ++i) {
            uint8_t val1 = g_World->diplomacy[i][partner_id];
            g_World->diplomacy[i][my_id] = val1;

            uint8_t val2 = g_World->diplomacy[partner_id][i];
            g_World->diplomacy[my_id][i] = val2;
        }

        g_World->diplomacy[partner_id][my_id] = 0x12;
    }

    // Set team pairing bytes and vision masks.
    for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
        Player* other = node->data;

        if (other != nullptr && other->is_ai == 0 && player->player_id != other->player_id) {
            int16_t other_id = other->player_id;
            int16_t my_id = player->player_id;

            switch (g_ServerConfig.gameType) {
            case 0:
                // Friendly: clear team slot bytes, remove from each other's vision mask
                g_World->diplomacy[other_id][my_id] = 0;
                g_World->diplomacy[my_id][other_id] = 0;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 1:
            case 3:
                // Deathmatch/arena: mark as enemy (1)
                g_World->diplomacy[other_id][my_id] = 1;
                g_World->diplomacy[my_id][other_id] = 1;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 2:
                // Team play
                if (player->field_0xa70 == other->field_0xa70) {
                    // Same team: mark 0x12 (teammate), share vision
                    g_World->diplomacy[other_id][my_id] = 0x12;
                    g_World->diplomacy[my_id][other_id] = 0x12;
                    player->vision_sharing_mask |= other->vision_sharing_id;
                    other->vision_sharing_mask |= player->vision_sharing_id;
                } else {
                    // Different team: mark 1 (enemy), clear vision
                    g_World->diplomacy[other_id][my_id] = 1;
                    g_World->diplomacy[my_id][other_id] = 1;
                    other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                    player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                }
                break;
            default:
                break;
            }
        }
    }

    // Player group setup.
    sub_5013D4(player);

    // Mark player as active.
    player->field_0x41 = 1;
    player->field_0x43 = 1;

    // Increment World::field65_0xc780
    g_World->sub_5AFBFD();

    // Notify MapStuff section
    sub_596131(&MapStuff_Instance->scan_presence_grid);

    // Add character unit to map.
    MapStuff_Instance->sub_58E3D1(player->main_unit);

    // Set unit_attrs |= 8 (phased in).
    player->main_unit->unit_attrs |= 8;

    // Co-op mode: set starting enchantments.
    if (g_ServerConfig.gameType == 0) {
        uint32_t& enchantments = player->main_unit->enchantments;
        enchantments |= 0x8000000u;
        enchantments |= 0x1000u;
        enchantments |= 0x10u;
        enchantments |= 0x100u;
        enchantments |= 0x2000u;
        enchantments |= 0x80000u;
    }

    // Arena mode: name/stat lookup.
    if (g_ServerConfig.gameType == 3) {
        sub_4FA4BB(&player->name, &player->frags);
        sub_4FA348(&player->name, 0);
    }

    // Server state update.
    sub_4F4570();

    // Remove player->main_unit from dword_6CDB3C->unit_list.
    {
        using CNode = CList<Unit*>::CNode;
        CList<Unit*>& lst = dword_6CDB3C->unit_list;
        Unit* target = player->main_unit;

        // Find the node whose data == target
        CNode* found = nullptr;
        for (CNode* node = lst.m_pNodeHead; node != nullptr; node = node->pNext) {
            if (node->data == target) {
                found = node;
                break;
            }
        }

        if (found != nullptr) {
            // Unlink
            if (found == lst.m_pNodeHead) {
                lst.m_pNodeHead = found->pNext;
            } else {
                found->pPrev->pNext = found->pNext;
            }

            if (found == lst.m_pNodeTail) {
                lst.m_pNodeTail = found->pPrev;
            } else {
                found->pNext->pPrev = found->pPrev;
            }

            // Recycle into free list
            found->pNext = lst.m_pNodeFree;
            lst.m_pNodeFree = found;
            --lst.m_nCount;

            // If list is now empty: release all CPlex blocks
            if (lst.m_nCount == 0) {
                lst.m_nCount = 0;
                lst.m_pNodeFree = nullptr;
                lst.m_pNodeTail = nullptr;
                lst.m_pNodeHead = nullptr;
                lst.m_pBlocks->FreeDataChain();
                lst.m_pBlocks = nullptr;
            }
        }
    }

    return 0;
}

void Server::FUN_004f94c0(int32_t arg) {
    //4F94C0
    LogMessage("Server::FUN_004f94c0\n");
    if ((g_ServerConfig.gameType == 1) || (g_ServerConfig.gameType == 2)) {
        g_NetStru1_main.FUN_0051d6b4(0);
    }
    g_NetStru1_main.FUN_0051cefb(0xc3, arg, 0, nullptr);
    this->field59_0x208 = 1;
}


ServerConfig::ServerConfig()
{
    //4f6fc9
    field_0x8 = 4;
    field_0x0 = 100;
    field_0x4 = 2;
    current_map_index = 0;
    field_0x8c = 0x100;
    field_0x90 = 0x78;
    field_0x98 = 1;
    field_0x9c = 0x10;
    server_name = "unnamed server";
    field_0xa0 = 0x3c;
    field_0xa4 = 1;
    gameType = 0;
    field_0xac = 0x7fffffff;
    map_range_check = 1;
    field_0xb8 = 5;
    field_0xbc = 0;
    field_0xc0 = 0x7fffffff;
    field_0xc4 = 100;
}
