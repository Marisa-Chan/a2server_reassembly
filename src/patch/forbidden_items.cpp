// Ported from my-srvmgr/scroll_burn.cpp (item sets + CanEat/IsScroll).

#include "forbidden_items.h"

#include <set>

#include "../item.h"
#include "../unit.h"

namespace tangar_items {

// The following two sets are all rings and amulets. All custom items are
// supposed to be worn exclusively by wizards or fighters.
static const std::set<uint16_t> forbidden_wizard_items{
    54337, 54594, // Black Pearl
    54273, 54530, // Black Widow
    54465, 54722, // Crimson Red Crystal
    54305, 54562, // Flamer
    17473, 17730, // Golden Wrath
    54433, 54690, // Valkyr
};
static const std::set<uint16_t> forbidden_warrior_items{
    58561, 58818, // Aizen's Hogyoku
    54401, 54658, // Elf
    5249, 5506,   // Light Fantastic
    1153, 1410,   // Sanctuary Red
    50305, 50562, // Sea Witcher
    37985, 38242, // Shaman
    54369, 54626, // Vampic
};

bool CanEat(Unit* unit, Item* item) {
    if (item->item_id == 3667) { // Treasure.
        return false;
    }

    if ((unit->unit_attrs & 4) == 0) { // warrior
        if (forbidden_warrior_items.count(item->item_id) != 0) {
            return false;
        }
    } else {
        if (forbidden_wizard_items.count(item->item_id) != 0) {
            return false;
        }
    }

    return true;
}

bool IsScroll(Item* item) {
    return 3590 <= item->item_id && item->item_id <= 3647;
}

} // namespace tangar_items
