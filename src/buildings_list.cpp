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