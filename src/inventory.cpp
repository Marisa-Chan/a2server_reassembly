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

// 551fc3
void Inventory::PutItemIntoBag(int32_t pos, Item* item) {
    if (!item) {
        return;
    }

    item->field11_0x4d = 0;

    // Check if item is stackable
    if (item->VMethod16() != 0) {
        // Try to find an existing stackable item with the same item_id
        for (POSITION p = this->items.GetHeadPosition(); p != nullptr;) {
            Item* existing = this->items.GetNext(p);
            if (existing->item_id == item->item_id && existing->VMethod16() != 0) {
                // Stack them together
                this->total_weight += item->weight * item->count;
                existing->count += item->count;
                existing->TokenID |= item->TokenID;
                delete item;
                return;
            }
        }
    }

    // Insert item at the specified position
    int32_t count = this->items.GetCount();
    if (pos >= count) {
        // Add at tail
        this->items.AddTail(item);
    } else {
        // Insert at position
        POSITION insert_pos = this->items.GetHeadPosition();
        for (int32_t i = 0; i < pos && insert_pos != nullptr; i++) {
            this->items.GetNext(insert_pos);
        }
        if (insert_pos != nullptr) {
            this->items.InsertBefore(insert_pos, item);
        } else {
            this->items.AddTail(item);
        }
    }

    // Update total weight
    this->total_weight += item->weight * item->count;
}

// 552A42
void Inventory::sub_552A42(Inventory* src) {
    if (!src || this->items.GetCount() >= 0x400) {
        return;
    }

    while (true) {
        Item* item = src->sub_552E42(0, 1);
        if (!item) {
            break;
        }
        this->PutItemIntoBagAtDefault(item);
    }

    src->items.RemoveAll();
    src->~Inventory();
    delete src;
}

// 552B6B
Item* Inventory::sub_552B6B(uint16_t item_id) {
    for (POSITION pos = this->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = this->items.GetNext(pos);
        if (item->item_id == item_id) {
            return item;
        }
    }
    return nullptr;
}

// 552C37
Item* Inventory::sub_552C37(uint16_t item_id) {
    Item* item = this->sub_552B6B(item_id);
    if (!item) {
        return nullptr;
    }

    if (item->count > 1) {
        Item* taken = item->TakeOne();
        this->total_weight -= taken->weight * taken->count;
        return taken;
    }

    POSITION pos = this->items.Find(item);
    if (pos != nullptr) {
        this->items.RemoveAt(pos);
    }

    this->total_weight -= item->weight * item->count;
    return item;
}

// 552E42
Item* Inventory::sub_552E42(int32_t item_number, int32_t count) {
    if (item_number >= this->items.GetCount()) {
        return nullptr;
    }

    // Find the item at the specified position
    int32_t index = 0;
    for (POSITION pos = this->items.GetHeadPosition(); pos != nullptr;) {
        POSITION pos_before = pos;
        Item* item = this->items.GetNext(pos);
        if (index == item_number) {
            // Found the item
            if (item->count > count) {
                // Reduce count and return a copy
                item->count -= (count - 1);
                Item* taken = item->TakeOne();
                taken->count = count;
                this->total_weight -= taken->weight * taken->count;
                return taken;
            } else {
                // Remove and return the entire item
                this->items.RemoveAt(pos_before);
                this->total_weight -= item->weight * item->count;
                return item;
            }
        }
        index++;
    }

    return nullptr;
}

// 5530A2
Item* Inventory::sub_5530A2(const char* name) {
    for (POSITION pos = this->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = this->items.GetNext(pos);
        uint8_t item_type = (item->item_id >> 8) & 0xF;
        if (item_type == 0xE) {
            if (item->world_equip->name == name) {
                return item;
            }
        }
    }
    return nullptr;
}

// 55CCE4
void Inventory::sub_55CCE4(CArchive& ar) {
    this->items.Serialize(ar);

    if (ar.IsStoring()) {
        // Write default_position and total_weight
        ar << this->default_position;
        ar << this->total_weight;
    } else {
        // Read default_position and total_weight
        ar >> this->default_position;
        ar >> this->total_weight;
    }
}
