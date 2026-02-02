#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "building.h"
#include "assert_offset.h"


class MultiShopTemplate;


struct AssortmentGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    uint32_t flags;
};
ASSERT_SIZE(AssortmentGenParams, 0x14);

class Shop : public Building {
public:
    MultiShopTemplate* shop_template;
    AssortmentGenParams gen_params[4];
    uint32_t field_0xc0;
};
ASSERT_OFFSET(Shop, gen_params, 0x70);
ASSERT_SIZE(Shop, 0xc4);


#endif
