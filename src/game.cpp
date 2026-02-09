#include <cstdint>
#include <cstring>

#include "asm_mfc.h"
#include "player.h"
#include "players_list.h"
#include "server.h"
#include "unit.h"

extern "C" {
    extern Server* Server_instance;
    extern PlayersList* PlayersList_global;
    extern uint8_t byte_6C3A08; // NetStru1
    extern uint8_t unk_6CE560;

    void* __cdecl sub_401D90();
    void __fastcall unknown_libname_597(void* this_ptr);
    Player* __fastcall sub_496DD0(void* iter, int32_t edx, void* list);
    Player* __fastcall sub_496E20(void* iter);
    int __fastcall sub_496590(void* this_ptr);
    void* __fastcall sub_518544(void* this_ptr, int32_t edx, int id);
    void __fastcall sub_5170B6(void* this_ptr, int32_t edx, void* node);
    void __fastcall sub_51800F(void* this_ptr);
    void __fastcall sub_51D49B(void* this_ptr, int32_t edx, void* player);
    void __fastcall sub_534DDD(void* this_ptr);
    void __stdcall sub_43AA23(LPCTSTR message);
    void __fastcall sub_5186CD(void* this_ptr, int32_t edx, void* packet);
    void __fastcall sub_51CE86(void* this_ptr, int32_t edx, int msg, int id, Player* player);
    void __fastcall sub_534AC1(Player* this_ptr, int32_t edx, int param_1, int param_2);
    void __fastcall sub_51CEAC(void* this_ptr, int32_t edx, int msg, Player* player);
    void __fastcall sub_51C748(void* this_ptr, int32_t edx, int value);
    void __fastcall sub_5188DB(void* this_ptr);
}

void sub_4954EA()
{
    if (Server_instance == nullptr) {
        return;
    }

    uint8_t* main_window = static_cast<uint8_t*>(sub_401D90());
    CString unused;

    uint32_t iter_state[2] = {};

    int target_index = sub_496590(main_window + 0x70C);
    int index = 0;

    Player* selected = nullptr;
    Player* player = sub_496DD0(iter_state, 0, PlayersList_global);
    while (player != nullptr) {
        if (player->is_ai == 0) {
            if (index == target_index) {
                selected = player;
                break;
            }
            ++index;
        }

        player = sub_496E20(iter_state);
    }

    player = selected;
    if (player == nullptr) {
        return;
    }

    CString message = CString("Player ") + player->name + " kicked from server";
    sub_43AA23(message);

    uint16_t player_id = player->player_id;
    void* node = sub_518544(&byte_6C3A08, 0, player_id);
    if (node == nullptr) {
        return;
    }

    sub_5170B6(&byte_6C3A08, 0, node);
    sub_51800F(&byte_6C3A08);
    sub_51D49B(&byte_6C3A08, 0, player);
    player->field_0xa50 = Server_instance->tick16 - 1;
    sub_534DDD(PlayersList_global);
}

void __fastcall sub_4FF439(Server* server, int32_t /*edx*/, Player* player, int32_t arg4)
{
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
        auto* node = PlayersList_global->m_pNodeHead;
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

    uint8_t* packet = &unk_6CE560;
    auto* node = PlayersList_global->m_pNodeHead;
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
                    const char* source = player->name;
                    std::strcpy(reinterpret_cast<char*>(packet + 0x10), source);
                    packet[9] = 0x96;
                    packet[10] = static_cast<uint8_t>(player->player_id);
                    packet[11] = static_cast<uint8_t>(player->token_id);
                    packet[12] = static_cast<uint8_t>(player->field_0xa44 - 1);
                    uint8_t flags = 0;
                    if (phased_in != 0) {
                        flags |= 2;
                    }
                    if (player->is_ai != 0) {
                        flags |= 1;
                    }
                    packet[13] = flags;
                    *reinterpret_cast<uint16_t*>(packet + 7) = target->player_id;
                    sub_5186CD(&byte_6C3A08, 0, packet);
                    int msg_type = (phased_in != 0) ? 3 : 4;
                    sub_51CE86(&byte_6C3A08, 0, msg_type, player->player_id, target);
                }
            }
        }
        node = node->pNext;
    }

    CString message = CString("Player ") + player->name + " has joined the game";
    sub_43AA23(message);

    sub_534AC1(player, 0, 0, 1);
    if (server->field18_0x94 != 0) {
        sub_51CEAC(&byte_6C3A08, 0, 0xB7, player);
    }

    sub_51C748(&byte_6C3A08, 0, 0);
    if (server->field3_0x70 != 0) {
        sub_5188DB(&byte_6C3A08);
    }
}
