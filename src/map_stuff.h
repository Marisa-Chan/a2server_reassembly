#pragma once

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "perf.h"
#include "unit_list.h"
#include "token.h"


class AreaEffect;
class Building;
class Human;
struct MapAlm;
class Sack;
class TokenPos;
class Unit;
struct World;

extern CString g_MissionText; //660f2c
extern CString g_MissionBriefing; //660de8
extern CStringArray g_MissionFailures; //660f18
extern CStringArray g_MissionSubjs; //660ea8


struct Obstacle
{
    uint8_t bits;

    inline void SetBits(uint8_t _bits) { bits |= _bits; } //5970a0
    inline void UnsetBits(uint8_t _bits) { bits &= ~_bits; } //

    inline void AndBits(uint8_t _bits) { bits &= _bits; } //

    inline uint8_t TestBits(uint8_t _bits) { return bits & _bits; } //5970f0

    inline operator uint8_t() const { return bits; }
    inline void operator=(uint8_t val) { bits = val; }
};

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

    void sub_596047(AreaEffect* ae); // Register area effect into the scan grid (sub_596047)
    void sub_596131();                 // Rebuild presence grid from all active units/players
};
ASSERT_OFFSET(ScanPresenceGrid, unit_list, 0x1610);
ASSERT_SIZE(ScanPresenceGrid, 0x1628);

struct CellState {
    uint8_t  walk_cost;          // +0x00: saved original walk_cost_map byte
    uint8_t  obstacle;           // +0x01: saved original obstacle_map byte
    uint8_t  effect_count;       // +0x02: number of non-null area_effects[] slots
    uint8_t  gap_0x03;           // +0x03
    Unit*    small_unit;         // +0x04: small unit on this cell (type < 3)
    Unit*    large_unit;         // +0x08: large unit on this cell (type == 3)
    Building* building;          // +0x0C: building on this cell, if any
    Sack*    sack;               // +0x10: sack placed on this cell
    AreaEffect* area_effects[6]; // +0x14: area effect layer slots, indexed by type via MapLayer()
    Unit*    static_blocker;     // +0x2C: unit causing static obstacle flag (0x40) when non-null
    Unit*    dynamic_blocker;    // +0x30: unit causing dynamic obstacle flag (0x80) when non-null
    uint8_t  spell_id;           // +0x34: spell placed on tile (triggers on entry)
    uint8_t  spell_damage;       // +0x35
    uint8_t  spell_x;            // +0x36
    uint8_t  spell_y;            // +0x37
    uint8_t  gap_0x38[2];
    PosYX  cell_yx;            // +0x3A
};
ASSERT_OFFSET(CellState, area_effects, 0x14);
ASSERT_SIZE(CellState, 0x3c);

struct MapStuff { // aka astruct_5
    uint8_t walk_cost_map[65536];
    Obstacle obstacle_map[65536];
    Obstacle obstacle_map2[65536];
    uint16_t field3_0x30000[65536];
    int32_t map_width;
    int32_t map_height;
    uint8_t field_0x50008[16384];
    uint16_t field7_0x54008;
    uint16_t field8_0x5400a;
    UnitList field9_0x5400c;
    CellState scratch_cell_state; // Scratch `CellState` buffer.
    CList<void*> field22_0x54068;
    CMap<uint16_t, uint16_t, CellState, CellState> cell_states; // Tile yx -> cell state.
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
    MapStuff(MapAlm* alm, UnitList* unit_list); // 587E19
    ~MapStuff(); // 587f70

    int sub_58E3D1(Unit* unit); // add unit to map
    void sub_5948B0(CWordArray* encode_buf); // encode map terrain into encode_buf
    void sub_58E525(class Sack* sack); // Remove a sack token from the map
    Sack* sub_58E5C7(uint16_t param_2, uint16_t param_3); // Look up sack at map position
    Sack* sub_58E5F3(TokenPos* pos);
    uint8_t sub_59166C(Unit* unit, PosYX yx); // Pick rotation angle for the unit to look at `yx`.
    void sub_5954AC(Unit* unit, uint8_t x, uint8_t y); // Teleport unit to (x, y)
    int sub_596576(uint16_t yx, void* src); // Apply terrain modification from 6-byte src buffer at yx
    AreaEffect** sub_59536C(uint32_t yx); // Get pointer to area_effects[6] array at cell yx.
    int16_t sub_5913BD(Unit* unit, uint8_t x, uint8_t y); // Movement time for unit entering tile (x,y).
    int sub_59190D(Unit* target, Unit* observer); // Visibility/range check between two units.
    uint8_t sub_591424(Unit* unit, Unit* target); // Compute facing direction from unit to target.
    int sub_58FE6D(Unit* unit, Unit* target, uint8_t max_range); // Check if unit faces target and is in range.
    int sub_58FEDA(Unit* unit, PosYX yx, uint8_t max_range); // Check if unit faces yx and is in range.
    void sub_58FF51(Unit* unit, PosYX yx, uint8_t max_range); // AI movement tick for unit toward yx.
    void sub_59028D(Unit* unit, Unit* target, uint8_t max_range); // AI movement tick for unit pursuing target.
    void sub_58826D(Unit* unit, uint8_t x, uint8_t y, int32_t flag, Unit* target);
    void sub_590678(Unit* unit);
    void sub_5907BE(Unit* unit);
    void sub_590902(Unit* unit, Unit* target);
    void sub_5918B8(Unit* unit, Unit* target);
    void sub_5918E2(Unit* unit, PosYX yx);
    uint16_t sub_593AA4(Unit* unit);
    uint8_t sub_593B29(PosYX yx1, PosYX yx2); // Chebyshev distance between two YX positions.
    int sub_597140(Unit* unit, PosYX yx, int32_t flag);
    int32_t sub_5945EF(class Building* building); // Remove building from map
    int sub_595438(AreaEffect* ae, uint8_t x, uint8_t y); // Check if area effect can be applied at (x, y) — 595438
    Unit* sub_58CA1B(PosYX yx); // Get unit at map coordinate yx — 58CA1B

    inline Obstacle& ObstacleAt(PosYX yx) { return obstacle_map[yx.val]; } //59a7a0
    inline Obstacle& Obstacle2At(PosYX yx) { return obstacle_map2[yx.val]; } //59a7c0
    inline uint8_t& WalkCostAt(PosYX yx) { return walk_cost_map[yx.val]; } //59a780

    int IsObstacle2_40(PosYX yx) { return Obstacle2At(yx).TestBits(0x40); } //58b9ee
    void SetObstacle2_40(PosYX yx) { Obstacle2At(yx).SetBits(0x40); } //58b920
    int IsObstacle2_80(PosYX yx) { return Obstacle2At(yx).TestBits(0x80); } //58ba15
    void SetObstacle2_80(PosYX yx) { Obstacle2At(yx).SetBits(0x80); } //58b952

    void FUN_0058b5d6(CellState& cell); //58b5d6

    int32_t GetWidth() const { return map_width; } //58b8df
    int32_t GetHeight() const { return map_height; } //58b8f3
};
ASSERT_OFFSET(MapStuff, map_width, 0x50000);
ASSERT_OFFSET(MapStuff, scratch_cell_state, 0x5402c);
ASSERT_OFFSET(MapStuff, walk_cost, 0x54146);
ASSERT_OFFSET(MapStuff, scan_presence_grid, 0x92ecc);
ASSERT_OFFSET(MapStuff, height_map, 0x944f4);
ASSERT_SIZE(MapStuff, 0xa4570);



class Scenario : public CObject
{
public:
    ~Scenario();

    Scenario(const char* fname);

    uint16_t* GetLandscape() const { return landscape; }; //41ee80
    int32_t GetWidth() const { return width; }; //41ef20
    int32_t GetHeight() const { return height; }; //41ef40
public:
    int32_t width;
    int32_t height;
    uint16_t* landscape;
    uint8_t* map_heights;
    uint8_t* field_0x14;
    uint8_t* field_0x18;
    uint8_t abrightness;
    uint8_t contrast;
    uint8_t field_0x1e;
    uint8_t field_0x1f;
    double inv_sun_angle;
    int32_t field_0x28;
    int32_t daytime_minutes;
};
ASSERT_SIZE(Scenario, 0x30);

struct Location
{
    int32_t field_0x0;
    int32_t loctype;
    CRect rect;

    int GetType() { return loctype; } //475150
};


void __cdecl MissionGetBriefing(CString* out); //4e13db
void __cdecl MissionGetFailure(int32_t idx, CString* out); //4e14a4
void __cdecl MissionGetTips(int32_t idx, CString* out); //4e159b
void __cdecl MissionGetSubj(int32_t idx, CString* out); //4e16ce
void __cdecl MissionGetLocName(int32_t t, int32_t idx, CString* out); //4e17e3
void __cdecl MissionGetDescription(int32_t t, int32_t idx, CString* out); //4e1915
