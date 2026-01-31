#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "building.h"


class MultiShopTemplate;


struct AssortmentGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    uint32_t flags;
};
static_assert(sizeof(AssortmentGenParams) == 0x14, "AssortmentGenParams size mismatch");

class Shop : public Building {
public:
    MultiShopTemplate* shop_template;
    AssortmentGenParams gen_params[4];
    uint32_t field_0xc0;
};
static_assert(offsetof(Shop, gen_params) == 0x70, "Shop::gen_params offset mismatch");
static_assert(sizeof(Shop) == 0xc4, "Shop size mismatch");


#endif
