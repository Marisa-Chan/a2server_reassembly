#pragma once

// Heroes from the local character files, drawn into the four oval medallions of
// the menunew.png start screen in place of the painted figures.
//
// Up to four *.a2c files from the client's directory are picked at random, in
// random order, one per medallion; a medallion left without one keeps its
// painted figure. A hero is the same paper-doll the info panel shows
// (CUnit::RenderHeroPortrait), laid onto the scroll the way the originals are:
// a per-medallion affine map, darkened, over the original backdrop with its
// figure painted out. Every two seconds another medallion with a hero lights up.
//
// The backdrops, frames and masks come from
// data\graphics\interface\mainmenu\medallions, and the allods2.cfg option
// "menu_medallions" (default true) switches the whole thing on and off;
// without the images nothing is drawn either way.

#include <stdint.h>

namespace tangar_menu_avatars {

void Load(); // pick the heroes and build their medallion pictures
void Draw(int32_t left, int32_t top); // left, top: where menunew.png is on screen

} // namespace tangar_menu_avatars
