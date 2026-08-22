#ifndef SHOP_ASSORTMENT_H
#define SHOP_ASSORTMENT_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


struct Inventory;
class Item;
class WorldEquip;
class MagicItem;


// Parameters passed to ShopAssortment::GenerateAssortment.
struct AssortGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    int32_t flags;
};
ASSERT_SIZE(AssortGenParams, 0x14);


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

    void sub_54C08A(); // 54C08A — reset fields (still in ASM)
    void sub_54BC09(); // 54BC09 — ClearItemsArray: delete all items (still in ASM)
    void sub_54ACCB(CArray<WorldEquip>* items, uint32_t flags); // 54ACCB — AddItemsFlags
    void sub_54AF21(CArray<MagicItem>* items); // 54AF21 — AddMagicItems
    void sub_54A420(CArray<WorldEquip>* items, int32_t shape, int32_t mat, int32_t item_type, int32_t param_5); // 54A420 — AddItems
    Inventory* ArrangeShelfs(int32_t max_count, int32_t max_same, int32_t min_cost, int32_t max_cost, CArray<Item*>* result); // 54D423

    int32_t sub_54EA76(Item* item); // 54EA76 — add magic to item
};
ASSERT_SIZE(ShopAssortment, 0x58);


#endif
