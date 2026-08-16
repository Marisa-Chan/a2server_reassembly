#pragma once

#include <cstdint>

#include "action.h"
#include "asm_mfc.h"
#include "assert_offset.h"
#include "logic.h"
#include "map_stuff.h"
#include "mfc_templ.h"
#include "perf.h"
#include "unit_list.h"
#include "token.h"


struct MapStuff;
struct Group;
class Player;
class PlayersList;
struct ResFile;
class Building;
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
    Diplomacy(); // 5B5855
    void sub_5B5643(Unit* attacker, Unit* target, int32_t hit_flag);
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
    uint8_t gap_0xa54[16];
    UnitList field26_0xa64;
    UnitList field27_0xa84;
    UnitList field28_0xaa4; // Non-attack list?
    UnitList field29_0xac4; // Attack list?
    uint32_t field_0xae4[10000];
    uint8_t field_0xa724;
    uint8_t gap_0xa725[3];
    uint8_t field32_0xa728[400];
    int32_t MinimalGuardRange;
    Diplomacy diplomacy;
    int32_t field37_0xbbe8;
    CList<Action> action_list;
    uint8_t field39_0xbc08[400];
    Action scrape_action;
    uint8_t field_0xbdec[16];
    uint8_t field_0xbdfc;
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
    World(MapStuff* map_stuff, PlayersList* players_list); // 5A4B1A
    ~World();

    void sub_5A457A(); // 5A457A
    void sub_5A44C5(); // 5A44C5
    void sub_5A44DB(); // 5A44DB
    void sub_5A3F3A(); // 5A3F3A
    void sub_5A3F88(); // 5A3F88
    void sub_5A3F9B(); // 5A3F9B
    void sub_5A4284(); // 5A4284
    void sub_5A4757(); // 5A4757
    void sub_5B0556(CArchive& ar); // Serialize
    void sub_5B2E7A(); // Post-load world relink helper

    void sub_5AFBFD(); // increments field65_0xc780
    void sub_5ABD16(class PlayersList* players);
    void sub_5B0724(); // 5B0724
    void sub_5B0762(); // 5B0762
    void sub_5B081E(TriggerCheck* check); // 5B081E — trigger-check dispatcher.
    void sub_5B0A6A(); // 5B0A6A
    Unit* sub_5B61D0(uint16_t pos_yx, uint8_t byte_0x3c, Player* owner, int32_t is_defensive); // 5B61D0
    void sub_5B6346(); // 5B6346
    void sub_5ABF50(Player* player); // 5ABF50

    // Group-order dispatch helpers (called from Server::sub_504a96)
    void sub_5AC137(Group* group); // Refresh group_sub position/range fields.
    void sub_5ACAA7(Group* group);
    void sub_5A99C7(Unit* unit);
    void sub_5ACA54(Group* group); //5aca54
    void sub_5AC80F(Group* group, Unit* target);
    void sub_5ACB4D(Group* group, Unit* target, uint8_t param_3);
    void sub_5AC289(Group* group, uint8_t x, uint8_t y); // Group move command.
    void sub_5AC507(Group* group, uint8_t x, uint8_t y); // Group swarm command.
    void sub_5ACBEF(Group* group, uint8_t x, uint8_t y);
    void sub_5AC187(Group* group, Unit* target, Spell*);
    void sub_5AC206(Group* group, uint8_t x, uint8_t y, Spell*);
    void sub_5A9961(Unit* unit, uint8_t x, uint8_t y);
    void sub_5A9AC4(Unit* unit); //5a9ac4
    void sub_5A8EFB(Unit* unit, uint8_t x, uint8_t y); // Set unit movement target, clamped to map bounds.
    void sub_5A90F4(Unit* unit, Building* bldg);
    void sub_5AC881(Group* group, uint8_t x, uint8_t y);
    void sub_5AC8A2(Group* group, uint8_t range); // Group guard command, range=0 means use world's MinimalGuardRange.
    void sub_5ACAFA(Group* group);
    void sub_5AC983(Group* group, int param); // Place a newly respawned group into the world.
    void sub_5A930F(Unit* unit, uint8_t x, uint8_t y, Spell* spell); // 5A930F — group area-cast setup (ASM).
    void sub_5AC785(Group* group, uint8_t x, uint8_t y);
    void sub_5ACB9E(Group* group, Unit* target, uint8_t param_3);
    uint32_t sub_5AB719(Group* group); // Compute average position of units in a group.
    uint8_t sub_5ABB32(Unit* unit, PosYX yx); // 5ABB32
    void sub_5AC7C8(Group* group);
    void sub_5A9383(Unit* unit);
    void sub_5A943B(Unit* unit); // 5A943B — helper for sub_5ACAA7.
    void sub_5A9918(Unit* unit); // 5A9918 — helper for sub_5ACAFA.
    void sub_5AF805(int32_t a, int32_t b, int32_t c, Player* player);
    void sub_5AFA01(int32_t a, int32_t b, int32_t c, Player* player);
    void sub_5AF683(Player* player, uint8_t param);
    void sub_5A9B6B(Unit* unit); // AI tick for a monster unit? 5A9B6B
    void sub_5AA375(Unit* unit, PosYX yx, uint8_t max_range); // 5AA375
    void sub_5AA426(Unit* unit, uint8_t facing); // 5AA426
    void sub_5AA485(Unit* unit); // 5AA485
    void sub_5AA78C(Unit* unit); // 5AA78C
    void sub_5AA8ED(Unit* unit); // 5AA8ED
    void sub_5AA91B(Unit* unit, Unit* target);
    void sub_5AA97E(Unit* unit); // 5AA97E
    void sub_5AA9F6(Unit* unit); // 5AA9F6
    void sub_5AAA89(Unit* unit);
    void sub_5AAB08(Unit* unit); // 5AAB08
    void sub_5AAB85(Unit* unit, Unit* target); // 5AAB85
    void sub_5A9087(Unit* unit, Unit* target); // 5A9087
    void sub_5A92AF(Unit* unit, Unit* target, Spell* spell); // 5A92AF
    void sub_5A9482(Unit* unit, Unit* target, uint8_t range); // 5A9482
    void sub_5A9501(Unit* unit, Unit* target, uint8_t range); // 5A9501
    void sub_5A9580(Unit* unit, uint8_t x, uint8_t y); // 5A9580
    void sub_5AA84F(Unit* unit); // 5AA84F
    int32_t sub_5AD0B3(Unit* unit, Unit* target); // 5AD0B3
    void sub_5ABFBC(Group* group); // 5ABFBC
    void sub_5AF623(Group* group); // 5AF623
    void sub_5AF646(Group* group); // 5AF646
    void sub_5AF5DC(Unit* unit); // 5AF5DC — helper for sub_5AF646.
    void sub_5AF022(Group* group); // 5AF022
    void sub_5ABCFD(Unit* unit); // 5ABCFD
    void sub_5A5155(Unit* unit); // 5A5155 — helper for sub_5ABCFD.
    void sub_5A5188(Unit* unit); // 5A5188 — AI wrapper for sub_5A51A1.
    void sub_5A51A1(Unit* unit); // 5A51A1 — per-state unit action dispatcher.
    void sub_5AE628(Group* group); // 5AE628
    void sub_5AEC20(Group* group, uint16_t param); // 5AEC20
    void sub_5AEF66(Group* group); // 5AEF66
    void sub_5AF061(Group* group, uint16_t param); // 5AF061
    void sub_5AECE7(Group* group, uint16_t param); // 5AECE7
    void sub_5AED8D(Group* group); // 5AED8D
    void sub_5AE7C3(Group* group); // 5AE7C3
    void sub_5AEFE5(Group* group); // 5AEFE5

    void sub_5AB35B(Unit* unit); // 5AB35B
    void sub_5AB5E3(Unit* unit); // 5AB5E3
    void sub_5AB92C(Group* group); // 5AB92C
    void sub_5ADB16(Group* group); // 5ADB16
    void sub_5A3D12(uint16_t param1, uint8_t param2); // 5A3D12
    void sub_5AE2D4(Group* group); // 5AE2D4
    void sub_5AE5C0(Group* group); // 5AE5C0
    void sub_5AE5F5(Group* group); // 5AE5F5
    void sub_5A8778(Unit* unit, uint16_t param); // 5A8778
    void sub_5A87A3(Unit* unit, uint16_t param); // 5A87A3
    void sub_5A8829(Unit* unit, Unit* target, uint8_t range); // 5A8829
    void sub_5A6B48(Unit* unit, Unit* target); // 5A6B48
    void sub_5A5568(Unit* unit, Unit* target); // 5A5568
    void sub_5A56D7(Unit* unit, Unit* target); // 5A56D7
    void sub_5A57E8(Unit* unit, Unit* target); // 5A57E8
    void sub_5A65BB(Unit* unit, Unit* target); // 5A65BB
    void sub_5A54B8(Unit* unit, uint16_t command_to); // 5A54B8

    void sub_5AAEBC(Unit* unit); // 5AAEBC
    void sub_5AAF84(Unit* unit, uint8_t facing); // 5AAF84 — turn unit to face target (ASM).
    void sub_5AAC17(Unit* unit); // 5AAC17
    void sub_5AA7CF(Unit* unit); // 5AA7CF
    int32_t sub_5B6F40(int32_t multiplier); // 5B6F40
    int32_t sub_5B6F60(int32_t min, int32_t max); // 5B6F60
    void sub_5AA581(Unit* target, Unit* attacker, int32_t param_4); // Record PvP hit and update combat log
    void sub_5A9A6A(Unit* unit); // World callback: register newly summoned/placed unit
    void sub_5A59E2(Unit* unit); // 5A59E2
    void sub_5A5B49(Unit* unit); // 5A5B49
    void sub_5A9A8F(Unit* unit); // 5A9A8F — reset unit to idle (ASM).
    void sub_5A6789(Unit* unit); // 5A6789
    void sub_5A8885(Unit* unit); // 5A8885
    void sub_5A8A55(Unit* unit); // 5A8A55
    uint32_t sub_5AB62E(UnitList* list); // 5AB62E
    void sub_5ACDF4(Group* group); //5acdf4 // World callback for animate-dead group
    void DoScriptInstID(int32_t script_id); // Run script by instance ID

    // Trigger check handlers, called from `sub_5B081E`.
    void CheckOP_1(TriggerCheck* check); // 5B2E91. How many units contains this group
    void CheckOP_2(TriggerCheck* check); // 5B2EF9. Is unit in a box
    void CheckOP_3(TriggerCheck* check); // 5B2F9A. Is unit in a circle
    void CheckOP_4(TriggerCheck* check); // 5B3071. Get unit parameter
    void CheckOP_5(TriggerCheck* check); // 5B2EB8. Is unit alive?
    void CheckOP_6(TriggerCheck* check); // 5B3131. Distance between units
    void CheckOP_7(TriggerCheck* check); // 5B3016. Distance from point to unit
    void CheckOP_8(TriggerCheck* check); // 5B30AD. How many units this player have
    void CheckOP_9(TriggerCheck* check); // 5B31C9. Is unit make attack
    void CheckOP_10(TriggerCheck* check); // 5B3222. Get diplomacy
    void CheckOP_12(TriggerCheck* check); // 5B3273. Item in inventory, same as check 17.
    void CheckOP_14(TriggerCheck* check); // 5B32CF. Get sack
    void CheckOP_15(TriggerCheck* check); // 5B3324. Distance to nearest units of player
    void CheckOP_16(TriggerCheck* check); // 5B33F0. Item to point
    void CheckOP_17(TriggerCheck* check); // 5B347A. Item in inventory
    void CheckOP_18(TriggerCheck* check); // 5B34C9. VIP
    void CheckOP_19(TriggerCheck* check); // 5B34F7. Return value of variable
    void CheckOP_20(TriggerCheck* check); // 5B3525. How many buildings this player have
    void CheckOP_21(TriggerCheck* check); // 5B35A0. Health of building
    void CheckOP_22(TriggerCheck* check); // 5B35C6. Teleportation
    void CheckOP_23(TriggerCheck* check); // 5B3644. Get Scenario Variable
    void CheckOP_24(TriggerCheck* check); // 5B366D. Get Subobjective
    void CheckOP_25(TriggerCheck* check); // 5B369C. Spell on Tile
    void CheckOP_26(TriggerCheck* check); // 5B3720. Spell on Unit
    void CheckOP_27(TriggerCheck* check); // 5B3776. Centered on Tile

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

    void sub_5A3AD6(Unit* unit, UnitList* pList); // Populate attack/non-attack target lists for unit from pList.
    void sub_5A3896(Unit* unit, UnitList* list, int32_t flag); // 5A3896 — filter a UnitList by enemy/allied relationship.
    void sub_5A3C5F(); // 5A3C5F — move unconscious targets out of field26_0xa64 into field27_0xa84.
    UnitList* sub_5A37C5(Unit* unit); // 5A37C5 — get nearby UnitList around unit.
    UnitList* sub_5A3808(Unit* caster, PosYX yx); // Get nearby enemy UnitList for caster at yx.
    UnitList* sub_5A384F(Unit* caster, PosYX yx); // Get nearby ally UnitList for caster at yx.
    void sub_5A4590(); // 5A4590
    uint8_t UnitMaxRange(Unit* unit);                  // Get effective max cast range for unit.
    void sub_5A5E54(Unit* unit); // 5A5E54 — build active/alternate target lists around unit.
    void sub_5A551C(Unit* caster, uint16_t pos_yx, Spell* spell); // 5A551C — set up a position-targeted cast action (ASM).
    void sub_5A6801(Unit* unit, Unit* target, int32_t spell_id); // 5A6801 — pick and start casting a known spell.
    void sub_5A6E2C(Unit* unit, int param);           // Set unit to idle/wander (param=0) or retreat (param!=0).
    void sub_5A607B(Unit* unit); // 5A607B — idle/wander helper.
    void sub_5A647C(Unit* unit); // 5A647C — helper for state 10/24 unit actions.
    void sub_5A6E59(Unit* unit); // 5A6E59 — retreat helper.
    void sub_5A8709(Unit* unit, Unit* target); // 5A8709 — set unit to pursue the chosen target.
    void sub_5A6EA5(Unit* unit); // 5A6EA5 — idle/wander helper (ASM).
    int32_t sub_5A6ED9(Unit* unit, Unit* target); // 5A6ED9 — target-pursuit check (ASM).
    int32_t sub_5B6632(Unit* unit, Unit* target); // 5B6632 — AI mage spell selection: pick a random affordable non-defensive spell.
    void sub_5A845B(Unit* unit); // 5A845B — AI mage retreat helper: cast healing/buff on self or most-damaged ally.
    Spell* sub_5A79D6(Unit* unit, int32_t spell_number, int32_t flag); // Pick spell by number if unit has enough MP.
    int32_t sub_5A7A1C(Unit* unit, Unit* other); // Check if `unit` should autobuff `other` (diplomacy/settings check).
    void sub_5A7B44(Unit* unit); // Autobuff handler: pick and cast buff spells on self or allies.
    int32_t sub_5A7AF7(Unit* unit, Unit* other, Spell* spell); // Check if `other` is in range of spell from `unit`.
    void sub_5A85F4(Unit* caster, Unit* target, Spell* spell); // Set up autobuff cast action for caster.
    void sub_5A6DA3(Unit* unit, Building* building); // 5A6DA3

    // Helpers for the functions above; bodies remain in Main.asm for now.
    UnitList* sub_5A3791(Unit* unit); // 5A3791 — helper for sub_5A37C5 (ASM).
    void sub_5A3DC9(Unit* unit, UnitList* list); // 5A3DC9 — helper for sub_5A37C5 (ASM).
    int32_t sub_5A89D5(Unit* unit); // 5A89D5 — helper for sub_5AF5DC (ASM).
    int32_t sub_5A8BDA(Unit* unit); // 5A8BDA — helper for sub_5AF5DC (ASM).
    void sub_5A8CD8(Unit* unit); // 5A8CD8 — helper for sub_5AF5DC (ASM).
    void sub_5A925C(Unit* unit, uint8_t a, uint8_t b); // 5A925C — helper for sub_5AB5E3 (ASM).

    void sub_5A39AD(Unit* unit, UnitList* list); // 5A39AD — filter UnitList for sub_5A845B (ASM).
    void sub_5B6862(Unit* unit, UnitList* list); // 5B6862 — fallback buff selection for sub_5A845B.
    void sub_5AF6F5(Unit* unit); // in asm
    void sub_5ADD64(Group* group); // Build nearby friendly unit list from `group`.
    bool sub_5B5816(Unit* unit1, Unit* unit2); // Check if unit1's owner is at war with unit2's owner.

    void FUN_005a9832(Unit* unit); //5a9832
    void FUN_005a93f4(Unit* unit); //5a93f4
    void FUN_005acd4c(Group* grp); //5acd4c
};
ASSERT_OFFSET(World, duration4, 0x8b0);
ASSERT_OFFSET(World, counter, 0xa4c);
ASSERT_OFFSET(World, diplomacy, 0xa8bc);
ASSERT_OFFSET(World, mission_complete, 0xbe0c);
ASSERT_OFFSET(World, mission_state, 0xc710);
ASSERT_OFFSET(World, players_list, 0xc77c);
ASSERT_OFFSET(World, trigger_checks, 0xeaac);
ASSERT_SIZE(World, 0xeb20);

extern "C" int32_t __cdecl sub_5B6F30(); // 5B6F30 — rand() wrapper.
extern "C" World* g_World;
