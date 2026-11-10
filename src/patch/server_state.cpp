#include "server_state.h"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <mutex>

#include "tangar_log.h"

#include "../game_app.h"

ServerState server_state;

const char* server_state_filename = "server_state.dat";

ServerState::ServerState() : map_index(0) {
}

bool ServerState::Save() {
    std::ofstream f(server_state_filename, std::ios::out | std::ios::trunc);
    if (!f) {
        Printf("[server_state] save: failed to open '%s' for writing: %s", server_state_filename, std::strerror(errno));
        return false;
    }

    f << this->map_index << '\n';

    // Save map order directly from the game's map array (0x6D15F0).
    f << "map_order " << g_ServerConfig.map_names.GetSize();
    for (int i = 0; i < g_ServerConfig.map_names.GetSize(); ++i) {
        f << ' ' << static_cast<const char*>(g_ServerConfig.map_names[i]);
    }
    f << '\n';

    // The player_settings_* lines of the original are written by the
    // player_settings feature; not yet ported.

    f.close();
    if (!f) {
        Printf("[server_state] save: failed to write to '%s': %s", server_state_filename, std::strerror(errno));
        return false;
    }
    return true;
}

std::mutex throttled_save_mutex;
time_t last_save_time = 0;

bool ServerState::ThrottledSave() {
    std::lock_guard<std::mutex> lock(throttled_save_mutex);
    if (time(NULL) - last_save_time < 60) {
        return false;
    }
    last_save_time = time(NULL);
    return this->Save();
}

bool ServerState::Load() {
    std::ifstream f(server_state_filename, std::ios::in);
    if (!f) {
        Printf("[server_state] load: failed to open '%s' for reading: %s", server_state_filename, std::strerror(errno));
        return false;
    }

    f >> this->map_index;

    std::string section;

    while (true) {
        f >> section;
        if (!f) {
            break;
        }

        if (section == "map_order") {
            int count;
            f >> count;

            if (!f) {
                Printf("[server_state] load: failed to read map count from '%s': %s", server_state_filename, std::strerror(errno));
                return false;
            }

            if (count < 0 || count > 1000) {
                Printf("[server_state] load: invalid map count %d in '%s'", count, server_state_filename);
                return false;
            }

            this->map_order.resize(count);
            for (int i = 0; i < count; ++i) {
                f >> this->map_order[i];
            }

            if (!f) {
                Printf("[server_state] load: failed to read map order from '%s': %s", server_state_filename, std::strerror(errno));
                return false;
            }
        } else if (section == "player_settings_autobuff_mask" || section == "player_settings_default_diplomacy") {
            // Written by the not-yet-ported player_settings feature; skip the
            // line so files from the full version still load.
            std::string player_name;
            time_t last_modified;
            uint32_t mask;
            f >> player_name >> last_modified >> mask;
        } else {
            Printf("[server_state] load: unknown section '%s' in '%s'", section.c_str(), server_state_filename);
            return false;
        }
    }

    return true;
}
