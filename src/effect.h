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

public:
    uint8_t effect_id;
    uint8_t usage_type;
    uint8_t gap_0x3e[2];
    uint16_t spell_or_damage; // Seems like this might store damage in two separate bytes.
    uint16_t spell_value;
    Unit* caster;
};
ASSERT_OFFSET(Effect, spell_or_damage, 0x40);
ASSERT_OFFSET(Effect, caster, 0x44);
ASSERT_SIZE(Effect, 0x48);

class DirectDamage : public Effect {
public: // VTable at 0060f2e0.
    // virtual void VMethod11(Unit* unit);
public:
    UnitToHit unit_to_hit;
};
ASSERT_SIZE(DirectDamage, 0x60);
