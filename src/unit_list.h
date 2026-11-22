#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>

class Unit;
class Player;

class UnitList {
public:
    virtual void Serialize(CArchive& ar); // 558ef8 (client: 536222)
    virtual void CallMethod5(); // 5591da (client: 53623e)

    UnitList(); // 555a1d (client: 534ebc)
    ~UnitList(); // 555a8d (client: 534ede)


    void AddTail(Unit* unit); // 555c1b (client: 534f4c)
    void AddTailAllocId(Unit* unit); // 555d67 (client: 534f68)
    void AddTailId6xxx(Unit* unit); // 555f1c (client: 534fa3)

    void Remove(Unit* unit); // 575aa0 (client: 546940) - Remove unit from list and in any do delete for this unit

    void ClearSentToMask(Player* player); // 5579d8 (client: 535b23) - Clear unit vision mask bits for player across this list
    void RegenTick(); // 5574b5 (client: 5358d9)
    Unit* FindById(uint16_t id); // 5560d2 (client: 534fdf) - Look up unit in list by ID
    void ProcessTick(); // 556345 (client: 53517b) - Tick alive units and reap dead ones
    int CountKillableMonsters(); // 557ab0 (client: 535b90) - Check if player has a rewarded unit
    void SendVisibilityUpdates(); // 5561a1 (client: 535043)

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
