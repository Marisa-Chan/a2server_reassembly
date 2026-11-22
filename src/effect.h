#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"
#include "unit_to_hit.h"


class Unit;


class Effect : public Token {
public: // Runtime class definition at 637098.
    DECLARE_SERIAL(Effect);

public: // VTable at 0060f288.
    virtual ~Effect() override; // 57c350 (client: 54ce00)
    virtual void Serialize(CArchive& ar) override; // 53e1ce (client: 5262d9)
    virtual void TickOnUnit(Unit* unit); // 53ee54 (client: 52698b)
    virtual void RemoveFromUnit(Unit* unit); // 53efb2 (client: 526afe)
    virtual void ApplyToUnit(Unit* unit); // 53f761 (client: 526e61)
    virtual void ModifyVision(Unit* unit); // 53f92c (client: 527023)
    virtual void ApplyStatDelta(Unit* unit, int32_t stack_delta); // 53fa2b (client: 527122)
    virtual int32_t CalcScrollPrice(); // 541c87 (client: 52915e)
    virtual int32_t CalcChargePercent(double mana); // 541d59 (client: 52921c)
    virtual void RollStrength(int32_t power); // 541eb5 (client: 529380)

public:
    Effect(); // 53ec02 (client: 526739)
    Effect(const CString& name); // 53ec55 (client: 52678c)
    Effect(const Effect* src); // 53ed4d (client: 526884)

    static Effect* CreateFromString(const CString& effstr); // 541046 (client: 5285cb)

    // Helpers for CreateFromString
    static void ParseDmg(CString* str, uint8_t* dmg_min, uint8_t* dmg_spread); // 54180c (client: 528d43)
    static int32_t _StrToInt(CString* str); // 5418f1 (client: 528e22)
    static int32_t GetSpellIDByName(CString* name); // 541963 (client: 528e94)
    static int32_t AtoI(CString* str); // 5419c3 (client: 528edb)
    static int32_t ParseUsage(CString* str, int32_t* out_value); // 541a5d (client: 528f6d)

    int Matches(Effect* other); // 53edb0 (client: 5268e7) - Returns true if both effects have matching effect_id, usage_type, and spell values
    int32_t CalcEffectLevel(int32_t budget, int32_t magic_volume); // 541fd7 (client: 5294a4) - Compute effect level from budget and magic volume
    int32_t EffectPrice(); // 540941 (client: 527fe9)
    static int64_t MagicPriceBonus(int32_t total); // 540a33 (client: 5280eb)

public:
    enum UsageFlags : uint8_t {
        USAGE_PERMANENT    = 0x00,
        USAGE_DURATION     = 0x01,
        USAGE_CONTINUOUS   = 0x02,
        USAGE_TICKING_MASK = USAGE_DURATION | USAGE_CONTINUOUS, // 0x03: ticks down on unit
        USAGE_CHARGES      = 0x04,
        USAGE_TIMED_MASK   = USAGE_DURATION | USAGE_CONTINUOUS | USAGE_CHARGES, // 0x07: uses 16-bit spell_or_damage
        USAGE_SINGLE_USE   = 0x08, // One-shot potion / consumable
        USAGE_EXPIRED      = 0x80, // Flagged for removal after TickOnUnit
    };

    int32_t GetPermanentValue() const { return this->full_magic_value; }
    int16_t GetTimedStatDelta() const { return this->spell_or_damage; }
    uint16_t GetRemainingTicks() const { return this->spell_value; }
    uint8_t GetImbuedSpellId() const { return this->damage_min; }
    uint16_t GetSpellPower() const { return this->spell_value; }

public:
    uint8_t effect_id;
    uint8_t usage_type;
    uint8_t gap_0x3e[2];
    union {
        int32_t full_magic_value;
        struct {
            union {
                int16_t spell_or_damage;
                struct {
                    uint8_t damage_min;
                    uint8_t damage_spread;
                };
            };
            uint16_t spell_value;
        };
    };
    Unit* caster;
};
ASSERT_OFFSET(Effect, full_magic_value, 0x40);
ASSERT_OFFSET(Effect, spell_or_damage, 0x40);
ASSERT_OFFSET(Effect, damage_min, 0x40);
ASSERT_OFFSET(Effect, damage_spread, 0x41);
ASSERT_OFFSET(Effect, spell_value, 0x42);
ASSERT_OFFSET(Effect, caster, 0x44);
ASSERT_SIZE(Effect, 0x48);

class DirectDamage : public Effect {
public:
    DECLARE_SERIAL(DirectDamage); // Runtime class definition at 6370b0.

public: // VTable at 60f2e0.
    virtual ~DirectDamage() override; // 57c120 (client: 54cde0)
    virtual void Serialize(CArchive& ar) override; // 53e422 (client: 526379)
    virtual void RemoveFromUnit(Unit* unit) override; // 540c77 (client: 52832c)

public:
    DirectDamage(); // 540b21 (client: 5281d6)
    DirectDamage(uint8_t hand_damage_min, uint8_t hand_damage_spread); // 540b82 (client: 528237)
    DirectDamage(DirectDamage* src); // 540bf7 (client: 5282ac)

public:
    UnitToHit unit_to_hit;
};
ASSERT_SIZE(DirectDamage, 0x60);
