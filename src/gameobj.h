#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include <inttypes.h>

#include <afx.h>
#include "alm.h"
#include "util.h"

#include <array>

class BigStruct2;
class CGameBitmap;
class CSprite256;
class SfxBank;
class Unit;

extern CStringArray g_CUnitMaterialSpritePaths; //660e70


class TokenEntry : public CObject {
public:
	enum {
		TYPE_SPELL = 0x0E,
		CAST_SPELL_ATTRIBUTE_ID = 0x29,
	};
public:

	int32_t IsEmpty();
	int32_t IsDepleted(); //4a78c0 (client: 4ab860) in asm
	int32_t IsQuestItem(); //43a5e5 (client: 43c8e3)
	int32_t IsQuestTreasure(); //43a6d5 (client: 43c9d3)
	int32_t IsPotion(); // client: 43ca26
	int32_t IsScroll(); // client: 43ca7a
	int32_t IsTwoHandedWeapon(); //4396fb (client: 43ba17)
	int32_t AddCount(int32_t num); //4a7850 (client: 4ab8a0)
	int32_t RemoveCount(int32_t num); //4a7880 (client: 4ab960)
	int32_t IsStackableWith(TokenEntry* other); //4a7900 (client: 4ab8d0)
	int32_t GetFirstModAttrId(); //43988e (client: 43bbaa) in asm

	virtual ~TokenEntry(); //4394bf (client: 43b7dd)

	TokenEntry(); //438f90 (client: 43b2b0)
	TokenEntry(int id1); //439009 (client: 43b329)
	TokenEntry(const TokenEntry* obj);
	TokenEntry(uint8_t** data, int id1);

	void operator=(const TokenEntry& obj);

	CString GetSpriteName(); //4394f3 (client: 43b811)
	int HasKnownName(); //4396d6
	const char* GetHintText(); //439973 (client: 43bc8f)

	int GetType() const { return (this->item_id >> 8) & 0xf; } //41f0b0
	int GetId() const { return this->item_id & 0x1f; } //43a800
	int GetMaterial() const { return (this->item_id >> 0xc); } //43a7c0
	int GetShape() const { return (this->item_id >> 5) & 7; } //43a7e0

	uint32_t GetAttribute(uint8_t aid) const { //4397de (client: 43bafa)
		if (!this->mods || this->mods_count == 0 || this->mods_size == 0) {
			return 0;
		}

		int pos = 0;
		uint32_t result = 0;

		const uint8_t* cmods = (const uint8_t*)this->mods;

		for (int i = 0; i < this->mods_count; i++) {
			uint8_t k = cmods[pos];
			if (k == aid) {
				if (k == 1) {
					result = GetU32(cmods + pos + 1);
				} else {
					result = cmods[pos + 1];
				}
			}
			
			pos += k == 1 ? 5 : 2;
		}
		return result;
	}

	uint32_t GetCastSpellId() const { return this->GetAttribute(CAST_SPELL_ATTRIBUTE_ID); } //41f0f0 (client: 420e10)
public:
	uint16_t field_0x4 = 0;
	uint16_t item_id = 0;
	uint8_t flg = 0;
	uint8_t mods_count = 0;
	uint8_t mods_size = 0;
	uint8_t field_0xb = 0;
	void* mods = nullptr;
	uint32_t count = 1;
	uint32_t field_0x14 = 0;
	uint32_t slot_code = 0;
	int32_t field_0x1c = -1;
	int32_t field_0x20 = -1;
};
ASSERT_SIZE(TokenEntry, 0x24);

struct GO_11c {
	GO_11c() = default;

	GO_11c(int16_t x, int16_t y, int16_t z, uint8_t vfx_type, uint8_t phase) { // 4cd3c0 (client: 4d1d10)
		this->x = x;
		this->y = y;
		this->z = z;
		this->vfx_type = vfx_type;
		this->phase = phase;
	}

	int16_t x;
	int16_t y;
	int16_t z;
	uint8_t vfx_type;
	uint8_t phase;
};
ASSERT_SIZE(GO_11c, 8);


class CGameObject : public CObject {
	DECLARE_DYNAMIC(CGameObject);
public:

	virtual ~CGameObject();

	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;

	virtual void SetSelected(int32_t selected);
	virtual int32_t VMethod2();
	virtual int32_t VMethod3();
	virtual int32_t GetTileWidth();
	virtual int32_t GetTileHeight();
	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light);
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light);
	virtual void DrawHealthBar(int32_t tile_x, int32_t tile_y, int32_t hgt);
	virtual void DrawMinimapRect(int32_t x, int32_t y, int32_t zoom);
	virtual void UpdateTileLayer();
	virtual int32_t Update();
	virtual void UpdateLight();
	virtual void UpdateOccupancy();
	virtual void MarkOccupancy();
	virtual int32_t UpdateBlockedState();
	virtual void UpdateTransientVisualElements();

	CGameObject();
	CGameObject(const CGameObject *obj);

	int32_t IsSelected() { return this->m_bSelected; } //41f110
	uint32_t IsInControlGroup(uint32_t t) { return (1 << t) & this->controlGroupMask; } //41f1c0
	void UpdateMapPosition();
	void SetVals(uint16_t uni_id, int type_id, int32_t xpos, int32_t ypos, int32_t unk1, int32_t unk2, int32_t _phase, int32_t _speed, int32_t hp); //46187d (client: 46548d)

	int32_t FindEffectTimer(int32_t effect_id);
	int32_t GetControlGroup(); //41f220 (client: 420e70)
	int32_t IsBlocked(); //41f130
	void SetTileLayerCell(int32_t layer_id, CGameObject* obj); //461c41 (client: 465851)
	int32_t GetDirTo(int32_t x, int32_t y); //461f8e (client: 465b9e)
	void DrawBarFill(int32_t left, int32_t right, int32_t y, int32_t fill_width, uint16_t color_lo, uint16_t color_mid, uint16_t color_hi); //46246b
	void DrawControlGroupNumber(int32_t x, int32_t y); //4627bb (client: 4663b3)
	void AddControlGroup(int32_t grp); //41f180 (client: 420db0)
	void SetControlGroup(int32_t grp); //41f150 (client: 420d50)
	void RemoveControlGroup(int32_t grp); //41f1f0 (client: 420d80)
public:
	uint16_t unit_id; // 4
	uint8_t __pad[2];
	int32_t x_pos;  // 8
	int32_t y_pos;  // c
	int32_t z_pos;  // 10
	MapPlayerData* map_player; // 14
	uint32_t availableSpellMask; // 18
	uint32_t spells; // 1c
	uint32_t activeSpellEffectMask; // 20
	int32_t typeId; // 24
	uint32_t face; // 28
	int32_t x_pos2; // 2c
	int32_t y_pos2; // 30
	int32_t field_0x34;
	int32_t tileX; // 38
	int32_t tileY; // 3c
	int32_t mapBoundsLeft; // 40
	int32_t mapBoundsTop; // 44
	int32_t mapBoundsRight; // 48
	int32_t mapBoundsBottom; // 4c
	int32_t mapLayerActive; // 50
	int32_t screenX; // 54
	int32_t screenY; // 58
	int32_t centerWorldX8; // 5c
	int32_t centerWorldY8; // 60
	int32_t centerScreenX; // 64
	int32_t centerScreenY; // 68
	int32_t terrainHeightOffset; // 6c
	int32_t dir; // 70
	int32_t phase; // 74
	int32_t last_action; // 78
	int32_t bIsBlocked; // 7c
	int32_t m_bSelected; // 80
	int32_t field_0x84;
	int32_t controlGroupMask; // 88
	uint8_t action; // 8c
	int8_t action_dir; // 8d
	uint16_t action_target; // 8e
	int32_t action_x; // 90
	int32_t action_y; // 94
	int32_t action_z; // 98
	int32_t action_phase; // 9c
	int32_t move_accum_x; // a0
	int32_t move_accum_y; // a4
	int32_t action_segments; // a8
	int32_t action_spell; // ac
	CArray<uint16_t> actionTargets; // b0
	int32_t turn_angle; // c4
	CPoint occupiedLocation; // c8
	CArray<TokenEntry*> tokenEntries; //0xd0
	int32_t shopInventoryVisibleStart; // e4
	BigStruct2* pMapObject; // e8
	char str1[12]; // ec
	char str2[12]; // f8
	int16_t hp; // 104
	int16_t field_0x106; // 106
	int16_t hp_max; // 108
	int16_t scan_range; // 10a
	int16_t speed; // 10c
	int16_t carrying_weight_100g; // 10e
	int32_t exp_summary; // 110
	int32_t m_bSelectionDirty; // 114
	uint8_t active_spell; //autoCastSpellId
	uint8_t __pad2[3];
	CArray<GO_11c> transientVisualElements; // 11c
	CDWordArray effect_timers;
};
ASSERT_SIZE(CGameObject, 0x144);


class CUnit : public CGameObject {
	DECLARE_DYNAMIC(CUnit);
public:

	virtual ~CUnit(); //46459d (client: 46826a)

	virtual void SetSelected(int32_t selected) override; //4648e9 (client: 4685b5)
	virtual int32_t VMethod2() override;
	virtual int32_t VMethod3() override;
	virtual int32_t GetTileWidth() override;
	virtual int32_t GetTileHeight() override;
	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override;
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light) override;
	virtual void DrawHealthBar(int32_t tile_x, int32_t tile_y, int32_t hgt) override;
	virtual void DrawMinimapRect(int32_t x, int32_t y, int32_t zoom) override;
	virtual void UpdateTileLayer() override;
	virtual int32_t Update() override;

	virtual void UpdateOccupancy() override;
	virtual void MarkOccupancy() override;
	virtual int32_t UpdateBlockedState() override;
	virtual void UpdateTransientVisualElements() override;
	virtual void InitState(int16_t unit_id, int32_t typeId, int32_t x_pos, int32_t y_pos, int32_t z_pos, MapPlayerData* map_player, int32_t dir, int32_t phase, int16_t speed, int16_t hp);
	virtual void SpawnAttackProjectile();
	virtual void SpawnSpellProjectile();
	virtual void PlayCastSound();
	virtual void PlayAttackSound();
	virtual void PlayAttackVoice();
	virtual void PlayMoveVoice();
	virtual void PlaySwarmVoice();
	virtual void PlayHitSound(int32_t kind);
	virtual void PlayDefendVoice();
	virtual void PlayRetreatVoice();
	virtual void PlaySelectVoice();
	virtual void PlayPickupVoice();
	virtual void RenderHeroPortrait(const char* str, CGameBitmap* bmp1, CGameBitmap* bmp2);

	CUnit(); //46411c (client: 467de9)

	void CopyFromUnit(const Unit& uni); //46b0d7 (client: 46ee45)
	void ApplyFace(int32_t _face); //46b7d2 (client: 46f540)
	void ReloadSprite(); //46b91c (client: 46f688)
	SfxBank* GetSfxBank(); //46978b (client: 46d507)
	const char* GetStatsHint(int32_t x, int32_t y); //46d0f7 (client: 470e63)
	void DrawStatsPanel(CRect* rect); //46c124 (client: 46fe90)
	int32_t IsItemRestricted(TokenEntry* entry); //46c0c9 (client: 46fe35)

public:
	uint8_t body;
	uint8_t mind;
	uint8_t spirit;
	uint8_t reaction;
	int16_t mp;
	int16_t mp_max;
	uint8_t __gap_u0[2];
	uint8_t absorb;
	uint8_t __gap_u1;
	int16_t defence;
	int16_t attack;
	uint8_t hand_dmg_min;
	uint8_t hand_dmg_spread;
	uint8_t field_0x156;
	uint8_t skill_levels[5];
	uint8_t magic_protect[5];
	uint8_t field_0x161;
	uint8_t field_0x162;
	uint8_t __gap_u2;
	int16_t field_0x164;
	uint8_t __gap_u3[6];
	int32_t experience_per_sphere[5];
	uint8_t stat_body; // 0x180
	uint8_t stat_reaction; // 0x181
	uint8_t stat_mind; // 0x182
	uint8_t stat_spirit; // 0x183
	uint8_t death_state; // 0x184: 0=alive, 1=dying, 2..4 bone phases, >=5 fully dead
	uint8_t __gap_u5[3];
	std::array<TokenEntry*, 12> equipmentTokens; // 0x188
	int32_t unitFlags; // 0x1b8
	int32_t lastVoicePlaybackTick; // 1bc
	CSprite256* sprite;
	CSprite256* sprite_b;
	char heroSpritePictureName[16];
	int32_t heroSpriteArmorMaterial;
	uint16_t serverId;
	uint8_t __gap_u4[2];
	int32_t questFlags; //1e0
};
ASSERT_SIZE(CUnit, 0x1e4);


class TransientVisualEffectBuilder {
public:
    TransientVisualEffectBuilder(); // 4cb0a0 (client: 4cfa20)
    ~TransientVisualEffectBuilder(); // 4cb190 (client: 4cfb10)

    int32_t BuildRisingParticles(CArray<GO_11c>* existing, int16_t x_radius, int16_t y_radius, int16_t z_radius, uint16_t visual_id); // 4cc22d
    int32_t BuildFallingParticles(CArray<GO_11c>* existing, int16_t x_radius, int16_t y_radius, int16_t z_radius, uint16_t visual_id); // 4cc3fd
    void BuildEnergyColumn(int16_t height, float speed, float radius, uint32_t phase); // 4cc944
    void BuildGroundRing(int16_t height, float radius, uint32_t phase); // 4ccaf2
    void BuildGroundRingReversed(int16_t height, float radius, uint32_t phase); // 4ccc7b
    void AddOffsetElement(int16_t height, uint8_t effect_type, uint8_t phase); // 4cce04
    void AddElement(int16_t height, uint8_t phase); // 4cced2

    void BuildCirclePoints(int32_t x, int32_t y, int32_t radius, int32_t step); // 4cc5d1 (client: 4d0f45)
    void BuildTiltedRing(int32_t x, int32_t y, int32_t height, float speed, float radius, int32_t angle); // 4cc798
    void BuildLightningLine(double x0, double y0, double x1, double y1, uint8_t vfx_type); // 4cc059
    void BuildJitterWalk(double x0, double y0, double x1, double y1); // 4cb993 (client: 4d030d)
    void SampleQuadraticArc(double px0, double py0, double px1, double py1, double px2, double py2); // 4cb85f (client: 4d01d9)

    void Clear(); // 46f390

    CArray<GO_11c>* GetVisualElements() { return &this->visual_elements; } // 46f3f0

public:
    CPoint field_0x0;
    uint8_t gap_0x8[0x1c];
    CPoint field_0x24;
    CPoint field_0x2c;
    CPoint field_0x34[2];
    CPoint field_0x44[2];
    int32_t field_0x54;
    double wobble_scale;
    double line_length;
    CArray<CPoint> circle_points;
    CArray<GO_11c> visual_elements; // 0x7c
    CArray<double> arc_x;
    CArray<double> arc_y;
    CArray<double> jitter_progress;
    CArray<double> jitter_wobble;
};
ASSERT_SIZE(TransientVisualEffectBuilder, 0xe0);


// Enterable structures collection (global at 65fa38)
struct StructEnter {
	CArray<CUnit*> field_0x0;
	CArray<CUnit*> field_0x14;
	int32_t field_0x28;
	int32_t field_0x2c;

	StructEnter(); //474f20 in asm
	~StructEnter(); //474f90 (client: 477b30) in asm
	void RebuildFromMap(); //473b80 (client: 4778d0) in asm
	int32_t FUN_00473d10(); //473d10 (client: 477a60) in asm
	void FUN_00474ff0(CUnit* unit); //474ff0 in asm
	void ClearGroups(); //475010 (client: 477b90) in asm
	void FUN_00475080(CUnit* unit); //475080 (client: 477f70) in asm
};
ASSERT_SIZE(StructEnter, 0x30);

extern StructEnter g_StructEnter; //65fa38 in asm


class CAirUnit : public CUnit {
	DECLARE_DYNAMIC(CAirUnit);
public:
	virtual ~CAirUnit(); // 46f010

	virtual void Dump(CDumpContext& dc) const override; // 46d7e8

	virtual void UpdateTileLayer() override; // 46d6fb (client: 471467)

	CAirUnit(); // 46d6ad (client: 471419)
};
ASSERT_SIZE(CAirUnit, 0x1e4);


class CBackPack : public CGameObject {
	DECLARE_DYNAMIC(CBackPack);
public:

	virtual ~CBackPack(); // 46ef30

	virtual void Dump(CDumpContext& dc) const override; // 462ab1

	virtual int32_t GetTileWidth() override; // 46f5c0
	virtual int32_t GetTileHeight() override; // 46f5d0
	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override; // 462887 (client: 46647f)
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light) override; // 46295a (client: 466629)
	virtual void UpdateTileLayer() override; // 462a98 (client: 46676f)

	CBackPack(); // 462843
};
ASSERT_SIZE(CBackPack, 0x144);

class CProjectile : public CGameObject {
	DECLARE_DYNAMIC(CProjectile);
public:
	virtual ~CProjectile(); // 46f060

	virtual void Dump(CDumpContext& dc) const override; // 46eeb4

	virtual int32_t GetTileWidth() override; // 46f660
	virtual int32_t GetTileHeight() override; // 46f670
	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override; // 46e291 (client: 471fee)

	virtual int32_t Update() override; // 46d8d8 (client: 471643)
	virtual void UpdateLight() override; // 46e805 (client: 472562)
	virtual void UpdateTransientVisualElements() override; // 46eb80 (client: 4728dd)

	CProjectile(); // 46d802 (client: 47156d)
	CProjectile(CProjectile* obj); //46d85c (client: 4715c7)

public:
	CDWordArray path_history;
};
ASSERT_SIZE(CProjectile, 0x158);




//60b520
class CStructure : public CGameObject {
	DECLARE_DYNAMIC(CStructure);
public:
	virtual ~CStructure(); //46f5a0

	virtual void Dump(CDumpContext& dc) const override; //4637b0

	virtual int32_t GetTileWidth() override; //4637ca
	virtual int32_t GetTileHeight() override; //4637eb
	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override; //462d11 (client: 4669e8)
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light) override; //462f58 (client: 466c2f)
	virtual void DrawHealthBar(int32_t tile_x, int32_t tile_y, int32_t hgt) override; //4631f3 (client: 466eca)
	virtual void DrawMinimapRect(int32_t x, int32_t y, int32_t zoom) override; //46380c (client: 4674dd)
	virtual void UpdateTileLayer() override; //462b0f (client: 4667e6)
	virtual int32_t Update() override; //463a05 (client: 4676d2)
	virtual void UpdateLight() override; //463a58 (client: 467725)
	virtual int32_t UpdateBlockedState() override; //462b28 (client: 4667ff)
	// no VMethod16 override: the binary vtable keeps CGameObject's here

	CStructure(); //462acb
	CStructure(const CStructure* obj); //462aea
};
ASSERT_SIZE(CStructure, 0x144);

//60b578
class CBridge : public CStructure {
	DECLARE_DYNAMIC(CBridge);
public:
	virtual ~CBridge(); //46f580

	virtual int32_t GetTileWidth() override; //46f510 (returns bridge_width)
	virtual int32_t GetTileHeight() override; //46f530 (returns bridge_height)
	virtual void SetSize(int32_t width, int32_t height); //46f550 (client: 472ea0)

	CBridge(int32_t width, int32_t height); //463c7f (client: 46794c)

	int32_t bridge_width; // 0x144
	int32_t bridge_height; // 0x148
};
ASSERT_SIZE(CBridge, 0x14c);

//60cb70
class CVerticalWoodenBridge : public CBridge {
	DECLARE_DYNAMIC(CVerticalWoodenBridge);
public:
	virtual ~CVerticalWoodenBridge(); //4a9ef0

	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override; //463d26 (client: 4679f3)
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light) override; //463f01 (stub)

	CVerticalWoodenBridge(int32_t width, int32_t height); //4a9e90
};
ASSERT_SIZE(CVerticalWoodenBridge, 0x14c);

//60cbc8
class CHorisontalWoodenBridge : public CBridge {
	DECLARE_DYNAMIC(CHorisontalWoodenBridge);
public:
	virtual ~CHorisontalWoodenBridge(); //4a9f70

	virtual void Draw(int32_t tile_x, int32_t tile_y, int32_t light) override; //463f0e (client: 467bdb)
	virtual void DrawShadow(int32_t tile_x, int32_t tile_y, int32_t light) override; //46410f (stub)

	CHorisontalWoodenBridge(int32_t width, int32_t height); //4a9f10
};
ASSERT_SIZE(CHorisontalWoodenBridge, 0x14c);






#endif
