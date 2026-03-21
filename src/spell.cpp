#include "spell.h"

#include <cmath>

#include "effect.h"
#include "game_app.h"
#include "net.h"
#include "unit.h"

// 53939E
bool Spell::sub_53939E(uint8_t, uint8_t)
{
    return spell_info->values.GetData()[0].spell_target == 2;
}

// 539541
void Spell::sub_539541(uint32_t power)
{
    const SpellInfoData& info = spell_info->values.GetData()[0];
    uint8_t power_byte = (uint8_t)power;

    this->max_range = (uint8_t)info.max_range;
    if (this->spell_id == 0x17) {
        int32_t divisor = (g_ServerConfig.gameType == 1 || g_ServerConfig.gameType == 2) ? 15 : 3;
        this->max_range += (uint8_t)(power_byte / divisor);
    } else if (this->max_range != 0) {
        this->max_range += (uint8_t)(power_byte / 30);
    }

    double multiplier = power_byte / 30.0 + 1.0;

    if (info.damage_min > 0) {
        this->damage_min = (uint8_t)(info.damage_min * multiplier);
    } else {
        this->damage_min = 0;
    }

    if (info.damage_max > 0) {
        this->damage_spread = (uint8_t)(info.damage_max * multiplier - this->damage_min);
    } else {
        this->damage_spread = 0;
    }

    if (info.duration > 0) {
        if (this->spell_id == 0x0C) {
            this->spell_power = (uint16_t)(power_byte << 4);
        } else if (this->spell_id == 0x12) {
            this->spell_power = (uint16_t)(power_byte * 480 / 100);
        } else {
            this->spell_power = (uint16_t)(std::pow(1.025, power_byte) * info.duration * 16.0);
        }
    } else if (info.area_duration > 0) {
        this->spell_power = (uint16_t)(info.area_duration * 16 + power_byte * 16 / 10);
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

    if (target != caster && (caster->enchantments & (1u << 0xC))) {
        for (POSITION p = caster->_effects.GetHeadPosition(); p != nullptr; ) {
            Effect* eff = caster->_effects.GetNext(p);
            if (eff->itemDataID == 0x0C) {
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
        if (spell_id == 0x0A || spell_id == 0x0B) {
            delay = 5;
		}
    } else if (caster->monster_info == nullptr) {
        return 1;
    }

    if (spell_id == 0x0B) {
        this->sub_53940D(caster);
        this->sub_539C49(caster, target);
        return 1;
    }

    if (this->sub_5393C7()) {
        g_NetStru1_main.sub_51BAB0(caster, this, target, (int16_t)delay);
    } else {
        g_NetStru1_main.sub_51BB94(caster, this, &local_pos, (int16_t)delay);
    }

    return 1;
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
