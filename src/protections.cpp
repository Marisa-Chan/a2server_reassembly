#include "protections.h"


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
