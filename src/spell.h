#ifndef SPELL_H
#define SPELL_H

#include <cstdint>

#include "asm_mfc.h"
#include "mfc_templ.h"
#include "table.h"


class Spell : public CObject {
public:
    SpellInfo* spell_info;
    uint8_t spell_id;
    uint8_t max_range;
    uint8_t is_defensive;
    uint8_t gap_0xb;
    uint16_t mana_cost;
    uint8_t damage_min;
    uint8_t damage_spread;
    uint16_t spell_power;
    uint8_t gap_0x12[2];
};
static_assert(offsetof(Spell, mana_cost) == 0xc, "Spell::mana_cost offset mismatch");
static_assert(sizeof(Spell) == 0x14, "Spell size mismatch");

class SpellBook : public CObject {
public:
    CArray<Spell*> spells;
    uint32_t current_spell_index;
};
static_assert(sizeof(SpellBook) == 0x1c, "SpellBook size mismatch");


#endif
