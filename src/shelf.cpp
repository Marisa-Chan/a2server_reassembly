#include "shelf.h"

#ifdef A2SERVER_PATCH
#include <climits>
#endif

#include "game_app.h"
#include "item.h"
#include "net.h"
#include "packet.h"
#include "player.h"
#include "server.h"
#include "shop.h"
#include "shop_assortment.h"
#include "unit.h"

IMPLEMENT_SERIAL(CMultiShopShelf, CObject, 1); // 637270

// 544a9d (client: 52b138)
CMultiShopShelf::CMultiShopShelf() {
    this->shelf_id = 0xFFFFFFFF;
}

// 544b0c (client: 52b199)
CMultiShopShelf::~CMultiShopShelf() {
}

// 545865 (client: 52b591)
void CMultiShopShelf::ClearItems() {
    for (int i = 0; i < this->items.GetSize(); i++) {
        #ifdef A2SERVER_PATCH
        // bug_0010: skip deleting zombie items (zeroed vtable)
        if (*reinterpret_cast<void**>(this->items[i]) != nullptr) {
            delete this->items[i];
        }
        #else
        delete this->items[i];
        #endif
    }
    this->items.RemoveAll();
}

// 54546e (client: 52b4fd) - remove item at index
void CMultiShopShelf::RemoveItemAt(int32_t index) {
    if (index >= this->items.GetSize() || index < 0) {
        return;
    }
    this->items.RemoveAt(index);
}

// 5453e8 (client: 52b473) - pick item from shelf
Item* CMultiShopShelf::PickItem(int32_t slot, int32_t count) {
    if (slot >= this->items.GetSize() || slot < 0) {
        return nullptr;
    }

    Item* item = this->items[slot];
    if (item->count < count) {
        count = item->count;
    }
    item->count -= count - 1;
    Item* taken = item->TakeOne();
    taken->count = count;
    return taken;
}

// 544bce (client: 52b1ed) - add item to shelf.
int32_t CMultiShopShelf::AddItem(Item** item_ptr) {
    Item* item = *item_ptr;
    item->shelf_id = this->shelf_id;

    for (int32_t i = 0; i < this->items.GetSize(); i++) {
        Item* existing = this->items[i];
        if ((item->IsSimpleItem() && existing->IsSimilar(item)) || (!item->IsSimpleItem() && existing->count == 0 && existing->IsSimilar(item))) {
            existing->count += item->count;
            delete item;
            *item_ptr = existing;
            return 1;
        }
    }

    this->items.SetAtGrow(this->items.GetSize(), item);
    return 0;
}

// 545528 (client: 52b531) - decrease refcount of all items and remove them from the shelf
void CMultiShopShelf::ReleaseItems() {
    for (int32_t i = 0; i < this->items.GetSize(); i++) {
        this->items[i]->shelf_refcount -= 1;
    }
    this->items.RemoveAll();
}


IMPLEMENT_SERIAL(CMultiShopInstance, CObject, 1); // 637288

// 545bd7 (client: 52b62f)
CMultiShopInstance::CMultiShopInstance() {
    this->unit = nullptr;
    this->shop_template = nullptr;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 545c8c (client: 52b6e3)
CMultiShopInstance::~CMultiShopInstance() {
    for (size_t i = 0; i < this->shelves.size(); i++) {
        this->shelves[i].ReleaseItems();
    }

    this->unit = nullptr;

    if (this->shop_template != nullptr) {
        this->shop_template->active_instances -= 1;
        this->shop_template = nullptr;
    }
}

#ifdef A2SERVER_PATCH
static void LogMoneyCheatAttempt(Player* player) {
    CString msg;
    msg.Format("E667: Money cheat attempt from character \"%s\"", static_cast<const char*>(player->name));
    LogMessage(msg);
}
#endif

// 545d5e (client: 52b7b4)
void CMultiShopInstance::Sell() {
    bool any_sold = false;

    for (POSITION next_pos = this->inventory.items.GetHeadPosition(); next_pos != nullptr; ) {
        POSITION current_pos = next_pos;

        Item* item = this->inventory.items.GetNext(next_pos);

        if (item && item->pOwner != nullptr && item->_exp != 0) {
#ifdef A2SERVER_PATCH
            int64_t price64 = static_cast<int64_t>(static_cast<uint16_t>(item->count)) * static_cast<int64_t>(item->_exp);
            if (price64 > INT32_MAX || price64 < INT32_MIN) {
                LogMoneyCheatAttempt(this->unit->pOwner);
                break;
            }
            int32_t price = static_cast<int32_t>(price64);
            int32_t sell_gain = static_cast<int32_t>(price * 0.5 + 0.5);
            int64_t new_money = static_cast<int64_t>(item->pOwner->money) + static_cast<int64_t>(sell_gain);
            if (new_money > INT32_MAX || new_money < INT32_MIN) {
                LogMoneyCheatAttempt(this->unit->pOwner);
                break;
            }
            item->pOwner->money = static_cast<int32_t>(new_money);
#else
            int32_t price = item->count * item->_exp;
            item->pOwner->money += (int32_t)(price * 0.5L + 0.5L);
#endif

            this->inventory.items.RemoveAt(current_pos);

            item->pOwner = nullptr;
            if (this->StowItemInShelf(item, -1) == 1) {
                any_sold = true;
            }
        }
    }
    
    if (any_sold) {
        this->shop_template->NotifyOtherInstances(this, -1);
    }
    this->SendShelfState(0);
    this->SendShelfState(-1);
    this->unit->pOwner->AddMoney(0, 0);
    this->unit->AddCarriedWeight(0);
    g_NetStru1_main.SendUnitUpdate(this->unit, this->unit->pOwner, UNIT_UPD_INVENTORY_WEIGHT_SPEED, EQUIP_SLOTS_ALL, 0, 0);
}

// 546027 (client: 52b8fa)
void CMultiShopInstance::Buy() {
    for (POSITION next_pos = this->inventory.items.GetHeadPosition(); next_pos != nullptr; ) {
        POSITION current_pos = next_pos;
        Item* item = this->inventory.items.GetNext(next_pos);

        if (item && item->pOwner == nullptr) {
#ifdef A2SERVER_PATCH
            int64_t price64 = static_cast<int64_t>(static_cast<uint16_t>(item->count)) * static_cast<int64_t>(item->_exp);
            if (price64 > INT32_MAX || price64 < INT32_MIN) {
                LogMoneyCheatAttempt(this->unit->pOwner);
                break;
            }
            int32_t price = static_cast<int32_t>(price64);
#else
            int32_t price = item->count * item->_exp;
#endif
            if (this->unit->pOwner->money < price) {
                break;
            }

            this->unit->pOwner->money -= price;
            this->inventory.items.RemoveAt(current_pos);
            item->pOwner = this->unit->pOwner;
            item->shelf_refcount = 0;
            this->unit->inventory->PutItemIntoBagAtDefault(item);
        }
    }

    this->SendShelfState(0);
    this->unit->pOwner->AddMoney(0, 0);
    this->unit->AddCarriedWeight(0);
    g_NetStru1_main.SendUnitUpdate(this->unit, this->unit->pOwner, UNIT_UPD_INVENTORY_WEIGHT_SPEED, EQUIP_SLOTS_ALL, 0, 0);
}

// 546537 (client: 52bc62) - add item to inventory
void CMultiShopInstance::SellItemFor(int32_t amount, Item* item) {
    this->inventory.AddItem(amount, item);
}

// 546557 (client: 52bc82) - pick item from inventory
Item* CMultiShopInstance::PickItem(int32_t item_number, int32_t count) {
    return this->inventory.TakeItemAt(item_number, count);
}

// 546857 (client: 52be85)
Unit* CMultiShopInstance::SetUnit(Unit* new_unit) {
    Unit* old_unit = this->unit;
    this->unit = new_unit;
    return old_unit;
}

// 5464b6 (client: 52bc0b)
void CMultiShopInstance::SendShelfState(int32_t shelf_id) {
    if (shelf_id == 0) {
        Token* shop_token = nullptr;
        if (this->shop_template != nullptr) {
            shop_token = this->shop_template->shop;
        }
        g_NetStru1_main.SendShopTable(shop_token, &this->inventory, this->unit->pOwner, ITEM_LIST_SHOP_TABLE);
    } else {
        g_NetStru1_main.SendShopShelves(this, this->unit->pOwner, shelf_id);
    }
}

// 546577 (client: 52bca2)
void CMultiShopInstance::MoveItems(uint8_t src_type, int16_t src_slot, uint8_t dst_type, int16_t dst_slot, int32_t count) {
    int32_t result = 0;
    Item* item = nullptr;

    if (src_type == TRANSFER_SRC_SHOP) {
        item = this->PickItem(src_slot, count);
        if (item == nullptr || item->count == 0) {
            this->SendShelfState(0);
            return;
        }
    } else {
        item = this->shelves[src_type - TRANSFER_SRC_SHELF_1].PickItem(src_slot, count);
        if (item == nullptr || item->count == 0) {
            this->SendShelfState(src_type - 4);
            return;
        }
        this->shop_template->NotifyOtherInstances(this, src_type - 4);
    }

    if (dst_type == TRANSFER_DST_SHOP) {
        this->SellItemFor(dst_slot, item);
    } else {
        result = this->StowItemInShelf(item, dst_slot);
    }

    if (result != 0) {
        this->shop_template->NotifyOtherInstances(this, -1);
    }
}

// 54668b (client: 52bdaa)
void CMultiShopInstance::ReturnConsignedItems() {
    if (this->inventory.items.GetCount() == 0) {
        return;
    }

    for (POSITION it = this->inventory.items.GetHeadPosition(); it != nullptr; ) {
        Item* item = this->inventory.items.GetNext(it);
        if (item->pOwner != nullptr) {
            this->unit->inventory->PutItemIntoBagAtDefault(item);
        } else {
            this->StowItemInShelf(item, -1);
        }
    }

    this->inventory.items.RemoveAll();
    this->SendShelfState(0);
    this->SendShelfState(-1);
    this->shop_template->NotifyOtherInstances(this, -1);
    g_NetStru1_main.SendUnitUpdate(this->unit, this->unit->pOwner, UNIT_UPD_INVENTORY_WEIGHT_SPEED, EQUIP_SLOTS_ALL, 0, 0);
}

// 5462c8 (client: 52ba1d)
int32_t CMultiShopInstance::StowItemInShelf(Item* item, int32_t dst_slot) {
    int32_t shelf_id = 0;
    int32_t result = 0;

    if (item->shelf_id == 0) {
        AssortGenParams* params;
        if (g_Server->multiplayer != 0) {
            params = this->shop_template->shop->gen_params;
        } else {
            params = ScenarioGetShopAssortment();
        }

        uint32_t type_flag = 0;
        if (item->item_type == Item::BOOK || item->item_type == Item::SCROLL || item->item_type == Item::POTION) {
            type_flag = AssortFlags::ALLOW_CONSUMABLES;
        } else if (item->item_type == Item::WEAPON) {
            type_flag = AssortFlags::ALLOW_WARRIOR_WEAPONS;
        } else if (item->item_type == Item::EQUIPMENT) {
            type_flag = AssortFlags::ALLOW_ARMORS;
        }

        if (item->IsSimpleItem() == 0 && item->item_type != Item::BOOK) {
            while (!params[shelf_id].flags.generate_magic && shelf_id < 3) {
                shelf_id++;
            }
        } else {
            while (((params[shelf_id].flags & type_flag) == 0 || params[shelf_id].flags.generate_magic) && shelf_id < 3) {
                shelf_id++;
            }
            if ((params[shelf_id].flags & type_flag) == 0 || params[shelf_id].flags.generate_magic) {
                shelf_id = 0;
                while ((params[shelf_id].flags & type_flag) == 0 && shelf_id < 3) {
                    shelf_id++;
                }
            }
        }
    } else {
        shelf_id = item->shelf_id - 1;
    }

    result = this->shop_template->shelves[shelf_id].AddItem(&item);
    if (result != 0) {
        item->shelf_refcount = 0;
    } else {
        result = this->shelves[shelf_id].AddItem(&item);
    }
    return result;
}


IMPLEMENT_SERIAL(CMultiShopTemplate, CObject, 1); // 6372a0

// 54687b (client: 52bea9)
CMultiShopTemplate::CMultiShopTemplate() {
    this->active_instances = 0;
    this->refresh_assortment_counter = 0;
    this->start_money = 1000;
    this->shop = nullptr;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 546c72 (client: 52bf83)
CMultiShopTemplate::CMultiShopTemplate(Shop* shop) {
    this->active_instances = 0;
    this->refresh_assortment_counter = 0;
    this->start_money = shop->money;
    this->shop = shop;
    this->shelves[0].shelf_id = 1;
    this->shelves[1].shelf_id = 2;
    this->shelves[2].shelf_id = 3;
    this->shelves[3].shelf_id = 4;
}

// 546da4 (client: 52c05e)
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

// 546f18 (client: 52c16a)
void CMultiShopTemplate::NotifyOtherInstances(CMultiShopInstance* inst, int shelf_id) {
    for (int32_t i = 0; i < this->shop_instances.GetSize(); i++) {
        CMultiShopInstance* other = this->shop_instances[i];
        if (other != nullptr && other != inst) {
            other->SendShelfState(shelf_id);
        }
    }
}

// 54740c (client: 52c395) - find shop instance index for unit's owner
int CMultiShopTemplate::FindInstanceIndex(Unit* unit) {
    for (int32_t i = 0; i < this->shop_instances.GetSize(); i++) {
        if (this->shop_instances[i]->unit->pOwner == unit->pOwner) {
            return i;
        }
    }
    return -1;
}

// 547e91 (client: 52ca6a) - refresh assortment if no live instances
int CMultiShopTemplate::TryRefreshAssortment() {
    if (this->refresh_assortment_counter <= 0 || this->active_instances != 0) {
        return 0;
    }
    this->GenerateAssortment();
    this->refresh_assortment_counter = 0;
    return 1;
}

// 547468 (client: 52c3ef) - find shop instance for player's unit
CMultiShopInstance* CMultiShopTemplate::FindInstanceFor(Humanoid* humanoid) {
    int32_t index = this->FindInstanceIndex(humanoid);
    if (index == -1) {
        return nullptr;
    }
    return this->shop_instances[index];
}

// 54749f (client: 52c428) - clear all shelf items
void CMultiShopTemplate::ClearShelves() {
    for (int32_t i = 0; i < 4; i++) {
        this->shelves[i].ClearItems();
    }
}

// 547b7b (client: 52c76b)
void CMultiShopTemplate::ConfirmBuy(Unit* unit) {
    CMultiShopInstance* instance = this->FindInstanceFor(static_cast<Humanoid*>(unit));
    if (instance == nullptr) {
        return;
    }
    instance->SetUnit(unit);
    instance->Buy();
}

// 547bbc (client: 52c7ac)
void CMultiShopTemplate::ConfirmSell(Unit* unit) {
    CMultiShopInstance* instance = this->FindInstanceFor(static_cast<Humanoid*>(unit));
    if (instance == nullptr) {
        return;
    }
    instance->SetUnit(unit);
    instance->Sell();
}

// 547d7f (client: 52c96f)
void CMultiShopTemplate::ReturnConsignedItems(Unit* unit) {
    CMultiShopInstance* instance = this->FindInstanceFor(static_cast<Humanoid*>(unit));
    if (instance == nullptr) {
        LogMessage("Invalid shop transaction - no such customer");
        return;
    }
    instance->SetUnit(unit);
    instance->ReturnConsignedItems();
}

// 547bfd (client: 52c7ed)
void CMultiShopTemplate::SellItemFor(Unit* unit, int amount, Item* item) {
    CMultiShopInstance* instance = this->FindInstanceFor(static_cast<Humanoid*>(unit));
    if (instance == nullptr) {
        LogMessage("Invalid shop transaction - no such customer");
        return;
    }
    instance->SetUnit(unit);
    instance->SellItemFor(amount, item);
}

// 547c5a (client: 52c84a)
Item* CMultiShopTemplate::BuyItem(Humanoid* humanoid, int16_t src_slot, int32_t count) {
    CMultiShopInstance* instance = this->FindInstanceFor(humanoid);
    if (instance == nullptr) {
        LogMessage("Invalid shop transaction - no such customer");
        return nullptr;
    }
    instance->SetUnit(humanoid);
    return instance->PickItem(src_slot, count);
}

// 547644 (client: 52c5bb)
void CMultiShopTemplate::OpenShopFor(Unit* unit) {
#ifdef A2SERVER_PATCH
    if (CMultiShopInstance* existing = this->FindInstanceFor(static_cast<Humanoid*>(unit))) {
        existing->SetUnit(unit);
        existing->SendShelfState(-1);
        return;
    }
#endif
    if (this->shop_instances.GetSize() >= 250) {
        return;
    }
    if (this->shelves[0].items.GetSize() == 0) {
        this->GenerateAssortment();
    }
    CMultiShopInstance* shop_instance = this->CreateInstance(unit);
    shop_instance->SetUnit(unit);
    this->shop_instances.SetAtGrow(this->shop_instances.GetSize(), shop_instance);
    shop_instance->SendShelfState(-1);
}

// 5474d8 (client: 52c461)
void CMultiShopTemplate::GenerateAssortment() {
    if (this->active_instances > 0) {
        return;
    }
    ShopAssortment assort;
    this->ClearShelves();
    AssortGenParams* params;
    if (g_Server->multiplayer == 0) {
        params = ScenarioGetShopAssortment();
    } else {
        params = this->shop->gen_params;
    }
    for (int32_t i = 0; i < 4; i++) {
        AssortGenParams* p = &params[i];
        assort.GenerateAssortment(p);
        assort.ArrangeShelfs(p->max_count, p->max_same_count, p->min_cost, p->max_cost, &this->shelves[i].items);
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            this->shelves[i].items[j]->shelf_id = this->shelves[i].shelf_id;
        }
    }
}

// 546f8d (client: 52c1e1)
CMultiShopInstance* CMultiShopTemplate::CreateInstance(Unit* unit) {
    CMultiShopInstance* instance = new CMultiShopInstance();
    instance->unit = unit;
    instance->shop_template = this;
    this->active_instances += 1;
    for (int32_t i = 0; i < 4; i++) {
        instance->shelves[i].items.SetSize(this->shelves[i].items.GetSize(), -1);
        int32_t added_items = 0;
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            Item* item = this->shelves[i].items[j];
            if (item->count != 0) {
                instance->shelves[i].items[added_items] = item;
                item->shelf_refcount += 1;
                added_items += 1;
            }
        }
        instance->shelves[i].items.SetSize(added_items, -1);
    }
    return instance;
}

// 547cb9 (client: 52c8a9)
void CMultiShopTemplate::HandleItemOperation(Humanoid* humanoid, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count) {
    CMultiShopInstance* shop_instance = this->FindInstanceFor(humanoid);
    if (shop_instance == nullptr) {
        LogMessage("Invalid shop transaction - no such customer");
        return;
    }
    shop_instance->SetUnit(humanoid);
    shop_instance->MoveItems(op_type, src_slot, dst_type, dst_word, count);
}

// 5479c6 (client: 52c630)
void CMultiShopTemplate::CloseInstance(Humanoid* humanoid) {
    int32_t index = this->FindInstanceIndex(humanoid);
    if (index == -1) {
        return;
    }
    CMultiShopInstance* instance = this->shop_instances[index];
    this->shop_instances.RemoveAt(index);
    delete instance;
    for (int32_t i = 0; i < 4; i++) {
        for (int32_t j = 0; j < this->shelves[i].items.GetSize(); j++) {
            Item* item = this->shelves[i].items[j];
            if (item->count == 0 && item->shelf_refcount == 0) {
                this->shelves[i].RemoveItemAt(j);
                j -= 1;
            }
        }
    }
    this->TryRefreshAssortment();
}

// 547dd4 (client: 52c9c4)
void CMultiShopTemplate::RefreshTick() {
    if (g_Server->multiplayer == 0) {
        return;
    }
    if (g_Server->tick16 % 180 == 0) {
        this->refresh_assortment_counter += 1;
        this->TryRefreshAssortment();
    }
    for (int32_t i = 0; i < this->shop_instances.GetSize();) {
        Unit* unit = this->shop_instances[i]->unit;
        if (unit->pOwner == nullptr) {
            this->CloseInstance(static_cast<Humanoid*>(unit));
            continue;
        }
        if (unit->pOwner->is_active == 0) {
            this->shop->CloseShop(unit);
        }
        i++;
    }
}
