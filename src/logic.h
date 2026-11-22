#ifndef LOGIC_H
#define LOGIC_H

#include <array>
#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


struct Check {
public:
    enum CompareOp : uint32_t {
        CMP_EQ = 0,
        CMP_NE = 1,
        CMP_GT = 2,
        CMP_LT = 3,
        CMP_GE = 4,
        CMP_LE = 5,
    };

    uint32_t arg1;
    uint32_t arg2;
    uint32_t compare;
};

class Building;
class Group;
class Player;
class Unit;

struct TriggerAction {
    uint32_t id;
    uint32_t type;
    std::array<int32_t, 10> data;
    std::array<uint8_t, 10> ref_kind;
    std::array<uint8_t, 10> ref_scope;
    std::array<uint8_t, 10> ref_page;
    std::array<uint8_t, 10> ref_index;
    uint8_t ref_count;
    uint8_t gap_0x59[3];
    Unit* unit;
    Group* group;
    Player* player;
    void* multi; // Can be a Unit*, Group* or Player* depending on the trigger.
    uint16_t item_id;
    uint8_t gap_0x6e[2];
    Building* building;

public:
    // 5afc9c
    TriggerAction(); // 5afc9c (client: 57de0c)
};
ASSERT_OFFSET(TriggerAction, ref_kind, 0x30);
ASSERT_OFFSET(TriggerAction, ref_scope, 0x3a);
ASSERT_OFFSET(TriggerAction, ref_page, 0x44);
ASSERT_OFFSET(TriggerAction, ref_index, 0x4e);
ASSERT_OFFSET(TriggerAction, unit, 0x5c);
ASSERT_SIZE(TriggerAction, 0x74);

struct TriggerCheck: public TriggerAction {
public:
    uint32_t exec_once;
    uint32_t counter;

public:
    // 5afc1d
    TriggerCheck(); // 5afc1d (client: 57dd8d)
    TriggerCheck& operator=(const TriggerCheck& other); // 5afc6f (client: 57dddf)
};
ASSERT_OFFSET(TriggerCheck, counter, 0x78);
ASSERT_SIZE(TriggerCheck, 0x7c);

struct Trigger {
public:
    CList<Check>* checks;
    CList<int32_t>* actions;
    uint32_t once;
    uint8_t gap_0xc[4];
    uint32_t gap_0x10;
    uint32_t trigger_id;

public:
    Trigger(); // 5afd79 (client: 57dee9)
    ~Trigger(); // 5afe3d (client: 57dfad)
    Trigger& operator=(const Trigger& other); // 5b006e (client: 57e1de)
};
ASSERT_SIZE(Trigger, 0x18);

struct ScriptSettings {
    uint32_t turn_tracing;
    uint32_t script_tracing;
    uint8_t gap_0x8[392];

    ScriptSettings() { memset(this, 0, sizeof(ScriptSettings)); } // 5b67ee (client: 58495e)
    void Serialize(CArchive& ar); // 5b681a (client: 58498a) - Serialize
};
ASSERT_SIZE(ScriptSettings, 0x190);


#endif
