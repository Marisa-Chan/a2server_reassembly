#ifndef SPELL_H
#define SPELL_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "table.h"


class Humanoid;
class Unit;


class Spell : public CObject {
public:
    void sub_53940D(Unit* unit);

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
ASSERT_OFFSET(Spell, mana_cost, 0xc);
ASSERT_SIZE(Spell, 0x14);

class SpellBook : public CObject {
public:
    void RefreshForHumanoid(Humanoid* humanoid);

public:
    CArray<Spell*> spells;
    uint32_t current_spell_index;
};
ASSERT_SIZE(SpellBook, 0x1c);


#endif
