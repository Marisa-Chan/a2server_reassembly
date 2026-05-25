#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class Item;


class TableLine : public CObject {
public:
    DECLARE_SERIAL(TableLine); // Runtime class definition at 6362b0.

public:
    TableLine(); // 536d8c

public: // VTable at 60eae8.
    virtual void Serialize(CArchive& ar) override;
    virtual void VMethod1(CString line, int32_t values_count, double* out_values);
    virtual void VMethod2(CString str, int32_t type_id, int32_t* out_value, double* out_double);

public:
    CString name;
    CArray<int32_t> values;

protected:
    template <typename T>
    CArray<T>& ValuesAs() {
        return *reinterpret_cast<CArray<T>*>(&this->values);
    }

    template <typename T>
    const CArray<T>& ValuesAs() const {
        return *reinterpret_cast<const CArray<T>*>(&this->values);
    }
};
ASSERT_SIZE(TableLine, 0x1c);

struct MatShapeData {
    uint32_t field_0x0;
    uint32_t field_0x4;
    uint32_t field_0x8;
    uint32_t field_0xc;
    double price;
    double weight;
    double damage;
    double attack;
    double defense;
    double absorption;
    double magic_volume;
};
ASSERT_SIZE(MatShapeData, 0x48);

struct EquipData;

class WorldEquip: public TableLine {
public:
    CArray<EquipData>& Values() {
        return this->ValuesAs<EquipData>();
    }

    const CArray<EquipData>& Values() const {
        return this->ValuesAs<EquipData>();
    }

    uint16_t shape_material_matrix[7];
    uint8_t gap_0x2a[2];
    CStringArray string_array;
};
ASSERT_OFFSET(WorldEquip, shape_material_matrix, 0x1c);
ASSERT_SIZE(WorldEquip, 0x40);

class MagicItem : public WorldEquip { // Inherits from `WorldEquip` at 005727e1.
public:
    CString effect;
};
ASSERT_SIZE(MagicItem, 0x44);

class MatShape : public TableLine {
public:
    CArray<int32_t>& Values() {
        return this->ValuesAs<int32_t>();
    }

    const CArray<int32_t>& Values() const {
        return this->ValuesAs<int32_t>();
    }

    uint32_t field_0x1c;
    MatShapeData data;
};
ASSERT_OFFSET(MatShape, data, 0x20);
ASSERT_SIZE(MatShape, 0x68);

struct BuildingInfoData {
    int32_t width;
    int32_t height;
    int32_t scan_range;
    int32_t health_max;
    uint32_t unpassable_mask;
    uint32_t tiles_mask;
};
ASSERT_SIZE(BuildingInfoData, 0x18);

class BuildingInfo : public TableLine {
public:
    CArray<BuildingInfoData>& Values() {
        return this->ValuesAs<BuildingInfoData>();
    }

    const CArray<BuildingInfoData>& Values() const {
        return this->ValuesAs<BuildingInfoData>();
    }
};
ASSERT_SIZE(BuildingInfo, 0x1c);

struct EquipData {
    int32_t shape;
    int32_t material;
    int32_t price;
    int32_t weight;
    int32_t slot;
    int32_t attack_type;
    int32_t damage_min;
    int32_t damage_max;
    int32_t attack;
    int32_t defense;
    int32_t absorption;
    int32_t range;
    int32_t relax;
    int32_t two_handed;
    int32_t suitable_for;
    int32_t other_param;
    int32_t shapes_allowed;
};
ASSERT_SIZE(EquipData, 0x44);


struct MagicInfoData {
    int32_t mana_cost;
    int32_t affect_min;
    int32_t affect_max;
    int32_t usable_by;
    int32_t in_warrior_weapon;
    int32_t in_warrior_shield;
    int32_t in_warrior_not_used_3;
    int32_t in_warrior_ring;
    int32_t in_warrior_amulet;
    int32_t in_warrior_helm;
    int32_t in_warrior_mail;
    int32_t in_warrior_cuirass;
    int32_t in_warrior_bracers;
    int32_t in_warrior_gauntlets;
    int32_t in_warrior_not_used_11;
    int32_t in_warrior_boots;
    int32_t in_mage_weapon;
    int32_t in_mage_not_used_2;
    int32_t in_mage_not_used_3;
    int32_t in_mage_ring;
    int32_t in_mage_amulet;
    int32_t in_mage_hat;
    int32_t in_mage_robe;
    int32_t in_mage_cloak;
    int32_t in_mage_not_used_9;
    int32_t in_mage_gloves;
    int32_t in_mage_not_used_11;
    int32_t in_mage_shoes;
};
ASSERT_SIZE(MagicInfoData, 0x70);

class MagicInfo: public TableLine {
public:
    CArray<MagicInfoData>& Values() {
        return this->ValuesAs<MagicInfoData>();
    }

    const CArray<MagicInfoData>& Values() const {
        return this->ValuesAs<MagicInfoData>();
    }
};
ASSERT_SIZE(MagicInfo, 0x1c);

struct MonsterInfoData {
    int32_t body;
    int32_t reaction;
    int32_t mind;
    int32_t spirit;
    int32_t health_max;
    int32_t health_regeneration;
    int32_t mana_max;
    int32_t mana_regeneration;
    int32_t speed;
    int32_t rotation_speed;
    int32_t scan_range;
    int32_t physical_min;
    int32_t physical_max;
    int32_t attack_type;
    int32_t attack;
    int32_t defence;
    int32_t absorption;
    int32_t charge;
    int32_t relax;
    int32_t protection_fire;
    int32_t protection_water;
    int32_t protection_air;
    int32_t protection_earth;
    int32_t protection_astral;
    int32_t protection_blade;
    int32_t protection_axe;
    int32_t protection_bludgeon;
    int32_t protection_pike;
    int32_t protection_shooting;
    int32_t type_id;
    int32_t face;
    int32_t token_size;
    int32_t movement_type;
    int32_t dying_time;
    int32_t withdraw;
    int32_t wimpy;
    int32_t see_invisible;
    int32_t experience;
    int32_t treasure_gold;
    int32_t treasure_gold_min;
    int32_t treasure_gold_max;
    int32_t treasure_item;
    int32_t treasure_item_min;
    int32_t treasure_item_max;
    int32_t treasure_item_mask;
    int32_t not_used_0;
    int32_t not_used_1;
    int32_t power;
    int32_t spell1_id;
    int32_t spell_probability1;
    int32_t spell2_id;
    int32_t spell_probability2;
    int32_t spell3_id;
    int32_t spell_probability3;
    int32_t spell_power;
    int32_t server_id;
    int32_t known_spells_mask;
    int32_t skill_fire;
    int32_t skill_water;
    int32_t skill_air;
    int32_t skill_earth;
    int32_t skill_astral;
};
ASSERT_SIZE(MonsterInfoData, 0xf8);

class MonsterInfo: public TableLine {
public:
    CArray<MonsterInfoData>& Values() {
        return this->ValuesAs<MonsterInfoData>();
    }

    const CArray<MonsterInfoData>& Values() const {
        return this->ValuesAs<MonsterInfoData>();
    }

    CStringArray equipped_items;
};
ASSERT_SIZE(MonsterInfo, 0x30);

struct HumanInfoData {
    int32_t body;
    int32_t reaction;
    int32_t mind;
    int32_t spirit;
    int32_t health_max;
    int32_t mana_max;
    int32_t speed;
    int32_t rotation_speed;
    int32_t scan_range;
    int32_t defence;
    std::array<int32_t, 6> skills;
    int32_t type_id;
    int32_t face;
    int32_t gender;
    int32_t charge;
    int32_t relax;
    int32_t token_size;
    int32_t movement_type;
    int32_t dying_time;
    int32_t server_id;
    int32_t known_spells_mask;
};
ASSERT_SIZE(HumanInfoData, 0x68);

class HumanInfo: public TableLine {
public:
    CArray<HumanInfoData>& Values() {
        return this->ValuesAs<HumanInfoData>();
    }

    const CArray<HumanInfoData>& Values() const {
        return this->ValuesAs<HumanInfoData>();
    }

    CStringArray equipped_items;
};

struct SpellInfoData {
    int32_t level;
    int32_t mana_cost;
    int32_t sphere;
    int32_t item;
    int32_t spell_target;
    int32_t delivery_system;
    int32_t max_range;
    int32_t effect_speed;
    int32_t distribution;
    int32_t radius;
    int32_t area_effect;
    int32_t area_duration;
    int32_t area_frequency;
    int32_t unit_apply_method;
    int32_t duration;
    int32_t frequency;
    int32_t damage_min;
    int32_t damage_max;
    int32_t defensive;
    int32_t skill_offset;
    int32_t scroll_cost;
    int32_t book_cost;
};
ASSERT_SIZE(SpellInfoData, 0x58);

class SpellInfo: public TableLine {
public:
    CArray<SpellInfoData>& Values() {
        return this->ValuesAs<SpellInfoData>();
    }

    const CArray<SpellInfoData>& Values() const {
        return this->ValuesAs<SpellInfoData>();
    }

    const char* effect;
};
ASSERT_SIZE(SpellInfo, 0x20);


struct GameDataRes {
    CArray<MatShape> materials;
    CArray<MatShape> shapes;
    CArray<WorldEquip> shields;
    CArray<WorldEquip> armors;
    CArray<WorldEquip> weapons;
    CArray<MagicItem> magic_items;
    CArray<MagicInfo> magics;
    CArray<MonsterInfo> monsters;
    CArray<HumanInfo> humans;
    CArray<HumanInfo> field9_0xb4;
    CArray<BuildingInfo> buildings;
    CArray<SpellInfo> spells;
    int loaded;

public:
    int ParseWorldIn(const CString& str); //50a15f
    Item* sub_510502(CString* name); // Look up an item template by name
    int sub_5126A2(uint32_t type_low_byte, uint32_t face); // Get monster table index by type and face
    Item* sub_510EE6(uint16_t item_id); // Create a new item instance by item data ID
    int sub_50DF19(int experience); // Get random recruitable monster index for inn reward
    int sub_50DDAE(uint32_t type_id, uint32_t face); // Get monster data index from encoded item type/face

    int32_t FUN_00512625(int32_t serverid);

    uint8_t sub_50D8BA(CString* name, CString* out); // 50D8BA: Find shape index by item name string
    uint8_t sub_50DA04(CString* name, CString* out); // 50DA04: Find material index by item name string
    CString* sub_50DB4E(CString* out1, const CString* in_name, CString* base_name_out); // 50DB4E: Extract base name and braced property from item name
    void sub_50DC69(int material_id, CString* name); // 50DC69: Strip material-specific prefix from item name
};
ASSERT_OFFSET(GameDataRes, magic_items, 0x64);
ASSERT_OFFSET(GameDataRes, humans, 0xa0);
ASSERT_SIZE(GameDataRes, 0xf4);

#endif
