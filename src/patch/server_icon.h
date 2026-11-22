#pragma once

#include <stdint.h>
#include <windows.h>

namespace server_icon {

// Peek ServerID from -cfg"..." (or server.cfg) before MainWindow is created,
// and assign a per-ServerID AppUserModelID for taskbar identity.
uint32_t InitProcessIdentity(const char* cmd_line);

// Set ICON_BIG, ICON_SMALL, and window-class icons on `hwnd`, drawing
// `server_id` onto the base Allods server icon when `server_id > 0`.
void Apply(HWND hwnd, uint32_t server_id);

} // namespace server_icon
