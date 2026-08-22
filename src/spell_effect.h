#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"


class Effect;
class Unit;


class SpellEffect : public Token {
public:
    DECLARE_SERIAL(SpellEffect); // Runtime class definition at 636488.

public: // VTable at 0060f170.
    virtual ~SpellEffect() override; // 57c080
    virtual void Serialize(CArchive& ar) override; // 53E44A
    virtual void VMethod2() override; // 53742d
    virtual void VMethod10(Unit* unit); // 537438

public:
    SpellEffect(); // 537371
    SpellEffect(const TokenPos* pos); // 5373cc

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
public:
    DECLARE_SERIAL(PointEffect); // Runtime class definition at 6364A0.

public: // VTable at 60f1b0.
    virtual ~PointEffect() override; // 537703
    virtual void Serialize(CArchive& ar) override; // 53E6D4
    virtual void VMethod2() override; // 57C0A0
    virtual void VMethod5() override; // 53E7F2

public:
    PointEffect(); // 5374FC
    PointEffect(Effect* effect, Unit* target); // 53752F

    void sub_5375A4();

public:
    Unit* target;
    Effect* effect;
};
ASSERT_OFFSET(PointEffect, effect, 0x4c);
ASSERT_SIZE(PointEffect, 0x50);

class AreaEffect : public SpellEffect {
public:
    DECLARE_SERIAL(AreaEffect); // Runtime class definition at 6364B8.

public: // VTable at 60f1f0.
    virtual ~AreaEffect() override; // 5378D8
    virtual void Serialize(CArchive& ar) override; // 53E81F
    virtual void VMethod2() override; // 537964
    virtual void VMethod5() override; // 53EB1E
    virtual void VMethod10(Unit* unit) override; // 5382FB

public:
    AreaEffect(); // 537846
    AreaEffect(Effect* effect, TokenPos* pos, uint8_t range); // 537880

    void sub_53831D(Unit* unit);
    void sub_537CD6();
    void sub_537C8C();
    void sub_537F2C();
    void sub_53801A();
    void sub_53822C();
    void sub_5384FF();
    void sub_53868D(uint8_t x, uint8_t y);
    uint32_t sub_538897() const; // Map area-effect layer index from effect type.

public:
    Effect* effect;
    uint8_t field_0x4c;
    uint8_t field_0x4d; // radius
    uint8_t field_0x4e;
    uint8_t field_0x4f;
    uint16_t duration;
    uint8_t field_0x52[2];
};
ASSERT_SIZE(AreaEffect, 0x54);

class SpellTransport : public SpellEffect {
public:
    DECLARE_SERIAL(SpellTransport); // Runtime class definition at 636FC8.

public: // VTable at 0060f230.
    virtual ~SpellTransport() override; // 538B27
    virtual void Serialize(CArchive& ar) override; // 53E588
    virtual void VMethod2() override; // 538BF4
    virtual void VMethod5() override; // 53E68D

public:
    SpellTransport(); // 5389E2
    SpellTransport(SpellEffect* spell_effect, TokenPos* from_position, int16_t speed); // 538A15
    SpellTransport(AreaEffect* ae, TokenPos* from_position, int16_t speed); // 538A9E

public:
    SpellEffect* spell_effect;
    AreaEffect* area_effect;
    int16_t duration;
    uint8_t gap_0x52;
    uint8_t gap_0x53;
};
ASSERT_SIZE(SpellTransport, 0x54);
