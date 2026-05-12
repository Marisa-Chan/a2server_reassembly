#include "effect.h"

#include <cmath>

#include "constants.h"
#include "eye.h"
#include "game_app.h"
#include "net.h"
#include "spell.h"
#include "unit.h"

// 53f761
void Effect::VMethod12(Unit* unit)
{
    if (this->itemDataID == spell::poison_cloud) {
        int32_t damage = -(int16_t)this->spell_or_damage;
        if (unit->protections.magic_protections[2] != 0) {
            damage = (int32_t)((damage * (100 - unit->protections.magic_protections[2])) / 100.0 + 0.5);
        }
        if (damage != 0) {
            unit->hp -= damage;
            if (this->caster != nullptr) {
                if (this->caster->hp < 0) {
                    this->caster = nullptr;
                } else {
                    this->caster->VMethod23(unit, damage, spell::poison_cloud);
                }
            }
            g_NetStru1_main.sub_51C601(unit, 0);
        }
    } else if (this->itemDataID == spell::darkness) {
        if (unit->VMethod8() == 0) {
            unit->scan_range -= this->spell_or_damage * 256;
            unit->field_0x150 |= 0x40000;
        }
    } else if (this->itemDataID == spell::light) {
        if (unit->VMethod8() != 0) {
            unit->equipment_extra.scan_range += this->spell_or_damage * 256;
            unit->scan_range += this->spell_or_damage * 256;
            unit->field_0x150 |= 0x40000;
        }
    } else {
        this->VMethod14(unit, 1);
    }
}


// 53EDB0
int Effect::sub_53EDB0(Effect* other)
{
    return effect_id == other->effect_id &&
           usage_type == other->usage_type &&
           spell_or_damage == other->spell_or_damage &&
           spell_value == other->spell_value;
}


Effect::Effect()
{
    //53ec02
    typeId = 0;
    effect_id = 0;
    usage_type = 0;
    spell_or_damage = 0;
    spell_value = 0;
    itemDataID = 0;
    caster = nullptr;
}


Effect::Effect(const CString& name)
{
    //53ec55
    Effect* tmp = CreateFromString(name);
    if (tmp)
    {
        effect_id = tmp->effect_id;
        usage_type = tmp->usage_type;
        spell_or_damage = tmp->spell_or_damage;
        spell_value = tmp->spell_value;
        itemDataID = tmp->itemDataID;

        delete tmp;

        caster = 0;
    }
    else
    {
        effect_id = 0;
        usage_type = 0;
        spell_or_damage = 0;
        spell_value = 0;
        itemDataID = 0;
    }
}

Effect::Effect(const Effect* src)
: Token(*src)
{
    //53ed4d
    effect_id = src->effect_id;
    usage_type = src->usage_type;
    spell_or_damage = src->spell_or_damage;
    spell_value = src->spell_value;
    itemDataID = src->itemDataID;
    caster = src->caster;
}

Effect::~Effect()
{
    //57c350
}

IMPLEMENT_SERIAL(Effect, Token, 1);

void Effect::Serialize(CArchive& ar)
{
    //53e1ce
    Token::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << effect_id;
        ar << usage_type;
        ar << spell_or_damage;
        ar << spell_value;
        ar << itemDataID;
    }
    else
    {
        ar >> effect_id;
        ar >> usage_type;
        ar >> spell_or_damage;
        ar >> spell_value;
        ar >> itemDataID;
    }
}


//void Effect::VMethod10(Unit* unit)
//{
    // TODO: migrate
//}

//void Effect::VMethod11(Unit* unit)
//{
    // TODO: migrate
//}

// 53FA2B
void Effect::VMethod14(Unit* unit, int32_t param_3) {
    int32_t value;
    if (this->usage_type & 7) {
        value = param_3 * this->spell_or_damage;
    } else {
        value = param_3 * this->full_magic_value;
    }

    int32_t new_value;

    switch (this->effect_id) {
        case 0: // no-op
            break;
        case modifier::price: // no-op
            break;
        case modifier::body:
            unit->body += value;
            if (!(this->usage_type & 8)) {
                unit->equipment_extra.body += value;
            }
            unit->body = (std::min)((int32_t)unit->body, 100);
            unit->field_0x150 |= 0x20030009;
            break;
        case modifier::mind:
            unit->mind += value;
            if (!(this->usage_type & 8)) {
                unit->equipment_extra.mind += value;
            }
            unit->mind = (std::min)((int32_t)unit->mind, 100);
            unit->field_0x150 |= 0x20040000;
            break;
        case modifier::reaction:
            unit->reaction += value;
            if (!(this->usage_type & 8)) {
                unit->equipment_extra.reaction += value;
            }
            unit->reaction = (std::min)((int32_t)unit->reaction, 100);
            unit->field_0x150 |= 0x20074000;
            break;
        case modifier::spirit:
            unit->spirit += value;
            if (!(this->usage_type & 8)) {
                unit->equipment_extra.spirit += value;
            }
            unit->spirit = (std::min)((int32_t)unit->spirit, 100);
            unit->field_0x150 |= 0x2100000A;
            break;
        case modifier::health:
            new_value = unit->hp + value;
            if (new_value >= unit->hp_max) {
                new_value = unit->hp_max;
            }
            unit->hp = new_value;
            unit->field_0x150 |= 1;
            break;
        case modifier::healthmax:
            unit->hp += value;
            unit->equipment_extra.hp_max += value;
            unit->field_0x150 |= 9;
            break;
        case modifier::healthregeneration:
            unit->equipment_extra.hp_regen += value;
            break;
        case modifier::mana:
            new_value = unit->mp + value;
            if (new_value >= unit->mp_max) {
                new_value = unit->mp_max;
            }
            unit->mp = new_value;
            unit->field_0x150 |= 2;
            break;
        case modifier::manamax:
            if (unit->unit_attrs & 4) { // mage-only
                unit->mp += value;
                unit->equipment_extra.mp_max += value;
                unit->field_0x150 |= 0x0A;
            }
            break;
        case modifier::manaregeneration: 
            if (unit->unit_attrs & 4) { // mage-only
                unit->equipment_extra.mp_regen += value;
            }
            break;
        case modifier::tohit:
            unit->equipment_extra.hit_values.attack += value;
            unit->field_0x150 |= 0x10000;
            break;
        case modifier::damagemin:
        case modifier::damage:
        case modifier::damagebonus:
            unit->equipment_extra.hit_values.hand_damage_min += value;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::damagemax:
            unit->equipment_extra.hit_values.hand_damage_spread += value;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::defence:
            if (unit->VMethod8()) {
                unit->equipment_extra.protections.defense += value;
            } else {
                unit->protections.defense += value;
                if (unit->protections.defense < 0) {
                    unit->protections.defense = 0;
                }
            }
            unit->field_0x150 |= 0x4000;
            break;
        case modifier::absorbtion:
            if (unit->VMethod8()) {
                unit->equipment_extra.protections.absorption += value;
                if (unit->equipment_extra.protections.absorption < 0) {
                    unit->equipment_extra.protections.absorption = 0;
                }
            } else {
                unit->protections.absorption += value;
            }
            unit->field_0x150 |= 0x8000;
            break;
        case modifier::speed:
            if (unit->VMethod8()) {
                unit->equipment_extra.speed += value;
            } else {
                unit->speed += value;
            }
            unit->field_0x150 |= 0x80000;
            break;
        case modifier::rotationspeed:
            unit->eye->rotation_speed += value;
            break;
        case modifier::scanrange:
            if (unit->VMethod8()) {
                unit->equipment_extra.scan_range += value << 8;
            } else {
                unit->scan_range += value << 8;
            }
            unit->field_0x150 |= 0x40000;
            break;
        case modifier::protection0:
            unit->equipment_extra.protections.magic_protections[0] += value;
            if (unit->equipment_extra.protections.magic_protections[0] < 0) {
                unit->equipment_extra.protections.magic_protections[0] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::protectionfire:
            unit->equipment_extra.protections.magic_protections[1] += value;
            if (unit->equipment_extra.protections.magic_protections[1] < 0) {
                unit->equipment_extra.protections.magic_protections[1] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::protectionwater:
            unit->equipment_extra.protections.magic_protections[2] += value;
            if (unit->equipment_extra.protections.magic_protections[2] < 0) {
                unit->equipment_extra.protections.magic_protections[2] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::protectionair:
            unit->equipment_extra.protections.magic_protections[3] += value;
            if (unit->equipment_extra.protections.magic_protections[3] < 0) {
                unit->equipment_extra.protections.magic_protections[3] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::protectionearth:
            unit->equipment_extra.protections.magic_protections[4] += value;
            if (unit->equipment_extra.protections.magic_protections[4] < 0) {
                unit->equipment_extra.protections.magic_protections[4] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::protectionastral:
            unit->equipment_extra.protections.magic_protections[5] += value;
            if (unit->equipment_extra.protections.magic_protections[5] < 0) {
                unit->equipment_extra.protections.magic_protections[5] = 0;
            }
            unit->field_0x150 |= 0x1000000;
            break;
        case modifier::fighterskill0:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[0] += value;
            }
            break;
        case modifier::skillblade:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[1] += value;
            }
            unit->field_0x150 |= 0x30004;
            break;
        case modifier::skillaxe:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[2] += value;
            }
            unit->field_0x150 |= 0x30004;
            break;
        case modifier::skillbludgeon:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[3] += value;
            }
            unit->field_0x150 |= 0x30004;
            break;
        case modifier::skillpike:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[4] += value;
            }
            unit->field_0x150 |= 0x30004;
            break;
        case modifier::skillshooting:
            if (!(unit->unit_attrs & 4)) {
                unit->equipment_extra.hit_values.skill_levels[5] += value;
            }
            unit->field_0x150 |= 0x30004;
            break;
        case modifier::mageskill0:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[0] += value;
            }
            break;
        case modifier::skillfire:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[1] += value;
            }
            unit->field_0x150 |= 4;
            break;
        case modifier::skillwater:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[2] += value;
            }
            unit->field_0x150 |= 4;
            break;
        case modifier::skillair:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[3] += value;
            }
            unit->field_0x150 |= 4;
            break;
        case modifier::skillearth:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[4] += value;
            }
            unit->field_0x150 |= 4;
            break;
        case modifier::skillastral:
            if (unit->unit_attrs & 4) {
                unit->equipment_extra.hit_values.skill_levels[5] += value;
            }
            unit->field_0x150 |= 4;
            break;
        case modifier::itemlore:
        case modifier::magiclore:
        case modifier::creaturelore:
            LogMessage("Effect affects not implemented yet parameter");
            break;
        case modifier::castspell: // handled in damage calculation code
            break;
        case modifier::teachspell:
            if (unit->spell_book != nullptr) {
                if (unit->spell_book->sub_53DB79(value) == nullptr) {
                    Spell* spell = new Spell(value);
                    unit->spell_book->sub_53D7F0(value, spell);
                }
            }
            unit->field_0x150 |= 0x100000;
            break;
        case modifier::damagefire:
            unit->equipment_extra.hit_values.spell_id = 1;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::damagewater:
            unit->equipment_extra.hit_values.spell_id = 2;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::damageair:
            unit->equipment_extra.hit_values.spell_id = 3;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::damageearth:
            unit->equipment_extra.hit_values.spell_id = 4;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->field_0x150 |= 0x20000;
            break;
        case modifier::damageastral:
            unit->equipment_extra.hit_values.spell_id = 5;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->field_0x150 |= 0x20000;
            break;
        default:
            LogMessage("Oops - unknown Effect case");
            break;
    }

    unit->VMethod18();
}

// 540941
int32_t Effect::EffectPrice() {
    if (this->effect_id == 0) {
        return 0;
    } else if (this->effect_id == modifier::castspell) {
        return this->spell_value * g_GameDataRes.magics[this->effect_id].values[0].mana_cost;
    } else if (this->effect_id >= modifier::damagefire && this->effect_id <= modifier::damageastral) {
        return (this->damage_min + this->damage_spread) * g_GameDataRes.magics[this->effect_id].values[0].mana_cost;
    } else {
        return this->full_magic_value * g_GameDataRes.magics[this->effect_id].values[0].mana_cost;
    }
}

// 540A33
int64_t Effect::MagicPriceBonus(int32_t total) {
    return (int64_t)((std::pow(1.5, total / 70.0) + 1.0) * total * 50.0);
}
