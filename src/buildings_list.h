#pragma once

#include <cstdint>
#include "asm_mfc.h"

#include "assert_offset.h"
#include "mfc_templ.h"


class Building;
struct TokenPos;
class Player;

class BuildingsList : public CList<Building*> {
    // No fields.
public:
    Building* sub_557DB2(uint16_t id); // Look up building by ID
    Building* sub_558128(TokenPos* pos); // Find building at exact position (adjusts pos by -1)
    Building* sub_557EA5(TokenPos* pos); // Get nearest building at position
    Building* sub_557F69(TokenPos* pos); // Find nearest building to position
    Building* sub_558055(Player* player); // Find inn owned by given player
    void sub_558228(Building* building); // AddTailAndSetID

    void FUN_0055817b(); //55817b
};
ASSERT_SIZE(BuildingsList, 0x1c);
