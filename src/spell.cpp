#include "spell.h"

#include "unit.h"

extern "C" {
    extern GameDataRes GameDataRes_6D0668;
	void __stdcall sub_43AA23(CString message);
}

void SpellBook::RefreshForHumanoid(Humanoid* humanoid)
{
	for (int index = 1; index < spells.GetSize(); ++index) {
		Spell* spell = spells[index];
		if (spell != nullptr) {
			uint8_t spell_id = spell->spell_id;
			SpellInfo* expected_info = &GameDataRes_6D0668.spells[spell_id];
			if (spell->spell_info != expected_info) {
				Spell* new_spell = new Spell();
				if (new_spell != nullptr) {
					new_spell->spell_info = &GameDataRes_6D0668.spells[index];
					new_spell->spell_id = static_cast<uint8_t>(index);
					new_spell->max_range = 0;
					new_spell->is_defensive = 0;
					new_spell->gap_0xb = 0;
					new_spell->mana_cost = 0;
					new_spell->damage_min = 0;
					new_spell->damage_spread = 0;
					new_spell->spell_power = 0;
					new_spell->gap_0x12[0] = 0;
					new_spell->gap_0x12[1] = 0;
				}

				spells[index] = new_spell;

				sub_43AA23("Bad spell data, restoring spell");
				spell = new_spell;
			}

			spell->sub_53940D(humanoid);
		}
	}
}
