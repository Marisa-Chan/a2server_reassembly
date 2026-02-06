#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "unit.h"

#include "eye.h"
#include "inventory.h"
#include "item.h"
#include "player.h"
#include "spell.h"
#include "table.h"

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

void Unit::VMethod16(Item*)
{}

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

void Unit::VMethod21(uint32_t newExp, Unit *target, int32_t sphere)
{
    experience += newExp;
    if (target && target->hp < 0)
        pOwner->monster_kills += 1;
}

void Unit::VMethod22(Unit*, int32_t)
{}

void Unit::VMethod23(Unit*, uint32_t, int32_t, int32_t)
{}

void Unit::VMethod24(Unit*, uint32_t, int32_t, int32_t)
{}
/*
int32_t Unit::VMethod25()
{
    return monster_info->values
}*/



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

/*void Humanoid::VMethod16(Item* item)
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
