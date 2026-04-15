#pragma once
#include <inttypes.h>
#include <array>
#include "asm_mfc.h"
#include "mfc_templ.h"

class UnitVFXUnfo;
class CGamePalette;

extern int32_t INT_00660f8c;
extern int32_t INT_00660f90;
extern CArray<UnitVFXUnfo*> g_VFX_info; //6610e0


class UnitVFXUnfo : public CObject
{
public:
	virtual ~UnitVFXUnfo();
	UnitVFXUnfo();

public:
	int32_t ID;
	int32_t N;
	int32_t file;
	int32_t index;
	int32_t move_phases;
	int32_t move_begin_phases;
	int32_t attack_phases;
	int32_t dying_phases;
	int32_t bone_phases;
	int32_t idle_phases;
	int32_t width;
	int32_t height;
	int32_t center_x;
	int32_t center_y;
	CArray<int32_t> move_anim_frames;
	int32_t move_anim_frame_cnt;
	CArray<int32_t> attack_anim_frames;
	int32_t attack_anim_frame_cnt;
	CArray<int32_t> idle_anim_frames;
	int32_t idle_anim_frame_cnt;
	CRect selection;
	int32_t dying;
	int32_t palette;
	std::array <CGamePalette*, 8> palettes;
	std::array<uint8_t*, 8> palette_datas;
	CArray<int32_t> sound;
	int32_t tile_size;
	int32_t projectile;
	char info_picture[16];
	CArray<int32_t> shoot_offset;
	int32_t shoot_delay;
	int32_t attack_delay;
	int32_t z;
	int32_t flip;
	char desc_text[32];
};
ASSERT_SIZE(UnitVFXUnfo, 0x14c);