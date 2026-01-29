#ifndef ITEM_H
#define ITEM_H

#include <cstdint>

#include "asm_mfc.h"
#include "table.h"
#include "token.h"
#include "unit.h"

class Spell;

class Item : public Token {
public:
    // GetRuntimeClass
    // Item::~Item
    // Item::Serialize
    // CObject::AssertValid(void)
    // CObject::Dump(CDumpContext_&)
    // Item::VMethod1
    // Item::VMethod2
    // Token::VMethod3
    // Token::VMethod4
    // Token::VMethod5
    // Item::VMethod6
    // Token::VMethod7
    // Token::VMethod8
    // Token::VMethod9
    // Item::VMethod10
    // Item::VMethod11
    // Item::TakeOne
    // Item::VMethod13
    // Item::VMethod14
    // Item::VMethod15
    // Item::VMethod16
    // Item::VMethod17

public:
    WorldEquip* world_equip;
    uint16_t item_id;
    uint16_t count;
    uint8_t item_type;
    uint8_t shape_id;
    uint8_t material_id;
    uint8_t gap_0x47;
    int16_t magic_volume;
    int16_t weight;
    uint8_t field10_0x4c;
    uint8_t field11_0x4d;
    uint8_t field12_0x4e;
    uint8_t field13_0x4f;
    uint32_t field14_0x50;
    uint32_t field15_0x54;
};
static_assert(sizeof(Item) == 0x58, "Item size mismatch");

class Armor : public Item {
public:
    uint8_t slot;
    uint8_t gap_0x59;
    Protections protections;
};
static_assert(offsetof(Armor, protections) == 0x5a, "Armor::protections offset mismatch");
static_assert(sizeof(Armor) == 0x70, "Armor size mismatch");

class Shield : public Item {
public:
    Protections protections;
    uint8_t gap_0x6e[2];
};
static_assert(sizeof(Shield) == 0x70, "Shield size mismatch");

class Weapon : public Item {
public:
    uint8_t range;
    uint8_t gap_0x59;
    UnitToHit hit_values;
    Protections protections;
    Spell* imbued_spell;
};
static_assert(offsetof(Weapon, hit_values) == 0x5a, "Weapon::hit_values offset mismatch");
static_assert(sizeof(Weapon) == 0x8c, "Weapon size mismatch");

#endif
