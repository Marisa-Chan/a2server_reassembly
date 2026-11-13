// Map rotation countdown, state persistence, and self-restart hooks, ported
// from my-srvmgr/map_rotation.cpp (notify_about_map_rotation @ 48AA87,
// map_rotation @ 48ABA4).

#include "map_rotation.h"

#include "../game_app.h"
#include "../net.h"
#include "../server.h"
#include "config_new.h"
#include "server_state.h"
#include "tangar_log.h"

#include <cstdlib>
#include <string>

namespace {

void StopServer()
{
    std::exit(0);
}

void RestartProcess()
{
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    LPCWSTR raw_args = GetCommandLineW();
    std::wstring cmd_line(raw_args != nullptr ? raw_args : L"");

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    Log() << "Restarting server on map change...";

    BOOL created = CreateProcessW(
        path,
        cmd_line.empty() ? nullptr : &cmd_line[0],
        NULL, NULL,
        FALSE,
        0,
        NULL, NULL,
        &si, &pi
    );

    if (created) {
        Log() << "Restarting server: new instance created successfully, cleaning up current process";

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        ExitProcess(0);
    } else {
        Log() << "Restarting server: failed to create new process, error " << GetLastError() << ". Retaining the current process";
    }
}

} // namespace

// 48ABA4
void MapRotation()
{
    int map_index = static_cast<int>(g_ServerConfig.current_map_index);

    Log() << "Rolling maps, new index: " << map_index << " out of " << g_ServerConfig.map_durations.GetSize();

    server_state.map_index = map_index;
    server_state.Save();

    if (Config::server_restart_on_map_change) {
        RestartProcess();
        return;
    }

    if (!Config::server_rotate_maps && map_index == 0) {
        Log() << "Stopping server after the last map";
        StopServer();
    }
}

// 48AA87
void NotifyAboutMapRotation(int32_t remaining_ms, int32_t throttle_div_16)
{
    if (g_ServerConfig.gameType != GAME_TYPE_COOPERATIVE && g_ServerConfig.gameType != GAME_TYPE_ARENA) {
        return;
    }

    int remaining = remaining_ms / 1000;
    int throttle = throttle_div_16 << 4;

    // Vanilla logic: post "server will restart in X minutes" every minute for the last 10 minutes.
    if (1 < remaining && remaining < 660 && (remaining_ms % 60000) < throttle) {
        g_NetStru1_main.SendTeamNotify(0x0C, remaining, nullptr);
    }

    // Vanilla logic: post "server will restart in X seconds" every 15 sec for the minute.
    if (1 < remaining && remaining < 60 && (remaining_ms % 15000) < throttle) {
        g_NetStru1_main.SendTeamNotify(0x0C, remaining, nullptr);
    }

    // Post "server will restart in X seconds" every second for the last 10 seconds.
    if (remaining <= 10 && (remaining_ms % 1000) < throttle) {
        g_NetStru1_main.SendTeamNotify(0x0C, remaining, nullptr);
    }
}
