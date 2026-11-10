#pragma once

// Tangar server state persistence (map rotation), ported from
// my-srvmgr/server_state.{h,cpp}. Also persists per-player settings
// (autobuff masks, default diplomacy) in the same file.

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
