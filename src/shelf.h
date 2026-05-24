#ifndef SHELF_H
#define SHELF_H


#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "inventory.h"
#include "mfc_templ.h"


class Humanoid;
class Item;
class Shop;
class Unit;


class MultiShopShelf : public CObject {
public:
    DECLARE_SERIAL(MultiShopShelf); // Runtime class definition at 637270.

public: // VTable at 60f468
    virtual ~MultiShopShelf(); // 544B0C

public:
    MultiShopShelf(); // 544A9D

public:
    uint32_t shelf_id;
    CArray<Item*> items;

    void ClearItems(); // 545865
};
ASSERT_SIZE(MultiShopShelf, 0x1c);

class MultiShopInstance;

class MultiShopTemplate : public CObject {
public:
    DECLARE_SERIAL(MultiShopTemplate); // Runtime class at stru_6372A0

public: // VTable at 60f468
    virtual ~MultiShopTemplate(); // 546DA4

public:
    MultiShopTemplate(); // 54687B
    MultiShopTemplate(Shop* shop); // 546C72

    void sub_546F18(MultiShopInstance* inst, int param); // 546F18
    Item* sub_547C5A(Humanoid* humanoid, int16_t src_slot, int32_t count); // 547C5A
    void sub_547DD4(); // 547DD4

public:
    uint32_t field_0x4;
    int32_t refresh_assortment_counter;
    std::array<MultiShopShelf, 4> shelves;
    CArray<MultiShopInstance*> shop_instances;
    int32_t field_0x90;
    Shop* shop;
};
ASSERT_OFFSET(MultiShopTemplate, shop_instances, 0x7c);
ASSERT_SIZE(MultiShopTemplate, 0x98);

class MultiShopInstance : public CObject {
public:
    DECLARE_SERIAL(MultiShopInstance); // Runtime class at 637288

public:
    virtual ~MultiShopInstance(); // 545C8C

public:
    MultiShopInstance(); // 545BD7

public:
    std::array<MultiShopShelf, 4> shelves;
    Unit* unit;
    Inventory inventory;
    MultiShopTemplate* shop_template;

    void Sell(); // 545D5E
    void Buy(); // 546027
    int sub_5462C8(Item* item, int arg1); // 5462C8
    void sub_5464B6(int param); // 5464B6
};
ASSERT_OFFSET(MultiShopInstance, unit, 0x74);
ASSERT_OFFSET(MultiShopInstance, shop_template, 0x9c);
ASSERT_SIZE(MultiShopInstance, 0xa0);


#endif
