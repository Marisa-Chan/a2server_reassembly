#pragma once

// GM world-manipulation, spellbook, and item-construction helpers ported from
// my-srvmgr/zxmgr.cpp, cheat_codes_new.cpp, and cheat_codes.cpp.

#include <cstdint>
#include <string>
#include <vector>

class Item;
class Player;
class Unit;

namespace tangar_gm {

// Construct an item from an item definition string (supporting "{effect=...}",
// "{teachSpell=...}", material/shape prefixes, etc.) via GameDataRes::FindItemTemplate.
Item* ConstructItem(const std::string& definition);

// Read/replace the unit's learned spell bitmask (bits 1..29).
uint32_t GetSpells(Unit* unit);
void SetSpells(Unit* unit, uint32_t spells);

// Drop a sack containing an optional constructed item and/or gold at (x, y).
void CreateSack(const char* item_definition, uint8_t x, uint8_t y, int money);

// #murder helper: in cooperative mode, drop all learned spells as spellbook
// sacks (and all gold when full == true) at the unit's coordinates.
void DropEverything(Unit* unit, bool full);

// zxmgr::Kill / zxmgr::KillAll: set hp = -50, last_hit_by = nullptr, and
// broadcast SendUnitUpdate, sparing caster->main_unit.
void KillPlayerUnits(Player* target, Player* caster);
void KillAllUnits(Player* caster, bool ai_only);

// Transfer all non-main_unit units owned by `from` to `to`.
void OwnUnits(Player* from, Player* to);

// Parse space-delimited arguments supporting ', ", ` quotes and \ escapes.
// Returns false on malformed trailing backslash.
bool ParseSpaceDelimited(const std::string& str, std::vector<std::string>& out);

} // namespace tangar_gm
