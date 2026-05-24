#include "shelf.h"

#include "game_app.h"
#include "item.h"
#include "net.h"
#include "player.h"
#include "shop.h"
#include "unit.h"

IMPLEMENT_SERIAL(MultiShopShelf, CObject, 1); // 637270

// 544A9D
MultiShopShelf::MultiShopShelf() {
    this->shelf_id = 0xFFFFFFFF;
}

// 544B0C
MultiShopShelf::~MultiShopShelf() {
}

// 545865
void MultiShopShelf::ClearItems() {
    for (int i = 0; i < items.GetSize(); i++) {
        delete items[i];
    }
    items.RemoveAll();
}


IMPLEMENT_SERIAL(MultiShopInstance, CObject, 1); // 637288

// 545BD7
MultiShopInstance::MultiShopInstance() {
    this->unit = nullptr;
    this->shop_template = nullptr;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 545C8C
MultiShopInstance::~MultiShopInstance() {
    this->unit = nullptr;

    if (this->shop_template != nullptr) {
        this->shop_template->field_0x4 -= 1;
        this->shop_template = nullptr;
    }
}

// 545D5E
void MultiShopInstance::Sell() {
    bool any_sold = false;

    for (POSITION next_pos = this->inventory.items.GetHeadPosition(); next_pos != nullptr; ) {
        POSITION current_pos = next_pos;

        Item* item = this->inventory.items.GetNext(next_pos);

        if (item && item->pOwner != nullptr && item->_exp != 0) {
            int32_t price = item->count * item->_exp;
            item->pOwner->money += (int32_t)(price * 0.5L + 0.5L);

            this->inventory.items.RemoveAt(current_pos);

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
    this->unit->pOwner->sub_534AC1(0, 0);
    this->unit->sub_52A790(0);
    g_NetStru1_main.sub_519221(this->unit, this->unit->pOwner, 0x282000, 0xFFB, 0, 0);
}

// 546027
void MultiShopInstance::Buy() {
    for (POSITION next_pos = this->inventory.items.GetHeadPosition(); next_pos != nullptr; ) {
        POSITION current_pos = next_pos;
        Item* item = this->inventory.items.GetNext(next_pos);

        if (item && item->pOwner == nullptr) {
            int32_t price = item->count * item->_exp;
            if (this->unit->pOwner->money < price) {
                break;
            }

            this->unit->pOwner->money -= price;
            this->inventory.items.RemoveAt(current_pos);
            item->pOwner = this->unit->pOwner;
            item->field10_0x4c = 0;
            this->unit->inventory->PutItemIntoBagAtDefault(item);
        }
    }

    this->sub_5464B6(0);
    this->unit->pOwner->sub_534AC1(0, 0);
    this->unit->sub_52A790(0);
    g_NetStru1_main.sub_519221(this->unit, this->unit->pOwner, 0x282000, 0xFFB, 0, 0);
}


IMPLEMENT_SERIAL(MultiShopTemplate, CObject, 1); // 6372a0

// 54687B
MultiShopTemplate::MultiShopTemplate() {
    this->field_0x4 = 0;
    this->refresh_assortment_counter = 0;
    this->field_0x90 = 1000;
    this->shop = nullptr;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 546C72
MultiShopTemplate::MultiShopTemplate(Shop* shop) {
    this->field_0x4 = 0;
    this->refresh_assortment_counter = 0;
    this->field_0x90 = shop->field_0xc0;
    this->shop = shop;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 546DA4
MultiShopTemplate::~MultiShopTemplate() {
    for (int32_t i = 0; i < this->shop_instances.GetSize(); i++) {
        MultiShopInstance* inst = this->shop_instances[i];
        if (inst != nullptr) {
            delete inst;
            this->shop_instances[i] = nullptr;
        }
    }
    this->shop_instances.SetSize(0, -1);
    for (int32_t i = 0; i < 4; i++) {
        this->shelves[i].ClearItems();
    }
}
