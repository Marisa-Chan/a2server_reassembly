#include "ingame.h"
#include <afx.h>
#include "gameobj.h"
#include "net.h"
#include "packet.h"
#include "main_window.h"
#include "txtfile.h"
#include "gfx.h"
#include "game_app.h"
#include "unit.h"
#include "mouse.h"
#include "item.h"
#include "player_file.h"
#include "map_stuff.h"
#include "quest.h"
#include "world.h"
#include "quest_map.h"
#include "file.h"
#include "spell.h"
#include "resource.h"
#include "sound.h"

#include <cmath>





CArray<UnitVFXUnfo*> g_VFX_info; //6610e0
CArray<GfxFile*> g_GfxFiles; //661100
CArray<GfxObject*> g_GfxObjects; //665330
CArray<StructureInfo*> g_StructuresInfo; //661098
CArray<ProjectileInfo*> g_ProjectileInfos; //6610b0
CArray<UnitGfxFile*> g_UnitGfxFiles; //665118

CGamePalette* g_pal_projectiles; //665490

int32_t g_TerrainHidden = 0; //660f68
int32_t g_ShowFps = 0; //660f6c
SessionError g_GameStatusCode = SESSION_ERROR_NONE; //660f8c
int32_t g_JoinState = 0; //660f90
int32_t g_ShowNetStats = 0; //660f70
CGamePalette* g_pal_projectile_; //665494

CA16* g_spr_smoke[2]; //6610f8

CStringArray g_WwwServerList; // 666a00
















GfxFile::GfxFile(const char* _fname) { //47ae53
	this->spr = nullptr;
	this->spr_b = nullptr;
	this->fname = CString("graphics\\objects\\") + _fname;
	this->inited = 0;
}

GfxFile::~GfxFile() { //47aed9
	this->Deinit();
}

void GfxFile::Init() { // 47af39
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}

void GfxFile::Deinit() { //47b0ab
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* GfxFile::GetSprite() { //41f870
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

CSprite256* GfxFile::GetSpriteB() { //41f8a0
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}


UnitGfxFile::UnitGfxFile(const char* _fname) { //47c85d
	this->spr = nullptr;
	this->spr_b = nullptr;
	this->fname = CString("graphics\\units\\") + _fname;
	this->inited = 0;
}

UnitGfxFile::~UnitGfxFile() { //47c8e3
	this->Deinit();
}

void UnitGfxFile::Init() { // 47c943
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}

void UnitGfxFile::Deinit() { //47caa4
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* UnitGfxFile::GetSprite() { //46f700
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

CSprite256* UnitGfxFile::GetSpriteB() { //46f730
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}

CGamePalette* UnitVFXUnfo::GetPalette(int32_t idx) { //46f680
    uint16_t* pal = this->palettes[idx]->GetPalette(0);
    if (pal == nullptr) {
        this->palettes[idx]->SetPalette((RGBQUAD*)this->palette_datas[idx], 0x10, 2, 1);
    }
    return this->palettes[idx];
}

uint8_t* UnitVFXUnfo::GetPaletteData(int32_t idx) { //46f6e0
    return this->palette_datas[idx];
}


StructureInfo::StructureInfo(const char* _fname) { //47eca2
	this->inited = 0;
	this->fname = CString("graphics\\structures\\") + _fname;
}


StructureInfo::~StructureInfo() { //47ed4e
	this->Deinit();
	if (this->anim_mask) {
		delete[] this->anim_mask;
		this->anim_mask = nullptr;
	}
}


void StructureInfo::Init() { //47edd8
	this->spr = new CSprite256(this->fname + ".256");
	this->spr_b = new CSprite256(this->fname + "b.256");
	this->spr->ResetPalette(0x10, 2, 1);
	this->inited = 1;
}


void StructureInfo::Deinit() { //47ef52
	if (this->inited) {
		if (this->spr) {
			delete this->spr;
			this->spr = nullptr;
		}
		if (this->spr_b) {
			delete this->spr_b;
			this->spr_b = nullptr;
		}
		this->inited = 0;
	}
}

CSprite256* StructureInfo::GetSprite() { //46f790
	if (this->inited == 0) {
		this->Init();
	}
	return this->spr;
}

CSprite256* StructureInfo::GetSpriteB() { //46f7c0
	if (this->inited == 0) {
		this->Init();
	}
	return this->spr_b;
}


ProjectileInfo::ProjectileInfo(const char* fname, int32_t _a16) { //47e1ec
	this->a16 = _a16;
	this->filename = fname;
	this->inited = 0;
}

ProjectileInfo::~ProjectileInfo() { //47e50b
	this->Deinit();
}


void ProjectileInfo::Init() { //47e26b
	if (this->inited) {
		return;
	}

	CString str = CString("graphics\\projectiles\\") + this->filename;
	if (this->a16) {
		this->sprite1 = new CA16(str + ".16a");
		this->sprite2 = nullptr;
	} else {
		this->sprite1 = new CSprite256(str + ".256");
		this->sprite2 = new CSprite256(str + "b.256");
	}

	if (this->palette) {
		if (this->a16) {
			this->sprite1->ResetPalette(16, 4, 0);
		} else {
			this->sprite1->ResetPalette(16, 2, 1);
			this->sprite2->ResetPalette(16, 2, 1);
		}
	}

	this->inited = 1;
}

void ProjectileInfo::Deinit() { //47e56b
	if (!this->inited) {
		return;
	}

	if (this->sprite1) {
		delete this->sprite1;
		this->sprite1 = nullptr;
	}

	if (this->sprite2) {
		delete this->sprite2;
		this->sprite2 = nullptr;
	}

	this->inited = 0;
}

CSprite256* ProjectileInfo::GetSprite() { //41f8f0
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite1;
}

CSprite256* ProjectileInfo::GetSpriteB() { //46f760
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite2;
}








