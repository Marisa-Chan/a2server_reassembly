#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "assert_offset.h"
#include "building.h"
#include "shop_assortment.h"


class CMultiShopTemplate;
class Humanoid;
class Item;
class Unit;

class Shop : public Building {
public:
    DECLARE_SERIAL(Shop); // Runtime class definition at 637258.

public: // VTable at 60f3f8.
    virtual ~Shop(); // 544564 (client: 52ac81)
    virtual void Serialize(CArchive& ar) override; // 5447b7 (client: 52aecc)
    virtual void RegenTick() override; // 544629 (client: 52ad3e)

public:
    Shop(); // 5442ca
    Shop(const CString& name); // 5443ef (client: 52ab14)
    Shop(uint8_t type_id, TokenPos* pos); // 544495 (client: 52abb7)

    void SellItemFor(Humanoid* humanoid, int amount, Item* item);  // client: 52aea8
    Item* BuyItem(Humanoid* unit, int16_t src_slot, int32_t count);  // client: 52addc
    void HandleItemOperation(Humanoid* unit, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count);  // client: 52ae00
    void AttachUnit(Unit* unit); // Mine: attach unit to this building (client: 52ad6a)
    void GenerateAssortment();  // client: 52ad54

    void ConfirmBuy(Unit* unit);  // client: 52ae30
    void ConfirmSell(Unit* unit);  // client: 52ae4c
    void ReturnItems(Unit* unit);  // client: 52ae8c
    void CloseShop(Unit* unit);  // client: 52ad9a

public:
    CMultiShopTemplate* shop_template;
    AssortGenParams gen_params[4];
    uint32_t money;
};
ASSERT_OFFSET(Shop, gen_params, 0x70);
ASSERT_SIZE(Shop, 0xc4);


#endif
