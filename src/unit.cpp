#include "unit.h"

UnitToHit::UnitToHit()
{
    //533c5c
    Clear(); 
}

void UnitToHit::Clear()
{
	//533c72
    skill_levels.fill(0);

    attack = 0;
    physical_damage_type = 0;
    hand_damage_min = 0;
    hand_damage_spread = 0;
    some_damage_min = 0;
    some_damage_spread = 0;
    some_damage2_min = 0;
    some_damage2_spread = 0;
    spell_id = 0;
}

//533d0d
UnitToHit::UnitToHit(const UnitToHit&) = default;

UnitToHit::UnitToHit(UnitToHit&&) = default;

//533e02
UnitToHit& UnitToHit::operator=(const UnitToHit&) = default;

UnitToHit& UnitToHit::operator+=(const UnitToHit &b)
{
    //533d2c
    for (int32_t i = 0; i < skill_levels.size(); i++)
        skill_levels[i] += b.skill_levels[i];
    attack += b.attack;
    hand_damage_min += b.hand_damage_min;
    hand_damage_spread += b.hand_damage_spread;
    some_damage_min += b.some_damage_min;
    some_damage_spread += b.some_damage_spread;
    some_damage2_min += b.some_damage2_min;
    some_damage2_spread += b.some_damage2_spread;
    spell_id += b.spell_id;
    return *this;
}

UnitToHit& UnitToHit::operator-=(const UnitToHit &b)
{
    //533f3a
    for (int32_t i = 0; i < skill_levels.size(); i++)
        skill_levels[i] -= b.skill_levels[i];
    attack += b.attack;
    hand_damage_min += b.hand_damage_min;
    hand_damage_spread += b.hand_damage_spread;
    some_damage_min += b.some_damage_min;
    some_damage_spread += b.some_damage_spread;
    some_damage2_min += b.some_damage2_min;
    some_damage2_spread += b.some_damage2_spread;

    if (some_damage2_min + some_damage2_spread != 0)
        spell_id = 0;
    return *this;
}

void UnitToHit::AddHits(const UnitToHit &b)
{
    //533ea3
    attack += b.attack;
    hand_damage_min += b.hand_damage_min;
    hand_damage_spread += b.hand_damage_spread;
    some_damage_min += b.some_damage_min;
    some_damage_spread += b.some_damage_spread;
    some_damage2_min += b.some_damage2_min;
    some_damage2_spread += b.some_damage2_spread;
    spell_id += b.spell_id;
}

void UnitToHit::Serialize(CArchive& ar)
{
    //534021
    if (ar.IsStoring())
    {
        //ar.Write(this, sizeof(UnitToHit));
        ar.Write(this, sizeof(0x18));
    }
    else
    {
        //ar.Read(this, sizeof(UnitToHit));
        ar.Read(this, sizeof(0x18));
    }
}



Protections::Protections()
{
    //5339f8
    Clear();
}

void Protections::Clear()
{
    //533a0e
    defense = 0;
    absorption = 0;
    magic_protections.fill(0);
    weapon_protections.fill(0);
}

//533a29
Protections::Protections(const Protections&) = default;

Protections::Protections(Protections&&) = default;

//533af0
Protections& Protections::operator=(const Protections&) = default;

Protections& Protections::operator+=(const Protections &b)
{
    //533a48
    for (int32_t i = 0; i < magic_protections.size(); i++)
        magic_protections[i] += b.magic_protections[i];
    for (int32_t i = 0; i < weapon_protections.size(); i++)
        weapon_protections[i] += b.weapon_protections[i];
    defense += b.defense;
    absorption += b.absorption;
    return *this;
}

Protections& Protections::operator-=(const Protections &b)
{
    //533b77
    for (int32_t i = 0; i < magic_protections.size(); i++)
        magic_protections[i] -= b.magic_protections[i];
    for (int32_t i = 0; i < weapon_protections.size(); i++)
        weapon_protections[i] -= b.weapon_protections[i];
    defense -= b.defense;
    absorption -= b.absorption;
    return *this;
}

void Protections::Serialize(CArchive& ar)
{
    //533c1f
    if (ar.IsStoring())
    {
        //ar.Write(this, sizeof(Protections));
        ar.Write(this, sizeof(0x16));
    }
    else
    {
        //ar.Read(this, sizeof(Protections));
        ar.Read(this, sizeof(0x16));
    }
}

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

