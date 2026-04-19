#include "kick_char.h"

#include "../game_app.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

void KickPlayer(Player* player)
{
    if (player == nullptr || player->is_ai) {
        return;
    }

    g_Server->sub_4EE028(player->main_unit);

    NetStru2* node = g_NetStru1_main.GetClientByPlayerID(player->player_id);
    if (node == nullptr) {
        return;
    }

    g_NetStru1_main.DisconnectClient(node);
    g_NetStru1_main.ProcessConnections();

    // WAT: original patch also has `|| ((*(uint *)(extraout_EDX + 0x14) & 0x800) == 0`, but `EDX` is unset. Looks like a typo?
    if ((player->flags & 0x3f000000) != 0x3f000000) {
        g_NetStru1_main.FUN_0051d49b(player);
    }
    player->field_0xa50 = g_Server->tick16 - 1;
    g_PlayersList->sub_534DDD();
}
