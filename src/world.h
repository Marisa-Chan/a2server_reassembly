#pragma once

#include <cstdint>

#include "action.h"
#include <afx.h>
#include "assert_offset.h"
#include "logic.h"
#include "map_stuff.h"
#include <afxtempl.h>
#include "perf.h"
#include "unit_list.h"
#include "token.h"


struct TerrainMap;
struct Group;
class Player;
class PlayersList;
struct ResFile;
class Building;
class Player;
class Spell;
class Token;
class Unit;
struct World;

// Diplomacy relation byte: one bit space shared by the server matrix
// (Diplomacy::diplomacy[][]) and the client copy (Unit::diplomacy[], the
// DIPLOMACY_STATE packet payload). Bits 0x1/0x2/0x4/0x10 are the player's own
// settings (SET_DIPLOMACY accepts them masked with 0x17); bits 0x8/0x20/0x40
// are display-only mirrors the server derives in SendVisibilityState from the
// other player's matrix row. Row/col 0 of the matrix is metadata (is_ai flags,
// registration markers), not diplomacy bits.
enum DiplomacyFlag : uint8_t {
    DIPLO_WAR           = 0x1,  // at war
    DIPLO_ALLIANCE      = 0x2,  // allied
    DIPLO_MUTE          = 0x4,  // muted (DiplomacyEntry::mute)
    DIPLO_SHARED_VISION = 0x8,  // display only: the other shares vision with us
    DIPLO_VISION_SHARE  = 0x10, // we share vision with the other
    DIPLO_THEY_ALLIED   = 0x20, // display only: the other set alliance toward us
    DIPLO_THEY_WAR      = 0x40, // display only: the other set war toward us
};

namespace trigger_check_op {
enum : uint32_t {
    group_unit_count           = 1,
    unit_in_box                = 2,
    unit_in_circle             = 3,
    unit_param                 = 4,
    unit_alive                 = 5,
    unit_distance              = 6,
    point_to_unit_dist         = 7,
    player_unit_count          = 8,
    unit_attack_target         = 9,
    diplomacy                  = 10,
    noop_11                    = 11,
    item_in_inventory          = 12,
    noop_13                    = 13,
    sack_at_point              = 14,
    nearest_player_unit_dist   = 15,
    item_carrier_to_point_dist = 16,
    item_in_inventory_alt      = 17,
    vip_unit_alive             = 18,
    variable_value             = 19,
    player_building_count      = 20,
    building_health            = 21,
    teleport_unit_on_tile      = 22,
    scenario_variable          = 23,
    subobjective               = 24,
    spell_on_tile              = 25,
    spell_on_unit              = 26,
    unit_centered_on_tile      = 27,
    constant_value             = 0x10002,
};
} // namespace trigger_check_op

namespace script_action_op {
enum : uint32_t {
    increment_mission_stage = 0x01,
    send_game_event         = 0x02,
    set_variable            = 0x03,
    mission_complete        = 0x04,
    mission_fail            = 0x05,
    group_command           = 0x06,
    set_formation           = 0x07,
    increment_variable      = 0x08,
    noop_09                 = 0x09,
    change_diplomacy        = 0x0A,
    transfer_item           = 0x0B,
    give_item               = 0x0C,
    remove_item             = 0x0D,
    kill_unit               = 0x0E,
    kill_unit_alt           = 0x0F,
    phase_out_unit          = 0x10,
    phase_in_unit           = 0x11,
    polymorph_unit          = 0x12,
    change_unit_owner       = 0x13,
    drop_all_items          = 0x14,
    cast_area_spell         = 0x15,
    change_group_owner      = 0x16,
    give_money              = 0x17,
    cast_unit_spell         = 0x18,
    set_trap                = 0x19,
    set_building_health     = 0x1A,
    teleport_unit           = 0x1B,
    transfer_all_items      = 0x1C,
    set_tile_spell_duration = 0x1D,
    set_unit_spell_duration = 0x1E,
    set_repop_delay         = 0x1F,
    phase_out_group         = 0x20,
    phase_in_group          = 0x21,
    set_unit_param          = 0x22,
    set_scenario_variable   = 0x23,
    set_subobjective        = 0x24,
    remove_item_from_all    = 0x26,
    suspend_group           = 0x27,
    drop_location           = 0x10002,
};
} // namespace script_action_op

namespace mission_result {
enum : uint8_t {
    in_progress = 0,
    victory     = 1,
    hero_died   = 2,
};
} // namespace mission_result

struct Diplomacy {
public:
    uint8_t flags[4];
    World* world;
    uint8_t diplomacy[70][70];

public:
    Diplomacy(); // 5b5855 (client: 5839c5)
    void RegisterPlayer(Player* player); // 5b545f (client: 5835cf) - Register player in the diplomacy matrix.
    void RecordHostility(Unit* attacker, Unit* target, int32_t hit_flag); // 5b5643 (client: 5837b3)
};
ASSERT_SIZE(Diplomacy, 0x132C);

struct World {
    int32_t gap_0x0;
    uint8_t gap_0x4[4];
    Perf perf_tick;
    Perf perf_presence;
    Perf perf_triggers;
    Perf perf_ai;
    MapScratchBlock2K gap_0xc8;
    uint8_t gap_0x8ac[4];
    int32_t duration4;
    int32_t duration4_low;
    uint8_t gap_0x8b8[4];
    int32_t duration5;
    uint32_t total5;
    uint8_t gap_0x8c4[24];
    int32_t duration1;
    int32_t total1;
    int32_t duration2;
    int32_t total2;
    int32_t duration3;
    int32_t total3;
    uint8_t gap_0x8f4[340];
    uint8_t saved_0xa48;
    uint8_t saved_0xa49;
    uint8_t gap_0xa4a[2];
    uint32_t counter;
    TerrainMap *map_stuff;
    uint8_t attack_decision_table[16];
    UnitList nearby_list;
    UnitList unconscious_list;
    UnitList non_targets; // Non-attack list?
    UnitList targets; // Attack list?
    uint32_t dist_scratch[10000];
    uint8_t relative_move_max_dist;
    uint8_t gap_0xa725[3];
    uint8_t saved_0xa728[400];
    int32_t MinimalGuardRange;
    Diplomacy diplomacy;
    int32_t skip_presence_grid;
    CList<Action> action_list;
    uint8_t gap_0xbc08[400];
    Action scrape_action;
    uint8_t waypoint_table[16];
    uint8_t patrol_counter;
    uint8_t gap_0xbdfd[3];
    Unit* patrol_unit;
    int32_t gap_0xbe04;
    int32_t gap_0xbe08;
    int32_t mission_complete;
    int32_t saved_0xbe10;
    int32_t mission_fail;
    MapScratchBlock1K gap_0xbe18;
    uint8_t gap_0xc20c[0x320];
    uint8_t gap_0xc52c[40];
    char gap_0xc554[10][44];
    uint8_t gap_0xc70c[4];
    int32_t mission_state;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> gap_0xc714;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> gap_0xc730;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> gap_0xc74c;
    CArray<uint32_t> gap_0xc768;
    PlayersList* players_list;
    int32_t joined_count;
    int32_t trigger_variables[1000];
    void* references[1000]; // Array of User*, Group* and Player* objects
    uint8_t trigger_results[1000];
    CList<TriggerCheck>* trigger_checks;
    CArray<TriggerAction>* trigger_actions;
    CList<Trigger>* triggers;
    char name[100]; // Not sure it's 100 bytes
    ResFile* gap_0xeb1c;

public:
    World(TerrainMap* map_stuff, PlayersList* players_list); // 5a4b1a (client: 572c8a)
    ~World(); // 5a4d38 (client: 572ea8)

    void Init(); // 5a457a (client: 5726ea)
    void ReinitState(); // 5a44c5 (client: 572635)
    void BeginDestroy(); // 5a44db (client: 57264b)
    void InitState(); // 5a3f3a (client: 5720aa)
    void DestroyState(); // 5a3f88 (client: 5720f8)
    void ConstructMembers(); // 5a3f9b (client: 57210b)
    void DestroyMembers(); // 5a4284 (client: 5723f4)
    void ResetMatchState(); // 5a4757 (client: 5728c7)
    void Serialize(CArchive& ar); // 5b0556 (client: 57e6c6) - Serialize
    void RelinkPointers(); // 5b2e7a (client: 580fea) - Post-load world relink helper

    void IncrementJoinedCount(); // 5afbfd (client: 57dd6d) - increments joined_count
    void ProcessTick(class PlayersList* players); // 5abd16 (client: 579e86)
    void ProcessTriggers(); // 5b0724 (client: 57e894)
    void EvaluateTriggerChecks(); // 5b0762 (client: 57e8d2)
    void DispatchTriggerCheck(TriggerCheck* check); // 5b081e (client: 57e98e) - trigger-check dispatcher.
    void ExecuteTriggerActions(); // 5b0a6a (client: 57ebda)
    Unit* PickRelationUnitNear(uint16_t pos_yx, uint8_t byte_0x3c, Player* owner, int32_t is_defensive); // 5b61d0 (client: 584340)
    void ProcessVirtualCasters(); // 5b6346 (client: 5844b6)
    void CollectRayTargets(Unit* caster, Unit* target, CList<Unit*>* units, uint32_t rays, uint32_t max_range); // 5b5ca8 (client: 583e18) - collect units hit by a multi-ray spell
    uint16_t PackDistFacing(Unit* unit1, Unit* unit2); // 5b5c2b (client: 583d9b) - packed distance/facing metric between two units
    void ProcessPlayerGroupAi(Player* player); // 5abf50 (client: 57a0c0)

    // Group-order dispatch helpers (called from Server::ProcessClientPacket)
    void RefreshGroupGuardAnchor(Group* group); // 5ac137 (client: 57a2a7) - Refresh group_sub position/range fields.
    void GroupStopCommand(Group* group); // 5acaa7 (client: 57ac17)
    void OrderGatherLoot(Unit* unit); // 5a99c7 (client: 577b37)
    void GroupAggressiveStance(Group* group); // 5aca54 (client: 57abc4)
    void GroupAttackCommand(Group* group, Unit* target); // 5ac80f (client: 57a97f)
    void GroupAttackTargetCommand(Group* group, Unit* target, uint8_t range); // 5acb4d (client: 57acbd)
    void GroupMoveCommand(Group* group, uint8_t x, uint8_t y); // 5ac289 (client: 57a3f9) - Group move command.
    void GroupSwarmCommand(Group* group, uint8_t x, uint8_t y); // 5ac507 (client: 57a677) - Group swarm command.
    void GroupPatrolCommand(Group* group, uint8_t x, uint8_t y); // 5acbef (client: 57ad5f)
    void GroupCastAtUnitCommand(Group* group, Unit* target, Spell*); // 5ac187 (client: 57a2f7)
    void GroupCastAtPositionCommand(Group* group, uint8_t x, uint8_t y, Spell*); // 5ac206 (client: 57a376)
    void OrderSwarmTo(Unit* unit, uint8_t x, uint8_t y); // 5a9961 (client: 577ad1)
    void BeginUnitOrder(Unit* unit); // 5a9ac4 (client: 577c34)
    void OrderMoveTo(Unit* unit, uint8_t x, uint8_t y); // 5a8efb (client: 57706b) - Set unit movement target, clamped to map bounds.
    void OrderEnterBuilding(Unit* unit, Building* bldg); // 5a90f4 (client: 577264)
    void GroupAttackMove(Group* group, uint8_t x, uint8_t y); // 5ac881 (client: 57a9f1)
    void GroupGuardCommand(Group* group, uint8_t range); // 5ac8a2 (client: 57aa12) - Group guard command, range=0 means use world's MinimalGuardRange.
    void GroupGoHomeCommand(Group* group); // 5acafa (client: 57ac6a)
    void PlaceRespawnedGroup(Group* group, int guard_range); // 5ac983 (client: 57aaf3) - Place a newly respawned group into the world.
    void OrderAreaCast(Unit* unit, uint8_t x, uint8_t y, Spell* spell); // 5a930f (client: 57747f) - group area-cast setup.
    void GroupAttackMoveCommand(Group* group, uint8_t x, uint8_t y); // 5ac785 (client: 57a8f5)
    void GroupFollowCommand(Group* group, Unit* target, uint8_t range); // 5acb9e (client: 57ad0e)
    uint32_t ComputeGroupAvgPos(Group* group); // 5ab719 (client: 579889) - Compute average position of units in a group.
    uint8_t CanClaimMoveCell(Unit* unit, PosYX yx); // 5abb32 (client: 579ca2)
    void GroupRoamCommand(Group* group); // 5ac7c8 (client: 57a938)
    void OrderGuardPosition(Unit* unit); // 5a9383 (client: 5774f3)
    void OrderStandGround(Unit* unit); // 5a943b (client: 5775ab) - helper for GroupStopCommand.
    void OrderTeleportHome(Unit* unit); // 5a9918 (client: 577a88) - helper for GroupGoHomeCommand.
    void SetWithdrawThresholds(int32_t a, int32_t b, int32_t c, Player* player); // 5af805 (client: 57d975)
    void SetWimpyThresholds(int32_t a, int32_t b, int32_t c, Player* player); // 5afa01 (client: 57db71)
    void SetFormation(Player* player, uint8_t formation); // 5af683 (client: 57d7f3)
    void MonsterAiTick(Unit* unit); // 5a9b6b (client: 577cdb) - AI tick for a monster unit?
    void DoMoveStepTo(Unit* unit, PosYX yx, uint8_t max_range); // 5aa375 (client: 5784e5)
    void DoTurnTo(Unit* unit, uint8_t facing); // 5aa426 (client: 578596)
    void DoIdleFidget(Unit* unit); // 5aa485 (client: 5785f5)
    void StartAttack(Unit* unit); // 5aa78c (client: 5788fc)
    void FinishSwarmArrival(Unit* unit); // 5aa8ed (client: 578a5d)
    void StepTowardUnit(Unit* unit, Unit* target); // 5aa91b (client: 578a8b)
    void ChaseOrAttack(Unit* unit); // 5aa97e (client: 578aee)
    void CheckMoveArrival(Unit* unit); // 5aa9f6 (client: 578b66)
    void ChaseOrCast(Unit* unit); // 5aaa89 (client: 578bf9)
    void ChaseOrAreaCast(Unit* unit); // 5aab08 (client: 578c78)
    void ApproachBuilding(Unit* unit, Unit* target); // 5aab85 (client: 578cf5)
    void OrderEngageTarget(Unit* unit, Unit* target); // 5a9087 (client: 5771f7)
    void OrderCastAtUnit(Unit* unit, Unit* target, Spell* spell); // 5a92af (client: 57741f)
    void OrderAttackTarget(Unit* unit, Unit* target, uint8_t range); // 5a9482 (client: 5775f2)
    void OrderFollowTarget(Unit* unit, Unit* target, uint8_t range); // 5a9501 (client: 577671)
    void OrderPatrolTo(Unit* unit, uint8_t x, uint8_t y); // 5a9580 (client: 5776f0)
    void StartAreaCast(Unit* unit); // 5aa84f (client: 5789bf)
    int32_t GetAttackDecision(Unit* unit, Unit* target); // 5ad0b3 (client: 57b223)
    void ProcessGroupAi(Group* group); // 5abfbc (client: 57a12c)
    void FlagEmptyGroup(Group* group); // 5af623 (client: 57d793)
    void ProcessGroupRecall(Group* group); // 5af646 (client: 57d7b6)
    void ProcessUnitRecall(Unit* unit); // 5af5dc (client: 57d74c) - helper for ProcessGroupRecall.
    void IdleGroupUnits(Group* group); // 5af022 (client: 57d192)
    void RunUnitAi(Unit* unit); // 5abcfd (client: 579e6d)
    void ProcessUnitAi(Unit* unit); // 5a5155 (client: 5732c5) - helper for RunUnitAi.
    void RunAiUnitState(Unit* unit); // 5a5188 (client: 5732f8) - AI wrapper for ProcessUnitState.
    void ProcessUnitState(Unit* unit); // 5a51a1 (client: 573311) - per-state unit action dispatcher.
    void AiGuardBrain(Group* group); // 5ae628 (client: 57c798)
    void AiAttackBrain(Group* group, uint16_t target_yx); // 5aec20 (client: 57cd90)
    void AiAggroBrain(Group* group); // 5aef66 (client: 57d0d6)
    void AiMoveBrain(Group* group, uint16_t yx); // 5af061 (client: 57d1d1)
    void AiSwarmBrain(Group* group, uint16_t target_yx); // 5aece7 (client: 57ce57)
    void AiRoamBrain(Group* group); // 5aed8d (client: 57cefd)
    void AiDefendBrain(Group* group); // 5ae7c3 (client: 57c933)
    void AiStandBrain(Group* group); // 5aefe5 (client: 57d155)

    void DoScriptedPatrol(Unit* unit); // 5ab35b (client: 5794cb)
    void DoScriptedWalk(Unit* unit); // 5ab5e3 (client: 579753)
    void UpdateGroupCenter(Group* group); // 5ab92c (client: 579a9c)
    void ScanNearbyUnits(Group* group); // 5adb16 (client: 57bc86)
    int32_t GetTargetScore(Unit* unit, Unit* target); // 5ad2c1 (client: 57b431)
    void AssignBestTargets(Group* group); // 5ae471 (client: 57c5e1)
    void PruneNearbyUnits(uint16_t pos_yx, uint8_t max_distance); // 5a3d12 (client: 571e82)
    void AssignNearbyTargets(Group* group); // 5ae2d4 (client: 57c444)
    void JitterGuardRangeWide(Group* group); // 5ae5c0 (client: 57c730)
    void JitterGuardRange(Group* group); // 5ae5f5 (client: 57c765)
    void DoMoveStep(Unit* unit, uint16_t target_yx); // 5a8778 (client: 5768e8)
    void DoSwarmMoveStep(Unit* unit, uint16_t pos_yx); // 5a87a3 (client: 576913)
    void BeginChase(Unit* unit, Unit* target, uint8_t range); // 5a8829 (client: 576999)
    void DoAttackTarget(Unit* unit, Unit* target); // 5a6b48 (client: 574cb8)
    void TickAttackTarget(Unit* unit, Unit* target); // 5a5568 (client: 5736d8)
    void TickFollowTarget(Unit* unit, Unit* target); // 5a56d7 (client: 573847)
    void PickHealSpell(Unit* unit, Unit* target); // 5a57e8 (client: 573958)
    void PickAdjacentEnemy(Unit* unit, Unit* target); // 5a65bb (client: 57472b)
    void DoAttackMove(Unit* unit, uint16_t command_to); // 5a54b8 (client: 573628)

    void TickRangedApproach(Unit* unit); // 5aaebc (client: 57902c)
    void TurnToFacing(Unit* unit, uint8_t facing); // 5aaf84 (client: 5790f4) - turn unit to face target.
    void AcquireNearestTarget(Unit* unit); // 5aac17 (client: 578d87)
    void StartCastAtUnit(Unit* unit); // 5aa7cf (client: 57893f)
    int32_t RandomScaled(int32_t multiplier); // 5b6f40 (client: 585000)
    int32_t RandomRange(int32_t min, int32_t max); // 5b6f60 (client: 585020)
    void RecordUnitHit(Unit* target, Unit* attacker, int32_t hit_flag); // 5aa581 (client: 5786f1) - Record PvP hit and update combat log
    void ResetUnitToHold(Unit* unit); // 5a9a6a (client: 577bda) - World callback: register newly summoned/placed unit
    void DoPatrolTick(Unit* unit); // 5a59e2 (client: 573b52)
    void AdvancePatrolStep(Unit* unit); // 5a5b49 (client: 573cb9)
    void ResetUnitToIdle(Unit* unit); // 5a9a8f (client: 577bff) - reset unit to idle.
    void DoGatherLootStep(Unit* unit); // 5a6789 (client: 5748f9)
    void DoTeleportHome(Unit* unit); // 5a8885 (client: 5769f5)
    void TeleportToSafety(Unit* unit); // 5a8a55 (client: 576bc5)
    uint32_t ComputeListAvgPos(UnitList* list); // 5ab62e (client: 57979e)
    void ProcessAnimateDeadGroup(Group* group); // 5acdf4 (client: 57af64) - World callback for animate-dead group
    void DoScriptInstID(int32_t script_id); // 5b0e08 (client: 57ef78) - Run script by instance ID

    // Trigger check handlers, called from `DispatchTriggerCheck`.
    void CheckOP_1(TriggerCheck* check); // 5b2e91 (client: 581001) - How many units contains this group
    void CheckOP_2(TriggerCheck* check); // 5b2ef9 (client: 581069) - Is unit in a box
    void CheckOP_3(TriggerCheck* check); // 5b2f9a (client: 58110a) - Is unit in a circle
    void CheckOP_4(TriggerCheck* check); // 5b3071 (client: 5811e1) - Get unit parameter
    void CheckOP_5(TriggerCheck* check); // 5b2eb8 (client: 581028) - Is unit alive?
    void CheckOP_6(TriggerCheck* check); // 5b3131 (client: 5812a1) - Distance between units
    void CheckOP_7(TriggerCheck* check); // 5b3016 (client: 581186) - Distance from point to unit
    void CheckOP_8(TriggerCheck* check); // 5b30ad (client: 58121d) - How many units this player have
    void CheckOP_9(TriggerCheck* check); // 5b31c9 (client: 581339) - Is unit make attack
    void CheckOP_10(TriggerCheck* check); // 5b3222 (client: 581392) - Get diplomacy
    void CheckOP_12(TriggerCheck* check); // 5b3273 (client: 5813e3) - Item in inventory, same as check 17.
    void CheckOP_14(TriggerCheck* check); // 5b32cf (client: 58143f) - Get sack
    void CheckOP_15(TriggerCheck* check); // 5b3324 (client: 581494) - Distance to nearest units of player
    void CheckOP_16(TriggerCheck* check); // 5b33f0 (client: 581560) - Item to point
    void CheckOP_17(TriggerCheck* check); // 5b347a (client: 5815ea) - Item in inventory
    void CheckOP_18(TriggerCheck* check); // 5b34c9 (client: 581639) - VIP
    void CheckOP_19(TriggerCheck* check); // 5b34f7 (client: 581667) - Return value of variable
    void CheckOP_20(TriggerCheck* check); // 5b3525 (client: 581695) - How many buildings this player have
    void CheckOP_21(TriggerCheck* check); // 5b35a0 (client: 581710) - Health of building
    void CheckOP_22(TriggerCheck* check); // 5b35c6 (client: 581736) - Teleportation
    void CheckOP_23(TriggerCheck* check); // 5b3644 (client: 5817b4) - Get Scenario Variable
    void CheckOP_24(TriggerCheck* check); // 5b366d (client: 5817dd) - Get Subobjective
    void CheckOP_25(TriggerCheck* check); // 5b369c (client: 58180c) - Spell on Tile
    void CheckOP_26(TriggerCheck* check); // 5b3720 (client: 581890) - Spell on Unit
    void CheckOP_27(TriggerCheck* check); // 5b3776 (client: 5818e6) - Centered on Tile

    // Trigger action dispatch handlers (called from DoScriptInstID)
    void ScriptOP_0x01(TriggerAction* action); // 5b37f3 (client: 581963)
    void ScriptOP_0x02(TriggerAction* action); // 5b3b11 (client: 581c81)
    void ScriptOP_0x03(TriggerAction* action); // 5b3b33 (client: 581ca3)
    void ScriptOP_0x04(TriggerAction* action); // 5b3b79 (client: 581ce9)
    void ScriptOP_0x05(TriggerAction* action); // 5b3b9b (client: 581d0b)
    void ScriptOP_0x06(TriggerAction* action); // 5b3bb7 (client: 581d27)
    void ScriptOP_0x07(TriggerAction* action); // 5b3b56 (client: 581cc6)
    void ScriptOP_0x08(TriggerAction* action); // 5b3ae1 (client: 581c51)
    void ScriptOP_0x0a(TriggerAction* action); // 5b3d75 (client: 581ee5)
    void ScriptOP_0x0b(TriggerAction* action); // 5b3e56 (client: 581fc6)
    void ScriptOP_0x0c(TriggerAction* action); // 5b3ed4 (client: 582044)
    void ScriptOP_0x0d(TriggerAction* action); // 5b3f2f (client: 58209f)
    void ScriptOP_0x0e(TriggerAction* action); // 5b408d (client: 5821fd)
    void ScriptOP_0x0f(TriggerAction* action); // 5b40ba (client: 58222a)
    void ScriptOP_0x10(TriggerAction* action); // 5b40e7 (client: 582257)
    void ScriptOP_0x11(TriggerAction* action); // 5b40ff (client: 58226f)
    void ScriptOP_0x12(TriggerAction* action); // 5b4117 (client: 582287)
    void ScriptOP_0x13(TriggerAction* action); // 5b415a (client: 5822ca)
    void ScriptOP_0x14(TriggerAction* action); // 5b41f9 (client: 582369)
    void ScriptOP_0x15(TriggerAction* action); // 5b4287 (client: 5823f7)
    void ScriptOP_0x16(TriggerAction* action); // 5b4183 (client: 5822f3)
    void ScriptOP_0x17(TriggerAction* action); // 5b41d8 (client: 582348)
    void ScriptOP_0x18(TriggerAction* action); // 5b4309 (client: 582479)
    void ScriptOP_0x19(TriggerAction* action); // 5b4347 (client: 5824b7)
    void ScriptOP_0x1a(TriggerAction* action); // 5b4428 (client: 582598)
    void ScriptOP_0x1b(TriggerAction* action); // 5b4446 (client: 5825b6)
    void ScriptOP_0x1c(TriggerAction* action); // 5b4494 (client: 582604)
    void ScriptOP_0x1d(TriggerAction* action); // 5b455f (client: 5826cf)
    void ScriptOP_0x1e(TriggerAction* action); // 5b45e8 (client: 582758)
    void ScriptOP_0x1f(TriggerAction* action); // 5b4646 (client: 5827b6)
    void ScriptOP_0x20(TriggerAction* action); // 5b46bb (client: 58282b)
    void ScriptOP_0x21(TriggerAction* action); // 5b46ff (client: 58286f)
    void ScriptOP_0x22(TriggerAction* action); // 5b4743 (client: 5828b3)
    void ScriptOP_0x23(TriggerAction* action); // 5b382a (client: 58199a)
    void ScriptOP_0x24(TriggerAction* action); // 5b39df (client: 581b4f)
    void ScriptOP_0x26(TriggerAction* action); // 5b3fa7 (client: 582117)
    void ScriptOP_0x27(TriggerAction* action); // 5b4070 (client: 5821e0)

    void SplitTargetLists(Unit* unit, UnitList* pList); // 5a3ad6 (client: 571c46) - Populate attack/non-attack target lists for unit from pList.
    void FilterByRelation(Unit* unit, UnitList* list, int32_t flag); // 5a3896 (client: 571a06) - filter a UnitList by enemy/allied relationship.
    void PruneUnconsciousTargets(); // 5a3c5f (client: 571dcf) - move unconscious targets out of nearby_list into unconscious_list.
    UnitList* GetNearbyUnits(Unit* unit); // 5a37c5 (client: 571935) - get nearby UnitList around unit.
    UnitList* GetNearbyEnemies(Unit* caster, PosYX yx); // 5a3808 (client: 571978) - Get nearby enemy UnitList for caster at yx.
    UnitList* GetNearbyAllies(Unit* caster, PosYX yx); // 5a384f (client: 5719bf) - Get nearby ally UnitList for caster at yx.
    void Destroy(); // 5a4590 (client: 572700)
    uint8_t UnitMaxRange(Unit* unit); // 5a6adb (client: 574c4b) - Get effective max cast range for unit.
    void BuildTargetLists(Unit* unit); // 5a5e54 (client: 573fc4) - build active/alternate target lists around unit.
    void BeginPositionCast(Unit* caster, uint16_t pos_yx, Spell* spell); // 5a551c (client: 57368c) - set up a position-targeted cast action.
    void CastKnownSpell(Unit* unit, Unit* target, int32_t spell_id); // 5a6801 (client: 574971) - pick and start casting a known spell.
    void SetIdleOrRetreat(Unit* unit, int retreat); // 5a6e2c (client: 574f9c) - Set unit to idle/wander (retreat=0) or retreat (retreat!=0).
    void DoGuardScan(Unit* unit); // 5a607b (client: 5741eb) - idle/wander helper.
    void AdvancePatrol(Unit* unit); // 5a647c (client: 5745ec) - helper for state 10/24 unit actions.
    void DoRetreat(Unit* unit); // 5a6e59 (client: 574fc9) - retreat helper.
    void PursueTarget(Unit* unit, Unit* target); // 5a8709 (client: 576879) - set unit to pursue the chosen target.
    void IdleSelfBuff(Unit* unit); // 5a6ea5 (client: 575015) - idle/wander helper.
    int32_t ShouldPursueTarget(Unit* unit, Unit* target); // 5a6ed9 (client: 575049) - target-pursuit check.
    int32_t PickAttackSpell(Unit* unit, Unit* target); // 5b6632 (client: 5847a2) - AI mage spell selection: pick a random affordable non-defensive spell.
    void CastSelfHealOrBuff(Unit* unit); // 5a845b (client: 5765cb) - AI mage retreat helper: cast healing/buff on self or most-damaged ally.
    Spell* GetCastableSpell(Unit* unit, int32_t spell_number, int32_t flag); // 5a79d6 (client: 575b46) - Pick spell by number if unit has enough MP.
    Spell* GetAutobuffSpell(Unit* unit, int32_t spell_number); // autobuff-family hooks: GetCastableSpell honoring the per-player autobuff_mask
    int32_t ShouldAutobuff(Unit* unit, Unit* other); // 5a7a1c (client: 575b8c) - Check if `unit` should autobuff `other` (diplomacy/settings check).
    void DoAutobuff(Unit* unit); // 5a7b44 (client: 575cb4) - Autobuff handler: pick and cast buff spells on self or allies.
    int32_t IsSpellInRange(Unit* unit, Unit* other, Spell* spell); // 5a7af7 (client: 575c67) - Check if `other` is in range of spell from `unit`.
    void BeginAutobuffCast(Unit* caster, Unit* target, Spell* spell); // 5a85f4 (client: 576764) - Set up autobuff cast action for caster.
    void BeginEnterBuilding(Unit* unit, Building* building); // 5a6da3 (client: 574f13)

    // Helpers for the functions above.
    UnitList* BuildNearbyList(Unit* unit); // 5a3791 (client: 571901) - helper for GetNearbyUnits.
    void RemoveAirborneTargets(Unit* unit, UnitList* list); // 5a3dc9 (client: 571f39) - helper for GetNearbyUnits.
    int32_t WantsRetreat(Unit* unit); // 5a89d5 (client: 576b45) - helper for ProcessUnitRecall.
    int32_t MustFlee(Unit* unit); // 5a8bda (client: 576d4a) - helper for ProcessUnitRecall.
    void FleeFromEnemies(Unit* unit); // 5a8cd8 (client: 576e48) - helper for ProcessUnitRecall.
    void TeleportAwayFromEnemies(Unit* unit); // 5a8dbc (client: 576f2c) - helper for FleeFromEnemies.
    void OrderWalkTo(Unit* unit, uint8_t a, uint8_t b); // 5a925c (client: 5773cc) - helper for DoScriptedWalk.

    void FilterBuffTargets(Unit* unit, UnitList* list); // 5a39ad (client: 571b1d) - filter UnitList for CastSelfHealOrBuff.
    void PickFallbackBuff(Unit* unit, UnitList* list); // 5b6862 (client: 5849d2) - fallback buff selection for CastSelfHealOrBuff.
    void InitUnitCombatProfile(Unit* unit); // 5af6f5 (client: 57d865)
    void BuildAllyList(Group* group); // 5add64 (client: 57bed4) - Build nearby friendly unit list from `group`.
    bool AreAtWar(Unit* unit1, Unit* unit2); // 5b5816 (client: 583986) - Check if unit1's owner is at war with unit2's owner.

    void PrepareUnitOrder(Unit* unit); // 5a9832 (client: 5779a2)
    void OrderHoldPosition(Unit* unit); // 5a93f4 (client: 577564)
    void ClearGroupOrders(Group* grp); // 5acd4c (client: 57aebc)
};
ASSERT_OFFSET(World, duration4, 0x8b0);
ASSERT_OFFSET(World, counter, 0xa4c);
ASSERT_OFFSET(World, diplomacy, 0xa8bc);
ASSERT_OFFSET(World, mission_complete, 0xbe0c);
ASSERT_OFFSET(World, mission_state, 0xc710);
ASSERT_OFFSET(World, players_list, 0xc77c);
ASSERT_OFFSET(World, trigger_checks, 0xeaac);
ASSERT_SIZE(World, 0xeb20);

int32_t __cdecl RandInt(); // 5b6f30 (client: 584ff0) - rand() wrapper.
extern World* g_World;
