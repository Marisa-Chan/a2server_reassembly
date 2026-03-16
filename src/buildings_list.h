#pragma once

#include <cstdint>
#include "asm_mfc.h"

#include "assert_offset.h"
#include "mfc_templ.h"


class Building;
struct TokenPos;


class BuildingsList : public CList<Building*> {
    // No fields.
public:
    Building* sub_557DB2(uint16_t id); // Look up building by ID
    Building* sub_558128(TokenPos* pos); // Find building at exact position (adjusts pos by -1)
    Building* sub_557EA5(TokenPos* pos); // Get nearest building at position
};
ASSERT_SIZE(BuildingsList, 0x1c);
