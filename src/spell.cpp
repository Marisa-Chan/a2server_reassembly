#include "spell.h"

#include <algorithm>
#include <cmath>

#include "constants.h"
#include "effect.h"
#include "game_app.h"
#include "group.h"
#include "item.h"
#include "map_stuff.h"
#include "net.h"
#include "player.h"
#include "server.h"
#include "spell_effect.h"
#include "unit.h"
#include "unit_list.h"
#include "world.h"

// 53939E
int Spell::sub_53939E(uint8_t, uint8_t)
{
    return spell_info->values.GetData()[0].spell_target == 2;
}

// 539541
void Spell::sub_539541(uint32_t power)
{
    const SpellInfoData& info = spell_info->values.GetData()[0];
    uint8_t power_byte = power;

    this->max_range = info.max_range;
    if (this->spell_id == spell::teleport) {
        int32_t divisor = (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) ? 15 : 3;
        this->max_range += power_byte / divisor;
    } else if (this->max_range != 0) {
        this->max_range += power_byte / 30;
    }

    double multiplier = power_byte / 30.0 + 1.0;

    if (info.damage_min > 0) {
        this->damage_min = info.damage_min * multiplier;
    } else {
        this->damage_min = 0;
    }

    if (info.damage_max > 0) {
        this->damage_spread = info.damage_max * multiplier - this->damage_min;
    } else {
        this->damage_spread = 0;
    }

    if (info.duration > 0) {
        if (this->spell_id == spell::invisibility) {
            this->spell_power = power_byte << 4;
        } else if (this->spell_id == spell::stone_curse) {
            this->spell_power = power_byte * 480 / 100;
        } else {
            this->spell_power = std::pow(1.025, power_byte) * info.duration * 16.0;
        }
    } else if (info.area_duration > 0) {
        this->spell_power = info.area_duration * 16 + power_byte * 16 / 10;
    } else {
        this->spell_power = 0;
    }
}

// Distance between two TokenPos positions in sub-cell units (still in asm).
uint32_t __cdecl sub_5365AB(TokenPos* a, TokenPos* b);

// 539958
int32_t Spell::sub_539958(Unit* caster, Unit* target, int8_t x, int8_t y)
{
    if ((caster->unit_attrs & 4) && caster->some_item == nullptr) {
        if (this->mana_cost > caster->mp) {
            return 0;
		}
        caster->mp -= this->mana_cost;
    }

    TokenPos local_pos;
    if (target == nullptr) {
        local_pos = TokenPos(x, y, MapStuff_Instance);
    } else {
        local_pos = *target->position;
        if (target->VMethod3() > 1) {
            local_pos.SetCoords2(target->sub_528725() - 1, target->sub_528763() - 1);
		}
    }

    if (target != caster && (caster->enchantments & (1u << spell::invisibility))) {
        for (POSITION p = caster->_effects.GetHeadPosition(); p != nullptr; ) {
            Effect* eff = caster->_effects.GetNext(p);
            if (eff->itemDataID == spell::invisibility) {
                eff->spell_value = 1;
			}
        }
    }

    if (!(caster->unit_attrs & 4) && caster->weapon != nullptr && caster->weapon == (Weapon*)caster->some_item) {
        return 1;
    }

    if (this->spell_info == nullptr) {
        return 0;
	}

    const SpellInfoData& info = spell_info->values.GetData()[0];
    int32_t delay = 0;
    if (info.delivery_system == 2) {
        delay = sub_5365AB(caster->position, &local_pos) / info.effect_speed;
        if (spell_id == spell::lightning || spell_id == spell::prismatic_spray) {
            delay = 5;
		}
    } else if (caster->monster_info == nullptr) {
        return 1;
    }

    if (spell_id == spell::prismatic_spray) {
        this->sub_53940D(caster);
        this->sub_539C49(caster, target);
        return 1;
    }

    if (this->sub_5393C7()) {
        g_NetStru1_main.sub_51BAB0(caster, this, target, delay);
    } else {
        g_NetStru1_main.sub_51BB94(caster, this, &local_pos, delay);
    }

    return 1;
}

// 53D368
SpellBook::~SpellBook()
{
    for (int i = 0; i < spells.GetSize(); ++i) {
        delete spells[i];
        spells[i] = nullptr;
    }
}

void SpellBook::RefreshForHumanoid(Humanoid* humanoid)
{
	//53dbc5
	for (int index = 1; index < spells.GetSize(); ++index) {
		Spell* spell = spells[index];
		if (spell != nullptr) {
			uint8_t spell_id = spell->spell_id;
			SpellInfo& expected_info = g_GameDataRes.spells[spell_id];
			if (spell->spell_info != &expected_info) {
				spell = new Spell(index);

				spells[index] = spell;

				LogMessage("Bad spell data, restoring spell");
			}

			spell->sub_53940D(humanoid);
		}
	}
}

// 539F5A
void Spell::sub_539F5A(Unit* caster, Unit* target, int8_t x, int8_t y)
{
    if (this->spell_info == nullptr) {
        return;
    }

    const SpellInfoData& info = spell_info->values.GetData()[0];
    int32_t sphere = info.sphere;

    // Phase 1: Power calculation.
    int32_t power;
    if (caster->some_item == nullptr) {
        int32_t raw = caster->hit_values.skill_levels[sphere] + caster->mind - 30;
        if (raw < 0) {
            raw = 0;
        }
        if (raw > 255) {
            raw = 255;
        }
        power = raw;
    } else {
        Effect* cast_effect = caster->some_item->sub_548E4E();
        power = (cast_effect != nullptr) ? cast_effect->spell_value : 0;
    }
    this->sub_539541((uint8_t)power);
    double power_div_45 = power / 45.0 + 1.0;

    // Phase 2: blessed damage multiplier for deathmatch and softcore.
    if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
        if (caster->enchantments & (1u << spell::bless)) {
            this->damage_min *= 4;
            this->damage_spread *= 4;
            power_div_45 *= 4.0;
        }
    }

    if ((caster->unit_attrs & 2) != 0 && caster->some_item == nullptr) {
        caster->VMethod24(target, this->spell_id);
    }

    // Phase 3: Setup local position from target or coordinates
    TokenPos local_pos;
    if (target == nullptr) {
        local_pos = TokenPos(x, y, MapStuff_Instance);
    } else {
        local_pos = *target->position;
        if (target->VMethod3() > 1) {
            local_pos.SetCoords2(target->sub_528725() - 1, target->sub_528763() - 1);
        }
        if (target->VMethod7() != 0 && (target->unit_attrs & 8)) {
            return;
        }
    }

    // Helper: check if caster is the opposing arena team player (blocks certain spells in PvP).
    auto is_arena_blocked = [&]() -> bool {
        if (caster->pOwner != nullptr && g_ServerConfig.gameType == 2) {
            Player* owner = caster->pOwner;
            int32_t team_idx = 1 - owner->field_0xa70;
            if ((&g_Server->field60_0x20c)[team_idx] == owner->player_id) {
                return true;
            }
        }
        return false;
    };

    // Phase 4: DirectDamage path (if spell has damage and is not heal/drain)
    Effect* effect = nullptr;
    Effect* direct_damage = nullptr;
    if ((int32_t)this->damage_min + this->damage_spread > 0 && this->spell_id != spell::heal && this->spell_id != spell::drain_life) {
        DirectDamage* dd = new DirectDamage();
        direct_damage = dd;
        dd->itemDataID = this->spell_id;
        dd->caster = (caster->monster_info != nullptr) ? caster : nullptr;
        if (1 <= sphere && sphere <= 5) {
            dd->unit_to_hit.some_damage2_min = this->damage_min;
            dd->unit_to_hit.some_damage2_spread = this->damage_spread;
            dd->unit_to_hit.spell_id = sphere;
        }
        dd->typeId = this->spell_id * 2 + 9;
    } else {
        // Phase 5: Spell-specific effect creation
        switch (this->spell_id) {
        case 0x00:
            LogMessage("Invalid spell #0 - can't cast.");
            return;

        case spell::protection_from_fire:
        case spell::protection_from_water:
        case spell::protection_from_air:
        case spell::protection_from_earth:
            effect = new Effect(spell_info->effect);
            effect->usage_type |= 1;
            effect->spell_or_damage = power / 2;
            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                effect->spell_value = power * 16;
            } else {
                effect->spell_value = std::pow(1.025, power) * info.duration * 16.0;
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::poison_cloud:
            effect = new Effect(spell_info->effect);
            effect->spell_or_damage = effect->spell_or_damage * power_div_45;
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::invisibility:
            if (is_arena_blocked()) {
                return;
            }
            effect = new Effect();
            effect->usage_type |= 1;
            effect->spell_value = power * 16;
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            if (effect->spell_value == 0) {
                delete effect;
                effect = nullptr;
                return;
            }
            break;

        case spell::darkness:
            effect = new Effect(spell_info->effect);
            effect->itemDataID = this->spell_id;
            effect->spell_or_damage = -1 - power / 30;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::light:
            effect = new Effect(spell_info->effect);
            effect->itemDataID = this->spell_id;
            effect->spell_or_damage = power / 30 + 1;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::wall_of_earth:
            effect = new Effect();
            break;

        case spell::stone_curse:
            if (is_arena_blocked()) {
                return;
            }
            effect = new Effect(spell_info->effect);
            effect->usage_type |= 1;
            {
                int32_t factor = (caster->some_item == nullptr) ? 2 : 1;
                effect->spell_value = power * 240 * factor / 100;
            }
            if (target->VMethod7() != 0) {
                int32_t earth_protection = target->protections.magic_protections[4];
                int32_t max_duration = effect->spell_value * (100 - earth_protection) / 100;
                effect->spell_value = Random0N(max_duration);
                if (effect->spell_value == 0) {
                    delete effect;
                    effect = nullptr;
                    return;
                }
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::bless:
        case spell::curse:
            effect = new Effect();
            effect->usage_type |= 1;
            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                effect->spell_or_damage = 0;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power * 4 / 5 + 20;
                effect->spell_value = std::pow(1.025, power) * info.duration * 16.0;
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::haste:
        case spell::slow:
            effect = new Effect(spell_info->effect);
            effect->usage_type |= 1;
            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                effect->spell_or_damage = power / 10;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power / 15 + 1;
                effect->spell_value = std::pow(1.025, power) * info.duration * 16.0;
            }
            if (this->spell_id == spell::slow) {
                effect->spell_or_damage = -(int16_t)effect->spell_or_damage;
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::animate_dead:
            {
                Unit* dead_unit = nullptr;
                auto& unit_list = g_Server->srv_stru1->units_list->unit_list;
                for (int decay_target = 2; decay_target <= 4 && dead_unit == nullptr; ++decay_target) {
                    for (POSITION p = unit_list.GetHeadPosition(); p != nullptr; ) {
                        Unit* u = unit_list.GetNext(p);
                        if (u->decay != decay_target) {
                            continue;
                        }
                        uint16_t type_id = u->typeId;
                        // 0x52--0x62: undeads. 0x21--0x24 don't seem to exist in `data.xml` though?
                        if (type_id >= 0x52 && type_id <= 0x62 || type_id >= 0x21 && type_id <= 0x24) {
                            continue;
                        }
                        if (u->position->IsSameYX(&local_pos)) {
                            dead_unit = u;
                            break;
                        }
                    }
                }
                if (dead_unit == nullptr) {
                    return;
                }

                uint8_t corpse_decay = dead_unit->decay;
                dead_unit->hp = -10001;
                dead_unit->decay = 5;
                g_NetStru1_main.sub_519221(dead_unit, nullptr, 1, 0xFFB, 0, 0);
                Player* owner_player = caster->pOwner;

                Unit* unit = nullptr;
                if (dword_6B37C4 != nullptr) {
                    for (POSITION p = dword_6B37C4->unit_list.GetHeadPosition(); p != nullptr; ) {
                        Unit* u = dword_6B37C4->unit_list.GetNext(p);
                        if (u->decay < 5 || u->VMethod8() != 0) {
                            continue;
                        }
                        POSITION rm = g_Server->srv_stru1->units_list->unit_list.Find(u);
                        if (rm) {
                            g_Server->srv_stru1->units_list->unit_list.RemoveAt(rm);
                        }
                        unit = u;
                        break;
                    }
                }
                if (unit == nullptr) {
                    unit = new Unit();
                    dword_6B37C4->AddTail(unit);
                }
                unit->VMethod10();

                switch (corpse_decay) {
                case 2:
                    unit->FUN_0052931b(CString("F_Zombie.1"));
                    unit->body = dead_unit->body;
                    unit->reaction = dead_unit->reaction / 2 + 1;
                    unit->mind = dead_unit->mind;
                    unit->spirit = dead_unit->spirit;
                    unit->hp_max = dead_unit->hp_max;
                    unit->hp = unit->hp_max;
                    unit->hit_values.attack = dead_unit->hit_values.attack;
                    unit->protections.defense = dead_unit->protections.defense;
                    break;
                case 3:
                    unit->FUN_0052931b(CString("F_Skeleton.1"));
                    unit->body = dead_unit->body / 2 + 1;
                    unit->reaction = dead_unit->reaction / 2 + 1;
                    unit->mind = dead_unit->mind;
                    unit->spirit = dead_unit->spirit;
                    unit->hp_max = dead_unit->hp_max / 2;
                    unit->hp = unit->hp_max;
                    unit->hit_values.attack = dead_unit->hit_values.attack;
                    unit->protections.defense = dead_unit->protections.defense;
                    break;
                case 4:
                    unit->FUN_0052931b(CString("Ghost"));
                    unit->reaction = dead_unit->reaction / 2 + 1;
                    unit->mind = dead_unit->mind;
                    unit->spirit = dead_unit->spirit;
                    unit->hp_max = dead_unit->hp_max / 2;
                    unit->hp = unit->hp_max;
                    unit->hit_values.attack = dead_unit->hit_values.attack;
                    unit->protections.defense = dead_unit->protections.defense;
                    break;
                }

                if (!unit->sub_52BF3D(dead_unit->position->GetX(), dead_unit->position->GetY(), 0)) {
                    return;
                }
                unit->summoned = 1;
                dword_6CDB3C->AddTailAllocId(unit);
                unit->pOwner = owner_player;
                unit->summon_id = 0;
                owner_player->unit_list->AddTail(unit);

                Group* group = new Group();
                owner_player->group_list->groups.AddTail(group);
                group->AddUnit(unit);
                g_World->sub_5A9A6A(unit);
                g_World->sub_5ACDF4(group);

                MapStuff_Instance->scan_presence_grid.sub_596131();
                g_NetStru1_main.sub_519221(unit, nullptr, 0xA31FFFFF, 0xFFB, 0, 0);
                return;
            }

        case spell::teleport:
            if (is_arena_blocked()) {
                return;
            }
            MapStuff_Instance->sub_5954AC(caster, local_pos.GetX(), local_pos.GetY());
            g_NetStru1_main.sub_519221(caster, nullptr, 0x10, 0xFFB, 0, 0);
            return;

        case spell::heal:
            {
                if (g_World != nullptr && caster->pOwner != nullptr && target->pOwner != nullptr) {
                    if (g_World->diplomacy[caster->pOwner->player_id][target->pOwner->player_id] & 1) {
                        return;
                    }
                }
                if (target->hp <= -10 || target->hp_regen == 0) {
                    return;
                }
                int32_t heal = this->damage_min + Random1N(this->damage_spread);
                int32_t max_heal = target->hp_max - target->hp;
                if (heal > max_heal) {
                    heal = max_heal;
                }
                target->field_0x150 = 0;
                target->sub_52C4DF(heal);
                g_NetStru1_main.sub_519221(target, nullptr, target->field_0x150, 0xFFB, 0, 0);
                return;
            }

        case spell::summon:
            {
                Player* owner_player = caster->pOwner;

                Unit* unit = nullptr;
                if (dword_6B37C4 != nullptr) {
                    for (POSITION p = dword_6B37C4->unit_list.GetHeadPosition(); p != nullptr; ) {
                        Unit* u = dword_6B37C4->unit_list.GetNext(p);
                        if (u->decay < 5 || u->VMethod8() != 0) {
                            continue;
                        }
                        POSITION rm = g_Server->srv_stru1->units_list->unit_list.Find(u);
                        if (rm) {
                            g_Server->srv_stru1->units_list->unit_list.RemoveAt(rm);
                        }
                        unit = u;
                        break;
                    }
                }
                if (unit == nullptr) {
                    unit = new Unit();
                    dword_6B37C4->AddTail(unit);
                }
                unit->VMethod10();

                int32_t roll = Random1N(3);
                int32_t tier = caster->hit_values.skill_levels[5] / 25 + 1;
                if (tier < 1) {
                    tier = 1;
                }
                if (tier > 4) {
                    tier = 4;
                }

                CString name;
                switch (roll) {
                case 1: name = "Squirrel"; break;
                case 2: name = "Turtle"; break;
                case 3: name = "Foot_Animated"; break;
                }
                if (tier > 1) {
                    CString dot_tier = "." + CString('0' + tier, 1);
                    name += dot_tier;
                }
                unit->FUN_0052931b(name);
                unit->summoned = 1;

                if (!unit->sub_52BF3D(caster->position->GetX(), caster->position->GetY(), 6)) {
                    return;
                }
                dword_6CDB3C->AddTailAllocId(unit);
                unit->pOwner = owner_player;
                unit->summon_id = caster->building_id;
                owner_player->unit_list->AddTail(unit);

                Group* group = new Group();
                owner_player->group_list->groups.AddTail(group);
                group->AddUnit(unit);
                g_World->sub_5A9A6A(unit);
                g_World->sub_5AC8A2(group, 0);

                MapStuff_Instance->scan_presence_grid.sub_596131();
                g_NetStru1_main.sub_519221(unit, nullptr, 0xA31FFFFF, 0xFFB, 0, 0);
                return;
            }

        case spell::drain_life:
            {
                if (target->VMethod7() == 0) {
                    return;
                }
                int32_t drain = this->damage_min + Random1N(this->damage_spread);
                int32_t astral_protection = target->protections.magic_protections[5];
                drain = drain * (100 - astral_protection) / 100;
                int32_t till_corpse = target->hp + 10;
                if (drain >= till_corpse) {
                    drain = till_corpse;
                }
                if (drain <= 0) {
                    return;
                }
                caster->VMethod23(target, drain, this->spell_id);
                if (target->pOwner != nullptr && caster->pOwner != nullptr) {
                    g_World->sub_5AA581(caster, target, 0);
                }
                target->hp -= drain;
                g_NetStru1_main.sub_519221(target, nullptr, 1, 0xFFB, 0, 0);
                target->last_hit_by = caster;
                target->last_hit_spell_id = this->spell_id;
                caster->sub_52C4DF(drain);
                g_NetStru1_main.sub_519221(caster, nullptr, 1, 0xFFB, 0, 0);
                return;
            }

        case spell::shield:
            effect = new Effect(spell_info->effect);
            effect->usage_type |= 1;
            if (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) {
                effect->spell_or_damage = power * 30 / 100;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power / 10 + 3;
                effect->spell_value = std::pow(1.025, power) * info.duration * 16.0;
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        default:
            effect = new Effect();
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;
        }
    }

    if (effect == nullptr) {
        effect = direct_damage;
    }

    SpellEffect* container = nullptr;

    if (info.distribution == 1) {
        if (target == nullptr) {
            LogMessage("Spell, oops - can't cast point effect of x,y");
            return;
        }
        PointEffect* pe = new PointEffect(effect, target);
        Unit* caster_val = (caster->monster_info != nullptr) ? caster : nullptr;
        pe->caster = caster_val;
        effect->caster = caster_val;
        pe->itemDataID = this->spell_id;
        pe->field3_0x41 = this->is_defensive ? 0 : 1;
        pe->typeId = this->spell_id * 2 + 9;
        container = pe;
    } else {
        AreaEffect* ae = new AreaEffect(effect, &local_pos, info.radius);

        Unit* caster_val = (caster->monster_info != nullptr) ? caster : nullptr;
        ae->caster = caster_val;
        if (ae->effect != nullptr) {
            ae->effect->caster = caster_val;
        }

        ae->itemDataID = this->spell_id;
        ae->typeId = this->spell_id * 2 + 9;
        ae->field_0x50 = info.area_duration * 16;
        if (ae->field_0x50 != 0) {
            ae->field_0x50 += power * 16 / 10;
            ae->TokenID = 1;
        }
        if (info.distribution == 5) {
            ae->TokenID = 2;
            ae->field_0x50 = 0;
        }
        ae->field_0x4e = MapStuff_Instance->sub_59166C(caster, local_pos.GetYX()) / 32;
        MapStuff_Instance->scan_presence_grid.sub_596047(ae);
        container = ae;
    }

    if (info.delivery_system == 1) {
        container->typeId = this->spell_id * 2 + 9;
        g_Server->srv_stru1->effects_list->list.AddTail(container);
    } else if (info.delivery_system == 2) {
        SpellTransport* transport = new SpellTransport(container, caster->position, info.effect_speed);
        if (this->spell_id == spell::lightning || this->spell_id == spell::prismatic_spray) {
            transport->field_0x50 = 10;
        }
        g_Server->srv_stru1->effects_list->list.AddTail(transport);
    }
}
