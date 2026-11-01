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
    EquipmentExtra();  // client: 51aea2
    EquipmentExtra& operator=(const EquipmentExtra&);
    void Serialize(CArchive& ar);  // client: 51b0f8

    void SetToUnit(Unit *);  // client: 51aed6
    void AddToUnit(Unit *);  // client: 51b023
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
    virtual void Serialize(CArchive& ar) override;  // client: 53633a
    virtual void RegenTick() override;  // client: 51985d
    virtual void ProcessEffects() override;  // client: 518d2f
    virtual int32_t GetTokenSize() override; // TokenSize
    virtual int32_t GetMovementType() override; // MovementType
    virtual void FixupPointers() override;  // client: 536c19
    virtual int32_t IsUnit() override; // IsUnit
    virtual int32_t IsHumanoid() override; // IsHumanoid
    virtual void ReleaseObjects();  // client: 518a17
    virtual void CopyFrom(Unit *);  // client: 51807f
    virtual Item* UseItem(Item *);
    virtual void UseItemOrStash(Item *);
    virtual Item* Unequip(Item *);
    virtual void UnequipAll();
    virtual void AwardKillMoney(Unit *);  // client: 520be5
    virtual uint32_t RollDamage(UnitToHit *, Unit *);  // client: 520be5
    virtual void RecalcStats();
    virtual uint32_t RecalcExperience();
    virtual void GiveLevelReward();  // client: 52124c
    virtual void AddExperience(uint32_t, Unit *, int32_t);  // client: 52124c
    virtual void AwardKillExperience(Unit*, int32_t);
    virtual void AwardDamageExperience(Unit*, uint32_t, int32_t);
    virtual void AwardCastExperience(Unit*, int32_t spell_id);
    virtual int32_t GetCorpseDecayTime();

public:
    Unit();  // client: 51781c
    Unit(const CString&);  // client: 517932
    Unit(const CString&, const TokenPos*);  // client: 517a2b
    Unit(const TokenPos*);  // client: 517b28
    Unit(const TokenPos*, Player*);  // client: 517c44

    void InitDefaults(const CString& str);  // client: 517d64
    void ApplyInvincibility();    // Apply invincibility (god-mode) enchantments to unit (client: 52129d)
    void AddCarriedWeight(int32_t delta_weight); // Add inventory weight and notify clients after item change. (client: 518c68)
    void InitDefaultStats(); // Initialize default unit stats (for newly created units) (client: 518863)
    void PickUpSack(class Sack* sack); // Pick up a sack into this unit's bag (client: 52ccb9)
    int NeedsRefreshFor(Player* player); // Check if this unit needs a refresh for the given player. (client: 517510)
    void ClearRefreshFor(Player* player); // Mark this unit as not needing the refresh for the given player. (client: 517538)
    int32_t ReturnToMap(); // Return unit to map. (client: 519fb1)
    void ResetActionState(); // 52c641 (client: 51a1e9)
    void RemoveFromMap();  // client: 519ea6
    void AddToMap(uint8_t x, uint8_t y, uint8_t flags);  // client: 519f1b
    void Revive(); // Revive unit (set flags, reset subcell, call VMethod18, notify world) (client: 51a137)
    void CancelQueuedSpell();  // client: 51a348
    void CreateSackFromInventory(Inventory* inventory, int32_t money, int32_t is_main_player_unit); // Create a sack from this unit's inventory and money. (client: 51a5f8)
    void AddHp(int16_t amount);  // Add HP (clamped to hp_max, triggers regen if newly positive) (client: 51a087)
    int PlaceOnMap(uint8_t x, uint8_t y, uint8_t size);  // Place unit on map; returns nonzero on success (client: 519c8c)
    Effect* FindEnchantment(uint16_t effect_id); // Find first Effect in _effects list matching the given itemDataID. (client: 51a2f7)
    uint16_t GetRightXx(); // Returns x bottom-right extent (sub-cell units). (client: 517475)
    uint16_t GetBottomYy(); // Returns y bottom-right extent (sub-cell units). (client: 5174b1)
    int32_t GetTicksSinceAction(); // Returns g_Server->tick - last_action_tick (client: 519841)
    void ProcessMoveStep(uint16_t x, uint16_t y, uint8_t eye0, uint8_t eye1); // Process movement step (client: 519b30)
    void Die(); // Kill this unit (sets some_state to 0x10 and runs death cleanup) (client: 51a673)
    void DecayTick(); // Corpse: decrement HP and recalculate decay. (client: 51acbb)
    int32_t GetQuestDefense(); // Defense value for quests. Flyers get +25, mages get +50. (client: 51a629)
    int32_t HasEnchantment(uint8_t spell_id); // 52C735 (client: 51a2d5): HasEnchantment.
    uint8_t GetSpellPower(uint8_t sphere); // 52F5BB: clamped spell power for sphere: max(0, skill_levels[sphere] + mind - 30) (client: 51b54f)
    void InitFromMonsterName(const CString& value);  // client: 51b595
    int32_t MovementType(); // Returns movement type. (client: 4c7743)
    int32_t IsMage(); // 5B6FB0 (client: 5444d0) — IsMage.

    void ApplyMonsterInfo(const CArray<MonsterInfoData>& values);  // client: 51b12f

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
    virtual ~Humanoid() override;  // client: 51bf2c
    virtual void Serialize(CArchive& ar) override;  // client: 5374ea
    // CObject AssertValid  override
    // CObject Dump  override
    //virtual void VMethod1() override;
    //virtual void VMethod2() override;
    //virtual int32_t VMethod3() override;
    //virtual int32_t VMethod4() override;
    virtual void FixupPointers() override;  // client: 536c9c
    //virtual int32_t VMethod6() override;
    virtual int32_t IsUnit() override;  // client: 54c710
    virtual int32_t IsHumanoid() override;  // client: 54c720
    //virtual int32_t VMethod9() override;
    virtual void ReleaseObjects() override;  // client: 51bf7d
    //virtual void VMethod11(Unit*) override;
    virtual Item* UseItem(Item*) override;  // client: 51c1af
    virtual void UseItemOrStash(Item*) override;  // client: 51c274
    virtual Item* Unequip(Item*) override;  // client: 51c2aa
    virtual void UnequipAll() override;  // client: 51c2d3
    virtual void AwardKillMoney(Unit*) override;  // client: 51ceb7
    //virtual uint32_t VMethod17(UnitToHit*, Unit*) override;
    virtual void RecalcStats() override;
    virtual uint32_t RecalcExperience() override;  // client: 5211ee
    //virtual void GiveLevelReward() override;
    virtual void AddExperience(uint32_t newExp, Unit *target, int32_t sphere) override;  // client: 51c507
    virtual void AwardKillExperience(Unit*, int32_t spell_id) override;  // client: 51cc07
    virtual void AwardDamageExperience(Unit*, uint32_t damage, int32_t spell_id) override;  // client: 51cd64
    virtual void AwardCastExperience(Unit*, int32_t spell_id) override;  // client: 51ce2b
    virtual int32_t GetCorpseDecayTime() override;  // client: 51c162

public:
    Humanoid();  // client: 51bd6f
    Humanoid(const TokenPos*);  // client: 51bdc3

    void InitStats();  // client: 51be1d
    void ReviveWithPenalty(); // Reduce humanoid's skills and revive the unit. (client: 51cefe)
    void RecalcExperienceFromSkills(); // Recalculate experience from skills. (client: 51d0d3)
    void CopyFrom(Humanoid* other); // 530231 (client: 51c002) - Copy stats and equipment from another humanoid.
    void InitCreationSkills(int8_t main_skill, int8_t skill_level);   // Character creation: set skills and give initial weapon. (client: 51eb95)
    void ApplyHumanInfo(CArray<HumanInfoData>* data);  // client: 51e92e

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
    virtual ~Human() override;  // client: 53759d
    virtual void Serialize(CArchive& ar) override;  // client: 53759d

    virtual void GiveLevelReward() override;  // client: 51f1b1

public:
    Human();  // client: 51dfb3
    Human(const CString& hname, int32_t is_hero, const char* unk);  // client: 51e020

    void InitFromName(CString hname, int32_t is_hero, const char* unk);  // client: 51e20d
};

ASSERT_SIZE(Human, 0x254);


#endif
