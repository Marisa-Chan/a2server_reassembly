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

static const std::set<uint16_t> disabled_shop_items{
    0xD10F,
    0xD202,
    0xD401,
    0xD502,
    0xD606,
    0xD710,
    0xD812,
    0xD916,
    0xDA19,
    0xDC1D,
    0xD12F,
    0xD222,
    0xD421,
    0xD522,
    0xD626,
    0xD730,
    0xD832,
    0xD936,
    0xDA3A,
    0xDC3E,
    0xD1AF,
    0xD2A2,
    0xD4A1,
    0xD5A2,
    0xD6A6,
    0xD7B0,
    0xD8B2,
    0xD9B6,
    0xDABA,
    0xDCBE,
    0xD481,
    0xD582,
    0xD685,
    0xD78E,
    0xD88B,
    0xDA97,
    0xDC9B,
    0x9461,
    0x9562,
    0x9665,
    0x976E,
    0x986B,
    0x9A77,
    0x9C7B,
    0xD461,
    0xD562,
    0xD665,
    0xD76E,
    0xD86B,
    0xDA77,
    0xDC7B,
    0x414F,
    0x4245,
    0x4441,
    0x4542,
    0x4648,
    0x4751,
    0x4853,
    0x4956,
    0x4A5A,
    0x4C5E,
    0xD1CF,
    0xD2C5,
    0xD4C1,
    0xD5C2,
    0xD6C8,
    0xD7D1,
    0xD8D3,
    0xD9D6,
    0xDADA,
    0xDCDE,
    0xD14F,
    0xD245,
    0xD441,
    0xD542,
    0xD648,
    0xD751,
    0xD853,
    0xD956,
    0xDA5A,
    0xDC5E,
    0x1481,
    0x1582,
    0x1685,
    0x178E,
    0x188B,
    0x1A97,
    0x1C9B,
    0x0481,
    0x0582,
    0x0685,
    0x078E,
    0x088B,
    0x0A97,
    0x0C9B,
    0xC481,
    0xC582,
    0xC685,
    0xC78E,
    0xC88B,
    0xCA97,
    0xCC9B,
    0xE18D,
    0xE481,
    0xE582,
    0xE683,
    0xE78E,
    0xE88B,
    0xEA97,
    0xEC9B,
    0xD190,
    0x7190,
    0xC285,
    0xE265,
    0x2245,
    0x0000,
};


bool IsDisabled(uint16_t item_id) {
    return disabled_shop_items.count(item_id) != 0;
}
} // namespace tangar_items

