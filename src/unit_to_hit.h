#ifndef UNIT_TO_HIT_H
#define UNIT_TO_HIT_H

#include <array>
#include <cstdint>

#include <afx.h>
#include "assert_offset.h"

struct UnitToHit {
    int16_t attack;
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

    UnitToHit(); // 533c5c (client: 51f461)
    UnitToHit(const UnitToHit&); // 533d0d (client: 51f512)
    UnitToHit(UnitToHit&&);

    UnitToHit& operator=(const UnitToHit&); // 533e02 (client: 51f607)
    UnitToHit& operator+=(const UnitToHit&); // 533d2c (client: 51f531)
    UnitToHit& operator-=(const UnitToHit&); // 533f3a (client: 51f73f)

    void AddHits(const UnitToHit&); // 533ea3 (client: 51f6a8)

    void Clear(); // 533c72 (client: 51f477)

    void Serialize(CArchive& ar); // 534021 (client: 51f826)
};
ASSERT_SIZE(UnitToHit, 0x18);

#endif
