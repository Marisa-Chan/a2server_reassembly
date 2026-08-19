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
void Shop::VMethod1() {
    this->shop_template->sub_547DD4();
}

// 5446C7
Item* Shop::sub_5446C7(Humanoid* humanoid, int16_t src_slot, int32_t count) {
    return this->shop_template->sub_547C5A(humanoid, src_slot, count);
}

// 5446EB
void Shop::sub_5446EB(Humanoid* unit, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count) {
    this->shop_template->sub_547CB9(unit, op_type, src_slot, dst_type, dst_word, count);
}

// 54463F
void Shop::sub_54463F() {
    this->shop_template->sub_5474D8();
}

// 544655
void Shop::sub_544655(Unit* unit) {
    this->shop_template->sub_547644(unit);
    if (g_Server->field4_0x74 != 0) {
        unit->sub_52C163();
    }
}

// 544685
void Shop::sub_544685(Unit* unit) {
    this->shop_template->sub_5479C6(static_cast<Humanoid*>(unit));
    if (g_Server->field4_0x74 != 0) {
        unit->sub_52C409();
    }
    g_NetStru1_main.sub_51AC77(unit, nullptr, 0);
}

// 54471B
void Shop::sub_54471B(Unit* unit) {
    this->shop_template->sub_547B7B(unit);
}

// 544737
void Shop::sub_544737(Unit* unit) {
    this->shop_template->sub_547BBC(unit);
}

// 544777
void Shop::sub_544777(Unit* unit) {
    this->shop_template->sub_547D7F(unit);
}

// 544793
void Shop::sub_544793(Humanoid* humanoid, int amount, Item* item) {
    this->shop_template->sub_547BFD(humanoid, amount, item);
}
