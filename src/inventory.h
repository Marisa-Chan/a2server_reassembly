#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "item.h"
#include "mfc_templ.h"


struct Inventory {
public:
    Inventory(); // 551C0A
    Inventory(const Inventory* src); // 551E55
    ~Inventory(); // 551C7A

public:
    void PutItemIntoBagAtDefault(Item *item);
    void PutItemIntoBag(int32_t pos, Item *item); // 551fc3
    Item* sub_552E42(int32_t item_number, int32_t count);
    void sub_552A42(Inventory* src); // Transfer all items from src into this inventory
    Item* sub_552B6B(uint16_t item_id); // Find item by item_id
    Item* sub_552C37(uint16_t item_id); // Remove and return item matching item_id from inventory
    Item* sub_5530A2(const char* name); // Find item by name, returns nullptr if not found
    void sub_55CCE4(CArchive& ar); // Serialize, in asm

public:
    CList<Item*> items;
    int32_t default_position;
    uint32_t total_weight;
};

ASSERT_OFFSET(Inventory, default_position, 0x1c);
ASSERT_SIZE(Inventory, 0x24);


#endif
