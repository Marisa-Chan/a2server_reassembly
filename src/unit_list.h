#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"

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

    void sub_5579D8(Player* player); // Clear unit vision mask bits for player across this list
    void sub_5574B5();
    Unit* sub_5560D2(uint16_t id);   // Look up unit in list by ID
    void ProcessTick();              // Tick alive units and reap dead ones (sub_556345)
    int sub_557AB0();                // Check if player has a rewarded unit
    void sub_5561A1(); // 5561A1

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
