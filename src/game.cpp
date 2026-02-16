#include <cstdint>
#include <cstring>
#include <windows.h>

#include "game_app.h"
#include "main_window.h"
#include "net.h"
#include "player.h"
#include "players_list.h"
#include "server.h"
#include "unit.h"
#include "util.h"

void sub_4954EA()
{
    if (g_Server == nullptr) {
        return;
    }

    MainWindow* main_window = (MainWindow*)AfxGetMainWnd();
    CString unused;

    int target_index = main_window->list_box2.GetCurSel();
    int index = 0;

    Player* selected = nullptr;

    Listerator<Player> it;
    Player* player = it.Begin(*g_PlayersList);
    while (player != nullptr) {
        if (player->is_ai == 0) {
            if (index == target_index) {
                selected = player;
                break;
            }
            ++index;
        }

        player = it.GetNext();
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
