#include "unit_list.h"
#include "unit.h"

//555a1d
UnitList::UnitList() = default;

//555a8d
UnitList::~UnitList() = default;

void UnitList::Serialize(CArchive& ar)
{
	//558ef8
	if (ar.IsStoring())
	{
		ar.WriteCount(unit_list.GetCount());

		POSITION pos = unit_list.GetHeadPosition();
		while (pos != NULL)
		{
			Unit *unit = unit_list.GetNext(pos);
			ar.WriteObject(unit);
		}
	}
	else
	{
		DWORD nNewCount = ar.ReadCount();
		Unit* unit;
		while (nNewCount--)
		{
			ar >> unit;
			unit_list.AddTail(unit);
		}
	}
}

void UnitList::CallMethod5()
{
	//5591da
	POSITION pos = unit_list.GetHeadPosition();
	while (pos != NULL)
	{
		Unit* unit = unit_list.GetNext(pos);
		unit->VMethod5();
	}
}

void UnitList::AddTail(Unit* unit)
{
	//555c1b
	unit_list.AddTail(unit);
}

void UnitList::AddTailAllocId(Unit* unit)
{
	//555d67
	unit_list.AddTail(unit);
	unit->building_id = g_buildingIdSet.AllocBit();
}

void UnitList::AddTailId6xxx(Unit* unit)
{
	//555f1c
	unit_list.AddTail(unit);
	unit->building_id = 0x6000 + unit->TokenID;
}