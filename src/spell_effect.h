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
    virtual ~SpellEffect() override; // 57c080 (client: 54ccd0)
    virtual void Serialize(CArchive& ar) override; // 53e44a (client: 5263a1)
    virtual void ProcessEffects() override; // 53742d (client: 521476)
    virtual void ExpireOnUnit(Unit* unit); // 537438 (client: 521481)

public:
    SpellEffect(); // 537371 (client: 5213ba)
    SpellEffect(const TokenPos* pos); // 5373cc (client: 521415)

public:
    Unit *caster;
    uint8_t expired;
    uint8_t offensive;
    uint16_t seen_by_mask;
    uint16_t seen_by_mask_prev;
    uint16_t notified_mask;
};
ASSERT_SIZE(SpellEffect, 0x48);

class PointEffect : public SpellEffect {
public:
    DECLARE_SERIAL(PointEffect); // Runtime class definition at 6364A0.

public: // VTable at 60f1b0.
    virtual ~PointEffect() override; // 537703 (client: 521741)
    virtual void Serialize(CArchive& ar) override; // 53e6d4 (client: 5264d1)
    virtual void ProcessEffects() override; // 57c0a0 (client: 54ccf0)
    virtual void FixupPointers() override; // 53e7f2 (client: 52654d)

public:
    PointEffect(); // 5374fc (client: 521542)
    PointEffect(Effect* effect, Unit* target); // 53752f (client: 521575)

    void FinishHit(); // 5375a4 (client: 5215ea)

public:
    Unit* target;
    Effect* effect;
};
ASSERT_OFFSET(PointEffect, effect, 0x4c);
ASSERT_SIZE(PointEffect, 0x50);

class AreaEffect : public SpellEffect {
public:
    enum Layer : uint8_t {
        LAYER_FIRE            = 0,
        LAYER_CLEARED_BY_FIRE = 1,
        LAYER_POISON          = 2,
        LAYER_EARTH_WALL      = 3,
        LAYER_LIGHT           = 4,
        LAYER_DARKNESS        = 5,
        LAYER_COUNT           = 6,
    };

    enum ModeFlags : uint8_t {
        MODE_INSTANT     = 0,
        MODE_PERSISTENT  = 1,
        MODE_PHASED_WAVE = 2,
    };

    DECLARE_SERIAL(AreaEffect); // Runtime class definition at 6364B8.

public: // VTable at 60f1f0.
    virtual ~AreaEffect() override; // 5378d8 (client: 521913)
    virtual void Serialize(CArchive& ar) override; // 53e81f (client: 52657a)
    virtual void ProcessEffects() override; // 537964 (client: 52199f)
    virtual void FixupPointers() override; // 53eb1e (client: 526658)
    virtual void ExpireOnUnit(Unit* unit) override; // 5382fb (client: 522300)

public:
    AreaEffect(); // 537846 (client: 521881)
    AreaEffect(Effect* effect, TokenPos* pos, uint8_t range); // 537880 (client: 5218bb)

    void DamageUnit(Unit* unit); // 53831d (client: 522322)
    void ApplyAtCell(uint8_t x, uint8_t y, bool spread_damage); // 538137 (client: 522160)
    void TickAcidBlizzard(); // 537cd6 (client: 521d0d) - Acid stream and blizzard effects.
    void ApplyDistribution(); // 537c8c (client: 521cc5)
    void ApplyWaveDistribution(); // 537f2c (client: 521f57)
    void ApplyAroundPosition(); // 53801a (client: 522043)
    void ApplyRadiusRing(); // 53822c (client: 522235)
    void BroadcastAndApply(); // 5384ff (client: 5224fa)
    void ApplyCellLayer(uint8_t x, uint8_t y); // 53868d (client: 52267c)
    uint32_t GetLayerIndex() const; // 538897 (client: 52286a) - Map area-effect layer index from effect type.

public:
    Effect* effect;
    uint8_t zone_active;
    uint8_t radius; // radius
    uint8_t wave_pattern;
    uint8_t wave_step;
    uint16_t duration;
    uint8_t gap_0x52[2];
};
ASSERT_SIZE(AreaEffect, 0x54);

class SpellTransport : public SpellEffect {
public:
    DECLARE_SERIAL(SpellTransport); // Runtime class definition at 636FC8.

public: // VTable at 0060f230.
    virtual ~SpellTransport() override; // 538b27 (client: 522af7)
    virtual void Serialize(CArchive& ar) override; // 53e588 (client: 526404)
    virtual void ProcessEffects() override; // 538bf4 (client: 522bc4)
    virtual void FixupPointers() override; // 53e68d (client: 52648a)

public:
    SpellTransport(); // 5389e2 (client: 5229b2)
    SpellTransport(SpellEffect* spell_effect, TokenPos* from_position, int16_t speed); // 538a15 (client: 5229e5)
    SpellTransport(AreaEffect* ae, TokenPos* from_position, int16_t speed); // 538a9e (client: 522a6e)

public:
    SpellEffect* spell_effect;
    AreaEffect* area_effect;
    int16_t duration;
    uint8_t gap_0x52;
    uint8_t gap_0x53;
};
ASSERT_SIZE(SpellTransport, 0x54);

// Wave propagation descriptor built from the static tables.
struct WavePattern {
public:
    const int32_t* x_offsets;
    const int32_t* y_offsets;
    int32_t count;
    int32_t x_mult;
    int32_t y_mult;

public:
    void SetWaveTable(const int32_t* table); // 537ae8 (client: 521b21)
    void SetWaveTables(const int32_t* table_a, const int32_t* table_b, uint8_t direction); // 537b2b (client: 521b64)
};
ASSERT_SIZE(WavePattern, 0x14);
