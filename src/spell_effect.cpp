#include "spell_effect.h"

#include "effect.h"
#include "game_app.h"
#include "player.h"
#include "server.h"
#include "unit.h"
#include "world.h"

// 636488
IMPLEMENT_SERIAL(SpellEffect, Token, 1);

// 537371
SpellEffect::SpellEffect() : Token() {
	this->typeId = 0;
	this->caster = nullptr;
	this->field2_0x40 = 0;
	this->field3_0x41 = 1;
	this->field4_0x42 = 0;
	this->field6_0x44 = 0;
	this->field6_0x46 = 0;
}

// 5373CC
SpellEffect::SpellEffect(const TokenPos* pos) : Token(pos) {
	this->typeId = 0;
	this->caster = nullptr;
	this->field2_0x40 = 0;
	this->field3_0x41 = 1;
	this->field4_0x42 = 0;
	this->field6_0x44 = 0;	
	this->field6_0x46 = 0;
}

// 57c080
SpellEffect::~SpellEffect() {}

// 53E44A
void SpellEffect::Serialize(CArchive& ar) {
	Token::Serialize(ar);
	if (ar.IsStoring()) {
		ar << this->field2_0x40;
		ar << this->field3_0x41;
	} else {
		ar >> this->field2_0x40;
		ar >> this->field3_0x41;
	}
}

// 53742d
void SpellEffect::VMethod2() {}

// 537438
void SpellEffect::VMethod10(Unit* /*unit*/) {}

// 5382FB
void AreaEffect::VMethod10(Unit* unit) {
	this->effect->VMethod11(unit);
}

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

// 53831D
void AreaEffect::sub_53831D(Unit* unit) {
	if (unit == nullptr || this->itemDataID == 0x11) {
		return;
	}

	if (this->itemDataID == 2) {
		DirectDamage local_dd(static_cast<DirectDamage*>(this->effect));
		int32_t size = (uint8_t)unit->VMethod3();
		int32_t size_sq = size * size;
		local_dd.unit_to_hit.some_damage2_min /= size_sq;
		local_dd.unit_to_hit.some_damage2_spread /= size_sq;
		local_dd.VMethod11(unit);
	} else {
		this->effect->VMethod11(unit);
	}

	if (this->itemDataID == 0xF) {
		return;
	}

	if ((unit->VMethod4() & 0xFF) == 0 || this->caster == nullptr) {
		return;
	}

	if (this->caster->monster_info == nullptr) {
		unit->last_hit_by = nullptr;
	} else if (this->caster->pOwner == nullptr) {
		unit->last_hit_by = nullptr;
	} else if (this->itemDataID != 0xE) {
		unit->last_hit_by = this->caster;
		unit->last_hit_spell_id = this->effect->itemDataID;
	}

	if (this->caster->pOwner != nullptr && unit->pOwner != nullptr) {
		if (g_Server->field4_0x74 == 0 || !this->caster->pOwner->is_ai || unit->pOwner->is_ai) {
			g_World->sub_5AA581(this->caster, unit, 1);
		}
	}
}
