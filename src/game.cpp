#include <cstdint>
#include <cstring>

#include "game_app.h"
#include "main_window.h"
#include "net.h"
#include "player.h"
#include "players_list.h"
#include "server.h"
#include "unit.h"


extern "C"
{
    Player* __fastcall sub_496DD0(void* iter, int32_t edx, void* list);
    Player* __fastcall sub_496E20(void* iter);
};

void sub_4954EA()
{
    if (g_Server == nullptr) {
        return;
    }

    MainWindow* main_window = (MainWindow*)AfxGetMainWnd();
    CString unused;

    uint32_t iter_state[2] = {};

    int target_index = main_window->field_0x70c.GetCurSel();
    int index = 0;

    Player* selected = nullptr;
    Player* player = sub_496DD0(iter_state, 0, g_PlayersList);
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

    LogMessage("Player " + player->name + " kicked from server");

    uint16_t player_id = player->player_id;
    NetStru2* node = g_NetStru1_main.FUN_00518544(player_id);
    if (node == nullptr) {
        return;
    }

    g_NetStru1_main.FUN_005170b6(node);
    g_NetStru1_main.FUN_0051800f();
    g_NetStru1_main.FUN_0051d49b(player);

    player->field_0xa50 = g_Server->tick16 - 1;

    g_PlayersList->FUN_00534ddd();
}
