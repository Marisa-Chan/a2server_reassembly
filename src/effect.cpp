#include "effect.h"

#include "constants.h"
#include "game_app.h"
#include "net.h"
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
            unit->scan_range -= (int16_t)this->spell_or_damage * 256;
            unit->field_0x150 |= 0x40000;
        }
    } else if (this->itemDataID == spell::light) {
        if (unit->VMethod8() != 0) {
            unit->equipment_extra.scan_range += (int16_t)this->spell_or_damage * 256;
            unit->scan_range += (int16_t)this->spell_or_damage * 256;
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
