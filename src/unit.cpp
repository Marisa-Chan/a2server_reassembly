#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "unit.h"

#include "building.h"
#include "buildings_list.h"
#include "constants.h"
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
#include "virtual_caster.h"
#include "world.h"

extern "C" CString MALE_HUMAN_NAMES[10];
extern "C" CString FEMALE_HUMAN_NAMES[7];  // Female human names.

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


// Free functions called from VMethod2 (all in Main.asm)
extern "C" void __cdecl sub_536630(Unit* self, Unit* target, int* out_charge); // Start attack?
extern "C" void __cdecl sub_53678F(Unit* self, Unit* target); // Execute attack?

// Free functions for Humanoid::VMethod21.
extern "C" uint32_t __cdecl sub_530DCB(uint32_t experience, int32_t skill_level); // Clamps experience gain to the max gain for the given skill level.


// 559393
void Unit::Serialize(CArchive& ar)
{
    Token::Serialize(ar);

    // Serialize effects.
    if (ar.IsStoring()) {
        ar << this->_effects.m_nCount;
        for (POSITION pos = this->_effects.GetHeadPosition(); pos != nullptr; ) {
            Effect* effect = this->_effects.GetNext(pos);
            ar.WriteObject(effect);
        }
    } else {
        this->_effects.RemoveAll();
        int32_t count;
        ar >> count;
        for (int32_t i = 0; i < count; i++) {
            Effect* effect = nullptr;
            ar >> effect;
            this->_effects.AddTail(effect);
        }
    }

    this->list1.Serialize(ar);
    this->list2.Serialize(ar);

    this->hit_values.Serialize(ar);
    this->protections.Serialize(ar);
    this->hit_values2.Serialize(ar);
    this->equipment_extra.Serialize(ar);

    // Serialize UnitEye (0xB4 bytes raw).
    if (ar.IsStoring()) {
        ar.Write(this->eye, sizeof(UnitEye));
    } else {
        ar.Read(this->eye, sizeof(UnitEye));
    }

    // Serialize UnitEye2 (0xB8 bytes raw + positions_list)
    if (ar.IsStoring()) {
        ar.Write(this->eye2, sizeof(UnitEye2));
        this->eye2->positions_list->Serialize(ar);
    } else {
        ar.Read(this->eye2, sizeof(UnitEye2));
        this->eye2->positions_list = new CList<uint16_t>();
        this->eye2->positions_list->Serialize(ar);
    }

    if (ar.IsStoring()) {
        ar.Write(&this->token_size, 1);
        ar.Write(&this->movement_type, 1);
        ar.Write(&this->face, 1);
        ar.Write(&this->unit_attrs, 1);
        ar.Write(&this->state, 4);
        ar.Write(&this->some_state, 4);
        ar.Write(&this->some_state2, 4);
        ar.Write(&this->area_cast_x, 1);
        ar.Write(&this->area_cast_y, 1);
        ar.Write(&this->charge_countdown, 1);
        ar.WriteObject(this->weapon);
        ar.WriteObject(this->shield);
        ar << this->name;
        ar << this->body;
        ar << this->reaction;
        ar << this->mind;
        ar << this->spirit;
        ar << this->speed;
        ar << this->extra_carrying_weight;
        ar << this->carrying_weight_100g;
        ar << this->carrying_body_100g;
        ar << this->hp;
        ar << this->hp_max;
        ar << this->hp_regen;
        ar << this->mp;
        ar << this->mp_max;
        ar << this->mp_regen;
        ar << this->hp_regen_carry;
        ar << this->mp_regen_carry;
        ar << this->mp2;
        ar << this->scan_range;
        ar << this->max_range;
        ar << this->experience;
        ar << this->charge;
        ar << this->relax;
        ar << this->field_0x136;
        ar << this->last_action_tick;
        ar << this->decay;
        uint32_t packed = this->server_id
            | ((this->field_0x204 & 0xFF) << 16)
            | ((this->summoned & 0xFF) << 24);
        ar << packed;
        ar << this->enchantments;
        ar.WriteObject(this->some_item);
        if (this->inventory != nullptr) {
            ar << (uint8_t)1;
            this->inventory->sub_55CCE4(ar);
        } else {
            ar << (uint8_t)0;
        }
        if (this->spell_book != nullptr) {
            ar << (uint8_t)1;
            this->spell_book->Serialize(ar);
        } else {
            ar << (uint8_t)0;
        }
        ar << (uint32_t)this->cast_target;
        ar << (uint32_t)this->some_spell;
        ar << (uint32_t)this->spell;
        ar << (uint32_t)this->last_hit_by;
        ar.Write(&this->last_hit_spell_id, 1);
    } else {
        ar.Read(&this->token_size, 1);
        ar.Read(&this->movement_type, 1);
        ar.Read(&this->face, 1);
        ar.Read(&this->unit_attrs, 1);
        ar.Read(&this->state, 4);
        ar.Read(&this->some_state, 4);
        ar.Read(&this->some_state2, 4);
        ar.Read(&this->area_cast_x, 1);
        ar.Read(&this->area_cast_y, 1);
        ar.Read(&this->charge_countdown, 1);
        this->weapon = (Weapon*)ar.ReadObject(RUNTIME_CLASS(Item));
        this->shield = (Shield*)ar.ReadObject(RUNTIME_CLASS(Item));
        ar >> this->name;
        ar >> this->body;
        ar >> this->reaction;
        ar >> this->mind;
        ar >> this->spirit;
        ar >> this->speed;
        ar >> this->extra_carrying_weight;
        ar >> this->carrying_weight_100g;
        ar >> this->carrying_body_100g;
        ar >> this->hp;
        ar >> this->hp_max;
        ar >> this->hp_regen;
        ar >> this->mp;
        ar >> this->mp_max;
        ar.Read(&this->mp_regen, 2);
        ar >> this->hp_regen_carry;
        ar >> this->mp_regen_carry;
        ar.Read(&this->mp2, 2);
        ar.Read(&this->scan_range, 2);
        ar >> this->max_range;
        ar.Read(&this->experience, 4);
        ar >> this->charge;
        ar >> this->relax;
        ar >> this->field_0x136;
        ar.Read(&this->last_action_tick, 4);
        ar >> this->decay;
        uint32_t packed;
        ar.Read(&packed, 4);
        ar.Read(&this->enchantments, 4);
        this->some_item = (Item*)ar.ReadObject(RUNTIME_CLASS(Item));
        uint8_t has_inventory;
        ar >> has_inventory;
        if (has_inventory) {
            this->inventory->sub_55CCE4(ar);
        }
        uint8_t has_spell_book;
        ar >> has_spell_book;
        if (has_spell_book) {
            this->spell_book = new SpellBook();
            this->spell_book->Serialize(ar);
        }
        ar.Read(&this->cast_target, 4);
        ar.Read(&this->some_spell, 4);
        ar.Read(&this->spell, 4);
        ar.Read(&this->last_hit_by, 4);
        ar.Read(&this->last_hit_spell_id, 1);
        this->server_id = (uint16_t)(packed & 0xFFFF);
        this->field_0x204 = (packed & 0xFF0000) != 0 ? 1 : 0;
        this->summoned = (packed & 0xFF000000) != 0 ? 1 : 0;
        if (!this->VMethod8()) {
            this->monster_info = &g_GameDataRes.monsters[this->itemDataID];
        } else {
            this->monster_info = nullptr;
        }
    }
}


// 55A9C5
void Unit::VMethod5()
{
    Token::VMethod5();

    void* result;

    if (g_Server->field23_0xdc.Lookup(this->cast_target, result)) {
        this->cast_target = (Unit*)result;
    } else {
        this->cast_target = nullptr;
    }

    if (g_Server->field23_0xdc.Lookup(this->some_spell, result)) {
        this->some_spell = (Spell*)result;
    } else {
        this->some_spell = nullptr;
    }

    if (g_Server->field23_0xdc.Lookup(this->spell, result)) {
        this->spell = (Spell*)result;
    } else {
        this->spell = nullptr;
    }

    if (g_Server->field23_0xdc.Lookup(this->some_item, result)) {
        this->some_item = (Item*)result;
    } else {
        this->some_item = nullptr;
    }

    if (g_Server->field23_0xdc.Lookup(this->last_hit_by, result)) {
        this->last_hit_by = (Unit*)result;
    } else {
        this->last_hit_by = nullptr;
    }

    // UnitEye pointer fixup (sub_5941C1)
    if (this->eye->field120_0x7c != nullptr) {
        if (g_Server->field23_0xdc.Lookup(this->eye->field120_0x7c, result)) {
            this->eye->field120_0x7c = (Unit*)result;
        }
    }

    // UnitEye2 pointer fixups (sub_5B2BC9)
    if (this->eye2->unit != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->unit, result)) {
        this->eye2->unit = (Unit*)result;
    }
    if (this->eye2->unit2 != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->unit2, result)) {
        this->eye2->unit2 = (Unit*)result;
    }
    if (this->eye2->unit3 != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->unit3, result)) {
        this->eye2->unit3 = (Unit*)result;
    }
    if (this->eye2->unit4 != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->unit4, result)) {
        this->eye2->unit4 = (Unit*)result;
    }
    if (this->eye2->unit5 != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->unit5, result)) {
        this->eye2->unit5 = (Unit*)result;
    }
    if (this->eye2->spell != nullptr && g_Server->field23_0xdc.Lookup(this->eye2->spell, result)) {
        this->eye2->spell = (Spell*)result;
    }
    if (this->eye2->field51_0x68 != nullptr) {
        if (g_Server->field23_0xdc.Lookup(this->eye2->field51_0x68, result)) {
            this->eye2->field51_0x68 = (Building*)result;
        }
    }
}


// 52963B
void Unit::VMethod11(Unit* unit) {
    // Copy Token base fields
    this->building_id = unit->building_id;
    this->TokenID = unit->TokenID;
    this->itemDataID = unit->itemDataID;
    this->typeId = unit->typeId;

    // Deep-copy Token::position
    if (this->position != nullptr) {
        delete this->position;
        this->position = nullptr;
    }
    if (unit->position != nullptr) {
        this->position = new TokenPos(*unit->position);
    }

    this->pOwner = unit->pOwner;
    this->field_x18 = 0;
    this->_exp = unit->_exp;

    while (!this->_effects.IsEmpty()) {
        Effect* e = this->_effects.RemoveHead();
        delete e;
    }

    // Copy Unit fields
    this->monster_info = unit->monster_info;
    this->last_hit_by = nullptr;
    this->spell = nullptr;
    this->last_hit_spell_id = 0;
    this->token_size = unit->token_size;
    this->movement_type = unit->movement_type;
    this->face = unit->face;
    this->unit_attrs = unit->unit_attrs;
    this->state = unit->state;
    this->some_state = unit->some_state;
    this->some_state2 = unit->some_state2;
    this->cast_target = nullptr;
    this->area_cast_x = 0;
    this->area_cast_y = 0;
    this->some_spell = nullptr;
    this->some_item = nullptr;
    this->charge_countdown = unit->charge_countdown;
    this->group = nullptr;

    // Deep-copy weapon
    if (this->weapon != nullptr) {
        delete this->weapon;
        this->weapon = nullptr;
    }
    if (unit->weapon != nullptr) {
        this->weapon = new Weapon(unit->weapon);
    }

    // Deep-copy shield
    if (this->shield != nullptr) {
        delete this->shield;
        this->shield = nullptr;
    }
    if (unit->shield != nullptr) {
        this->shield = new Shield(unit->shield);
    }

    // Deep-copy inventory
    if (this->inventory != nullptr) {
        delete this->inventory;
        this->inventory = nullptr;
    }
    if (unit->inventory != nullptr) {
        this->inventory = new Inventory(unit->inventory);
    }

    this->name = unit->name;
    this->body = unit->body;
    this->reaction = unit->reaction;
    this->mind = unit->mind;
    this->spirit = unit->spirit;
    this->speed = unit->speed;
    this->extra_carrying_weight = unit->extra_carrying_weight;
    this->carrying_weight_100g = unit->carrying_weight_100g;
    this->carrying_body_100g = unit->carrying_body_100g;
    this->hp = unit->hp;
    this->hp_max = unit->hp_max;
    this->hp_regen = unit->hp_regen;
    this->mp = unit->mp;
    this->mp_max = unit->mp_max;
    this->mp_regen = unit->mp_regen;
    this->mp2 = unit->mp2;
    this->hp_regen_carry = unit->hp_regen_carry;
    this->mp_regen_carry = unit->mp_regen_carry;
    this->scan_range = unit->scan_range;
    this->hit_values = unit->hit_values;
    this->protections = unit->protections;
    this->equipment_extra = unit->equipment_extra;
    this->hit_values2 = unit->hit_values2;
    this->max_range = unit->max_range;
    this->experience = unit->experience;
    this->charge = unit->charge;
    this->relax = unit->relax;
    this->field_0x136 = unit->field_0x136;
    this->last_action_tick = unit->last_action_tick;
    this->decay = unit->decay;
    this->enchantments = 0;
    this->summon_id = this->summon_id;
    this->server_id = unit->server_id;
    this->field_0x150 = 0;
    memset(this->something_per_player, 0, sizeof(this->something_per_player));

    // Create fresh eye/eye2. WAT: old ones are leaked?
    this->eye = new UnitEye();
    this->eye2 = new UnitEye2();
    this->eye->sub_5925C9(this);

    // Clear list1 and list2
    this->list1.RemoveAll();
    this->list2.RemoveAll();

    // Deep-copy token_pos (create new empty if source has one)
    if (this->token_pos != nullptr) {
        delete this->token_pos;
        this->token_pos = nullptr;
    }
    if (unit->token_pos != nullptr) {
        this->token_pos = new TokenPos();
    }

    this->field_0x194 = unit->field_0x194;
    this->field_0x198 = unit->field_0x198;
    this->field_0x19c = unit->field_0x19c;
    this->field_0x204 = unit->field_0x204;
}


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
                        this->charge_countdown += (int8_t)this->some_spell->spell_info->Values()[0].level;
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
                if (building->IsKindOf(RUNTIME_CLASS(Shop))) { // Shop
                    g_NetStru1_main.FUN_0051cefb(0x83, building->object_info_id, 0, this->pOwner);
                    break;
                }
                if (building->IsKindOf(RUNTIME_CLASS(Inn))) { // Inn
                    g_NetStru1_main.FUN_0051cefb(0x84, building->object_info_id, building->building_id, this->pOwner);
                    break;
                }
                if (building->IsKindOf(&Pointer::classPointer)) { // Pointer
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
        delete eye2;
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
                if (Random0N(100) < effect->spell_or_damage) {
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
                if (Random0N(100) < effect->spell_or_damage) {
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
    return monster_info->Values()[0].dying_time;
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
        this->pOwner->sub_534AC1(sack->money, 0);
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
                        int32_t exp_val = ExperienceTable::GetExp(new_lvl);
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
                    uint32_t player_id = g_Server->ctf_carrying[1 - this->pOwner->field_0xa70];
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
                            g_Server->ctf_carrying[0] = this->pOwner->player_id;
                            g_Server->ctf_dropped[0] = 0;
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
                            g_Server->ctf_carrying[1] = this->pOwner->player_id;
                            g_Server->ctf_dropped[1] = 0;
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
        if (this->weapon != nullptr && this->weapon->world_equip->Values().GetData()[0].other_param != 0) {
            this->inventory->PutItemIntoBagAtDefault(this->Unequip(this->weapon));
        }

        // VMethod15: unequip all remaining equipped items.
        this->VMethod15();

        bool is_npc = (this->monster_info->name.Find("NPC") != -1);
        bool should_drop = false;

        if (this->typeId >= 0x40) {
            int roll = Random0N(99); // [0, 99]
            should_drop = (roll < 100 - g_ServerConfig.treasure_probability);
        }

        // Reset inventory if NPC, summoned, or flagged for drop.
        if (is_npc || this->summoned != 0 || should_drop) {
            delete this->inventory;
            this->inventory = new Inventory();
        }

        int32_t gold = 0;

        if (this->typeId >= 0x40 && this->summoned == 0) {
            MonsterInfoData& monster_data = this->monster_info->Values().GetData()[0];

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
            int32_t rune_holder = g_Server->ctf_carrying[opposite_team];

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
                g_Server->ctf_carrying[opposite_team] = 0;
                g_Server->ctf_dropped[opposite_team] = 1;

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

// 52BABD
int32_t Unit::sub_52BABD()
{
    return g_Server->tick - this->last_action_tick;
}

// 528725
uint16_t Unit::sub_528725()
{
    return this->position->GetXx() + ((this->VMethod3() & 0xFF) - 1) * 0x80;
}

// 528763
uint16_t Unit::sub_528763()
{
    return this->position->GetYy() + ((this->VMethod3() & 0xFF) - 1) * 0x80;
}

// 5287C4
int Unit::sub_5287c4(Player* player)
{
    return (this->field_x18 & player->vision_sharing_id) == 0;
}

// 5287EC
void Unit::sub_5287ec(Player* player)
{
    this->field_x18 |= player->vision_sharing_id;
}

// 52D904
int32_t Unit::sub_52D904()
{
    int32_t result = this->protections.defense;
    if (this->movement_type == 3) {
        result += 25;
    }
    if (this->mp_max != 0) {
        result += 50;
    }
    return result;
}

// 52A790
void Unit::sub_52A790(int32_t delta_weight)
{
    int32_t old_ratio = this->carrying_weight_100g / this->carrying_body_100g;

    this->extra_carrying_weight += delta_weight;
    this->carrying_weight_100g = this->extra_carrying_weight;
    if (this->inventory != nullptr) {
        if (this->inventory->total_weight < 64000) {
            this->carrying_weight_100g += this->inventory->total_weight / 2;
        } else {
            this->carrying_weight_100g = 32000;
        }
    }

    if (this->carrying_weight_100g / this->carrying_body_100g != old_ratio) {
        this->VMethod18();
    }
}

// 52C4DF
void Unit::sub_52C4DF(int16_t amount)
{
    this->field_0x150 |= 1;
    int16_t old_hp = this->hp;
    this->hp += amount;
    this->hp = (this->hp < this->hp_max) ? this->hp : this->hp_max;
    if (old_hp < 1 && this->hp > 0) {
        this->sub_52C58F();
    }
}

// 52C58F
void Unit::sub_52C58F()
{
    this->field_0x150 |= 0x4011;
    // Inline sub_58FD16: reset subcell position to cell center.
    this->position->x_subcell = 0x80;
    this->position->y_subcell = 0x80;
    this->decay = 0;
    this->some_state2 = 0;
    this->field_0x136 = 1;
    this->protections.defense <<= 1;
    this->VMethod18();
    this->hp = (this->hp < 1) ? 1 : this->hp;
    g_World->sub_5A9A6A(this);
}

// 52C641
void Unit::sub_52C641()
{
    this->sub_52C813();
    if (this->field_0x136 == 0 && this->some_item != nullptr && ((this->some_item->item_id >> 8) & 0xF) == 0xE) {
        delete this->some_item;
        this->some_item = nullptr;
        delete this->some_spell;
        this->some_spell = nullptr;
    }
    this->some_spell = nullptr;
    this->some_state2 = 0;
    this->field_0x136 = 1;
    this->some_state = 0;
    this->state = 0;
}

// 52E7FA
void Unit::sub_52E7FA()
{
    if (this->hp < -10000) {
        return;
    }
    uint8_t old_decay = this->decay;

    // Clamp HP for player character types (typeId 0x21-0x24) to leave dried bones forever.
    if (0x21 <= this->typeId && this->typeId <= 0x24 && this->hp < -61) {
        this->hp = -61;
    }

    // Decrement HP on every even tick16.
    if ((g_Server->tick16 & 1) == 0) {
        if (0x21 <= this->typeId && this->typeId <= 0x24) {
            if (this->hp > -61) {
                this->hp -= 1;
            }
        } else {
            this->hp -= 1;
        }
    }

    // Update decay state based on HP thresholds.
    if (this->hp < -10) {
        this->decay = 2;
    }
    if (this->hp < -20) {
        this->decay = 3;
    }
    if (this->hp < -40) {
        this->decay = 4;
    }
    if (this->hp < -600) {
        this->decay = 5;
        this->hp = -10001;
        if (old_decay != this->decay) {
            g_NetStru1_main.sub_519221(this, nullptr, 1, 0xFFB, 0, 0);
        }
        if (this->building_id != 0 && this->movement_type < 2) {
            BldIdSet_Unset(this->building_id);
            this->building_id = 0;
        }
    } else if (old_decay != this->decay) {
        g_NetStru1_main.sub_519221(this, nullptr, 1, 0xFFB, 0, 0);
    }
}

// 52931B
void Unit::FUN_0052931b(const CString& str)
{
    this->last_hit_by = nullptr;
    this->last_hit_spell_id = 0;
    this->spell = nullptr;
    this->field_0x1bc = 0;
    this->monster_info = nullptr;
    this->typeId = 0;
    this->decay = 0;
    this->token_size = 1;
    this->movement_type = 1;
    this->unit_attrs = 0;
    this->server_id = 0;
    this->face = 1;
    this->enchantments = 0;
    this->eye = new UnitEye();
    this->eye2 = new UnitEye2();
    this->token_pos = nullptr;
    this->eye->field0_0x0 = Random0N(0x80) + 0x40;
    this->eye->field1_0x1 = this->eye->field0_0x0;
    this->group = nullptr;
    this->state = 0;
    this->some_state = 0;
    this->some_state2 = 0;
    this->field_0x136 = 1;
    this->cast_target = nullptr;
    this->area_cast_x = 0;
    this->area_cast_y = 0;
    this->some_spell = nullptr;
    this->some_item = nullptr;
    this->spell_book = nullptr;
    this->inventory = new Inventory();
    this->weapon = nullptr;
    this->shield = nullptr;
    this->experience = 0;
    this->hp_regen_carry = 0;
    this->mp_regen_carry = 0;
    this->name = ""; // Originally it was set to `byte_70B460`, which was never populated, so it's always an empty string.
    this->last_action_tick = 0;
    this->summon_id = 0;
    this->field_0x194 = 0;
    this->field_0x198 = 0;
    this->field_0x19c = 0;
    this->summoned = 0;
    this->field_0x1a4 = 0;
    this->field_0x1a6 = 0;
    this->field_0x204 = 0;
    memset(this->something_per_player, 0, 0x40);
    this->sub_52A215();
    if (!str.IsEmpty()) {
        this->sub_52F601(str);
    }
}

// 537251
void Unit::sub_537251()
{
    this->equipment_extra.protections.magic_protections.fill(100);
    this->equipment_extra.protections.weapon_protections.fill(100);
    this->VMethod18();
}

// 52C813
void Unit::sub_52C813()
{
    if (this->some_item == nullptr || this->some_item->item_type == Item::ItemType::WEAPON) {
        return;
    }

    Effect* effect = this->some_item->_effects.IsEmpty() ? nullptr : this->some_item->_effects.GetHead();
    if (effect == nullptr || effect->effect_id != modifier::castspell) {
        return;
    }

    if ((this->some_state == 0xD || this->some_state == 0xE) && this->field_0x136 == 0) {
        return;
    }

    if (this->some_spell == nullptr || this->some_spell->spell_id == effect->spell_or_damage) {
        return;
    }

    LogMessage("Unit: Canceling wrong spell from item(scroll?)");
}

// 52D8D3
void Unit::sub_52D8D3(Inventory* inventory, int32_t money, int32_t is_main_player_unit)
{
    g_Server->srv_stru1->sack_list->sub_554927(this->position, inventory, money, is_main_player_unit);
}

// 52C163
void Unit::sub_52C163()
{
    if ((this->unit_attrs & 8) != 0) {
        return;
    }

    MapStuff_Instance->sub_58E3D1(this);
    this->unit_attrs |= 8;
    POSITION pos = dword_6CDB3C->unit_list.Find(this);
    if (pos != nullptr) {
        dword_6CDB3C->unit_list.RemoveAt(pos);
    }
    g_NetStru1_main.sub_51CF5C(this, 1, nullptr);
    g_NetStru1_main.sub_51CF5C(this, 0, this->pOwner);
}

// 52C36D
void Unit::sub_52C36D(uint8_t x, uint8_t y, uint8_t flags)
{
    if (!this->sub_52BF3D(x, y, flags)) {
        CString msg("Unit can't enter map - no free place");
        LogMessage(msg);
        return;
    }
    dword_6CDB3C->AddTail(this);
    this->unit_attrs &= ~8;
    g_NetStru1_main.sub_519221(this, nullptr, 0x10, 0xFFB, 0, 0);
    g_NetStru1_main.sub_51CF5C(this, 0, nullptr);
}

// 52BF3D
int Unit::sub_52BF3D(uint8_t x, uint8_t y, uint8_t size)
{
    uint32_t size_u32 = size;
    int32_t max_tries = size_u32 * size_u32 / 2 + 1;
    uint8_t radius = size / 2;
    bool placed = false;

    this->position->Set(x, y, MapStuff_Instance);

    for (int i = 0; i <= max_tries; i++) {
        uint8_t try_y = y - radius + Random0N(size);
        uint8_t try_x = x - radius + Random0N(size);
        this->position->FUN_0058a7e8(try_x, try_y);
        uint8_t gy = this->position->GetY();
        if (gy > 11 || g_Server->field4_0x74 == 0) { // WAT: why don't we allow placing units at `y <= 11`?
            if (MapStuff_Instance->sub_590F0A(this)) {
                placed = true;
                break;
            }
        }
    }

    if (!placed && size != 0) {
        int32_t x_start = x - radius;
        int32_t x_end = x + radius;
        int32_t y_start = y - radius;
        int32_t y_end = y + radius;
        for (int32_t tx = x_start; tx <= x_end; tx++) {
            for (int32_t ty = y_start; ty <= y_end; ty++) {
                if (ty > 11 || g_Server->field4_0x74 == 0) {
                    this->position->FUN_0058a7e8((uint8_t)tx, (uint8_t)ty);
                    if (MapStuff_Instance->sub_590F0A(this)) {
                        placed = true;
                        break;
                    }
                }
            }
            if (placed) {
                break;
            }
        }
    }

    if (!placed) {
        return 0;
    }

    return MapStuff_Instance->sub_58AD4A(this);
}

// 52BDD7
void Unit::sub_52BDD7(uint16_t xx, uint16_t yy, uint8_t eye0, uint8_t eye1)
{
    uint16_t cur_xx = this->position->GetXx();
    uint16_t cur_yy = this->position->GetYy();
    if (cur_xx == xx && cur_yy == yy) {
        uint8_t out_dir = 0;
        if (this->eye->sub_59367D(this, &out_dir, eye0, eye1)) {
            uint8_t angle = (this->eye->field1_0x1 + 8) / 16;
            g_NetStru1_main.sub_51B99E(this, angle, out_dir, 'm');
        }
        return;
    }

    if ((xx & 0xFF) == 0x80 && (yy & 0xFF) == 0x80) {
        uint8_t dir = this->eye->field158_0xae * 2;
        uint8_t step_dir = this->eye->field154_0xaa;
        g_NetStru1_main.sub_51B99E(this, dir, step_dir, 0);
    }
}

// 52ec7a
void Unit::FUN_0052ec7a(const CArray<MonsterInfoData>& values)
{
    const MonsterInfoData& data = values[0];

    if (data.body != -1) {
        this->body = (uint16_t)data.body;
    }
    if (data.reaction != -1) {
        this->reaction = (uint16_t)data.reaction;
    }
    if (data.mind != -1) {
        this->mind = (uint16_t)data.mind;
    }
    if (data.spirit != -1) {
        this->spirit = (uint16_t)data.spirit;
    }
    if (data.health_max != -1) {
        this->hp_max = (int16_t)data.health_max;
    }
    this->hp = this->hp_max;
    if (data.health_regeneration != -1) {
        this->hp_regen = (int16_t)data.health_regeneration;
    }
    if (data.mana_max != -1) {
        this->mp_max = (int16_t)data.mana_max;
    }
    this->mp = this->mp_max;
    this->mp2 = this->mp;
    if (data.mana_regeneration != -1) {
        this->mp_regen = (int16_t)data.mana_regeneration;
    }
    if (data.speed != -1) {
        this->speed = (uint16_t)data.speed;
    }
    if (data.rotation_speed != -1) {
        this->eye->rotation_speed = (uint8_t)data.rotation_speed;
    }
    if (data.scan_range != -1) {
        this->scan_range = (uint16_t)((uint8_t)data.scan_range << 8);
    }

    int32_t attack_min = 0, attack_delta = 0, attack_type = 0;
    if (data.physical_min != -1) {
        attack_min = data.physical_min;
    }
    if (data.physical_max != -1) {
        attack_delta = data.physical_max;
    }
    attack_delta -= attack_min;
    if (data.attack_type != -1) {
        attack_type = data.attack_type;
    }
    if (attack_type < 1) {
        if (attack_min < 128 && attack_delta <= attack_min + 255) {
            this->hit_values.hand_damage_min = (uint8_t)attack_min;
            this->hit_values.hand_damage_spread = (uint8_t)attack_delta;
        } else {
            this->hit_values.hand_damage_min = (uint8_t)(attack_min / 15 < 0x7f ? attack_min / 15 : 0x7f) | 0x80;
            this->hit_values.hand_damage_spread = (uint8_t)(attack_delta / 15 < 0xff ? attack_delta / 15 : 0xff);
        }
    } else if (attack_type == 1) {
        this->hit_values.some_damage_min = (uint8_t)attack_min;
        this->hit_values.some_damage_spread = (uint8_t)attack_delta;
    } else if (attack_type == 2) {
        this->hit_values.some_damage2_min = (uint8_t)(-attack_min);
        this->hit_values.some_damage2_spread = (uint8_t)(-attack_delta);
    } else if (attack_type == 3) {
        this->unit_attrs |= 0x10;
        this->hit_values.hand_damage_min = (uint8_t)attack_min;
        this->hit_values.hand_damage_spread = (uint8_t)attack_delta;
    }
    if (data.attack != -1) {
        this->hit_values.attack = (int16_t)data.attack;
    }
    this->hit_values.skill_levels[0] = this->hit_values.attack;
    if (data.defence != -1) {
        this->protections.defense = (int16_t)data.defence;
    }
    if (data.absorption != -1) {
        this->protections.absorption = (int16_t)data.absorption;
    }
    if (data.charge != -1) {
        this->charge = (uint8_t)data.charge;
    }
    if (data.relax != -1) {
        this->relax = (uint8_t)data.relax;
    }
    for (int32_t i = 1; i < 6; i++) {
        int32_t prot = data.protection_magic[i-1];
        if (prot != -1) {
            this->protections.magic_protections[i] = (uint16_t)prot;
        }
    }
    for (int32_t i = 1; i < 6; i++) {
        int32_t prot = data.protection_physical[i-1];
        if (prot != -1) {
            this->protections.weapon_protections[i] = (uint8_t)prot;
        }
    }
    if (data.type_id != -1) {
        this->typeId = (uint16_t)data.type_id;
    }
    if (data.face != -1) {
        this->face = (int8_t)data.face;
    }
    if (data.token_size != -1) {
        this->token_size = (int8_t)data.token_size;
    }
    if (data.movement_type != -1) {
        this->movement_type = (int8_t)data.movement_type;
    }
    if (data.withdraw != -1) {
        this->eye2->withdraw = (uint32_t)data.withdraw;
    }
    if (data.wimpy != -1) {
        this->eye2->wimpy = (uint32_t)data.wimpy;
    }
    if (data.see_invisible != -1) {
        this->eye2->seeInvisible = (uint8_t)data.see_invisible;
    }
    if (data.experience != -1) {
        this->_exp = data.experience;
    }
}

// 52F601
void Unit::sub_52F601(const CString& name)
{
    this->typeId = 0;

    // Search for monster by name in g_GameDataRes.monsters array
    int32_t idx = 26;
    while (idx < g_GameDataRes.monsters.GetSize()) {
        if (idx == 30) { // Skip index 30 and jump to 63.
            idx = 63;
        }
        if (name == g_GameDataRes.monsters[idx].name) {
            break;
        }
        idx++;
    }

    if (idx >= g_GameDataRes.monsters.GetSize()) {
        LogMessage("Invalid unit " + name + " created");
        return;
    }

    // Found monster - initialize unit
    this->itemDataID = idx;
    this->monster_info = &g_GameDataRes.monsters[idx];

    // Set base stats from monster data
    this->FUN_0052ec7a(this->monster_info->Values());

    // Set server_id if extended data exists
    if (this->monster_info->Values().GetSize() > 55) {
        const MonsterInfoData& data = this->monster_info->Values()[0];
        this->server_id = (uint16_t)data.server_id;
    }

    // Equip weapon and shield.
    for (int32_t i = 0; i < 2; i++) {
        if (i < this->monster_info->equipped_items.GetSize()) {
            const CString& item_name = this->monster_info->equipped_items[i];
            if (!item_name.IsEmpty()) {
                if (item_name.Find("Shield") != -1) {
                    this->VMethod13(new Shield(item_name));
                } else {
                    this->VMethod13(new Weapon(item_name));
                }
            }
        }
    }

    // Create spell book and add known spells
    const MonsterInfoData& data = this->monster_info->Values()[0];
    if (data.spell1_id > 0) {
        this->unit_attrs |= 2;
        this->spell_book = new SpellBook();
    }

    for (int32_t i = 0; i < 3; i++) {
        int32_t spell_id = i == 0 ? data.spell1_id : i == 1 ? data.spell2_id : data.spell3_id;
        if (spell_id > 0) {
            Spell* spell = new Spell(spell_id);
            this->spell_book->sub_53D7F0(spell_id, spell);
            this->eye2->known_spells[i] = spell_id;
            
            int32_t probability = i == 0 ? data.spell_probability1 : i == 1 ? data.spell_probability2 : data.spell_probability3;
            this->eye2->known_spell_probabilities[i] = probability * 0x147;
        }
    }

    // Set default skill level
    for (int32_t i = 1; i < 6; i++) {
        this->hit_values.skill_levels[i] = data.spell_power;
    }

    // Create full spell book with bitmask if extended data exists
    if (this->monster_info->Values().GetSize() > 56) {
        uint32_t spell_mask = (uint32_t)data.known_spells_mask;
        if (spell_mask != 0 && spell_mask != 0xFFFFFFFF) {
            this->unit_attrs |= 6;
            this->spell_book = new SpellBook();

            // Add all spells matching bitmask
            for (int32_t spell_id = 1; spell_id <= spell::max_spell_id; spell_id++) {
                if ((spell_mask & (1 << spell_id)) != 0) {
                    Spell* spell = new Spell(spell_id);
                    this->spell_book->sub_53D7F0(spell_id, spell);
                }
            }

            this->hit_values.skill_levels[1] = data.skill_fire;
            this->hit_values.skill_levels[2] = data.skill_water;
            this->hit_values.skill_levels[3] = data.skill_air;
            this->hit_values.skill_levels[4] = data.skill_earth;
            this->hit_values.skill_levels[5] = data.skill_astral;
        }
    }

    this->equipment_extra.AddToUnit(this);

    this->VMethod19();

    if (data.experience > 0) {
        this->_exp = data.experience;
    }

    // Initialize eye and world state
    this->eye->sub_5925C9(this);
    g_World->sub_5AF6F5(this);
}

// 6363e8.
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


// 55c760
void Humanoid::Serialize(CArchive& ar) {
    Unit::Serialize(ar);
    if (ar.IsStoring()) {
        ar.Write(&this->main_sphere, 0x18);
        for (int32_t i = 1; i < 13; i++) {
            ar.WriteObject(this->equipment[i]);
        }
    } else {
        ar.Read(&this->main_sphere, 0x18);
        for (int32_t i = 1; i < 13; i++) {
            this->equipment[i] = (Item*)ar.ReadObject(RUNTIME_CLASS(Item));
        }
    }
}

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

// 53004c
void Humanoid::InitStats()
{
    for (int32_t i = 0; i < 13; i++) {
        this->equipment[i] = nullptr;
    }

    this->main_sphere = 0;

    for (int32_t i = 0; i < 5; i++) {
        this->experience_per_sphere[i] = 0;
    }

    this->body = 30;
    this->mind = 30;
    this->reaction = 30;
    this->spirit = 30;
    this->hp_max = 50;
    this->hp = 50;
    this->speed = 16;
    this->eye->rotation_speed = 8;
    this->charge = 8;
    this->relax = 4;
    this->hit_values.hand_damage_min = 1;
    this->hit_values.hand_damage_spread = 2 - this->hit_values.hand_damage_min;

    this->eye->sub_5925C9(this);
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

// 5303e3
Item* Humanoid::VMethod12(Item* item) {
    if (item->IsKindOf(RUNTIME_CLASS(Armor)) || item->IsKindOf(RUNTIME_CLASS(Weapon)) || item->IsKindOf(RUNTIME_CLASS(Shield))) {
        int32_t other_param = item->world_equip->Values().GetData()->other_param;
        if ((other_param & 1) == 0 && (this->unit_attrs & 4) == 0) {
            return item;
        }
        if ((other_param & 2) == 0 && (this->unit_attrs & 4) != 0) {
            return item;
        }
    }
    return item->VMethod10(this);
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

// 53111b
void Humanoid::VMethod16(Unit* unit) {
    if (g_Server->field4_0x74 == 0) {
        return;
    }
    if (unit->VMethod8() != 0) {
        return;
    }
    if (this->hp < 0) {
        return;
    }
    this->pOwner->sub_534AC1(unit->_exp, 1);
}

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
        hit_values.attack = hit_values.attack + lvl * 3;
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
        if (g_World != nullptr && (g_World->diplomacy.diplomacy[target->pOwner->player_id][this->pOwner->player_id] & 2) != 0) {
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

            leveled_up = ExperienceTable::GetExp(this->hit_values.skill_levels[phys_sphere] + 1) <= this->experience_per_sphere[phys_sphere - 1];
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

            leveled_up = ExperienceTable::GetExp(this->hit_values.skill_levels[sphere] + 1) <= this->experience_per_sphere[sphere - 1];
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

// 530e52   
void Humanoid::VMethod22(Unit* unit, int32_t spell_id) {
    if (spell_id != 0 && (this->unit_attrs & 4) == 0) {
        return;
    }
    int32_t sphere = 0;
    if (spell_id != 0 && (this->unit_attrs & 4) != 0) {
        sphere = g_GameDataRes.spells[spell_id].Values().GetData()->sphere;
    }
    this->VMethod21((int32_t)(unit->_exp * 0.5), unit, sphere);
    if (!unit) {
        return;
    }
    bool found = false;
    for (int32_t i = 0; i < this->mob_kills_in_session.GetSize(); i++) {
        if (this->mob_kills_in_session[i].server_id == unit->server_id) {
            this->mob_kills_in_session[i].kills++;
            found = true;
            break;
        }
    }
    if (!found) {
        if (this->mob_kills_in_session.GetSize() >= 5) {
            this->mob_kills_in_session.RemoveAt(0, 1);
        }
        SessionMobKill entry;
        entry.server_id = unit->server_id;
        entry.kills = 0;
        this->mob_kills_in_session.Add(entry);
    }
}

// 530fc1
void Humanoid::VMethod23(Unit* unit, uint32_t damage, int32_t spell_id) {
    if (spell_id != 0 && (this->unit_attrs & 4) == 0) {
        return;
    }
    int32_t sphere = 0;
    if (spell_id != 0 && (this->unit_attrs & 4) != 0) {
        sphere = g_GameDataRes.spells[spell_id].Values().GetData()->sphere;
    }
    if (unit != nullptr && unit->pOwner != nullptr && unit->pOwner->is_ai) {
        int32_t xp = (int32_t)(unit->_exp * 0.5 * damage / unit->hp_max + 1.0);
        this->VMethod21(xp, unit, sphere);
    }
}

// 531090
void Humanoid::VMethod24(Unit*, int32_t spell_id) {
    int32_t sphere = 0;
    if (spell_id != 0 && (this->unit_attrs & 4) != 0) {
        sphere = g_GameDataRes.spells[spell_id].Values().GetData()->sphere;
    }
    if (sphere == 0) {
        return;
    }
    int32_t mana_cost = g_GameDataRes.spells[spell_id].Values().GetData()->mana_cost;
    int32_t xp = (int32_t)(mana_cost * 0.5 + 0.5);
    this->VMethod21(xp, nullptr, sphere);
}

// 530394
int32_t Humanoid::VMethod25() {
    if (this->monster_info->Values().GetSize() == 0) {
        return 8;
    }
    int32_t val = this->monster_info->Values().GetData()->protection_magic[4];
    if (val == -1) {
        return 8;
    }
    return val;
}

// 53116B
void Humanoid::sub_53116B() {
    this->experience = 0;
    for (int32_t i = 1; i < 6; i++) {
        if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
            this->hit_values.skill_levels[i] = 70;
            this->experience_per_sphere[i - 1] = ExperienceTable::GetExp(this->hit_values.skill_levels[i]);
            this->experience += this->experience_per_sphere[i - 1];
        } else if (g_ServerConfig.gameType == 3) {
            this->experience += this->experience_per_sphere[i - 1];
            this->hit_values.skill_levels[i] = ExperienceTable::GetLevel(this->experience_per_sphere[i - 1]);
        } else {
            this->experience_per_sphere[i - 1] = (this->experience_per_sphere[i - 1] * 9) / 10;
            this->experience += this->experience_per_sphere[i - 1];
            this->hit_values.skill_levels[i] = ExperienceTable::GetLevel(this->experience_per_sphere[i - 1]);
        }
        this->hit_values2.skill_levels[i] = this->hit_values.skill_levels[i];
    }

    delete(this->eye);
    this->eye = new UnitEye();
    if (this->eye2 != nullptr) {
        delete this->eye2;
    }
    this->eye2 = new UnitEye2();
    this->sub_52C58F();
}

// 533345
void Humanoid::sub_533345(int8_t main_skill, int8_t skill_level) {
    CString m;
    m.Format("sub_533345: (main skill %d, level %d)", main_skill, skill_level);
    LogMessage(m);
    // Unequip and delete current weapon.
    Item* old_weapon = this->Unequip(this->weapon);
    delete old_weapon;

    // Zero all combat skill levels.
    for (int32_t i = 1; i < 6; i++) {
        this->hit_values.skill_levels[i] = 0;
    }

    // Set skills for main sphere and astral/shooting.
    this->hit_values.skill_levels[main_skill] = skill_level;
    this->hit_values.skill_levels[5] = skill_level / 2;

    // Recalculate experience from skills (inline sub_531418).
    this->experience = 0;
    for (int32_t i = 1; i < 6; i++) {
        this->experience_per_sphere[i - 1] = ExperienceTable::GetExp(this->hit_values.skill_levels[i]);
        this->experience += this->experience_per_sphere[i - 1];
        
        this->hit_values2.skill_levels[i] = this->hit_values.skill_levels[i];
    }

    // Set main sphere.
    this->main_sphere = main_skill;

    if (this->unit_attrs & 4) {
        // Mage: create a staff with cast spell.
        CString spell_name;
        switch (main_skill) {
            case 1: spell_name = "Fire_Arrow"; break;
            case 2: spell_name = "Ice_Missile"; break;
            case 3: spell_name = "Lightning"; break;
            case 4: spell_name = "Diamond_Dust"; break;
            case 5: spell_name = "Drain_Life"; break;
        }

        CString weapon_name;
        if (this->typeId == 0x22 || this->typeId == 0x24) {
            weapon_name = "Wood Staff {castSpell=" + spell_name + ":20}";
        } else {
            weapon_name = "Uncommon Wood Staff {castSpell=" + spell_name + ":20}";
        }

        this->VMethod13(new Weapon(weapon_name));
    } else {
        // Fighter: create appropriate weapon.
        Weapon* weapon = nullptr;
        switch (main_skill) {
            case 1: weapon = new Weapon("Iron Long Sword"); break;
            case 2: weapon = new Weapon("Iron Axe"); break;
            case 3: weapon = new Weapon("Iron Mace"); break;
            case 4: weapon = new Weapon("Iron Pike"); break;
            case 5: weapon = new Weapon("Uncommon Wood Long Bow"); break;
        }
        this->VMethod13(weapon);
    }
}

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

// 532587
void Human::FUN_00532587(CString hname, int32_t is_hero, const char* unk)
{
    this->typeId = 0;
    bool load_equipment = (unk == nullptr);
    int32_t face_index = -1;
    int32_t is_female = 1;

    int32_t dot_pos = hname.ReverseFind('.');
    if (dot_pos != -1) {
        face_index = atoi(hname.Mid(dot_pos + 2));
        is_female = (hname[dot_pos + 1] == 'f') ? 1 : 0;
        hname = hname.Left(dot_pos);
    }

    if (hname.Find("Hero") == 0) {
        is_hero = 1;
        load_equipment = false;
        hname = hname.Mid(5);
        if (hname.Find("Man") != 0) {
            hname = "Man_" + hname;
        }
        this->name = "Unknown";
        if (is_female == 0) {
            if (face_index < 10) { // TODO: unhardcode after migrating the arrays to C++.
                this->name = MALE_HUMAN_NAMES[face_index];
            }
        } else {
            if (face_index < 7) {
                this->name = FEMALE_HUMAN_NAMES[face_index];
            }
        }
    }

    for (int i = 1; i < g_GameDataRes.humans.GetSize(); ++i) {
        if (hname != g_GameDataRes.humans[i].name) {
            continue;
        }

        this->itemDataID = static_cast<int16_t>(i);
        this->monster_info = reinterpret_cast<MonsterInfo*>(&g_GameDataRes.humans[i]);
        this->sub_532dde(&g_GameDataRes.humans[i].Values());
        const HumanInfoData& data = g_GameDataRes.humans[i].Values()[0];

        if (data.gender != -1) {
            is_female = data.gender;
        }

        if (g_GameDataRes.humans[i].Values().GetSize() > 24) {
            uint16_t sid = static_cast<uint16_t>(data.server_id);
            if (sid > 10000) {
                sid = static_cast<uint16_t>((sid / 10) % 1000);
            }
            this->server_id = sid;
        }

        if (this->mp_max > 0) {
            this->unit_attrs |= 6;
        }

        if (load_equipment) {
            for (int32_t slot = 0; slot < 10; slot++) {
                if (slot < g_GameDataRes.humans[i].equipped_items.GetSize()) {
                    const CString& item_name = g_GameDataRes.humans[i].equipped_items[slot];
                    if (!item_name.IsEmpty()) {
                        if (slot == 0) {
                            this->VMethod13(new Weapon(item_name));
                        } else if (slot == 1) {
                            this->VMethod13(new Shield(item_name));
                        } else {
                            this->VMethod13(new Armor(item_name));
                        }
                    }
                }
            }
        }
        break;
    }

    if (this->mp_max > 0) {
        this->unit_attrs |= 6;
        this->spell_book = new SpellBook();
        const HumanInfoData& data = reinterpret_cast<HumanInfo*>(this->monster_info)->Values()[0];
        int32_t known_spells_mask = data.known_spells_mask;
        for (uint8_t spell_id = 1; spell_id < 30; spell_id++) {
            if (known_spells_mask & (1 << spell_id)) {
                Spell* spell = new Spell(spell_id);
                this->spell_book->sub_53D7F0(spell_id, spell);
            }
        }
    }

    if (face_index > 0) {
        this->face = static_cast<int8_t>(face_index);
    }

    if (is_hero == 0) {
        this->face = static_cast<int8_t>(this->face | (is_female << 7));
    } else {
        if ((this->unit_attrs & 4) == 0) {
            this->typeId = static_cast<uint16_t>(is_female + 33);
        } else {
            this->typeId = static_cast<uint16_t>(is_female + 35);
        }
    }

    this->sub_531418();
    this->VMethod18();
    this->hp = this->hp_max;
    this->mp = this->mp_max;
    this->VMethod19();

    if (this->typeId == 0) {
        LogMessage("Invalid human " + hname + " created");
    }
}

Human::~Human()
{}

// 55C81C
void Human::Serialize(CArchive& ar) {
    Humanoid::Serialize(ar);
    if (!ar.IsStoring()) {
        if (this->typeId < 0x21) {
            this->monster_info = reinterpret_cast<MonsterInfo*>(&g_GameDataRes.humans[this->itemDataID]);
        } else {
            this->monster_info = reinterpret_cast<MonsterInfo*>(&g_GameDataRes.humans[5]);
        }
    }
}

// 5339a6
void Human::VMethod20() {
    int32_t level = (this->_exp > 1000) ? (this->_exp / 10) : 0;
    Item* item = sub_5499A6(level, this->_exp);
    this->inventory->PutItemIntoBagAtDefault(item);
}
