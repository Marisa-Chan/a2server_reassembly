#pragma once

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "perf.h"
#include "unit_list.h"


struct Human;
struct MapAlm;
struct World;

// Player presence scan grid, embedded in MapStuff at offset 0x92ecc.
// Tracks which player side bitmasks have units in each 8x8-tile sector.
// On unit add, the unit's owner bitmask (unit->player->field_0x32) is OR'd
// into a 5x5 block of sectors centered on the unit (unit_x>>3, unit_y>>3).
// Used by the server to quickly detect which players can see each unit.
struct ScanPresenceGrid {
    uint8_t  control[0x400];      // header/control, zeroed on construct
    uint32_t sector_grid[34][34]; // [x_sector+1][y_sector], accumulates player side bitmasks
    UnitList* unit_list;
    int32_t  num_detected;        // count of server units flagged as detected this scan
    int32_t  scan_delta;
    uint8_t  gap_0x161c[12];
};
ASSERT_OFFSET(ScanPresenceGrid, unit_list, 0x1610);
ASSERT_SIZE(ScanPresenceGrid, 0x1628);

struct MapStuff { // aka astruct_5
    uint8_t walk_cost_map[65536];
    uint8_t obstacle_map[65536];
    uint8_t obstacle_map2[65536];
    uint16_t field3_0x30000[65536];
    int32_t map_width;
    int32_t map_height;
    uint8_t field_0x50008[16384];
    uint16_t field7_0x54008;
    uint16_t field8_0x5400a;
    UnitList field9_0x5400c;
    uint8_t gap_0x5402c[4];
    Human* field11_0x54030;
    Human* field12_0x54034;
    int32_t field13_0x54038;
    int32_t field14_0x5403c;
    uint8_t field15_0x54040[32];
    uint8_t some_unit_spell_id;
    uint8_t some_unit_spell_damage;
    uint8_t some_unit_x;
    uint8_t some_unit_y;
    uint8_t gap_0x54064[2];
    int16_t field21_0x54066;
    CList<void*> field22_0x54068;
    CMap<uint32_t, uint32_t, void*, void*> field23_0x54084;
    MapAlm* alm;
    uint8_t gap_0x540a4[2];
    uint8_t field26_0x540a6[4][16];
    uint8_t field_0x540e6[64];
    uint16_t field28_0x54126[16];
    uint8_t walk_cost[11];
    uint8_t field_0x54151[31];
    int32_t field31_0x54170[8];
    uint8_t field_0x54190[16384];
    uint8_t field_0x58190[1012];
    int32_t static_scan_ahead;
    int32_t dynamic_scan_ahead;
    int32_t static_refresh_rate;
    int32_t dynamic_refresh_rate;
    int32_t dynamic_by_static_lookup;
    int32_t static_isnt_needed;
    uint8_t field_0x5859c[2020];
    World* field41_0x58d80;
    int32_t speed_multiplier;
    uint8_t field_0x58d88[64];
    Perf field44_0x58dc8;
    Perf field45_0x58df8;
    Perf field46_0x58e28;
    Perf field47_0x58e58;
    uint8_t field48_0x58e88[8];
    uint8_t field49_0x58e90[8];
    int32_t field50_0x58e98[8];
    uint8_t map_min_x;
    uint8_t map_min_y;
    uint8_t map_max_x;
    uint8_t map_max_y;
    int16_t map_min_xy;
    int16_t map_max_xy;
    uint8_t field_0x58ec0[131072];
    uint8_t field_0x78ec0[106504];
    MapStuff* self;
    ScanPresenceGrid scan_presence_grid;
    uint8_t height_map[65536];
    CList<void*> field66_0xa44f4;
    CList<void*> field67_0xa4510;
    uint8_t field_0xa452c[64];
    UnitList* field69_0xa456c;

public:
    int sub_58E3D1(Unit* unit); // add unit to map
    void sub_5948B0(CArray<uint16_t>* encode_buf); // encode map terrain into encode_buf
    void sub_58E525(class Sack* sack); // Remove a sack token from the map
};
ASSERT_OFFSET(MapStuff, map_width, 0x50000);
ASSERT_OFFSET(MapStuff, walk_cost, 0x54146);
ASSERT_OFFSET(MapStuff, scan_presence_grid, 0x92ecc);
ASSERT_OFFSET(MapStuff, height_map, 0x944f4);
ASSERT_SIZE(MapStuff, 0xa4570);
