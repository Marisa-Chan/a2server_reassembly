#pragma once

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "logic.h"
#include "mfc_templ.h"
#include "perf.h"
#include "unit_list.h"


struct MapStuff;
struct Group;
class Player;
class PlayersList;
struct ResFile;
class Building;
class Spell;
class Token;
class Unit;


struct World {
    int32_t field0_0x0;
    uint8_t gap_0x4[4];
    Perf field2_0x8;
    Perf field3_0x38;
    Perf field4_0x68;
    Perf field5_0x98;
    uint8_t gap_0xc8[2024];
    int32_t duration4;
    int32_t duration4_low;
    uint8_t gap_0x8b8[4];
    int32_t duration5;
    int32_t total5;
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
    int32_t counter;
    MapStuff *field24_0xa50;
    uint8_t gap_0xa54[16];
    UnitList field26_0xa64;
    UnitList field27_0xa84;
    UnitList field28_0xaa4; // Non-attack list?
    UnitList field29_0xac4; // Attack list?
    uint32_t field_0xae4[10000];
    uint8_t gap_0xa724[4];
    uint8_t field32_0xa728[400];
    int32_t MinimalGuardRange;
    uint8_t gap_0xa8bc[4];
    World *self;
    uint8_t diplomacy[70][70];
    int32_t field37_0xbbe8;
    CList<int32_t> field38_0xbbec;
    uint8_t field39_0xbc08[400];
    uint32_t field40_0xbd98;
    uint32_t field41_0xbd9c;
    uint32_t field42_0xbda0;
    uint8_t gap_0xbda4[32];
    Unit *field44_0xbdc4;
    Spell *field45_0xbdc8;
    Unit *field46_0xbdcc;
    uint8_t gap_0xbdd0[48];
    int32_t field48_0xbe00;
    int32_t field49_0xbe04;
    int32_t field50_0xbe08;
    int32_t mission_complete;
    int32_t field52_0xbe10;
    int32_t mission_fail;
    int32_t field54_0xbe18;
    uint8_t gap_0xbe1c[1808];
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
    CList<TriggerCheck*>* trigger_checks;
    CArray<TriggerAction>* trigger_actions;
    CList<Trigger*>* triggers;
    char name[100]; // Not sure it's 100 bytes
    ResFile* field74_0xeb1c;

public:
    void sub_5AFBFD(); // increments field65_0xc780

    // Group-order dispatch helpers (called from Server::sub_504a96)
    void sub_5ACAA7(Group* group);
    void sub_5A99C7(Unit* unit);
    void sub_5ACA54(Group* group);
    void sub_5AC80F(Group* group, Token* target);
    void sub_5ACB4D(Group* group, Token* target, uint8_t param_3);
    void sub_5AC289(Group* group, uint8_t x, uint8_t y);
    void sub_5ACBEF(Group* group, uint8_t x, uint8_t y);
    void sub_5AC187(Group* group, Token* target, Spell*);
    void sub_5AC206(Group* group, uint8_t x, uint8_t y, Spell*);
    void sub_5A9961(Unit* unit, uint8_t x, uint8_t y);
    void sub_5A90F4(Unit* unit, Building* bldg);
    void sub_5AC881(Group* group, uint8_t x, uint8_t y);
    void sub_5AC8A2(Group* group, uint8_t param_2);
    void sub_5ACAFA(Group* group);
    void sub_5AC983(Group* group, int param); // Place a newly respawned group into the world.
    void sub_5AC785(Group* group, uint8_t x, uint8_t y);
    void sub_5AC507(Group* group, uint8_t x, uint8_t y);
    void sub_5ACB9E(Group* group, Token* target, uint8_t param_3);
    void sub_5AC7C8(Group* group);
    void sub_5A9383(Unit* unit);
    void sub_5AF805(int32_t a, int32_t b, int32_t c, Player* player);
    void sub_5AFA01(int32_t a, int32_t b, int32_t c, Player* player);
    void sub_5AF683(Player* player, int32_t param);
    void sub_5A79C9(Unit* unit);
    void sub_5A9B6B(Unit* unit); // AI tick for a monster unit?
    void sub_5AA581(Unit* attacker, Unit* defender, int32_t unk); // Record PvP hit and update combat log
    void sub_5A9A6A(Unit* unit); // World callback: register newly summoned/placed unit
    void sub_5ACDF4(Group* group); // World callback for animate-dead group
    void DoScriptInstID(int32_t script_id); // Run script by instance ID

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
    UnitList* sub_5A3808(Unit* caster, uint16_t yx); // Get nearby enemy UnitList for caster at yx.
    UnitList* sub_5A384F(Unit* caster, uint16_t yx); // Get nearby ally UnitList for caster at yx.
    uint8_t sub_5A6ADB(Unit* unit);                  // Get effective max cast range for unit.
    void sub_5A6E2C(Unit* unit, int param);           // Set unit to idle/wander (param=0) or retreat (param!=0).
    void sub_5A85F4(Unit* caster, Unit* target, Spell* spell); // Set up autobuff cast action for caster.
};
ASSERT_OFFSET(World, duration4, 0x8b0);
ASSERT_OFFSET(World, counter, 0xa4c);
ASSERT_OFFSET(World, diplomacy, 0xa8c4);
ASSERT_OFFSET(World, mission_complete, 0xbe0c);
ASSERT_OFFSET(World, players_list, 0xc77c);
ASSERT_SIZE(World, 0xeb20);

extern "C" World* g_World;
