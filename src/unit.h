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
    EquipmentExtra(); // 52e9e3 (client: 51aea2)
    EquipmentExtra& operator=(const EquipmentExtra&); // 52eaa3 (client: 51af62)
    void Serialize(CArchive& ar); // 52ec3d (client: 51b0f8)

    void SetToUnit(Unit *); // 52ea17 (client: 51aed6)
    void AddToUnit(Unit *); // 52eb64 (client: 51b023)
};
ASSERT_SIZE(EquipmentExtra, 0x40);

struct SessionMobKill {
    uint16_t server_id;
    uint16_t kills;
};
ASSERT_SIZE(SessionMobKill, 0x4);

struct UnitSaveFlags {
    union {
        uint32_t raw;
        struct {
            uint32_t server_id         : 16; // [15:0]
            uint32_t ownership_changed : 8;  // [23:16]
            uint32_t summoned          : 8;  // [31:24]
        };
    };

    constexpr UnitSaveFlags(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
};
ASSERT_SIZE(UnitSaveFlags, 4);

namespace unit_type {
enum : uint16_t {
    cavalry_a           = 0x13, // 19: Horse-rider with sword
    cavalry_b           = 0x15, // 21: Horse-rider with lance
    npc_mage            = 0x17, // 23: NPC mage
    npc_druid           = 0x18, // 24: NPC druid
    boss_a              = 0x1A, // 26: Special boss creature
    boss_b              = 0x1B, // 27: Special boss creature
    hero_sprite_base    = 0x20, // 32: Hero class range base (< 0x21 is non-hero)
    hero_fighter_male   = 0x21, // 33
    hero_fighter_female = 0x22, // 34 (Amazon)
    hero_mage_male      = 0x23, // 35
    hero_mage_female    = 0x24, // 36 (Sorceress / Witch)
    humanoid_last       = 0x3F, // 63: Last humanoid / hero typeId
    monster_first       = 0x40, // 64: First monster typeId
    ghost               = 0x45, // 69
    bat                 = 0x46, // 70
    dragon              = 0x47, // 71
    urd                 = 0x48, // 72
    bee                 = 0x49, // 73
    summon_squirrel     = 0x4A, // 74
    summon_snake        = 0x4B, // 75
    summon_turtle       = 0x4C, // 76
    orc_shaman          = 0x51, // 81
    zombie              = 0x52, // 82: First undead typeId (F_Zombie)
    skeleton            = 0x59, // 89: F_Skeleton
    skeleton_mage_a     = 0x61, // 97: M_Skeleton.3
    skeleton_mage_b     = 0x62, // 98: M_Skeleton.4 (last classic undead)
    necromancer         = 0x63, // 99
    druid               = 0x64, // 100
    catapult            = 0x65, // 101: Catapult / campaign mercenary orc
    merc_troll          = 0x66, // 102: Campaign mercenary troll
    spider              = 0x68, // 104
    succubus            = 0x69, // 105
    monster_last        = 0x6B, // 107
};
} // namespace unit_type

class Unit : public Token {
public:
    DECLARE_SERIAL(Unit); // Runtime class definition at 6363e8.

    enum Attr : uint8_t {
        ATTR_HAS_SPELLBOOK = 0x02,
        ATTR_MAGE          = 0x04,
        ATTR_PHASED_OUT    = 0x08,
        ATTR_PIERCING      = 0x10,
    };

    enum : uint8_t {
        HAND_DAMAGE_VALUE_MASK = 0x7F,
        HAND_DAMAGE_15X_FLAG   = 0x80,
        HAND_DAMAGE_15X_MULT   = 15,
    };

    enum OrderState : int32_t {
        ORDER_IDLE           = 0,
        ORDER_MOVE           = 1,
        ORDER_PICKUP         = 2,
        ORDER_ATTACK_UNIT    = 3,
        ORDER_ATTACK_MOVE    = 4,
        ORDER_DEFEND_TARGET  = 8,
        ORDER_PATROL_STEP    = 10,
        ORDER_GUARD_POSITION = 11,
        ORDER_HOLD_POSITION  = 12,
        ORDER_CAST_UNIT      = 13,
        ORDER_CAST_POS       = 14,
        ORDER_ENTER_BUILDING = 15,
        ORDER_DEAD           = 16,
        ORDER_FOLLOW         = 17,
        ORDER_RETURN_HOME    = 22,
        ORDER_STAND_GROUND   = 23,
        ORDER_SCRIPT_PATROL  = 24,
        ORDER_SCRIPT_WALK    = 25,
        ORDER_LOOT_NEARBY    = 26,
        ORDER_SCRIPT_SPECIAL = 27,
    };

    enum ExecState : int32_t {
        EXEC_IDLE              = 0,
        EXEC_STEP              = 1,
        EXEC_PICKUP            = 2,
        EXEC_ATTACK            = 3,
        EXEC_CAST_UNIT         = 13,
        EXEC_CAST_POS          = 14,
        EXEC_INTERACT_BUILDING = 15,
        EXEC_DEAD              = 16,
        EXEC_SPECIAL           = 27,
    };

    enum SwingPhase : int32_t {
        SWING_READY    = 0,
        SWING_WINDUP   = 5,
        SWING_RECOVERY = 7,
    };

    enum DecayStage : uint8_t {
        DECAY_ALIVE       = 0,
        DECAY_UNCONSCIOUS = 1,
        DECAY_CORPSE      = 2,
        DECAY_BONES       = 3,
        DECAY_OLD_BONES   = 4,
        DECAY_REMOVED     = 5,
    };

public: // VTable at 60efc0.
    virtual ~Unit() override; // 52a330 (client: 51897e)
    virtual void Serialize(CArchive& ar) override; // 5593b6 (client: 53633a)
    virtual void RegenTick() override; // 52bad9 (client: 51985d)
    virtual void ProcessEffects() override; // 52a857 (client: 518d2f)
    virtual int32_t GetTokenSize() override; // 57b790 (client: 54c350) - TokenSize
    virtual int32_t GetMovementType() override; // 57b7b0 (client: 54c370) - MovementType
    virtual void FixupPointers() override; // 55a9c5 (client: 536c19)
    virtual int32_t IsUnit() override; // 57b840 (client: 54c400) - IsUnit
    virtual int32_t IsHumanoid() override; // 57b850 (client: 54c410) - IsHumanoid
    virtual void ReleaseObjects(); // 52a504 (client: 518a17)
    virtual void CopyFrom(Unit *); // 52963b (client: 51807f)
    virtual Item* UseItem(Item *); // 52c8e5 (client: 51a457)
    virtual void UseItemOrStash(Item *); // 52c914 (client: 51a486)
    virtual Item* Unequip(Item *); // 52c957 (client: 51a4c9)
    virtual void UnequipAll(); // 57b7d0 (client: 54c390)
    virtual void AwardKillMoney(Unit *); // 57b7e0 (client: 54c3a0)
    virtual uint32_t RollDamage(UnitToHit *, Unit *); // 536b31 (client: 520be5)
    virtual void RecalcStats(); // 52f566 (client: 51b4fa)
    virtual uint32_t RecalcExperience(); // 57b7f0 (client: 54c3b0)
    virtual void GiveLevelReward(); // 52fed7 (client: 51bcab)
    virtual void AddExperience(uint32_t, Unit *, int32_t); // 5371f7 (client: 52124c)
    virtual void AwardKillExperience(Unit*, int32_t); // 57b810 (client: 54c3d0)
    virtual void AwardDamageExperience(Unit*, uint32_t, int32_t); // 57b820 (client: 54c3e0)
    virtual void AwardCastExperience(Unit*, int32_t spell_id); // 57b830 (client: 54c3f0)
    virtual int32_t GetCorpseDecayTime(); // 52a771 (client: 518c4b)

public:
    Unit(); // 528aef (client: 51781c)
    Unit(const CString&); // 528c97 (client: 517932)
    Unit(const CString&, const TokenPos*); // 528e24 (client: 517a2b)
    Unit(const TokenPos*); // 528fb7 (client: 517b28)
    Unit(const TokenPos*, Player*); // 529167 (client: 517c44)

    void InitDefaults(const CString& str); // 52931b (client: 517d64)
    void ApplyInvincibility(); // 537251 (client: 52129d) - Apply invincibility (god-mode) enchantments to unit
    void AddCarriedWeight(int32_t delta_weight); // 52a790 (client: 518c68) - Add inventory weight and notify clients after item change.
    void InitDefaultStats(); // 52a215 (client: 518863) - Initialize default unit stats (for newly created units)
    void PickUpSack(class Sack* sack); // 52c98b (client: 51a4fd) - Pick up a sack into this unit's bag
    int NeedsRefreshFor(Player* player); // 5287c4 (client: 517510) - Check if this unit needs a refresh for the given player.
    void ClearRefreshFor(Player* player); // 5287ec (client: 517538) - Mark this unit as not needing the refresh for the given player.
    int32_t ReturnToMap(); // 52c409 (client: 519fb1) - Return unit to map.
    void ResetActionState(); // 52c641 (client: 51a1e9)
    void RemoveFromMap(); // 52c163 (client: 519ea6)
    void AddToMap(uint8_t x, uint8_t y, uint8_t flags); // 52c36d (client: 519f1b)
    void Revive(); // 52c58f (client: 51a137) - Revive unit (set flags, reset subcell, call VMethod18, notify world)
    void CancelQueuedSpell(); // 52c813 (client: 51a348)
    void CreateSackFromInventory(Inventory* inventory, int32_t money, int32_t is_main_player_unit); // 52d8d3 (client: 51a5f8) - Create a sack from this unit's inventory and money.
    void AddHp(int16_t amount); // 52c4df (client: 51a087) - Add HP (clamped to hp_max, triggers regen if newly positive)
    int PlaceOnMap(uint8_t x, uint8_t y, uint8_t size); // 52bf3d (client: 519c8c) - Place unit on map; returns nonzero on success
    Effect* FindEnchantment(uint16_t effect_id); // 52c757 (client: 51a2f7) - Find first Effect in _effects list matching the given itemDataID.
    uint16_t GetRightXx(); // 528725 (client: 517475) - Returns x bottom-right extent (sub-cell units).
    uint16_t GetBottomYy(); // 528763 (client: 5174b1) - Returns y bottom-right extent (sub-cell units).
    int32_t GetTicksSinceAction(); // 52babd (client: 519841) - last_action_tick
    void ProcessMoveStep(uint16_t x, uint16_t y, uint8_t eye0, uint8_t eye1); // 52bdd7 (client: 519b30) - Process movement step
    void Die(); // 52d94e (client: 51a673) - Kill this unit (sets some_state to 0x10 and runs death cleanup)
    void DecayTick(); // 52e7fa (client: 51acbb) - Corpse: decrement HP and recalculate decay.
    int32_t GetQuestDefense(); // 52d904 (client: 51a629) - Defense value for quests. Flyers get +25, mages get +50.
    int32_t HasEnchantment(uint8_t spell_id); // 52c735 (client: 51a2d5) - HasEnchantment.
    uint8_t GetSpellPower(uint8_t sphere); // 52f5bb (client: 51b54f) - clamped spell power for sphere: max(0, skill_levels[sphere] + mind - 30)
    void InitFromMonsterName(const CString& value); // 52f601 (client: 51b595)
    int32_t MovementType(); // 59a030 (client: 569320) - Returns movement type.
    int32_t IsMage(); // 5b6fb0 (client: 5444d0) - IsMage.

    // Scan range or 0 if it's negative.
    int32_t EffectiveScanRange() const {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        int32_t range = static_cast<int16_t>(this->scan_range);
        return (range < 0) ? 0 : range;
#else
        return this->scan_range;
#endif
    }

    void ApplyMonsterInfo(const CArray<MonsterInfoData>& values); // 52ec7a (client: 51b12f)

    Inventory* DetachInventory(); // split from 52d94e (client: 51a673)
    
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
    int32_t has_flagged_item;
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
    virtual ~Humanoid() override; // 53015b (client: 51bf2c)
    virtual void Serialize(CArchive& ar) override; // 55c760 (client: 5374ea)
    // CObject AssertValid  override
    // CObject Dump  override
    //virtual void VMethod1() override;
    //virtual void VMethod2() override;
    //virtual int32_t VMethod3() override;
    //virtual int32_t VMethod4() override;
    virtual void FixupPointers() override; // 55ab26 (client: 536c9c)
    //virtual int32_t VMethod6() override;
    virtual int32_t IsUnit() override; // 57bb50 (client: 54c710)
    virtual int32_t IsHumanoid() override; // 57bb60 (client: 54c720)
    //virtual int32_t VMethod9() override;
    virtual void ReleaseObjects() override; // 5301ac (client: 51bf7d)
    //virtual void VMethod11(Unit*) override;
    virtual Item* UseItem(Item*) override; // 5303e3 (client: 51c1af)
    virtual void UseItemOrStash(Item*) override; // 5304b2 (client: 51c274)
    virtual Item* Unequip(Item*) override; // 5304e8 (client: 51c2aa)
    virtual void UnequipAll() override; // 530511 (client: 51c2d3)
    virtual void AwardKillMoney(Unit*) override; // 53111b (client: 51ceb7)
    //virtual uint32_t VMethod17(UnitToHit*, Unit*) override;
    virtual void RecalcStats() override; // 5315ec (client: 51d2a7)
    virtual uint32_t RecalcExperience() override; // 537199 (client: 5211ee)
    //virtual void GiveLevelReward() override;
    virtual void AddExperience(uint32_t newExp, Unit *target, int32_t sphere) override; // 530745 (client: 51c507)
    virtual void AwardKillExperience(Unit*, int32_t spell_id) override; // 530e52 (client: 51cc07)
    virtual void AwardDamageExperience(Unit*, uint32_t damage, int32_t spell_id) override; // 530fc1 (client: 51cd64)
    virtual void AwardCastExperience(Unit*, int32_t spell_id) override; // 531090 (client: 51ce2b)
    virtual int32_t GetCorpseDecayTime() override; // 530394 (client: 51c162)

public:
    Humanoid(); // 52ff9e (client: 51bd6f)
    Humanoid(const TokenPos*); // 52fff2 (client: 51bdc3)

    void InitStats(); // 53004c (client: 51be1d)
    void ReviveWithPenalty(); // 53116b (client: 51cefe) - Reduce humanoid's skills and revive the unit.
    void RecalcExperienceFromSkills(); // 531418 (client: 51d0d3) - Recalculate experience from skills.
    void CopyFrom(Humanoid* other); // 530231 (client: 51c002) - Copy stats and equipment from another humanoid.
    void InitCreationSkills(int8_t main_skill, int8_t skill_level); // 533345 (client: 51eb95) - Character creation: set skills and give initial weapon.
    void ApplyHumanInfo(CArray<HumanInfoData>* data); // 532dde (client: 51e92e)

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
    virtual ~Human() override; // 57bbd0 (client: 54c7b0)
    virtual void Serialize(CArchive& ar) override; // 55c81c (client: 53759d)

    virtual void GiveLevelReward() override; // 5339a6 (client: 51f1b1)

public:
    Human(); // 53232d (client: 51dfb3)
    Human(const CString& hname, int32_t is_hero, const char* unk); // 53239a (client: 51e020)

    void InitFromName(CString hname, int32_t is_hero, const char* unk); // 532587 (client: 51e20d)
};

ASSERT_SIZE(Human, 0x254);


#endif
