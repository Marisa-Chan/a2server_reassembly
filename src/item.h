#ifndef ITEM_H
#define ITEM_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "table.h"
#include "token.h"
#include "protections.h"
#include "unit_to_hit.h"

class Spell;
class Unit;

class Item : public Token {
public:
    DECLARE_SERIAL(Item);
public:
    // GetRuntimeClass
    virtual ~Item();
    virtual void Serialize(CArchive& ar) override;
    // CObject::AssertValid(void)
    // CObject::Dump(CDumpContext_&)
    virtual void VMethod1() override;
    virtual void VMethod2() override;
    // Token::VMethod3
    // Token::VMethod4
    // Token::VMethod5
    virtual int32_t VMethod6() override;
    // Token::VMethod7
    // Token::VMethod8
    // Token::VMethod9
    virtual Item* VMethod10(Unit*);
    virtual void VMethod11(Unit*);
    virtual Item* TakeOne();
    virtual Item* VMethod13();
    virtual int32_t VMethod14();
    virtual int32_t VMethod15();
    virtual int32_t VMethod16();
    virtual void VMethod17(void*, void*);

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
ASSERT_SIZE(Item, 0x58);

class Armor : public Item {
public:
    uint8_t slot;
    uint8_t gap_0x59;
    Protections protections;
};
ASSERT_OFFSET(Armor, protections, 0x5a);
ASSERT_SIZE(Armor, 0x70);

class Shield : public Item {
public:
    Protections protections;
    uint8_t gap_0x6e[2];
};
ASSERT_SIZE(Shield, 0x70);

class Weapon : public Item {
public:
    uint8_t range;
    uint8_t gap_0x59;
    UnitToHit hit_values;
    Protections protections;
    Spell* imbued_spell;
};
ASSERT_OFFSET(Weapon, hit_values, 0x5a);
ASSERT_SIZE(Weapon, 0x8c);

#endif
