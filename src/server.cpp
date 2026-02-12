#include "game_app.h"
#include "server.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "players_list.h"
#include "net.h"


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
                    std::strcpy(packet.name, player->name);
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
