#pragma once

#include <cstdint>

#include "assert_offset.h"


class Unit;
struct MapStuff;

struct Visibility {
    uint16_t field_0x0[65536];      // 0x0
    uint16_t field_0x20000[64][64]; // 0x20000
    uint16_t field_0x22000[64][64]; // 0x22000
    uint32_t field_0x24000[64][64]; // 0x24000
    uint16_t field_0x28000[64][64]; // 0x28000
    int32_t field_0x2a000;          // 0x2a000
    int32_t scan_shift;             // 0x2a004
    uint8_t field_0x2a008[65536];   // 0x2a008
    MapStuff* map_stuff;            // 0x3a00c

public:
    Visibility(); // 58E1E4
    void sub_58CD84(); // Builds part of the path-cost lookup tables — 58CD84
    void sub_58CE74(); // Builds part of the path-cost lookup tables — 58CE74
    void sub_58E1C1(); // 58E1C1
    void sub_58DE69(Unit* unit, uint16_t yx); // 58DE69
};
ASSERT_SIZE(Visibility, 0x3a00c);
