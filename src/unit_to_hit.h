#ifndef UNIT_TO_HIT_H
#define UNIT_TO_HIT_H

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"

struct UnitToHit {
    uint16_t attack;
    std::array<uint16_t, 6> skill_levels;
    uint8_t hand_damage_min;
    uint8_t hand_damage_spread;
    uint8_t physical_damage_type;
    uint8_t some_damage_min;
    uint8_t some_damage_spread;
    uint8_t some_damage2_min;
    uint8_t some_damage2_spread;
    uint8_t spell_id;
    uint8_t __padding[2];

    UnitToHit();
    UnitToHit(const UnitToHit&);
    UnitToHit(UnitToHit&&);

    UnitToHit& operator=(const UnitToHit&);
    UnitToHit& operator+=(const UnitToHit&);
    UnitToHit& operator-=(const UnitToHit&);

    void AddHits(const UnitToHit&);

    void Clear();

    void Serialize(CArchive& ar);
};
ASSERT_SIZE(UnitToHit, 0x18);

#endif
