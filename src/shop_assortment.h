#ifndef SHOP_ASSORTMENT_H
#define SHOP_ASSORTMENT_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class Item;


struct ShopAssortment {
    CArray<double> arr;
    uint8_t gap_0x14[4];
    double field_0x18;
    CArray<Item*> items;
    int32_t min_cost;
    int32_t max_cost;
    uint8_t gap_0x3c[4];
    double field_0x40;
    uint8_t gap_0x48[12];
    uint32_t flags;
};
ASSERT_SIZE(ShopAssortment, 0x58);


#endif
