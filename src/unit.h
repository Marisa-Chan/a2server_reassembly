#ifndef UNIT_H
#define UNIT_H

#include <cstdint>
#include <array>

#include "token.h"
#include "protections.h"
#include "unit_to_hit.h"
#include "assert_offset.h"


class Group;
struct Inventory;
class Item;
class MonsterInfo;
class Shield;
class Spell;
class SpellBook;
class Unit;
struct UnitEye;
struct UnitEye2;
class Weapon;


// Sum of extra properties from equipment.
struct EquipmentExtra {
    uint8_t body = 0;
    uint8_t reaction = 0;
    uint8_t mind = 0;
    uint8_t spirit = 0;
    int16_t speed = 0;
    uint16_t carrying_body_100g = 0;
    uint16_t hp_max = 0;
    uint16_t hp_regen = 0;
    uint16_t mp_max = 0;
    uint16_t mp_regen = 0;
    uint16_t scan_range = 0;
    UnitToHit hit_values;
    Protections protections;

    EquipmentExtra();
    EquipmentExtra& operator=(const EquipmentExtra&);
    void Serialize(CArchive& ar);

    void SetToUnit(Unit *);
    void AddToUnit(Unit *);

};
ASSERT_SIZE(EquipmentExtra, 0x40);

struct SessionMobKill {
    uint16_t server_id;
    uint16_t kills;
};
ASSERT_SIZE(SessionMobKill, 0x4);

class Unit : public Token
{
public:
    DECLARE_SERIAL(Unit);

public: //vtbl
    //virtual CRuntimeClass* GetRuntimeClass() const override; // defined by DECLARE_SERIAL
    virtual ~Unit() override;
    virtual void Serialize(CArchive& ar) override;
    // CObject AssertValid  override
    // CObject Dump  override
    virtual void VMethod1() override;
    virtual void VMethod2() override;
    virtual int32_t VMethod3() override;
    virtual int32_t VMethod4() override;
    virtual void VMethod5() override;
    //virtual int32_t VMethod6() override;
    virtual int32_t VMethod7() override;
    virtual int32_t VMethod8() override;
    //virtual int32_t VMethod9() override;
    virtual void VMethod10();
    virtual void VMethod11(Unit *);
    virtual Item* VMethod12(Item *);
    virtual void VMethod13(Item *);
    virtual Item* Unequip(Item *);
    virtual void VMethod15();
    virtual void VMethod16(Item *);
    virtual uint32_t VMethod17(UnitToHit *, Unit *);
    virtual void VMethod18();
    virtual uint32_t VMethod19();
    virtual void VMethod20();
    virtual void VMethod21(uint32_t, Unit *, int32_t);
    virtual void VMethod22(Unit*, int32_t);
    virtual void VMethod23(Unit*, uint32_t, int32_t, int32_t);
    virtual void VMethod24(Unit*, uint32_t, int32_t, int32_t);
    virtual int32_t VMethod25();

public:
    Unit();
    Unit(const CString&);
    Unit(const CString&, const TokenPos*);
    Unit(const TokenPos*);
    Unit(const TokenPos*, Player*);

    void FUN_0052931b(const CString& str); //in asm

public:
    MonsterInfo* monster_info;
    Unit* last_hit_by;
    Spell* spell;
    int8_t last_hit_spell_id;
    int8_t token_size;
    int8_t movement_type;
    int8_t face;
    int8_t unit_attrs; // Fighters have `& 4 == 0`. Phased-out units have `& 8 == 0`.
    int8_t gap_0x4d[3];
    int32_t state;
    int32_t some_state;
    int32_t some_state2;
    Unit *cast_target;
    int8_t area_cast_x;
    int8_t area_cast_y;
    int8_t gap_0x62[2];
    Spell* some_spell;
    Item* some_item;
    int8_t charge_countdown;
    int8_t gap_0x6d[3];
    Group* group;
    Weapon* weapon;
    Shield* shield;
    Inventory* inventory;
    CString name;
    uint16_t body;
    uint16_t reaction;
    uint16_t mind;
    uint16_t spirit;
    uint16_t speed;
    uint16_t extra_carrying_weight;
    uint16_t carrying_weight_100g; // How much stuff the character is carrying. In 100g increments.
    uint16_t carrying_body_100g; // Used to calculate how much the character's speed drops when carrying things. In vanilla, set to (body*10 + 1).
    int16_t hp;
    int16_t hp_max;
    int16_t hp_regen;
    int16_t mp;
    int16_t mp_max;
    int16_t mp_regen;
    int16_t mp2;
    uint8_t hp_regen_carry; // Carry from the previous hp regen computation, divided by 100.
    uint8_t mp_regen_carry;
    uint16_t scan_range; // Scan range multiplied by 256.
    UnitToHit hit_values;
    Protections protections;
    EquipmentExtra equipment_extra;
    UnitToHit hit_values2;
    uint8_t max_range;
    uint8_t gap12d[3];
    uint32_t experience;
    uint8_t charge;
    uint8_t relax;
    uint8_t gap_0x136[2];
    uint32_t last_action_tick;
    uint8_t decay; // Decay level. 0: alive (HP > 0), 1: on the ground (HP = 0), 2: fresh corpse (HP >= -20), 3: bones (HP >= -50), 4: old bones (HP >= -600), 5: completely gone.
    uint8_t gap_0x13d[3];
    SpellBook* spell_book;
    uint32_t enchantments;
    uint32_t summon_id; // All summoned mobs of one player will have the same non-null value of this field.
    uint16_t server_id;
    uint8_t gap_0x14e[6];
    uint32_t something_per_player[16];
    int32_t field_0x194;
    int32_t field_0x198;
    int32_t field_0x19c;
    uint32_t summoned; // Summoned or raised from the dead units have this set to 1.
    int8_t gap_0x1a4[4];
    CArray<SessionMobKill> mob_kills_in_session;
    int32_t field_0x1bc;
    UnitEye* eye;
    UnitEye2* eye2;
    CList<int16_t> list1;
    CList<int16_t> list2;
    TokenPos* token_pos;
    int32_t field_0x204;
};

ASSERT_OFFSET(Unit, inventory, 0x7C);
ASSERT_OFFSET(Unit, decay, 0x13C);
ASSERT_SIZE(Unit, 0x208);

class Humanoid : public Unit {
public:
    DECLARE_SERIAL(Humanoid);

public:
    //virtual CRuntimeClass* GetRuntimeClass() const override;  // declared by DECLARE_SERIAL
    virtual ~Humanoid() override;
    virtual void Serialize(CArchive& ar) override;
    // CObject AssertValid  override
    // CObject Dump  override
    //virtual void VMethod1() override;
    //virtual void VMethod2() override;
    //virtual int32_t VMethod3() override;
    //virtual int32_t VMethod4() override;
    virtual void VMethod5() override;
    //virtual int32_t VMethod6() override;
    virtual int32_t VMethod7() override;
    virtual int32_t VMethod8() override;
    //virtual int32_t VMethod9() override;
    virtual void VMethod10() override;
    //virtual void VMethod11(Unit*) override;
    virtual Item* VMethod12(Item*) override;
    virtual void VMethod13(Item*) override;
    virtual Item* Unequip(Item*) override;
    virtual void VMethod15() override;
    virtual void VMethod16(Item*) override;
    //virtual uint32_t VMethod17(UnitToHit*, Unit*) override;
    virtual void VMethod18() override;
    virtual uint32_t VMethod19() override;
    //virtual void VMethod20() override;
    virtual void VMethod21(uint32_t newExp, Unit *target, int32_t sphere) override;
    virtual void VMethod22(Unit*, int32_t) override;
    virtual void VMethod23(Unit*, uint32_t, int32_t, int32_t) override;
    virtual void VMethod24(Unit*, uint32_t, int32_t, int32_t) override;
    virtual int32_t VMethod25() override;

public:
    Humanoid();
    Humanoid(const TokenPos*);

    void InitStats();

public:
    std::array<Item*, 13> equipment;
    uint32_t main_sphere;
    std::array <uint32_t, 5> experience_per_sphere;
};

ASSERT_OFFSET(Humanoid, main_sphere, 0x23C);
ASSERT_SIZE(Humanoid, 0x254);


class Human: public Humanoid {
public:
    enum EquipmentSlot {
        // 0, 1, 2 and 3 are unused.
        SLOT_RING = 4,
        SLOT_AMULET = 5,
        SLOT_HELM = 6, // Or mage's hat.
        SLOT_MAIL = 7, // Or mage's dress.
        SLOT_CUIRASS = 8, // Or mage's cape.
        SLOT_BRACERS = 9, // Unused for mage.
        SLOT_GAUNTLETS = 10, // Or mage's gloves.
        // 11 is unused.
        SLOT_BOOTS = 12, // Or mage's shoes.
    };

public:
    virtual CRuntimeClass* GetRuntimeClass() const override;
    virtual ~Human() override;
    virtual void Serialize(CArchive& ar) override;

    virtual void VMethod20() override;

public:
    Human();

};

ASSERT_OFFSET(Human, main_sphere, 0x23C);
ASSERT_SIZE(Human, 0x254);


#endif
