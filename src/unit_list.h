#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>

class Unit;
class Player;

class UnitList {
public:
    virtual void Serialize(CArchive& ar);
    virtual void CallMethod5();

    UnitList();
    ~UnitList();


    void AddTail(Unit* unit);
    void AddTailAllocId(Unit* unit);
    void AddTailId6xxx(Unit* unit);

    void Remove(Unit* unit); //575aa0   Remove unit from list and in any do delete for this unit

    void ClearSentToMask(Player* player); // Clear unit vision mask bits for player across this list
    void RegenTick();
    Unit* FindById(uint16_t id);   // Look up unit in list by ID
    void ProcessTick();              // Tick alive units and reap dead ones (sub_556345)
    int CountKillableMonsters();                // Check if player has a rewarded unit
    void SendVisibilityUpdates(); // 5561A1

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
