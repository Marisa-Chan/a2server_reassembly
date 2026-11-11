#pragma once

// Tangar forbidden-items system, ported from my-srvmgr/scroll_burn.cpp.
// Certain rings/amulets are reserved per class; moving them from the bag onto
// the character is rejected (the item is forced back into the bag), and
// treasure (id 3667) can never be equipped.

class Item;
class Unit;

namespace tangar_items {

// True if the item may be put on by this unit.
bool CanEat(Unit* unit, Item* item);

// True for scroll item ids (3590..3647).
bool IsScroll(Item* item);

} // namespace tangar_items
