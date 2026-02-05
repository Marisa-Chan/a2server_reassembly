#pragma once

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "perf.h"
#include "unit_list.h"


struct MapStuff;
class PlayersList;
struct ResFile;
class Spell;
class Trigger;
class TriggerAction;
class TriggerCheck;
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
    UnitList field28_0xaa4;
    UnitList field29_0xac4;
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
    int32_t references[1000]; // Array of User*, Group* and Player* objects
    uint8_t trigger_results[1000];
    CList<TriggerCheck*>* trigger_checks;
    CArray<TriggerAction*>* trigger_actions;
    CList<Trigger*>* triggers;
    char name[100]; // Not sure it's 100 bytes
    ResFile* field74_0xeb1c;
};
ASSERT_OFFSET(World, duration4, 0x8b0);
ASSERT_OFFSET(World, counter, 0xa4c);
ASSERT_OFFSET(World, diplomacy, 0xa8c4);
ASSERT_OFFSET(World, mission_complete, 0xbe0c);
ASSERT_OFFSET(World, players_list, 0xc77c);
ASSERT_SIZE(World, 0xeb20);
