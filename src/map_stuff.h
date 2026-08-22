#pragma once

#include <array>
#include <cstdint>

#include "2darray.h"
#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "perf.h"
#include "token.h"
#include "unit_list.h"
#include "visibility.h"


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

public:
    ScanPresenceGrid(); // 5955B5
    void sub_596047(AreaEffect* ae); // Register area effect into the scan grid (sub_596047)
    void sub_596131();                 // Rebuild presence grid from all active units/players
};
ASSERT_OFFSET(ScanPresenceGrid, unit_list, 0x1610);
ASSERT_SIZE(ScanPresenceGrid, 0x1628);

// Sub-field of MapStuff. Purpose not fully reversed; kept opaque.
struct FieldBlock54190 {
    uint32_t field_0x0;
    uint32_t field_0x4;
    uint8_t data[0x3ec];

public:
    FieldBlock54190(); // 599FF0
};
ASSERT_SIZE(FieldBlock54190, 0x3f4);

// Sub-field of MapStuff. Purpose not fully reversed; kept opaque.
struct FieldBlock5859c {
    uint8_t* field_0x0;
    uint8_t* field_0x4;
    uint8_t* field_0x8;
    int32_t field_0xc;
    uint8_t data[2004];

public:
    FieldBlock5859c(); // 5A3180
};
ASSERT_SIZE(FieldBlock5859c, 2020);

struct CellState {
    uint8_t  walk_cost;          // +0x00: saved original walk_cost_map byte
    uint8_t  obstacle;           // +0x01: saved original obstacle_map byte
    uint8_t  effect_count;       // +0x02: number of non-null area_effects[] slots
    uint8_t  gap_0x03;           // +0x03
    Unit*    small_unit;         // +0x04: small unit on this cell (type < 3)
    Unit*    large_unit;         // +0x08: large unit on this cell (type == 3)
    Building* building;          // +0x0C: building on this cell, if any
    Sack*    sack;               // +0x10: sack placed on this cell
    std::array<AreaEffect*, 6> area_effects; // +0x14: area effect layer slots, indexed by type via MapLayer()
    Unit*    static_blocker;     // +0x2C: unit causing static obstacle flag (0x40) when non-null
    Unit*    dynamic_blocker;    // +0x30: unit causing dynamic obstacle flag (0x80) when non-null
    uint8_t  spell_id;           // +0x34: spell placed on tile (triggers on entry)
    uint8_t  spell_damage;       // +0x35
    uint8_t  spell_x;            // +0x36
    uint8_t  spell_y;            // +0x37
    uint8_t  teleport_x;         // +0x38: teleport destination x
    uint8_t  teleport_y;         // +0x39: teleport destination y
    PosYX  cell_yx;              // +0x3A

    int IsEmpty() const; //58bef6
    void Null(); //58a3da
    void sub_59423F(); // Remaps stale on-disk pointer fields through g_Server's pointer remap table.
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
    uint8_t field_0x54151[5];
    int8_t line_walk_delta[12][2]; // (dx,dy) steps cycling right,down,left,up x3; used by sub_592A48's box perimeter walk.
    uint8_t field_0x5416E[2];
    int32_t field31_0x54170[8];
    FieldBlock54190 field_0x54190;
    uint16_t field_0x54584[0x1000]; // Path lookup table for static obstacles, used by sub_5882AE.
    uint16_t field_0x56584[0x1000]; // Path lookup table for dynamic obstacles, used by sub_5882AE.
    int32_t static_scan_ahead;
    int32_t dynamic_scan_ahead;
    int32_t static_refresh_rate;
    int32_t dynamic_refresh_rate;
    int32_t dynamic_by_static_lookup;
    int32_t static_isnt_needed;
    FieldBlock5859c field_0x5859c;
    World* field41_0x58d80;
    int32_t speed_multiplier;
    CellState field_0x58d88; // Seems unused?
    uint32_t field_0x58dc4; // Seems unused?
    Perf field44_0x58dc8;
    Perf field45_0x58df8;
    Perf field46_0x58e28;
    Perf field47_0x58e58;
    int8_t field48_0x58e88[8]; // Neighbor cell dx offsets.
    int8_t field49_0x58e90[8]; // Neighbor cell dy offsets.
    int32_t field50_0x58e98[8]; // Neighbor cell (dy,xy) offsets.
    uint8_t map_min_x;
    uint8_t map_min_y;
    uint8_t map_max_x;
    uint8_t map_max_y;
    int16_t map_min_xy;
    int16_t map_max_xy;
    Visibility visibility;
    ScanPresenceGrid scan_presence_grid;
    uint8_t height_map[65536];
    CList<void*> field66_0xa44f4;
    CList<uint16_t> field67_0xa4510;
    Array2D<uint8_t, 8, 8> field_0xa452c;
    UnitList* field69_0xa456c;

public:
    MapStuff(MapAlm* alm, UnitList* unit_list); // 587E19
    ~MapStuff(); // 587f70

    void sub_58E891(UnitList* unit_list); // 58E891
    void sub_58F166(MapAlm* alm); // 58F166
    uint8_t sub_58F48D(uint16_t terrain_code, uint8_t* out_walk_cost); // Computes edge-blended walk cost + classification code for a terrain cell — 58F48D
    void sub_58E670(); // Marks an 8-tile-thick obstacle border around the map edges — 58E670

    int sub_58E3D1(Unit* unit); // add unit to map
    void sub_58AB48(CArchive& ar); // Serialize
    void sub_5948B0(CWordArray* encode_buf); // encode map terrain into encode_buf
    void sub_594125(); // Post-load map relink helper
    int sub_58E407(class Sack* sack); // Check if sack can be placed on map
    void sub_58E525(class Sack* sack); // Remove a sack token from the map
    Sack* sub_58E5C7(uint16_t param_2, uint16_t param_3); // Look up sack at map position
    Sack* sub_58E5F3(TokenPos* pos);
    int sub_590F0A(Unit* unit); // Check if unit can be placed at current position
    int sub_58AD4A(Unit* unit); // Place unit on map at current TokenPos position
    int32_t sub_58AEEF(Unit* unit, uint8_t x, uint8_t y); // Checks whether unit can occupy cell (x,y), triggering any spell tile in the process.
    uint8_t sub_59166C(Unit* unit, PosYX yx); // Pick rotation angle for the unit to look at `yx`.
    void sub_5954AC(Unit* unit, uint8_t x, uint8_t y); // Teleport unit to (x, y)
    void sub_59501E(AreaEffect* ae, uint8_t x, uint8_t y); // 59501E
    int sub_595073(AreaEffect* ae, uint16_t yx); // 595073 — remove area effect from cell state at yx
    Unit* sub_5946BF(uint8_t x, uint8_t y); // 5946BF
    void sub_594768(uint8_t x, uint8_t y); // 594768
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
    void sub_58FD2F(Unit* unit); // Rotates eye->field0_0x0 toward eye->field1_0x1 by up to rotation_speed.
    void sub_590F94(Unit* unit, uint8_t facing);
    void sub_590902(Unit* unit, Unit* target);
    void sub_5918B8(Unit* unit, Unit* target);
    void sub_5918E2(Unit* unit, PosYX yx);
    uint16_t sub_593AA4(Unit* unit);
    uint8_t sub_593B29(PosYX yx1, PosYX yx2); // Chebyshev distance between two YX positions.
    void sub_5964D7(uint16_t yx, uint8_t range, Player* owner, UnitList* out_list); // Gather units near yx within range into out_list.
    int32_t sub_593CD5(Unit* unit, uint16_t yx, uint8_t val); // Computes movement step magnitude/heading toward yx.
    int sub_597140(Unit* unit, PosYX yx, int32_t flag);
    int32_t sub_5945EF(class Building* building); // Remove building from map
    int32_t sub_59451F(class Building* building); // Place building on the map
    int sub_5943D0(Building* building, uint16_t yx); // 5943D0 — set building pointer in cell state at yx
    int sub_595438(AreaEffect* ae, uint8_t x, uint8_t y); // Check if area effect can be applied at (x, y) — 595438
    uint8_t FacingDiff(uint8_t a, uint8_t b); // 595561 — smallest angular difference between two facing bytes.
    AreaEffect* sub_5953CB(AreaEffect* ae, uint16_t coord);  // Check if area effect covers cell coord — 5953CB
    Unit* sub_58CA1B(PosYX yx); // Get unit at map coordinate yx — 58CA1B
    void sub_594E65(AreaEffect* ae, uint8_t x, uint8_t y); // 594E65
    int sub_594EBA(AreaEffect* ae, uint16_t yx); // 594EBA — register area effect into cell state at yx
    UnitList* sub_5897AA(PosYX yx, uint8_t range); // Build a temporary UnitList of units around yx — 5897AA
    int16_t sub_5936D2(Unit* unit, uint32_t packed_yyxx, uint8_t param_3); // 5936D2

    Unit* sub_58CB5A(uint16_t yx); // Get small unit occupying cell yx — 58CB5A
    Unit* sub_58CBB9(uint16_t yx); // Get large unit occupying cell yx — 58CBB9
    void sub_594A2D(Unit* unit, int32_t flag); // 594A2D — collect passable sack cells in expanding rings around the unit.
    int32_t sub_594C4B(Unit* unit, uint16_t yx); // 594C4B — check whether cell yx holds a sack and is passable for unit.
    int sub_58B1D7(Unit* unit); // Check if unit fits/can be placed over all cells at its current position — 58B1D7
    void sub_58B593(PosYX yx); // Refresh derived obstacle bits for cell yx from its stored cell state — 58B593
    Sack* sub_58E611(uint16_t yx); // Get sack at cell yx if the cell has a stored state — 58E611
    int sub_59449A(class Building* building, PosYX yx); // Clear building from the cell state at yx — 59449A
    int sub_595468(Unit* unit, PosYX yx); // Check if unit can move onto and off of yx (both directions clear) — 595468
    void sub_597BB0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map, queue into dynamic ring buffer — 597BB0
    void sub_5983D0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map, queue into static ring buffer — 5983D0
    void sub_598BF0(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map2, queue into dynamic ring buffer — 598BF0
    void sub_599410(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map2, queue into static ring buffer — 599410
    int32_t sub_58A158(Unit* unit, uint16_t yx); // Classifies direction/octant from unit's position toward yx.
    int32_t sub_58BFA3(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2); // Chebyshev distance between two (x,y) points.
    uint8_t sub_592831(Unit* target, Unit* unit); // Classifies 16-way direction/octant from target toward unit. `this` is unused.

    void sub_58FB12(Unit* unit); // Reconciles eye position1/rotation-target state against the unit's current tile.
    int32_t sub_593C9A(Unit* unit); // Returns unit's speed, overridden by group_sub->field_0x44 if nonzero. `this` is unused.
    uint8_t sub_594C87(uint16_t yx); // Walk cost at yx, adjusted for area effects when the cell is occupied.

    void sub_5890CC(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // Builds unit->list1 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y) — 5890CC
    void sub_5893C6(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // Builds unit->list2 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y) — 5893C6
    uint16_t sub_592A48(Unit* unit, Unit* target); // 592A48
    int16_t sub_593134(Unit* unit, int32_t zero, uint16_t yx, uint32_t radius); // 593134
    Building* sub_594709(uint16_t yx); // 594709
    uint8_t sub_591A96(uint16_t yx1, uint16_t yx2, int32_t size); // 591A96
    void sub_597290(Unit* unit, uint8_t x, uint8_t y); // Expand field3_0x30000 cost grid from (x,y), gated by sub_596F80, queueing improved cells into queue2 — 597290
    void sub_5974B0(Unit* unit, uint8_t x, uint8_t y); // Expand field3_0x30000 cost grid from (x,y), gated by sub_596F80, queueing improved cells into queue1 — 5974B0
    void sub_597990(Unit* unit, uint8_t x, uint8_t y); // Expand field3_0x30000 cost grid from (x,y), gated by sub_5978F0, queueing improved cells into queue1 — 597990
    void sub_5976D0(Unit* unit, uint8_t x, uint8_t y); // Expand field3_0x30000 cost grid from (x,y), gated by sub_5978F0, queueing improved cells into queue2 — 5976D0
    int32_t sub_596F80(Unit* unit, uint16_t yx); // 596F80
    int32_t sub_5978F0(Unit* unit, uint16_t yx); // Passability check used by sub_5976D0/sub_597990 — 5978F0
    uint16_t* sub_59A670(uint8_t x, uint8_t y); // Get pointer into field3_0x30000 cost grid for (x,y) — 59A670
    uint8_t* sub_59A6F0(uint8_t x, uint8_t y); // Get pointer into walk_cost_map for (x,y) — 59A6F0

    // Member function pointer to a passability check with the same shape as sub_596F80/sub_5978F0.
    using PassableCheckFn = int32_t (MapStuff::*)(Unit*, uint16_t);

    // Shared by sub_597290/sub_5974B0/sub_597990/sub_5976D0: recomputes movement cost to each
    // of the 9 neighbor cells (including self) around (x,y) in the field3_0x30000 cost grid,
    // gated by the given passability check, and enqueues improved cells into the given queue.
    void ExpandCostGridNeighbors(Unit* unit, uint8_t x, uint8_t y, PassableCheckFn passable, uint8_t* out_x, uint8_t* out_y, uint16_t& out_count);

    // Shared by sub_5890CC/sub_5893C6: walks backward from (x,y) toward (cur_x,cur_y) through the
    // field3_0x30000 cost grid, picking the cheapest neighbor at each step (its cost weighted by
    // walk_cost_map at the current work position), appending each visited cell to `path`. On
    // success (reached (cur_x,cur_y) within 1000 steps) drops the initial (x,y) entry from the
    // front of `path`; on failure (gave up) clears `path` entirely.
    void WalkCostPath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, CList<PosYX>& path);

    void sub_5882AE(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, int32_t flag, Unit* target); // 5882AE

    inline Obstacle& ObstacleAt(PosYX yx) { return obstacle_map[yx.val]; } //59a7a0
    inline Obstacle& Obstacle2At(PosYX yx) { return obstacle_map2[yx.val]; } //59a7c0
    inline uint8_t& WalkCostAt(PosYX yx) { return walk_cost_map[yx.val]; } //59a780

    int IsObstacle2_40(PosYX yx) { return Obstacle2At(yx).TestBits(0x40); } //58b9ee
    void SetObstacle2_40(PosYX yx) { Obstacle2At(yx).SetBits(0x40); } //58b920
    int IsObstacle2_80(PosYX yx) { return Obstacle2At(yx).TestBits(0x80); } //58ba15
    void SetObstacle2_80(PosYX yx) { Obstacle2At(yx).SetBits(0x80); } //58b952

    void FUN_0058b5d6(CellState& cell); //58b5d6
    int FUN_0058b4a6(PosYX yx); //58b4a6
    void FUN_00596d3e(Unit* unit, PosYX yx); //596d3e
    void FUN_0058b3e0(PosYX yx); //58b3e0
    void FUN_00596e0e(Unit* unit, PosYX yx); //596e0e
    void FUN_005969c6(Unit* unit, PosYX yx, uint8_t t); //5969c6
    int sub_58B27F(Unit* unit, uint8_t x, uint8_t y); // Clear unit occupancy from cell (x,y), refresh state, cache eye position — 58B27F

    // Shared by sub_597BB0/sub_5983D0/sub_598BF0/sub_599410: relaxes path cost of the 8
    // neighbors of yx, skipping any neighbor blocked in obstacle_map (masked by flag_byte),
    // and pushes improved neighbors into the ring-buffer queue (queue_tail/queue).
    void PropagateNeighborCost(uint16_t yx, uint8_t flag_byte, int32_t cond_flag, Obstacle* obstacle_map, uint16_t& queue_tail, uint16_t* queue);

    // Shared by sub_591424/sub_59166C/sub_592831: classifies the 16-point compass direction of a
    // signed (dx, dy) offset into a raw octant index (0-15).
    uint8_t ClassifyDirection16(int32_t dx, int32_t dy);

    // Shared by sub_591424/sub_59166C: coarsens ClassifyDirection16's raw index into one of 8
    // facing-angle steps (0, 32, 64, ..., 224).
    uint8_t ClassifyFacing16(int32_t dx, int32_t dy);

    int32_t GetWidth() const { return map_width; } //58b8df
    int32_t GetHeight() const { return map_height; } //58b8f3
};
ASSERT_OFFSET(MapStuff, map_width, 0x50000);
ASSERT_OFFSET(MapStuff, scratch_cell_state, 0x5402c);
ASSERT_OFFSET(MapStuff, walk_cost, 0x54146);
ASSERT_OFFSET(MapStuff, field_0x54190, 0x54190);
ASSERT_OFFSET(MapStuff, field_0x54584, 0x54584);
ASSERT_OFFSET(MapStuff, static_scan_ahead, 0x58584);
ASSERT_OFFSET(MapStuff, visibility, 0x58ec0);
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


void __cdecl MissionGetBriefing(CString* out); //4e13db
void __cdecl MissionGetFailure(int32_t idx, CString* out); //4e14a4
void __cdecl MissionGetTips(int32_t idx, CString* out); //4e159b
void __cdecl MissionGetSubj(int32_t idx, CString* out); //4e16ce
void __cdecl MissionGetLocName(int32_t t, int32_t idx, CString* out); //4e17e3
void __cdecl MissionGetDescription(int32_t t, int32_t idx, CString* out); //4e1915
