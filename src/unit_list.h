#ifndef UNIT_LIST_H
#define UNIT_LIST_H

#include "asm_mfc.h"
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

static_assert(sizeof(UnitList) == 0x20, "UnitList size mismatch");

#endif
