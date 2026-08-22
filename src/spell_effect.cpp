#include "spell_effect.h"

#include "effect.h"
#include "game_app.h"
#include "map_stuff.h"
#include "player.h"
#include "server.h"
#include "unit.h"
#include "world.h"

// Distance between two TokenPos positions in sub-cell units (still in asm).
uint32_t __cdecl sub_5365AB(TokenPos* a, TokenPos* b);

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


// 6364A0
IMPLEMENT_SERIAL(PointEffect, SpellEffect, 1);

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

// 5374FC
PointEffect::PointEffect() : SpellEffect() {
	this->target = nullptr;
	this->effect = nullptr;
}

// 53752F
PointEffect::PointEffect(Effect* effect, Unit* target) : SpellEffect() {
	this->target = target;
	this->effect = effect;
	*this->position = *target->position;
}

// 537703
PointEffect::~PointEffect() {
	delete this->effect;
	this->effect = nullptr;
}

// 53E6D4
void PointEffect::Serialize(CArchive& ar) {
	SpellEffect::Serialize(ar);

	if (ar.IsStoring()) {
		ar.WriteObject(this->effect);
		ar.Write(&this->target, sizeof(this->target));
	} else {
		ar >> this->effect;
		uint32_t key;
		ar.Read(&key, sizeof(key));
		void* found;
		if (g_Server->field23_0xdc.Lookup(reinterpret_cast<void*>(key), found)) {
			this->target = static_cast<Unit*>(found);
		} else {
			this->target = nullptr;
		}
	}
}

// 57C0A0
void PointEffect::VMethod2() {
	this->sub_5375A4();
}

// 53E7F2
void PointEffect::VMethod5() {
	Token::VMethod5();
	if (this->effect != nullptr) {
		this->effect->VMethod5();
	}
}


// 5382FB
void AreaEffect::VMethod10(Unit* unit) {
	this->effect->VMethod11(unit);
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

// 537C8C
void AreaEffect::sub_537C8C() {
	if (g_GameDataRes.spells[this->itemDataID].Values()[0].distribution == 4) {
		this->sub_537F2C();
	} else {
		this->sub_53801A();
	}
}

// 6364B8
IMPLEMENT_SERIAL(AreaEffect, SpellEffect, 1);

// 537846
AreaEffect::AreaEffect() : SpellEffect() {
	this->effect = nullptr;
	this->TokenID = 0;
	this->field_0x4c = 0;
}

// 537880
AreaEffect::AreaEffect(Effect* effect, TokenPos* pos, uint8_t range) : SpellEffect(pos) {
	this->effect = effect;
	this->TokenID = 0;
	this->field_0x4c = 0;
	this->field_0x4d = range;
	this->field_0x4f = 0;
	this->duration = 0;
}

// 5378D8
AreaEffect::~AreaEffect() {
	delete this->effect;
	this->effect = nullptr;
}

// 53E81F
void AreaEffect::Serialize(CArchive& ar) {
	SpellEffect::Serialize(ar);
	if (ar.IsStoring()) {
		ar << this->field_0x4c;
		ar << this->field_0x4d;
		ar << this->field_0x4e;
		ar << this->field_0x4f;
		ar << this->duration;
		ar.WriteObject(this->effect);
	} else {
		ar >> this->field_0x4c;
		ar >> this->field_0x4d;
		ar >> this->field_0x4e;
		ar >> this->field_0x4f;
		ar >> this->duration;
		ar >> this->effect;
	}
}

// 537964
void AreaEffect::VMethod2() {
	if (this->TokenID & 2) {
		this->sub_537CD6();
		return;
	}
	if (!(this->TokenID & 1)) {
		this->sub_5384FF();
		return;
	}
	if (this->field_0x4c == 0) {
		this->sub_537C8C();
		return;
	}
	if (this->duration == 0) {
		this->sub_53822C();
		this->field2_0x40 = 1;
		return;
	}

	this->duration--;
	if ((this->duration & 0xF) == 0) {
		uint8_t x = this->position->GetX();
		uint8_t y = this->position->GetY();
		int32_t radius = this->field_0x4d;
		for (int32_t dx = -radius; dx <= radius; dx++) {
			for (int32_t dy = -radius; dy <= radius; dy++) {
				if (MapStuff_Instance->sub_595438(this, x + dx, y + dy)) {
					uint16_t yx = ((y + dy) << 8) | (x + dx);
					Unit* unit = MapStuff_Instance->sub_58CA1B(yx);
					this->sub_53831D(unit);
				}
			}
		}
	}
}

// 53EB1E
void AreaEffect::VMethod5() {
	Token::VMethod5();
	if (this->effect != nullptr) {
		this->effect->VMethod5();
	}
}

IMPLEMENT_SERIAL(SpellTransport, SpellEffect, 1); // 636FC8

// 5389E2
SpellTransport::SpellTransport() : SpellEffect() {
	this->spell_effect = nullptr;
	this->area_effect = nullptr;
}

// 538A15
SpellTransport::SpellTransport(SpellEffect* spell_effect, TokenPos* from_position, int16_t speed) : SpellEffect(from_position) {
	this->spell_effect = spell_effect;
	this->area_effect = nullptr;
	int32_t dist = (int32_t)sub_5365AB(from_position, this->spell_effect->position);
	this->duration = (int16_t)(dist / speed);
}

// 538A9E
SpellTransport::SpellTransport(AreaEffect* ae, TokenPos* from_position, int16_t speed) : SpellEffect(from_position) {
	this->spell_effect = nullptr;
	this->area_effect = ae;
	// WAT: dereferencing `this->spell_effect` is guaranteed to crash. This function has no callers in ASM.
	// I guess they meant `area_effect` instead?
	int32_t dist = (int32_t)sub_5365AB(from_position, this->spell_effect->position);
	this->duration = (int16_t)(dist / speed);
}

// 538B27
SpellTransport::~SpellTransport() {
	if (this->spell_effect != nullptr) {
		delete this->spell_effect;
		this->spell_effect = nullptr;
	}
	if (this->area_effect != nullptr) {
		delete this->area_effect;
		this->area_effect = nullptr;
	}
}

// 53E588
void SpellTransport::Serialize(CArchive& ar) {
	SpellEffect::Serialize(ar);
	if (ar.IsStoring()) {
		ar.WriteObject(this->spell_effect);
		ar.WriteObject(this->area_effect);
		ar << this->duration;
	} else {
		ar >> this->spell_effect;
		ar >> this->area_effect;
		ar >> this->duration;
	}
}

// 538BF4
void SpellTransport::VMethod2() {
	this->duration--;
	if (this->duration < 1) {
		SpellEffect* effect = (this->spell_effect != nullptr) ? this->spell_effect : this->area_effect;
		g_Server->srv_stru1->effects_list->list.AddTail(effect);
		this->spell_effect = nullptr;
		this->area_effect = nullptr;
		this->field2_0x40 = 1;
	}
}

// 53E68D
void SpellTransport::VMethod5() {
	Token::VMethod5();
	if (this->spell_effect != nullptr) {
		this->spell_effect->VMethod5();
	}
	if (this->area_effect != nullptr) {
		this->area_effect->VMethod5();
	}
}
