#ifndef SHELF_H
#define SHELF_H


#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "inventory.h"
#include "mfc_templ.h"


class Item;
class Shop;
class Unit;


class MultiShopShelf : public CObject {
public: // VTable at 0060f438
    // virtual GetRuntimeClass() const;
    // virtual ~MultiShopShelf();
    // virtual void Serialize(CArchive& ar);
    // virtual void AssertValid() const;
    // virtual void Dump(CDumpContext& dc) const;

public:
    uint32_t shelf_id;
    CArray<Item*> items;
};
ASSERT_SIZE(MultiShopShelf, 0x1c);

class MultiShopInstance;

class MultiShopTemplate : public CObject {
public: // VTable at 0060f468
    // virtual GetRuntimeClass() const;
    // virtual ~MultiShopTemplate();
    // virtual void Serialize(CArchive& ar);
    // virtual void AssertValid() const;
    // virtual void Dump(CDumpContext& dc) const;

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
public: // VTable at 0060f450
    // virtual GetRuntimeClass() const;
    // virtual ~MultiShopTemplate();
    // virtual void Serialize(CArchive& ar);
    // virtual void AssertValid() const;
    // virtual void Dump(CDumpContext& dc) const;

public:
    std::array<MultiShopShelf, 4> shelves;
    Unit* unit;
    Inventory inventory;
    MultiShopTemplate* shop_template;
};
ASSERT_OFFSET(MultiShopInstance, unit, 0x74);
ASSERT_OFFSET(MultiShopInstance, shop_template, 0x9c);
ASSERT_SIZE(MultiShopInstance, 0xa0);


#endif
