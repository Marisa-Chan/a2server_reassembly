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
    virtual ~Effect() override; // 57c350
    virtual void Serialize(CArchive& ar) override; // 53e1ce
    virtual void VMethod10(Unit* unit); // 53ee54
    virtual void VMethod11(Unit* unit); // 53efb2
    virtual void VMethod12(Unit* unit); // 53f761
    virtual void VMethod13(Unit* unit); // 53f92c
    virtual void VMethod14(Unit* unit, int32_t param); // 53FA2B
    virtual int32_t VMethod15(); // 541c87
    virtual int32_t VMethod16(double param); // 541d59
    virtual void VMethod17(int32_t param); // 541eb5

public:
    Effect();
    Effect(const CString& name);
    Effect(const Effect* src);

    static Effect* CreateFromString(const CString& effstr);

    int sub_53EDB0(Effect* other); // Returns true if both effects have matching effect_id, usage_type, and spell values
    int32_t sub_541FD7(int32_t budget, int32_t magic_volume); // Compute effect level from budget and magic volume
    int32_t EffectPrice(); // 540941
    static int64_t MagicPriceBonus(int32_t total); // 540A33

public:
    uint8_t effect_id;
    uint8_t usage_type;
    uint8_t gap_0x3e[2];
    union { // TODO: god that spaghetti looks terrible. Maybe use getters/setters?
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
public: // VTable at 0060f2e0.
    virtual void VMethod11(Unit* unit) override;

public:
    DirectDamage();
    DirectDamage(Effect* src); // sub_540BF7

public:
    UnitToHit unit_to_hit;
};
ASSERT_SIZE(DirectDamage, 0x60);
