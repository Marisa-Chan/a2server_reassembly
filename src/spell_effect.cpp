#include "spell_effect.h"

#include "effect.h"
#include "unit.h"
#include "world.h"

// 5375A4
void PointEffect::sub_5375A4() {
	// Clean up effect caster if caster has no owner.
	if (this->effect != nullptr && this->effect->caster != nullptr && this->effect->caster->pOwner == nullptr) {
		this->effect->caster = nullptr;
	}

	if (!this->target->VMethod7()) {
		if (this->target->VMethod9()) {
			this->effect->VMethod11(this->target);
		}
	} else {
		Unit* attacker = this->target;
		if (attacker != nullptr && attacker->pOwner != nullptr) {
			this->effect->VMethod11(attacker);
		}

		if (this->caster != nullptr && this->field3_0x41 != 0) {
			if (this->caster->monster_info == nullptr) {
				attacker->last_hit_by = nullptr;
			} else if (this->caster->pOwner != nullptr) {
				attacker->last_hit_by = this->caster;
				attacker->last_hit_spell_id = this->effect->itemDataID;
			}

			if (attacker != nullptr && this->caster != nullptr &&
				this->caster->pOwner != nullptr && attacker->pOwner != nullptr) {
				g_World->sub_5AA581(this->caster, attacker, 0);
			}
		}
	}

	this->field2_0x40 = 1;
}
