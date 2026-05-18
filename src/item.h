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

extern CMapWordToPtr unk_660D28;
extern CMapWordToPtr unk_660DA8;

extern "C" Item* __cdecl sub_4F499B(uint8_t** packet_data);
void LoadItemNames(); //475988

class Item : public Token {
public:
    DECLARE_SERIAL(Item); // Runtime class definition at 6372b8.

public: // VTable at 60f498.
    virtual ~Item();
    virtual void Serialize(CArchive& ar) override;
    virtual void VMethod1() override;
    virtual void VMethod2() override;
    virtual int32_t VMethod6() override;
    virtual Item* VMethod10(Unit*); // Equip.
    virtual void VMethod11(Unit*); // Unequip.
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
    void InitFromTemplate(uint8_t slot); // 5482AB: init item fields from magic_items table
    void sub_548FAA(Effect* effect); // Add effect to item
    int sub_548F07(); // IsArtifact: returns 1 if the item has magic and price 2. 548f07.
    int sub_548F6A(); // Returns 1 if this item template can be given via #create cheat
    void sub_548F3F(const CString& str); // Apply special property string to item effects, then call VMethod15
    void sub_54A0BE(); // Recalculate item properties from template data

    Effect* sub_548E4E(); // Find the cast-spell Effect on this item (returns nullptr if none)
    int IsSimilar(Item* other); // 548860. Is this item similar to another? Same non-magic item or same base with same magic.

    void StoreToPacket(PacketUnitStateVec* pkt, int arg); // 549afb
    void WriteEffects(PacketUnitStateVec* pkt, uint8_t* slot); // 549F2C
    void ApplyEffects(Unit* unit); // 548C2E
    void RemoveEffects(Unit* unit); // 548CE4

public:
    enum ItemType : uint8_t {
        NONE = 0,
        EQUIPMENT = 1,
        WEAPON = 2,
        POTION = 3,
        SCROLL = 4,
        BOOK = 5
    };

public:
    WorldEquip* world_equip;
    uint16_t item_id;
    uint16_t count;
    ItemType item_type;
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
    DECLARE_SERIAL(Armor); // Runtime class definition at 6372d0.

public: // VTable at 60f508.
    virtual ~Armor() override; // 54F9D1
    virtual void Serialize(CArchive& ar) override; // 55CF2B
    virtual Item* VMethod10(Unit* unit) override; // 54FA6E
    virtual void VMethod11(Unit* unit) override; // 54FB7E
    virtual Item* TakeOne() override; // 54FC19
    virtual Item* VMethod13() override; // 57D5B0
    virtual int32_t VMethod15() override; // 54F84D
    virtual void VMethod17(PacketUnitStateVec* pkt, uint8_t* slot) override; // 54FCA8

public:
    Armor(); // 54F2E9: default constructor
    Armor(const CString& name); // 54F361: construct from name string
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
    DECLARE_SERIAL(Shield); // Runtime class definition at 6372e8.

public: // VTable at 60f560.
    virtual ~Shield() override; // 55044b
    virtual void Serialize(CArchive& ar) override; // 55AF5E
    virtual Item* VMethod10(Unit*) override; // 55056B
    virtual void VMethod11(Unit*) override; // 550686
    virtual Item* TakeOne() override; // 5504DC
    virtual Item* VMethod13() override; // 57D660
    virtual int32_t VMethod15() override; // 5502C7
    virtual void VMethod17(PacketUnitStateVec* pkt, uint8_t* slot) override; // 5506FC

public:
    Shield(); // 54FE1B: default constructor
    Shield(const CString& name); // 54FE8E: construct from name string
    Shield(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 5500FA
    Shield(const Shield* src); // 550467: copy constructor

    void LoadInfo(); // 550183

public:
    Protections protections;
    uint8_t gap_0x6e[2];
};
ASSERT_SIZE(Shield, 0x70);

class Weapon : public Item {
public:
    DECLARE_SERIAL(Weapon); // Runtime class definition at 637300.

public: // VTable at 60f5b8.
    virtual ~Weapon() override; // 551159
    virtual void Serialize(CArchive& ar) override; // 55929F
    virtual Item* VMethod10(Unit* unit) override; // 5512E7
    virtual void VMethod11(Unit* unit) override; // 5515B9
    virtual Item* TakeOne() override; // 5517C3
    virtual Item* VMethod13() override; // 57D710
    virtual int32_t VMethod15() override; // 550FA6
    virtual void VMethod17(PacketUnitStateVec* pkt, uint8_t* slot) override; // 551855

public:
    Weapon(); // 550872: default constructor
    Weapon(const CString& name); // 550929: construct weapon from name string
    Weapon(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 550B8F
    Weapon(const Weapon* src); // 5511EE: copy constructor
    void LoadEquipInfo(WorldEquip* params); // 550E26
    void LoadInfo(); // 550C23

private:
    void SetImbuedSpell(uint8_t spell_id); // 551B12
    void InitImbuedSpell(); // 551BDB

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
