#ifndef SHOP_H
#define SHOP_H

#include <cstdint>

#include "building.h"
#include "assert_offset.h"


class MultiShopTemplate;
class Humanoid;
class Item;
class Unit;


struct AssortmentGenParams {
    int32_t min_cost;
    int32_t max_cost;
    int32_t max_count;
    int32_t max_same_count;
    uint32_t flags;
};
ASSERT_SIZE(AssortmentGenParams, 0x14);

class Shop : public Building {
public:
    MultiShopTemplate* shop_template;
    AssortmentGenParams gen_params[4];
    uint32_t field_0xc0;

public:
    void sub_544793(Humanoid* humanoid, int amount, Item* item);
    Item* sub_5446C7(Humanoid* unit, int16_t src_slot, int32_t count);
    void sub_5446EB(Humanoid* unit, uint8_t op_type, int16_t src_slot, uint8_t dst_type, int16_t dst_word, int32_t count);
    void sub_544655(Unit* unit); // Mine: attach unit to this building
    void sub_54463F();

    void sub_54471B(Unit* unit);
    void sub_544737(Unit* unit);
    void sub_544777(Unit* unit);
    void sub_544685(Unit* unit);
};
ASSERT_OFFSET(Shop, gen_params, 0x70);
ASSERT_SIZE(Shop, 0xc4);


#endif
