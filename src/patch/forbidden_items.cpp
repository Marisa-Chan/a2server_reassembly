// Ported from srvmgr/scroll_burn.cpp (item sets + CanEat/IsScroll).

#include "forbidden_items.h"

#include <set>

#include "../item.h"
#include "../unit.h"

namespace tangar_items {

// The following two sets are all rings and amulets. All custom items are
// supposed to be worn exclusively by wizards or fighters.
static const std::set<uint16_t> forbidden_wizard_items{
    ItemId::MakeEquip(material::bone, item_slot::ring,   item_shape::rare,     1), // 54337: Black Pearl
    ItemId::MakeEquip(material::bone, item_slot::amulet, item_shape::rare,     2), // 54594
    ItemId::MakeEquip(material::bone, item_slot::ring,   item_shape::common,   1), // 54273: Black Widow
    ItemId::MakeEquip(material::bone, item_slot::amulet, item_shape::common,   2), // 54530
    ItemId::MakeEquip(material::bone, item_slot::ring,   item_shape::good,     1), // 54465: Crimson Red Crystal
    ItemId::MakeEquip(material::bone, item_slot::amulet, item_shape::good,     2), // 54722
    ItemId::MakeEquip(material::bone, item_slot::ring,   item_shape::uncommon, 1), // 54305: Flamer
    ItemId::MakeEquip(material::bone, item_slot::amulet, item_shape::uncommon, 2), // 54562
    ItemId::MakeEquip(material::gold, item_slot::ring,   item_shape::rare,     1), // 17473: Golden Wrath
    ItemId::MakeEquip(material::gold, item_slot::amulet, item_shape::rare,     2), // 17730
    ItemId::MakeEquip(material::bone, item_slot::ring,   item_shape::bad,      1), // 54433: Valkyr
    ItemId::MakeEquip(material::bone, item_slot::amulet, item_shape::bad,      2), // 54690
};
static const std::set<uint16_t> forbidden_warrior_items{
    ItemId::MakeEquip(material::crystal,        item_slot::ring,   item_shape::good,      1), // 58561: Aizen's Hogyoku
    ItemId::MakeEquip(material::crystal,        item_slot::amulet, item_shape::good,      2), // 58818
    ItemId::MakeEquip(material::bone,           item_slot::ring,   item_shape::elven,     1), // 54401: Elf
    ItemId::MakeEquip(material::bone,           item_slot::amulet, item_shape::elven,     2), // 54658
    ItemId::MakeEquip(material::bronze,         item_slot::ring,   item_shape::elven,     1), // 5249:  Light Fantastic
    ItemId::MakeEquip(material::bronze,         item_slot::amulet, item_shape::elven,     2), // 5506
    ItemId::MakeEquip(material::iron,           item_slot::ring,   item_shape::elven,     1), // 1153:  Sanctuary Red
    ItemId::MakeEquip(material::iron,           item_slot::amulet, item_shape::elven,     2), // 1410
    ItemId::MakeEquip(material::dragon_leather, item_slot::ring,   item_shape::elven,     1), // 50305: Sea Witcher
    ItemId::MakeEquip(material::dragon_leather, item_slot::amulet, item_shape::elven,     2), // 50562
    ItemId::MakeEquip(material::magic_wood,     item_slot::ring,   item_shape::very_rare, 1), // 37985: Shaman
    ItemId::MakeEquip(material::magic_wood,     item_slot::amulet, item_shape::very_rare, 2), // 38242
    ItemId::MakeEquip(material::bone,           item_slot::ring,   item_shape::very_rare, 1), // 54369: Vampic
    ItemId::MakeEquip(material::bone,           item_slot::amulet, item_shape::very_rare, 2), // 54626
};

bool CanEat(Unit* unit, Item* item) {
    if (item->item_id == ItemId::QUEST_TREASURE) { // Treasure.
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
    return item->item_id.IsScroll();
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
