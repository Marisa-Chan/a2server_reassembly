#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "unit.h"

#include "building.h"
#include "buildings_list.h"
#include "effect.h"
#include "eye.h"
#include "game_app.h"
#include "inn.h"
#include "inventory.h"
#include "item.h"
#include "map_stuff.h"
#include "net.h"
#include "player.h"
#include "sack.h"
#include "server.h"
#include "shop.h"
#include "shop_assortment.h"
#include "spell.h"
#include "table.h"
#include "world.h"

//52e9e3
EquipmentExtra::EquipmentExtra() = default;
//52eaa3
EquipmentExtra& EquipmentExtra::operator=(const EquipmentExtra&) = default;

void EquipmentExtra::Serialize(CArchive& ar)
{
    //52ec3d
    if (ar.IsStoring())
    {
        //ar.Write(this, sizeof(EquipmentExtra));
        ar.Write(this, 0x40);
    }
    else
    {
        //ar.Read(this, sizeof(EquipmentExtra));
        ar.Read(this, 0x40);
    }
}

void EquipmentExtra::SetToUnit(Unit *unit)
{
    //52ea17
    unit->speed = speed;
    unit->carrying_body_100g = carrying_body_100g;
    unit->hp_max = hp_max;
    unit->mp_max = mp_max;
    unit->scan_range = scan_range;
    unit->hit_values = hit_values;
    unit->protections = protections;
}

void EquipmentExtra::AddToUnit(Unit *unit)
{
    //52eb64
    unit->speed += speed;

    if (unit->speed < 6)
        unit->speed = 6;

    unit->carrying_body_100g += carrying_body_100g;
    unit->hp_max += hp_max;
    unit->mp_max += mp_max;
    unit->scan_range += scan_range;

    unit->protections += protections;

    unit->hit_values.AddHits(hit_values);
}




/***************   Unit   *******************/
Unit::Unit()
{
    //528aef
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const CString &str)
{
    //528c97
    FUN_0052931b(str);
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const CString &str, const TokenPos *pos)
    : Token(pos)
{
    //528e24
    FUN_0052931b(str);
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const TokenPos *pos)
    : Token(pos)
{
    //528fb7
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::Unit(const TokenPos *pos, Player *player)
    : Token(pos, player)
{
    //529167
    FUN_0052931b("");
    last_hit_by = nullptr;
    last_hit_spell_id = 0;
    spell = nullptr;
}

Unit::~Unit()
{
    Unit::VMethod10();
}

// CRuntimeClass objects for building sub-types, defined in Main.asm.
extern "C" CRuntimeClass PointerRuntimeClass; // stru_637228
extern "C" CRuntimeClass ShopRuntimeClass;   // stru_637258
extern "C" CRuntimeClass InnRuntimeClass;    // stru_637330

// Free functions called from VMethod2 (all in Main.asm)
extern "C" void __cdecl sub_536630(Unit* self, Unit* target, int* out_charge); // Start attack?
extern "C" void __cdecl sub_53678F(Unit* self, Unit* target); // Execute attack?

// Free functions for Humanoid::VMethod21.
extern "C" uint32_t __cdecl sub_530726(int32_t skill_level); // Returns experience required for given skill level.
extern "C" uint32_t __cdecl sub_530DCB(uint32_t experience, int32_t skill_level); // Clamps experience gain to the max gain for the given skill level.


// 52BAD9
void Unit::VMethod1() {
    if (this->some_state == 0x10) {
        return;
    }

    if (this->hp < 0) {
        // Fallen or dead unit: decrement HP periodically.
        if (g_Server->tick16 % 4 == 0) {
            this->hp -= 1;
            g_NetStru1_main.sub_519221(this, this->pOwner, 1, 0xFFB, 0, 0);
        }
    } else if (this->hp > 0) {
        // Alive unit: HP and MP regeneration
        int32_t regen_mult = 1;

        // Higher regen for units "at rest".
        if (this->sub_52BABD() > 80) {
            regen_mult = 3;
        }

        // HP regeneration
        if (this->hp < this->hp_max && this->hp_regen != 0) {
            if (g_Server->tick16 % 4 == this->building_id % 4) {
                int32_t hp_accum = this->hp * 100 + this->hp_regen_carry;
                hp_accum += (this->hp_max * 2 * (this->equipment_extra.hp_regen + 100) * regen_mult) / this->hp_regen;
                this->hp_regen_carry = hp_accum % 100;
                this->hp = hp_accum / 100;
                if (this->hp > this->hp_max) {
                    this->hp = this->hp_max;
                }
                g_NetStru1_main.sub_519221(this, nullptr, 1, 0xFFB, 0, 0);
            }
        }

        // MP regeneration
        if (this->mp < this->mp_max) {
            if (this->mp_regen == 0) {
                this->mp_max = 0;
                this->mp = 0;
            } else {
                int32_t mp_accum = this->mp * 100 + this->mp_regen_carry;
                mp_accum += (this->mp_max * (this->equipment_extra.mp_regen + 100) * regen_mult) / this->mp_regen;
                this->mp_regen_carry = mp_accum % 100;
                this->mp = mp_accum / 100;
                if (this->mp > this->mp_max) {
                    this->mp = this->mp_max;
                }
                g_NetStru1_main.sub_519221(this, nullptr, 2, 0xFFB, 0, 0);
            }
        }
    }
}

// 52A857
void Unit::VMethod2()
{
    if (this->some_state == 0x10) {
        return;
    }

    // --- Effect processing ---
    {
        POSITION pos = this->_effects.GetHeadPosition();
        while (pos != nullptr) {
            POSITION cur_pos = pos;
            Effect* effect = this->_effects.GetNext(pos); // returns cur, advances pos to next

            effect->VMethod10(this);

            if (effect->usage_type & 0x80) {
                this->_effects.RemoveAt(cur_pos);
                delete effect;
            }
        }
    }

    // Decay.
    if (hp < 1) {
        this->state = 0;
        this->some_state = 0;
        if (this->decay == 0) {
            this->decay = 1;
            this->sub_52C813();
            this->protections.defense /= 2;
            this->charge_countdown = (int8_t)(this->VMethod25() - 1);
            g_NetStru1_main.sub_519221(this, nullptr, 0x4001, 0xffb, 0, 0);
        } else if (this->charge_countdown < 1) {
            if (this->movement_type > 1)
                this->hp = -1000;
            if (this->hp < -9) {
                this->some_state = 0x10;
                this->sub_52D94E(); // Kill the unit.
            }
        } else {
            this->charge_countdown--;
        }
        return;
    }

    if (g_Server && g_Server->field4_0x74) {
        if (this->pOwner && this == this->pOwner->main_unit && this->sub_52BABD() == 0x50) {
            g_NetStru1_main.sub_519221(this, nullptr, 0x10, 0xffb, 0, 0);
        }
    }

    uint16_t x_wide = this->position->GetXx();
    uint16_t y_wide = this->position->GetYy();
    uint8_t eye_f0 = this->eye->field0_0x0;
    uint8_t eye_f1 = this->eye->field1_0x1;

    if (this->monster_info) {
        g_World->sub_5A9B6B(this);
    }

    switch (this->some_state) {
    case 1: // Move?
        sub_52BDD7(x_wide, y_wide, eye_f0, eye_f1);
        this->some_state2 = 0;
        break;

    case 2: // Pick up sack.
        {
            Sack* sack = MapStuff_Instance->sub_58E5F3(this->position);
            if (sack) {
                g_Server->sub_4F9AD3(sack);
                MapStuff_Instance->sub_58E525(sack);
                POSITION pos = g_Server->srv_stru1->sack_list->list.Find(sack);
                if (pos) {
                    g_Server->srv_stru1->sack_list->list.RemoveAt(pos);
                }
                sub_52C98B(sack);
            }
            break;
        }

    case 3: // Attack.
    case 0xd: // Spell cast.
    case 0xe: // Area spell cast.
        {
            if (this->some_state2 == 0) {
                int32_t charge_extra = 0;
                if (this->some_state == 3) {
                    if (!weapon || !weapon->imbued_spell || !(unit_attrs & 4)) {
                        sub_536630(this, cast_target, &charge_extra);
                    } else {
                        this->some_spell = weapon->imbued_spell;
                        this->some_item  = static_cast<Item*>(weapon);
                        this->some_state = 0xd;
                    }
                }
                if (this->some_state == 0xd || this->some_state == 0xe) {
                    int32_t ok;

                    if (this->some_state == 0xd) {
                        ok = this->some_spell->sub_539958(this, this->cast_target, 0, 0);
                    } else {
                        ok = this->some_spell->sub_539958(this, nullptr, this->area_cast_x, this->area_cast_y);
                    }

                    if (ok) {
                        this->field_0x136 = 0;
                        this->some_state2 = 5;
                        this->charge_countdown = this->charge;
                    }
                } else {
                    this->field_0x136 = 0;
                    this->some_state2 = 5;
                    this->charge_countdown = (int8_t)(this->charge + charge_extra);
                }
            } else if (this->some_state2 == 5) { // Countdown to attack or spell execution.
                this->charge_countdown--;
                if (this->charge_countdown == 0) {
                    if (this->some_state == 3) {
                        if (!weapon || !weapon->imbued_spell || !(unit_attrs & 4)) {
                            sub_53678F(this, cast_target);
                        } else {
                            this->some_spell = weapon->imbued_spell;
                            this->some_item  = static_cast<Item*>(weapon);
                            this->some_state = 0xd;
                        }
                    }
                    if (this->some_state == 0xd || this->some_state == 0xe) {
                        SpellInfo* expected = &g_GameDataRes.spells[this->some_spell->spell_id];
                        if (this->some_spell->spell_info != expected) {
                            uint8_t sid = this->some_spell->spell_id;
                            if (sid == 0 || sid >= 30) {
                                LogMessage(CString("Bad spell data, casting rejected"));
                                break;
                            }
                            LogMessage(CString("Bad spell data, restoring spell"));
                            this->some_spell->spell_info = expected;
                        }
                        if (this->some_state == 0xd) {
                            this->some_spell->sub_539F21(this, this->cast_target);
                        } else {
                            this->some_spell->sub_539F5A(this, nullptr, this->area_cast_x, this->area_cast_y);
                        }

                        if (this->some_item != nullptr) {
                            if (((this->some_item->item_id >> 8) & 0xF) == 0xE) {
                                delete this->some_item;
                                delete this->some_spell;
                            }
                            this->some_item = nullptr;
                            this->some_spell = nullptr;
                        }
                    }

                    this->some_state2 = 7;
                    int32_t relax_extra = 0;
                    if (this->weapon && this->VMethod8()) {
                        relax_extra = (this->weapon->weight + (30 - (int)this->reaction) * 5) / 12;
                        if (relax_extra < 0) {
                            relax_extra = 0;
                        } else if (relax_extra > 12) {
                            relax_extra = 12;
                        }
                    }
                    this->charge_countdown = (int8_t)(this->relax + relax_extra + Random0N(3));
                    if ((this->some_state == 0xd || this->some_state == 0xe) && this->some_spell != nullptr) {
                        this->charge_countdown += (int8_t)this->some_spell->spell_info->values[0].level;
                    }
                }
            } else if (this->some_state2 == 7) { // Countdown after attack or spell execution.
                this->charge_countdown--;
                if (this->charge_countdown == 0) {
                    this->some_state2 = 0;
                    this->field_0x136 = 1;
                }
            }
            break;
        }

    case 0xf: // Interact with building
        {
            BuildingsList* building_list = g_Server->srv_stru1->building_list;

            Building* building = building_list->sub_558128(this->position);
            if (building) {
                // TODO: also check C++ runtime class.
                if (building->IsKindOf(&ShopRuntimeClass)) { // Shop
                    g_NetStru1_main.FUN_0051cefb(0x83, building->object_info_id, 0, this->pOwner);
                    break;
                }
                if (building->IsKindOf(&InnRuntimeClass)) { // Inn
                    g_NetStru1_main.FUN_0051cefb(0x84, building->object_info_id, building->building_id, this->pOwner);
                    break;
                }
                if (building->IsKindOf(&PointerRuntimeClass)) { // Pointer
                    Pointer* ptr = static_cast<Pointer*>(building);
                    if (ptr->script_instance_id > 0) {
                        g_World->DoScriptInstID(ptr->script_instance_id);
                    }
                    break;
                }
                if (building->typeId == 0x42 && g_CLlDriver.provider == 4) {
                    NetStru2* ns2 = g_NetStru1_main.GetClientByPlayerID(this->pOwner->player_id);
                    if (ns2) {
                        g_NetStru1_main.DisconnectClient(ns2);
                    }
                    g_NetStru1_main.ProcessConnections();
                    this->pOwner->field_0xa50 = (uint32_t)g_Server->tick16;
                }
                // Toggles.
                if (building->typeId == 0x1c || building->typeId == 0x1d || building->typeId == 0x4c || building->typeId == 0x4d) {
                    building->hp = (building->hp == 0) ? 1 : 0;
                    g_NetStru1_main.sub_51AC77(building, nullptr, 0);
                }
            }

            Building* near_building = building_list->sub_557EA5(this->position);
            if (near_building) {
                if (near_building->object_info_id == 0x0f && near_building->hp > 0) {
                    near_building->hp--;
                    g_NetStru1_main.sub_51AC77(near_building, nullptr, 0);
                    this->VMethod13(new Item(CString("Potion Big Healing")));
                } else if (near_building->object_info_id == 0x10 && near_building->hp > 0) {
                    near_building->hp--;
                    g_NetStru1_main.sub_51AC77(near_building, nullptr, 0);
                    this->VMethod13(new Item(CString("Potion Big Mana")));
                }
            }
            break;
        }
    }
}

int32_t Unit::VMethod3()
{
    return token_size;
}

int32_t Unit::VMethod4()
{
    return movement_type;
}

int32_t Unit::VMethod7()
{
    return 1;
}

int32_t Unit::VMethod8()
{
    return 0;
}

void Unit::VMethod10()
{
    // 52a4fa
    if (some_item && ((some_item->item_id >> 8) & 0xF) == 0xE) {
        delete some_item;
        some_item = nullptr;
        if (some_spell) {
            delete some_spell;
        }
        some_spell = nullptr;
    }

    if (eye) {
        ::operator delete(eye);
        eye = nullptr;
    }

    if (eye2) {
        eye2->sub_5A4F30();
        ::operator delete(eye2);
        eye2 = nullptr;
    }

    if (inventory) {
        delete inventory;
        inventory = nullptr;
    }

    if (shield) {
        delete shield;
        shield = nullptr;
    }

    if (weapon) {
        delete weapon;
        weapon = nullptr;
    }

    spell = nullptr;

    if (spell_book) {
        delete spell_book;
        spell_book = nullptr;
    }
}

Item *Unit::VMethod12(Item *item)
{
    Item *itm = item->VMethod10(this);
    VMethod19();
    return itm;
}

void Unit::VMethod13(Item *item)
{
    Item* itm = item->VMethod10(this);
    if (itm)
    {
        inventory->PutItemIntoBagAtDefault(itm);
        VMethod19();
    }
}

Item* Unit::Unequip(Item* item)
{
    if (item)
    {
        item->VMethod11(this);
        VMethod19();
    }
    return item;
}

void Unit::VMethod15()
{}

void Unit::VMethod16(Unit*)
{}

// 536B31
uint32_t Unit::VMethod17(UnitToHit* unit_to_hit, Unit* attacker)
{
    if (this->pOwner) {
        this->pOwner->sub_534B59();
    }

    if (unit_to_hit == nullptr || (this->unit_attrs & 8)) {
        return 0;
    }

    int32_t dmg_min = unit_to_hit->hand_damage_min;
    int32_t dmg_spread = unit_to_hit->hand_damage_spread;

    // If top bit of `hand_damage_min` is set and target is not a humanoid -> damage x15??
    if ((dmg_min & 0x80) && attacker->VMethod8() == 0) {
        dmg_min = (dmg_min & 0x7F) * 15;
        dmg_spread = dmg_spread * 15;
    }

    // Target has bless AND is a humanoid -> damage x4??
    if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
        if ((attacker->enchantments & (1u << 0x14)) && attacker->VMethod8() != 0) {
            dmg_min *= 4;
            dmg_spread *= 4;
        }
    }

    // Protection / evasion enchantment checks (only for player-owned targets).
    bool randomize = true;
    if (attacker != nullptr && attacker->pOwner != nullptr) {
        // Bless: chance to deal max damage.
        const uint32_t bless_mask = (1u << 0x14);
        if (attacker->enchantments & bless_mask) {
            Effect* effect = attacker->FindEnchantment(0x14);
            if (effect == nullptr) {
                attacker->enchantments &= ~bless_mask;
            } else {
                if (Random0N(100) < (int16_t)effect->spell_or_damage) {
                    dmg_min += dmg_spread;
                    randomize = false;
                }
            }
        }

        // Curse: chance to deal minimum damage.
        const uint32_t curse_mask = (1u << 0x1C);
        if (attacker->enchantments & curse_mask) {
            Effect* effect = attacker->FindEnchantment(0x1C);
            if (effect == nullptr) {
                attacker->enchantments &= ~curse_mask;
            } else {
                if (Random0N(100) < (int16_t)effect->spell_or_damage) {
                    randomize = false;
                }
            }
        }
    }

    int32_t preliminary_damage = dmg_min;
    if (randomize) {
        preliminary_damage += Random0N(dmg_spread);
    }

    int32_t cumulative = 0;

    // Attack roll.
    int32_t attack_roll = (int16_t)unit_to_hit->attack;
    int32_t rand_roll = Random0N(200) - 100;
    attack_roll += rand_roll;
    int32_t defense = this->protections.defense;

    // Check if attack hits.
    int32_t damage = 0;
    if (attack_roll > defense || (attacker != nullptr && (attacker->unit_attrs & 0x10))) {
        damage = preliminary_damage;
    } else if (rand_roll >= 90) {
        damage = preliminary_damage;
    }

    // Absorption and weapon-type resistance.
    if (damage > 0) {
        if (attacker == nullptr || !(attacker->unit_attrs & 0x10)) {
            damage -= this->protections.absorption;
            if (damage < 0) {
                damage = 0;
            }
        }
        uint8_t protection = this->protections.weapon_protections[unit_to_hit->physical_damage_type];
        if (protection != 0) {
            damage = (int32_t)((100.0 - protection) * (double)damage / 100.0 + 0.75);
        }
        cumulative += damage;
    }

    // Add magic damage.
    if ((int32_t)unit_to_hit->some_damage_spread + unit_to_hit->some_damage_min != 0) {
        int32_t some_dmg = (int32_t)Random0N(unit_to_hit->some_damage_spread) + unit_to_hit->some_damage_min;
        int16_t protection = this->protections.magic_protections[2];
        if (protection != 0) {
            some_dmg = (int32_t)((100.0 - protection) * (double)some_dmg / 100.0 + 0.75);
            if (some_dmg < 0) {
                some_dmg = 0;
            }
        }
        cumulative += some_dmg;
    }

    // Spell damage?
    bool no_phys = (unit_to_hit->hand_damage_min + unit_to_hit->hand_damage_spread == 0);
    if ((unit_to_hit->some_damage2_spread + unit_to_hit->some_damage2_min) != 0 && (attack_roll > defense || no_phys)) {
        int32_t spell_dmg = (int32_t)Random0N(unit_to_hit->some_damage2_spread) + unit_to_hit->some_damage2_min;
        uint8_t sphere = unit_to_hit->spell_id;
        if (sphere >= 1 && sphere <= 5) {
            int16_t protection = this->protections.magic_protections[sphere];
            if (protection != 0) {
                spell_dmg = (int32_t)((100.0 - protection) * (double)spell_dmg / 100.0 + 0.75);
                if (spell_dmg < 0) {
                    spell_dmg = 0;
                }
            }
        } else {
            LogMessage(CString("Unknown magic damage type"));
        }
        if (spell_dmg > 0) {
            cumulative += spell_dmg;
        }
    }

    // Update last-attacker and notify the world.
    if (attacker != nullptr && attacker->monster_info != nullptr) {
        if (attacker->pOwner == nullptr) {
            this->last_hit_by = nullptr;
        } else {
            this->last_hit_by = attacker;
            if (attacker->unit_attrs & 4) {
                this->last_hit_spell_id = (int8_t)unit_to_hit->spell_id;
            } else {
                this->last_hit_spell_id = 0;
            }
        }
        if (attacker->pOwner != nullptr && this->pOwner != nullptr) {
            if (g_Server->field4_0x74 == 0 || attacker->pOwner->is_ai == 0 || this->pOwner->is_ai != 0) {
                g_World->sub_5AA581(attacker, this, 0);
            }
        }
    }

    return cumulative > 0 ? cumulative : 0;
}

void Unit::VMethod18()
{
    if (!pOwner)
        mp2 = mp_max;
    else
        mp2 = (pOwner->field_0xa5c * mp_max) / 100;
}

uint32_t Unit::VMethod19()
{
    return _exp;
}

void Unit::VMethod20()
{}

// 5371F7
void Unit::VMethod21(uint32_t newExp, Unit *target, int32_t sphere)
{
    experience += newExp;
    if (target && target->hp < 0)
        pOwner->monster_kills += 1;
}

void Unit::VMethod22(Unit*, int32_t)
{}

void Unit::VMethod23(Unit*, uint32_t, int32_t)
{}

void Unit::VMethod24(Unit*, int32_t)
{}

int32_t Unit::VMethod25()
{
    return monster_info->values[0].dying_time;
}

// 52C757
Effect* Unit::FindEnchantment(uint16_t effect_id)
{
    for (POSITION p = this->_effects.GetHeadPosition(); p != nullptr; ) {
        Effect* e = this->_effects.GetNext(p);
        if ((uint16_t)e->itemDataID == effect_id) {
            return e;
        }
    }
    return nullptr;
}

// 52A215
void Unit::sub_52A215() {
    this->state = 0x0B;
    this->body = 30;
    this->reaction = 30;
    this->mind = 20;
    this->spirit = 20;
    this->speed = 10;
    this->eye->rotation_speed = 8;
    this->scan_range = 5 * 0x100;
    this->extra_carrying_weight = 0;
    this->carrying_weight_100g = 0;
    this->carrying_body_100g = this->body * 10;
    this->hp_max = 30;
    this->hp = 30;
    this->hp_regen = 100;
    this->mp = 0;
    this->mp_max = 0;
    this->mp2 = this->mp_max;
    this->mp_regen = 50;
    this->charge = 8;
    this->relax = 4;
    this->max_range = 1;
}

// 52C409
int32_t Unit::sub_52C409() {
    // First try to place unit at the exact position.
    uint8_t y = this->position->GetY();
    uint8_t x = this->position->GetX();
    if (!this->sub_52BF3D(x, y, 0)) {
        // If that fails, try to find a nearby free position (at max distance 3).
        y = this->position->GetY();
        x = this->position->GetX();
        if (!this->sub_52BF3D(x, y, 3)) {
            LogMessage(CString("Unit can't return to map - no free place"));
            return 0;
        }
    }
    dword_6CDB3C->AddTail(this);
    this->unit_attrs &= ~0x08;
    g_NetStru1_main.sub_51B870(this);
    g_NetStru1_main.sub_519221(this, nullptr, 0x10, 0xFFB, 0, 0);
    g_NetStru1_main.sub_51CF5C(this, 0, nullptr);
    return 1;
}

// 52C98B
void Unit::sub_52C98B(Sack* sack)
{
    if (sack == nullptr) {
        return;
    }

    uint32_t update_mask = 0;

    // Give gold to owner.
    if (sack->money > 0) {
        this->pOwner->FUN_00534AC1(sack->money, 0);
    }

    if (sack->inventory != nullptr) {
        // Safe iteration: cache next pointer before processing, in case node is removed.

        //while (item != nullptr) {
        for(POSITION next_node = sack->inventory->items.GetHeadPosition(); next_node != nullptr;) {
            POSITION cur_node = next_node;
            Item* item = sack->inventory->items.GetNext(next_node);

            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                bool apply_buff = false;

                // Check for consumable buff scrolls.
                if (item->world_equip->name.Find("Scroll Protection") != -1 ||
                        item->world_equip->name.Find("Scroll Bless") != -1 ||
                        item->world_equip->name.Find("Scroll Shield") != -1 ||
                        item->world_equip->name.Find("Scroll Haste") != -1 ||
                        item->world_equip->name.Find("Scroll Invisibility") != -1) {
                    sack->inventory->items.RemoveAt(cur_node);
                    apply_buff = true;
                } else if (item->world_equip->name.Find("Quest Meta") == 0) {
                    // "Quest Meta X": digit at name[10] gives per-sphere skill boost.
                    int16_t boost = (item->world_equip->name[10] - '0') * 5;
                    sack->inventory->items.RemoveAt(cur_node);
                    delete item;
                    item = nullptr;

                    Humanoid* human = static_cast<Humanoid*>(this);
                    human->experience = 0;
                    for (int sphere = 1; sphere < 6; ++sphere) {
                        int16_t new_lvl = (std::min)(human->hit_values.skill_levels[sphere] + boost, 100);
                        human->hit_values.skill_levels[sphere] = new_lvl;
                        human->hit_values2.skill_levels[sphere] = new_lvl;
                        int32_t exp_val = sub_530726(new_lvl);
                        human->experience_per_sphere[sphere - 1] = exp_val;
                        human->experience += exp_val;
                    }
                    human->VMethod18();
                    update_mask = 0x31f0f;
                    g_NetStru1_main.FUN_0051ce86(9, boost, human->pOwner);
                } else {
                    item->TokenID = 1;
                }

                // In team mode, discard buff if this player is the current rune-holder.
                if (g_ServerConfig.gameType == 2 && apply_buff && item != nullptr) {
                    uint32_t player_id = (this->pOwner->field_0xa70 == 0) ? g_Server->field61_0x210 : g_Server->field60_0x20c;
                    if (player_id == this->pOwner->player_id) {
                        apply_buff = false;
                        delete item;
                        item = nullptr;
                    }
                }

                // Apply buff scroll effect to self.
                if (apply_buff && item != nullptr) {
                    Effect* effect = item->_effects.m_nCount > 0 ? item->_effects.m_pNodeHead->data : nullptr;
                    if (effect != nullptr) {
                        this->some_item = item;
                        Spell* spl = new Spell(static_cast<uint8_t>(effect->spell_or_damage));
                        spl->sub_539541(static_cast<uint8_t>(effect->spell_value));
                        spl->sub_539F5A(this, this, 0, 0);
                        if (this->some_item != nullptr) {
                            delete this->some_item;
                            this->some_item = nullptr;
                        }
                    }
                    item = nullptr;
                }

                // Quest rune logic (team deathmatch only).
                if (g_ServerConfig.gameType == 2 && item != nullptr) {
                    bool rune_scored = false;

                    if (item->world_equip->name.Find("Quest RuneF") != -1) {
                        // Original logic: remove item from inventory and call `sub_57b990`. Looks equivalent to calling `items.RemoveAt`.
                        sack->inventory->items.RemoveAt(cur_node);

                        delete item;
                        item = nullptr;

                        if (this->pOwner->field_0xa70 == 0) {
                            g_Server->sub_4F8FBF(0, 0);
                            g_NetStru1_main.FUN_0051ce86(0x100, 0, nullptr);
                        } else {
                            rune_scored = true;
                            g_Server->field60_0x20c = this->pOwner->player_id;
                            g_Server->field62_0x214 = 0;
                            g_NetStru1_main.FUN_0051d6b4(0);
                            g_NetStru1_main.FUN_0051ce86(0x102, this->pOwner->player_id, nullptr);
                        }
                    } else if (item->world_equip->name.Find("Quest RuneA") != -1) {
                        sack->inventory->items.RemoveAt(cur_node);
                        delete item;
                        item = nullptr;
                        if (this->pOwner->field_0xa70 == 1) {
                            g_Server->sub_4F8FBF(1, 0);
                            g_NetStru1_main.FUN_0051ce86(0x101, 0, nullptr);
                        } else {
                            rune_scored = true;
                            g_Server->field61_0x210 = this->pOwner->player_id;
                            g_Server->field63_0x218 = 0;
                            g_NetStru1_main.FUN_0051d6b4(0);
                            g_NetStru1_main.FUN_0051ce86(0x103, this->pOwner->player_id, nullptr);
                        }
                    }

                    // Rune scored: flush all active effects on this unit.
                    if (rune_scored) {
                        POSITION effect_pos = this->_effects.GetHeadPosition();
                        while (effect_pos != nullptr) {
                            POSITION cur_pos = effect_pos;
                            Effect* effect = this->_effects.GetNext(effect_pos);

                            effect->spell_value = 1;
                            effect->caster = 0;
                            effect->VMethod10(this);

                            if (effect->usage_type & 0x80) {
                                this->_effects.RemoveAt(cur_pos);
                                delete effect;
                            }
                        }
                    }
                }
            }
        } // end item loop

        // Transfer all remaining items from sack to this unit's inventory.
        this->inventory->sub_552A42(sack->inventory);
        sack->inventory = nullptr;
        this->sub_52A790(0); // Add weight.
    } // end if sack->inventory

    g_NetStru1_main.sub_51C61E(sack);
    delete sack;

    g_NetStru1_main.sub_519221(this, nullptr, update_mask | 0x282000, 0xffb, 0, 0);
}

// 52D94E
void Unit::sub_52D94E()
{
    // Summoned units: remove completely, don't leave a corpse.
    if (this->summon_id != 0) {
        this->hp = -10000;
        this->summon_id = 0;
    }

    this->some_state = 0x10;
    this->state = 0x10;

    MapStuff_Instance->sub_58E3D1(this);

    // Inline sub_58FD16: reset subcell position to cell center.
    this->position->x_subcell = 0x80;
    this->position->y_subcell = 0x80;

    if (g_ServerConfig.gameType == 0 || (g_ServerConfig.gameType == 3 && this->typeId >= 0x40)) {
        if (this->shield != nullptr) {
            this->inventory->PutItemIntoBagAtDefault(this->Unequip(this->shield));
        }

        // Unequip weapon if it has a non-zero 'other_param' (why?).
        if (this->weapon != nullptr && this->weapon->world_equip->values.GetData()[0].other_param != 0) {
            this->inventory->PutItemIntoBagAtDefault(this->Unequip(this->weapon));
        }

        // VMethod15: unequip all remaining equipped items.
        this->VMethod15();

        bool is_npc = (this->monster_info->name.Find("NPC") != -1);
        bool should_drop = false;

        if (this->typeId >= 0x40) {
            int roll = Random0N(99); // [0, 99]
            should_drop = (roll < 100 - g_ServerConfig.field_0xc4);
        }

        // Reset inventory if NPC, summoned, or flagged for drop.
        if (is_npc || this->summoned != 0 || should_drop) {
            delete this->inventory;
            this->inventory = new Inventory();
        }

        int32_t gold = 0;

        if (this->typeId >= 0x40 && this->summoned == 0) {
            MonsterInfoData& monster_data = this->monster_info->values.GetData()[0];

            // Gold drop.
            if (Random0N(100) < monster_data.treasure_gold) {
                gold = monster_data.treasure_gold_min + Random0N(monster_data.treasure_gold_max);
            }

            // Item drop.
            if (Random0N(100) < monster_data.treasure_item) {
                ShopAssortment assort;

                AssortGenParams params;
                params.min_cost = monster_data.treasure_item_min;
                params.max_cost = monster_data.treasure_item_max;
                params.max_count = 100;
                params.max_same_count = 1;
                params.flags = monster_data.treasure_item_mask;

                assort.GenerateAssortment(&params);

                CArray<Item*> result;
                assort.ArrangeShelfs(100, 1, params.min_cost, params.max_cost, &result);

                if (result.GetSize() > 0) {
                    int idx = Random0N(result.GetSize() - 1); // [0, m_nSize-1]
                    Item* item = result[idx];
                    item->count = 1;
                    this->inventory->PutItemIntoBagAtDefault(item);
                }
                // assort destructor deletes all items in assort.items.
                // result destructor frees the pointer array (items not deleted here).
            }
        }

        // Create sack if inventory has items or gold was dropped.
        if ((this->inventory->items.m_nCount > 0 || gold > 0) && this->pOwner != nullptr) {
            int is_main = (this->pOwner->main_unit == this) ? 1 : 0;
            // Inline sub_52D8D3: drop inventory as a sack on the map.
            g_Server->srv_stru1->sack_list->sub_554927(this->position, this->inventory, gold, is_main);
            delete this->inventory;
            this->inventory = new Inventory();
        }
    } else if (this->pOwner != nullptr && g_ServerConfig.gameType != 3) {
        if (g_ServerConfig.gameType == 2) {
            // Check if this unit's player is holding the opposing team's rune.
            int32_t opposite_team = 1 - this->pOwner->field_0xa70;
            int32_t rune_holder = (opposite_team == 0)
                ? g_Server->field60_0x20c
                : g_Server->field61_0x210;

            if (rune_holder == this->pOwner->player_id) {
                // Drop the appropriate rune item.
                Item* rune;
                if (this->pOwner->field_0xa70 == 0) {
                    // Blue team was holding red's rune.
                    rune = new Item(CString("Quest RuneA"));
                } else {
                    // Red team was holding blue's rune.
                    rune = new Item(CString("Quest RuneF"));
                }
                this->inventory->PutItemIntoBagAtDefault(rune);

                // Clear rune-holder state and flag rune as on the ground.
                if (opposite_team == 0) {
                    g_Server->field60_0x20c = 0;
                    g_Server->field62_0x214 = 1;
                } else {
                    g_Server->field61_0x210 = 0;
                    g_Server->field63_0x218 = 1;
                }

                g_NetStru1_main.FUN_0051ce86(this->pOwner->field_0xa70 ? 0x104 : 0x105, this->pOwner->player_id, nullptr);
                g_NetStru1_main.FUN_0051d6b4(0);
            }
        }

        // Create sack if inventory has items.
        if (this->inventory->items.m_nCount > 0) {
            int is_main = (this->pOwner->main_unit == this) ? 1 : 0;
            g_Server->srv_stru1->sack_list->sub_554927(this->position, this->inventory, 0, is_main);
            delete this->inventory;
            this->inventory = new Inventory();
        }
    }

    {
        POSITION pos = this->_effects.GetHeadPosition();
        while (pos != nullptr) {
            POSITION cur_pos = pos;
            Effect* effect = this->_effects.GetNext(pos);

            // Inline sub_53EE08: check `usage_type & 3`.
            if (effect->usage_type & 3) {
                effect->spell_value = 1;
                effect->VMethod10(this);

                if (effect->usage_type & 0x80) {
                    this->_effects.RemoveAt(cur_pos);
                    delete effect;
                }
            }
        }
    }

    // Hero-death: increment death counter and notify clients.
    if (this->pOwner != nullptr && this->pOwner->main_unit == this) {
        this->pOwner->deaths += 1;
        g_NetStru1_main.sub_519221(this, this->pOwner, 0x2400000, 0xFFB, 0, 0);
        g_Server->sub_4EE028(this->pOwner->main_unit);
        this->hp = -50;
    }

    // Respawn notification for player character types (typeId 0x21-0x3F).
    if (!g_Server->field4_0x74 && this->pOwner != nullptr && !this->pOwner->is_ai && 0x21 <= this->typeId && this->typeId <= 0x3F) {
        this->pOwner->field_0x40 = (uint8_t)(this->server_id - 0x13);
        g_NetStru1_main.FUN_0051cefb(0xb4, this->pOwner->field_0x40, 0, this->pOwner);
    }

    this->sub_52E7FA();
}

IMPLEMENT_SERIAL(Unit, Token, 1);

/*  Created by IMPLEMENT_SERIAL
// 6363e8
AFX_DATA CRuntimeClass Unit::classUnit =
{ "Unit", sizeof(Unit), 1, &Unit::CreateObject, &Token::classCObject, NULL };

CObject* PASCAL Unit::CreateObject() // 528a35
CRuntimeClass* Unit::GetRuntimeClass() const //528aa5

CArchive& AFXAPI operator>>(CArchive& ar, Unit* &pOb) //528ad3

//528ab5
static const AFX_CLASSINIT _init_Unit((CRuntimeClass*)&Unit::classUnit);
*/







/***************   Humanoid   *******************/

IMPLEMENT_SERIAL(Humanoid, Unit, 1);
/*
* Humanoid::classHumanoid   636400
* CreateObject     52fee4
* GetRuntimeClass  52ff54
* _init_Humanoid   52ff64
* operator>>       52ff82
*/


Humanoid::Humanoid()
{
    //52ff9e
    InitStats();
}

Humanoid::Humanoid(const TokenPos *tpos)
 :Unit(tpos)
{
    //52fff2
    InitStats();
}

Humanoid::~Humanoid()
{
    //53015b
    Humanoid::VMethod10();
}

void Humanoid::VMethod5()
{
    //55ab26
    Unit::VMethod5();
}

int32_t Humanoid::VMethod7()
{
    //57bb50
    return 1;
}

int32_t Humanoid::VMethod8()
{
    //57bb60
    return 1;
}

void Humanoid::VMethod10()
{
    //5301ac
    Unit::VMethod10();
    for (int i = 1; i < equipment.size(); i++)
    {
        Item*& rEq = equipment[i];
        if (rEq)
        {
            delete rEq;
            rEq = nullptr;
        }
    }
}



void Humanoid::VMethod13(Item *item)
{
    //5304b2
    Item *itm = VMethod12(item);
    if (itm)
        inventory->PutItemIntoBagAtDefault(itm);
}

Item *Humanoid::Unequip(Item* item)
{
    //5304e8
    if (item)
        item->VMethod11(this);
    return item;
}


void Humanoid::VMethod15()
{
    //530511
    for (int i = 1; i < equipment.size(); i++)
    {
        Item *itm = equipment[i];
        Unequip(itm);
        inventory->PutItemIntoBagAtDefault(itm);
    }
}

/*void Humanoid::VMethod16(Unit* unit)
{
    //53111b
    
}*/

void Humanoid::VMethod18()
{
    // 005315ec
    const bool fighter = (unit_attrs & 4) == 0;
    const bool female = (typeId == 0x22 || typeId == 0x24);

    uint16_t max_body, max_reaction, max_mind, max_spirit;

    if (fighter) {
        if (female) {
            max_body = 50;
            max_reaction = 52;
            max_mind = 46;
            max_spirit = 48;
        } else {
            max_body = 52;
            max_reaction = 50;
            max_mind = 48;
            max_spirit = 46;
        }
    } else {
        if (female) {
            max_body = 46;
            max_reaction = 48;
            max_mind = 50;
            max_spirit = 52;
        } else {
            max_body = 48;
            max_reaction = 46;
            max_mind = 52;
            max_spirit = 50;
        }
    }

    body = min(body, max_body + equipment_extra.body);
    reaction = min(reaction, max_reaction + equipment_extra.reaction);
    mind = min(mind, max_mind + equipment_extra.mind);
    spirit = min(spirit, max_spirit + equipment_extra.spirit);

    hp_max = static_cast<int16_t>(body * (fighter ? 2 : 1));
    if (hp_max > 0) {
        const double exp_factor = std::log(static_cast<double>(experience) / 5000.0 + 1.0) / std::log(1.1);
        hp_max = static_cast<int16_t>(hp_max + (fighter ? 2.0 : 1.0) * exp_factor);
        const double body_factor = std::pow(1.1, static_cast<double>(body)) / 100.0 + 1.0;
        hp_max = static_cast<int16_t>(hp_max * body_factor);
    }

    if (mp_max == 0) {
        mp = 0;
    } else {
        mp_max = static_cast<int16_t>(spirit * 2);
        const double exp_factor = std::log(static_cast<double>(experience) / 5000.0 + 1.0) / std::log(1.1);
        mp_max = static_cast<int16_t>(mp_max + (fighter ? 1.0 : 2.0) * exp_factor);
        const double spirit_factor = std::pow(1.1, static_cast<double>(spirit)) / 100.0 + 1.0;
        mp_max = static_cast<int16_t>(mp_max * spirit_factor);
    }

    scan_range = static_cast<uint16_t>(((mind + reaction) / 25.0 + 4.0) * 256.0);
    carrying_body_100g = static_cast<uint16_t>(body * 10 + 1);

    speed = (reaction < 12) ? reaction : reaction / 5 + 12;
    if (typeId == 0x13 || typeId == 0x15) { // Horse-riders, with sword and with lance.
        speed += 10;
    }

    carrying_weight_100g = extra_carrying_weight;
    if (inventory) {
        if (inventory->total_weight < 64000) {
            carrying_weight_100g = static_cast<uint16_t>(carrying_weight_100g + (inventory->total_weight / 2));
        }  else {
            carrying_weight_100g = 32000;
        }
    }

    if (carrying_body_100g != 0 && carrying_body_100g <= carrying_weight_100g) {
        const uint16_t penalty = carrying_weight_100g / carrying_body_100g;
        const int16_t adjusted = static_cast<int16_t>(speed) - penalty;
        speed = static_cast<uint16_t>(adjusted < 6 ? 6 : adjusted);
    }

    const double body_pow = std::pow(1.1, static_cast<double>(body));
    hit_values.hand_damage_spread = static_cast<uint8_t>(body_pow / 20.0);
    hit_values.hand_damage_min = hit_values.hand_damage_spread;

    const double reaction_pow = std::pow(1.1, static_cast<double>(reaction));
    hit_values.attack = static_cast<uint16_t>((body_pow + reaction_pow) / 5.0);

    for (int i = 1; i < static_cast<int>(hit_values.skill_levels.size()); i++) {
        hit_values.skill_levels[i] = hit_values2.skill_levels[i];
    }

    for (int i = 1; i < static_cast<int>(hit_values.skill_levels.size()); i++) {
        if (hit_values.skill_levels[i] > 100) {
            hit_values.skill_levels[i] = 100;
        }
    }

    if (hit_values.physical_damage_type != 0 && hit_values.physical_damage_type < hit_values.skill_levels.size()) {
        const uint16_t lvl = hit_values.skill_levels[hit_values.physical_damage_type];
        hit_values.attack = static_cast<uint16_t>(hit_values.attack + lvl * 3);
        hit_values.hand_damage_min = static_cast<uint8_t>(hit_values.hand_damage_min + (lvl / 5));
    }

    hit_values.some_damage_min = 0;
    hit_values.some_damage_spread = 0;
    hit_values.some_damage2_min = 0;
    hit_values.some_damage2_spread = 0;

    protections.Clear();
    protections.defense = reaction / 3;
    for (size_t i = 1; i < protections.magic_protections.size(); i++) {
        protections.magic_protections[i] = spirit / 2;
    }

    equipment_extra.AddToUnit(this);

    if (hp > hp_max) {
        hp = hp_max;
    }

    if (mp > mp_max) {
        mp = mp_max;
    }
    if (mp < 0) {
        mp = 0;
    }

    if (!pOwner) {
        mp2 = mp_max;
    } else {
        mp2 = static_cast<int16_t>((pOwner->field_0xa5c * mp_max) / 100);
    }

    if (eye) {
        eye->rotation_speed = static_cast<uint8_t>(speed);
    }

    protections.defense = max(0, protections.defense);
    protections.absorption = max(0, protections.absorption);
    carrying_weight_100g = max(0, carrying_weight_100g);

    const uint16_t max_magic = spirit / 2 + 70;
    const uint16_t ceiling = min(100, max_magic);
    for (size_t i = 1; i < protections.magic_protections.size(); i++) {
        uint16_t value = protections.magic_protections[i];
        protections.magic_protections[i] = min(value, ceiling);
    }

    if (spell_book != nullptr) {
        spell_book->RefreshForHumanoid(this);
    }
}

uint32_t Humanoid::VMethod19()
{
    //537199
    uint32_t exps = 0;
    for (uint32_t ex : experience_per_sphere)
        exps += ex;

    _exp = (exps * 0.01);
    return _exp;
}

// 530745
void Humanoid::VMethod21(uint32_t new_exp, Unit* target, int32_t sphere)
{
    if (g_ServerConfig.gameType != 0) {
        return;
    }

    if (this->typeId <= 0x20 || this->typeId >= 0x40) {
        return;
    }

    if (target != nullptr && target->summoned != 0) {
        return;
    }

    new_exp = static_cast<uint32_t>(new_exp * ((double)this->mind / 30.0 + 0.25));

    // Diminishing returns: reduce XP if this unit was killed multiple times this session.
    if (target != nullptr) {
        for (int i = 0; i < this->mob_kills_in_session.GetSize(); i++) {
            if (this->mob_kills_in_session[i].server_id == target->server_id) {
                new_exp = (new_exp * 10) / (this->mob_kills_in_session[i].kills + 10);
                break;
            }
        }
    }

    if (target != nullptr && target->hp >= 0) {
        if (target->pOwner == this->pOwner || this->pOwner == nullptr) {
            return;
        }
        if (g_World != nullptr && (g_World->diplomacy[target->pOwner->player_id][this->pOwner->player_id] & 2) != 0) {
            return;
        }
    }

    // Sync current active skill levels from the base values before processing.
    for (int i = 1; i <= 5; i++) {
        this->hit_values.skill_levels[i] = this->hit_values2.skill_levels[i];
    }

    bool leveled_up = false;
    uint32_t update_mask = 0;

    if ((this->unit_attrs & 4) == 0) {
        // Fighter: physical attack XP goes to the weapon combat sphere.
        int phys_sphere = this->hit_values.physical_damage_type;
        if (sphere == 0 && phys_sphere != 0 && this->hit_values.skill_levels[phys_sphere] < 100) {
            // Secondary spheres gain XP at 1/8 the normal rate.
            if (phys_sphere != this->main_sphere && phys_sphere != 5) {
                new_exp = (new_exp + 7) / 8;
            }

            new_exp = sub_530DCB(new_exp, this->hit_values.skill_levels[phys_sphere]);
            this->experience_per_sphere[phys_sphere - 1] += new_exp;
            this->experience += new_exp;

            leveled_up = sub_530726(this->hit_values.skill_levels[phys_sphere] + 1) <= this->experience_per_sphere[phys_sphere - 1];
            update_mask = 0x100u << (phys_sphere - 1);

            if (leveled_up) {
                for (int i = 1; i <= 5; i++) {
                    this->hit_values.skill_levels[i] = this->hit_values2.skill_levels[i];
                }
                this->hit_values.skill_levels[phys_sphere]++;
                for (int i = 1; i <= 5; i++) {
                    this->hit_values2.skill_levels[i] = this->hit_values.skill_levels[i];
                }
                if (this->pOwner->is_ai == 0) {
                    g_NetStru1_main.FUN_0051ce86(2, phys_sphere | ((uint32_t)this->server_id << 16), this->pOwner);
                }
            }
        }
    } else {
        // Mage: XP goes to the specified sphere.
        if (sphere > 0 && this->hit_values.skill_levels[sphere] < 100) {
            // Secondary spheres gain XP at 1/8 the normal rate.
            if (sphere != this->main_sphere && sphere != 5) {
                new_exp = (new_exp + 7) / 8;
            }

            new_exp = sub_530DCB(new_exp, this->hit_values.skill_levels[sphere]);
            this->experience_per_sphere[sphere - 1] += new_exp;
            this->experience += new_exp;

            leveled_up = sub_530726(this->hit_values.skill_levels[sphere] + 1) <= this->experience_per_sphere[sphere - 1];
            update_mask = 0x100u << (sphere - 1);

            if (leveled_up) {
                for (int i = 1; i <= 5; i++) {
                    this->hit_values.skill_levels[i] = this->hit_values2.skill_levels[i];
                }
                this->hit_values.skill_levels[sphere]++;
                if (this->pOwner->is_ai == 0) {
                    g_NetStru1_main.FUN_0051ce86(2, sphere | ((uint32_t)this->server_id << 16), this->pOwner);
                }
                for (int i = 1; i <= 5; i++) {
                    this->hit_values2.skill_levels[i] = this->hit_values.skill_levels[i];
                }
                this->spell_book->RefreshForHumanoid(this);
            }
        }
    }

    // Post-processing: recalculate stats and notify clients.
    if (leveled_up) {
        this->VMethod18();
        g_NetStru1_main.sub_519221(this, nullptr, update_mask | 0x3000C, 0xFFB, 0, 0);
    } else {
        if (new_exp != 0) {
            g_NetStru1_main.sub_519221(this, nullptr, update_mask, 0xFFB, 0, 0);
        }
        // Apply equipment bonuses on top of base skill levels.
        for (int i = 1; i <= 5; i++) {
            int16_t base = (this->hit_values.skill_levels[i] < 100) ? this->hit_values.skill_levels[i] : 100;
            this->hit_values.skill_levels[i] = base + this->equipment_extra.hit_values.skill_levels[i];
        }
    }

    // Trigger periodic save when XP is gained in tournament mode.
    if (new_exp != 0 && g_Server->field4_0x74 != 0 && GetTickCount() - this->pOwner->field_0xa7c > 15000) {
        g_Server->sub_4EE028(this);
    }
}

/*
int32_t Humanoid::VMethod25()
{
    //530394
    if ()
}
*/




/***************   Human   *******************/

IMPLEMENT_SERIAL(Human, Humanoid, 1);
/*
* Human::classHuman   636418
* CreateObject     532273
* GetRuntimeClass  5322e3
* _init_Human      5322f3
* operator>>       532311
*/

Human::Human()
{
    //53232d
    FUN_00532587(CString("Man_Unarmed"), 0, nullptr);
}

Human::Human(const CString& hname, int32_t t, const char* unk)
{
    //53239a
    FUN_00532587(hname, t, unk);
}

Human::~Human()
{}

// 532DDE
void Humanoid::sub_532dde(CArray<HumanInfoData>* data)
{
    HumanInfoData& d = data->GetData()[0];

    if (d.body != -1) {
        this->body = static_cast<uint16_t>(d.body);
    }
    if (d.reaction != -1) {
        this->reaction = static_cast<uint16_t>(d.reaction);
    }
    if (d.mind != -1) {
        this->mind = static_cast<uint16_t>(d.mind);
    }
    if (d.spirit != -1) {
        this->spirit = static_cast<uint16_t>(d.spirit);
    }
    if (d.health_max != -1) {
        this->hp_max = static_cast<int16_t>(d.health_max);
    }
    this->hp = this->hp_max;
    if (d.mana_max != -1) {
        this->mp_max = static_cast<int16_t>(d.mana_max);
    }
    this->mp = this->mp_max;
    if (d.speed != -1) {
        this->speed = static_cast<uint16_t>(d.speed);
    }
    if (d.rotation_speed != -1) {
        this->eye->rotation_speed = static_cast<uint8_t>(d.rotation_speed);
    }
    if (d.scan_range != -1) {
        this->scan_range = d.scan_range * 256;
    }
    if (d.defence != -1) {
        this->protections.defense = static_cast<int16_t>(d.defence);
    }

    for (int i = 0; i < 6; i++) {
        if (d.skills[i] != -1) {
            this->hit_values.skill_levels[i] = static_cast<uint16_t>(d.skills[i]);
        }
    }
    this->hit_values.attack = 0;

    bool is_hero = this->monster_info->name.Find("_Hero") != -1 || this->monster_info->name.Find("Start_") != -1;

    int best_sphere = 0;
    int best_val = 0;
    for (int i = 1; i <= 5; i++) {
        this->hit_values2.skill_levels[i] = this->hit_values.skill_levels[i];
        int sl = static_cast<int16_t>(this->hit_values.skill_levels[i]);
        if (sl > best_val) {
            best_sphere = i;
            best_val = sl;
        }
    }
    if (is_hero) {
        this->main_sphere = best_sphere;
    }

    if (d.type_id != -1) {
        this->typeId = static_cast<uint16_t>(d.type_id);
    }
    if (d.face != -1) {
        this->face = static_cast<int8_t>(d.face);
    }
    // d.gender is not assigned to any field
    if (d.charge != -1) {
        this->charge = static_cast<uint8_t>(d.charge);
    }
    if (d.relax != -1) {
        this->relax = static_cast<uint8_t>(d.relax);
    }
    if (d.token_size != -1) {
        this->token_size = static_cast<int8_t>(d.token_size);
    }
    if (d.movement_type != -1) {
        this->movement_type = static_cast<int8_t>(d.movement_type);
    }
}
