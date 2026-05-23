#include "inventory.h"

// 551C0A
Inventory::Inventory() {
    this->default_position = 0x7FFF;
    this->total_weight = 0;
}

// 551E55
Inventory::Inventory(const Inventory* src) {
    for (POSITION pos = src->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = src->items.GetNext(pos);
        this->PutItemIntoBagAtDefault(item->TakeOne());
    }
    this->default_position = src->default_position;
    this->total_weight = src->total_weight;
}

// 551C7A
Inventory::~Inventory() {
    for (POSITION pos = this->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = this->items.GetNext(pos);
        delete item;
    }
    this->items.RemoveAll();
}

// 551fa3
void Inventory::PutItemIntoBagAtDefault(Item* item) {
    this->PutItemIntoBag(this->default_position, item);
}
