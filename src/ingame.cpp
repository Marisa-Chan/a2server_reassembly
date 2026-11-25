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





CArray<UnitVFXInfo*> g_VFX_info; //6610e0
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
















// 47ae53 (client: 47ed0d)
GfxFile::GfxFile(const char* _fname) {
    this->spr = nullptr;
    this->spr_b = nullptr;
    this->fname = CString("graphics\\objects\\") + _fname;
    this->inited = 0;
}

// 47aed9 (client: 47ed93)
GfxFile::~GfxFile() {
    this->Deinit();
}

// 47af39 (client: 47edf3)
void GfxFile::Init() {
    this->spr = new CSprite256(this->fname + ".256");
    this->spr_b = new CSprite256(this->fname + "b.256");
    this->spr->ResetPalette(0x10, 2, 1);
    this->inited = 1;
}

// 47b0ab (client: 47ef65)
void GfxFile::Deinit() {
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

// 41f870 (client: 420720)
CSprite256* GfxFile::GetSprite() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

// 41f8a0 (client: 420750)
CSprite256* GfxFile::GetSpriteB() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}


// 47c85d (client: 480717)
UnitGfxFile::UnitGfxFile(const char* _fname) {
    this->spr = nullptr;
    this->spr_b = nullptr;
    this->fname = CString("graphics\\units\\") + _fname;
    this->inited = 0;
}

// 47c8e3 (client: 48079d)
UnitGfxFile::~UnitGfxFile() {
    this->Deinit();
}

// 47c943 (client: 4807fd)
void UnitGfxFile::Init() {
    this->spr = new CSprite256(this->fname + ".256");
    this->spr_b = new CSprite256(this->fname + "b.256");
    this->spr->ResetPalette(0x10, 2, 1);
    this->inited = 1;
}

// 47caa4 (client: 48095e)
void UnitGfxFile::Deinit() {
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

// 46f700 (client: 473090)
CSprite256* UnitGfxFile::GetSprite() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

// 46f730 (client: 4730c0)
CSprite256* UnitGfxFile::GetSpriteB() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}

// 46f680 (client: 473010)
CGamePalette* UnitVFXInfo::GetPalette(int32_t idx) {
    uint16_t* pal = this->palettes[idx]->GetPalette(0);
    if (pal == nullptr) {
        this->palettes[idx]->SetPalette((RGBQUAD*)this->palette_datas[idx], 0x10, 2, 1);
    }
    return this->palettes[idx];
}

// 46f6e0 (client: 473070)
uint8_t* UnitVFXInfo::GetPaletteData(int32_t idx) {
    return this->palette_datas[idx];
}


// 47eca2 (client: 482b5c)
StructureInfo::StructureInfo(const char* _fname) {
    this->inited = 0;
    this->fname = CString("graphics\\structures\\") + _fname;
}


// 47ed4e (client: 482c08)
StructureInfo::~StructureInfo() {
    this->Deinit();
    if (this->anim_mask) {
        delete[] this->anim_mask;
        this->anim_mask = nullptr;
    }
}


// 47edd8 (client: 482c92)
void StructureInfo::Init() {
    this->spr = new CSprite256(this->fname + ".256");
    this->spr_b = new CSprite256(this->fname + "b.256");
    this->spr->ResetPalette(0x10, 2, 1);
    this->inited = 1;
}


// 47ef52 (client: 482e0c)
void StructureInfo::Deinit() {
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

// 46f790 (client: 472e00)
CSprite256* StructureInfo::GetSprite() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr;
}

// 46f7c0 (client: 472e30)
CSprite256* StructureInfo::GetSpriteB() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->spr_b;
}


// 47e1ec (client: 4820a6)
ProjectileInfo::ProjectileInfo(const char* fname, int32_t _a16) {
    this->a16 = _a16;
    this->filename = fname;
    this->inited = 0;
}

// 47e50b (client: 4823c5)
ProjectileInfo::~ProjectileInfo() {
    this->Deinit();
}


// 47e26b (client: 482125)
void ProjectileInfo::Init() {
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

// 47e56b (client: 482425)
void ProjectileInfo::Deinit() {
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

// 41f8f0 (client: 420780)
CSprite256* ProjectileInfo::GetSprite() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite1;
}

// 46f760 (client: 473240)
CSprite256* ProjectileInfo::GetSpriteB() {
    if (this->inited == 0) {
        this->Init();
    }
    return this->sprite2;
}








