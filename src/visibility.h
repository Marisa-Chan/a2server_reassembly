#pragma once

#include <cstdint>

#include "assert_offset.h"


class Unit;
struct MapStuff;

struct Visibility {
public:
    static const int32_t CENTER = 20;

public:
    uint16_t gap_0x0[65536];      // 0x0
    uint16_t dist_sq_table[64][64]; // 0x20000
    uint16_t ray_dir_table[64][64]; // 0x22000
    int32_t vis_budget_scratch[64][64];  // 0x24000
    uint16_t dist_scale_table[64][64]; // 0x28000
    int32_t scan_radius;          // 0x2a000
    int32_t scan_shift;             // 0x2a004
    uint8_t vis_scratch[65536];   // 0x2a008
    MapStuff* map_stuff;            // 0x3a00c

public:
    Visibility(); // 58e1e4 (client: 55d694)
    void BuildDistSqTable(); // 58cd84 (client: 55c234) - Builds part of the path-cost lookup tables
    void BuildRayTables(); // 58ce74 (client: 55c324) - Builds part of the path-cost lookup tables
    void ClearVisScratch(); // 58e1c1 (client: 514f28)
    void RefreshUnitVisibility(Unit* unit, uint16_t yx); // 58de69 (client: 55d319)
    void ScanUnitVisibility(Unit* unit, uint16_t yx); // 58d768 (client: 55cc18)
    int32_t StepVisRay(int32_t x, int32_t y, int32_t base_yx, int32_t base_height); // 58d638 (client: 55cae8)
};
ASSERT_SIZE(Visibility, 0x3a00c);
