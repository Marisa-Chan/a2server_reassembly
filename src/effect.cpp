#include "effect.h"

#include <cmath>

#include "building.h"
#include "constants.h"
#include "eye.h"
#include "game_app.h"
#include "net.h"
#include "spell.h"
#include "table.h"
#include "unit.h"
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
#include "patch/config_new.h"
#include "patch/damage_filter.h"
#include "patch/flags.h"
#endif

IMPLEMENT_SERIAL(Effect, Token, 1); // 637098.

// 53f761 (client: 526d61)
void Effect::ApplyToUnit(Unit* unit) {
    if (this->itemDataID == spell::poison_cloud) {
        int32_t damage = -this->spell_or_damage;
        if (unit->protections.magic_protections[magic_sphere::water] != 0) {
            damage = (int32_t)((damage * (100 - unit->protections.magic_protections[magic_sphere::water])) / 100.0 + 0.5);
        }
        if (damage != 0) {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            // srv_pvm.xck + verify_damage hook (53F7CD -> 5048D3)
            int32_t applied_damage = TangarOnDamage(this->caster, unit, static_cast<int16_t>(damage));
            unit->hp -= static_cast<int16_t>(applied_damage);
#else
            unit->hp -= damage;
#endif
            if (this->caster != nullptr) {
                if (this->caster->hp < 0) {
                    this->caster = nullptr;
                } else {
                    this->caster->AwardDamageExperience(unit, damage, spell::poison_cloud);
                }
            }
            g_NetStru1_main.SendLevelUp(unit, 0);
        }
    } else if (this->itemDataID == spell::light) {
        if (unit->IsHumanoid() == 0) {
            unit->scan_range -= this->spell_or_damage * 256;
            unit->item_op_dirty_flags |= UNIT_UPD_SCAN_RANGE;
        }
    } else if (this->itemDataID == spell::darkness) {
        if (unit->IsHumanoid() != 0) {
            unit->equipment_extra.scan_range += this->spell_or_damage * 256;
            unit->scan_range += this->spell_or_damage * 256;
            unit->item_op_dirty_flags |= UNIT_UPD_SCAN_RANGE;
        }
    } else {
        this->ApplyStatDelta(unit, 1);
    }
}


// 53edb0 (client: 5268e7) - Returns true if both effects have matching effect_id, usage_type, and spell values
int Effect::Matches(Effect* other) {
    return this->effect_id == other->effect_id &&
           usage_type == other->usage_type &&
           this->spell_or_damage == other->spell_or_damage &&
           this->spell_value == other->spell_value;
}


// 53ec02 (client: 526739)
Effect::Effect() {
    this->typeId = 0;
    this->effect_id = 0;
    this->usage_type = 0;
    this->spell_or_damage = 0;
    this->spell_value = 0;
    this->itemDataID = 0;
    this->caster = nullptr;
}


// 53ec55 (client: 52678c)
Effect::Effect(const CString& name) {
    Effect* tmp = CreateFromString(name);
    if (tmp) {
        this->effect_id = tmp->effect_id;
        this->usage_type = tmp->usage_type;
        this->spell_or_damage = tmp->spell_or_damage;
        this->spell_value = tmp->spell_value;
        this->itemDataID = tmp->itemDataID;

        delete tmp;

        this->caster = 0;
    } else {
        this->effect_id = 0;
        this->usage_type = 0;
        this->spell_or_damage = 0;
        this->spell_value = 0;
        this->itemDataID = 0;
    }
}

// 53ed4d (client: 526884)
Effect::Effect(const Effect* src)
: Token(*src) {
    this->effect_id = src->effect_id;
    this->usage_type = src->usage_type;
    this->spell_or_damage = src->spell_or_damage;
    this->spell_value = src->spell_value;
    this->itemDataID = src->itemDataID;
    this->caster = src->caster;
}

// 57c350 (client: 54ce00)
Effect::~Effect() {
}

// 53e1ce (client: 5262d9)
void Effect::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->effect_id;
        ar << this->usage_type;
        ar << this->spell_or_damage;
        ar << this->spell_value;
        ar << this->itemDataID;
    } else {
        ar >> this->effect_id;
        ar >> this->usage_type;
        ar >> this->spell_or_damage;
        ar >> this->spell_value;
        ar >> this->itemDataID;
    }
}


// 53ee54 (client: 52698b)
void Effect::TickOnUnit(Unit* unit) {
    if ((this->usage_type & USAGE_TICKING_MASK) == 0) {
        return;
    }
    if ((this->usage_type & USAGE_CONTINUOUS) != 0 && (this->spell_value & 7) == 0) {
        unit->item_op_dirty_flags = 0;
        this->ApplyToUnit(unit);
        g_NetStru1_main.SendUnitUpdate(unit, nullptr, unit->item_op_dirty_flags, EQUIP_SLOTS_ALL, 0, 0);
    }
    this->spell_value -= 1;
    if (this->spell_value == 0) {
        if ((this->usage_type & USAGE_CONTINUOUS) == 0) {
            unit->item_op_dirty_flags = 0;
            this->ModifyVision(unit);
            g_NetStru1_main.SendUnitUpdate(unit, nullptr, unit->item_op_dirty_flags, EQUIP_SLOTS_ALL, 0, 0);
        }
        unit->enchantments &= ~(1u << (this->itemDataID & 0x1f));
        if (this->itemDataID != 0) {
            g_NetStru1_main.BroadcastEffect(this, unit, EFFECT_GONE);
        }
        this->usage_type |= USAGE_EXPIRED;
    }
}

// 53efb2 (client: 526afe)
void Effect::RemoveFromUnit(Unit* unit) {
    unit->item_op_dirty_flags = 0;
    if ((this->usage_type & USAGE_TICKING_MASK) != 0 && (this->itemDataID == spell::bless || this->itemDataID == spell::curse)) {
        Effect* counterpart = nullptr; // Note: can reuse Unit::FindEnchantment here.
        for (POSITION pos = unit->_effects.GetHeadPosition(); pos != nullptr; ) {
            Effect* e = unit->_effects.GetNext(pos);
            if (e->itemDataID == spell::bless && this->itemDataID == spell::curse) {
                counterpart = e;
                break;
            }
            if (e->itemDataID == spell::curse && this->itemDataID == spell::bless) {
                counterpart = e;
                break;
            }
        }

        if (counterpart != nullptr) {
            unit->enchantments &= ~(1u << (counterpart->itemDataID & 0x1f));
            counterpart->ModifyVision(unit);
            g_NetStru1_main.BroadcastEffect(counterpart, unit, EFFECT_GONE);
            POSITION found_pos = nullptr;
            for (POSITION pos = unit->_effects.GetHeadPosition(); pos != nullptr; ) {
                POSITION cur_pos = pos;
                Effect* e = unit->_effects.GetNext(pos);
                if (e == counterpart) {
                    found_pos = cur_pos;
                    break;
                }
            }
            if (found_pos != nullptr) {
                unit->_effects.RemoveAt(found_pos);
            }
            delete counterpart;
            g_NetStru1_main.SendUnitUpdate(unit, nullptr, unit->item_op_dirty_flags, EQUIP_SLOTS_ALL, 0, 0);
            return;
        }
    }

    if ((this->usage_type & USAGE_TICKING_MASK) == 0) {
        this->ApplyToUnit(unit);
    } else {
        Effect* existing = nullptr;
        for (POSITION pos = unit->_effects.GetHeadPosition(); pos != nullptr; ) {
            Effect* e = unit->_effects.GetNext(pos);
            if (e->itemDataID == this->itemDataID) {
                existing = e;
                break;
            }
        }
        if (existing == nullptr) {
            Effect* new_effect = new Effect(this);
            if (new_effect->itemDataID != spell::poison_cloud) {
                new_effect->ApplyToUnit(unit);
            }
            unit->_effects.AddTail(new_effect);
            unit->enchantments |= 1u << (this->itemDataID & 0x1f);
            if (this->itemDataID != 0) {
                g_NetStru1_main.BroadcastEffect(this, unit, 0);
            }
        } else {
            existing->ModifyVision(unit);

            const bool protection_or_shield = (
                    existing->itemDataID == spell::protection_from_fire ||
                    existing->itemDataID == spell::protection_from_water ||
                    existing->itemDataID == spell::protection_from_air ||
                    existing->itemDataID == spell::protection_from_earth ||
                    existing->itemDataID == spell::shield
            );

            if (existing->spell_or_damage == 150 && protection_or_shield) {
                existing->spell_or_damage = this->spell_or_damage;
                existing->spell_value = this->spell_value;
            } else {
                if (this->spell_or_damage > existing->spell_or_damage) {
                    existing->spell_or_damage = this->spell_or_damage;
                }
                if (this->spell_value > existing->spell_value) {
                    existing->spell_value = this->spell_value;
                }
            }
            if (existing->itemDataID != spell::poison_cloud) {
                existing->ApplyToUnit(unit);
            }
        }
    }
    g_NetStru1_main.SendUnitUpdate(unit, nullptr, unit->item_op_dirty_flags, EQUIP_SLOTS_ALL, 0, 0);
}

// 53fa2b (client: 527122)
void Effect::ApplyStatDelta(Unit* unit, int32_t stack_delta) {
#if defined(A2SERVER_PATCH) && defined(A2TANGAR_HAT)
    // test_health hook (53FA2B): block single-use health potions when SVF_NOHEALING is active.
    if (this->effect_id == modifier::health && this->usage_type == USAGE_SINGLE_USE && (Config::ServerFlags & SVF_NOHEALING) != 0) {
        return;
    }
#endif
    int32_t value;
    if (this->usage_type & USAGE_TIMED_MASK) {
        value = stack_delta * this->spell_or_damage;
    } else {
        value = stack_delta * this->full_magic_value;
    }

    int32_t new_value;

    switch (this->effect_id) {
        case 0: // no-op
            break;
        case modifier::price: // no-op
            break;
        case modifier::body:
            unit->body += value;
            if (!(this->usage_type & USAGE_SINGLE_USE)) {
                unit->equipment_extra.body += value;
            }
            unit->body = (std::min)((int32_t)unit->body, 100);
            unit->item_op_dirty_flags |= (UNIT_UPD_BASE_STATS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE | UNIT_UPD_HP | UNIT_UPD_HP_MP_MAX);
            break;
        case modifier::mind:
            unit->mind += value;
            if (!(this->usage_type & USAGE_SINGLE_USE)) {
                unit->equipment_extra.mind += value;
            }
            unit->mind = (std::min)((int32_t)unit->mind, 100);
            unit->item_op_dirty_flags |= (UNIT_UPD_BASE_STATS | UNIT_UPD_SCAN_RANGE);
            break;
        case modifier::reaction:
            unit->reaction += value;
            if (!(this->usage_type & USAGE_SINGLE_USE)) {
                unit->equipment_extra.reaction += value;
            }
            unit->reaction = (std::min)((int32_t)unit->reaction, 100);
            unit->item_op_dirty_flags |= (UNIT_UPD_BASE_STATS | UNIT_UPD_DEFENSE | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE | UNIT_UPD_SCAN_RANGE);
            break;
        case modifier::spirit:
            unit->spirit += value;
            if (!(this->usage_type & USAGE_SINGLE_USE)) {
                unit->equipment_extra.spirit += value;
            }
            unit->spirit = (std::min)((int32_t)unit->spirit, 100);
            unit->item_op_dirty_flags |= (UNIT_UPD_BASE_STATS | UNIT_UPD_MAGIC_PROTECT | UNIT_UPD_MP | UNIT_UPD_HP_MP_MAX);
            break;
        case modifier::health:
            new_value = unit->hp + value;
            if (new_value >= unit->hp_max) {
                new_value = unit->hp_max;
            }
            unit->hp = new_value;
            unit->item_op_dirty_flags |= UNIT_UPD_HP;
            break;
        case modifier::healthmax:
            unit->hp += value;
            unit->equipment_extra.hp_max += value;
            unit->item_op_dirty_flags |= (UNIT_UPD_HP | UNIT_UPD_HP_MP_MAX);
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
            unit->item_op_dirty_flags |= UNIT_UPD_MP;
            break;
        case modifier::manamax:
            if (unit->unit_attrs & Unit::ATTR_MAGE) { // mage-only
                unit->mp += value;
                unit->equipment_extra.mp_max += value;
                unit->item_op_dirty_flags |= (UNIT_UPD_MP | UNIT_UPD_HP_MP_MAX);
            }
            break;
        case modifier::manaregeneration: 
            if (unit->unit_attrs & Unit::ATTR_MAGE) { // mage-only
                unit->equipment_extra.mp_regen += value;
            }
            break;
        case modifier::tohit:
            unit->equipment_extra.hit_values.attack += value;
            unit->item_op_dirty_flags |= UNIT_UPD_ATTACK;
            break;
        case modifier::damagemin:
        case modifier::damage:
        case modifier::damagebonus:
            unit->equipment_extra.hit_values.hand_damage_min += value;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::damagemax:
            unit->equipment_extra.hit_values.hand_damage_spread += value;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::defence:
            if (unit->IsHumanoid()) {
                unit->equipment_extra.protections.defense += value;
            } else {
                unit->protections.defense += value;
                if (unit->protections.defense < 0) {
                    unit->protections.defense = 0;
                }
            }
            unit->item_op_dirty_flags |= UNIT_UPD_DEFENSE;
            break;
        case modifier::absorbtion:
            if (unit->IsHumanoid()) {
                unit->equipment_extra.protections.absorption += value;
                if (unit->equipment_extra.protections.absorption < 0) {
                    unit->equipment_extra.protections.absorption = 0;
                }
            } else {
                unit->protections.absorption += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_ABSORB;
            break;
        case modifier::speed:
            if (unit->IsHumanoid()) {
                unit->equipment_extra.speed += value;
            } else {
                unit->speed += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SPEED;
            break;
        case modifier::rotationspeed:
            unit->eye->rotation_speed += value;
            break;
        case modifier::scanrange:
            if (unit->IsHumanoid()) {
                unit->equipment_extra.scan_range += value << 8;
            } else {
                unit->scan_range += value << 8;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SCAN_RANGE;
            break;
        case modifier::protection0:
            unit->equipment_extra.protections.magic_protections[magic_sphere::none] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::none] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::none] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::protectionfire:
            unit->equipment_extra.protections.magic_protections[magic_sphere::fire] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::fire] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::fire] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::protectionwater:
            unit->equipment_extra.protections.magic_protections[magic_sphere::water] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::water] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::water] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::protectionair:
            unit->equipment_extra.protections.magic_protections[magic_sphere::air] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::air] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::air] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::protectionearth:
            unit->equipment_extra.protections.magic_protections[magic_sphere::earth] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::earth] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::earth] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::protectionastral:
            unit->equipment_extra.protections.magic_protections[magic_sphere::astral] += value;
            if (unit->equipment_extra.protections.magic_protections[magic_sphere::astral] < 0) {
                unit->equipment_extra.protections.magic_protections[magic_sphere::astral] = 0;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_MAGIC_PROTECT;
            break;
        case modifier::fighterskill0:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::unarmed] += value;
            }
            break;
        case modifier::skillblade:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::blade] += value;
            }
            unit->item_op_dirty_flags |= (UNIT_UPD_SKILL_LEVELS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
            break;
        case modifier::skillaxe:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::axe] += value;
            }
            unit->item_op_dirty_flags |= (UNIT_UPD_SKILL_LEVELS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
            break;
        case modifier::skillbludgeon:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::bludgeon] += value;
            }
            unit->item_op_dirty_flags |= (UNIT_UPD_SKILL_LEVELS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
            break;
        case modifier::skillpike:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::pike] += value;
            }
            unit->item_op_dirty_flags |= (UNIT_UPD_SKILL_LEVELS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
            break;
        case modifier::skillshooting:
            if (!(unit->unit_attrs & Unit::ATTR_MAGE)) {
                unit->equipment_extra.hit_values.skill_levels[weapon_attack::shooting] += value;
            }
            unit->item_op_dirty_flags |= (UNIT_UPD_SKILL_LEVELS | UNIT_UPD_ATTACK | UNIT_UPD_HAND_DAMAGE);
            break;
        case modifier::mageskill0:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::none] += value;
            }
            break;
        case modifier::skillfire:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::fire] += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SKILL_LEVELS;
            break;
        case modifier::skillwater:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::water] += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SKILL_LEVELS;
            break;
        case modifier::skillair:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::air] += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SKILL_LEVELS;
            break;
        case modifier::skillearth:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::earth] += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SKILL_LEVELS;
            break;
        case modifier::skillastral:
            if (unit->unit_attrs & Unit::ATTR_MAGE) {
                unit->equipment_extra.hit_values.skill_levels[magic_sphere::astral] += value;
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SKILL_LEVELS;
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
                if (unit->spell_book->GetSpell(value) == nullptr) {
                    Spell* spell = new Spell(value);
                    unit->spell_book->SetSpell(value, spell);
                }
            }
            unit->item_op_dirty_flags |= UNIT_UPD_SPELLS;
            break;
        case modifier::damagefire:
            unit->equipment_extra.hit_values.spell_id = magic_sphere::fire;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::damagewater:
            unit->equipment_extra.hit_values.spell_id = magic_sphere::water;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::damageair:
            unit->equipment_extra.hit_values.spell_id = magic_sphere::air;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::damageearth:
            unit->equipment_extra.hit_values.spell_id = magic_sphere::earth;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        case modifier::damageastral:
            unit->equipment_extra.hit_values.spell_id = magic_sphere::astral;
            unit->equipment_extra.hit_values.some_damage2_min = this->damage_min;
            unit->equipment_extra.hit_values.some_damage2_spread = this->damage_spread;
            unit->item_op_dirty_flags |= UNIT_UPD_HAND_DAMAGE;
            break;
        default:
            LogMessage("Oops - unknown Effect case");
            break;
    }

    unit->RecalcStats();
}

// 53f92c (client: 527023)
void Effect::ModifyVision(Unit* unit) {
    if (this->itemDataID == spell::light) {
        if (unit->IsHumanoid() == 0) {
            unit->scan_range += this->spell_or_damage * 256;
            unit->item_op_dirty_flags |= UNIT_UPD_SCAN_RANGE;
        }
    } else if (this->itemDataID == spell::darkness) {
        if (unit->IsHumanoid() != 0) {
            unit->equipment_extra.scan_range -= this->spell_or_damage * 256;
            unit->scan_range -= this->spell_or_damage * 256;
            unit->item_op_dirty_flags |= UNIT_UPD_SCAN_RANGE;
        }
    } else if (this->itemDataID != spell::poison_cloud) {
        this->ApplyStatDelta(unit, -1);
    }
}

// 541c87 (client: 52915e)
int32_t Effect::CalcScrollPrice() {
    if (this->effect_id == modifier::castspell) {
#ifdef A2CLIENT_PATCH
        // p10st2.xck (52916f..529195): reject spell_id == 0 or > 0x1D (spell::max_spell_id)
        uint8_t spell_id = this->damage_min;
        if (spell_id == 0 || spell_id > spell::max_spell_id) {
            this->_exp = 0;
            return this->_exp;
        }
        int32_t scroll_cost = g_GameDataRes.spells[spell_id].Values()[0].scroll_cost;
#else
        if (this->spell_or_damage > spell::max_spell_id) {
            this->spell_or_damage = spell::drain_life;
        }

        int32_t scroll_cost = g_GameDataRes.spells[this->spell_or_damage].Values()[0].scroll_cost;
#endif
        double level = (double)this->spell_value / 30.0 + 1.0;
        this->_exp = (int32_t)(std::pow(2.0, std::log(level) / std::log(1.2)) * (scroll_cost * 10));
    } else {
        this->_exp = 0;
    }
    return this->_exp;
}

// 541d59 (client: 52921c)
int32_t Effect::CalcChargePercent(double mana) {
    int32_t result = 1;
    if ((this->usage_type & USAGE_TICKING_MASK) == 0) {
        if (this->effect_id == modifier::castspell) {
            result = (int32_t)(mana * 10.0 / g_GameDataRes.spells[this->spell_or_damage].Values()[0].book_cost);
            if (result > 100) {
                result = 100;
            }
            if (result <= 0) {
                result = -1;
            }
        } else {
            int32_t affect_min = g_GameDataRes.magics[this->effect_id].Values()[0].affect_min;
            int32_t affect_max = g_GameDataRes.magics[this->effect_id].Values()[0].affect_max;
            int32_t mana_cost = g_GameDataRes.magics[this->effect_id].Values()[0].mana_cost;
            result = (int32_t)(std::log(mana / (mana_cost * 500.0)) / std::log(20.0) * affect_max + 0.5);
            if (result > affect_max) {
                result = affect_max;
            }
            if (result < affect_min) {
                result = -1;
            }
        }
    }
    return result;
}

// 541eb5 (client: 529380)
void Effect::RollStrength(int32_t power) {
    int32_t r = Random1N(power);
    if (this->effect_id == modifier::castspell) {
        this->spell_value = (uint16_t)r;
        if (this->spell_value > 100) {
            this->spell_value = 100;
        }
    } else if (this->effect_id >= modifier::damagefire && this->effect_id <= modifier::damageastral) {
        if (power > 255) {
            power = 255;
        }
        this->damage_min = (uint8_t)Random1N(power);
        this->damage_spread = (uint8_t)Random1N(power / 2);
    } else {
        int32_t affect_min = g_GameDataRes.magics[this->effect_id].Values()[0].affect_min;
        if (r < affect_min) {
            this->full_magic_value = affect_min;
        } else {
            this->full_magic_value = r;
        }
    }
}

// 541fd7 (client: 5294a4) - Compute effect level from budget and magic volume
int32_t Effect::CalcEffectLevel(int32_t budget, int32_t magic_volume) {
    const MagicInfoData* magic_data = &g_GameDataRes.magics[this->effect_id].Values()[0];
    int32_t max_level = magic_volume / magic_data->mana_cost;
    int32_t level;
    if (this->effect_id == modifier::castspell) {
        double cost_ratio = budget / (g_GameDataRes.spells[this->spell_or_damage].Values()[0].scroll_cost * 10.0);
        double log2_ratio = std::log(cost_ratio) / std::log(2.0);
        if (log2_ratio <= 0.0) {
            level = -1;
        } else {
            level = (int32_t)((std::pow(1.2, log2_ratio) - 1.0) * 30.0);
        }
        level = (std::min)(level, max_level);
        if (level > 100) {
            level = 100;
        }
        if (level < 0) {
            level = -1;
        }
    } else {
        level = (int32_t)(std::log(budget / (magic_volume * 50.0) - 1.0) / std::log(1.5) * 70.0 / magic_data->mana_cost);
        level = (std::min)(level, max_level);
        if (level < magic_data->affect_min) {
            return -1;
        }
        if (level > magic_data->affect_max) {
            level = magic_data->affect_max;
        }
    }
    return level;
}

// 541a5d (client: 528f6d)
int32_t Effect::ParseUsage(CString* str, int32_t* out_value) {
    str->TrimLeft();
    str->TrimRight();

    if (*str == "permanent") {
        return USAGE_PERMANENT;
    }
    if (*str == "singleuse") {
        return USAGE_SINGLE_USE;
    }
    int32_t pos = str->Find("charges");
    if (pos != -1) {
        *str = str->Right(str->GetLength() - pos - 8);
        str->TrimLeft();
        *out_value = Effect::AtoI(str);
        return USAGE_CHARGES;
    }
    pos = str->Find("duration");
    if (pos != -1) {
        *str = str->Right(str->GetLength() - pos - 9);
        str->TrimLeft();
        *out_value = Effect::AtoI(str);
        return USAGE_DURATION;
    }
    pos = str->Find("continuous");
    if (pos != -1) {
        *str = str->Right(str->GetLength() - pos - 11);
        str->TrimLeft();
        *out_value = Effect::AtoI(str);
        return USAGE_CONTINUOUS;
    }
    return USAGE_PERMANENT;
}

// 6370C8
static const char* const g_EffectNames[50] = {
    "unused_value_0", "price", "body", "mind", "reaction", "spirit", "health", "healthmax",
    "healthregeneration", "mana", "manamax", "manaregeneration", "tohit", "damagemin", "damagemax",
    "defence", "absorbtion", "speed", "rotationspeed", "scanrange", "protection0", "protectionfire",
    "protectionwater", "protectionair", "protectionearth", "protectionastral", "fighterskill0",
    "skillblade", "skillaxe", "skillbludgeon", "skillpike", "skillshooting", "mageskill0",
    "skillfire", "skillwater", "skillair", "skillearth", "skillastral", "itemlore", "magiclore",
    "creaturelore", "castspell", "teachspell", "damage", "damagefire", "damagewater", "damageair",
    "damageearth", "damageastral", "damagebonus"
};

// 541046 (client: 5285cb)
Effect* Effect::CreateFromString(const CString& effect_str) {
    if (effect_str.GetLength() == 0) {
        return nullptr;
    }

    CString name;    // effect name part (before '=')
    CString value;   // first part after '=' (before ',' or ':')
    CString subpart; // part before ':' when present
    CString str = effect_str;
    Effect* effect = new Effect();
    str += ',';
    str.MakeLower();

    int32_t eq_pos = str.Find('=');
    if (eq_pos == -1) {
        delete effect;
        return nullptr;
    }

    name = str.Left(eq_pos);
    name.TrimLeft();
    name.TrimRight();
    str = str.Right(str.GetLength() - eq_pos - 1);
    str.TrimLeft();
    str.TrimLeft();

    int32_t effect_id = 0;
    for (int32_t i = 0; i < 50; i++) {
        if (name == g_EffectNames[i]) {
            effect_id = i;
            break;
        }
    }
    if (effect_id == 0) {
        delete effect;
        return nullptr;
    }
    effect->effect_id = effect_id;

    int32_t colon_pos = str.Find(':');
    int32_t comma_pos = str.Find(',');
    value = str.Left(comma_pos);
    value.TrimLeft();
    value.TrimRight();
    str = str.Right(str.GetLength() - comma_pos - 1);

    int32_t usage_flags;
    if (colon_pos < comma_pos && colon_pos != -1) {
        subpart = value.Left(colon_pos);
        subpart.TrimLeft();
        subpart.TrimRight();
        value = value.Right(value.GetLength() - colon_pos - 1);
        value.TrimLeft();

        int32_t parsed_value = 0;
        if (effect_id == modifier::castspell || effect_id == modifier::teachspell) {
            int32_t spell_id = Effect::GetSpellIDByName(&subpart);
            effect->spell_or_damage = (int16_t)spell_id;
            effect->spell_value = 0;
            if (spell_id == -1) {
                delete effect;
                return nullptr;
            }
        } else if (effect_id >= modifier::damage && effect_id <= modifier::damageastral) {
            Effect::ParseDmg(&subpart, &effect->damage_min, &effect->damage_spread);
        } else {
            parsed_value = Effect::AtoI(&subpart);
            effect->full_magic_value = parsed_value;
        }

        if (effect->effect_id == modifier::castspell) {
            effect->spell_value = (uint16_t)Effect::_StrToInt(&value);
            effect->usage_type = 0;
            usage_flags = 0;
        } else {
            usage_flags = Effect::ParseUsage(&value, &parsed_value);
            effect->usage_type = (uint8_t)usage_flags;
        }

        if ((usage_flags & USAGE_TIMED_MASK) != 0) {
            effect->spell_value = (uint16_t)(parsed_value << 4);
        }
    } else if (effect_id == modifier::castspell || effect_id == modifier::teachspell) {
        int32_t spell_id = Effect::GetSpellIDByName(&value);
        effect->spell_or_damage = (int16_t)spell_id;
        effect->spell_value = 0;
        if (spell_id == -1) {
            delete effect;
            return nullptr;
        }
    } else if (effect_id >= modifier::damage && effect_id <= modifier::damageastral) {
        Effect::ParseDmg(&value, &effect->damage_min, &effect->damage_spread);
    } else {
        int32_t val = Effect::AtoI(&value);
        effect->full_magic_value = val;
        effect->_exp = val * 50;
    }
    return effect;
}

// 54180c (client: 528d43)
void Effect::ParseDmg(CString* str, uint8_t* dmg_min, uint8_t* dmg_spread) {
    int32_t dash_pos = str->Find('-');
    if (dash_pos == -1) {
        return;
    }
    CString left = str->Left(dash_pos);
    CString right = str->Right(str->GetLength() - dash_pos - 1);
    left.TrimLeft();
    left.TrimRight();
    right.TrimLeft();
    right.TrimRight();
    *dmg_min = (uint8_t)Effect::AtoI(&left);
    *dmg_spread = (uint8_t)Effect::AtoI(&right) - *dmg_min;
}

// 637190
static const char* const g_SpellNames[30] = {
    "unused_spell_0", "fire_arrow", "fire_ball", "wall_of_fire", "protection_from_fire",
    "ice_missile", "poison_cloud", "blizzard", "protection_from_water", "acid_stream",
    "lightning", "prismatic_spray", "invisibility", "protection_from_air", "darkness",
    "light", "diamond_dust", "wall_of_earth", "stone_curse", "protection_from_earth",
    "bless", "haste", "control_spirit", "teleport", "heal",
    "summon", "drain_life", "shield", "curse", "slow"
};

// 541963 (client: 528e94)
int32_t Effect::GetSpellIDByName(CString* name) {
    for (int32_t i = 0; i < 30; i++) {
        if (*name == g_SpellNames[i]) {
            return i;
        }
    }
    return -1;
}

// 5418f1 (client: 528e22)
int32_t Effect::_StrToInt(CString* str) {
    str->TrimLeft();
    str->TrimRight();
    CString span = str->SpanIncluding("+1234567890");
    return Effect::AtoI(&span);
}

// 5419c3 (client: 528edb)
int32_t Effect::AtoI(CString* str) {
    int32_t value = 0;
    if (str->GetLength() != 0) {
        *str = str->SpanIncluding("+-1234567890");
        sscanf(*str, "%d", &value);
    }
    return value;
}

// 540941 (client: 527fe9)
int32_t Effect::EffectPrice() {
    if (this->effect_id == 0) {
        return 0;
    } else if (this->effect_id == modifier::castspell) {
        return this->spell_value * g_GameDataRes.magics[this->effect_id].Values()[0].mana_cost;
    } else if (this->effect_id >= modifier::damagefire && this->effect_id <= modifier::damageastral) {
        return (this->damage_min + this->damage_spread) * g_GameDataRes.magics[this->effect_id].Values()[0].mana_cost;
    } else {
        return this->full_magic_value * g_GameDataRes.magics[this->effect_id].Values()[0].mana_cost;
    }
}

// 540a33 (client: 5280eb)
int64_t Effect::MagicPriceBonus(int32_t total) {
    return (int64_t)((std::pow(1.5, total / 70.0) + 1.0) * total * 50.0);
}

IMPLEMENT_SERIAL(DirectDamage, Effect, 1); // 6370b0

// 540b21 (client: 5281d6)
DirectDamage::DirectDamage() {
    this->caster = nullptr;
}

// 540b82 (client: 528237)
DirectDamage::DirectDamage(uint8_t hand_damage_min, uint8_t hand_damage_spread) {
    this->caster = nullptr;
    this->unit_to_hit.hand_damage_min = hand_damage_min;
    this->unit_to_hit.hand_damage_spread = hand_damage_spread;
}

// 540bf7 (client: 5282ac)
DirectDamage::DirectDamage(DirectDamage* src) : Effect(src) {
    this->unit_to_hit = src->unit_to_hit;
    this->caster = src->caster;
}

// 57c120 (client: 54cde0)
DirectDamage::~DirectDamage() {}

// 53e422 (client: 526379)
void DirectDamage::Serialize(CArchive& ar) {
    Effect::Serialize(ar);
    this->unit_to_hit.Serialize(ar);
}

// 540c77 (client: 52832c)
void DirectDamage::RemoveFromUnit(Unit* unit) {
    // WAT: that shouldn't be possible if the input is actually a `Unit*` and not a `Token*`.
    // Looks like in vanilla they're passing `Token*` and force-casting it. Eugh.
    if (unit->IsUnit() == 0) {
        if (unit->IsBuilding() != 0) {
            Building* building = reinterpret_cast<Building*>(unit);
            int32_t damage = building->RollDamage(&this->unit_to_hit, this->caster);
            building->hp -= damage;
            if (building->hp < 0) {
                building->hp = 0;
            }
            if (damage > 0) {
                g_NetStru1_main.SendLevelUp(unit, damage);
            }
        }
    } else {
        int32_t damage = unit->RollDamage(&this->unit_to_hit, this->caster);
        unit->hp -= (int16_t)damage;
        if (damage > 0) {
            if (this->caster != nullptr && this->itemDataID != 0) {
                this->caster->AwardDamageExperience(unit, damage, this->itemDataID);
            }
            g_NetStru1_main.SendLevelUp(unit, damage);
        }
    }
}
