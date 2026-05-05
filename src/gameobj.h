#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include <inttypes.h>

#include "asm_mfc.h"
#include "alm.h"

class BigStruct2;
class CGameBitmap;
class CSprite256;
class Unit;
class ProjectileInfo;

extern CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0


class GO_d0 : public CObject
{
public:

	int32_t FUN_0041f0d0();

	virtual ~GO_d0();

	GO_d0();
	GO_d0(int arg); //439009
	GO_d0(const GO_d0* obj);
	GO_d0(uint8_t** data, int arg);

	void operator=(const GO_d0& obj);

	CString FUN_004394f3(); //4394f3
	int FUN_004396d6(); //4396d6
public:
	uint16_t field_0x4 = 0;
	uint16_t item_id = 0;
	uint8_t flg = 0;
	uint8_t field_0x9 = 0;
	uint8_t field_0xa = 0;
	uint8_t field_0xb = 0;
	void* field_0xc = nullptr;
	uint32_t field_0x10 = 1;
	uint32_t field_0x14 = 0;
	uint32_t field_0x18 = 0;
	int32_t field_0x1c = -1;
	int32_t field_0x20 = -1;
};
ASSERT_SIZE(GO_d0, 0x24);

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

	int32_t FUN_0041f110() { return field_0x80; } //41f110
	uint32_t FUN_0041f1c0(uint32_t t) { return (1 << t) & field_0x88; } //41f1c0
	void FUN_0046190d();
	void SetVals(uint16_t uni_id, int type_id, int32_t xpos, int32_t ypos, int32_t unk1, int32_t unk2, int32_t _phase, int32_t _speed, int32_t hp); //46187d

	int32_t FUN_00462405(int32_t effect_id);
public:
	uint16_t unit_id;
	uint8_t __pad[2];
	int32_t x_pos;
	int32_t y_pos;
	int32_t z_pos;
	MapPlayerData* field_0x14;
	int32_t field_0x18;
	int32_t spells;
	int32_t field_0x20;
	int32_t typeId;
	int32_t face;
	int32_t x_pos2;
	int32_t y_pos2;
	int32_t field_0x34;
	int32_t field_0x38;
	int32_t field_0x3c;
	int32_t field_0x40;
	int32_t field_0x44;
	int32_t field_0x48;
	int32_t field_0x4c;
	int32_t field_0x50;
	int32_t field_0x54;
	int32_t field_0x58;
	int32_t field_0x5c;
	int32_t field_0x60;
	int32_t screen_x;
	int32_t screen_y;
	int32_t field_0x6c;
	int32_t dir;
	int32_t phase;
	int32_t last_action;
	int32_t field_0x7c;
	int32_t field_0x80;
	int32_t field_0x84;
	int32_t field_0x88;
	uint8_t action;
	int8_t action_dir;
	uint16_t action_target;
	int32_t action_x;
	int32_t action_y;
	int32_t action_z;
	int32_t action_phase;
	int32_t field_0xa0;
	int32_t field_0xa4;
	int32_t action_segments;
	int32_t action_spell;
	CArray<uint16_t> field_0xb0;
	int32_t field_0xc4;
	int32_t field_0xc8;
	int32_t field_0xcc;
	CArray<GO_d0*> field_0xd0;
	int32_t field_0xe4;
	BigStruct2* field_0xe8;
	char field_0xec[12];
	char field_0xf8[12];
	int16_t hp;
	int16_t field_0x106;
	int16_t hp_max;
	int16_t scan_range;
	int16_t speed;
	int16_t carrying_weight_100g;
	int32_t exp_summary;
	int32_t field_0x114;
	uint8_t active_spell;
	uint8_t __pad2[3];
	CArray<GO_11c> field_0x11c;
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

	void FUN_0046b0d7(const Unit& uni);
	void FUN_0046b7d2(int32_t _face);
	void FUN_0046b91c(); //46b91c

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
	GO_d0* field_0x188[12];
	int32_t field_0x1b8;
	int32_t field_0x1bc;
	CSprite256* field_0x1c0;
	CSprite256* field_0x1c4;
	char field_0x1c8[16];
	int32_t field_0x1dc;
	uint16_t serverId;
	uint8_t __gap_u4[2];
	int32_t field_0x1e0;
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




class ProjectileInfo : public CObject
{
public:
	virtual ~ProjectileInfo();
public:
	CSprite256* sprite1;
	CSprite256* sprite2;
	CString filename;
	int32_t phases;
	int32_t id;
	int32_t rotation_phases;
	int32_t width;
	int32_t height;
	int32_t sfx;
	int32_t palette;
	int32_t homing;
	int32_t flip;
	int32_t spr_8bit;
	int32_t loaded;
};
ASSERT_SIZE(ProjectileInfo, 0x3c);


#endif