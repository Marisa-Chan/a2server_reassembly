#include "spell.h"
#ifdef A2SERVER_TANGAR_HAT
#include "patch/summon.h"
#include "patch/damage_filter.h"
#endif

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

IMPLEMENT_SERIAL(Spell, CObject, 1);

IMPLEMENT_SERIAL(SpellBook, CObject, 1);

//637010
const std::array<uint32_t, 32> SpellBtB = { 
    {0,         1,              2,          4,
     8,         0x1000,         0x2000,     0x4000,
     0x8000,    0,              0x800,      0x400,
     0x200,     0x100,          0,          0,
     0x800000,  0x400000,       0x200000,   0x100000,
     0x20,      0x40,           0x20000,    0x4000,
     0x10,      0x10000,        0x80,       0x80000,
     0,         0,              0,          0} };


void Spell::Serialize(CArchive& ar) { // 53cf8d
    if (ar.IsStoring()) {
        ar.Write(&this->spell_id, 1);
        ar.Write(&this->max_range, 1);
        ar.Write(&this->is_defensive, 1);
        ar.Write(&this->mana_cost, 2);
        ar.Write(this, 4);
    } else {
        ar.Read(&this->spell_id, 1);
        ar.Read(&this->max_range, 1);
        ar.Read(&this->is_defensive, 1);
        ar.Read(&this->mana_cost, 2);

        void* oldptr;
        ar.Read(&oldptr, 4);

        g_Server->saved_ptr_map[oldptr] = this;

        this->spell_info = &g_GameDataRes.spells[this->spell_id];
    }
}

Spell::Spell() { // 538fa5
    this->spell_info = nullptr;
    this->is_defensive = false;
    this->spell_id = 0;
}

Spell::Spell(uint8_t id) { // 538fdd
    this->spell_info = nullptr;
    this->is_defensive = false;
    this->spell_id = 0;
    this->max_range = 1;
    this->spell_id = id;
    this->Init();
}

Spell::Spell(const CString& name) { // 539055
    this->spell_info = nullptr;
    this->is_defensive = false;
    this->spell_id = 0;
    this->max_range = 1;
    this->spell_id = 0;

    for (int i = 0; i < g_GameDataRes.spells.GetSize(); i++) {
        if (g_GameDataRes.spells[i].name == name) {
            this->spell_id = i;
            break;
        }
    }

    if (this->spell_id == 0) {
        CString msg("Invalid spell ");
        LogMessage(msg + name);
    } else {
        this->Init();
    }
}

Spell::~Spell() //5392fe
{}

void Spell::Init() { //5391ab
#ifdef A2CLIENT_PATCH
    // p10st2.xck (522f14 -> 6e20f9): also reject spell_id > 0x1D (spell::max_spell_id)
    if (this->spell_id == 0 || this->spell_id > spell::max_spell_id) {
#else
    if (this->spell_id == 0) {
#endif
        CString msg;
        msg.Format("Invalid spell #%d - no such ID", this->spell_id);
        LogMessage(msg);
    } else {
        this->spell_info = &g_GameDataRes.spells[this->spell_id];
        this->mana_cost = this->spell_info->Values()[0].mana_cost;
        this->max_range = this->spell_info->Values()[0].max_range;
        this->is_defensive = this->spell_info->Values()[0].defensive == 1;
    }
}

// 53939E
int Spell::IsAreaSpell(uint8_t, uint8_t) {
    #ifdef A2SERVER_PATCH
    // bug_0008: null spell_info or empty values array -> not an area spell
    // (guards both derefs instead of crashing)
    if (this->spell_info == nullptr || this->spell_info->Values().GetData() == nullptr) {
        return 0;
    }
    #endif
    return this->spell_info->Values().GetData()[0].spell_target == 2;
}

// 5393C7
int Spell::IsUnitSpell() {
    return this->spell_info->Values().GetData()[0].spell_target == 1;
}

// 53940D
void Spell::SetPowerFromCaster(Unit* unit) {
    const SpellInfoData& info = this->spell_info->Values().GetData()[0];
    int32_t power = unit->hit_values.skill_levels[info.sphere] + unit->mind - 30;
    if (power < 0) {
        power = 0;
    }
    if (power > 255) {
        power = 255;
    }
    this->SetPower(power);
}

// 539C49
void Spell::ExecuteRaySpell(Unit* caster, Unit* target) {
    int32_t rays = 7;
    if (caster->some_item != nullptr) {
        for (POSITION it = caster->some_item->_effects.GetHeadPosition(); it != nullptr; ) {
            Effect* effect = caster->some_item->_effects.GetNext(it);
            if (effect->effect_id == modifier::castspell) {
                rays = (int16_t)effect->spell_value / 20 + 2;
                break;
            }
        }
    } else {
        rays = caster->GetSpellPower(this->spell_info->Values().GetData()[0].sphere) / 20 + 2;
    }
    if (rays > 7) {
        rays = 7;
    }

    CList<Unit*> units;
    g_World->CollectRayTargets(caster, target, &units, (uint8_t)rays, this->max_range);
    g_NetStru1_main.SendSpellCastMulti(caster, this, &units, 0);
    for (POSITION it = units.GetHeadPosition(); it != nullptr; ) {
        Unit* unit = units.GetNext(it);
        this->ExecuteAtArea(caster, unit, 0, 0);
    }
}

// 539F21
void Spell::ExecuteOnTarget(Unit* caster, Unit* target) {
    if (this->spell_id != spell::prismatic_spray) {
        this->ExecuteAtArea(caster, target, 0, 0);
    }
}

// 539541
void Spell::SetPower(uint32_t power) {
    const SpellInfoData& info = this->spell_info->Values().GetData()[0];
    uint8_t power_byte = power;

#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    // CRASH_fix_523295 (client 523295) / srv_range.fc (server 53955F): data-driven range formula -
    // data.bin max_range packs [base, divider]; range = 0 if base == 0,
    // else base + power/divider (default teleport divider: 15 in DM/TP on server, else 3; 30 otherwise)
    {
        uint8_t range_base = (uint8_t)(info.max_range & 0xFF);
        uint8_t range_divider = (uint8_t)((info.max_range >> 8) & 0xFF);
        if (range_base != 0) {
            if (range_divider == 0) {
                if (this->spell_id == spell::teleport) {
#ifndef A2CLIENT
                    range_divider = (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) ? 15 : 3;
#else
                    range_divider = 3;
#endif
                } else {
                    range_divider = 30;
                }
            }
            this->max_range = (uint8_t)(range_base + power_byte / range_divider);
        } else {
            this->max_range = 0;
        }
    }
#else
    this->max_range = info.max_range;
    if (this->spell_id == spell::teleport) {
        int32_t divisor = (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) ? 15 : 3;
        this->max_range += power_byte / divisor;
    } else if (this->max_range != 0) {
        this->max_range += power_byte / 30;
    }
#endif

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
#ifdef A2CLIENT_PATCH
            // Tangar fix_StoneCurseDurationCalculation (5233E6): the multiplier
            // comes from the data.bin duration parameter (SpellInfoData[0].duration)
            // instead of the hardcoded 15: power * duration * 32 / 100.
            this->spell_power = power_byte * info.duration * 32 / 100;
#elif defined(A2SERVER_PATCH)
            // fix_StoneCurseDurationCalculation (5396D9): 6-byte CALL replaces
            // `imul eax, 0x0F; shl eax, 4` with `imul info.duration`, followed by
            // `shl eax, 1; idiv 100`.
            this->spell_power = static_cast<uint16_t>(power_byte * info.duration * 2 / 100);
#else
            this->spell_power = power_byte * 480 / 100;
#endif
        } else {
            int32_t dur = static_cast<int32_t>(std::pow(1.025, power_byte) * info.duration * 16.0);
#ifdef A2SERVER_PATCH
            // fix_spell_duration_overflow_0053973E (53973E)
            if (dur > 0xFFFF) {
                dur = 0xFFFF;
            }
#endif
            this->spell_power = static_cast<uint16_t>(dur);
        }
    } else if (info.area_duration > 0) {
        this->spell_power = info.area_duration * 16 + power_byte * 16 / 10;
    } else {
        this->spell_power = 0;
    }
}

// Distance between two TokenPos positions in sub-cell units (still in asm).
uint32_t __cdecl CalcWalkDist(TokenPos* a, TokenPos* b);

// 539958
int32_t Spell::StartCast(Unit* caster, Unit* target, int8_t x, int8_t y) {
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
        if (target->GetTokenSize() > 1) {
            local_pos.SetCoords2(target->GetRightXx() - 1, target->GetBottomYy() - 1);
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

    #ifdef A2SERVER_PATCH
    // bug_0009: empty values array (null m_pData) - fail the cast instead of
    // crashing on the delivery_system read
    if (this->spell_info->Values().GetData() == nullptr) {
        return 0;
    }
    #endif
    const SpellInfoData& info = this->spell_info->Values().GetData()[0];
    int32_t delay = 0;
    if (info.delivery_system == 2) {
        delay = CalcWalkDist(caster->position, &local_pos) / info.effect_speed;
        if (this->spell_id == spell::lightning || this->spell_id == spell::prismatic_spray) {
            delay = 5;
		}
    } else if (caster->monster_info == nullptr) {
        return 1;
    }

    if (this->spell_id == spell::prismatic_spray) {
        this->SetPowerFromCaster(caster);
        this->ExecuteRaySpell(caster, target);
        return 1;
    }

    if (this->IsUnitSpell()) {
        g_NetStru1_main.SendSpellCastAtUnit(caster, this, target, delay);
    } else {
        g_NetStru1_main.SendSpellCastAtPos(caster, this, &local_pos, delay);
    }

    return 1;
}

// 53D368
SpellBook::~SpellBook() {
    #ifdef A2SERVER_PATCH
    // bug_000F: spells.m_pData can be null (corrupted array) - skip the loop
    if (this->spells.GetData() == nullptr) {
        return;
    }
    #endif
    for (int i = 0; i < this->spells.GetSize(); ++i) {
        delete this->spells[i];
        this->spells[i] = nullptr;
    }
}

// 53DDA4
void SpellBook::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << this->current_spell_index;
        int32_t count = this->spells.GetSize();
        ar << count;
        for (int32_t i = 1; i < count; i++) {
            ar.WriteObject(this->spells[i]);
        }
    } else {
        ar >> this->current_spell_index;
        int32_t count;
        ar >> count;
        this->spells.SetSize(count);
        for (int32_t i = 1; i < count; i++) {
            this->spells[i] = (Spell*)ar.ReadObject(RUNTIME_CLASS(Spell));
        }
    }
}

// 53DD3D
int32_t SpellBook::CalcSpellMask() {
    int32_t mask = 0;
    for (int32_t i = 1; i < this->spells.GetSize(); i++) {
        if (this->spells[i] != nullptr) {
            mask |= 1 << i;
        }
    }
    return mask;
}

// 53D7F0
void SpellBook::SetSpell(int32_t spell_id, Spell* spell) {
    if (spell_id < this->spells.GetSize()) {
        delete this->spells[spell_id];
    } else {
        this->spells.SetSize(spell_id + 1);
    }
    this->spells[spell_id] = spell;
}

void SpellBook::RefreshForHumanoid(Humanoid* humanoid) {
	//53dbc5
	for (int index = 1; index < this->spells.GetSize(); ++index) {
		Spell* spell = this->spells[index];
		if (spell != nullptr) {
			uint8_t spell_id = spell->spell_id;
			SpellInfo& expected_info = g_GameDataRes.spells[spell_id];
			if (spell->spell_info != &expected_info) {
				spell = new Spell(index);

				this->spells[index] = spell;

				LogMessage("Bad spell data, restoring spell");
			}

			spell->SetPowerFromCaster(humanoid);
		}
	}
}

// 539F5A
void Spell::ExecuteAtArea(Unit* caster, Unit* target, int8_t x, int8_t y) {
    if (this->spell_info == nullptr) {
        return;
    }

    #ifdef A2SERVER_PATCH
    // bug_000A/000B/000C/000D/000E family (7 sites: 539FFE, 53BF64, 53C736,
    // 53C73F, 53C94B, 53CA30, 53CA39): the values array can be empty (null
    // m_pData). The patched binary guards each m_pData re-read mid-function
    // (six recoveries jump to the common cleanup; the 53A091 one skips the
    // power calc with stale locals); our single top binding covers all paths,
    // so one guard + return is the equivalent.
    if (this->spell_info->Values().GetData() == nullptr) {
        return;
    }
    #endif
    const SpellInfoData& info = this->spell_info->Values().GetData()[0];
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
        Effect* cast_effect = caster->some_item->FindCastSpellEffect();
        power = (cast_effect != nullptr) ? cast_effect->spell_value : 0;
    }
    this->SetPower((uint8_t)power);
    double power_div_45 = power / 45.0 + 1.0;

    // Phase 2: blessed damage multiplier for deathmatch and softcore.
    if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
        if (caster->enchantments & (1u << spell::bless)) {
            this->damage_min *= 4;
            this->damage_spread *= 4;
            power_div_45 *= 4.0;
        }
    }

    if ((caster->unit_attrs & 2) != 0 && caster->some_item == nullptr) {
        caster->AwardCastExperience(target, this->spell_id);
    }

    // Phase 3: Setup local position from target or coordinates
    TokenPos local_pos;
    if (target == nullptr) {
        local_pos = TokenPos(x, y, MapStuff_Instance);
    } else {
        local_pos = *target->position;
        if (target->GetTokenSize() > 1) {
            local_pos.SetCoords2(target->GetRightXx() - 1, target->GetBottomYy() - 1);
        }
        if (target->IsUnit() != 0 && (target->unit_attrs & 8)) {
            return;
        }
    }

    // Helper: check if caster is the opposing arena team player (blocks certain spells in PvP).
    auto is_arena_blocked = [&]() -> bool {
        if (caster->pOwner != nullptr && g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
            Player* owner = caster->pOwner;
            int32_t team_idx = 1 - owner->team;
            if (g_Server->ctf_carrying[team_idx] == owner->player_id) {
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
            effect = new Effect(this->spell_info->effect);
            effect->usage_type |= 1;
            effect->spell_or_damage = power / 2;
            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                effect->spell_value = power * 16;
            } else {
                int32_t dur = static_cast<int32_t>(std::pow(1.025, power) * info.duration * 16.0);
#ifdef A2SERVER_PATCH
                // fix_spell_duration_overflow (53BFA1)
                if (dur > 0xFFFF) {
                    dur = 0xFFFF;
                }
#endif
                effect->spell_value = static_cast<uint16_t>(dur);
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::poison_cloud:
            effect = new Effect(this->spell_info->effect);
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
            effect = new Effect(this->spell_info->effect);
            effect->itemDataID = this->spell_id;
            effect->spell_or_damage = -1 - power / 30;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::light:
            effect = new Effect(this->spell_info->effect);
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
            effect = new Effect(this->spell_info->effect);
            effect->usage_type |= 1;
            {
                int32_t factor = (caster->some_item == nullptr) ? 2 : 1;
#ifdef A2SERVER_PATCH
                // fix_StoneCurseDurationCalculation (53C32E): 6-byte CALL replaces
                // `imul eax, 0x0F; shl eax, 4` with `imul info.duration`.
                effect->spell_value = static_cast<uint16_t>(power * info.duration * factor / 100);
#else
                effect->spell_value = power * 240 * factor / 100;
#endif
            }
            if (target->IsUnit() != 0) {
                int32_t earth_protection = target->protections.magic_protections[4];
                int32_t max_duration = effect->spell_value * (100 - earth_protection) / 100;
#ifdef A2SERVER_TANGAR_HAT
                // srv_pvm.xck + verify_damage hook (53C390 -> 5040D2)
                max_duration = TangarOnDamage(caster, target, static_cast<int16_t>(max_duration));
#endif
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
            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                effect->spell_or_damage = 0;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power * 4 / 5 + 20;
                int32_t dur = static_cast<int32_t>(std::pow(1.025, power) * info.duration * 16.0);
#ifdef A2SERVER_PATCH
                // fix_spell_duration_overflow (53C0E1)
                if (dur > 0xFFFF) {
                    dur = 0xFFFF;
                }
#endif
                effect->spell_value = static_cast<uint16_t>(dur);
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::haste:
        case spell::slow:
            effect = new Effect(this->spell_info->effect);
            effect->usage_type |= 1;
            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                effect->spell_or_damage = power / 10;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power / 15 + 1;
                int32_t dur = static_cast<int32_t>(std::pow(1.025, power) * info.duration * 16.0);
#ifdef A2SERVER_PATCH
                // fix_spell_duration_overflow (53C21E)
                if (dur > 0xFFFF) {
                    dur = 0xFFFF;
                }
#endif
                effect->spell_value = static_cast<uint16_t>(dur);
            }
            if (this->spell_id == spell::slow) {
                effect->spell_or_damage = -effect->spell_or_damage;
            }
            effect->itemDataID = this->spell_id;
            effect->typeId = this->spell_id * 2 + 8;
            break;

        case spell::animate_dead: {
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
                g_NetStru1_main.SendUnitUpdate(dead_unit, nullptr, UNIT_UPD_HP, EQUIP_SLOTS_ALL, 0, 0);
                Player* owner_player = caster->pOwner;

                Unit* unit = nullptr;
                if (g_UnitPool != nullptr) {
                    for (POSITION p = g_UnitPool->unit_list.GetHeadPosition(); p != nullptr; ) {
                        Unit* u = g_UnitPool->unit_list.GetNext(p);
                        if (u->decay < 5 || u->IsHumanoid() != 0) {
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
                    g_UnitPool->AddTail(unit);
                }
                unit->ReleaseObjects();

                switch (corpse_decay) {
                case 2:
                    unit->InitDefaults(CString("F_Zombie.1"));
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
                    unit->InitDefaults(CString("F_Skeleton.1"));
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
                    unit->InitDefaults(CString("Ghost"));
                    unit->reaction = dead_unit->reaction / 2 + 1;
                    unit->mind = dead_unit->mind;
                    unit->spirit = dead_unit->spirit;
                    unit->hp_max = dead_unit->hp_max / 2;
                    unit->hp = unit->hp_max;
                    unit->hit_values.attack = dead_unit->hit_values.attack;
                    unit->protections.defense = dead_unit->protections.defense;
                    break;
                }

                if (!unit->PlaceOnMap(dead_unit->position->GetX(), dead_unit->position->GetY(), 0)) {
                    return;
                }
                unit->summoned = 1;
                g_ServerUnits->AddTailAllocId(unit);
                unit->pOwner = owner_player;
                unit->summon_id = 0;
                owner_player->unit_list->AddTail(unit);

                Group* group = new Group();
                owner_player->group_list->groups.AddTail(group);
                group->AddUnit(unit);
                g_World->ResetUnitToHold(unit);
                g_World->ProcessAnimateDeadGroup(group);

                MapStuff_Instance->scan_presence_grid.RebuildPresenceGrid();
                g_NetStru1_main.SendUnitUpdate(unit, nullptr, UNIT_UPD_ALL_STATS, EQUIP_SLOTS_ALL, 0, 0);
                return;
            }

        case spell::teleport:
            if (is_arena_blocked()) {
                return;
            }
            MapStuff_Instance->TeleportUnit(caster, local_pos.GetX(), local_pos.GetY());
            g_NetStru1_main.SendUnitUpdate(caster, nullptr, UNIT_UPD_DIR_POS, EQUIP_SLOTS_ALL, 0, 0);
            return;

        case spell::heal: {
                if (g_World != nullptr && caster->pOwner != nullptr && target->pOwner != nullptr) {
                    if (g_World->diplomacy.diplomacy[caster->pOwner->player_id][target->pOwner->player_id] & DIPLO_WAR) {
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
                target->item_op_dirty_flags = 0;
                target->AddHp(heal);
                g_NetStru1_main.SendUnitUpdate(target, nullptr, target->item_op_dirty_flags, EQUIP_SLOTS_ALL, 0, 0);
                return;
            }

        case spell::summon: {
                Player* owner_player = caster->pOwner;

                Unit* unit = nullptr;
                if (g_UnitPool != nullptr) {
                    for (POSITION p = g_UnitPool->unit_list.GetHeadPosition(); p != nullptr; ) {
                        Unit* u = g_UnitPool->unit_list.GetNext(p);
                        if (u->decay < 5 || u->IsHumanoid() != 0) {
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
                    g_UnitPool->AddTail(unit);
                }
                unit->ReleaseObjects();

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
                    CString dot_tier = "." + CString((char)('0' + tier), 1);
                    name += dot_tier;
                }
                unit->InitDefaults(name);
#ifdef A2SERVER_TANGAR_HAT
                // summon hook (53AC32): full stat override per ServerID/tier
                TangarFixSummonedUnit(unit, tier);
#else
                unit->summoned = 1;
#endif

                if (!unit->PlaceOnMap(caster->position->GetX(), caster->position->GetY(), 6)) {
                    return;
                }
                g_ServerUnits->AddTailAllocId(unit);
                unit->pOwner = owner_player;
                unit->summon_id = caster->building_id;
                owner_player->unit_list->AddTail(unit);

                Group* group = new Group();
                owner_player->group_list->groups.AddTail(group);
                group->AddUnit(unit);
                g_World->ResetUnitToHold(unit);
                g_World->GroupGuardCommand(group, 0);

                MapStuff_Instance->scan_presence_grid.RebuildPresenceGrid();
                g_NetStru1_main.SendUnitUpdate(unit, nullptr, UNIT_UPD_ALL_STATS, EQUIP_SLOTS_ALL, 0, 0);
                return;
            }

        case spell::drain_life: {
                if (target->IsUnit() == 0) {
                    return;
                }
                int32_t drain = this->damage_min + Random1N(this->damage_spread);
                int32_t astral_protection = target->protections.magic_protections[5];
                drain = drain * (100 - astral_protection) / 100;
                int32_t till_corpse = target->hp + 10;
                if (drain >= till_corpse) {
                    drain = till_corpse;
                }
#ifdef A2SERVER_TANGAR_HAT
                // srv_pvm.xck + verify_damage hook (53A740 -> 5040D2)
                drain = TangarOnDamage(caster, target, static_cast<int16_t>(drain));
#endif
                if (drain <= 0) {
                    return;
                }
                caster->AwardDamageExperience(target, drain, this->spell_id);
                if (target->pOwner != nullptr && caster->pOwner != nullptr) {
                    g_World->RecordUnitHit(caster, target, 0);
                }
                target->hp -= drain;
                g_NetStru1_main.SendUnitUpdate(target, nullptr, UNIT_UPD_HP, EQUIP_SLOTS_ALL, 0, 0);
                #ifdef A2SERVER_PATCH
                // prevent_suicide_drain_life hook (53A7CC): self-damage must
                // not overwrite the kill attribution
                if (target != caster) {
                #endif
                target->last_hit_by = caster;
                target->last_hit_spell_id = this->spell_id;
                #ifdef A2SERVER_PATCH
                }
                #endif
                caster->AddHp(drain);
                g_NetStru1_main.SendUnitUpdate(caster, nullptr, UNIT_UPD_HP, EQUIP_SLOTS_ALL, 0, 0);
                return;
            }

        case spell::shield:
            effect = new Effect(this->spell_info->effect);
            effect->usage_type |= 1;
            if (g_ServerConfig.gameType == GAME_TYPE_DEATHMATCH || g_ServerConfig.gameType == GAME_TYPE_TEAMPLAY) {
                effect->spell_or_damage = power * 30 / 100;
                effect->spell_value = power * 16;
            } else {
                effect->spell_or_damage = power / 10 + 3;
                int32_t dur = static_cast<int32_t>(std::pow(1.025, power) * info.duration * 16.0);
#ifdef A2SERVER_PATCH
                // fix_spell_duration_overflow (53C52A)
                if (dur > 0xFFFF) {
                    dur = 0xFFFF;
                }
#endif
                effect->spell_value = static_cast<uint16_t>(dur);
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
        pe->offensive = this->is_defensive ? 0 : 1;
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
        ae->duration = info.area_duration * 16;
        if (ae->duration != 0) {
            ae->duration += power * 16 / 10;
            ae->TokenID = 1;
        }
        if (info.distribution == 5) {
            ae->TokenID = 2;
            ae->duration = 0;
        }
        ae->wave_pattern = MapStuff_Instance->GetFacingFor(caster, local_pos.GetYX()) / 32;
        MapStuff_Instance->scan_presence_grid.RegisterAreaEffect(ae);
        container = ae;
    }

    if (info.delivery_system == 1) {
        container->typeId = this->spell_id * 2 + 9;
        g_Server->srv_stru1->effects_list->list.AddTail(container);
    } else if (info.delivery_system == 2) {
        SpellTransport* transport = new SpellTransport(container, caster->position, info.effect_speed);
        if (this->spell_id == spell::lightning || this->spell_id == spell::prismatic_spray) {
            transport->duration = 10;
        }
        g_Server->srv_stru1->effects_list->list.AddTail(transport);
    }
}

Spell* SpellBook::GetSpell(int spell_id) { // 53db79 // Look up spell in book by id
    if ((uint32_t)spell_id >= this->spells.GetSize()) {
        return nullptr;
    }
    
    if (this->spells[spell_id] == nullptr) {
        return nullptr;
    }

    this->current_spell_index = spell_id;
    return this->spells[spell_id];
}
