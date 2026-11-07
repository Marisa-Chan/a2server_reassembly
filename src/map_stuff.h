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


struct Obstacle {
    uint8_t bits;

    inline void SetBits(uint8_t _bits) { this->bits |= _bits; } //5970a0 (client: 5664f0)
    inline void UnsetBits(uint8_t _bits) { this->bits &= ~_bits; } //

    inline void AndBits(uint8_t _bits) { this->bits &= _bits; } //

    inline uint8_t TestBits(uint8_t _bits) { return this->bits & _bits; } //5970f0 (client: 566540)

    inline operator uint8_t() const { return this->bits; }
    inline void operator=(uint8_t val) { this->bits = val; }
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
    uint8_t  gap_0x161c[8];
    int32_t  unwatched_active_detections;        // count of units detected via active group while in an unwatched sector

public:
    ScanPresenceGrid(); // 5955B5 (client: 564a65)
    void RegisterAreaEffect(AreaEffect* ae); // Register area effect into the scan grid (RegisterAreaEffect) (client: 5654f7)
    void RebuildPresenceGrid();                 // Rebuild presence grid from all active units/players (client: 5655e1)
    void MarkWatchedSector(Unit* unit); // OR the unit owner's vision mask into the 5x5 sector block around the unit (5957E2) (client: 564c92)
    void DetectWatchedUnit(Unit* unit); // Mark unit detected when its sector is watched (595F46) (client: 5653f6)
    void ClearPresenceGrid();           // Clear the sector grid and reset per-unit/per-group detection state (5963B9) (client: 565869)
};
ASSERT_OFFSET(ScanPresenceGrid, unit_list, 0x1610);
ASSERT_SIZE(ScanPresenceGrid, 0x1628);

// Sub-field of MapStuff. Purpose not fully reversed; kept opaque.
struct FieldBlock54190 {
    uint32_t gap_0x0;
    uint32_t gap_0x4;
    uint8_t data[0x3ec];

public:
    FieldBlock54190(); // 599FF0 (client: 5692e0)
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
    FieldBlock5859c(); // 5A3180 (client: 4cc890)
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

    int IsEmpty() const; //58bef6 (client: 55b3a6)
    void Null(); //58a3da (client: 51f219)
    void RelinkPointers(); // Remaps stale on-disk pointer fields through g_Server's pointer remap table. (client: 5636ef)
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
    MapStuff(MapAlm* alm, UnitList* unit_list); // 587E19 (client: 5572c9)
    ~MapStuff(); // 587f70 (client: 557420)

    void InitFromUnitList(UnitList* unit_list); // 58E891 (client: 55dd41)
    void InitFromAlm(MapAlm* alm); // 58F166 (client: 55e616)
    uint8_t CalcTerrainWalkCost(uint16_t terrain_code, uint8_t* out_walk_cost); // Computes edge-blended walk cost + classification code for a terrain cell — 58F48D (client: 55e93d)
    void MarkMapBorder(); // Marks an 8-tile-thick obstacle border around the map edges — 58E670 (client: 55db20)

    int AddUnitToMap(Unit* unit); // add unit to map (client: 55d881)
    void Serialize(CArchive& ar); // Serialize (client: 559ff8)
    void EncodeTerrain(CWordArray* encode_buf); // encode map terrain into encode_buf (client: 563d60)
    bool IsInMapBounds(uint8_t x, uint8_t y) const; // 594076 (client: 563526)
    void RelinkAfterLoad(); // Post-load map relink helper (client: 5635d5)
    int CanPlaceSack(class Sack* sack); // Check if sack can be placed on map (client: 55d8b7)
    void RemoveSack(class Sack* sack); // Remove a sack token from the map (client: 55d9d5)
    Sack* GetSackAt(uint16_t x, uint16_t y); // Look up sack at map position (client: 55da77)
    Sack* GetSackAtPos(TokenPos* pos);  // client: 55daa3
    int CanPlaceUnit(Unit* unit); // Check if unit can be placed at current position (client: 5603ba)
    int PlaceUnit(Unit* unit); // Place unit on map at current TokenPos position (client: 55a1fa)
    int32_t CanOccupyCell(Unit* unit, uint8_t x, uint8_t y); // Checks whether unit can occupy cell (x,y), triggering any spell tile in the process. (client: 55a39f)
    uint8_t GetFacingFor(Unit* unit, PosYX yx); // Pick rotation angle for the unit to look at `yx`. (client: 560b1c)
    void TeleportUnit(Unit* unit, uint8_t x, uint8_t y); // Teleport unit to (x, y) (client: 56495c)
    void PlaceAreaEffectAt(AreaEffect* ae, uint8_t x, uint8_t y); // 59501E (client: 5644ce)
    int RemoveAreaEffectAt(AreaEffect* ae, uint16_t yx); // 595073 (client: 564523) — remove area effect from cell state at yx
    Unit* GetBuildingUnitAt(uint8_t x, uint8_t y); // 5946BF (client: 563b6f)
    void BlockCell(uint8_t x, uint8_t y); // 594768 (client: 563c18)
    int ApplyTerrainMod(uint16_t yx, void* src); // Apply terrain modification from 6-byte src buffer at yx (client: 565a26)
    AreaEffect** GetAreaEffectsAt(uint32_t yx); // Get pointer to area_effects[6] array at cell yx. (client: 56481c)
    int16_t GetTileMoveTime(Unit* unit, uint8_t x, uint8_t y); // Movement time for unit entering tile (x,y). (client: 56086d)
    int GetRangeTo(Unit* target, Unit* observer); // Visibility/range check between two units. (client: 560dbd)
    uint8_t GetFacingTo(Unit* unit, Unit* target); // Compute facing direction from unit to target. (client: 5608d4)
    int FacesTargetInRange(Unit* unit, Unit* target, uint8_t max_range); // Check if unit faces target and is in range. (client: 55f31d)
    int FacesPosInRange(Unit* unit, PosYX yx, uint8_t max_range); // Check if unit faces yx and is in range. (client: 55f38a)
    void MoveStepTowardPos(Unit* unit, PosYX yx, uint8_t max_range); // AI movement tick for unit toward yx. (client: 55f401)
    void MoveStepTowardUnit(Unit* unit, Unit* target, uint8_t max_range); // AI movement tick for unit pursuing target. (client: 55f73d)
    void FindPath(Unit* unit, uint8_t x, uint8_t y, int32_t flag, Unit* target);  // client: 55771d
    void UpdateEyeMoveTime(Unit* unit);  // client: 55fb28
    void AdvancePathList2(Unit* unit);  // client: 55fc6e
    void StepRotateToTarget(Unit* unit); // Rotates eye->facing toward eye->target_facing by up to rotation_speed. (client: 55f1df)
    void SetFacing(Unit* unit, uint8_t facing);  // client: 560444
    void PickNextPathTarget(Unit* unit, Unit* target);  // client: 55fdb2
    void FaceTarget(Unit* unit, Unit* target);  // client: 560d68
    void FacePosition(Unit* unit, PosYX yx);  // client: 560d92
    uint16_t PeekNextStep(Unit* unit);  // client: 562f54
    uint8_t ChebyshevDistance(PosYX yx1, PosYX yx2); // Chebyshev distance between two YX positions. (client: 562fd9)
    void GatherUnitsNear(uint16_t yx, uint8_t range, Player* owner, UnitList* out_list); // Gather units near yx within range into out_list. (client: 565987)
    int32_t CalcMoveStep(Unit* unit, uint16_t yx, uint8_t val); // Computes movement step magnitude/heading toward yx. (client: 563185)
    int IsAreaClear(Unit* unit, PosYX yx, int32_t flag);  // client: 566590
    int32_t RemoveBuilding(class Building* building); // Remove building from map (client: 563a9f)
    int32_t PlaceBuilding(class Building* building); // Place building on the map (client: 5639cf)
    int SetCellBuilding(Building* building, uint16_t yx); // 5943D0 — set building pointer in cell state at yx (client: 563880)
    int CanApplyAreaEffect(AreaEffect* ae, uint8_t x, uint8_t y); // Check if area effect can be applied at (x, y) — 595438 (client: 5648e8)
    uint8_t FacingDiff(uint8_t a, uint8_t b); // 595561 (client: 564a11) — smallest angular difference between two facing bytes.
    AreaEffect* AreaEffectCovers(AreaEffect* ae, uint16_t coord);  // Check if area effect covers cell coord — 5953CB (client: 56487b)
    Unit* GetUnitAt(PosYX yx); // Get unit at map coordinate yx — 58CA1B (client: 55becb)
    void RegisterAreaEffectAt(AreaEffect* ae, uint8_t x, uint8_t y); // 594E65 (client: 564315)
    int RegisterAreaEffectCell(AreaEffect* ae, uint16_t yx); // 594EBA — register area effect into cell state at yx (client: 56436a)
    UnitList* BuildNearbyUnitList(PosYX yx, uint8_t range); // Build a temporary UnitList of units around yx — 5897AA (client: 558c5a)
    int16_t ClampPosToMap(Unit* unit, uint32_t packed_yyxx, uint8_t range); // 5936D2 (client: 562b82)

    Unit* GetSmallUnitAt(uint16_t yx); // Get small unit occupying cell yx — 58CB5A (client: 55c00a)
    Unit* GetLargeUnitAt(uint16_t yx); // Get large unit occupying cell yx — 58CBB9 (client: 55c069)
    void CollectSackCells(Unit* unit, int32_t flag); // 594A2D — collect passable sack cells in expanding rings around the unit. (client: 563edd)
    int32_t IsPassableSackCell(Unit* unit, uint16_t yx); // 594C4B — check whether cell yx holds a sack and is passable for unit. (client: 5640fb)
    int UnitFitsAt(Unit* unit); // Check if unit fits/can be placed over all cells at its current position — 58B1D7 (client: 55a687)
    void RefreshCellObstacles(PosYX yx); // Refresh derived obstacle bits for cell yx from its stored cell state — 58B593 (client: 55aa43)
    Sack* GetSackInCell(uint16_t yx); // Get sack at cell yx if the cell has a stored state — 58E611 (client: 55dac1)
    int ClearCellBuilding(class Building* building, PosYX yx); // Clear building from the cell state at yx — 59449A (client: 56394a)
    int CanTraverseCell(Unit* unit, PosYX yx); // Check if unit can move onto and off of yx (both directions clear) — 595468 (client: 564918)
    void RelaxCostsMap1Queue2(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map, queue into dynamic ring buffer — 597BB0 (client: 567000)
    void RelaxCostsMap1Queue1(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map, queue into static ring buffer — 5983D0 (client: 567820)
    void RelaxCostsMap2Queue2(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map2, queue into dynamic ring buffer — 598BF0 (client: 568040)
    void RelaxCostsMap2Queue1(Unit* unit, uint16_t yx, uint8_t flag_byte, int32_t cond_flag); // Relax 8-neighbor path costs of yx against obstacle_map2, queue into static ring buffer — 599410 (client: 568860)
    int32_t ClassifyDirection8(Unit* unit, uint16_t yx); // Classifies direction/octant from unit's position toward yx. (client: 559608)
    int32_t ChebyshevDistanceXY(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2); // Chebyshev distance between two (x,y) points. (client: 55b453)
    uint8_t ClassifyDirection16(Unit* target, Unit* unit); // Classifies 16-way direction/octant from target toward unit. `this` is unused. (client: 561ce1)

    void SyncEyeState(Unit* unit); // Reconciles eye position1/rotation-target state against the unit's current tile. (client: 55efc2)
    int32_t GetEffectiveSpeed(Unit* unit); // Returns unit's speed, overridden by group_sub->speed_override if nonzero. `this` is unused. (client: 56314a)
    uint8_t GetWalkCost(uint16_t yx); // Walk cost at yx, adjusted for area effects when the cell is occupied. (client: 564137)

    void BuildPathList1(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // Builds unit->list1 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y) — 5890CC (client: 55857c)
    void BuildPathList2(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y); // Builds unit->list2 with a cost-grid-guided path from (x,y) back to (cur_x,cur_y) — 5893C6 (client: 558876)
    uint16_t PickAttackPosition(Unit* unit, Unit* target); // 592A48 (client: 561ef8)
    int16_t FindCheapestCellNear(Unit* unit, int32_t zero, uint16_t yx, uint32_t radius); // 593134 (client: 5625e4)
    Building* GetBuildingAt(uint16_t yx); // 594709 (client: 563bb9)
    uint8_t BoxDistance(uint16_t yx1, uint16_t yx2, int32_t size); // 591A96 (client: 560f46)
    void ExpandCostsStaticQueue2(Unit* unit, uint8_t x, uint8_t y); // Expand path_cost cost grid from (x,y), gated by IsCellPassableStatic, queueing improved cells into queue2 — 597290 (client: 5666e0)
    void ExpandCostsStaticQueue1(Unit* unit, uint8_t x, uint8_t y); // Expand path_cost cost grid from (x,y), gated by IsCellPassableStatic, queueing improved cells into queue1 — 5974B0 (client: 566900)
    void ExpandCostsDynamicQueue1(Unit* unit, uint8_t x, uint8_t y); // Expand path_cost cost grid from (x,y), gated by IsCellPassableDynamic, queueing improved cells into queue1 — 597990 (client: 566de0)
    void ExpandCostsDynamicQueue2(Unit* unit, uint8_t x, uint8_t y); // Expand path_cost cost grid from (x,y), gated by IsCellPassableDynamic, queueing improved cells into queue2 — 5976D0 (client: 566b20)
    int32_t IsCellPassableStatic(Unit* unit, uint16_t yx); // 596F80 (client: 566430)
    int32_t IsCellPassableDynamic(Unit* unit, uint16_t yx); // Passability check used by ExpandCostsDynamicQueue2/ExpandCostsDynamicQueue1 — 5978F0 (client: 566d40)
    uint16_t* GetCostGridAt(uint8_t x, uint8_t y); // Get pointer into path_cost cost grid for (x,y) — 59A670 (client: 569960)
    uint8_t* GetWalkCostMapAt(uint8_t x, uint8_t y); // Get pointer into walk_cost_map for (x,y) — 59A6F0 (client: 5699e0)

    // Member function pointer to a passability check with the same shape as IsCellPassableStatic/IsCellPassableDynamic.
    using PassableCheckFn = int32_t (MapStuff::*)(Unit*, uint16_t);

    // Shared by ExpandCostsStaticQueue2/ExpandCostsStaticQueue1/ExpandCostsDynamicQueue1/ExpandCostsDynamicQueue2: recomputes movement cost to each
    // of the 9 neighbor cells (including self) around (x,y) in the path_cost cost grid,
    // gated by the given passability check, and enqueues improved cells into the given queue.
    void ExpandCostGridNeighbors(Unit* unit, uint8_t x, uint8_t y, PassableCheckFn passable, uint8_t* out_x, uint8_t* out_y, uint16_t& out_count);

    // Shared by BuildPathList1/BuildPathList2: walks backward from (x,y) toward (cur_x,cur_y) through the
    // path_cost cost grid, picking the cheapest neighbor at each step (its cost weighted by
    // walk_cost_map at the current work position), appending each visited cell to `path`. On
    // success (reached (cur_x,cur_y) within 1000 steps) drops the initial (x,y) entry from the
    // front of `path`; on failure (gave up) clears `path` entirely.
    void WalkCostPath(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, CList<PosYX>& path);

    void FindPathInternal(Unit* unit, uint8_t cur_x, uint8_t cur_y, uint8_t x, uint8_t y, int32_t flag, Unit* target); // 5882AE (client: 55775e)

    inline Obstacle& ObstacleAt(PosYX yx) { return this->obstacle_map[yx.val]; } //59a7a0 (client: 569a60)
    inline Obstacle& Obstacle2At(PosYX yx) { return this->obstacle_map2[yx.val]; } //59a7c0 (client: 569a80)
    inline uint8_t& WalkCostAt(PosYX yx) { return this->walk_cost_map[yx.val]; } //59a780 (client: 569a40)

    int IsObstacle2_40(PosYX yx) { return this->Obstacle2At(yx).TestBits(0x40); } //58b9ee (client: 55ae9e)
    void SetObstacle2_40(PosYX yx) { this->Obstacle2At(yx).SetBits(0x40); } //58b920 (client: 55add0)
    int IsObstacle2_80(PosYX yx) { return this->Obstacle2At(yx).TestBits(0x80); } //58ba15 (client: 55aec5)
    void SetObstacle2_80(PosYX yx) { this->Obstacle2At(yx).SetBits(0x80); } //58b952 (client: 55ae02)

    void ApplyCellState(CellState& cell); //58b5d6 (client: 55aa86)
    int RestoreCellState(PosYX yx); //58b4a6 (client: 55a956)
    void ClearBlockerAt(Unit* unit, PosYX yx); //596d3e (client: 5661ee)
    void SnapshotCellState(PosYX yx); //58b3e0 (client: 55a890)
    void SetBlockerAt(Unit* unit, PosYX yx); //596e0e (client: 5662be)
    void MoveUnitBlockers(Unit* unit, PosYX yx, uint8_t t); //5969c6 (client: 565e76)
    int ClearUnitCell(Unit* unit, uint8_t x, uint8_t y); // Clear unit occupancy from cell (x,y), refresh state, cache eye position — 58B27F (client: 55a72f)

    // Shared by RelaxCostsMap1Queue2/RelaxCostsMap1Queue1/RelaxCostsMap2Queue2/RelaxCostsMap2Queue1: relaxes path cost of the 8
    // neighbors of yx, skipping any neighbor blocked in obstacle_map (masked by flag_byte),
    // and pushes improved neighbors into the ring-buffer queue (queue_tail/queue).
    void PropagateNeighborCost(uint16_t yx, uint8_t flag_byte, int32_t cond_flag, Obstacle* obstacle_map, uint16_t& queue_tail, uint16_t* queue);

    // Shared by GetFacingTo/GetFacingFor/ClassifyDirection16: classifies the 16-point compass direction of a
    // signed (dx, dy) offset into a raw octant index (0-15).
    uint8_t ClassifyDirection16(int32_t dx, int32_t dy);  // client: 561ce1

    // Shared by GetFacingTo/GetFacingFor: coarsens ClassifyDirection16's raw index into one of 8
    // facing-angle steps (0, 32, 64, ..., 224).
    uint8_t ClassifyFacing16(int32_t dx, int32_t dy);

    int32_t GetWidth() const { return this->map_width; } //58b8df (client: 55ad8f)
    int32_t GetHeight() const { return this->map_height; } //58b8f3 (client: 55ada3)
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
void __cdecl ReadRecord(char* buf, File2* file); //4a7b3c (client: 4abb6c)

class Scenario : public CObject {
public:
    ~Scenario(); //4a9468 (client: 4ad48f)

    Scenario(); //4a7a50 (client: 4abc1c)
    Scenario(const char* fname); //4a7bec (client: 4aba80)
    bool ReadAlmInfo(NetMapInfo* info); //44ec40 (client: 451f65)

    uint16_t* GetLandscape() const { return this->landscape; }; //41ee80 (client: 4203b0)
    int32_t GetWidth() const { return this->width; }; //41ef20 (client: 420240)
    int32_t GetHeight() const { return this->height; }; //41ef40 (client: 420260)

    int8_t* GetMapHeights() const { return this->map_heights; }; //41eea0 (client: 420280)
    uint8_t* GetObjectMap() const { return this->object_map; }; //41eec0 (client: 420680)
    uint8_t* GetOverlayMap() const { return this->overlay_map; }; //41eee0 (client: 420420)
    int32_t IsInShadedBounds(int32_t x, int32_t y); //41ef60 (client: 4202a0)

    double GetRenderSunAngle(); //4a7b79 (client: 4abba9)

    void UpdateTerrainShading(int32_t x, int32_t y, uint32_t w, uint32_t h); //4a952b (client: 4ad552)
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


void __cdecl MissionGetBriefing(CString* out); //4e13db (client: 4e5d5c)
void __cdecl MissionGetFailure(int32_t idx, CString* out); //4e14a4 (client: 4e5e25)
void __cdecl MissionGetTips(int32_t idx, CString* out); //4e15b9 (client: 4e5f3a)
void __cdecl MissionGetSubj(int32_t idx, CString* out); //4e16ce (client: 4e604f)
void __cdecl MissionGetLocName(int32_t t, int32_t idx, CString* out); //4e17e3 (client: 4e6164)
void __cdecl MissionGetDescription(int32_t t, int32_t idx, CString* out); //4e1915 (client: 4e6296)
