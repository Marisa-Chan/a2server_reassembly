#ifndef SHOP_ASSORTMENT_H
#define SHOP_ASSORTMENT_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


struct Inventory;
class Item;
class WorldEquip;
class MagicItem;


// Bit-packed shop/inn/loot item generation mask.
struct AssortFlags {
    enum : uint32_t {
        MATERIAL_MASK          = 0x00007FFFu,
        SHAPE_SHIFT            = 15,
        SHAPE_MASK             = 0x003F8000u,
        ALLOW_WARRIOR_WEAPONS  = 0x00400000u,
        ALLOW_SHIELDS          = 0x00800000u,
        ALLOW_ARMORS           = 0x01000000u,
        UNUSED_TYPE_25         = 0x02000000u,
        ALLOW_CONSUMABLES      = 0x04000000u,
        ALLOW_MAGE_WEAPONS     = 0x08000000u,
        TYPE_MASK              = 0x0FC00000u,
        EQUIP_TYPES_CLEAR_MASK = 0xF23FFFFFu,
        MIXED_MAGIC_50PCT      = 0x10000000u,
        GENERATE_MAGIC         = 0x20000000u,

        ALL_SHOP_DEFAULT       = 0x2FFFFFFFu,
        INN_NON_MAGIC_EQUIP    = 0x1B7FFFFFu,
        INN_MAGIC_EQUIP        = 0x2BFFFFFFu,
    };

    union {
        uint32_t raw;
        struct {
            uint32_t material_mask         : 15;
            uint32_t shape_mask            : 7;
            uint32_t allow_warrior_weapons : 1;
            uint32_t allow_shields         : 1;
            uint32_t allow_armors          : 1;
            uint32_t unused_type_25        : 1;
            uint32_t allow_consumables     : 1;
            uint32_t allow_mage_weapons    : 1;
            uint32_t mixed_magic_50pct     : 1;
            uint32_t generate_magic        : 1;
            uint32_t reserved              : 2;
        };
    };

    constexpr AssortFlags(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
    AssortFlags& operator|=(uint32_t v) { this->raw |= v; return *this; }
    AssortFlags& operator&=(uint32_t v) { this->raw &= v; return *this; }
};
ASSERT_SIZE(AssortFlags, 4);


// Parameters passed to ShopAssortment::GenerateAssortment.
struct AssortGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    AssortFlags flags;
};
ASSERT_SIZE(AssortGenParams, 0x14);


struct ShopAssortment {
    enum AddCategory : int32_t {
        ADD_ARMOR          = 1,
        ADD_WARRIOR_WEAPON = 2,
        ADD_SHIELD         = 7,
        ADD_MAGE_WEAPON    = 8,
    };

    CArray<double> arr;
    uint8_t gap_0x14[4];
    double edge_weight;
    CArray<Item*> items;
    int32_t min_cost;
    int32_t max_cost;
    uint8_t gap_0x3c[4];
    double gap_0x40;
    uint8_t gap_0x48[12];
    AssortFlags flags;

    ShopAssortment();   // 54A13B (??1CWinThread@@UAE@XZ_37 in asm listing) (client: 422960)
    ~ShopAssortment();  // 54A2DD (client: 422cd0)
    void GenerateAssortment(AssortGenParams* params);  // 54BF6B (client: 52f209)

    void Reset(); // 54C08A — reset fields (still in ASM)
    void ClearItems(); // 54BC09 (client: 52f180) — ClearItemsArray: delete all items (still in ASM)
    void AddItemsFlags(CArray<WorldEquip>* items, AssortFlags flags); // 54ACCB (client: 52e8ec) — AddItemsFlags
    void AddMagicItems(CArray<MagicItem>* items); // 54AF21 (client: 52eb53) — AddMagicItems
    void AddItems(CArray<WorldEquip>* items, int32_t shape, int32_t mat, int32_t item_type, int32_t is_magic); // 54A420 (client: 52e4fd) — AddItems
    Inventory* ArrangeShelfs(int32_t max_count, int32_t max_same, int32_t min_cost, int32_t max_cost, CArray<Item*>* result); // 54D423

    int32_t AddMagicToItem(Item* item); // 54EA76 — add magic to item
};
ASSERT_SIZE(ShopAssortment, 0x58);


#endif
