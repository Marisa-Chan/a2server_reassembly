#include "protections.h"


Protections::Protections() {
    //5339f8
    this->Clear();
}

void Protections::Clear() {
    //533a0e
    this->defense = 0;
    this->absorption = 0;
    this->magic_protections.fill(0);
    this->weapon_protections.fill(0);
}

//533a29
Protections::Protections(const Protections&) = default;

Protections::Protections(Protections&&) = default;

//533af0
Protections& Protections::operator=(const Protections&) = default;

Protections& Protections::operator+=(const Protections &b) {
    //533a48
    for (int32_t i = 0; i < this->magic_protections.size(); i++) {
        this->magic_protections[i] += b.magic_protections[i];
    }
    for (int32_t i = 0; i < this->weapon_protections.size(); i++) {
        this->weapon_protections[i] += b.weapon_protections[i];
    }
    this->defense += b.defense;
    this->absorption += b.absorption;
    return *this;
}

Protections& Protections::operator-=(const Protections &b) {
    //533b77
    for (int32_t i = 0; i < this->magic_protections.size(); i++) {
        this->magic_protections[i] -= b.magic_protections[i];
    }
    for (int32_t i = 0; i < this->weapon_protections.size(); i++) {
        this->weapon_protections[i] -= b.weapon_protections[i];
    }
    this->defense -= b.defense;
    this->absorption -= b.absorption;
    return *this;
}

void Protections::Serialize(CArchive& ar) {
    //533c1f
    if (ar.IsStoring()) {
        //ar.Write(this, sizeof(Protections));
        ar.Write(this, 0x16);
    } else {
        //ar.Read(this, sizeof(Protections));
        ar.Read(this, 0x16);
    }
}
