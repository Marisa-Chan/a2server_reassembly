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


struct MapStuff;
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

struct Diplomacy {
public:
    uint8_t flags[4];
    World* world;
    uint8_t diplomacy[70][70];

public:
    Diplomacy(); // 5B5855 (client: 5839c5)
    void RegisterPlayer(Player* player); // Register player in the diplomacy matrix. 5B545F (client: 5835cf)
    void RecordHostility(Unit* attacker, Unit* target, int32_t hit_flag);
};
ASSERT_SIZE(Diplomacy, 0x132C);

struct World {
    int32_t field0_0x0;
    uint8_t gap_0x4[4];
    Perf field2_0x8;
    Perf field3_0x38;
    Perf field4_0x68;
    Perf field5_0x98;
    FieldBlock5859c field_0xc8;
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
    uint8_t field20_0xa48;
    uint8_t field21_0xa49;
    uint8_t gap_0xa4a[2];
    uint32_t counter;
    MapStuff *field24_0xa50;
    uint8_t attack_decision_table[16];
    UnitList field26_0xa64;
    UnitList field27_0xa84;
    UnitList field28_0xaa4; // Non-attack list?
    UnitList field29_0xac4; // Attack list?
    uint32_t dist_scratch[10000];
    uint8_t relative_move_max_dist;
    uint8_t gap_0xa725[3];
    uint8_t field32_0xa728[400];
    int32_t MinimalGuardRange;
    Diplomacy diplomacy;
    int32_t field37_0xbbe8;
    CList<Action> action_list;
    uint8_t field39_0xbc08[400];
    Action scrape_action;
    uint8_t waypoint_table[16];
    uint8_t patrol_counter;
    uint8_t gap_0xbdfd[3];
    Unit* field48_0xbe00;
    int32_t field49_0xbe04;
    int32_t field50_0xbe08;
    int32_t mission_complete;
    int32_t field52_0xbe10;
    int32_t mission_fail;
    FieldBlock54190 field54_0xbe18;
    uint8_t gap_0xc20c[0x320];
    uint8_t field56_0xc52c[40];
    char field57_0xc554[10][44];
    uint8_t gap_0xc70c[4];
    int32_t mission_state;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> field60_0xc714;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> field61_0xc730;
    CMap<uint32_t, uint32_t, uint32_t, uint32_t> field62_0xc74c;
    CArray<uint32_t> field63_0xc768;
    PlayersList* players_list;
    int32_t field65_0xc780;
    int32_t trigger_variables[1000];
    void* references[1000]; // Array of User*, Group* and Player* objects
    uint8_t trigger_results[1000];
    CList<TriggerCheck>* trigger_checks;
    CArray<TriggerAction>* trigger_actions;
    CList<Trigger>* triggers;
    char name[100]; // Not sure it's 100 bytes
    ResFile* field74_0xeb1c;

public:
    World(MapStuff* map_stuff, PlayersList* players_list); // 5A4B1A (client: 572c8a)
    ~World();

    void Init(); // 5A457A
    void ReinitState(); // 5A44C5
    void BeginDestroy(); // 5A44DB
    void InitState(); // 5A3F3A (client: 5720aa)
    void DestroyState(); // 5A3F88
    void ConstructMembers(); // 5A3F9B (client: 57210b)
    void DestroyMembers(); // 5A4284 (client: 5723f4)
    void ResetMatchState(); // 5A4757 (client: 5728c7)
    void Serialize(CArchive& ar); // Serialize
    void RelinkPointers(); // Post-load world relink helper

    void IncrementJoinedCount(); // increments field65_0xc780
    void ProcessTick(class PlayersList* players);
    void ProcessTriggers(); // 5B0724 (client: 57e894)
    void EvaluateTriggerChecks(); // 5B0762 (client: 57e8d2)
    void DispatchTriggerCheck(TriggerCheck* check); // 5B081E — trigger-check dispatcher. (client: 57e98e)
    void ExecuteTriggerActions(); // 5B0A6A (client: 57ebda)
    Unit* PickRelationUnitNear(uint16_t pos_yx, uint8_t byte_0x3c, Player* owner, int32_t is_defensive); // 5B61D0 (client: 584340)
    void ProcessVirtualCasters(); // 5B6346 (client: 5844b6)
    void CollectRayTargets(Unit* caster, Unit* target, CList<Unit*>* units, uint32_t rays, uint32_t max_range); // 5B5CA8: collect units hit by a multi-ray spell
    uint16_t PackDistFacing(Unit* unit1, Unit* unit2); // 5B5C2B: packed distance/facing metric between two units (client: 583d9b)
    void ProcessPlayerGroupAi(Player* player); // 5ABF50 (client: 57a0c0)

    // Group-order dispatch helpers (called from Server::ProcessClientPacket)
    void RefreshGroupGuardAnchor(Group* group); // Refresh group_sub position/range fields.
    void GroupStopCommand(Group* group);
    void OrderGatherLoot(Unit* unit);
    void GroupAggressiveStance(Group* group); //5aca54
    void GroupAttackCommand(Group* group, Unit* target);
    void GroupAttackTargetCommand(Group* group, Unit* target, uint8_t range);
    void GroupMoveCommand(Group* group, uint8_t x, uint8_t y); // Group move command.
    void GroupSwarmCommand(Group* group, uint8_t x, uint8_t y); // Group swarm command.
    void GroupPatrolCommand(Group* group, uint8_t x, uint8_t y);
    void GroupCastAtUnitCommand(Group* group, Unit* target, Spell*);
    void GroupCastAtPositionCommand(Group* group, uint8_t x, uint8_t y, Spell*);
    void OrderSwarmTo(Unit* unit, uint8_t x, uint8_t y);
    void BeginUnitOrder(Unit* unit); //5a9ac4 (client: 577c34)
    void OrderMoveTo(Unit* unit, uint8_t x, uint8_t y); // Set unit movement target, clamped to map bounds.
    void OrderEnterBuilding(Unit* unit, Building* bldg);
    void GroupAttackMove(Group* group, uint8_t x, uint8_t y);
    void GroupGuardCommand(Group* group, uint8_t range); // Group guard command, range=0 means use world's MinimalGuardRange.
    void GroupGoHomeCommand(Group* group);
    void PlaceRespawnedGroup(Group* group, int guard_range); // Place a newly respawned group into the world.
    void OrderAreaCast(Unit* unit, uint8_t x, uint8_t y, Spell* spell); // 5A930F — group area-cast setup (ASM). (client: 57747f)
    void GroupAttackMoveCommand(Group* group, uint8_t x, uint8_t y);
    void GroupFollowCommand(Group* group, Unit* target, uint8_t range);
    uint32_t ComputeGroupAvgPos(Group* group); // Compute average position of units in a group.
    uint8_t CanClaimMoveCell(Unit* unit, PosYX yx); // 5ABB32 (client: 579ca2)
    void GroupRoamCommand(Group* group);
    void OrderGuardPosition(Unit* unit);
    void OrderStandGround(Unit* unit); // 5A943B — helper for GroupStopCommand.
    void OrderTeleportHome(Unit* unit); // 5A9918 — helper for GroupGoHomeCommand. (client: 577a88)
    void SetWithdrawThresholds(int32_t a, int32_t b, int32_t c, Player* player);
    void SetWimpyThresholds(int32_t a, int32_t b, int32_t c, Player* player);
    void SetFormation(Player* player, uint8_t formation);
    void MonsterAiTick(Unit* unit); // AI tick for a monster unit? 5A9B6B (client: 577cdb)
    void DoMoveStepTo(Unit* unit, PosYX yx, uint8_t max_range); // 5AA375 (client: 5784e5)
    void DoTurnTo(Unit* unit, uint8_t facing); // 5AA426 (client: 578596)
    void DoIdleFidget(Unit* unit); // 5AA485 (client: 5785f5)
    void StartAttack(Unit* unit); // 5AA78C (client: 5788fc)
    void FinishSwarmArrival(Unit* unit); // 5AA8ED (client: 578a5d)
    void StepTowardUnit(Unit* unit, Unit* target);
    void ChaseOrAttack(Unit* unit); // 5AA97E (client: 578aee)
    void CheckMoveArrival(Unit* unit); // 5AA9F6 (client: 578b66)
    void ChaseOrCast(Unit* unit);
    void ChaseOrAreaCast(Unit* unit); // 5AAB08 (client: 578c78)
    void ApproachBuilding(Unit* unit, Unit* target); // 5AAB85 (client: 578cf5)
    void OrderEngageTarget(Unit* unit, Unit* target); // 5A9087 (client: 5771f7)
    void OrderCastAtUnit(Unit* unit, Unit* target, Spell* spell); // 5A92AF (client: 57741f)
    void OrderAttackTarget(Unit* unit, Unit* target, uint8_t range); // 5A9482
    void OrderFollowTarget(Unit* unit, Unit* target, uint8_t range); // 5A9501
    void OrderPatrolTo(Unit* unit, uint8_t x, uint8_t y); // 5A9580 (client: 5776f0)
    void StartAreaCast(Unit* unit); // 5AA84F (client: 5789bf)
    int32_t GetAttackDecision(Unit* unit, Unit* target); // 5AD0B3 (client: 57b223)
    void ProcessGroupAi(Group* group); // 5ABFBC (client: 57a12c)
    void FlagEmptyGroup(Group* group); // 5AF623 (client: 57d793)
    void ProcessGroupRecall(Group* group); // 5AF646
    void ProcessUnitRecall(Unit* unit); // 5AF5DC — helper for ProcessGroupRecall. (client: 57d74c)
    void IdleGroupUnits(Group* group); // 5AF022 (client: 57d192)
    void RunUnitAi(Unit* unit); // 5ABCFD
    void ProcessUnitAi(Unit* unit); // 5A5155 — helper for RunUnitAi. (client: 5732c5)
    void RunAiUnitState(Unit* unit); // 5A5188 — AI wrapper for ProcessUnitState.
    void ProcessUnitState(Unit* unit); // 5A51A1 — per-state unit action dispatcher. (client: 573311)
    void AiGuardBrain(Group* group); // 5AE628 (client: 57c798)
    void AiAttackBrain(Group* group, uint16_t target_yx); // 5AEC20 (client: 57cd90)
    void AiAggroBrain(Group* group); // 5AEF66 (client: 57d0d6)
    void AiMoveBrain(Group* group, uint16_t yx); // 5AF061 (client: 57d1d1)
    void AiSwarmBrain(Group* group, uint16_t target_yx); // 5AECE7 (client: 57ce57)
    void AiRoamBrain(Group* group); // 5AED8D (client: 57cefd)
    void AiDefendBrain(Group* group); // 5AE7C3 (client: 57c933)
    void AiStandBrain(Group* group); // 5AEFE5

    void DoScriptedPatrol(Unit* unit); // 5AB35B (client: 5794cb)
    void DoScriptedWalk(Unit* unit); // 5AB5E3 (client: 579753)
    void UpdateGroupCenter(Group* group); // 5AB92C (client: 579a9c)
    void ScanNearbyUnits(Group* group); // 5ADB16 (client: 57bc86)
    int32_t GetTargetScore(Unit* unit, Unit* target); // 5AD2C1 (client: 57b431)
    void AssignBestTargets(Group* group); // 5AE471 (client: 57c5e1)
    void PruneNearbyUnits(uint16_t pos_yx, uint8_t max_distance); // 5A3D12 (client: 571e82)
    void AssignNearbyTargets(Group* group); // 5AE2D4 (client: 57c444)
    void JitterGuardRangeWide(Group* group); // 5AE5C0 (client: 57c730)
    void JitterGuardRange(Group* group); // 5AE5F5 (client: 57c765)
    void DoMoveStep(Unit* unit, uint16_t target_yx); // 5A8778
    void DoSwarmMoveStep(Unit* unit, uint16_t pos_yx); // 5A87A3 (client: 576913)
    void BeginChase(Unit* unit, Unit* target, uint8_t range); // 5A8829 (client: 576999)
    void DoAttackTarget(Unit* unit, Unit* target); // 5A6B48 (client: 574cb8)
    void TickAttackTarget(Unit* unit, Unit* target); // 5A5568 (client: 5736d8)
    void TickFollowTarget(Unit* unit, Unit* target); // 5A56D7 (client: 573847)
    void PickHealSpell(Unit* unit, Unit* target); // 5A57E8 (client: 573958)
    void PickAdjacentEnemy(Unit* unit, Unit* target); // 5A65BB (client: 57472b)
    void DoAttackMove(Unit* unit, uint16_t command_to); // 5A54B8 (client: 573628)

    void TickRangedApproach(Unit* unit); // 5AAEBC (client: 57902c)
    void TurnToFacing(Unit* unit, uint8_t facing); // 5AAF84 — turn unit to face target (ASM). (client: 5790f4)
    void AcquireNearestTarget(Unit* unit); // 5AAC17 (client: 578d87)
    void StartCastAtUnit(Unit* unit); // 5AA7CF (client: 57893f)
    int32_t RandomScaled(int32_t multiplier); // 5B6F40 (client: 585000)
    int32_t RandomRange(int32_t min, int32_t max); // 5B6F60 (client: 585020)
    void RecordUnitHit(Unit* target, Unit* attacker, int32_t hit_flag); // Record PvP hit and update combat log
    void ResetUnitToHold(Unit* unit); // World callback: register newly summoned/placed unit
    void DoPatrolTick(Unit* unit); // 5A59E2 (client: 573b52)
    void AdvancePatrolStep(Unit* unit); // 5A5B49
    void ResetUnitToIdle(Unit* unit); // 5A9A8F — reset unit to idle (ASM). (client: 577bff)
    void DoGatherLootStep(Unit* unit); // 5A6789 (client: 5748f9)
    void DoTeleportHome(Unit* unit); // 5A8885 (client: 5769f5)
    void TeleportToSafety(Unit* unit); // 5A8A55 (client: 576bc5)
    uint32_t ComputeListAvgPos(UnitList* list); // 5AB62E (client: 57979e)
    void ProcessAnimateDeadGroup(Group* group); //5acdf4 // World callback for animate-dead group
    void DoScriptInstID(int32_t script_id); // Run script by instance ID

    // Trigger check handlers, called from `DispatchTriggerCheck`.
    void CheckOP_1(TriggerCheck* check); // 5B2E91. How many units contains this group (client: 581001)
    void CheckOP_2(TriggerCheck* check); // 5B2EF9. Is unit in a box (client: 581069)
    void CheckOP_3(TriggerCheck* check); // 5B2F9A. Is unit in a circle (client: 58110a)
    void CheckOP_4(TriggerCheck* check); // 5B3071. Get unit parameter (client: 5811e1)
    void CheckOP_5(TriggerCheck* check); // 5B2EB8. Is unit alive? (client: 581028)
    void CheckOP_6(TriggerCheck* check); // 5B3131. Distance between units (client: 5812a1)
    void CheckOP_7(TriggerCheck* check); // 5B3016. Distance from point to unit (client: 581186)
    void CheckOP_8(TriggerCheck* check); // 5B30AD. How many units this player have (client: 58121d)
    void CheckOP_9(TriggerCheck* check); // 5B31C9. Is unit make attack (client: 581339)
    void CheckOP_10(TriggerCheck* check); // 5B3222. Get diplomacy (client: 581392)
    void CheckOP_12(TriggerCheck* check); // 5B3273. Item in inventory, same as check 17.
    void CheckOP_14(TriggerCheck* check); // 5B32CF. Get sack (client: 58143f)
    void CheckOP_15(TriggerCheck* check); // 5B3324. Distance to nearest units of player (client: 581494)
    void CheckOP_16(TriggerCheck* check); // 5B33F0. Item to point (client: 581560)
    void CheckOP_17(TriggerCheck* check); // 5B347A. Item in inventory
    void CheckOP_18(TriggerCheck* check); // 5B34C9. VIP (client: 581639)
    void CheckOP_19(TriggerCheck* check); // 5B34F7. Return value of variable
    void CheckOP_20(TriggerCheck* check); // 5B3525. How many buildings this player have (client: 581695)
    void CheckOP_21(TriggerCheck* check); // 5B35A0. Health of building (client: 581710)
    void CheckOP_22(TriggerCheck* check); // 5B35C6. Teleportation (client: 581736)
    void CheckOP_23(TriggerCheck* check); // 5B3644. Get Scenario Variable (client: 5817b4)
    void CheckOP_24(TriggerCheck* check); // 5B366D. Get Subobjective (client: 5817dd)
    void CheckOP_25(TriggerCheck* check); // 5B369C. Spell on Tile (client: 58180c)
    void CheckOP_26(TriggerCheck* check); // 5B3720. Spell on Unit (client: 581890)
    void CheckOP_27(TriggerCheck* check); // 5B3776. Centered on Tile (client: 5818e6)

    // Trigger action dispatch handlers (called from DoScriptInstID)
    void ScriptOP_0x01(TriggerAction* action);
    void ScriptOP_0x02(TriggerAction* action);
    void ScriptOP_0x03(TriggerAction* action);
    void ScriptOP_0x04(TriggerAction* action);
    void ScriptOP_0x05(TriggerAction* action);
    void ScriptOP_0x06(TriggerAction* action);
    void ScriptOP_0x07(TriggerAction* action);
    void ScriptOP_0x08(TriggerAction* action);
    void ScriptOP_0x0a(TriggerAction* action);
    void ScriptOP_0x0b(TriggerAction* action);
    void ScriptOP_0x0c(TriggerAction* action);
    void ScriptOP_0x0d(TriggerAction* action);
    void ScriptOP_0x0e(TriggerAction* action);
    void ScriptOP_0x0f(TriggerAction* action);
    void ScriptOP_0x10(TriggerAction* action);
    void ScriptOP_0x11(TriggerAction* action);
    void ScriptOP_0x12(TriggerAction* action);
    void ScriptOP_0x13(TriggerAction* action);
    void ScriptOP_0x14(TriggerAction* action);
    void ScriptOP_0x15(TriggerAction* action);
    void ScriptOP_0x16(TriggerAction* action);
    void ScriptOP_0x17(TriggerAction* action);
    void ScriptOP_0x18(TriggerAction* action);
    void ScriptOP_0x19(TriggerAction* action);
    void ScriptOP_0x1a(TriggerAction* action);
    void ScriptOP_0x1b(TriggerAction* action);
    void ScriptOP_0x1c(TriggerAction* action);
    void ScriptOP_0x1d(TriggerAction* action);
    void ScriptOP_0x1e(TriggerAction* action);
    void ScriptOP_0x1f(TriggerAction* action);
    void ScriptOP_0x20(TriggerAction* action);
    void ScriptOP_0x21(TriggerAction* action);
    void ScriptOP_0x22(TriggerAction* action);
    void ScriptOP_0x23(TriggerAction* action);
    void ScriptOP_0x24(TriggerAction* action);
    void ScriptOP_0x26(TriggerAction* action);
    void ScriptOP_0x27(TriggerAction* action);

    void SplitTargetLists(Unit* unit, UnitList* pList); // Populate attack/non-attack target lists for unit from pList.
    void FilterByRelation(Unit* unit, UnitList* list, int32_t flag); // 5A3896 — filter a UnitList by enemy/allied relationship. (client: 571a06)
    void PruneUnconsciousTargets(); // 5A3C5F — move unconscious targets out of field26_0xa64 into field27_0xa84. (client: 571dcf)
    UnitList* GetNearbyUnits(Unit* unit); // 5A37C5 — get nearby UnitList around unit. (client: 571935)
    UnitList* GetNearbyEnemies(Unit* caster, PosYX yx); // Get nearby enemy UnitList for caster at yx.
    UnitList* GetNearbyAllies(Unit* caster, PosYX yx); // Get nearby ally UnitList for caster at yx.
    void Destroy(); // 5A4590
    uint8_t UnitMaxRange(Unit* unit);                  // Get effective max cast range for unit.
    void BuildTargetLists(Unit* unit); // 5A5E54 — build active/alternate target lists around unit. (client: 573fc4)
    void BeginPositionCast(Unit* caster, uint16_t pos_yx, Spell* spell); // 5A551C — set up a position-targeted cast action (ASM). (client: 57368c)
    void CastKnownSpell(Unit* unit, Unit* target, int32_t spell_id); // 5A6801 — pick and start casting a known spell. (client: 574971)
    void SetIdleOrRetreat(Unit* unit, int retreat);           // Set unit to idle/wander (retreat=0) or retreat (retreat!=0).
    void DoGuardScan(Unit* unit); // 5A607B — idle/wander helper. (client: 5741eb)
    void AdvancePatrol(Unit* unit); // 5A647C — helper for state 10/24 unit actions. (client: 5745ec)
    void DoRetreat(Unit* unit); // 5A6E59 — retreat helper. (client: 574fc9)
    void PursueTarget(Unit* unit, Unit* target); // 5A8709 — set unit to pursue the chosen target. (client: 576879)
    void IdleSelfBuff(Unit* unit); // 5A6EA5 — idle/wander helper (ASM). (client: 575015)
    int32_t ShouldPursueTarget(Unit* unit, Unit* target); // 5A6ED9 — target-pursuit check (ASM). (client: 575049)
    int32_t PickAttackSpell(Unit* unit, Unit* target); // 5B6632 — AI mage spell selection: pick a random affordable non-defensive spell. (client: 5847a2)
    void CastSelfHealOrBuff(Unit* unit); // 5A845B — AI mage retreat helper: cast healing/buff on self or most-damaged ally. (client: 5765cb)
    Spell* GetCastableSpell(Unit* unit, int32_t spell_number, int32_t flag); // Pick spell by number if unit has enough MP.
    int32_t ShouldAutobuff(Unit* unit, Unit* other); // Check if `unit` should autobuff `other` (diplomacy/settings check).
    void DoAutobuff(Unit* unit); // Autobuff handler: pick and cast buff spells on self or allies.
    int32_t IsSpellInRange(Unit* unit, Unit* other, Spell* spell); // Check if `other` is in range of spell from `unit`.
    void BeginAutobuffCast(Unit* caster, Unit* target, Spell* spell); // Set up autobuff cast action for caster.
    void BeginEnterBuilding(Unit* unit, Building* building); // 5A6DA3

    // Helpers for the functions above; bodies remain in Main.asm for now.
    UnitList* BuildNearbyList(Unit* unit); // 5A3791 — helper for GetNearbyUnits (ASM). (client: 571901)
    void RemoveAirborneTargets(Unit* unit, UnitList* list); // 5A3DC9 — helper for GetNearbyUnits (ASM). (client: 571f39)
    int32_t WantsRetreat(Unit* unit); // 5A89D5 — helper for ProcessUnitRecall (ASM). (client: 576b45)
    int32_t MustFlee(Unit* unit); // 5A8BDA — helper for ProcessUnitRecall (ASM). (client: 576d4a)
    void FleeFromEnemies(Unit* unit); // 5A8CD8 — helper for ProcessUnitRecall. (client: 576e48)
    void TeleportAwayFromEnemies(Unit* unit); // 5A8DBC — helper for FleeFromEnemies (ASM). (client: 576f2c)
    void OrderWalkTo(Unit* unit, uint8_t a, uint8_t b); // 5A925C — helper for DoScriptedWalk (ASM). (client: 5773cc)

    void FilterBuffTargets(Unit* unit, UnitList* list); // 5A39AD — filter UnitList for CastSelfHealOrBuff (ASM). (client: 571b1d)
    void PickFallbackBuff(Unit* unit, UnitList* list); // 5B6862 — fallback buff selection for CastSelfHealOrBuff. (client: 5849d2)
    void InitUnitCombatProfile(Unit* unit); // in asm
    void BuildAllyList(Group* group); // Build nearby friendly unit list from `group`.
    bool AreAtWar(Unit* unit1, Unit* unit2); // Check if unit1's owner is at war with unit2's owner.

    void PrepareUnitOrder(Unit* unit); //5a9832 (client: 5779a2)
    void OrderHoldPosition(Unit* unit); //5a93f4
    void ClearGroupOrders(Group* grp); //5acd4c (client: 57aebc)
};
ASSERT_OFFSET(World, duration4, 0x8b0);
ASSERT_OFFSET(World, counter, 0xa4c);
ASSERT_OFFSET(World, diplomacy, 0xa8bc);
ASSERT_OFFSET(World, mission_complete, 0xbe0c);
ASSERT_OFFSET(World, mission_state, 0xc710);
ASSERT_OFFSET(World, players_list, 0xc77c);
ASSERT_OFFSET(World, trigger_checks, 0xeaac);
ASSERT_SIZE(World, 0xeb20);

int32_t __cdecl RandInt(); // 5B6F30 — rand() wrapper.
extern World* g_World;
