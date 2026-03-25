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
    ~Inventory(); // sub_551C7A
    void PutItemIntoBagAtDefault(Item *item);
    void PutItemIntoBag(int32_t pos, Item *item); // in asm
    Item* sub_552E42(int32_t item_number, int32_t count);
    void sub_552A42(Inventory* src); // Transfer all items from src into this inventory

public:
    CList<Item*> items;
    int32_t default_position;
    uint32_t total_weight;
};

ASSERT_OFFSET(Inventory, default_position, 0x1c);
ASSERT_SIZE(Inventory, 0x24);


#endif
