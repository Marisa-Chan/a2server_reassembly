#include <stdio.h>
#include <stdlib.h>
#include "unit.h"

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


Humanoid::Humanoid(void****) {}; // extra to enforce compiler create vftable in C++
