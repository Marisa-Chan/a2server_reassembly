#include "unit_to_hit.h"

// 533c5c (client: 51f461)
UnitToHit::UnitToHit() {
    this->Clear(); 
}

// 533c72 (client: 51f477)
void UnitToHit::Clear() {
    this->skill_levels.fill(0);

    this->attack = 0;
    this->physical_damage_type = 0;
    this->hand_damage_min = 0;
    this->hand_damage_spread = 0;
    this->some_damage_min = 0;
    this->some_damage_spread = 0;
    this->some_damage2_min = 0;
    this->some_damage2_spread = 0;
    this->spell_id = 0;
}

// 533d0d (client: 51f512)
UnitToHit::UnitToHit(const UnitToHit&) = default;

UnitToHit::UnitToHit(UnitToHit&&) = default;

// 533e02 (client: 51f607)
UnitToHit& UnitToHit::operator=(const UnitToHit&) = default;

// 533d2c (client: 51f531)
UnitToHit& UnitToHit::operator+=(const UnitToHit &b) {
    for (int32_t i = 0; i < this->skill_levels.size(); i++) {
        this->skill_levels[i] += b.skill_levels[i];
    }
    this->attack += b.attack;
    this->hand_damage_min += b.hand_damage_min;
    this->hand_damage_spread += b.hand_damage_spread;
    this->some_damage_min += b.some_damage_min;
    this->some_damage_spread += b.some_damage_spread;
    this->some_damage2_min += b.some_damage2_min;
    this->some_damage2_spread += b.some_damage2_spread;
    this->spell_id = b.spell_id;
    return *this;
}

// 533f3a (client: 51f73f)
UnitToHit& UnitToHit::operator-=(const UnitToHit &b) {
    for (int32_t i = 0; i < this->skill_levels.size(); i++){
        this->skill_levels[i] -= b.skill_levels[i];
    }
    this->attack += b.attack;
    this->hand_damage_min += b.hand_damage_min;
    this->hand_damage_spread += b.hand_damage_spread;
    this->some_damage_min += b.some_damage_min;
    this->some_damage_spread += b.some_damage_spread;
    this->some_damage2_min += b.some_damage2_min;
    this->some_damage2_spread += b.some_damage2_spread;

    if (this->some_damage2_min + this->some_damage2_spread != 0) {
        this->spell_id = 0;
    }
    return *this;
}

// 533ea3 (client: 51f6a8)
void UnitToHit::AddHits(const UnitToHit &b) {
    this->attack += b.attack;
    this->hand_damage_min += b.hand_damage_min;
    this->hand_damage_spread += b.hand_damage_spread;
    this->some_damage_min += b.some_damage_min;
    this->some_damage_spread += b.some_damage_spread;
    this->some_damage2_min += b.some_damage2_min;
    this->some_damage2_spread += b.some_damage2_spread;
    this->spell_id = b.spell_id;
}

// 534021 (client: 51f826)
void UnitToHit::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        //ar.Write(this, sizeof(UnitToHit));
        ar.Write(this, 0x18);
    } else {
        //ar.Read(this, sizeof(UnitToHit));
        ar.Read(this, 0x18);
    }
}
