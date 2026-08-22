#include "buildings_list.h"
#include "building.h"
#include "inn.h"


void BuildingsList::FUN_0055817b()
{ // 55817b
	for (POSITION pos = GetHeadPosition(); pos != nullptr;)
	{
		Building* bld = GetNext(pos);
		bld->VMethod1();
	}
}

// 558055
Building* BuildingsList::sub_558055(Player* player) {
	for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
		Building* building = this->GetNext(it);
		if (building->object_info_id != 0 && building->pOwner == player && building->IsKindOf(RUNTIME_CLASS(Inn))) {
			return building;
		}
	}
	return nullptr;
}

// 557DB2
Building* BuildingsList::sub_557DB2(uint16_t id) {
	for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
		Building* building = this->GetNext(it);
		if (building->building_id == id) {
			return building;
		}
	}
	return nullptr;
}

// 557EA5
Building* BuildingsList::sub_557EA5(TokenPos* pos) {
	for (POSITION it = this->GetHeadPosition(); it != nullptr;) {
		Building* building = this->GetNext(it);
		if (building->position->Distance(pos) <= 2) {
			return building;
		}
	}
	return nullptr;
}

// 557F69
Building* BuildingsList::sub_557F69(TokenPos* pos) {
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
void BuildingsList::sub_558228(Building* building) {
	this->AddTail(building);
	building->building_id = building->TokenID + 0x6000;
}

// 558908
void BuildingsList::sub_558908(CArchive& ar) {
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
