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
    virtual ~CMultiShopShelf(); // 544B0C

public:
    CMultiShopShelf(); // 544A9D

public:
    uint32_t shelf_id;
    CArray<Item*> items;

    void ClearItems(); // 545865 (client: 52b591)
    void RemoveItemAt(int32_t index); // 54546E - remove item at index
    Item* PickItem(int32_t slot, int32_t count); // 5453E8 - pick item from shelf
    int32_t AddItem(Item** item_ptr); // 544BCE - add item to shelf. (client: 52b1ed)
    void ReleaseItems(); // 545528 - decrease refcount of all items and remove them from the shelf
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

    void NotifyOtherInstances(CMultiShopInstance* inst, int shelf_id); // 546F18
    CMultiShopInstance* CreateInstance(Unit* unit); // 546F8D
    int FindInstanceIndex(Unit* unit); // 54740C - find shop instance index for unit's owner
    void ClearShelves(); // 54749F - clear all shelf items (client: 52c428)
    Item* BuyItem(Humanoid* humanoid, int16_t src_slot, int32_t count); // 547C5A (client: 52c84a)
    void HandleItemOperation(Humanoid* humanoid, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count); // 547CB9 (client: 52c8a9)
    void RefreshTick(); // 547DD4
    int TryRefreshAssortment(); // 547E91 - refresh assortment if no live instances (client: 52ca6a)
    CMultiShopInstance* FindInstanceFor(Humanoid* humanoid); // 547468 - find shop instance for player's unit
    void CloseInstance(Humanoid* humanoid); // 5479C6
    void GenerateAssortment(); // 5474D8
    void OpenShopFor(Unit* unit); // 547644
    void ConfirmBuy(Unit* unit); // 547B7B
    void ConfirmSell(Unit* unit); // 547BBC
    void SellItemFor(Unit* unit, int amount, Item* item); // 547BFD (client: 52c7ed)
    void ReturnConsignedItems(Unit* unit); // 547D7F (client: 52c96f)

public:
    uint32_t active_instances;
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
    int StowItemInShelf(Item* item, int dst_slot); // 5462C8 (client: 52ba1d)
    void SendShelfState(int shelf_id); // 5464B6
    void MoveItems(uint8_t src_type, int16_t src_slot, uint8_t dst_type, int16_t dst_slot, int32_t count); // 546577
    void SellItemFor(int amount, Item* item); // 546537 - add item to inventory
    Item* PickItem(int item_number, int count); // 546557 - pick item from inventory
    void ReturnConsignedItems(); // 54668B
    Unit* SetUnit(Unit* unit); // 546857 (client: 52be85)
};
ASSERT_OFFSET(CMultiShopInstance, unit, 0x74);
ASSERT_OFFSET(CMultiShopInstance, shop_template, 0x9c);
ASSERT_SIZE(CMultiShopInstance, 0xa0);


#endif
