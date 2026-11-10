#pragma once

// Tangar server state persistence (map rotation), ported from
// my-srvmgr/server_state.{h,cpp}.
//
// The original also persisted per-player settings (autobuff masks, default
// diplomacy) in the same file; those sections return here together with the
// player_settings feature port. Files written by the full version are still
// parsed correctly: the player_settings_* lines are skipped on load.

#include <string>
#include <vector>

struct ServerState {
    int map_index;
    std::vector<std::string> map_order;

    ServerState();

    bool Save();
    bool ThrottledSave(); // Saves at most once per minute.
    bool Load();
};

extern ServerState server_state;
