// Ported from my-srvmgr/pvm2.cpp (vd2_CheckStrong + spell/scroll/item
// queries). Field mapping vs the DLL: value1 -> damage_min, value2 ->
// damage_spread (the Effect union at +0x40), amount -> count, option ->
// value1 on Item.

#include "pvm2.h"

#include <cstdint>

#include "../effect.h"
#include "../inventory.h"
#include "../item.h"
#include "../player.h"
#include "../spell.h"
#include "../unit.h"

namespace tangar_pvm2 {

namespace {

const uint32_t vd2_sp1[] = {1, 2, 4, 5, 8, 10, 13, 16, 19, 20, 21, 24, 26, 27}; // level "easy"
const uint32_t vd2_sp2[] = {3, 6, 7, 11, 12, 17, 23, 25};                       // level "hard"
const uint32_t vd2_sp3[] = {18, 22};                                            // level "horror"

// Returns count of spells in the book that belong to the specified level.
uint32_t QuerySpells(Unit* character, uint32_t level) {
    if (character == nullptr || level == 0 || level > 3) {
        return 0;
    }

    SpellBook* spell_book = character->spell_book;
    if (spell_book == nullptr) {
        return 0;
    }
    const uint32_t* checkwith = nullptr;
    uint32_t checksize = 0;

    if (level == 1) {
        checkwith = vd2_sp1;
        checksize = sizeof(vd2_sp1) / sizeof(uint32_t);
    } else if (level == 2) {
        checkwith = vd2_sp2;
        checksize = sizeof(vd2_sp2) / sizeof(uint32_t);
    } else if (level == 3) {
        checkwith = vd2_sp3;
        checksize = sizeof(vd2_sp3) / sizeof(uint32_t);
    }

    uint32_t retval = 0;
    for (uint32_t i = 0; i < checksize; i++) {
        uint32_t spell = checkwith[i];
        if (spell >= 32) {
            continue;
        }
        if (spell_book->GetSpell((int32_t)spell) != nullptr) {
            retval++;
        }
    }

    return retval;
}

// Returns the biggest stack of scrolls of the specified level (elven =
// superscrolls).
uint32_t QueryScrolls(Unit* character, uint32_t level, bool elven) {
    if (character == nullptr || level == 0 || level > 3) {
        return 0;
    }
    if (character->inventory == nullptr) {
        return 0;
    }

    const uint32_t* checkwith = nullptr;
    uint32_t checksize = 0;

    if (level == 1) {
        checkwith = vd2_sp1;
        checksize = sizeof(vd2_sp1) / sizeof(uint32_t);
    } else if (level == 2) {
        checkwith = vd2_sp2;
        checksize = sizeof(vd2_sp2) / sizeof(uint32_t);
    } else if (level == 3) {
        checkwith = vd2_sp3;
        checksize = sizeof(vd2_sp3) / sizeof(uint32_t);
    }

    uint32_t retval = 0;

    for (POSITION pos = character->inventory->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = character->inventory->items.GetNext(pos);
        if (item == nullptr) {
            continue;
        }

        uint8_t item_class = item->shape_id;
        uint8_t item_material = item->material_id;
        uint16_t item_option = (uint8_t)item->value1;
        uint8_t item_slot = static_cast<Armor*>(item)->slot;

        if (item_slot == 0 && item_class == 0 && item_material == 0 && item->weight == 1) {
            // that is, slot = 14
            item_slot = 14;
        }

        uint16_t iid = item_material & 0xF;
        iid <<= 4;
        iid |= item_slot & 0xF;
        iid <<= 8;
        iid |= (item_class & 0x7) << 5;
        iid |= item_option;

        uint32_t item_count = item->count;

        if ((iid & 0xF000) == 0 && (iid & 0x0E00) == 0x0E00) { // scroll
            bool spell_match = false;
            uint32_t kindof = iid & 0x00FF;
            if (((kindof >= 6 && kindof <= 34) && !elven) ||   // scroll
                ((kindof >= 35 && kindof <= 63) && elven)) {   // superscroll
                kindof -= 5;
                if (elven) {
                    kindof -= 29;
                }

                for (uint32_t i = 0; i < checksize; i++) {
                    if (checkwith[i] == kindof) {
                        spell_match = true;
                        break;
                    }
                }
            }

            if (spell_match && item_count > retval) {
                retval = item_count;
            }
        }
    }

    return retval;
}

// True if the item belongs to the specified tier (1 easy / 2 hard / 3 horror).
bool CheckItemLevel(Item* item, uint32_t level) {
    uint8_t item_class = item->shape_id;
    uint8_t item_material = item->material_id;
    uint16_t item_option = (uint8_t)item->value1;
    uint8_t item_slot = static_cast<Armor*>(item)->slot;

    uint32_t item_level = 0;

    // special case: (null)
    if (item_class == 0 && item_material == 0 && item_option == 0 && item_slot == 0) {
        return false;
    }

    if (item_slot == 0 && item_class == 0 && item_material == 0 && item->weight == 1) {
        // that is, slot = 14
        item_slot = 14;
    }

    if (item_slot == 14) {
        return false; // don't check scrolls here
    }

    if (item_material == 7 || item_material == 12) {
        item_level = 2; // hard, meteoric/dragonleather
    } else if (item_material == 14) {
        item_level = 3; // horror, crystal
    } else {
        item_level = 1; // allowed, easy
    }

    for (POSITION pos = item->_effects.GetHeadPosition(); pos != nullptr;) {
        Effect* parm = item->_effects.GetNext(pos);
        if (parm == nullptr) {
            continue;
        }
        uint8_t prm1 = parm->effect_id;
        uint8_t val1 = parm->damage_min;
        uint8_t val2 = parm->damage_spread;

        if (prm1 == 2) { // body
            if (item_level <= 2) {
                item_level = 2;
            }
            if (val1 >= 3) {
                item_level = 3;
            }
        } else if (prm1 >= 3 && prm1 <= 5) { // reaction, mind, spirit
            if (val1 >= 2 && item_level < 2) {
                item_level = 2;
            }
        } else if (prm1 == 41) { // castspell
            if (item_level < 3) {
                for (uint32_t i = 0; i < sizeof(vd2_sp3) / sizeof(uint32_t); i++) {
                    if (vd2_sp3[i] == val1) {
                        item_level = 3;
                    }
                }
            }

            if (item_level < 2) {
                for (uint32_t i = 0; i < sizeof(vd2_sp2) / sizeof(uint32_t); i++) {
                    if (vd2_sp2[i] == val1) {
                        item_level = 2;
                    }
                }
            }

            if (val2 > 75 && item_level < 2) {
                item_level = 2;
            }
            if (val2 > 100 && item_level < 3) {
                item_level = 3;
            }
        } else if ((prm1 >= 33 && prm1 <= 37) || (prm1 >= 27 && prm1 <= 31)) { // skillX (mage/fighter)
            if (item_level < 3 && val1 > 18) {
                item_level = 3;
            }
            if (item_level < 2 && val1 > 10) {
                item_level = 2;
            }
        }
    }

    if (item_level == 0) {
        return false;
    }
    return item_level == level;
}

// Returns count of items (both on body and in pack) of the specified tier.
uint32_t QueryItems(Unit* character, uint32_t level) {
    if (character == nullptr || level == 0 || level > 3) {
        return 0;
    }
    if (character->inventory == nullptr) {
        return 0;
    }

    uint32_t retval = 0;

    // in bag
    for (POSITION pos = character->inventory->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = character->inventory->items.GetNext(pos);
        if (item != nullptr && CheckItemLevel(item, level)) {
            retval += item->count;
        }
    }

    // on body
    Humanoid* human = static_cast<Humanoid*>(character);
    for (uint32_t i = 1; i <= 12; i++) {
        Item* item = nullptr;
        if (i == 1) {
            item = character->weapon;
        } else if (i == 2) {
            item = character->shield;
        } else {
            item = human->equipment[i];
        }
        if (item != nullptr && CheckItemLevel(item, level)) {
            retval++; // count = always 1
        }
    }

    return retval;
}

} // namespace

bool CheckStrong(Unit* p1) {
    uint32_t p1_statsum = p1->body + p1->reaction + p1->mind + p1->spirit;
    uint32_t max_statsum = 140;
    if (p1_statsum > max_statsum && (p1_statsum - max_statsum) > 10) {
        return true;
    }

    if (QuerySpells(p1, 3) != 0) {
        return true;
    }
    if (QuerySpells(p1, 2) > 2) {
        return true;
    }

    if (QueryScrolls(p1, 1, false) > 100) {
        return true; // >100 scrolls of easy
    }
    if (QueryScrolls(p1, 1, true) > 25) {
        return true; // >25 superscrolls of easy
    }
    if (QueryScrolls(p1, 2, false) > 50) {
        return true; // >50 scrolls of hard
    }
    if (QueryScrolls(p1, 2, true) > 10) {
        return true; // >10 superscrolls of hard
    }
    if (QueryScrolls(p1, 3, false) > 10) {
        return true; // >10 scrolls of horror
    }
    if (QueryScrolls(p1, 3, true) > 5) {
        return true; // >5 superscrolls of horror
    }

    if (QueryItems(p1, 3) != 0) {
        return true;
    }
    if (QueryItems(p1, 2) > 2) {
        return true;
    }

    return false;
}

bool VerifyDamage2(Unit* p1, Unit* p2) {
    if (p1 == nullptr || p2 == nullptr) {
        return false;
    }
    Player* player1 = p1->pOwner;
    Player* player2 = p2->pOwner;
    if (player1 == nullptr || player2 == nullptr) {
        return false;
    }
    if (player1->is_ai != 0 || player2->is_ai != 0) {
        return false;
    }
    if (player1->main_unit != p1 || player2->main_unit != p2) {
        return false;
    }

    bool strong_1 = CheckStrong(p1);
    bool strong_2 = CheckStrong(p2);

    if (strong_1 && !strong_2) {
        return true; // do not allow strong players attack weak players
    }
    if (!strong_1 && strong_2) {
        return true; // do not allow weak players attack strong players
    }
    return false;
}

} // namespace tangar_pvm2
