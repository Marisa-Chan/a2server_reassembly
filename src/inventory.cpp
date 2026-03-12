#include "inventory.h"


Inventory::~Inventory()
{
    // 551C7A
    for (POSITION pos = items.GetHeadPosition(); pos != nullptr;) {
        Item* item = items.GetNext(pos);
        delete item;
    }
    items.RemoveAll();
}

void Inventory::PutItemIntoBagAtDefault(Item* item)
{
	// 551fa3
	PutItemIntoBag(default_position, item);
}
