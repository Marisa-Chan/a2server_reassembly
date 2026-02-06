#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"

class Unit;

class UnitList {
public:
    virtual void Serialize(CArchive& ar);
    virtual void CallMethod5();

    UnitList();
    ~UnitList();


    void AddTail(Unit* unit);
    void AddTailAllocId(Unit* unit);
    void AddTailId6xxx(Unit* unit);

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
