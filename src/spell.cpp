#include "spell.h"

#include <cmath>

#include "game_app.h"
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
