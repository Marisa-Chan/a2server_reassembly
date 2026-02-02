#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"

class Unit;

class UnitList {
public:
    void* vtable_placeholder;
    // virtual void Serialize(CArchive* archive);
    // virtual ~UnitList();

public:
    CList<Unit*> unit_list;
};

ASSERT_SIZE(UnitList, 0x20);

#endif
