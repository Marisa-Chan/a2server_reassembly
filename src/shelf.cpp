#include "shelf.h"

#include "game_app.h"
#include "item.h"
#include "net.h"
#include "player.h"
#include "server.h"
#include "shop.h"
#include "shop_assortment.h"
#include "unit.h"

IMPLEMENT_SERIAL(CMultiShopShelf, CObject, 1); // 637270

// 544A9D
CMultiShopShelf::CMultiShopShelf() {
    this->shelf_id = 0xFFFFFFFF;
}

// 544B0C
CMultiShopShelf::~CMultiShopShelf() {
}

// 545865
void CMultiShopShelf::ClearItems() {
    for (int i = 0; i < items.GetSize(); i++) {
        delete items[i];
    }
    items.RemoveAll();
}


IMPLEMENT_SERIAL(CMultiShopInstance, CObject, 1); // 637288

// 545BD7
CMultiShopInstance::CMultiShopInstance() {
    this->unit = nullptr;
    this->shop_template = nullptr;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 545C8C
CMultiShopInstance::~CMultiShopInstance() {
    this->unit = nullptr;

    if (this->shop_template != nullptr) {
        this->shop_template->field_0x4 -= 1;
        this->shop_template = nullptr;
    }
}

// 545D5E
void CMultiShopInstance::Sell() {
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
void CMultiShopInstance::Buy() {
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


IMPLEMENT_SERIAL(CMultiShopTemplate, CObject, 1); // 6372a0

// 54687B
CMultiShopTemplate::CMultiShopTemplate() {
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
CMultiShopTemplate::CMultiShopTemplate(Shop* shop) {
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
CMultiShopTemplate::~CMultiShopTemplate() {
    for (int32_t i = 0; i < this->shop_instances.GetSize(); i++) {
        CMultiShopInstance* inst = this->shop_instances[i];
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

// 547644
void CMultiShopTemplate::sub_547644(Unit* unit) {
    if (this->shop_instances.GetSize() >= 250) {
        return;
    }
    if (this->shelves[0].items.GetSize() == 0) {
        this->sub_5474D8();
    }
    CMultiShopInstance* shop_instance = this->sub_546F8D(unit);
    shop_instance->sub_546857(unit);
    this->shop_instances.SetAtGrow(this->shop_instances.GetSize(), shop_instance);
    shop_instance->sub_5464B6(-1);
}

// 5474D8
void CMultiShopTemplate::sub_5474D8() {
    if (this->field_0x4 > 0) {
        return;
    }
    ShopAssortment assort;
    this->sub_54749F();
    AssortGenParams* params;
    if (g_Server->field4_0x74 == 0) {
        params = ScenarioGetShopAssortment();
    } else {
        params = (AssortGenParams*)this->shop->gen_params;
    }
    for (int32_t i = 0; i < 4; i++) {
        AssortGenParams* p = &params[i];
        assort.GenerateAssortment(p);
        assort.ArrangeShelfs(p->max_count, p->max_same_count, p->min_cost, p->max_cost, &this->shelves[i].items);
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            this->shelves[i].items[j]->field11_0x4d = this->shelves[i].shelf_id;
        }
    }
}

// 546F8D
CMultiShopInstance* CMultiShopTemplate::sub_546F8D(Unit* unit) {
    CMultiShopInstance* instance = new CMultiShopInstance();
    instance->unit = unit;
    instance->shop_template = this;
    this->field_0x4 += 1;
    for (int32_t i = 0; i < 4; i++) {
        instance->shelves[i].items.SetSize(this->shelves[i].items.GetSize(), -1);
        int32_t added_items = 0;
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            Item* item = this->shelves[i].items[j];
            if (item->count != 0) {
                instance->shelves[i].items[added_items] = item;
                item->field10_0x4c += 1;
                added_items += 1;
            }
        }
        instance->shelves[i].items.SetSize(added_items, -1);
    }
    return instance;
}

// 5479C6
void CMultiShopTemplate::sub_5479C6(Humanoid* humanoid) {
    int32_t index = this->sub_54740C(humanoid);
    if (index == -1) {
        return;
    }
    CMultiShopInstance* instance = this->shop_instances[index];
    this->shop_instances.RemoveAt(index);
    delete instance;
    for (int32_t i = 0; i < 4; i++) {
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            Item* item = this->shelves[i].items[j];
            if (item->count == 0 && item->field10_0x4c == 0) {
                this->shelves[i].sub_54546E(j);
                j -= 1;
            }
        }
    }
    this->sub_547E91();
}

// 547DD4
void CMultiShopTemplate::sub_547DD4() {
    if (g_Server->field4_0x74 == 0) {
        return;
    }
    if (g_Server->tick16 % 180 == 0) {
        this->refresh_assortment_counter += 1;
        this->sub_547E91();
    }
    for (int i = 0; i < this->shop_instances.GetSize(); i++) {
        Unit* unit = this->shop_instances[i]->unit;
        if (unit->pOwner == nullptr) {
            this->sub_5479C6(static_cast<Humanoid*>(unit));
            continue;
        }
        if (unit->pOwner->field_0x43 == 0) {
            this->shop->sub_544685(unit);
        }
    }
}
