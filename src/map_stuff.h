#pragma once

#include <array>
#include <cstdint>

#include "2darray.h"
#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>
#include "perf.h"
#include "token.h"
#include "unit_list.h"
#include "visibility.h"


class AreaEffect;
class Building;
class Human;
struct MapAlm;
struct NetMapInfo;
class Sack;
class TokenPos;
class Unit;
struct World;

extern CString g_MissionText; //660f2c
extern CString g_MissionBriefing; //660de8
extern CStringArray g_MissionFailures; //660f18
extern CStringArray g_MissionSubjs; //660ea8


namespace obstacle_flag {
enum : uint8_t {
    land          = 0x01, // impassable terrain / water for land units
    air_border    = 0x02, // impassable border for flying units
    structure     = 0x04, // static map object / building / earth wall (blocks amphibious too)
    map_border    = 0x08, // outer map border flag
    dynamic_block = 0x10, // runtime terrain block (encoded in terrain sync)
    border_all    = 0x1F, // all static/border obstacle bits (8-tile map border)
    cell_state    = 0x20, // cell has an entry in MapStuff::cell_states
    ground_unit   = 0x40, // occupied by ground/amphibious unit in obstacle_map2
    air_unit      = 0x80, // occupied by flying unit in obstacle_map2
};
} // namespace obstacle_flag

namespace path_mask {
enum : uint8_t {
    land       = obstacle_flag::ground_unit | obstacle_flag::land,      // 0x41 (65)
    amphibious = obstacle_flag::ground_unit | obstacle_flag::structure, // 0x44 (68)
    flying     = obstacle_flag::air_unit | obstacle_flag::air_border,   // 0x82 (130)
};
} // namespace path_mask

struct Obstacle {
    uint8_t bits;

    inline void SetBits(uint8_t _bits) { this->bits |= _bits; } // 5970a0 (client: 5664f0)
    inline void UnsetBits(uint8_t _bits) { this->bits &= ~_bits; } //

    inline void AndBits(uint8_t _bits) { this->bits &= _bits; } //

    inline uint8_t TestBits(uint8_t _bits) { return this->bits & _bits; } // 5970f0 (client: 566540)

    inline operator uint8_t() const { return this->bits; }
    inline void operator=(uint8_t val) { this->bits = val; }
};

struct LandscapeTile {
    enum : uint16_t {
        SERVER_TERRAIN_MASK  = 0x03FF,
        CLIENT_TERRAIN_MASK  = 0x1FFF,
        SCORCHED_OR_IMP_FLAG = 0x2000,
        FOW_VISIBLE_NOW_BIT  = 0x4000,
        FOW_EXPLORED         = 0x8000,
        FOW_VISIBLE          = 0xC000,
        FOW_MASK             = 0xC000,
    };

    union {
        uint16_t raw;
        struct {
            uint16_t sub_tile        : 4; // [3:0]   sub-tile index (0..15)
            uint16_t variant         : 2; // [5:4]   terrain variant (0..3)
            uint16_t dir_idx         : 4; // [9:6]   server terrain / transition direction index
            uint16_t extra_tile_bits : 3; // [12:10] extended client tile set bits
            uint16_t scorched_or_imp : 1; // [13]    ALM impassable flag on disk / client scorched flag
            uint16_t visible_now     : 1; // [14]    currently in sight
            uint16_t explored        : 1; // [15]    previously explored
        };
        struct {
            uint16_t low_6           : 6; // [5:0]
            uint16_t terrain_group   : 7; // [12:6]  client terrain palette/tile group
            uint16_t high_3          : 3; // [15:13]
        };
        struct {
            uint16_t client_tile_id  : 13; // [12:0]
            uint16_t flags_3         : 3;  // [15:13]
        };
    };

    constexpr LandscapeTile(uint16_t v = 0) : raw(v) {}
    constexpr operator uint16_t() const { return this->raw; }
};
ASSERT_SIZE(LandscapeTile, 2);

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
    uint8_t  gap_0x161c[8];
    int32_t  unwatched_active_detections;        // count of units detected via active group while in an unwatched sector

public:
    ScanPresenceGrid(); // 5955b5 (client: 564a65)
    void RegisterAreaEffect(AreaEffect* ae); // 596047 (client: 5654f7) - Register area effect into the scan grid (RegisterAreaEffect)
    void RebuildPresenceGrid(); // 596131 (client: 5655e1) - Rebuild presence grid from all active units/players
    void MarkWatchedSector(Unit* unit); // 5957e2 (client: 564c92) - OR the unit owner's vision mask into the 5x5 sector block around the unit
    void DetectWatchedUnit(Unit* unit); // 595f46 (client: 5653f6) - Mark unit detected when its sector is watched
    void ClearPresenceGrid(); // 5963b9 (client: 565869) - Clear the sector grid and reset per-unit/per-group detection state
};
ASSERT_OFFSET(ScanPresenceGrid, unit_list, 0x1610);
ASSERT_SIZE(ScanPresenceGrid, 0x1628);

// Sub-field of MapStuff. Purpose not fully reversed; kept opaque.
struct FieldBlock54190 {
    uint32_t gap_0x0;
    uint32_t gap_0x4;
    uint8_t data[0x3ec];

public:
    FieldBlock54190(); // 599ff0 (client: 5692e0)
};
ASSERT_SIZE(FieldBlock54190, 0x3f4);

// Sub-field of MapStuff. Purpose not fully reversed; kept opaque.
struct FieldBlock5859c {
    uint8_t* gap_0x0;
    uint8_t* gap_0x4;
    uint8_t* gap_0x8;
    int32_t gap_0xc;
    uint8_t data[2004];

public:
    FieldBlock5859c(); // 5a3180 (client: 4cc890)
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

    int IsEmpty() const; // 58bef6 (client: 55b3a6)
    void Null(); // 58a3da (client: 55988a)
    void RelinkPointers(); // 59423f (client: 5636ef) - Remaps stale on-disk pointer fields through g_Server's pointer remap table.
};
ASSERT_OFFSET(CellState, area_effects, 0x14);
ASSERT_SIZE(CellState, 0x3c);

struct MapStuff { // aka astruct_5
    uint8_t walk_cost_map[65536];
    Obstacle obstacle_map[65536];
    Obstacle obstacle_map2[65536];
    uint16_t path_cost[65536];
    int32_t map_width;
    int32_t map_height;
    uint8_t path_queue_scratch[16384];
    uint16_t static_queue_count;
    uint16_t dynamic_queue_count;
    UnitList nearby_units;
    CellState scratch_cell_state; // Scratch `CellState` buffer.
    CList<void*> gap_0x54068;
    CMap<uint16_t, uint16_t, CellState, CellState> cell_states; // Tile yx -> cell state.
    MapAlm* alm;
    uint8_t gap_0x540a4[2];
    uint8_t direction_costs[4][16];
    uint8_t gap_0x540e6[64];
    uint16_t direction_pairs[16];
    uint8_t walk_cost[11];
    uint8_t gap_0x54151[5];
    int8_t line_walk_delta[12][2]; // (dx,dy) steps cycling right,down,left,up x3; used by PickAttackPosition's box perimeter walk.
    uint8_t gap_0x5416E[2];
    int32_t gap_0x54170[8];
    FieldBlock54190 gap_0x54190;
    uint16_t path_queue_static[0x1000]; // Path lookup table for static obstacles, used by FindPathInternal.
    uint16_t path_queue_dynamic[0x1000]; // Path lookup table for dynamic obstacles, used by FindPathInternal.
    int32_t static_scan_ahead;
    int32_t dynamic_scan_ahead;
    int32_t static_refresh_rate;
    int32_t dynamic_refresh_rate;
    int32_t dynamic_by_static_lookup;
    int32_t static_isnt_needed;
    FieldBlock5859c gap_0x5859c;
    World* world;
    int32_t speed_multiplier;
    CellState gap_0x58d88; // Seems unused?
    uint32_t gap_0x58dc4; // Seems unused?
    Perf gap_0x58dc8;
    Perf gap_0x58df8;
    Perf gap_0x58e28;
    Perf gap_0x58e58;
    int8_t neighbor_dx[8]; // Neighbor cell dx offsets.
    int8_t neighbor_dy[8]; // Neighbor cell dy offsets.
    int32_t neighbor_yx[8]; // Neighbor cell (dy,xy) offsets.
    uint8_t map_min_x;
    uint8_t map_min_y;
    uint8_t map_max_x;
    uint8_t map_max_y;
    int16_t map_min_xy;
    int16_t map_max_xy;
    Visibility visibility;
    ScanPresenceGrid scan_presence_grid;
    uint8_t height_map[65536];
    CList<void*> gap_0xa44f4;
    CList<uint16_t> sack_cells;
    Array2D<uint8_t, 8, 8> footprint_scratch;
    UnitList* all_units;

public:
    MapStuff(MapAlm* alm, UnitList* unit_list); // 587e19 (client: 5572c9)
    ~MapStuff(); // 587f70 (client: 557420)

    void InitFromUnitList(UnitList* unit_list); // 58e891 (client: 55dd41)
    void InitFromAlm(MapAlm* alm); // 58f166 (client: 55e616)
    uint8_t CalcTerrainWalkCost(uint16_t terrain_code, uint8_t* out_walk_cost); // 58f48d (client: 55e93d) - Computes edge-blended walk cost + classification code for a terrain cell
    void MarkMapBorder(); // 58e670 (client: 55db20) - Marks an 8-tile-thick obstacle border around the map edges

    int AddUnitToMap(Unit* unit); // 58e3d1 (client: 55d881) - add unit to map
    void Serialize(CArchive& ar); // 58ab48 (client: 559ff8) - Serialize
    void EncodeTerrain(CWordArray* encode_buf); // 5948b0 (client: 563d60) - encode map terrain into encode_buf
    bool IsInMapBounds(uint8_t x, uint8_t y) const; // 594076 (client: 563526)
    void RelinkAfterLoad(); // 594125 (client: 5635d5) - Post-load map relink helper
    int CanPlaceSack(class Sack* sack); // 58e407 (client: 55d8b7) - Check if sack can be placed on map
    void RemoveSack(class Sack* sack); // 58e525 (client: 55d9d5) - Remove a sack token from the map
    Sack* GetSackAt(uint16_t x, uint16_t y); // 58e5c7 (client: 55da77) - Look up sack at map position
    Sack* GetSackAtPos(TokenPos* pos); // 58e5f3 (client: 55daa3)
    int CanPlaceUnit(Unit* unit); // 590f0a (client: 5603ba) - Check if unit can be placed at current position
    int PlaceUnit(Unit* unit); // 58ad4a (client: 55a1fa) - Place unit on map at current TokenPos position
    int32_t CanOccupyCell(Unit* unit, uint8_t x, uint8_t y); // 58aeef (client: 55a39f) - Checks whether unit can occupy cell (x,y), triggering any spell tile in the process.
    uint8_t GetFacingFor(Unit* unit, PosYX yx); // 59166c (client: 560b1c) - Pick rotation angle for the unit to look at `yx`.
    void TeleportUnit(Unit* unit, uint8_t x, uint8_t y); // 5954ac (client: 56495c) - Teleport unit to (x, y)
    void PlaceAreaEffectAt(AreaEffect* ae, uint8_t x, uint8_t y); // 59501e (client: 5644ce)
    int RemoveAreaEffectAt(AreaEffect* ae, uint16_t yx); // 595073 (client: 564523) - remove area effect from cell state at yx
    Unit* GetBuildingUnitAt(uint8_t x, uint8_t y); // 5946bf (client: 563b6f)
    void BlockCell(uint8_t x, uint8_t y); // 594768 (client: 563c18)
    int ApplyTerrainMod(uint16_t yx, void* src); // 596576 (client: 565a26) - Apply terrain modification from 6-byte src buffer at yx
    AreaEffect** GetAreaEffectsAt(uint32_t yx); // 59536c (client: 56481c) - Get pointer to area_effects[6] array at cell yx.
    int16_t GetTileMoveTime(Unit* unit, uint8_t x, uint8_t y); // 5913bd (client: 56086d) - Movement time for unit entering tile (x,y).
    int GetRangeTo(Unit* target, Unit* observer); // 59190d (client: 560dbd) - Visibility/range check between two units.
    uint8_t GetFacingTo(Unit* unit, Unit* target); // 591424 (client: 5608d4) - Compute facing direction from unit to target.
    int FacesTargetInRange(Unit* unit, Unit* target, uint8_t max_range); // 58fe6d (client: 55f31d) - Check if unit faces target and is in range.
    int FacesPosInRange(Unit* unit, PosYX yx, uint8_t max_range); // 58feda (client: 55f38a) - Check if unit faces yx and is in range.
    void MoveStepTowardPos(Unit* unit, PosYX yx, uint8_t max_range); // 58ff51 (client: 55f401) - AI movement tick for unit toward yx.
    void MoveStepTowardUnit(Unit* unit, Unit* target, uint8_t max_range); // 59028d (client: 55f73d) - AI movement tick for unit pursuing target.
    void FindPath(Unit* unit, uint8_t x, uint8_t y, int32_t flag, Unit* target); // 58826d (client: 55771d)
    void UpdateEyeMoveTime(Unit* unit); // 590678 (client: 55fb28)
    void AdvanceLocalPath(Unit* unit); // 5907be (client: 55fc6e)
    void StepRotateToTarget(Unit* unit); // 58fd2f (client: 55f1df) - Rotates eye->facing toward eye->target_facing by up to rotation_speed.
    void SetFacing(Unit* unit, uint8_t facing); // 590f94 (client: 560444)
    void PickNextPathTarget(Unit* unit, Unit* target); // 590902 (client: 55fdb2)
    void FaceTarget(Unit* unit, Unit* target); // 5918b8 (client: 560d68)
    void FacePosition(Unit* unit, PosYX yx); // 5918e2 (client: 560d92)
    uint16_t PeekNextStep(Unit* unit); // 593aa4 (client: 562f54)
    uint8_t ChebyshevDistance(PosYX yx1, PosYX yx2); // 593b29 (client: 562fd9) - Chebyshev distance between two YX positions.
    void GatherUnitsNear(uint16_t yx, uint8_t range, Player* owner, UnitList* out_list); // 5964d7 (client: 565987) - Gather units near yx within range into out_list.
    int32_t CalcMoveStep(Unit* unit, uint16_t yx, uint8_t val); // 593cd5 (client: 563185) - Computes movement step magnitude/heading toward yx.
    int IsAreaClear(Unit* unit, PosYX yx, int32_t flag); // 597140 (client: 566590)
    int32_t RemoveBuilding(class Building* building); // 5945ef (client: 563a9f) - Remove building from map
    int32_t PlaceBuilding(class Building* building); // 59451f (client: 5639cf) - Place building on the map
    int SetCellBuilding(Building* building, uint16_t yx); // 5943d0 (client: 563880) - set building pointer in cell state at yx
    int CanApplyAreaEffect(AreaEffect* ae, uint8_t x, uint8_t y); // 595438 (client: 5648e8) - Check if area effect can be applied at (x, y)
    uint8_t FacingDiff(uint8_t a, uint8_t b); // 595561 (client: 564a11) - smallest angular difference between two facing bytes.
    AreaEffect* AreaEffectCovers(AreaEffect* ae, uint16_t coord); // 5953cb (client: 56487b) - Check if area effect covers cell coord
    Unit* GetUnitAt(PosYX yx); // 58ca1b (client: 55becb) - Get unit at map coordinate yx
    void RegisterAreaEffectAt(AreaEffect* ae, uint8_t x, uint8_t y); // 594e65 (client: 564315)
    int RegisterAreaEffectCell(AreaEffect* ae, uint16_t yx); // 594eba (client: 56436a) - register area effect into cell state at yx
    UnitList* BuildNearbyUnitList(PosYX yx, uint8_t range); // 5897aa (client: 558c5a) - Build a temporary UnitList of units around yx
    int16_t ClampPosToMap(Unit* unit, uint32_t packed_yyxx, uint8_t range); // 5936d2 (client: 562b82)

    Unit* GetSmallUnitAt(uint16_t yx); // 58cb5a (client: 55c00a) - Get small unit occupying cell yx
    Unit* GetLargeUnitAt(uint16_t yx); // 58cbb9 (client: 55c069) - Get large unit occupying cell yx
    void CollectSackCells(Unit* unit, int32_t flag); // 594a2d (client: 563edd) - collect passable sack cells in expanding rings around the unit.
    int32_t IsPassableSackCell(Unit* unit, uint16_t yx); // 594c4b (client: 5640fb) - check whether cell yx holds a sack and is passable for unit.
    int UnitFitsAt(Unit* unit); // 58b1d7 (client: 55a687) - Check if unit fits/can be placed over all cells at its current position
    void RefreshCellObstacles(PosYX yx); // 58b593 (client: 55aa43) - Refresh derived obstacle bits for cell yx from its stored cell state
    Sack* GetSackInCell(uint16_t yx); // 58e611 (client: 55dac1) - Get sack at cell yx if the cell has a stored state
    int ClearCellBuilding(class Building* building, PosYX yx); // 59449a (client: 56394a) - Clear building from the cell state at yx
    int CanTraverseCell(Unit* unit, PosYX yx); // 595468 (client: 564918) - Check if unit can move onto and off of yx (both directions clear)
    void RelaxStaticMapToDynamicQueue(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // 597bb0 (client: 567000) - Relax 8-neighbor path costs of yx against obstacle_map, queue into dynamic ring buffer
    void RelaxStaticMapToStaticQueue(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // 5983d0 (client: 567820) - Relax 8-neighbor path costs of yx against obstacle_map, queue into static ring buffer
    void RelaxDynamicMapToDynamicQueue(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // 598bf0 (client: 568040) - Relax 8-neighbor path costs of yx against obstacle_map2, queue into dynamic ring buffer
    void RelaxDynamicMapToStaticQueue(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // 599410 (client: 568860) - Relax 8-neighbor path costs of yx against obstacle_map2, queue into static ring buffer
    int32_t ClassifyDirection8(Unit* unit, uint16_t yx); // 58a158 (client: 559608) - Classifies direction/octant from unit's position toward yx.
    int32_t ChebyshevDistanceXY(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2); // 58bfa3 (client: 55b453) - Chebyshev distance between two (x,y) points.
    uint8_t ClassifyDirection16(Unit* target, Unit* unit); // split from 592831 (client: 561ce1) - Classifies 16-way direction/octant from target toward unit. `this` is unused.

    void SyncEyeState(Unit* unit); // 58fb12 (client: 55efc2) - Reconciles eye position1/rotation-target state against the unit's current tile.
    int32_t GetEffectiveSpeed(Unit* unit); // 593c9a (client: 56314a) - Returns unit's speed, overridden by group_sub->speed_override if nonzero. `this` is unused.
    uint8_t GetWalkCost(uint16_t yx); // 594c87 (client: 564137) - Walk cost at yx, adjusted for area effects when the cell is occupied.

    void BuildCoarsePath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // 5890cc (client: 55857c) - Builds unit->list1 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y)
    void BuildLocalPath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // 5893c6 (client: 558876) - Builds unit->list2 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y)
    uint16_t PickAttackPosition(Unit* unit, Unit* target); // 592a48 (client: 561ef8)
    int16_t FindCheapestCellNear(Unit* unit, int32_t zero, uint16_t yx, uint32_t radius); // 593134 (client: 5625e4)
    Building* GetBuildingAt(uint16_t yx); // 594709 (client: 563bb9)
    uint8_t BoxDistance(uint16_t yx1, uint16_t yx2, int32_t size); // 591a96 (client: 560f46)
    void ExpandStaticCostsToDynamicQueue(Unit* unit, uint8_t x, uint8_t y); // 597290 (client: 5666e0) - Expand path_cost cost grid from (x,y), gated by IsCellPassableStatic, queueing improved cells into queue2
    void ExpandStaticCostsToStaticQueue(Unit* unit, uint8_t x, uint8_t y); // 5974b0 (client: 566900) - Expand path_cost cost grid from (x,y), gated by IsCellPassableStatic, queueing improved cells into queue1
    void ExpandDynamicCostsToStaticQueue(Unit* unit, uint8_t x, uint8_t y); // 597990 (client: 566de0) - Expand path_cost cost grid from (x,y), gated by IsCellPassableDynamic, queueing improved cells into queue1
    void ExpandDynamicCostsToDynamicQueue(Unit* unit, uint8_t x, uint8_t y); // 5976d0 (client: 566b20) - Expand path_cost cost grid from (x,y), gated by IsCellPassableDynamic, queueing improved cells into queue2
    int32_t IsCellPassableStatic(Unit* unit, uint16_t yx); // 596f80 (client: 566430)
    int32_t IsCellPassableDynamic(Unit* unit, uint16_t yx); // 5978f0 (client: 566d40) - Passability check used by ExpandDynamicCostsToDynamicQueue/ExpandDynamicCostsToStaticQueue
    uint16_t* GetCostGridAt(uint8_t x, uint8_t y); // 59a670 (client: 569960) - Get pointer into path_cost cost grid for (x,y)
    uint8_t* GetWalkCostMapAt(uint8_t x, uint8_t y); // 59a6f0 (client: 5699e0) - Get pointer into walk_cost_map for (x,y)

    // Member function pointer to a passability check with the same shape as IsCellPassableStatic/IsCellPassableDynamic.
    using PassableCheckFn = int32_t (MapStuff::*)(Unit*, uint16_t);

    // Shared by ExpandStaticCostsToDynamicQueue/ExpandStaticCostsToStaticQueue/ExpandDynamicCostsToStaticQueue/ExpandDynamicCostsToDynamicQueue: recomputes movement cost to each
    // of the 9 neighbor cells (including self) around (x,y) in the path_cost cost grid,
    // gated by the given passability check, and enqueues improved cells into the given queue.
    void ExpandCostGridNeighbors(Unit* unit, uint8_t x, uint8_t y, PassableCheckFn passable, uint8_t* out_x, uint8_t* out_y, uint16_t& out_count); // split from 597290 (client: 5666e0)

    // Shared by BuildCoarsePath/BuildLocalPath: walks backward from (x,y) toward (cur_x,cur_y) through the
    // path_cost cost grid, picking the cheapest neighbor at each step (its cost weighted by
    // walk_cost_map at the current work position), appending each visited cell to `path`. On
    // success (reached (cur_x,cur_y) within 1000 steps) drops the initial (x,y) entry from the
    // front of `path`; on failure (gave up) clears `path` entirely.
    void WalkCostPath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, CList<PosYX>& path); // split from 5890cc (client: 55857c)

    void FindPathInternal(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, int32_t flag, Unit* target); // 5882ae (client: 55775e)

    inline Obstacle& ObstacleAt(PosYX yx) { return this->obstacle_map[yx.val]; } // 59a7a0 (client: 569a60)
    inline Obstacle& DynamicObstacleAt(PosYX yx) { return this->obstacle_map2[yx.val]; } // 59a7c0 (client: 569a80)
    inline uint8_t& WalkCostAt(PosYX yx) { return this->walk_cost_map[yx.val]; } // 59a780 (client: 569a40)

    int HasGroundUnitObstacle(PosYX yx) { return this->DynamicObstacleAt(yx).TestBits(obstacle_flag::ground_unit); } // 58b9ee (client: 55ae9e)
    void SetGroundUnitObstacle(PosYX yx) { this->DynamicObstacleAt(yx).SetBits(obstacle_flag::ground_unit); } // 58b920 (client: 55add0)
    int HasAirUnitObstacle(PosYX yx) { return this->DynamicObstacleAt(yx).TestBits(obstacle_flag::air_unit); } // 58ba15 (client: 55aec5)
    void SetAirUnitObstacle(PosYX yx) { this->DynamicObstacleAt(yx).SetBits(obstacle_flag::air_unit); } // 58b952 (client: 55ae02)

    void ApplyCellState(CellState& cell); // 58b5d6 (client: 55aa86)
    int RestoreCellState(PosYX yx); // 58b4a6 (client: 55a956)
    void ClearBlockerAt(Unit* unit, PosYX yx); // 596d3e (client: 5661ee)
    void SnapshotCellState(PosYX yx); // 58b3e0 (client: 55a890)
    void SetBlockerAt(Unit* unit, PosYX yx); // 596e0e (client: 5662be)
    void MoveUnitBlockers(Unit* unit, PosYX yx, uint8_t t); // 5969c6 (client: 565e76)
    int ClearUnitCell(Unit* unit, uint8_t x, uint8_t y); // 58b27f (client: 55a72f) - Clear unit occupancy from cell (x,y), refresh state, cache eye position

    // Shared by RelaxStaticMapToDynamicQueue/RelaxStaticMapToStaticQueue/RelaxDynamicMapToDynamicQueue/RelaxDynamicMapToStaticQueue: relaxes path cost of the 8
    // neighbors of yx, skipping any neighbor blocked in obstacle_map (masked by flag_byte),
    // and pushes improved neighbors into the ring-buffer queue (queue_tail/queue).
    void PropagateNeighborCost(uint16_t yx, uint8_t flag_byte, int32_t cond_flag, Obstacle* obstacle_map, uint16_t& queue_tail, uint16_t* queue); // split from 597bb0 (client: 567000)

    // Shared by GetFacingTo/GetFacingFor/ClassifyDirection16: classifies the 16-point compass direction of a
    // signed (dx, dy) offset into a raw octant index (0-15).
    uint8_t ClassifyDirection16(int32_t dx, int32_t dy); // split from 592831 (client: 561ce1)

    // Shared by GetFacingTo/GetFacingFor: coarsens ClassifyDirection16's raw index into one of 8
    // facing-angle steps (0, 32, 64, ..., 224).
    uint8_t ClassifyFacing16(int32_t dx, int32_t dy); // split from 591424 (client: 5608d4)

    int32_t GetWidth() const { return this->map_width; } // 58b8df (client: 55ad8f)
    int32_t GetHeight() const { return this->map_height; } // 58b8f3 (client: 55ada3)
};
ASSERT_OFFSET(MapStuff, map_width, 0x50000);
ASSERT_OFFSET(MapStuff, scratch_cell_state, 0x5402c);
ASSERT_OFFSET(MapStuff, walk_cost, 0x54146);
ASSERT_OFFSET(MapStuff, gap_0x54190, 0x54190);
ASSERT_OFFSET(MapStuff, path_queue_static, 0x54584);
ASSERT_OFFSET(MapStuff, static_scan_ahead, 0x58584);
ASSERT_OFFSET(MapStuff, visibility, 0x58ec0);
ASSERT_OFFSET(MapStuff, scan_presence_grid, 0x92ecc);
ASSERT_OFFSET(MapStuff, height_map, 0x944f4);
ASSERT_SIZE(MapStuff, 0xa4570);


class File2;

// Reads one length-prefixed record from the scenario file into buf.
void __cdecl ReadRecord(char* buf, File2* file); // 4a7b3c (client: 4abb6c)

class Scenario : public CObject {
public:
    ~Scenario(); // 4a9468 (client: 4ad48f)

    Scenario(); // 4a7a50 (client: 4aba80)
    Scenario(const char* fname); // 4a7bec (client: 4abc1c)
    bool ReadAlmInfo(NetMapInfo* info); // 44ec40 (client: 451f65)

    uint16_t* GetLandscape() const { return this->landscape; }; // 41ee80 (client: 4203b0)
    int32_t GetWidth() const { return this->width; }; // 41ef20 (client: 420240)
    int32_t GetHeight() const { return this->height; }; // 41ef40 (client: 420260)

    int8_t* GetMapHeights() const { return this->map_heights; }; // 41eea0 (client: 420280)
    uint8_t* GetObjectMap() const { return this->object_map; }; // 41eec0 (client: 420680)
    uint8_t* GetOverlayMap() const { return this->overlay_map; }; // 41eee0 (client: 420420)
    int32_t IsInShadedBounds(int32_t x, int32_t y); // 41ef60 (client: 4202a0)

    double GetRenderSunAngle(); // 4a7b79 (client: 4abba9)

    void UpdateTerrainShading(int32_t x, int32_t y, uint32_t w, uint32_t h); // 4a952b (client: 4ad552)
public:
    int32_t width;
    int32_t height;
    uint16_t* landscape;
    int8_t* map_heights;
    uint8_t* object_map;
    uint8_t* overlay_map;
    uint8_t abrightness;
    uint8_t contrast;
    uint8_t gap_0x1e;
    uint8_t gap_0x1f;
    double inv_sun_angle;
    int32_t terrain_tile_mask;
    int32_t daytime_minutes;
};
ASSERT_SIZE(Scenario, 0x30);

struct ScenarioLocation {
    int32_t kind = 0;
    int32_t id = 0;
    CRect rect;

    int GetId() { return this->id; } // 475150 (client: 477f20)
    CRect& GetRect() { return this->rect; } // 475170 (client: 477c50)
    int32_t GetKind() { return this->kind; }

    void Read(CFile* fl); // scenario.dll: 10003a20
    void Write(CFile* fl); // scenario.dll: 10003990

    ScenarioLocation() = default;

    ScenarioLocation(int32_t _kind, int32_t _id, int32_t x, int32_t y, int32_t w, int32_t h)
    : kind(_kind), id(_id), rect(x, y, x + w, y + h) {}
};
ASSERT_SIZE(ScenarioLocation, 0x18);


void __cdecl MissionGetBriefing(CString* out); // 4e13db (client: 4e5d5c)
void __cdecl MissionGetFailure(int32_t idx, CString* out); // 4e14a4 (client: 4e5e25)
void __cdecl MissionGetTips(int32_t idx, CString* out); // 4e15b9 (client: 4e5f3a)
void __cdecl MissionGetSubj(int32_t idx, CString* out); // 4e16ce (client: 4e604f)
void __cdecl MissionGetLocName(int32_t t, int32_t idx, CString* out); // 4e17e3 (client: 4e6164)
void __cdecl MissionGetDescription(int32_t t, int32_t idx, CString* out); // 4e1915 (client: 4e6296)
