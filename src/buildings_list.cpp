#include "buildings_list.h"
#include "building.h"
#include "inn.h"
#ifdef A2SERVER_PATCH
#include "shop.h"
#endif


void BuildingsList::RegenTick() { // 55817b
    for (POSITION pos = GetHeadPosition(); pos != nullptr;) {
        Building* bld = GetNext(pos);
        bld->RegenTick();
    }
}

// 558055
Building* BuildingsList::FindPlayerInn(Player* player) {
    for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
        Building* building = this->GetNext(it);
        if (building->object_info_id != 0 && building->pOwner == player && building->IsKindOf(RUNTIME_CLASS(Inn))) {
            return building;
        }
    }
    return nullptr;
}

// 558128
Building* BuildingsList::FindNearby(TokenPos* pos) {
    TokenPos adjusted(pos->GetX() - 1, pos->GetY() - 1, MapStuff_Instance);
    return this->FindNearbyExact(&adjusted);
}

#ifdef A2SERVER_PATCH
// A2SERVER_PATCH: Shop-only variant of FindNearby to prevent non-shop buildings from hijacking the lookup.
Shop* BuildingsList::FindNearestShop(TokenPos* pos) {
    TokenPos adjusted(pos->GetX() - 1, pos->GetY() - 1, MapStuff_Instance);
    Shop* best_shop = nullptr;
    uint8_t best_dist = 0xFF;
    for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
        Building* building = this->GetNext(it);
        if (!building->IsKindOf(RUNTIME_CLASS(Shop))) {
            continue;
        }
        uint8_t dist = building->position->Distance(&adjusted);
        if (dist < best_dist) {
            best_dist = dist;
            best_shop = static_cast<Shop*>(building);
        }
    }
    if (best_shop != nullptr && best_dist > 2) {
        return nullptr;
    }
    return best_shop;
}
#endif

// 557DB2
Building* BuildingsList::FindById(uint16_t id) {
    for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
        Building* building = this->GetNext(it);
        if (building->building_id == id) {
            return building;
        }
    }
    return nullptr;
}

// 557EA5
Building* BuildingsList::FindNearbyExact(TokenPos* pos) {
    for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
        Building* building = this->GetNext(it);
        if (building->position->Distance(pos) <= 2) {
            return building;
        }
    }
    return nullptr;
}

// 557F69
Building* BuildingsList::FindClosest(TokenPos* pos) {
    int32_t best_distance = 0x7FFF;
    Building* best_building = nullptr;
    for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
        Building* building = this->GetNext(it);
        if (building->object_info_id != 0) {
            uint8_t distance = building->position->Distance(pos);
            if (distance <= best_distance) {
                best_distance = distance;
                best_building = building;
            }
        }
    }
    return best_building;
}

// 558228
void BuildingsList::AddBuilding(Building* building) {
    this->AddTail(building);
    building->building_id = building->TokenID + 0x6000;
}

// 558908
void BuildingsList::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->GetCount();
        for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
            Building* building = this->GetNext(it);
            ar.WriteObject(building);
        }
    } else {
        this->RemoveAll();
        int32_t count;
        ar >> count;
        for (int32_t i = 0; i < count; i++) {
            Building* building = nullptr;
            ar >> building;
            this->AddTail(building);
        }
    }
}
