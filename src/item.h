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
class PacketUnitStateVec;

extern "C" CMapWordToPtr unk_660D28;
extern "C" CMapWordToPtr unk_660DA8;

extern "C" Item* __cdecl sub_4F499B(uint8_t** packet_data);
void LoadItemNames(); //475988

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
    virtual int32_t VMethod14(int, int);
    virtual int32_t VMethod15();
    virtual int32_t VMethod16();
    virtual void VMethod17(PacketUnitStateVec* pkt, uint8_t* slot);

public:
    Item(); // sub_547F80: default constructor (blank item)
    Item(const CString& name); // sub_5480E3: construct item from name string
    Item(uint8_t type, uint8_t subtype); // sub_54800E: construct item from type and subtype
    Item(const Item* src); // sub_54842A: copy constructor
    void sub_548FAA(Effect* effect); // Add effect to item
    int sub_548F07(); // IsArtifact: returns 1 if the item has magic and price 2. 548f07.
    int sub_548F6A(); // Returns 1 if this item template can be given via #create cheat
    void sub_548F3F(const CString& str); // Apply special property string to item effects, then call VMethod15
    void sub_54A0BE(); // Recalculate item properties from template data

    Effect* sub_548E4E(); // Find the cast-spell Effect on this item (returns nullptr if none)
    int IsSimilar(Item* other); // 548860. Is this item similar to another? Same non-magic item or same base with same magic.

    void StoreToPacket(PacketUnitStateVec* pkt, int arg); // 549afb
    void WriteEffects(PacketUnitStateVec* pkt, uint8_t* slot); // 549F2C

public:
    WorldEquip* world_equip;
    uint16_t item_id;
    uint16_t count;
    uint8_t item_type;
    uint8_t shape_id;
    uint8_t material_id;
    uint8_t field_0x47;
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
    static AFX_DATA CRuntimeClass classArmor; // defined in Main.asm

public:
    Armor(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // sub_54F5AB
    Armor(const Armor* src); // sub_54F9ED: copy constructor
    void LoadInfo(); // sub_54F634

public:
    uint8_t slot;
    uint8_t gap_0x59;
    Protections protections;
};
ASSERT_OFFSET(Armor, protections, 0x5a);
ASSERT_SIZE(Armor, 0x70);

class Shield : public Item {
public:
    static AFX_DATA CRuntimeClass classShield; // defined in Main.asm

public:
    Shield(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // sub_5500FA
    Shield(const Shield* src); // sub_550467: copy constructor

public:
    Protections protections;
    uint8_t gap_0x6e[2];
};
ASSERT_SIZE(Shield, 0x70);

class Weapon : public Item {
public:
    static AFX_DATA CRuntimeClass classWeapon; // defined in Main.asm

public:
    Weapon(const CString& name); // sub_550929: construct weapon from name string
    Weapon(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // sub_550B8F
    Weapon(const Weapon* src); // sub_5511EE: copy constructor
    void LoadEquipInfo(WorldEquip* params); // sub_550E26

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
