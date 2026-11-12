#pragma once

// Tangar stats-based server-level selection, ported from my-a2mgr gui_bugfix.cpp
// (StatsBasedLevelSelection + ParseServerNumber + the creating_new_character flag).
// Replaces the retail skill-tier brackets in VisHatBrowserList::IsItemEnabled.

#include <cstdint>

namespace tangar_stats {

// True while the char-gen stats panel is up (a new character is being created);
// cleared when a player file is parsed (an existing character is loaded).
extern bool creating_new_character;

// Extract the server number from a HAT server-list row
// ("|[6:38] #1 Mind < 15|..." -> 1). Returns 0 on any syntax problem.
int32_t ParseServerNumber(const char* row);

// True when the current character may enter the given server row. Fails open
// (returns true) when the feature is disabled or the row cannot be parsed.
bool StatsBasedLevelSelection(const char* row);

} // namespace tangar_stats
