#include "shop.h"

#include "game_app.h"
#include "net.h"
#include "server.h"
#include "shelf.h"
#include "unit.h"


IMPLEMENT_SERIAL(Shop, Building, 1);

// 5442CA
Shop::Shop() {
    this->field_0xc0 = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
    this->gen_params[0].min_cost = 1000000;
    this->gen_params[0].max_cost = 10000000;
    this->gen_params[0].max_count = 1;
    this->gen_params[0].max_same_count = 1;
    this->gen_params[0].flags = 0x2FFFFFFF;
    this->gen_params[1] = this->gen_params[0];
    this->gen_params[2] = this->gen_params[0];
    this->gen_params[3] = this->gen_params[0];
}

// 5443EF
Shop::Shop(const CString& name) : Building(name) {
    this->field_0xc0 = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
}

// 544495
Shop::Shop(uint8_t type_id, TokenPos* pos) : Building(type_id, pos, 0, 0) {
    this->field_0xc0 = 10000000;
    this->shop_template = new CMultiShopTemplate(this);
}

// 544564
Shop::~Shop() {
    if (this->shop_template != nullptr) {
        delete this->shop_template;
    }
    this->shop_template = nullptr;
}

// 5447B7
void Shop::Serialize(CArchive& ar) {
    Building::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->field_0xc0;
    } else {
        ar >> this->field_0xc0;
    }
}

// 544629
void Shop::RegenTick() {
    this->shop_template->RefreshTick();
}

// 5446C7
Item* Shop::BuyItem(Humanoid* humanoid, int16_t src_slot, int32_t count) {
    return this->shop_template->BuyItem(humanoid, src_slot, count);
}

// 5446EB
void Shop::HandleItemOperation(Humanoid* unit, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count) {
    this->shop_template->HandleItemOperation(unit, op_type, src_slot, dst_type, dst_word, count);
}

// 54463F
void Shop::GenerateAssortment() {
    this->shop_template->GenerateAssortment();
}

// 544655
void Shop::AttachUnit(Unit* unit) {
    this->shop_template->OpenShopFor(unit);
    if (g_Server->field4_0x74 != 0) {
        unit->RemoveFromMap();
    }
}

// 544685
void Shop::sub_544685(Unit* unit) {
    this->shop_template->CloseInstance(static_cast<Humanoid*>(unit));
    if (g_Server->field4_0x74 != 0) {
        unit->ReturnToMap();
    }
    g_NetStru1_main.BroadcastTokenState(unit, nullptr, 0);
}

// 54471B
void Shop::ConfirmBuy(Unit* unit) {
    this->shop_template->ConfirmBuy(unit);
}

// 544737
void Shop::ConfirmSell(Unit* unit) {
    this->shop_template->ConfirmSell(unit);
}

// 544777
void Shop::ReturnItems(Unit* unit) {
    this->shop_template->ReturnConsignedItems(unit);
}

// 544793
void Shop::SellItemFor(Humanoid* humanoid, int amount, Item* item) {
    this->shop_template->SellItemFor(humanoid, amount, item);
}
