#ifndef UNIT_H
#define UNIT_H

#include <cstdint>
#include <array>

#include "assert_offset.h"
#include <afx.h>
#include <afxtempl.h>
#include "protections.h"
#include "table.h"
#include "token.h"
#include "unit_to_hit.h"


class Group;
class Inn;
struct Inventory;
class Item;
class MonsterInfo;
class Shield;
class Shop;
class Spell;
class SpellBook;
class Unit;
struct UnitEye;
struct UnitEye2;
class Weapon;


// Sum of extra properties from equipment.
struct EquipmentExtra {
public:
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

public:
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

class Unit : public Token {
public:
    DECLARE_SERIAL(Unit); // Runtime class definition at 6363e8.

public: // VTable at 60efc0.
    virtual ~Unit() override;
    virtual void Serialize(CArchive& ar) override;
    virtual void RegenTick() override;
    virtual void ProcessEffects() override;
    virtual int32_t GetTokenSize() override; // TokenSize
    virtual int32_t GetMovementType() override; // MovementType
    virtual void FixupPointers() override;
    virtual int32_t IsUnit() override; // IsUnit
    virtual int32_t IsHumanoid() override; // IsHumanoid
    virtual void ReleaseObjects();
    virtual void CopyFrom(Unit *);
    virtual Item* UseItem(Item *);
    virtual void UseItemOrStash(Item *);
    virtual Item* Unequip(Item *);
    virtual void UnequipAll();
    virtual void AwardKillMoney(Unit *);
    virtual uint32_t RollDamage(UnitToHit *, Unit *);
    virtual void RecalcStats();
    virtual uint32_t RecalcExperience();
    virtual void VMethod20();
    virtual void AddExperience(uint32_t, Unit *, int32_t);
    virtual void AwardKillExperience(Unit*, int32_t);
    virtual void AwardDamageExperience(Unit*, uint32_t, int32_t);
    virtual void AwardCastExperience(Unit*, int32_t spell_id);
    virtual int32_t GetCorpseDecayTime();

public:
    Unit();
    Unit(const CString&);
    Unit(const CString&, const TokenPos*);
    Unit(const TokenPos*);
    Unit(const TokenPos*, Player*);

    void InitDefaults(const CString& str);
    void ApplyInvincibility();    // Apply invincibility (god-mode) enchantments to unit
    void AddCarriedWeight(int32_t delta_weight); // Add inventory weight and notify clients after item change.
    void InitDefaultStats(); // Initialize default unit stats (for newly created units)
    void PickUpSack(class Sack* sack); // Pick up a sack into this unit's bag
    int NeedsRefreshFor(Player* player); // Check if this unit needs a refresh for the given player.
    void ClearRefreshFor(Player* player); // Mark this unit as not needing the refresh for the given player.
    int32_t ReturnToMap(); // Return unit to map.
    void sub_52C641(); // 52c641
    void sub_52C163();
    void sub_52C36D(uint8_t x, uint8_t y, uint8_t flags);
    void sub_52C58F(); // Revive unit (set flags, reset subcell, call VMethod18, notify world)
    void sub_52C813();
    void sub_52D8D3(Inventory* inventory, int32_t money, int32_t is_main_player_unit); // Create a sack from this unit's inventory and money.
    void sub_52C4DF(int16_t amount);  // Add HP (clamped to hp_max, triggers regen if newly positive)
    int sub_52BF3D(uint8_t x, uint8_t y, uint8_t size);  // Place unit on map; returns nonzero on success
    Effect* FindEnchantment(uint16_t effect_id); // Find first Effect in _effects list matching the given itemDataID.
    uint16_t sub_528725(); // Returns x bottom-right extent (sub-cell units).
    uint16_t sub_528763(); // Returns y bottom-right extent (sub-cell units).
    int32_t sub_52BABD(); // Returns g_Server->tick - last_action_tick
    void sub_52BDD7(uint16_t x, uint16_t y, uint8_t eye0, uint8_t eye1); // Process movement step
    void sub_52D94E(); // Kill this unit (sets some_state to 0x10 and runs death cleanup)
    void sub_52E7FA(); // Corpse: decrement HP and recalculate decay.
    int32_t sub_52D904(); // Defense value for quests. Flyers get +25, mages get +50.
    int32_t sub_52C735(uint8_t spell_id); // 52C735: HasEnchantment.
    uint8_t sub_52F5BB(uint8_t sphere); // 52F5BB: clamped spell power for sphere: max(0, skill_levels[sphere] + mind - 30)
    void sub_52F601(const CString& value);
    int32_t sub_59A030(); // Returns movement type.
    int32_t sub_5B6FB0(); // 5B6FB0 — IsMage.

    void FUN_0052ec7a(const CArray<MonsterInfoData>& values);

    Inventory* DetachInventory();
    
public:
    MonsterInfo* monster_info;
    Unit* last_hit_by;
    Spell* spell;
    int8_t last_hit_spell_id;
    int8_t token_size;
    int8_t movement_type;
    uint8_t face;
    // Unit attributes. Fighters have `& 4 == 0`. Phased-out units have `& 8 == 0`.
    // Piercing attack: `& 0x10 != 0`.
    int8_t unit_attrs;
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
    int16_t speed;
    int16_t extra_carrying_weight;
    int16_t carrying_weight_100g; // How much stuff the character is carrying. In 100g increments.
    int16_t carrying_body_100g; // Used to calculate how much the character's speed drops when carrying things. In vanilla, set to (body*10 + 1).
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
    uint8_t attack_ready;
    uint8_t gap_0x137;
    uint32_t last_action_tick;
    uint8_t decay; // Decay level. 0: alive (HP > 0), 1: on the ground (HP = 0), 2: fresh corpse (HP >= -20), 3: bones (HP >= -50), 4: old bones (HP >= -600), 5: completely gone.
    uint8_t gap_0x13d[3];
    SpellBook* spell_book;
    uint32_t enchantments;
    uint32_t summon_id; // All summoned mobs of one player will have the same non-null value of this field.
    uint16_t server_id;
    uint8_t gap_0x14e[2];
    uint32_t item_op_dirty_flags; // per-item-operation dirty/update flags
    uint32_t something_per_player[16];
    int32_t quest_kill_mark;
    int32_t quest_escort_mark;
    int32_t quest_intercept_mark;
    uint32_t summoned; // Summoned or raised from the dead units have this set to 1.
    uint16_t seen_by_mask;
    uint16_t seen_by_mask_prev;
    CArray<SessionMobKill> mob_kills_in_session;
    int32_t field_0x1bc;
    UnitEye* eye;
    UnitEye2* eye2;
    CList<PosYX> list1;
    CList<PosYX> list2;
    TokenPos* token_pos;
    int32_t ownership_changed;
};

ASSERT_OFFSET(Unit, inventory, 0x7C);
ASSERT_OFFSET(Unit, decay, 0x13C);
ASSERT_OFFSET(Unit, something_per_player, 0x154);
ASSERT_SIZE(Unit, 0x208);

class Humanoid : public Unit {
public:
    DECLARE_SERIAL(Humanoid);

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
    //virtual CRuntimeClass* GetRuntimeClass() const override;  // declared by DECLARE_SERIAL
    virtual ~Humanoid() override;
    virtual void Serialize(CArchive& ar) override;
    // CObject AssertValid  override
    // CObject Dump  override
    //virtual void VMethod1() override;
    //virtual void VMethod2() override;
    //virtual int32_t VMethod3() override;
    //virtual int32_t VMethod4() override;
    virtual void FixupPointers() override;
    //virtual int32_t VMethod6() override;
    virtual int32_t IsUnit() override;
    virtual int32_t IsHumanoid() override;
    //virtual int32_t VMethod9() override;
    virtual void ReleaseObjects() override;
    //virtual void VMethod11(Unit*) override;
    virtual Item* UseItem(Item*) override;
    virtual void UseItemOrStash(Item*) override;
    virtual Item* Unequip(Item*) override;
    virtual void UnequipAll() override;
    virtual void AwardKillMoney(Unit*) override;
    //virtual uint32_t VMethod17(UnitToHit*, Unit*) override;
    virtual void RecalcStats() override;
    virtual uint32_t RecalcExperience() override;
    //virtual void VMethod20() override;
    virtual void AddExperience(uint32_t newExp, Unit *target, int32_t sphere) override;
    virtual void AwardKillExperience(Unit*, int32_t spell_id) override;
    virtual void AwardDamageExperience(Unit*, uint32_t damage, int32_t spell_id) override;
    virtual void AwardCastExperience(Unit*, int32_t spell_id) override;
    virtual int32_t GetCorpseDecayTime() override;

public:
    Humanoid();
    Humanoid(const TokenPos*);

    void InitStats();
    void sub_53116B(); // Reduce humanoid's skills and revive the unit.
    void sub_531418(); // Recalculate experience from skills.
    void sub_530231(Humanoid* other); // 530231 - Copy stats and equipment from another humanoid.
    void sub_533345(int8_t main_skill, int8_t skill_level);   // Character creation: set skills and give initial weapon.
    void sub_532dde(CArray<HumanInfoData>* param_2);

public:
    std::array<Item*, 13> equipment;
    uint32_t main_sphere;
    std::array<uint32_t, 5> experience_per_sphere;
};

ASSERT_OFFSET(Humanoid, main_sphere, 0x23C);
ASSERT_SIZE(Humanoid, 0x254);


class Human: public Humanoid {
public:
    DECLARE_SERIAL(Human);

public:
    //virtual CRuntimeClass* GetRuntimeClass() const override;
    virtual ~Human() override;
    virtual void Serialize(CArchive& ar) override;

    virtual void VMethod20() override;

public:
    Human();
    Human(const CString& hname, int32_t is_hero, const char* unk);

    void FUN_00532587(CString hname, int32_t is_hero, const char* unk);
};

ASSERT_SIZE(Human, 0x254);


#endif
