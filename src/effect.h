#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"
#include "unit_to_hit.h"


class Unit;


class Effect : public Token {
    DECLARE_SERIAL(Effect);
public: // VTable at 0060f288.
    virtual ~Effect() override;
    virtual void Serialize(CArchive& ar) override;

    virtual void VMethod10(Unit* unit); // TODO: migrate.
    virtual void VMethod11(Unit* unit); // TODO: migrate. VTable slot 15 (0x3C).
    virtual void VMethod12(Unit* unit);
    virtual void VMethod13(Unit* unit);
    virtual void VMethod14(Unit* unit, int32_t param_3);
    virtual int32_t VMethod15();
    virtual int32_t VMethod16(double param_2);
    virtual void VMethod17(int32_t param_2);

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
                uint16_t spell_or_damage;
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
    // virtual void VMethod11(Unit* unit);

public:
    DirectDamage();
    DirectDamage(Effect* src); // sub_540BF7

public:
    UnitToHit unit_to_hit;
};
ASSERT_SIZE(DirectDamage, 0x60);
