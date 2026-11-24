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

extern CMapWordToPtr g_ItemNamePktMap;
extern CMapWordToPtr g_ItemNameMap;

extern void* g_ItemNamePktData; // 65fbb8 - item name packet area

Item* __cdecl ReadItem(uint8_t** packet_data); // 4f499b (client: 4f63b9)
void LoadItemNames(); // 475988 (client: 4796d8)
void ParseEffectString(const CString& effect_str, CList<Effect*>* effects); // 540daa (client: 52845f) - parse effect string into effects list
int32_t TotalEffectPrice(Item* item); // 54f176 (client: 530f95) - total EffectPrice() of all effects

namespace item_slot {
    enum : uint8_t {
        none       = 0,
        weapon     = 1,
        shield     = 2,
        unused_3   = 3,
        ring       = 4,
        amulet     = 5,
        helm       = 6,  // Warrior: helm, Mage: hat
        mail       = 7,  // Warrior: mail, Mage: robe
        cuirass    = 8,  // Warrior: cuirass, Mage: cloak
        bracers    = 9,
        gauntlets  = 10, // Warrior: gauntlets, Mage: gloves
        unused_11  = 11,
        boots      = 12, // Warrior: boots, Mage: shoes
        max_armor  = 12,
        count      = 13,
        extra_13   = 13, // 0x0D: accepted in GameDataRes::CreateItem, rejected in Armor::LoadInfo
        magic_item = 14, // 0x0E: books, scrolls, potions, quest items
    };
}

namespace material {
    enum : uint8_t {
        iron           = 0,
        bronze         = 1,
        steel          = 2,
        silver         = 3,
        gold           = 4,
        mithril        = 5,
        adamantium     = 6,
        meteoric       = 7,
        wood           = 8,
        magic_wood     = 9,
        leather        = 10,
        hard_leather   = 11,
        dragon_leather = 12,
        bone           = 13, // 0x0D: skipped in shop material_mask
        crystal        = 14,
        none           = 15,
    };
}

namespace item_shape {
    enum : uint8_t {
        common    = 0,
        uncommon  = 1,
        rare      = 2,
        very_rare = 3,
        elven     = 4,
        bad       = 5,
        good      = 6,
        count     = 7,
    };
}

struct ItemId {
    enum Sentinel : uint16_t {
        NONE               = 0x0000,
        FAME_HALL_SCROLL   = 0x0E4D, // 3661
        QUEST_TREASURE     = 0x0E53, // 3667
        SCRIPT_ITEM_OFFSET = 0x0E18, // 3608: added to scenario script item args
        GOLD_PSEUDO_MIN    = 0xFFF0,
        REWARD_ALLY        = 0xFFFD,
        REWARD_EXP         = 0xFFFE,
        REWARD_GOLD        = 0xFFFF,
        EMPTY_EQUIP_SLOT   = 0xFFFF,
    };

    enum MagicRange : uint8_t {
        BOOK_FIRST         = 1,
        BOOK_LAST          = 5,
        SCROLL_OFFSET      = 5,  // 6..34  (spell_id 1..29)
        SUPERSCROLL_OFFSET = 34, // 35..63 (spell_id 1..29)
    };

    union {
        uint16_t raw;
        struct {
            uint16_t equip_id    : 5; // [4:0]   1..31 in weapons/shields/armors
            uint16_t shape_id    : 3; // [7:5]   item_shape::* (0..6)
            uint16_t slot        : 4; // [11:8]  item_slot::* (1..12)
            uint16_t material_id : 4; // [15:12] material::* (0..15)
        };
        struct {
            uint16_t magic_item_id : 8; // [7:0]   1..255 in magic_items[] when slot == item_slot::magic_item
            uint16_t magic_slot    : 4; // [11:8]  item_slot::magic_item (0x0E)
            uint16_t magic_mat     : 4; // [15:12] 0
        };
    };

    constexpr ItemId() : raw(0) {}
    constexpr ItemId(uint16_t v) : raw(v) {}
    constexpr operator uint16_t() const { return this->raw; }

    static constexpr ItemId MakeEquip(uint8_t mat, uint8_t sl, uint8_t sh, uint8_t id) {
        return ItemId(static_cast<uint16_t>(((mat & 0x0F) << 12) | ((sl & 0x0F) << 8) | ((sh & 0x07) << 5) | (id & 0x1F)));
    }
    static constexpr ItemId MakeMagic(uint8_t magic_idx, uint8_t sl = item_slot::magic_item, uint8_t mat = 0) {
        return ItemId(static_cast<uint16_t>(((mat & 0x0F) << 12) | ((sl & 0x0F) << 8) | magic_idx));
    }

    constexpr bool IsMagicTemplate() const { return this->slot == item_slot::magic_item && this->raw < GOLD_PSEUDO_MIN; }
    constexpr bool IsScroll() const {
        return this->raw >= MakeMagic(SCROLL_OFFSET + 1).raw &&
               this->raw <= MakeMagic(SUPERSCROLL_OFFSET + 29).raw;
    }

    friend CArchive& operator>>(CArchive& ar, ItemId& id) { return ar >> id.raw; }
};
ASSERT_SIZE(ItemId, 2);
static_assert(ItemId::MakeEquip(material::bone, item_slot::ring, item_shape::rare, 1).raw == 0xD441, "ItemId equip bit layout");
static_assert(ItemId::MakeMagic(83).raw == ItemId::QUEST_TREASURE, "ItemId magic bit layout");

struct ItemWireFlags {
    enum FlagBit : uint8_t {
        FLAG_USABLE        = 0x01, // Consumable / usable item (potions, scrolls, books)
        FLAG_WARRIOR       = 0x02, // Usable by warrior (in itemname.pkt) / mage-restricted on client
        FLAG_MAGE          = 0x04, // Usable by mage (in itemname.pkt) / warrior-restricted on client
        FLAG_ALL_CLASSES   = FLAG_USABLE | FLAG_WARRIOR | FLAG_MAGE,
        FLAG_CAST_SPELL    = 0x10, // Has modifier::castspell
        FLAG_MAGIC_EFFECTS = 0x20, // Wire: 4B price + effect pairs follow; client: glow trail
        FLAG_NEWLY_PICKED  = 0x40, // Highlight / "picked up" message
        FLAG_COMPACT_COUNT = 0x80, // Wire: inline 6-bit stack count (1..63)
        COMPACT_COUNT_MAX  = 0x40,
    };

    union {
        uint8_t raw;
        struct {
            uint8_t effect_pair_count : 4; // [3:0] TLV pair count when has_magic_effects == 1
            uint8_t has_cast_spell    : 1; // [4]   0x10
            uint8_t has_magic_effects : 1; // [5]   0x20
            uint8_t is_newly_picked   : 1; // [6]   0x40
            uint8_t is_compact_count  : 1; // [7]   0x80
        };
        struct {
            uint8_t compact_count     : 6; // [5:0] stack count (1..63) when is_compact_count == 1
            uint8_t compact_new_flag  : 1; // [6]   0x40
            uint8_t compact_mode_bit  : 1; // [7]   0x80
        };
    };

    constexpr ItemWireFlags(uint8_t v = 0) : raw(v) {}
    constexpr operator uint8_t() const { return this->raw; }
};
ASSERT_SIZE(ItemWireFlags, 1);

class Item : public Token {
public:
    DECLARE_SERIAL(Item); // Runtime class definition at 6372b8.

public: // VTable at 60f498.
    virtual ~Item(); // 54840e (client: 52cfb7)
    virtual void Serialize(CArchive& ar) override; // 55c2f1 (client: 53737c)
    virtual void RegenTick() override; // 548da5 (client: 52d541)
    virtual void ProcessEffects() override; // 548d9a (client: 52d536)
    virtual int32_t IsItem() override; // 57ce40 (client: 54e0d0)
    virtual Item* Equip(Unit*); // 548a59 (client: 52d335) - Equip.
    virtual void Unequip(Unit*); // 548c04 (client: 52d476) - Unequip.
    virtual Item* TakeOne(); // 548dbf (client: 52d55b)
    virtual Item* Clone(); // 57cdc0 (client: 54e050)
    virtual int32_t OnUse(int32_t arg1, int32_t arg2); // 548db0 (client: 52d54c)
    virtual int32_t CalcValue(); // 54872b (client: 52d131)
    virtual int32_t IsSimpleItem(); // 548817 (client: 52d1e4)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot); // 549e86 (client: 52e1f8)

public:
    Item(); // 547f80 (client: 52cb56) - default constructor (blank item)
    Item(const CString& name); // 5480e3 (client: 52ccb9) - construct item from name string
    Item(uint8_t type, uint8_t subtype); // 54800e (client: 52cbe4) - construct item from type and subtype
    Item(const Item* src); // 54842a (client: 52cfd3) - copy constructor
    void InitFromTemplate(uint8_t slot); // 5482ab (client: 52ce60) - init item fields from magic_items table
    void AddEffect(Effect* effect); // 548faa (client: 52d6a0) - Add effect to item
    int IsArtifact(); // 548f07 (client: none) - IsArtifact: returns 1 if the item has magic and price 2.
    int IsCheatCreatable(); // 548f6a (client: 52d660) - Returns 1 if this item template can be given via #create cheat
    void ApplyPropertyString(const CString& str); // 548f3f (client: 52d635) - Apply special property string to item effects, then call CalcValue
    void RecalcFromTemplate(); // 54a0be (client: none) - Recalculate item properties from template data

    Effect* FindCastSpellEffect(); // 548e4e (client: 52d5e7) - Find the cast-spell Effect on this item (returns nullptr if none)
    int IsSimilar(Item* other); // 548860 (client: 52d22d) - Is this item similar to another? Same non-magic item or same base with same magic.

    void StoreToPacket(PacketUnitStateVec* pkt, int with_shelf_id); // 549afb (client: 52dee1)
    void WriteEffects(PacketUnitStateVec* pkt, uint8_t* slot); // 549f2c (client: 52e29e)
    void ApplyEffects(Unit* unit); // 548c2e (client: 52d4a0)
    void RemoveEffects(Unit* unit); // 548ce4 (client: 52d4eb)

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
    ItemId item_id;
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
    virtual ~Armor() override; // 54f9d1 (client: 531698)
    virtual void Serialize(CArchive& ar) override; // 55cf2b (client: 5377f2)
    virtual Item* Equip(Unit* unit) override; // 54fa6e (client: 531735)
    virtual void Unequip(Unit* unit) override; // 54fb7e (client: 531843)
    virtual Item* TakeOne() override; // 54fc19 (client: 5318dc)
    virtual Item* Clone() override; // 57d5b0 (client: 54e710)
    virtual int32_t CalcValue() override; // 54f84d (client: 531584)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 54fca8 (client: 531968)

public:
    Armor(); // 54f2e9 (client: 53109a) - default constructor
    Armor(const CString& name); // 54f361 (client: 531112) - construct from name string
    Armor(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 54f5ab (client: 5312f4)
    Armor(const Armor* src); // 54f9ed (client: 5316b4) - copy constructor
    void LoadInfo(); // 54f634 (client: 53137d)

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
    virtual void Serialize(CArchive& ar) override; // 55af5e (client: 536e3d)
    virtual Item* Equip(Unit*) override; // 55056b (client: 53218d)
    virtual void Unequip(Unit*) override; // 550686 (client: 5322a3)
    virtual Item* TakeOne() override; // 5504dc (client: 532101)
    virtual Item* Clone() override; // 57d660 (client: 54e7c0)
    virtual int32_t CalcValue() override; // 5502c7 (client: 531f5c)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 5506fc (client: 532317)

public:
    Shield(); // 54fe1b (client: 531ad2) - default constructor
    Shield(const CString& name); // 54fe8e (client: 531b45) - construct from name string
    Shield(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 5500fa (client: 531da6)
    Shield(const Shield* src); // 550467 (client: 53208c) - copy constructor

    void LoadInfo(); // 550183 (client: 531e2f)

public:
    Protections protections;
    uint8_t gap_0x6e[2];
};
ASSERT_SIZE(Shield, 0x70);

class Weapon : public Item {
public:
    DECLARE_SERIAL(Weapon); // Runtime class definition at 637300.

public: // VTable at 60f5b8.
    virtual ~Weapon() override; // 551159 (client: 532c65)
    virtual void Serialize(CArchive& ar) override; // 55929f (client: 536298)
    virtual Item* Equip(Unit* unit) override; // 5512e7 (client: 532dee)
    virtual void Unequip(Unit* unit) override; // 5515b9 (client: 5330b4)
    virtual Item* TakeOne() override; // 5517c3 (client: 5332bc)
    virtual Item* Clone() override; // 57d710 (client: 54e870)
    virtual int32_t CalcValue() override; // 550fa6 (client: 532b26)
    virtual void WriteState(PacketUnitStateVec* pkt, uint8_t* slot) override; // 551855 (client: 53334b)

public:
    Weapon(); // 550872 (client: 532484) - default constructor
    Weapon(const CString& name); // 550929 (client: 53252a) - construct weapon from name string
    Weapon(uint8_t shape_id, uint8_t material_id, uint8_t item_data_id); // 550b8f (client: 532729)
    Weapon(const Weapon* src); // 5511ee (client: 532cfa) - copy constructor
    void LoadEquipInfo(WorldEquip* params); // 550e26 (client: 5329b4)
    void LoadInfo(); // 550c23 (client: 5327bd)

private:
    void SetImbuedSpell(uint8_t spell_id); // 551b12 (client: 5335f5)
    void InitImbuedSpell(); // 551bdb (client: 5336a8)

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
