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

extern "C" {
	extern int32_t unk_6364D0[];  // Wave table for area-effect spell 7: count, 20 x-offsets, 20 y-offsets.
	extern int32_t dword_6366C4;  // Number of wave patterns for area-effect spell 9 (= 6).
	extern int32_t dword_6366C8[]; // Wave patterns A for area-effect spell 9 (6 records of 41 dwords).
	extern int32_t dword_636AA0[]; // Wave patterns B for area-effect spell 9.
}

// Wave propagation descriptor built from the static wave tables.
struct WavePattern {
	const int32_t* x_offsets;
	const int32_t* y_offsets;
	int32_t count;
	int32_t x_mult;
	int32_t y_mult;

	void sub_537AE8(const int32_t* table);
	void sub_537B2B(const int32_t* table_a, const int32_t* table_b, uint8_t direction);
};

// 537AE8
void WavePattern::sub_537AE8(const int32_t* table) {
	this->x_offsets = table + 1;
	this->y_offsets = table + 0x15;
	this->count = table[0];
	this->x_mult = 1;
	this->y_mult = 1;
}

// 537B2B
void WavePattern::sub_537B2B(const int32_t* table_a, const int32_t* table_b, uint8_t direction) {
	this->x_offsets = table_b + 1;
	this->y_offsets = table_b + 0x15;
	this->count = table_b[0];
	this->x_mult = 1;
	this->y_mult = 1;
	switch (direction) {
	case 0:
		this->x_offsets = table_a + 1;
		this->y_offsets = table_a + 0x15;
		this->count = table_a[0];
		this->y_mult = -1;
		break;
	case 1:
		this->y_mult = -1;
		break;
	case 2:
		this->x_offsets = table_a + 0x15;
		this->y_offsets = table_a + 1;
		this->count = table_a[0];
		break;
	case 4:
		this->x_offsets = table_a + 1;
		this->y_offsets = table_a + 0x15;
		this->count = table_a[0];
		this->x_mult = -1;
		break;
	case 5:
		this->x_mult = -1;
		break;
	case 6:
		this->x_offsets = table_a + 0x15;
		this->y_offsets = table_a + 1;
		this->count = table_a[0];
		this->x_mult = -1;
		break;
	case 7:
		this->x_mult = -1;
		this->y_mult = -1;
		break;
	}
}

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

// 538897
uint32_t AreaEffect::sub_538897() const {
	switch (this->itemDataID) {
	case 3:
		return 0;
	case 6:
		return 2;
	case 0xE:
		return 5;
	case 0xF:
		return 4;
	case 0x11:
		return 3;
	default:
		// The original builds a temp CString "MapLayer() call - Invalid Area Effect" and immediately destroys it (dead code).
		return 0;
	}
}

// 53822C
void AreaEffect::sub_53822C() {
	uint8_t x = this->position->GetX();
	uint8_t y = this->position->GetY();
	int32_t radius = this->field_0x4d;
	for (int32_t dx = x - radius; dx <= x + radius; dx++) {
		for (int32_t dy = y - radius; dy <= y + radius; dy++) {
			MapStuff_Instance->sub_59501E(this, dx, dy);
		}
	}
	g_NetStru1_main.sub_51BE8F(this, 0);
	this->field_0x4c = 0;
}

// 5384FF
void AreaEffect::sub_5384FF() {
	g_NetStru1_main.sub_51BE8F(this, 1);
	bool spread_damage = this->effect->IsKindOf(RUNTIME_CLASS(DirectDamage)) &&
		static_cast<DirectDamage*>(this->effect)->unit_to_hit.some_damage2_spread > 0;
	uint8_t x = this->position->GetX();
	uint8_t y = this->position->GetY();
	int32_t radius = this->field_0x4d;
	for (int32_t dx = x - radius; dx <= x + radius; dx++) {
		for (int32_t dy = y - radius; dy <= y + radius; dy++) {
			uint16_t yx = (dy << 8) | dx;
			this->sub_53831D(MapStuff_Instance->sub_58CB5A(yx));
			this->sub_53831D(MapStuff_Instance->sub_58CBB9(yx));
			this->sub_53831D(MapStuff_Instance->sub_5946BF(dx, dy));
			if (spread_damage) {
				this->sub_53868D(dx, dy);
				MapStuff_Instance->sub_594768(dx, dy);
			}
		}
	}
	this->field2_0x40 = 1;
}

// 537CD6
void AreaEffect::sub_537CD6() {
	uint16_t old_duration = this->duration;
	this->duration--;
	if (old_duration != 0) {
		return;
	}
	this->duration = 2;
	uint8_t x = this->position->GetX();
	uint8_t y = this->position->GetY();
	WavePattern wave = {};
	int32_t wave_count = 0;
	uint8_t packet_type = 0x10;
	if (this->itemDataID == 7) {
		unk_6364D0[1] = Random0N(5) - 2;
		unk_6364D0[0x15] = Random0N(5) - 2;
		wave.sub_537AE8(unk_6364D0);
		wave_count = 0x20;
	} else if (this->itemDataID == 9) {
		wave.sub_537B2B(dword_6366C8 + this->field_0x4f * 41, dword_636AA0 + this->field_0x4f * 41, this->field_0x4e);
		wave_count = dword_6366C4;
		packet_type = 0x12;
	}
	for (int32_t i = 0; i < wave.count; i++) {
		uint8_t nx = x + wave.x_mult * wave.x_offsets[i];
		uint8_t ny = y + wave.y_mult * wave.y_offsets[i];
		this->effect->typeId = this->itemDataID * 2 + 9;
		if (this->effect->position->FUN_0058a7e8(nx, ny)) {
			g_NetStru1_main.sub_51BE0E(this->effect, packet_type);
			uint16_t yx = (ny << 8) | nx;
			this->sub_53831D(MapStuff_Instance->sub_58CB5A(yx));
			this->sub_53831D(MapStuff_Instance->sub_58CBB9(yx));
			this->sub_53831D(MapStuff_Instance->sub_5946BF(nx, ny));
		}
	}
	this->field_0x4f++;
	if (this->field_0x4f >= wave_count) {
		this->field2_0x40 = 1;
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
