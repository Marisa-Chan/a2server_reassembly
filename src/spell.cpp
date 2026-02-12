#include "spell.h"
#include "unit.h"
#include "game_app.h"

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
