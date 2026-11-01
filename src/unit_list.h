#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>

class Unit;
class Player;

class UnitList {
public:
    virtual void Serialize(CArchive& ar);  // client: 536222
    virtual void CallMethod5();

    UnitList();  // client: 534ebc
    ~UnitList();  // client: 534ede


    void AddTail(Unit* unit);  // client: 534f4c
    void AddTailAllocId(Unit* unit);  // client: 534f68
    void AddTailId6xxx(Unit* unit);

    void Remove(Unit* unit); //575aa0   Remove unit from list and in any do delete for this unit

    void ClearSentToMask(Player* player); // Clear unit vision mask bits for player across this list (client: 535b23)
    void RegenTick();  // client: 5358d9
    Unit* FindById(uint16_t id);   // Look up unit in list by ID (client: 534fdf)
    void ProcessTick();              // Tick alive units and reap dead ones (sub_556345) (client: 53517b)
    int CountKillableMonsters();                // Check if player has a rewarded unit (client: 535b90)
    void SendVisibilityUpdates(); // 5561A1 (client: 535043)

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
