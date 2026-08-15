#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "spell.h"
#include "unit.h"

struct Action {
public:
    int32_t action_kind;
    int32_t x;
    int32_t y;
    uint8_t gap_0xc[32];
    Unit* unit;
    CObject* payload; // Can be spell or unit.
    Unit* target;
    uint8_t gap_0x38[28];

public:
    Action(); // 5b6c42
};
ASSERT_SIZE(Action, 0x54);
