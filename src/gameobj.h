#ifndef GAMEOBJ_H
#define GAMEOBJ_H

#include "asm_mfc.h"
#include "alm.h"

class BigStruct2;

class CGameObject : CObject
{
	DECLARE_DYNAMIC(CGameObject);
public:


public:
	uint16_t field_0x4;
	uint8_t __pad[2];
	int32_t x_pos;
	int32_t y_pos;
	int32_t field_0x10;
	MapPlayerData* field_0x14;
	int32_t field_0x18;
	int32_t field_0x1c;
	int32_t field_0x20;
	int32_t picture;
	int32_t field_0x28;
	int32_t x_pos2;
	int32_t y_pos2;
	int32_t field_0x34[15];
	int32_t field_0x70;
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
	CArray<uint32_t> field_0xb0;
	int32_t field_0xc4;
	int32_t field_0xc8;
	int32_t field_0xcc;
	CArray<uint32_t> field_0xd0;
	int32_t field_0xe4;
	BigStruct2* field_0xe8;
	int32_t field_0xec;
	int32_t field_0xf0;
	int32_t field_0xf4;
	int32_t field_0xf8;
	int32_t field_0xfc;
	int32_t field_0x100;
	int16_t field_0x104;
	int16_t field_0x106;
	int16_t field_0x108;
	int16_t field_0x10a;
	int16_t field_0x10c;
	int16_t field_0x10e;
	int32_t field_0x110;
	int32_t field_0x114;
	int32_t field_0x118;
	CArray<uint32_t> field_0x11c;
	CArray<int32_t> field_0x130;
};
ASSERT_SIZE(CGameObject, 0x144);



#endif