#pragma once

// Tangar client configuration, ported from a2mgr config.{h,cpp}. Parses
// allods2.cfg (new 2x format: "key value" lines) or allods2.conf (old 1x
// format: "key=value" lines). Only the knobs consumed by ported client
// features are carried; screenshots/PNG/net-key knobs return with those
// subsystems.

#include <cstdint>

namespace tangar_cfg {

// Overlay toggles.
extern bool r_fps;
extern bool r_time;
extern bool r_stopwatch;

// Stopwatch state (Right Shift toggles/freeze/restart in-game).
extern bool show_stopwatch;
extern uint32_t show_stopwatch_frozen;
extern int64_t stopwatch_start; // time_t of the last stopwatch restart

// World-view scroll rate limiter (milliseconds between scroll steps; default 25).
extern uint32_t g_scrollintv;

// Tile-grid overlay.
extern bool r_grid;
extern uint32_t r_grid_color;

// Language override: "russian" / "english" (empty = auto from system).
extern bool z_lang_override;
extern bool z_russian;

// Stats-based server-level selection brackets in the HAT browser.
extern bool stats_based_level_selection;

// Forced game mode for the HAT connect dialog (0 = keep dropdown choice).
extern int32_t z_softcore;

// Parse allods2.cfg / allods2.conf if present. Safe to call every startup.
void ParseConfig();

// Chat: prefix chat messages with their type (say/private/yell).
extern bool g_altchat;

// Screenshots: put screenshot files into the screenshots subdirectory.
extern bool z_screendir;

// Client logs: put log files into the logs subdirectory (default true).
extern bool z_logsdir;

// Initialize Config::LogFile for the client ("logs\\allods2_DD-MM-YYYY[.pid].log"
// when z_logsdir is enabled, else "allods2[.pid].log") and write startup header.
void InitClientLogFile();

// Screen resolution from allods2.cfg (r_width / r_height when both set).
extern uint32_t r_width;
extern uint32_t r_height;
extern uint32_t z_resolution;

// Windowed mode from allods2.cfg ("window true"), equivalent to the -window CLI flag.
extern bool window;

// True when windowed mode was requested via allods2.cfg ("window true") or -window on the CLI.
bool IsWindowed();

// Resolve initial client resolution (allods2.cfg r_width/r_height or desktop
// smart default, overridden by -640/-800/-1024/--res:WxH CLI flags).
void GetResolution(int32_t* w, int32_t* h);

// HAT address from the -hat command-line argument (default "hat.rom2.ru"),
// ported from a2mgr network_hat_enter.cpp ParseHatAddress.
const char* HatAddress();

// Write a message to the in-game chat HUD (msglog.Add, 30s), falling back to
// LogMessage if the playing area is not yet constructed (zxmgr::WriteChat).
void WriteChat(const char* msg);

} // namespace tangar_cfg
