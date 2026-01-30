#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>

#include "asm_mfc.h"
#include "item.h"
#include "mfc_templ.h"


struct Inventory {
public:
    CList<Item*> items;
    int32_t default_position;
    uint32_t total_weight;
};

static_assert(offsetof(Inventory, default_position) == 0x1c, "Inventory::default_position offset mismatch");
static_assert(sizeof(Inventory) == 0x24, "Inventory size mismatch");


#endif
