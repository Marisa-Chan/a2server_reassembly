#ifndef ITEM_H
#define ITEM_H

#include <cstdint>

#include <afx.h>
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

extern void* dword_65FBB8; // 65fbb8 - item name packet area

Item* __cdecl ReadItem(uint8_t** packet_data);
void LoadItemNames(); //475988 (client: 4796d8)
void ParseEffectString(const CString& effect_str, CList<Effect*>* effects); // 540DAA (client: 52845f): parse effect string into effects list
int32_t TotalEffectPrice(Item* item); // 54F176: total EffectPrice() of all effects

class Item : public Token {
public:
    DECLARE_SERIAL(Item); // Runtime class definition at 6372b8.

public: // VTable at 60f498.
    virtual ~Item();  // client: 52cfb7
    virtual void Serialize(CArchive& ar) override;  // client: 53737c
    virtual void RegenTick() override;  // client: 52d541
    virtual void ProcessEffects() override;  // client: 52d536
    virtual int32_t IsItem() override;  // client: 54e0d0
    virtual Item* Equip(Unit*); // Equip. (client: 52d335)
    virtual void Unequip(Unit*); // Unequip. (client: 52d476)
    virtual Item* TakeOne();  // client: 52d55b
    virtual Item* Clone();  // client: 54e050
    virtual int32_t VMethod14(int, int);  // client: 52d54c
    virtual int32_t CalcValue();  // client: 52d131
    virtual int32_t IsSimpleItem();  // client: 52d1e4
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot);  // client: 52e1f8

public:
    Item(); // 547F80: default constructor (blank item) (client: 52cb56)
    Item(const CString& name); // 5480E3: construct item from name string (client: 52ccb9)
    Item(uint8_t type, uint8_t subtype); // 54800E: construct item from type and subtype (client: 52cbe4)
    Item(const Item* src); // 54842A: copy constructor (client: 52cfd3)
    void InitFromTemplate(uint8_t slot); // 5482AB (client: 52ce60): init item fields from magic_items table
    void AddEffect(Effect* effect); // Add effect to item
    int IsArtifact(); // IsArtifact: returns 1 if the item has magic and price 2. 548f07.
    int IsCheatCreatable(); // Returns 1 if this item template can be given via #create cheat (client: 52d660)
    void ApplyPropertyString(const CString& str); // Apply special property string to item effects, then call VMethod15 (client: 52d635)
    void RecalcFromTemplate(); // Recalculate item properties from template data

    Effect* FindCastSpellEffect(); // Find the cast-spell Effect on this item (returns nullptr if none) (client: 52d5e7)
    int IsSimilar(Item* other); // 548860 (client: 54e170). Is this item similar to another? Same non-magic item or same base with same magic.

    void StoreToPacket(PacketUnitStateVec* pkt, int with_shelf_id); // 549afb (client: 52dee1)
    void WriteEffects(PacketUnitStateVec* pkt, uint8_t* slot); // 549F2C (client: 52e29e)
    void ApplyEffects(Unit* unit); // 548C2E
    void RemoveEffects(Unit* unit); // 548CE4 (client: 52d4eb)

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
    uint8_t value1;
    int16_t magic_volume;
    int16_t weight;
    uint8_t shelf_refcount;
    uint8_t shelf_id; // Shelf ID?
    uint8_t gap_0x4e;
    uint8_t gap_0x4f;
    uint32_t special_flag;
    uint32_t upgraded_at_inn;
};
ASSERT_SIZE(Item, 0x58);

class Armor : public Item {
public:
    DECLARE_SERIAL(Armor); // Runtime class definition at 6372d0.

public: // VTable at 60f508.
    virtual ~Armor() override; // 54F9D1 (client: 531698)
    virtual void Serialize(CArchive& ar) override; // 55CF2B (client: 5377f2)
    virtual Item* Equip(Unit* unit) override; // 54FA6E (client: 531735)
    virtual void Unequip(Unit* unit) override; // 54FB7E (client: 531843)
    virtual Item* TakeOne() override; // 54FC19 (client: 5318dc)
    virtual Item* Clone() override; // 57D5B0 (client: 54e710)
    virtual int32_t CalcValue() override; // 54F84D (client: 531584)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 54FCA8 (client: 531968)

public:
    Armor(); // 54F2E9: default constructor (client: 53109a)
    Armor(const CString& name); // 54F361: construct from name string (client: 531112)
    Armor(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 54F5AB (client: 5312f4)
    Armor(const Armor* src); // 54F9ED: copy constructor (client: 5316b4)
    void LoadInfo(); // 54F634 (client: 53137d)

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
    virtual ~Shield() override; // 55044b (client: 532070)
    virtual void Serialize(CArchive& ar) override; // 55AF5E (client: 536e3d)
    virtual Item* Equip(Unit*) override; // 55056B (client: 53218d)
    virtual void Unequip(Unit*) override; // 550686 (client: 5322a3)
    virtual Item* TakeOne() override; // 5504DC (client: 532101)
    virtual Item* Clone() override; // 57D660 (client: 54e7c0)
    virtual int32_t CalcValue() override; // 5502C7 (client: 531f5c)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 5506FC (client: 532317)

public:
    Shield(); // 54FE1B (client: 531b45): default constructor
    Shield(const CString& name); // 54FE8E (client: 531b45): construct from name string
    Shield(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 5500FA (client: 531da6)
    Shield(const Shield* src); // 550467 (client: 53208c): copy constructor

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
    virtual ~Weapon() override; // 551159 (client: 480e53)
    virtual void Serialize(CArchive& ar) override; // 55929F (client: 536298)
    virtual Item* Equip(Unit* unit) override; // 5512E7 (client: 532dee)
    virtual void Unequip(Unit* unit) override; // 5515B9 (client: 5330b4)
    virtual Item* TakeOne() override; // 5517C3 (client: 532101)
    virtual Item* Clone() override; // 57D710 (client: 54e870)
    virtual int32_t CalcValue() override; // 550FA6 (client: 532b26)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 551855 (client: 53334b)

public:
    Weapon(); // 550872 (client: 53252a): default constructor
    Weapon(const CString& name); // 550929 (client: 5327bd): construct weapon from name string
    Weapon(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 550B8F (client: 532729)
    Weapon(const Weapon* src); // 5511EE (client: 532cfa): copy constructor
    void LoadEquipInfo(WorldEquip* params); // 550E26
    void LoadInfo(); // 550C23 (client: 4fc79a)

private:
    void SetImbuedSpell(uint8_t spell_id); // 551B12 (client: 5335f5)
    void InitImbuedSpell(); // 551BDB (client: 5336a8)

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
