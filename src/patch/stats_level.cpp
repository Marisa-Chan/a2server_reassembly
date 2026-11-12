// Tangar stats-based server-level selection, ported from my-a2mgr gui_bugfix.cpp.
// The character stats come from the player-file parse buffer (client 0x6995a8 twin,
// see GetParsedPlayerStats in player_file.cpp).

#include "stats_level.h"

#include <cstring>

#include "client_config.h"
#include "../player_file.h"

namespace tangar_stats {

bool creating_new_character = false;

int32_t ParseServerNumber(const char* row) {
    // Row example (sent by HAT, see CLCMD_SendServerList):
    //   |[6:38] #1 Mind < 15|1.02|Road to Plagat|64x64|1|0|127.0.0.1:8001|
    // The level is the "#N" inside the first section.
    if (row[0] != '|' || row[1] != '[') {
        return 0;
    }

    const char* second_pipe = strchr(row + 2, '|');
    if (second_pipe == nullptr) {
        return 0;
    }

    const char* hash = strstr(row, "] #");
    if (hash == nullptr) {
        return 0;
    }
    if (hash > second_pipe) {
        return 0;
    }

    const char* level_ptr = hash + 3;
    if (*level_ptr < '1' || *level_ptr > '9') {
        return 0;
    }

    int32_t server_number = 0;
    while (*level_ptr >= '0' && *level_ptr <= '9') {
        server_number = server_number * 10 + (*level_ptr - '0');
        level_ptr++;
    }
    return server_number;
}

bool StatsBasedLevelSelection(const char* row) {
    if (!tangar_cfg::stats_based_level_selection) {
        return true;
    }

    int32_t server_number = ParseServerNumber(row);
    if (server_number == 0) {
        return true; // fail open
    }

    if (creating_new_character) {
        // New characters go to the first server.
        return server_number == 1;
    }

    FileSectionStats* stats = GetParsedPlayerStats();
    switch (server_number) {
        case 1:
            return stats->mind < 15;
        case 2:
            return stats->mind >= 15 && stats->reaction < 20;
        case 3:
            return stats->reaction >= 20 && stats->reaction < 30;
        case 4:
            return stats->reaction >= 30 && stats->reaction < 40;
        case 5:
            return stats->reaction >= 40 && (stats->reaction < 50 || stats->mind < 50 || stats->spirit < 50);
        default:
            return stats->reaction >= 50 && stats->mind >= 50 && stats->spirit >= 50;
    }
}

} // namespace tangar_stats
