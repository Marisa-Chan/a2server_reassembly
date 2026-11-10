#include "kick_char.h"

#include <string>

#include "flags.h"

#include "../game_app.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

// Port of the manager's zxmgr::Kick. The kick notice is suppressed for
// silent kicks and for invisible GMs (GMF_INVISIBLE), matching the patched
// server; the rest matches the earlier kick_char port.
void KickPlayer(Player* player, bool silent) {
    if (player == nullptr || player->is_ai) {
        return;
    }

    g_Server->SaveCharacter(player->main_unit);

    NetStru2* node = g_NetStru1_main.GetClientByPlayerID(player->player_id);
    if (node == nullptr) {
        return;
    }

    g_NetStru1_main.AnnouncePlayer(player);
    g_NetStru1_main.ProcessConnections();

    if (!silent && (player->flags & GMF_INVISIBLE) != GMF_INVISIBLE) {
        g_NetStru1_main.AnnouncePlayer(player);
    }
    player->drop_deadline_tick16 = g_Server->tick16 - 1;
    g_PlayersList->ProcessPlayerTimeouts();
}

// Port of zxmgr::KickAll / KickAllSilent: kick every non-AI player except
// `except_player` (may be null).
void TangarKickAll(Player* except_player, bool silent) {
    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* other = g_PlayersList->list.GetNext(pos);
        if (other != nullptr && other != except_player && !other->is_ai) {
            KickPlayer(other, silent);
        }
    }
}

// Port of zxmgr::Disconnect: save the character and drop the connection
// without the kick notice or the immediate player-list cleanup.
void DisconnectPlayer(Player* player) {
    if (player == nullptr || player->is_ai) {
        return;
    }

    g_Server->SaveCharacter(player->main_unit);

    NetStru2* node = g_NetStru1_main.GetClientByPlayerID(player->player_id);
    if (node == nullptr) {
        return;
    }

    g_NetStru1_main.AnnouncePlayer(player);
    g_NetStru1_main.ProcessConnections();
}

// Port of zxmgr::FindByNickname: match a player by name; AI names are stored
// in CP-866 and get converted before comparing.
Player* FindPlayerByNickname(const char* nickname) {
    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* other = g_PlayersList->list.GetNext(pos);
        if (other == nullptr) {
            continue;
        }
        std::string pl_nickname = (const char*)other->name;
        if (other->is_ai) {
            // convert to cp-866
            for (size_t i = 0; i < pl_nickname.length(); i++) {
                uint8_t ch = pl_nickname[i];
                if (ch == 0xA8) {
                    ch = 0xF0;
                } else if (ch == 0xB8) {
                    ch = 0xF1;
                } else if (ch >= 0xC0 && ch <= 0xEF) {
                    ch -= 0x40;
                } else if (ch >= 0xF0 && ch <= 0xFF) {
                    ch -= 0x10;
                }
                pl_nickname[i] = (char)ch;
            }
        }

        if (pl_nickname == nickname) {
            return other;
        }
    }
    return nullptr;
}
