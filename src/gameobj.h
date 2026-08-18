#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include <inttypes.h>

#include "asm_mfc.h"
#include "alm.h"
#include "util.h"

class BigStruct2;
class CGameBitmap;
class CSprite256;
class Unit;

extern CStringArray g_CUnitMaterialSpritePaths; //660e70


class TokenEntry : public CObject
{
public:
	enum {
		TYPE_SPELL = 0x0E,
		CAST_SPELL_ATTRIBUTE_ID = 0x29,
	};
public:

	int32_t FUN_0041f0d0();

	virtual ~TokenEntry(); //4394bf

	TokenEntry(); //438f90
	TokenEntry(int arg); //439009
	TokenEntry(const TokenEntry* obj);
	TokenEntry(uint8_t** data, int arg);

	void operator=(const TokenEntry& obj);

	CString FUN_004394f3(); //4394f3
	int FUN_004396d6(); //4396d6

	int GetType() const { return (item_id >> 8) & 0xf; } //41f0b0
	int GetId() const { return item_id & 0x1f; } //43a800
	int GetMaterial() const { return (item_id >> 0xc); } //43a7c0

	uint32_t GetAttribute(uint8_t aid) const //4397de
	{
		if (!mods || mods_count == 0 || mods_size == 0)
			return 0;

		int pos = 0;
		uint32_t result = 0;

		const uint8_t* cmods = (const uint8_t*)mods;

		for (int i = 0; i < mods_count; i++)
		{
			uint8_t k = cmods[pos];
			if (k == aid)
			{
				if (k == 1)
					result = GetU32(cmods + pos + 1);
				else
					result = cmods[pos + 1];
			}
			
			pos += k == 1 ? 5 : 2;
		}
		return result;
	}

	uint32_t GetCastSpellId() const { return GetAttribute(CAST_SPELL_ATTRIBUTE_ID); } //41f0f0
public:
	uint16_t field_0x4 = 0;
	uint16_t item_id = 0;
	uint8_t flg = 0;
	uint8_t mods_count = 0;
	uint8_t mods_size = 0;
	uint8_t field_0xb = 0;
	void* mods = nullptr;
	uint32_t field_0x10 = 1;
	uint32_t field_0x14 = 0;
	uint32_t field_0x18 = 0;
	int32_t field_0x1c = -1;
	int32_t field_0x20 = -1;
};
ASSERT_SIZE(TokenEntry, 0x24);

struct GO_11c
{
	int16_t field_0x0;
	int16_t field_0x2;
	int16_t field_0x4;
	uint8_t field_0x6;
	uint8_t field_0x7;
};
ASSERT_SIZE(GO_11c, 8);


class CGameObject : public CObject
{
	DECLARE_DYNAMIC(CGameObject);
public:

	virtual ~CGameObject();

	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod1(int32_t arg1);
	virtual int32_t VMethod2();
	virtual int32_t VMethod3();
	virtual int32_t VMethod4();
	virtual int32_t VMethod5();
	virtual void VMethod6(int32_t arg1, int32_t arg2, int32_t arg3);
	virtual void VMethod7(int32_t arg1, int32_t arg2, int32_t arg3);
	virtual void VMethod8(int32_t arg1, int32_t arg2, int32_t arg3);
	virtual void VMethod9(int32_t arg1, int32_t arg2, int32_t arg3);
	virtual void VMethod10();
	virtual int32_t VMethod11();
	virtual void VMethod12();
	virtual void VMethod13();
	virtual void VMethod14();
	virtual int32_t VMethod15();
	virtual void VMethod16();

	CGameObject();
	CGameObject(const CGameObject *obj);

	int32_t IsSelected() { return m_bSelected; } //41f110
	uint32_t FUN_0041f1c0(uint32_t t) { return (1 << t) & controlGroupMask; } //41f1c0
	void FUN_0046190d();
	void SetVals(uint16_t uni_id, int type_id, int32_t xpos, int32_t ypos, int32_t unk1, int32_t unk2, int32_t _phase, int32_t _speed, int32_t hp); //46187d

	int32_t FUN_00462405(int32_t effect_id);
	void FUN_0041f180(int32_t grp); //41f180
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
	int32_t face; // 28
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
	int32_t field_0xa0; // a0
	int32_t field_0xa4; // a4
	int32_t action_segments; // a8
	int32_t action_spell; // ac
	CArray<uint16_t> actionTargets; // b0
	int32_t field_0xc4; // c4
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
	CDWordArray field_0x130;
};
ASSERT_SIZE(CGameObject, 0x144);


class CUnit : public CGameObject
{
	DECLARE_DYNAMIC(CUnit);
public:

	virtual ~CUnit();

	virtual void VMethod1(int32_t arg1) override;
	virtual int32_t VMethod2() override;
	virtual int32_t VMethod3() override;
	virtual int32_t VMethod4() override;
	virtual int32_t VMethod5() override;
	virtual void VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod8(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod9(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod10() override;
	virtual int32_t VMethod11() override;

	virtual void VMethod13() override;
	virtual void VMethod14() override;
	virtual int32_t VMethod15() override;
	virtual void VMethod16() override;
	virtual void VMethod17(int16_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, MapPlayerData* arg6, int32_t arg7, int32_t arg8, int16_t arg9, int16_t arg10);
	virtual void VMethod18();
	virtual void VMethod19();
	virtual void VMethod20();
	virtual void VMethod21();
	virtual void VMethod22();
	virtual void VMethod23();
	virtual void VMethod24();
	virtual void VMethod25(int32_t arg1);
	virtual void VMethod26();
	virtual void VMethod27();
	virtual void VMethod28();
	virtual void VMethod29();
	virtual void VMethod30(const char* str, CGameBitmap* bmp1, CGameBitmap* bmp2);

	CUnit();

	void CopyFromUnit(const Unit& uni); //46b0d7
	void ApplyFace(int32_t _face); //46b7d2
	void ReloadSprite(); //46b91c

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
	uint8_t field_0x180[8];
	TokenEntry* equipmentTokens[12]; // 0x188
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


class CAirUnit : public CUnit
{
	DECLARE_DYNAMIC(CAirUnit);
public:
	virtual ~CAirUnit();

	virtual void VMethod10() override;

	CAirUnit();
};
ASSERT_SIZE(CAirUnit, 0x1e4);


class CBackPack : public CGameObject
{
	DECLARE_DYNAMIC(CBackPack);
public:

	virtual ~CBackPack();

	virtual void Dump(CDumpContext& dc) const override;

	virtual int32_t VMethod4() override;
	virtual int32_t VMethod5() override;
	virtual void VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod10() override;

	CBackPack();
};
ASSERT_SIZE(CBackPack, 0x144);

//60b6f0
class CProjectile : public CGameObject
{
	DECLARE_DYNAMIC(CProjectile);
public:
	virtual ~CProjectile();

	virtual void Dump(CDumpContext& dc) const override;

	virtual int32_t VMethod4() override;
	virtual int32_t VMethod5() override;
	virtual void VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) override;

	virtual int32_t VMethod11() override;
	virtual void VMethod12() override;
	virtual void VMethod16() override;

	CProjectile();

public:
	CDWordArray field_0x144;
};
ASSERT_SIZE(CProjectile, 0x158);




//60b520
class CStructure : public CGameObject
{
	DECLARE_DYNAMIC(CStructure);
public:
	virtual ~CStructure();

	virtual void Dump(CDumpContext& dc) const override;

	virtual int32_t VMethod4() override;
	virtual int32_t VMethod5() override;
	virtual void VMethod6(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod7(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod8(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod9(int32_t arg1, int32_t arg2, int32_t arg3) override;
	virtual void VMethod10() override;
	virtual int32_t VMethod11() override;
	virtual void VMethod12() override;
	virtual int32_t VMethod15() override;
	virtual void VMethod16() override;

	

	CStructure();
};
ASSERT_SIZE(CStructure, 0x144);






#endif