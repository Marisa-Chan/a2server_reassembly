#pragma once

#include <cstdint>

#include "assert_offset.h"


class Unit;
struct MapStuff;

struct Visibility {
public:
    static const int32_t CENTER = 20;

public:
    uint16_t field_0x0[65536];      // 0x0
    uint16_t dist_sq_table[64][64]; // 0x20000
    uint16_t ray_dir_table[64][64]; // 0x22000
    int32_t vis_budget_scratch[64][64];  // 0x24000
    uint16_t dist_scale_table[64][64]; // 0x28000
    int32_t scan_radius;          // 0x2a000
    int32_t scan_shift;             // 0x2a004
    uint8_t vis_scratch[65536];   // 0x2a008
    MapStuff* map_stuff;            // 0x3a00c

public:
    Visibility(); // 58E1E4 (client: 55d694)
    void sub_58CD84(); // Builds part of the path-cost lookup tables — 58CD84 (client: 55c234)
    void sub_58CE74(); // Builds part of the path-cost lookup tables — 58CE74
    void sub_58E1C1(); // 58E1C1 (client: 514f28)
    void sub_58DE69(Unit* unit, uint16_t yx); // 58DE69 (client: 55d319)
    void sub_58D768(Unit* unit, uint16_t yx); // 58D768 (client: 55cc18)
    int32_t sub_58D638(int32_t x, int32_t y, int32_t base_yx, int32_t base_height); // 58D638 (client: 55cae8)
};
ASSERT_SIZE(Visibility, 0x3a00c);
