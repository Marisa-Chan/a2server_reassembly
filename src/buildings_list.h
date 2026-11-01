#pragma once

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


class Building;
struct TokenPos;
class Player;

class BuildingsList : public CList<Building*> {
    // No fields.
public:
    Building* FindById(uint16_t id); // Look up building by ID (client: 535cc5)
    Building* FindNearby(TokenPos* pos); // Find building at exact position (adjusts pos by -1) (client: 535e4b)
    Building* FindNearbyExact(TokenPos* pos); // Get nearest building at position (client: 535d09)
    Building* FindClosest(TokenPos* pos); // Find nearest building to position (client: 535d62)
    Building* FindPlayerInn(Player* player); // Find inn owned by given player (client: 535de3)
    void AddBuilding(Building* building); // AddTailAndSetID (client: 535ee0)
    void Serialize(CArchive& ar); // Serialize (client: 5360fc)

    void RegenTick(); //55817b (client: 535e9e)
};
ASSERT_SIZE(BuildingsList, 0x1c);
