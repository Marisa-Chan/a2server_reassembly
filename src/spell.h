#ifndef SPELL_H
#define SPELL_H

#include <array>
#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include "constants.h"
#include <afxtempl.h>
#include "table.h"


class Humanoid;
class Unit;

namespace magic_sphere {
    enum : uint8_t {
        none   = 0,
        fire   = 1,
        water  = 2,
        air    = 3,
        earth  = 4,
        astral = 5,
        count  = 6,
    };
}

struct PatchedSpellRange {
    union {
        int32_t raw;
        struct {
            uint8_t  base_range;
            uint8_t  power_divider;
            uint16_t unused;
        };
    };

    constexpr PatchedSpellRange(int32_t v = 0) : raw(v) {}
    constexpr operator int32_t() const { return this->raw; }
};
ASSERT_SIZE(PatchedSpellRange, 4);

class Spell : public CObject {
public:
    enum TargetType : uint8_t {
        TARGET_UNIT = 1,
        TARGET_AREA = 2,
    };

    enum DeliveryType : uint8_t {
        DELIVERY_INSTANT    = 1,
        DELIVERY_PROJECTILE = 2,
    };

    enum Distribution : uint8_t {
        DIST_POINT        = 1,
        DIST_DEFAULT_AREA = 2,
        DIST_ROUND        = 3,
        DIST_LONG_WALL    = 4,
        DIST_PHASED_WAVE  = 5,
    };

    enum ApplyMethod : uint8_t {
        APPLY_INSTANT      = 1,
        APPLY_HANG_ON_UNIT = 2,
    };

    DECLARE_SERIAL(Spell);

    virtual ~Spell(); // 5392fe (client: 523042)
    virtual void Serialize(CArchive& ar) override; // 53cf8d (client: 525c30)

public:
    Spell(); // 538fa5 (client: 522cfd)
    Spell(uint8_t spell_id); // 538fdd (client: 522d34)
    Spell(const CString& name); // 539055 (client: 522dab)

    void Init(); // 5391ab (client: 522ef5)

    void SetPowerFromCaster(Unit* unit); // 53940d (client: 52314a)
    int IsAreaSpell(uint8_t unused1 = 0, uint8_t unused2 = 0); // 53939e (client: 5230e1) - Checks if it's area spell (spell target == 2).
    void SetPower(uint32_t power); // 539541 (client: 523278)
    int IsUnitSpell(); // 5393c7 (client: 523108) - Checks if spell_target == 1 (unit-targeted).
    int32_t StartCast(Unit* caster, Unit* target, int8_t x, int8_t y); // 539958 (client: 52365f) - Validate and start cast; returns nonzero if accepted
    void ExecuteRaySpell(Unit* caster, Unit* target); // 539c49 (client: 5238cb)
    void ExecuteOnTarget(Unit* caster, Unit* target); // 539f21 (client: 523a46) - Execute targeted spell
    void ExecuteAtArea(Unit* caster, Unit* target, int8_t x, int8_t y); // 539f5a (client: 523a7f) - Execute area spell


    uint8_t GetSpeed(uint8_t v) const { // 539926 (client: 52362d)
        if (this->spell_id != spell::haste) {
            return 0;
        }
        return (v / 15) + 1;
    }

    uint8_t GetResistance(uint8_t v) const { // 53985f (client: 523566)
        if (this->spell_id == spell::protection_from_fire || this->spell_id == spell::protection_from_water ||
            this->spell_id == spell::protection_from_air || this->spell_id == spell::protection_from_earth) {
            return v / 2;
        }
        return 0;
    }

    int16_t GetSight(uint8_t v) const { // 5398e2 (client: 5235e9)
        if (this->spell_id == spell::light || this->spell_id == spell::darkness) {
            return -(v / 30) - 1;
        }
        return 0;
    }

    uint8_t GetMaximumDamageProbability(uint8_t v) const { // 5397f7 (client: 5234fe)
        if (this->spell_id == spell::bless) {
            return (v * 4 / 5) + 20;
        }
        return 0;
    }

    uint8_t GetMinimumDamageProbability(uint8_t v) const { // 53982b (client: 523532)
        if (this->spell_id == spell::curse) {
            return (v * 4 / 5) + 20;
        }
        return 0;
    }

    uint8_t GetRays(uint8_t v) const { // 5397a3 (client: 5234aa)
        if (this->spell_id == spell::prismatic_spray) {
            uint8_t r = v / 20 + 2;
            if (r > 7) {
                r = 7;
            }
            return r;
        }
        return 0;
    }

    uint8_t GetBonusType5(uint8_t v) const { // 5398b1 (client: 5235b8)
        if (this->spell_id == spell::shield) {
            return v / 10 + 3;
        }
        return 0;
    }

    int32_t GetSphere() { return this->spell_info->Values()[0].sphere; } // 5393ee (client: 52312d)

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
    DECLARE_SERIAL(SpellBook); // Runtime class definition at 636ff8.

public: // VTable at 60ec30.
    virtual ~SpellBook(); // 53d368 (client: 525dc4)
    virtual void Serialize(CArchive& ar) override; // 53dd9a (client: 5261f1)

public:
    SpellBook() {}; // server: none (client: 544530) - inlined in 53d28e on server
    int32_t CalcSpellMask(); // 53dd3d (client: 526190) - Calculate spellbook bitmask for this unit's spells.

    void RefreshForHumanoid(Humanoid* humanoid); // 53dbc5 (client: 5260d0)

public:
    CArray<Spell*> spells;
    uint32_t current_spell_index = 0;

    void SetSpell(int32_t spell_id, Spell* spell); // 53d7f0 (client: 525f8a) - Add/replace a spell in the book
    Spell* GetSpell(int spell_id); // 53db79 (client: 52607d) - Look up spell in book by id
};
ASSERT_SIZE(SpellBook, 0x1c);

// 636248 (0-based; 1-based cells start at 63624C).
// Maps 1-based VisSpellBook grid slot (2 rows x 12 cols) to spell::* ID.
constexpr std::array<uint32_t, 25> BOOK_POS_TO_SPELL_ID = {
    0,
    // First row: Fire (0..3), Astral top (4..7), Air reversed (8..11)
    spell::fire_arrow, spell::fire_ball, spell::wall_of_fire, spell::protection_from_fire,
    spell::heal, spell::bless, spell::haste, spell::drain_life,
    spell::protection_from_air, spell::invisibility, spell::prismatic_spray, spell::lightning,
    // Second row: Water (12..15), Astral bottom (16..19), Earth reversed (20..23)
    spell::ice_missile, spell::poison_cloud, spell::blizzard, spell::protection_from_water,
    spell::summon, spell::animate_dead, spell::teleport, spell::shield,
    spell::protection_from_earth, spell::stone_curse, spell::wall_of_earth, spell::stone_missile,
};

// 637010: inverse of BOOK_POS_TO_SPELL_ID; maps spell::* ID -> (1u << ui_cell).
constexpr std::array<uint32_t, 32> SPELL_ID_TO_BOOK_MASK = []() constexpr {
    std::array<uint32_t, 32> masks = {};
    for (size_t pos = 1; pos < BOOK_POS_TO_SPELL_ID.size(); ++pos) {
        masks[BOOK_POS_TO_SPELL_ID[pos]] = 1u << (pos - 1);
    }
    return masks;
}();


#endif
