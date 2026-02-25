#include "game_app.h"
#include "server.h"
#include "world.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "players_list.h"
#include "net.h"
#include "map_stuff.h"
#include "mfc_plex.h"
#include <cstring>

// ---- Global variables used by sub_4FC644 ----
extern "C" World*   dword_6A8B8C;   // TODO: g_World
extern "C" UnitList* dword_6CDB3C;  // pending-unit list
extern "C" CStringArray unk_6D15DC;  // banned names list
extern "C" int32_t  dword_6D1654;   // map-level range check enabled flag

// ---- ASM subroutines called by sub_4FC644 ----
extern "C" void __fastcall sub_596131(void* map_section); // map stuff reset/init section


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
    if (g_GameType == 2 && g_PlayersList->sub_53636E()) {
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
    for (int i = 0; i < unk_6D15DC.GetSize(); ++i) {
        if (std::strcmp(nickname, unk_6D15DC[i]) == 0) {
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
    void* raw = operator new(0xAA0);
    Player* new_player_raw = reinterpret_cast<Player*>(raw);
    player = (new_player_raw != nullptr) ? new_player_raw->sub_534118() : nullptr;

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
    if (g_GameType == 0 && dword_6D1654 != 0) {
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
    if (g_GameType == 2) {
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
            uint8_t val1 = dword_6A8B8C->diplomacy[i][partner_id];
            dword_6A8B8C->diplomacy[i][my_id] = val1;

            uint8_t val2 = dword_6A8B8C->diplomacy[partner_id][i];
            dword_6A8B8C->diplomacy[my_id][i] = val2;
        }

        dword_6A8B8C->diplomacy[partner_id][my_id] = 0x12;
    }

    // Set team pairing bytes and vision masks.
    for (auto* node = g_PlayersList->m_pNodeHead; node != nullptr; node = node->pNext) {
        Player* other = node->data;

        if (other != nullptr && other->is_ai == 0 && player->player_id != other->player_id) {
            int16_t other_id = other->player_id;
            int16_t my_id = player->player_id;

            switch (g_GameType) {
            case 0:
                // Friendly: clear team slot bytes, remove from each other's vision mask
                dword_6A8B8C->diplomacy[other_id][my_id] = 0;
                dword_6A8B8C->diplomacy[my_id][other_id] = 0;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 1:
            case 3:
                // Deathmatch/arena: mark as enemy (1)
                dword_6A8B8C->diplomacy[other_id][my_id] = 1;
                dword_6A8B8C->diplomacy[my_id][other_id] = 1;
                other->vision_sharing_mask &= (uint16_t)(~other->vision_sharing_id);
                player->vision_sharing_mask &= (uint16_t)(~player->vision_sharing_id);
                break;
            case 2:
                // Team play
                if (player->field_0xa70 == other->field_0xa70) {
                    // Same team: mark 0x12 (teammate), share vision
                    dword_6A8B8C->diplomacy[other_id][my_id] = 0x12;
                    dword_6A8B8C->diplomacy[my_id][other_id] = 0x12;
                    player->vision_sharing_mask |= other->vision_sharing_id;
                    other->vision_sharing_mask |= player->vision_sharing_id;
                } else {
                    // Different team: mark 1 (enemy), clear vision
                    dword_6A8B8C->diplomacy[other_id][my_id] = 1;
                    dword_6A8B8C->diplomacy[my_id][other_id] = 1;
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
    dword_6A8B8C->sub_5AFBFD();

    // Notify MapStuff section
    sub_596131(MapStuff_Instance->field_0x92ecc);

    // Add character unit to map.
    MapStuff_Instance->sub_58E3D1(player->main_unit);

    // Set unit_attrs |= 8 (phased in).
    player->main_unit->unit_attrs |= 8;

    // Co-op mode: set starting enchantments.
    if (g_GameType == 0) {
        uint32_t& enchantments = player->main_unit->enchantments;
        enchantments |= 0x8000000u;
        enchantments |= 0x1000u;
        enchantments |= 0x10u;
        enchantments |= 0x100u;
        enchantments |= 0x2000u;
        enchantments |= 0x80000u;
    }

    // Arena mode: name/stat lookup.
    if (g_GameType == 3) {
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
    if ((g_GameType == 1) || (g_GameType == 2)) {
        g_NetStru1_main.FUN_0051d6b4(0);
    }
    g_NetStru1_main.FUN_0051cefb(0xc3, arg, 0, nullptr);
    this->field59_0x208 = 1;
}
