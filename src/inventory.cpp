#include "inventory.h"


void Inventory::PutItemIntoBagAtDefault(Item* item)
{
	// 551fa3
	PutItemIntoBag(default_position, item);
}