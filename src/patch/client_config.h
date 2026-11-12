#pragma once

// Tangar client configuration, ported from my-a2mgr config.{h,cpp}. Parses
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

// World-view scroll rate limiter (milliseconds between scroll steps; 0 = default 35).
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

// HAT address from the -hat command-line argument (default "hat.rom2.ru"),
// ported from my-a2mgr network_hat_enter.cpp ParseHatAddress.
const char* HatAddress();

} // namespace tangar_cfg
