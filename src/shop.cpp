#include "shop.h"

#include "shelf.h"


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
