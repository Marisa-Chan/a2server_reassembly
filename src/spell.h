#ifndef SPELL_H
#define SPELL_H

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "table.h"


class Humanoid;
class Unit;


class Spell : public CObject {
public:
    DECLARE_SERIAL(Spell);

    virtual ~Spell();
    virtual void Serialize(CArchive& ar) override;

public:
    Spell();
    Spell(uint8_t spell_id);
    Spell(const CString& name);

    void sub_53940D(Unit* unit);
    bool sub_53939E(); // Checks if it's area spell (spell target == 2).
    void sub_539541(uint32_t param);

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
    DECLARE_SERIAL(SpellBook);

    virtual ~SpellBook();
    virtual void Serialize(CArchive& ar) override;
public:
    SpellBook();
    int32_t sub_53DD3D(); // Calculate spellbook bitmask for this unit's spells.

    void RefreshForHumanoid(Humanoid* humanoid);

public:
    CArray<Spell*> spells;
    uint32_t current_spell_index;

    void sub_53D7F0(int32_t spell_id, Spell* spell); // Add/replace a spell in the book
    Spell* sub_53DB79(int spell_id); // Look up spell in book by id
};
ASSERT_SIZE(SpellBook, 0x1c);

constexpr std::array<uint32_t, 25> BOOK_POS_TO_SPELL_ID = {
    // 636248, 0-based.
    0,
    // 63624C, 1-based.
    1, 2, 3, 4, 24, 20, 21, 26, 13, 12, 11, 10,
    5, 6, 7, 8, 25, 22, 23, 27, 19, 18, 17, 16,
};


#endif
