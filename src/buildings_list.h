#pragma once

#include <cstdint>
#include "asm_mfc.h"

#include "assert_offset.h"
#include "mfc_templ.h"


class Building;


class BuildingsList : public CList<Building*> {
    // No fields.
public:
    Building* sub_557DB2(uint16_t id); // Look up building by ID
};
ASSERT_SIZE(BuildingsList, 0x1c);
