#include "shelf.h"

#include "game_app.h"
#include "item.h"
#include "net.h"
#include "player.h"
#include "unit.h"

// 545865
void MultiShopShelf::ClearItems() {
    for (int i = 0; i < items.GetSize(); i++) {
        delete items[i];
    }
    items.RemoveAll();
}

// 545D5E
void MultiShopInstance::Sell() {
    POSITION next_pos = this->inventory.items.GetHeadPosition();
    bool any_sold = false;

    while (true) {
        POSITION current_pos = next_pos;
        Item* item = (next_pos != nullptr) ? this->inventory.items.GetNext(next_pos) : nullptr;

        if (item == nullptr) {
            break;
        }

        if (item->pOwner != nullptr && item->_exp != 0) {
            int32_t price = item->count * item->_exp;
            item->pOwner->money += (int32_t)(price * 0.5L + 0.5L);

            this->inventory.sub_574C20((CList<Item*>::CNode*)current_pos);

            item->pOwner = nullptr;
            if (this->sub_5462C8(item, -1) == 1) {
                any_sold = true;
            }
        }
    }
    
    if (any_sold) {
        this->shop_template->sub_546F18(this, -1);
    }
    this->sub_5464B6(0);
    this->sub_5464B6(-1);
    this->unit->pOwner->FUN_00534AC1(0, 0);
    this->unit->sub_52A790(0);
    g_NetStru1_main.sub_519221(this->unit, this->unit->pOwner, 0x282000, 0xFFB, 0, 0);
}

// 546027
void MultiShopInstance::Buy() {
    POSITION next_pos = this->inventory.items.GetHeadPosition();

    while (true) {
        POSITION current_pos = next_pos;
        Item* item = (next_pos != nullptr) ? this->inventory.items.GetNext(next_pos) : nullptr;

        if (item == nullptr) {
            break;
        }

        if (item->pOwner == nullptr) {
            int32_t price = item->count * item->_exp;
            if (this->unit->pOwner->money < price) {
                break;
            }

            this->unit->pOwner->money -= price;
            this->inventory.sub_574C20((CList<Item*>::CNode*)current_pos);
            item->pOwner = this->unit->pOwner;
            item->field10_0x4c = 0;
            this->unit->inventory->PutItemIntoBagAtDefault(item);
        }
    }

    this->sub_5464B6(0);
    this->unit->pOwner->FUN_00534AC1(0, 0);
    this->unit->sub_52A790(0);
    g_NetStru1_main.sub_519221(this->unit, this->unit->pOwner, 0x282000, 0xFFB, 0, 0);
}
