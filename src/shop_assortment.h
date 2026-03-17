#ifndef SHOP_ASSORTMENT_H
#define SHOP_ASSORTMENT_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class Item;


// Parameters passed to ShopAssortment::GenerateAssortment.
struct AssortGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    int32_t flags;
};


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

    ShopAssortment();   // sub_54A13B (??1CWinThread@@UAE@XZ_37 in asm listing)
    ~ShopAssortment();  // sub_54A2DD
    void GenerateAssortment(AssortGenParams* params);  // sub_54BF6B
    void ArrangeShelfs(int max_count, int max_same, int min_cost, int max_cost, CArray<Item*>* result); // sub_54D423
};
ASSERT_SIZE(ShopAssortment, 0x58);


#endif
