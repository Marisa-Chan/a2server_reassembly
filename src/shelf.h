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


class CMultiShopShelf : public CObject {
public:
    DECLARE_SERIAL(CMultiShopShelf); // Runtime class definition at 637270.

public: // VTable at 60f468
    virtual ~CMultiShopShelf(); // 544B0C

public:
    CMultiShopShelf(); // 544A9D

public:
    uint32_t shelf_id;
    CArray<Item*> items;

    void ClearItems(); // 545865
};
ASSERT_SIZE(CMultiShopShelf, 0x1c);

class CMultiShopInstance;

class CMultiShopTemplate : public CObject {
public:
    DECLARE_SERIAL(CMultiShopTemplate); // Runtime class at stru_6372A0

public: // VTable at 60f468
    virtual ~CMultiShopTemplate(); // 546DA4

public:
    CMultiShopTemplate(); // 54687B
    CMultiShopTemplate(Shop* shop); // 546C72

    void sub_546F18(CMultiShopInstance* inst, int param); // 546F18
    Item* sub_547C5A(Humanoid* humanoid, int16_t src_slot, int32_t count); // 547C5A
    void sub_547DD4(); // 547DD4
    CMultiShopInstance* sub_547468(Humanoid* humanoid); // 547468 - find shop instance for player's unit
    void sub_5479C6(Humanoid* humanoid); // 5479C6

public:
    uint32_t field_0x4;
    int32_t refresh_assortment_counter;
    std::array<CMultiShopShelf, 4> shelves;
    CArray<CMultiShopInstance*> shop_instances;
    int32_t field_0x90;
    Shop* shop;
};
ASSERT_OFFSET(CMultiShopTemplate, shop_instances, 0x7c);
ASSERT_SIZE(CMultiShopTemplate, 0x98);

class CMultiShopInstance : public CObject {
public:
    DECLARE_SERIAL(CMultiShopInstance); // Runtime class at 637288

public:
    virtual ~CMultiShopInstance(); // 545C8C

public:
    CMultiShopInstance(); // 545BD7

public:
    std::array<CMultiShopShelf, 4> shelves;
    Unit* unit;
    Inventory inventory;
    CMultiShopTemplate* shop_template;

    void Sell(); // 545D5E
    void Buy(); // 546027
    int sub_5462C8(Item* item, int arg1); // 5462C8
    void sub_5464B6(int param); // 5464B6
};
ASSERT_OFFSET(CMultiShopInstance, unit, 0x74);
ASSERT_OFFSET(CMultiShopInstance, shop_template, 0x9c);
ASSERT_SIZE(CMultiShopInstance, 0xa0);


#endif
