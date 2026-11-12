#pragma once

// Tangar custom main menu, ported from my-a2mgr menu.cpp (hooks MENU_load /
// MENU_draw / MENU_hover1-3; MENU_click was behaviorally a no-op in the
// release patch — its DoMenuClick experiment is dead code).

#include <stdint.h>

class VisMainMenu;

namespace tangar_menu {

void Load(); // MENU_load (client 4AEDF5): replaces VisMainMenu::LoadGraphics
void Draw(); // MENU_draw (client 4AF2C8): replaces VisMainMenu::VMethod7
// MENU_hover1/2/3 (client 4AF8E6 / 4AF8C3 / 4AFA5B): custom hit-testing that
// replaces VisMainMenu::UpdateButtonState in the three mouse handlers.
void Hover(VisMainMenu* menu, int32_t x, int32_t y, uint32_t buttons);

} // namespace tangar_menu
