#ifndef SHELF_H
#define SHELF_H


#include <array>
#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include "inventory.h"
#include <afxtempl.h>


class Humanoid;
class Item;
class Shop;
class Unit;


class CMultiShopShelf : public CObject {
public:
    DECLARE_SERIAL(CMultiShopShelf); // Runtime class definition at 637270.

public: // VTable at 60f468
    virtual ~CMultiShopShelf(); // 544b0c (client: 52b199)

public:
    CMultiShopShelf(); // 544a9d (client: 52b138)

public:
    uint32_t shelf_id;
    CArray<Item*> items;

    void ClearItems(); // 545865 (client: 52b591)
    void RemoveItemAt(int32_t index); // 54546e (client: 52b4fd) - remove item at index
    Item* PickItem(int32_t slot, int32_t count); // 5453e8 (client: 52b473) - pick item from shelf
    int32_t AddItem(Item** item_ptr); // 544bce (client: 52b1ed) - add item to shelf.
    void ReleaseItems(); // 545528 (client: 52b531) - decrease refcount of all items and remove them from the shelf
};
ASSERT_SIZE(CMultiShopShelf, 0x1c);

class CMultiShopInstance;

class CMultiShopTemplate : public CObject {
public:
    DECLARE_SERIAL(CMultiShopTemplate); // Runtime class at 6372A0

public: // VTable at 60f468
    virtual ~CMultiShopTemplate(); // 546da4 (client: 52c05e)

public:
    CMultiShopTemplate(); // 54687b (client: 52bea9)
    CMultiShopTemplate(Shop* shop); // 546c72 (client: 52bf83)

    void NotifyOtherInstances(CMultiShopInstance* inst, int shelf_id); // 546f18 (client: 52c16a)
    CMultiShopInstance* CreateInstance(Unit* unit); // 546f8d (client: 52c1e1)
    int FindInstanceIndex(Unit* unit); // 54740c (client: 52c395) - find shop instance index for unit's owner
    void ClearShelves(); // 54749f (client: 52c428) - clear all shelf items
    Item* BuyItem(Humanoid* humanoid, int16_t src_slot, int32_t count); // 547c5a (client: 52c84a)
    void HandleItemOperation(Humanoid* humanoid, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count); // 547cb9 (client: 52c8a9)
    void RefreshTick(); // 547dd4 (client: 52c9c4)
    int TryRefreshAssortment(); // 547e91 (client: 52ca6a) - refresh assortment if no live instances
    CMultiShopInstance* FindInstanceFor(Humanoid* humanoid); // 547468 (client: 52c3ef) - find shop instance for player's unit
    void CloseInstance(Humanoid* humanoid); // 5479c6 (client: 52c630)
    void GenerateAssortment(); // 5474d8 (client: 52c461)
    void OpenShopFor(Unit* unit); // 547644 (client: 52c5bb)
    void ConfirmBuy(Unit* unit); // 547b7b (client: 52c76b)
    void ConfirmSell(Unit* unit); // 547bbc (client: 52c7ac)
    void SellItemFor(Unit* unit, int amount, Item* item); // 547bfd (client: 52c7ed)
    void ReturnConsignedItems(Unit* unit); // 547d7f (client: 52c96f)

public:
    uint32_t active_instances;
    int32_t refresh_assortment_counter;
    std::array<CMultiShopShelf, 4> shelves;
    CArray<CMultiShopInstance*> shop_instances;
    int32_t start_money;
    Shop* shop;
};
ASSERT_OFFSET(CMultiShopTemplate, shop_instances, 0x7c);
ASSERT_SIZE(CMultiShopTemplate, 0x98);

class CMultiShopInstance : public CObject {
public:
    DECLARE_SERIAL(CMultiShopInstance); // Runtime class at 637288

public:
    virtual ~CMultiShopInstance(); // 545c8c (client: 52b6e3)

public:
    CMultiShopInstance(); // 545bd7 (client: 52b62f)

public:
    std::array<CMultiShopShelf, 4> shelves;
    Unit* unit;
    Inventory inventory;
    CMultiShopTemplate* shop_template;

    void Sell(); // 545d5e (client: 52b7b4)
    void Buy(); // 546027 (client: 52b8fa)
    int StowItemInShelf(Item* item, int dst_slot); // 5462c8 (client: 52ba1d)
    void SendShelfState(int shelf_id); // 5464b6 (client: 52bc0b)
    void MoveItems(uint8_t src_type, int16_t src_slot, uint8_t dst_type, int16_t dst_slot, int32_t count); // 546577 (client: 52bca2)
    void SellItemFor(int amount, Item* item); // 546537 (client: 52bc62) - add item to inventory
    Item* PickItem(int item_number, int count); // 546557 (client: 52bc82) - pick item from inventory
    void ReturnConsignedItems(); // 54668b (client: 52bdaa)
    Unit* SetUnit(Unit* unit); // 546857 (client: 52be85)
};
ASSERT_OFFSET(CMultiShopInstance, unit, 0x74);
ASSERT_OFFSET(CMultiShopInstance, shop_template, 0x9c);
ASSERT_SIZE(CMultiShopInstance, 0xa0);


#endif
