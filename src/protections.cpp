#include "protections.h"


// 5339f8 (client: 51f203)
Protections::Protections() {
    this->Clear();
}

// 533a0e (client: 51f219)
void Protections::Clear() {
    this->defense = 0;
    this->absorption = 0;
    this->magic_protections.fill(0);
    this->weapon_protections.fill(0);
}

// 533a29 (client: 51f234)
Protections::Protections(const Protections&) = default;

Protections::Protections(Protections&&) = default;

// 533af0 (client: 51f2fb)
Protections& Protections::operator=(const Protections&) = default;

// 533a48 (client: 51f253)
Protections& Protections::operator+=(const Protections &b) {
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

// 533b77 (client: 51f382)
Protections& Protections::operator-=(const Protections &b) {
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

// 533c1f (client: 51f42a)
void Protections::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        //ar.Write(this, sizeof(Protections));
        ar.Write(this, 0x16);
    } else {
        //ar.Read(this, sizeof(Protections));
        ar.Read(this, 0x16);
    }
}
