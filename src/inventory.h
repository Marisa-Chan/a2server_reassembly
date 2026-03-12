#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "item.h"
#include "mfc_templ.h"


struct Inventory {
public:
    Inventory(); // sub_551C0A — initializes CList and default fields
    void PutItemIntoBagAtDefault(Item *item);
    void PutItemIntoBag(int32_t pos, Item *item); // in asm
    Item* sub_552E42(int32_t item_number, int32_t count);

public:
    CList<Item*> items;
    int32_t default_position;
    uint32_t total_weight;
};

ASSERT_OFFSET(Inventory, default_position, 0x1c);
ASSERT_SIZE(Inventory, 0x24);


#endif
