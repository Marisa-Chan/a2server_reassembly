#include "shop.h"

#include "game_app.h"
#include "net.h"
#include "server.h"
#include "shelf.h"
#include "unit.h"


IMPLEMENT_SERIAL(Shop, Building, 1);

// 5442ca (client: 52a9f2)
Shop::Shop() {
    this->money = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
    this->gen_params[0].min_cost = 1000000;
    this->gen_params[0].max_cost = 10000000;
    this->gen_params[0].max_count = 1;
    this->gen_params[0].max_same_count = 1;
    this->gen_params[0].flags = AssortFlags::ALL_SHOP_DEFAULT;
    this->gen_params[1] = this->gen_params[0];
    this->gen_params[2] = this->gen_params[0];
    this->gen_params[3] = this->gen_params[0];
}

// 5443ef (client: 52ab14)
Shop::Shop(const CString& name) : Building(name) {
    this->money = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
}

// 544495 (client: 52abb7)
Shop::Shop(uint8_t type_id, TokenPos* pos) : Building(type_id, pos, 0, 0) {
    this->money = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
}

// 544564 (client: 52ac81)
Shop::~Shop() {
    if (this->shop_template != nullptr) {
        delete this->shop_template;
    }
    this->shop_template = nullptr;
}

// 5447b7 (client: 52aecc)
void Shop::Serialize(CArchive& ar) {
    Building::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->money;
    } else {
        ar >> this->money;
    }
}

// 544629 (client: 52ad3e)
void Shop::RegenTick() {
    this->shop_template->RefreshTick();
}

// 5446c7 (client: 52addc)
Item* Shop::BuyItem(Humanoid* humanoid, int16_t src_slot, int32_t count) {
#if defined(A2SERVER_PATCH) && !defined(A2CLIENT)
    // sub_5446C7 hook (5446C7): only allow taking back consigned player-owned
    // items (item->pOwner != nullptr) from the shop counter, blocking unpaid
    // shop items from being dragged directly into the player's inventory.
    if (src_slot < 0 || this->shop_template == nullptr) {
        return nullptr;
    }
    CMultiShopInstance* instance = this->shop_template->FindInstanceFor(humanoid);
    if (instance == nullptr) {
        return nullptr;
    }
    int32_t idx = 0;
    bool allowed = false;
    for (POSITION pos = instance->inventory.items.GetHeadPosition(); pos != nullptr; ++idx) {
        Item* item = instance->inventory.items.GetNext(pos);
        if (item == nullptr) {
            return nullptr;
        }
        if (idx == src_slot) {
            if (item->pOwner == nullptr) {
                return nullptr;
            }
            allowed = true;
            break;
        }
    }
    if (!allowed) {
        return nullptr;
    }
#endif
    return this->shop_template->BuyItem(humanoid, src_slot, count);
}

// 5446eb (client: 52ae00)
void Shop::HandleItemOperation(Humanoid* unit, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count) {
    this->shop_template->HandleItemOperation(unit, op_type, src_slot, dst_type, dst_word, count);
}

// 54463f (client: 52ad54)
void Shop::GenerateAssortment() {
    this->shop_template->GenerateAssortment();
}

// 544655 (client: 52ad6a) - Mine: attach unit to this building
void Shop::AttachUnit(Unit* unit) {
    this->shop_template->OpenShopFor(unit);
    if (g_Server->multiplayer != 0) {
        unit->RemoveFromMap();
    }
}

// 544685 (client: 52ad9a)
void Shop::CloseShop(Unit* unit) {
    this->shop_template->CloseInstance(static_cast<Humanoid*>(unit));
    if (g_Server->multiplayer != 0) {
        unit->ReturnToMap();
    }
    g_NetStru1_main.BroadcastTokenState(unit, nullptr, 0);
}

// 54471b (client: 52ae30)
void Shop::ConfirmBuy(Unit* unit) {
    this->shop_template->ConfirmBuy(unit);
}

// 544737 (client: 52ae4c)
void Shop::ConfirmSell(Unit* unit) {
    this->shop_template->ConfirmSell(unit);
}

// 544777 (client: 52ae8c)
void Shop::ReturnItems(Unit* unit) {
    this->shop_template->ReturnConsignedItems(unit);
}

// 544793 (client: 52aea8)
void Shop::SellItemFor(Humanoid* humanoid, int amount, Item* item) {
    this->shop_template->SellItemFor(humanoid, amount, item);
}
