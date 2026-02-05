#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"


class Effect;
class Unit;


class SpellEffect : public Token {
public: // VTable at 0060f170.
    // virtual CRuntimeClass* GetRuntimeClass() override;
    // virtual ~SpellEffect() override;
    // virtual void Serialize(CArchive& ar) override;
    // virtual uint32_t VMethod10(void*) override; // --- unclear about the args and return type.

public:
    Unit *caster;
    uint8_t field2_0x40;
    uint8_t field3_0x41;
    uint16_t field4_0x42;
    uint16_t field6_0x44;
    uint16_t field6_0x46;
};
ASSERT_SIZE(SpellEffect, 0x48);

class PointEffect : public SpellEffect {
public: // VTable at 0060f1b0.
    // virtual CRuntimeClass* GetRuntimeClass() override;
    // virtual ~PointEffect() override;
    // virtual void Serialize(CArchive& ar) override;
    // virtual void VMethod2() override;
    // virtual void VMethod5() override;

public:
    Unit* target;
    Effect* effect;
};
ASSERT_SIZE(PointEffect, 0x50);

class AreaEffect : public SpellEffect {
public: // VTable at 0060f1f0.
    // virtual CRuntimeClass* GetRuntimeClass() override;
    // virtual ~PointEffect() override;
    // virtual void Serialize(CArchive& ar) override;
    // virtual void VMethod2() override;
    // virtual void VMethod5() override;
    // virtual uint32_t VMethod10(void*) override;

public:
    Effect* effect;
    uint8_t field_0x4c;
    uint8_t field_0x4d;
    uint8_t field_0x4e;
    uint8_t field_0x4f;
    uint16_t field_0x50;
    uint8_t field_0x52[2];
};
ASSERT_SIZE(AreaEffect, 0x54);
