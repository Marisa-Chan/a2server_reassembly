#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include "item.h"
#include <afxtempl.h>


struct Inventory {
public:
    Inventory(); // 551c0a (client: 5336d5)
    Inventory(const Inventory* src); // 551e55 (client: 533747)
    ~Inventory(); // 551c7a (client: 5336ff)

public:
    void PutItemIntoBagAtDefault(Item *item); // 551fa3 (client: 5337e4)
    void PutItemIntoBag(int32_t pos, Item *item); // 551fc3 (client: 533804)
    Item* PeekItemAt(int32_t item_number, POSITION* out_pos = nullptr); // split from 552e42 (client: 533cd8)
    Item* TakeItemAtPos(POSITION it, int32_t count = 1); // split from 552e42 (client: 533cd8)
    Item* TakeItemAt(int32_t item_number, int32_t count); // 552e42 (client: 533cd8)
    void AddItem(int32_t amount, Item* item); // 5524fa (client: 5339b8) - add item to inventory
    void TransferFrom(Inventory* src); // 552a42 (client: 533b7d) - Transfer all items from src into this inventory
    Item* FindItemById(uint16_t item_id); // 552b6b (client: 533c07) - Find item by item_id
    Item* TakeItemById(uint16_t item_id); // 552c37 (client: 533c61) - Remove and return item matching item_id from inventory
    Item* FindItemByName(const char* name); // 5530a2 (client: none) - Find item by name, returns nullptr if not found
    void Serialize(CArchive& ar); // 55cce4 (client: 53776f) - Serialize

    void SerializeItems(CArchive& ar); // split from 55cce4 (client: 53776f) - Serialize items list

public:
    CList<Item*> items;
    int32_t default_position;
    uint32_t total_weight;
};

ASSERT_OFFSET(Inventory, default_position, 0x1c);
ASSERT_SIZE(Inventory, 0x24);


#endif
