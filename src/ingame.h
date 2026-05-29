#pragma once
#include <inttypes.h>
#include <array>
#include "asm_mfc.h"
#include "mfc_templ.h"
#include "gfx.h"

class UnitVFXUnfo;
class CGamePalette;
class GfxFile;
class GfxObject;
class StructureInfo;
class ProjectileInfo;
class UnitGfxFile;

extern int32_t INT_00660f8c;
extern int32_t INT_00660f90;
extern CArray<UnitVFXUnfo*> g_VFX_info; //6610e0
extern CArray<GfxFile*> g_GfxFiles; //661100
extern CArray<GfxObject*> g_GfxObjects; //665330
extern CArray<StructureInfo*> g_StructuresInfo; //661098
extern CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0
extern CArray<UnitGfxFile*> g_UnitGfxFiles; //665118

extern CGamePalette* g_pal_projectiles; //665490
extern CGamePalette* g_pal_projectile_; //665494
extern CA16* g_spr_smoke[2]; //6610f8

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

//60b880
class GfxFile : public CObject
{
public:
	GfxFile(const char* _fname); //47ae53
	~GfxFile();

	void Init();
	void Deinit(); //47b0ab

public:
	CSprite256* spr;
	CSprite256* spr_b;
	CString fname;
	int32_t inited;
};
ASSERT_SIZE(GfxFile, 0x14);


//60b898
class GfxObject : public CObject
{
public:
	GfxObject() = default;  //47b127
	~GfxObject() = default; //47b1ef

public:
	int32_t id;
	int32_t sn;
	int32_t file;
	int32_t index;
	int32_t phases;
	int32_t width;
	int32_t height;
	int32_t center_x;
	int32_t center_y;
	CDWordArray frames;
	int32_t frame_count;
	int32_t fire_object;
	int32_t dead_object;
	int32_t in_map_editor;
	char desc_text[32];
};
ASSERT_SIZE(GfxObject, 0x6c);

//60b8b0
class UnitGfxFile : public CObject
{
public:
	UnitGfxFile() = default;  //47c806
	UnitGfxFile(const char* _fname); //47c85d
	~UnitGfxFile(); //47c8e3

	void Init(); //47c943
	void Deinit(); //47caa4
public:
	CSprite256* spr;
	CSprite256* spr_b;
	CString fname;
	int32_t inited;
};
ASSERT_SIZE(UnitGfxFile, 0x14);


//60b910
class StructureInfo : public CObject
{
public:
	StructureInfo(const char *fname); //47eca2
	~StructureInfo(); //47ed4e

	void Init(); //
	void Deinit(); //47ef52
public:
	CSprite256* spr = nullptr;
	CSprite256* spr_b = nullptr;
	int32_t id;
	int32_t tile_width;
	int32_t tile_height;
	int32_t full_height;
	int32_t phases;
	CRect selection;
	int32_t shadow_y;
	char* anim_mask = nullptr;
	int32_t field_0x38;
	int32_t frame_count;
	CDWordArray frames;
	char picture[16];
	int32_t indestructible;
	char desc_text[32];
	int32_t variable_size;
	int32_t usable;
	int32_t flat;
	int32_t light_radius;
	int32_t light_pulse;
	CString fname;
	int32_t inited;
};
ASSERT_SIZE(StructureInfo, 0xa4);


class ProjectileInfo : public CObject
{
public:
	ProjectileInfo(const char* fname, int32_t a16); //47e1ec
	virtual ~ProjectileInfo(); //47e50b

	void Init(); //47e26b
	void Deinit(); //47e56b
public:
	CSprite256* sprite1 = nullptr;
	CSprite256* sprite2 = nullptr;
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
	int32_t a16;
	int32_t inited = 0;
};
ASSERT_SIZE(ProjectileInfo, 0x3c);