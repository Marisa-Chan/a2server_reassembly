#pragma once

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


class Building;
class Player;
class Shop;
class Inn;
struct TokenPos;

class BuildingsList : public CList<Building*> {
    // No fields.
public:
    Building* FindById(uint16_t id); // 557db2 (client: 535cc5) - Look up building by ID
    Building* FindNearby(TokenPos* pos); // 558128 (client: 535e4b) - Find building at exact position (adjusts pos by -1)
    Building* FindNearbyExact(TokenPos* pos); // 557ea5 (client: 535d09) - Get nearest building at position
    Building* FindClosest(TokenPos* pos); // 557f69 (client: 535d62) - Find nearest building to position
    Building* FindPlayerInn(Player* player); // 558055 (client: 535de3) - Find inn owned by given player
    void AddBuilding(Building* building); // 558228 (client: 535ee0) - AddTailAndSetID
    void Serialize(CArchive& ar); // 558908 (client: 5360fc) - Serialize
#ifdef A2SERVER_PATCH
    Shop* FindNearestShop(TokenPos* pos); // Shop-only variant of FindNearby (see buildings_list.cpp)
    Inn* FindNearestInn(TokenPos* pos); // Inn-only variant of FindNearby (see buildings_list.cpp)
#endif

    void RegenTick(); // 55817b (client: 535e9e)
};
ASSERT_SIZE(BuildingsList, 0x1c);
