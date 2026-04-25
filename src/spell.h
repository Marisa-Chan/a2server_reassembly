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

extern const std::array<uint32_t, 32> SpellBtB;

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
    int sub_53939E(uint8_t unused1 = 0, uint8_t unused2 = 0); // Checks if it's area spell (spell target == 2).
    void sub_539541(uint32_t param);
    int sub_5393C7(); // Checks if spell_target == 1 (unit-targeted).
    int32_t sub_539958(Unit* caster, Unit* target, int8_t x, int8_t y); // Validate and start cast; returns nonzero if accepted
    void sub_539C49(Unit* caster, Unit* target);
    void sub_539F21(Unit* caster, Unit* target); // Execute targeted spell
    void sub_539F5A(Unit* caster, Unit* target, int8_t x, int8_t y); // Execute area spell

public:
    SpellInfo* spell_info;
    uint8_t spell_id;
    uint8_t max_range;
    uint8_t is_defensive;
    uint8_t gap_0xb;
    int16_t mana_cost;
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
    SpellBook() = default; // 53D28E (inlined in ASM callers)
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
