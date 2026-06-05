#ifndef LOGIC_H
#define LOGIC_H

#include <array>
#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


struct Check {
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
    std::array<uint8_t, 10> field_0x30;
    std::array<uint8_t, 10> field_0x3a;
    std::array<uint8_t, 10> field_0x44;
    std::array<uint8_t, 10> field_0x4e;
    uint8_t field_0x58;
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
    TriggerAction();
};
ASSERT_OFFSET(TriggerAction, field_0x30, 0x30);
ASSERT_OFFSET(TriggerAction, field_0x3a, 0x3a);
ASSERT_OFFSET(TriggerAction, field_0x44, 0x44);
ASSERT_OFFSET(TriggerAction, field_0x4e, 0x4e);
ASSERT_OFFSET(TriggerAction, unit, 0x5c);
ASSERT_SIZE(TriggerAction, 0x74);

struct TriggerCheck: public TriggerAction {
public:
    uint32_t exec_once;
    uint32_t counter;

public:
    // 5afc1d
    TriggerCheck();
};
ASSERT_OFFSET(TriggerCheck, counter, 0x78);
ASSERT_SIZE(TriggerCheck, 0x7c);

struct Trigger {
public:
    CList<Check*>* checks;
    CList<TriggerAction*>* actions;
    uint32_t once;
    uint8_t gap_0xc[4];
    uint32_t field_0x10;
    uint32_t trigger_id;
};
ASSERT_SIZE(Trigger, 0x18);

struct ScriptSettings {
    uint32_t turn_tracing;
    uint32_t script_tracing;
    uint8_t gap_0x8[392];

    ScriptSettings() { memset(this, 0, sizeof(ScriptSettings)); } //5b67ee
};
ASSERT_SIZE(ScriptSettings, 0x190);


#endif
