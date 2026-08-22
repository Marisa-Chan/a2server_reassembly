#include "buildings_list.h"
#include "building.h"


void BuildingsList::FUN_0055817b()
{ // 55817b
	for (POSITION pos = GetHeadPosition(); pos != nullptr;)
	{
		Building* bld = GetNext(pos);
		bld->VMethod1();
	}
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
