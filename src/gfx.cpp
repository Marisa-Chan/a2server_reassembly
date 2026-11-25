#include <algorithm>
#include <inttypes.h>
#include <stdio.h>
#include "game_app.h"
#include <ddraw.h>
#include "gfx.h"
#ifdef A2CLIENT_PATCH
#include <vector>
#include "patch/build_version.h"
#include "patch/client_config.h"
#include "patch/hat_client.h"
#include "lib/lodepng.h"
#endif
#include "ingame.h"
#include "util.h"
#include "file.h"
#include "main_window.h"

IMPLEMENT_DYNAMIC(CGamePalette, CObject);

IMPLEMENT_DYNAMIC(CGameBitmap, CObject);
IMPLEMENT_DYNAMIC(CBmp256, CGameBitmap);
IMPLEMENT_DYNAMIC(CBmp64, CGameBitmap);
IMPLEMENT_DYNAMIC(CSprite256, CGameBitmap);
IMPLEMENT_DYNAMIC(CA16, CSprite256);
IMPLEMENT_DYNAMIC(CSprite16, CGameBitmap);

#ifdef A2CLIENT_PATCH
int32_t gfx_bits = 32; //65fb74
#else
int32_t gfx_bits = 16; //65fb74
#endif

IDirectDraw* g_ddraw = nullptr; //659bec
IDirectDrawSurface* g_ddsurface = nullptr; //659c90
int32_t g_ddsurf_lock = 0; //659bfc
#ifndef A2CLIENT_PATCH
IDirectDrawSurface* g_ddsurface2 = nullptr; //659c8c
#endif
int32_t g_ddsurf_lock2 = 0; //659bf4
#ifndef A2CLIENT_PATCH
IDirectDrawSurface* g_ddsurface3 = nullptr; //659a68
#endif
int32_t g_ddsurf_lock3 = 0; //659bf8

#ifdef A2CLIENT_PATCH
CRect g_DisplaySize;
static uint16_t* g_surf1_buf = nullptr;
static uint16_t* g_surf2_buf = nullptr;
static uint16_t* g_surf3_buf = nullptr;
static int32_t g_surf3_w = 0;
static int32_t g_surf3_h = 0;
static int32_t g_is_fullscreen = 0;
static int32_t g_display_bpp = 32;
static int32_t g_display_is_rgb565 = 0;
static uint32_t g_rgb565_to_32[65536];
static uint16_t g_rgb565_to_16[65536];

static int32_t g_scale_src_w = 0;
static int32_t g_scale_src_h = 0;
static int32_t g_scale_dst_w = 0;
static int32_t g_scale_dst_h = 0;
static int32_t* g_scale_sx0 = nullptr;
static int32_t* g_scale_sx1 = nullptr;
static uint32_t* g_scale_wx = nullptr;
static int32_t* g_scale_sy0 = nullptr;
static int32_t* g_scale_sy1 = nullptr;
static uint32_t* g_scale_wy = nullptr;
static uint32_t* g_scale_row0 = nullptr;
static uint32_t* g_scale_row1 = nullptr;

static void FreeScaleTables() {
    delete[] g_scale_sx0;
    g_scale_sx0 = nullptr;
    delete[] g_scale_sx1;
    g_scale_sx1 = nullptr;
    delete[] g_scale_wx;
    g_scale_wx = nullptr;
    delete[] g_scale_sy0;
    g_scale_sy0 = nullptr;
    delete[] g_scale_sy1;
    g_scale_sy1 = nullptr;
    delete[] g_scale_wy;
    g_scale_wy = nullptr;
    delete[] g_scale_row0;
    g_scale_row0 = nullptr;
    delete[] g_scale_row1;
    g_scale_row1 = nullptr;
    g_scale_src_w = 0;
    g_scale_src_h = 0;
    g_scale_dst_w = 0;
    g_scale_dst_h = 0;
}

static void EnsureScaleTables(int32_t src_w, int32_t src_h, int32_t dst_w, int32_t dst_h) {
    if (g_scale_src_w == src_w && g_scale_src_h == src_h &&
        g_scale_dst_w == dst_w && g_scale_dst_h == dst_h &&
        g_scale_sx0 != nullptr) {
        return;
    }

    FreeScaleTables();
    g_scale_src_w = src_w;
    g_scale_src_h = src_h;
    g_scale_dst_w = dst_w;
    g_scale_dst_h = dst_h;

    g_scale_sx0 = new int32_t[dst_w];
    g_scale_sx1 = new int32_t[dst_w];
    g_scale_wx = new uint32_t[dst_w];
    g_scale_sy0 = new int32_t[dst_h];
    g_scale_sy1 = new int32_t[dst_h];
    g_scale_wy = new uint32_t[dst_h];
    g_scale_row0 = new uint32_t[dst_w];
    g_scale_row1 = new uint32_t[dst_w];

    for (int32_t dx = 0; dx < dst_w; dx++) {
        int64_t num = static_cast<int64_t>(2 * dx + 1) * src_w * 128 - static_cast<int64_t>(dst_w) * 128;
        if (num <= 0 || src_w <= 1) {
            g_scale_sx0[dx] = 0;
            g_scale_sx1[dx] = 0;
            g_scale_wx[dx] = 0;
        } else {
            int32_t fp = static_cast<int32_t>(num / dst_w);
            int32_t sx0 = fp >> 8;
            if (sx0 >= src_w - 1) {
                g_scale_sx0[dx] = src_w - 1;
                g_scale_sx1[dx] = src_w - 1;
                g_scale_wx[dx] = 0;
            } else {
                g_scale_sx0[dx] = sx0;
                g_scale_sx1[dx] = sx0 + 1;
                g_scale_wx[dx] = static_cast<uint32_t>(fp & 0xFF);
            }
        }
    }

    for (int32_t dy = 0; dy < dst_h; dy++) {
        int64_t num = static_cast<int64_t>(2 * dy + 1) * src_h * 128 - static_cast<int64_t>(dst_h) * 128;
        if (num <= 0 || src_h <= 1) {
            g_scale_sy0[dy] = 0;
            g_scale_sy1[dy] = 0;
            g_scale_wy[dy] = 0;
        } else {
            int32_t fp = static_cast<int32_t>(num / dst_h);
            int32_t sy0 = fp >> 8;
            if (sy0 >= src_h - 1) {
                g_scale_sy0[dy] = src_h - 1;
                g_scale_sy1[dy] = src_h - 1;
                g_scale_wy[dy] = 0;
            } else {
                g_scale_sy0[dy] = sy0;
                g_scale_sy1[dy] = sy0 + 1;
                g_scale_wy[dy] = static_cast<uint32_t>(fp & 0xFF);
            }
        }
    }
}
#endif

CRect g_clipRect; //659d08
DDSURFACEDESC g_selDrawBitmap; //659c98
CRect g_ScreenSize; //65fb78

uint32_t g_isLowMemory = 0; //660f58

int32_t g_Shadows = 1; //62f880
int32_t g_Animation = 1; //62f884
int32_t g_Lightning = 1; //62f888


int32_t g_RBits = 5; //62f870
int32_t g_GBits = 6; //62f874
int32_t g_BBits = 5; //62f878
int32_t g_RBitShift = 11; //62f868
int32_t g_GBitShift = 5; //62f86c
int32_t g_BBitShift = 0; //660f5c
int32_t g_RBitMask = 0xf800; //62f85c
int32_t g_GBitMask = 0x7e0; //62f860
int32_t g_BBitMask = 0x1f; //62f864


uint16_t clrsh_Black[16]; //65f558
uint16_t clrsh_Oxley[16]; //65f4d8
uint16_t clrsh_GoldenFizz[16]; //65f578
uint16_t clrsh_ScreaminGreen[16]; //65ec70
uint16_t clrsh_Cumin[16]; //65f538
uint16_t clrsh_CreteToRed[16]; //65f5c8
uint16_t clrsh_ScreamGreenToRed[16]; //65f5a8
uint16_t clrsh_InvBarleyCorn[16]; //65e208

uint16_t clrsh_CoralRed[16]; //65f608
uint16_t clrsh_DullGold[16]; //65f4f8
uint16_t clrsh_TechBlack[16]; //65f090
uint16_t clrsh_ShockingBlack[16]; //65e1e8
uint16_t clrsh_CharlieBrown[16]; //65f4b8
uint16_t clrsh_LuxorGold[16]; //65e848
uint16_t* p_clrsh_Black = clrsh_TechBlack; //62e584
uint16_t* p_clrsh_Gold = clrsh_DullGold; //62e58c
uint16_t* p_clrsh_ShockingBlack = clrsh_ShockingBlack; //62e588



CGamePalette* palette_husk = nullptr; //65f628
CGamePalette* palette_paris_daisy = nullptr; //65f52c
CGamePalette* palette_black = nullptr; //65f604
CGamePalette* palette_brown_derby = nullptr; //65f5e8
CGamePalette* palette_tawny_port = nullptr; //65f5a0

uint16_t g_colors_human_pals[16][16]; //65e230
int32_t g_MessageColors; //660f80

uint16_t* g_brightnessLookup = nullptr; //65dd20
uint32_t g_brightnessLookupCount = 0; //659bf0

ColorThing g_DeltaCLR; //660e28

uint8_t g_GfxTempBuffer[92160]; //642d10
uint16_t g_ColorAddMask; //660f60

uint8_t g_TerrainHelper[128][128]; //659d20

RGBQUAD g_human_pals_data[16][256]; //661118
CArray<CGamePalette*> g_Human_palettes; //6610c8

Array2D<CBmp256*, 4, 32> g_ter_tiles; //665130

CGamePalette* g_TerPalettes[32] = {}; //661018 terrain tile palettes, filled from g_ter_tiles
CGameBitmap* g_TerDirtBmp = nullptr; //665344 terrain dirt bitmap

CBmp64* g_bmp_t_back = nullptr; //0066512c
CBmp64* g_bmp_crystalr = nullptr; //00665348
CBmp64* g_bmp_crystall = nullptr; //0066534c
CBmp64* g_bmp_headsr = nullptr; //00665350
CBmp64* g_bmp_headsl = nullptr; //00665354
CBmp64* g_bmp_cmdbarr = nullptr; //00665358
CBmp64* g_bmp_cmdbarl = nullptr; //0066535c
CBmp64* g_bmp_cmddnr = nullptr; //00665360
CBmp64* g_bmp_cmdempr = nullptr; //00665364
CBmp64* g_bmp_humanbackl = nullptr; //00665368
CBmp64* g_bmp_humanbackr = nullptr; //0066536c
CBmp64* g_bmp_textbackl = nullptr; //00665370
CBmp64* g_bmp_textbackr = nullptr; //00665374
CBmp64* g_bmp_bookopened = nullptr; //00665378
CBmp64* g_bmp_bookclosed = nullptr; //0066537c
CBmp64* g_bmp_backpackop = nullptr; //00665380
CBmp64* g_bmp_backpackcl = nullptr; //00665384
CBmp64* g_bmp_humanmode = nullptr; //00665388
CBmp64* g_bmp_textmode = nullptr; //0066538c
CBmp64* g_bmp_diskette = nullptr; //00665390
CBmp64* g_bmp_ar1 = nullptr; //00665394
CBmp64* g_bmp_ar2 = nullptr; //00665398
CBmp64* g_bmp_ar3 = nullptr; //0066539c
CBmp64* g_bmp_ar4 = nullptr; //006653a0
CBmp64* g_bmp_spellbook = nullptr; //006653a4
CBmp64* g_bmp_spellback = nullptr; //006653a8
CBmp64* g_bmp_invframe = nullptr; //006653ac
CBmp64* g_bmp_invarrow1 = nullptr; //006653b0
CBmp64* g_bmp_invarrow2 = nullptr; //006653b4
CBmp64* g_bmp_invarrow3 = nullptr; //006653b8
CBmp64* g_bmp_invarrow4 = nullptr; //006653bc
CBmp64* g_bmp_backinv = nullptr; //006653c0
CBmp64* g_bmp_spb1024l = nullptr; //006653c4
CBmp64* g_bmp_spb1024r = nullptr; //006653c8
CBmp64* g_bmp_extra1024l = nullptr; //006653cc
CBmp64* g_bmp_extra1024r = nullptr; //006653d0
CBmp64* g_bmp_inv1024l = nullptr; //006653d4
CBmp64* g_bmp_inv1024r = nullptr; //006653d8
CBmp64* g_bmp_spb800l = nullptr; //006653dc
CBmp64* g_bmp_spb800r = nullptr; //006653e0
CBmp64* g_bmp_extra800l = nullptr; //006653e4
CBmp64* g_bmp_extra800r = nullptr; //006653e8
CSprite256* gfx_interface_lm = nullptr; //006653ec
CSprite256* gfx_scrollbars = nullptr; //006653f0
CSprite256* gfx_radiob = nullptr; //006653f4
CSprite256* g_spr_t_border = nullptr; //006653f8
CSprite256* g_spr_backm = nullptr; //006653fc
CSprite256* g_spr_backf = nullptr; //00665400
CA16* g_ca16_money = nullptr; //00665404
CBmp64* g_bmp_server = nullptr; //00665408
CSprite256* g_spr_backpack = nullptr; //0066540c
CSprite256* g_spr_backpackb = nullptr; //00665410
CBmp64* gfx_ball = nullptr; //00665414
CBmp64* g_bmp_minimapdata = nullptr; //00665418
CBmp256* g_bmp_testiva = nullptr; //0066541c


CGameFont* g_font1 = nullptr; //65e228
CGameFont* g_font2 = nullptr; //65ec68
CGameFont* g_font3 = nullptr; //65f598
CGameFont* g_font4 = nullptr; //65f530



// 454a0c (client: 45864c)
void GetClipRect(CRect* r) {
    *r = g_clipRect;
}

// 4549ba (client: 4585fa)
void SetClipRect(CRect& r) {
    g_clipRect = r;
}

// 4549e5 (client: 458625)
void __cdecl SetClipRect(int32_t l, int32_t t, int32_t r, int32_t b) {
    g_clipRect.left = l;
    g_clipRect.top = t;
    g_clipRect.right = r;
    g_clipRect.bottom = b;
}

// 454a1e (client: 45865e)
void __cdecl FlushRect(int32_t l, int32_t t, int32_t r, int32_t b) {
    CRect rc(l, t, r, b);
    gfxFlushRect(rc);
}


// 4255a6 (client: 4274d4)
void __cdecl Convert24To16(uint8_t* src, uint16_t* dst, int32_t num) {
    for (; num; num--) {
        *dst = (uint16_t)((src[2] >> (8 - g_RBits)) << g_RBitShift) |
            (uint16_t)((src[1] >> (8 - g_GBits)) << g_GBitShift) |
            (uint16_t)((src[0] >> (8 - g_BBits)) << g_BBitShift);

        src += 3;
        dst++;
    }
}

// 425688 (client: 4275b6)
void __cdecl Convert16To24(uint16_t* src, uint8_t* dst, int32_t num) {
    for (; num; num--) {
        uint16_t c = *src;
        dst[0] = (c >> g_BBitShift) << (8 - g_BBits);
        dst[1] = (c >> g_GBitShift) << (8 - g_GBits);
        dst[2] = (c >> g_RBitShift) << (8 - g_RBits);

        src++;
        dst += 3;
    }
}

// 4553db (client: 45901b)
void BlitPaletted(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height, const uint16_t *palette) {
    int32_t w = r - l;
    int32_t h = b - t;

    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        l += g_clipRect.left - x;
        x = g_clipRect.left;
    }

    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        t += g_clipRect.top - y;
        y = g_clipRect.top;
    }

    if (x + w >= g_clipRect.right) {
        w = g_clipRect.right - x;
    }

    if (y + h >= g_clipRect.bottom) {
        h = g_clipRect.bottom - y;
    }

    if (w <= 0 || h <= 0) {
        return;

    //MMX optimise omitted

    }
    w = l + w;
    h = t + h;

    uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2);
    uint8_t* src = src_data + l + t * src_width;

    while (t < h) {
        for (int32_t n = w - l; n; n--) {
            *dst = palette[*src];
            src++;
            dst++;
        }
        dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch - (w - l) * 2);
        src = src + src_width - (w - l);
    }
}

// 4555f9 (client: 459239)
void BlitFlipPalettedAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height, const uint16_t* palette) {
    int32_t w = r - l;
    int32_t h = b - t;

    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        l += g_clipRect.left - x;
        x = g_clipRect.left;
    }

    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        t += g_clipRect.top - y;
        y = g_clipRect.top;
    }

    if (x + w >= g_clipRect.right) {
        w = g_clipRect.right - x;
    }

    if (y + h >= g_clipRect.bottom) {
        h = g_clipRect.bottom - y;
    }

    if (w <= 0 || h <= 0) {
        return;
    }

    h = t + h;

    uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2);
    uint8_t* src = src_data + l + (src_height - 1 - t) * src_width;

    while (t < h) {
        for (int32_t n = w; n; n--) {
            *dst += palette[*src];
            src++;
            dst++;
        }
        dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch - w * 2);
        src = src - src_width - w;
    }
}


// 454f6d (client: 458bad)
void Blit16(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height) {
    int32_t w = r - l;
    int32_t h = b - t;

#ifdef A2CLIENT_PATCH
    if (src_data == nullptr || g_selDrawBitmap.lpSurface == nullptr || src_width <= 0 || src_height <= 0) {
        return;
    }
    if (l < 0) {
        x -= l;
        w += l;
        l = 0;
    }
    if (t < 0) {
        y -= t;
        h += t;
        t = 0;
    }
#endif

    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        l += g_clipRect.left - x;
        x = g_clipRect.left;
    }
    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        t += g_clipRect.top - y;
        y = g_clipRect.top;
    }
    if (x + w >= g_clipRect.right) {
        w = g_clipRect.right - x;
    }
    
    if (y + h >= g_clipRect.bottom) {
        h = g_clipRect.bottom - y;
    }

#ifdef A2CLIENT_PATCH
    if (l >= src_width || t >= src_height) {
        return;
    }
    if (l + w > src_width) {
        w = src_width - l;
    }
    if (t + h > src_height) {
        h = src_height - t;
    }
#endif

    if (w <= 0 || h <= 0) {
        return;
    }

    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    uint8_t* src = src_data + (((src_height - 1) - t) * src_width + l) * 2;
    for (int32_t yy = t; yy < t + h; yy++) {
#ifdef A2CLIENT_PATCH
        // CRASH_fix_458CF9 (client 458CF9): validate the computed row pointers
        // before the copy; a bad clip result or freed surface aborts the blit
        if (dst == nullptr || src == nullptr) {
            return;
        }
#endif
        memcpy(dst, src, w * 2);
        dst += g_selDrawBitmap.lPitch;
        src -= src_width * 2;
    }
}

// 45525a (client: 458e9a)
void Blit16Masked(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height) {
    int32_t w = r - l;
    int32_t h = b - t;

#ifdef A2CLIENT_PATCH
    if (src_data == nullptr || g_selDrawBitmap.lpSurface == nullptr || src_width <= 0 || src_height <= 0) {
        return;
    }
    if (l < 0) {
        x -= l;
        w += l;
        l = 0;
    }
    if (t < 0) {
        y -= t;
        h += t;
        t = 0;
    }
#endif

    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        l += g_clipRect.left - x;
        x = g_clipRect.left;
    }
    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        t += g_clipRect.top - y;
        y = g_clipRect.top;
    }
    if (x + w >= g_clipRect.right) {
        w = g_clipRect.right - x;
    }

    if (y + h >= g_clipRect.bottom) {
        h = g_clipRect.bottom - y;
    }

#ifdef A2CLIENT_PATCH
    if (l >= src_width || t >= src_height) {
        return;
    }
    if (l + w > src_width) {
        w = src_width - l;
    }
    if (t + h > src_height) {
        h = src_height - t;
    }
#endif

    if (w <= 0 || h <= 0) {
        return;
    }

    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    uint16_t* src = (uint16_t*)(src_data + (((src_height - 1) - t) * src_width + l) * 2);
    for (; h; h--) {
        for (int32_t ww = w; ww; ww--) {
            if (*src != 0) {
                *(uint16_t*)dst = *src;
            }

            src++;
            dst += 2;
        }

        dst += g_selDrawBitmap.lPitch - w * 2;
        src -= src_width + w;
    }
}

// 4550e8 (client: 458d28)
void Blit16Add(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height) {
    int32_t w = r - l;
    int32_t h = b - t;

#ifdef A2CLIENT_PATCH
    if (src_data == nullptr || g_selDrawBitmap.lpSurface == nullptr || src_width <= 0 || src_height <= 0) {
        return;
    }
    if (l < 0) {
        x -= l;
        w += l;
        l = 0;
    }
    if (t < 0) {
        y -= t;
        h += t;
        t = 0;
    }
#endif

    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        l += g_clipRect.left - x;
        x = g_clipRect.left;
    }
    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        t += g_clipRect.top - y;
        y = g_clipRect.top;
    }
    if (x + w >= g_clipRect.right) {
        w = g_clipRect.right - x;
    }

    if (y + h >= g_clipRect.bottom) {
        h = g_clipRect.bottom - y;
    }

#ifdef A2CLIENT_PATCH
    if (l >= src_width || t >= src_height) {
        return;
    }
    if (l + w > src_width) {
        w = src_width - l;
    }
    if (t + h > src_height) {
        h = src_height - t;
    }
#endif

    if (w <= 0 || h <= 0) {
        return;
    }

    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    uint16_t* src = (uint16_t*)(src_data + (((src_height - 1) - t) * src_width + l) * 2);
    for (; h; h--) {
        for (int32_t ww = w; ww; ww--) {
            *(uint16_t*)dst += *src;
            src++;
            dst += 2;
        }

        dst += g_selDrawBitmap.lPitch - w * 2;
        src -= src_width + w;
    }
}



// 4559d4 (client: 459614)
void __cdecl BlitRle8(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t *)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = clr[*src];
                            }
                            src++;
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    for (int i = count / 2; i > 0; i--) {
                        *(uint32_t*)dst = (clr[src[1]] << 16) | clr[src[0]];
                        src += 2;
                        dst += 2;
                    }
                    if (count & 1) {
                        *dst = clr[*src];
                        src++;
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}


// 456a0d (client: 45a64d)
void __cdecl BlitRle8FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = clr[*src];
                            }
                            src++;
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }
            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    for (int i = count; i > 0; i--) {
                        *dst = clr[ *src ];
                        src++;
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            h--;
        }
    }
}


// 45614c (client: 459d8c)
void __cdecl BlitLightRle8(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + *dst];
                            }
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + *dst];
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}


// 455f1c (client: 459b5c)
void __cdecl BlitLightRle8Low(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + (*dst >> 3)];
                            }
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + (*dst >> 3)];
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}

// 457141 (client: 45ad81)
void __cdecl BlitLightRle8FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + *dst];
                            }
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }
            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + *dst];
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            h--;
        }
    }
}

// 456ef8 (client: 45ab38)
void __cdecl BlitLightRle8FlipXLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + (*dst >> 3)];
                            }
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }
            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + (*dst >> 3)];
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            h--;
        }
    }
}


// 455ce6 (client: 459926)
void __cdecl BlitRle8Blend(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
                            }
                            src++;
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    for (int i = count; i > 0; i--) {
                        *dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
                        src++;
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}


// 456ca7 (client: 45a8e7)
void __cdecl BlitRle8BlendFlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        while (count) {
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
                            }
                            src++;
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }
            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    for (int i = count; i > 0; i--) {
                        *dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
                        src++;
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            h--;
        }
    }
}



// 4566ff (client: 45a33f)
void __cdecl BlitLightRle8Tilt(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt) {
    int32_t off = (int16_t)((tilt * h) >> 16);
    int32_t aoff = abs(off);

    if (x + w + aoff <= g_clipRect.left) {
        return;
    }
    if (x - aoff >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    int32_t tl = 0;

    if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + *dst];
                            }
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);
                    off -= tmp;

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            
            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);
            off -= tmp;

            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - (w + tmp) * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + *dst];
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);

            dst = (uint16_t*)((uint8_t*)dst - (w + tmp) * 2 + dstpitch);
            h--;
        }
    }
}

// 4563e7 (client: 45a027)
void __cdecl BlitLightRle8TiltLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt) {
    int32_t off = (int16_t)((tilt * h) >> 16);
    int32_t aoff = abs(off);

    if (x + w + aoff <= g_clipRect.left) {
        return;
    }
    if (x - aoff >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    int32_t tl = 0;

    if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + (*dst >> 3)];
                            }
                            dst++;
                            xx++;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);
                    off -= tmp;

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);
            off -= tmp;

            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - (w + tmp) * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + (*dst >> 3)];
                        dst++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);;

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);

            dst = (uint16_t*)((uint8_t*)dst - (w + tmp) * 2 + dstpitch);
            h--;
        }
    }
}


// 4576b1 (client: 45b2f1)
void __cdecl BlitLightRle8TiltFlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt) {
    int32_t off = (int16_t)((tilt * h) >> 16);
    int32_t aoff = abs(off);

    if (x + w + aoff <= g_clipRect.left) {
        return;
    }
    if (x - aoff >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    int32_t tl = 0;

    if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + *dst];
                            }
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);
                    off -= tmp;

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }

            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);
            off -= tmp;

            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + (w - tmp) * 2 + dstpitch - tmp * 2);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + *dst];
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }
            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);

            dst = (uint16_t*)((uint8_t*)dst + (w - tmp) * 2 + dstpitch);
            h--;
        }
    }
}

// 457380 (client: 45afc0)
void __cdecl BlitLightRle8TiltFlipXLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt) {
    int32_t off = (int16_t)((tilt * h) >> 16);
    int32_t aoff = abs(off);

    if (x + w + aoff <= g_clipRect.left) {
        return;
    }
    if (x - aoff >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;
    int32_t bl = blvl * g_brightnessLookupCount;

    int32_t tl = 0;

    if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        src += count;
                        while (count) {
                            if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right) {
                                *dst = g_brightnessLookup[bl + (*dst >> 3)];
                            }
                            dst--;
                            xx--;
                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);
                    off -= tmp;

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst -= count;
                    xx -= count;
                }
            }

            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);
            off -= tmp;

            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + (w - tmp) * 2 + dstpitch - tmp * 2);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    xx += count;
                    src += count;
                    for (int i = count; i > 0; i--) {
                        *dst = g_brightnessLookup[bl + (*dst >> 3)];
                        dst--;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    int32_t tmp = tl + count * tilt;
                    tl = tmp & 0xffff;
                    tmp = (int16_t)(tmp >> 16);

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
                    h -= count;
                } else {
                    dst -= count;
                    xx += count;
                }
            }
            int32_t tmp = tl + tilt;
            tl = tmp & 0xffff;
            tmp = (int16_t)(tmp >> 16);

            dst = (uint16_t*)((uint8_t*)dst + (w - tmp) * 2 + dstpitch);
            h--;
        }
    }
}



// 455c55 (client: 459895)
void __cdecl BlitRle8IndexMask(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, uint8_t mv) {
    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + x + y * dstpitch;
    while (h > 0) {
        int32_t xx = 0;
        while (xx < w) {
            const Rle8Cmd cmd = *src;
            src++;
            const uint8_t count = cmd.count;
            if (cmd.op == Rle8Cmd::OP_PIXELS) {
                xx += count;
                src += count;

                for (int i = count; i > 0; i--) {
                    *dst = mv;
                    dst++;
                }
            } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                if (h <= count) {
                    return;
                }

                dst = (uint8_t*)dst + count * dstpitch;
                h -= count;
            } else {
                dst += count;
                xx += count;
            }
        }

        dst = (uint8_t*)dst - w + dstpitch;
        h--;
    }
}


// 459f3d (client: 45db6b)
void __cdecl BlitRle16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* src, const uint16_t* clr) { //
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle16Cmd cmd = *src;
                src++;
                uint16_t count = cmd.count;
                if (cmd.op == Rle16Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count;
                        xx += count;
                    } else {
                        if (g_isLowMemory == 0) {
                            while (count) {
                                if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                    const Rle16Pixel px = *src;
                                    *dst = clr[px.clut_index] + g_brightnessLookup[g_brightnessLookupCount + (static_cast<uint32_t>(px.alpha_level) << 16) + *dst];
                                }
                                src++;
                                dst++;
                                xx++;
                                count--;
                            }
                        } else {
                            while (count) {
                                if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                    const Rle16Pixel px = *src;
                                    *dst = clr[px.clut_index] + g_brightnessLookup[(static_cast<uint32_t>(px.alpha_level) << 13) + (*dst >> 3)];
                                }
                                src++;
                                dst++;
                                xx++;
                                count--;
                            }
                        }
                    }
                } else if (cmd.op == Rle16Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else if (cmd.op == Rle16Cmd::OP_SKIP_PIXELS) {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle16Cmd cmd = *src;
                src++;
                const uint16_t count = cmd.count;
                if (cmd.op == Rle16Cmd::OP_PIXELS) {
                    xx += count;
                    if (g_isLowMemory == 0) {
                        for (int i = count; i > 0; i--) {
                            const Rle16Pixel px = *src;
                            *dst = clr[px.clut_index] + g_brightnessLookup[g_brightnessLookupCount + (static_cast<uint32_t>(px.alpha_level) << 16) + *dst];
                            src++;
                            dst++;
                        }
                    } else {
                        for (int i = count; i > 0; i--) {
                            const Rle16Pixel px = *src;
                            *dst = clr[px.clut_index] + g_brightnessLookup[(static_cast<uint32_t>(px.alpha_level) << 13) + (*dst >> 3)];
                            src++;
                            dst++;
                        }
                    }
                } else if (cmd.op == Rle16Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else if (cmd.op == Rle16Cmd::OP_SKIP_PIXELS) {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}

// 45a2b2 (client: 45dee0)
void __cdecl BlitRle16FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* src, const uint16_t* clr) { //
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x + w - 1;
        while (h != 0) {
            while (xx >= x) {
                const Rle16Cmd cmd = *src;
                src++;
                uint16_t count = cmd.count;
                if (cmd.op == Rle16Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst -= count;
                        xx -= count;
                    } else {
                        if (g_isLowMemory == 0) {
                            while (count) {
                                if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                    const Rle16Pixel px = *src;
                                    *dst = clr[px.clut_index] + g_brightnessLookup[g_brightnessLookupCount + (static_cast<uint32_t>(px.alpha_level) << 16) + *dst];
                                }
                                src++;
                                dst--;
                                xx--;
                                count--;
                            }
                        } else {
                            while (count) {
                                if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                    const Rle16Pixel px = *src;
                                    *dst = clr[px.clut_index] + g_brightnessLookup[(static_cast<uint32_t>(px.alpha_level) << 13) + (*dst >> 3)];
                                }
                                src++;
                                dst--;
                                xx--;
                                count--;
                            }
                        }
                    }
                } else if (cmd.op == Rle16Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else if (cmd.op == Rle16Cmd::OP_SKIP_PIXELS) {
                    dst -= count;
                    xx -= count;
                }
            }
            xx = x + w - 1;
            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle16Cmd cmd = *src;
                src++;
                const uint16_t count = cmd.count;
                if (cmd.op == Rle16Cmd::OP_PIXELS) {
                    xx += count;
                    if (g_isLowMemory == 0) {
                        for (int i = count; i > 0; i--) {
                            const Rle16Pixel px = *src;
                            *dst = clr[px.clut_index] + g_brightnessLookup[g_brightnessLookupCount + (static_cast<uint32_t>(px.alpha_level) << 16) + *dst];
                            src++;
                            dst--;
                        }
                    } else {
                        for (int i = count; i > 0; i--) {
                            const Rle16Pixel px = *src;
                            *dst = clr[px.clut_index] + g_brightnessLookup[(static_cast<uint32_t>(px.alpha_level) << 13) + (*dst >> 3)];
                            src++;
                            dst--;
                        }
                    }
                } else if (cmd.op == Rle16Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else if (cmd.op == Rle16Cmd::OP_SKIP_PIXELS) {
                    dst -= count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
            h--;
        }
    }
}

// 455761 (client: 4593a1)
void __cdecl BlitRle4(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr) {
    if (x + w <= g_clipRect.left) {
        return;
    }
    if (x >= g_clipRect.right) {
        return;
    }
    if (y + h <= g_clipRect.top) {
        return;
    }
    if (y >= g_clipRect.bottom) {
        return;
    }

    const uint32_t dstpitch = g_selDrawBitmap.lPitch;

    if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom) {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        int32_t yy = y;
        int32_t xx = x;
        while (h != 0) {
            while (xx - x < w) {
                const Rle8Cmd cmd = *src;
                src++;
                uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    if (yy < g_clipRect.top || yy >= g_clipRect.bottom) {
                        src += count;
                        dst += count * 2;
                        xx += count * 2;

                        if ((src[-1] & 0xf0) == 0) {
                            dst--;
                            xx--;
                        }
                    } else {
                        while (count) {
                            uint8_t bb = *src;
                            src++;

                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = clr[bb & 0xf];
                            }
                            dst++;
                            xx++;

                            bb >>= 4;
                            if (bb == 0) {
                                break;
                            }
                            if (xx >= g_clipRect.left && xx < g_clipRect.right) {
                                *dst = clr[bb];
                            }
                            dst++;
                            xx++;

                            count--;
                        }
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    yy += count;
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }
            xx = x;
            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            yy++;
            h--;
        }
    } else {
        uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
        while (h > 0) {
            int32_t xx = 0;
            while (xx < w) {
                const Rle8Cmd cmd = *src;
                src++;
                const uint8_t count = cmd.count;
                if (cmd.op == Rle8Cmd::OP_PIXELS) {
                    for (int i = count; i > 0; i--) {
                        uint8_t bb = *src;
                        src++;

                        *dst = clr[bb & 0xf];
                        dst++;
                        xx++;

                        bb >>= 4;
                        if (bb == 0) {
                            break;
                        }
                        *dst = clr[bb];
                        dst++;
                        xx++;
                    }
                } else if (cmd.op == Rle8Cmd::OP_SKIP_LINES) {
                    if (h <= count) {
                        return;
                    }

                    dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
                    h -= count;
                } else {
                    dst += count;
                    xx += count;
                }
            }

            dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
            h--;
        }
    }
}



// 4586a4 (client: 45c2d2)
void __cdecl SetPixelColor(int32_t x, int32_t y, uint32_t clr) {
    if (x >= g_clipRect.left && x < g_clipRect.right &&
        y >= g_clipRect.top && y < g_clipRect.bottom) {
        ((uint16_t*)g_selDrawBitmap.lpSurface)[x + y * (g_selDrawBitmap.lPitch / 2)] = clr;
    }
}

// 457aa6 (client: 45b6e6)
void __cdecl FillRectClipped(int32_t l, int32_t t, int32_t r, int32_t b, uint16_t clr) {
    if (l < g_clipRect.left) {
        l = g_clipRect.left;
    }
    if (r > g_clipRect.right) {
        r = g_clipRect.right;
    }

    uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + l * 2 + t * g_selDrawBitmap.lPitch);
    int32_t width = r - l;

    if (width <= 0) {
        return;
    }

    uint32_t clr2 = ((uint32_t)clr << 16) | clr;

    for (int32_t y = t; y < b; y++) {
        if (y >= g_clipRect.top && y < g_clipRect.bottom) {
            uint32_t* p32 = (uint32_t*)dst;
            for (int32_t n = width >> 1; n != 0; n--) {
                *p32 = *p32 + clr2;
                p32++;
            }
            if (width & 1) {
                *(uint16_t*)p32 = (uint16_t)(*(uint16_t*)p32 + clr);
            }
        }
        dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch);
    }
}

// 457c5d (client: 45b89d)
void __cdecl FillRectBrightness(int32_t l, int32_t t, int32_t r, int32_t b, int32_t brightness) {
    uint16_t add = (uint16_t)(((((brightness << 4) - 1) >> (8 - g_RBits)) << g_RBitShift) |
                              ((((brightness << 4) - 1) >> (8 - g_GBits)) << g_GBitShift) |
                              ((((brightness << 4) - 1) >> (8 - g_BBits)) << g_BBitShift));
    uint16_t* table = g_brightnessLookup + brightness * g_brightnessLookupCount;

    if (l < g_clipRect.left) {
        l = g_clipRect.left;
    }
    if (r > g_clipRect.right) {
        r = g_clipRect.right;
    }

    uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + l * 2 + t * g_selDrawBitmap.lPitch);
    int32_t width = r - l;

    if (width <= 0) {
        return;
    }

    for (int32_t y = t; y < b; y++) {
        if (y >= g_clipRect.top && y < g_clipRect.bottom) {
            uint16_t* p = dst;
            if (g_isLowMemory != 0) {
                for (int32_t n = width; n != 0; n--) {
                    *p = table[*p >> 3] + add;
                    p++;
                }
            } else {
                for (int32_t n = width; n != 0; n--) {
                    *p = table[*p] + add;
                    p++;
                }
            }
        }
        dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch);
    }
}

// 4586fe (client: 45c32c)
void __cdecl PlotPixelBlend(int32_t x, int32_t y, int32_t r, int32_t g, int32_t b, int32_t alpha) {
    if (x < g_clipRect.left || x >= g_clipRect.right || y < g_clipRect.top || y >= g_clipRect.bottom) {
        return;
    }

    if (alpha < 0) {
        alpha = 0;
    }

    uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + y * (g_selDrawBitmap.lPitch >> 1)) * 2);
    uint16_t pixel = *dst;

    int32_t src_r = ((pixel & g_RBitMask) >> g_RBitShift) << (8 - g_RBits);
    int32_t src_g = ((pixel & g_GBitMask) >> g_GBitShift) << (8 - g_GBits);
    int32_t src_b = ((pixel & g_BBitMask) >> g_BBitShift) << (8 - g_BBits);

    int32_t new_r = (((255 - alpha) * src_r) / 255 + (r * alpha) / 255) >> (8 - g_RBits);
    int32_t new_g = (((255 - alpha) * src_g) / 255 + (g * alpha) / 255) >> (8 - g_GBits);
    int32_t new_b = (((255 - alpha) * src_b) / 255 + (b * alpha) / 255) >> (8 - g_BBits);

    *dst = (uint16_t)((new_r << g_RBitShift) | (new_g << g_GBitShift) | (new_b << g_BBitShift));
}

// 4588ec (client: 45c51a)
void __cdecl PlotGlowDot(int32_t x, int32_t y, int32_t r, int32_t g, int32_t b, int32_t alpha) {
    PlotPixelBlend(x, y, r, g, b, alpha);
    PlotPixelBlend(x + 1, y, r, g, b, alpha * 3 / 4);
    PlotPixelBlend(x, y + 1, r, g, b, alpha * 3 / 4);
    PlotPixelBlend(x - 1, y, r, g, b, alpha * 3 / 4);
    PlotPixelBlend(x, y - 1, r, g, b, alpha * 3 / 4);
    PlotPixelBlend(x + 2, y, r, g, b, alpha / 2);
    PlotPixelBlend(x, y + 2, r, g, b, alpha / 2);
    PlotPixelBlend(x - 2, y, r, g, b, alpha / 2);
    PlotPixelBlend(x, y - 2, r, g, b, alpha / 2);
    PlotPixelBlend(x + 1, y + 1, r, g, b, alpha / 4);
    PlotPixelBlend(x - 1, y + 1, r, g, b, alpha / 4);
    PlotPixelBlend(x + 1, y - 1, r, g, b, alpha / 4);
    PlotPixelBlend(x - 1, y - 1, r, g, b, alpha / 4);
}

// 457dcc (client: 45ba0c)
void __cdecl FillRectColor(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr) {
    uint16_t* sp = (uint16_t*)g_selDrawBitmap.lpSurface;

    int32_t w = abs(r - l);
    int32_t h = abs(b - t);

    int32_t bb;
    int32_t rr;
    int32_t dl;
    int32_t dd;
    int32_t dp;

    if (h < w) {
        dp = h * 2 - w;
        dl = h - w;
        rr = r;
        dd = h;
    } else {
        dp = w * 2 - h;
        dl = w - h;
        bb = b;
        dd = w;
    }

    dd *= 2;
    dl *= 2;

    int32_t dx = 1;
    if (r < l) {
        dx = -1;
    }

    int32_t dy = 1;
    if (b < t) {
        dy = -1;
    }

    int32_t xx = l;
    int32_t yy = t;

    if (l >= g_clipRect.left && l < g_clipRect.right &&
        t >= g_clipRect.top && t < g_clipRect.bottom) {
        sp[l + t * (g_selDrawBitmap.lPitch / 2)] = clr;
    }

    if (h < w) {
        while (xx != rr) {
            xx += dx;
            w = dd;
            if (dp > -1) {
                yy += dy;
                w = dl;
            }
            dp += w;

            if (xx >= g_clipRect.left && xx < g_clipRect.right &&
                yy >= g_clipRect.top && yy < g_clipRect.bottom) {
                sp[xx + yy * (g_selDrawBitmap.lPitch / 2)] = clr;
            }
        }
    } else {
        while (yy != bb) {
            yy += dy;
            w = dd;
            if (dp > -1) {
                xx += dx;
                w = dl;
            }
            dp += w;

            if (xx >= g_clipRect.left && xx < g_clipRect.right &&
                yy >= g_clipRect.top && yy < g_clipRect.bottom) {
                sp[xx + yy * (g_selDrawBitmap.lPitch / 2)] = clr;
            }
        }
    }
}

// 4579d8 (client: 45b618)
void __cdecl FillRectColorSimple(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr) {
    if (g_IsServer != 0) {
        return;
    }

    if (l < g_clipRect.left) {
        l = g_clipRect.left;
    }
    if (r > g_clipRect.right) {
        r = g_clipRect.right;
    }
    if (t < g_clipRect.top) {
        t = g_clipRect.top;
    }
    if (b > g_clipRect.bottom) {
        b = g_clipRect.bottom;
    }

    const uint32_t dcolor = (clr << 16) | clr;

    for (int32_t y = t; y < b; y++) {
        uint16_t* dout = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + l * 2);

        int32_t num = r - l;

        while (num >= 2) {
            *(uint32_t*)dout = dcolor;
            dout += 2;
            num -= 2;
        }

        if (num == 1) {
            *dout = clr;
        }
    }
}

// 457b6f (client: 45b7af)
void __cdecl ShadowRect(CRect rect, int shadow) {
    uint16_t* lookup = g_brightnessLookup + shadow * g_brightnessLookupCount;

    rect.IntersectRect(rect, g_clipRect);

    if (rect.Width() == 0 || rect.Height() == 0) {
        return;
    }

    uint16_t* px = (uint16_t*)g_selDrawBitmap.lpSurface + (g_selDrawBitmap.lPitch / 2) * rect.top + rect.left;
    for (int32_t y = rect.top; y < rect.bottom; y++) {
        uint16_t* row = px;
        for (int32_t x = rect.left; x < rect.right; x++) {
            if (g_isLowMemory) {
                *row = lookup[(*row) >> 3];
            } else {
                *row = lookup[*row];
            }
            row++;
        }
        px += g_selDrawBitmap.lPitch / 2;
    }
}


// 45424d (client: 457e8d)
void CopyIndexedSkip0(uint8_t* dst, uint8_t* src, int32_t size) {
    while (size > 0) {
        uint8_t px = *src;
        if (px != 0) {
            *dst = px;
        }
        dst++;
        src++;
        size--;
    }
}


// 458b29 (client: 45c757)
void DrawFlatTile(int32_t x, int32_t y, int32_t l1, int32_t l2, int32_t l3, int32_t l4, uint8_t* src, uint16_t* pal) {
#ifdef A2CLIENT_PATCH
    if (x + 32 <= g_clipRect.left || x >= g_clipRect.right || y >= g_clipRect.bottom || y + 32 < g_clipRect.top) {
        return;
    }
#else
    if (x < g_clipRect.left || x + 32 > g_clipRect.right || y >= g_clipRect.bottom || y + 32 < g_clipRect.top) {
        return;
    }
#endif

    int32_t tl = (l2 - l1) * 32 / 32; //top light diff
    int32_t bl = (l4 - l3) * 32 / 32; //bottom light diff

    l1 *= 32;
    l3 *= 32;
    
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    for (int32_t i = 0; i < 32; i++) {
#ifdef A2CLIENT_PATCH
        if (x + i < g_clipRect.left || x + i >= g_clipRect.right) {
            l1 += tl;
            l3 += bl;
            dst += 2;
            continue;
        }
#endif
        int32_t num_row = 32;
        int32_t lstep = (l3 - l1) / 32; // 256 / 32 lines

        if (y + 32 > g_clipRect.bottom) {
            num_row = g_clipRect.bottom - y;
        }
        
        if (num_row < 0) {
            num_row = 0;
        }

        uint8_t* srcpx = src + i;

        int32_t lght = l1 + 32 / 2;
        int32_t yy = y;
        uint8_t* out_dst = dst;

        while (num_row > 0) {
            if (yy >= g_clipRect.top) {
                const int32_t pal_idx = (lght & ~(32-1)) << 3; //256
                *(uint16_t*)out_dst = pal[pal_idx + *srcpx];
            }

            out_dst += g_selDrawBitmap.lPitch;

            lght += lstep;
            srcpx += 32;
            yy++;
            num_row--;
        }

        l1 += tl;
        l3 += bl;

        dst += 2;
    }
}


// 458ca0 (client: 45c8ce)
void DrawDeformedTile(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3, int32_t l1, int32_t l2, int32_t l3, int32_t l4, uint8_t* src, uint16_t* pal) {
#ifdef A2CLIENT_PATCH
    if (x1 <= g_clipRect.left || x0 >= g_clipRect.right ||
#else
    if (x0 < g_clipRect.left || x1 > g_clipRect.right || 
#endif
        (h0 >= g_clipRect.bottom && h1 >= g_clipRect.bottom) ||
        (h2 < g_clipRect.top && h3 < g_clipRect.top)) {
        return;
    }

    int32_t dtop = abs(h1 - h0);
    int32_t dbtm = abs(h3 - h2);
    int32_t tl = (l2 - l1) * 32 / 32;
    int32_t bl = (l4 - l3) * 32 / 32;

    l1 *= 32;
    l3 *= 32;

    int32_t tidx = 0;
    int32_t bidx = 0;

    if (h1 < h0) {
        tidx = dtop - 1;
    }

    if (h2 < h3) {
        bidx = dbtm - 1;
    }

    bool bVar2 = false;
    bool bVar3 = false;

    for (int i = 0; i < 32; i++) {
        uint8_t* srcpx = src + i;

        if (!bVar2) {
            if (h0 < h1) {
                while (g_TerrainHelper[dtop][tidx] <= i && tidx < dtop) {
                    h0++;
                    tidx++;
                }
            } else {
                while (32 - g_TerrainHelper[dtop][tidx] <= i && tidx > -1) {
                    h0--;
                    tidx--;
                }
            }

            if (tidx < 0 || tidx == dtop) {
                bVar2 = true;
            }
        }

        if (!bVar3) {
            if (h2 < h3) {
                while (32 - g_TerrainHelper[dbtm][bidx] <= i && bidx > -1) {
                    h2++;
                    bidx--;
                }
            } else {
                while (g_TerrainHelper[dbtm][bidx] <= i && bidx < dbtm) {
                    h2--;
                    bidx++;
                }
            }

            if (bidx < 0 || bidx == dbtm) {
                bVar3 = true;
            }
        }

        uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + i * 2 + x0 * 2;
        int32_t num_rows = h2 - h0;

#ifdef A2CLIENT_PATCH
        if (num_rows > 0 && x0 + i >= g_clipRect.left && x0 + i < g_clipRect.right) {
#else
        if (num_rows > 0) {
#endif
            int32_t pix_step = (32 << 16) / num_rows;
            int32_t light_step = ((l3 - l1) / num_rows) / 2;
            if (g_clipRect.bottom < h2) {
                num_rows = g_clipRect.bottom - h0;
            }
            if (num_rows < 0) {
                num_rows = 0;
            }
            int32_t pix_float = 0;
            int32_t lght = l1 + 32 / 2;
            int32_t yy = h0;

            while (num_rows > 0) {
                if (yy >= g_clipRect.top) {
                    const int32_t pal_idx = (lght & ~(32-1)) << 3; //256
                    const int32_t pixuse = (pix_float >> 16) * 32;
                    *(uint16_t*)out_dst = pal[pal_idx + *(srcpx + pixuse)];
                }

                out_dst += g_selDrawBitmap.lPitch;
                lght += light_step;
                pix_float += pix_step;
                yy++;
                num_rows--;
            }
        }

        l1 += tl;
        l3 += bl;
    }
}



#ifdef A2CLIENT_PATCH
static inline void ZeroClippedSpan16(uint8_t* row_base, int32_t sx, int32_t ex) {
    int32_t cx0 = (std::max)(sx, (int32_t)g_clipRect.left);
    int32_t cx1 = (std::min)(ex, (int32_t)g_clipRect.right);
    if (cx0 < cx1) {
        memset(row_base + cx0 * 2, 0, (size_t)(cx1 - cx0) * 2);
    }
}

static inline void GreyClippedSpan16(uint8_t* row_base, int32_t sx, int32_t ex) {
    int32_t cx0 = (std::max)(sx, (int32_t)g_clipRect.left);
    int32_t cx1 = (std::min)(ex, (int32_t)g_clipRect.right);
    if (cx0 < cx1) {
        uint16_t* pix = (uint16_t*)(row_base + cx0 * 2);
        for (int32_t n = cx1 - cx0; n > 0; n--) {
            *pix = (*pix >> 1) & g_ColorAddMask;
            pix++;
        }
    }
}
#endif

// 458fe6 (client: 45cc14)
void FillFlatTerrain(int32_t x, int32_t h0, int32_t h2) {
#ifdef A2CLIENT_PATCH
    int32_t x_start = (std::max)(x, (int32_t)g_clipRect.left);
    int32_t x_end = (std::min)(x + 32, (int32_t)g_clipRect.right);
    if (x_start >= x_end || h0 >= g_clipRect.bottom || h2 < g_clipRect.top) {
        return;
    }
#else
    if (h0 >= g_clipRect.bottom || h2 < g_clipRect.top) {
        return;
    }
#endif

    if (h0 < g_clipRect.top) {
        h0 = g_clipRect.top;
    }

    int32_t num_rows = h2 - h0;
    if (g_clipRect.bottom < h2) {
        num_rows = g_clipRect.bottom - h0;
    }

#ifdef A2CLIENT_PATCH
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x_start * 2;
    size_t span_bytes = (size_t)(x_end - x_start) * 2;
    while (num_rows > 0) {
        memset(dst, 0, span_bytes);

        dst += g_selDrawBitmap.lPitch;
        num_rows--;
    }
#else
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x * 2;
    while (num_rows > 0) {
        memset(dst, 0, 32 * 2);

        dst += g_selDrawBitmap.lPitch;
        num_rows--;
    }
#endif
}

// 4590ef (client: 45cd1d)
void FillDeformedTerrain(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3) {
#ifdef A2CLIENT_PATCH
    if (x1 <= g_clipRect.left || x0 >= g_clipRect.right ||
#else
    if (x0 < g_clipRect.left || x1 > g_clipRect.right ||
#endif
        (h0 >= g_clipRect.bottom && h1 >= g_clipRect.bottom) ||
        (h2 < g_clipRect.top && h3 < g_clipRect.top)) {
        return;
    }

    int32_t local_30 = h3;
    if (h2 < h3) {
        local_30 = h2;
    }

    int32_t local_34 = h1;
    if (h1 < h0) {
        local_34 = h0;
    }

    if (local_34 < local_30) {
        if (h0 < h1) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h1 - h0;
            int32_t idx = 0;
            while (h0 < h1) {
                if (h0 >= g_clipRect.top && h0 < g_clipRect.bottom) {
                    uint8_t num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    ZeroClippedSpan16(out_dst - x0 * 2, x0, x0 + num);
#else
                    memset(out_dst, 0, num * 2);
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h0++;
                idx++;
            }
        } else if (h1 < h0) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h1 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h0 - h1;
            int32_t idx = 0;
            while (h1 < h0) {
                if (h1 >= g_clipRect.top && h1 < g_clipRect.bottom) {
                    uint8_t num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    ZeroClippedSpan16(out_dst - x0 * 2, x0 + 32 - num, x0 + 32);
#else
                    memset(out_dst + 32 * 2 - num * 2, 0, num * 2);
#endif
                }
                h1++;
                idx++;
                out_dst += g_selDrawBitmap.lPitch;
            }
        }

        if (local_34 < local_30) {
            FillFlatTerrain(x0, local_34, local_30);
        }

        if (h3 < h2) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h3 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h2 - h3;
            int32_t idx = hid - 1;
            while (h3 < h2) {
                if (h3 >= g_clipRect.top && h3 < g_clipRect.bottom) {
                    uint8_t num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    ZeroClippedSpan16(out_dst - x0 * 2, x0, x0 + num);
#else
                    memset(out_dst, 0, num * 2);
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h3++;
                idx--;
            }
        } else if (h2 < h3) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h2 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h3 - h2;
            int32_t idx = hid - 1;
            while (h2 < h3) {
                if (h2 >= g_clipRect.top && h2 < g_clipRect.bottom) {
                    uint8_t num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    ZeroClippedSpan16(out_dst - x0 * 2, x0 + 32 - num, x0 + 32);
#else
                    memset(out_dst + 32 * 2 - num * 2, 0, num * 2);
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h2++;
                idx--;
            }
        }
    } else {
        FillDeformedLight(x0, x1, h0, h1, h2, h3, 16, 16, 16, 16);
    }
}


// 459449 (client: 45d077)
void FillFlatLight(int32_t x, int32_t y, int32_t t0, int32_t t1, int32_t t2, int32_t t3) {
#ifdef A2CLIENT_PATCH
    if (x + 32 <= g_clipRect.left || x >= g_clipRect.right || y >= g_clipRect.bottom || y + 32 < g_clipRect.top) {
        return;
    }
#else
    if (x < g_clipRect.left || x + 32 > g_clipRect.right || y >= g_clipRect.bottom || y + 32 < g_clipRect.top) {
        return;
    }
#endif

    int32_t tl = (t1 - t0) * 32 / 32; //top light diff
    int32_t bl = (t3 - t2) * 32 / 32; //bottom light diff

    t0 *= 32;
    t2 *= 32;

    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    for (int32_t i = 0; i < 32; i++) {
#ifdef A2CLIENT_PATCH
        if (x + i < g_clipRect.left || x + i >= g_clipRect.right) {
            t0 += tl;
            t2 += bl;
            dst += 2;
            continue;
        }
#endif
        int32_t num_row = 32;
        int32_t lstep = (t2 - t0) / 32;

        if (y + 32 > g_clipRect.bottom) {
            num_row = g_clipRect.bottom - y;
        }

        if (num_row < 0) {
            num_row = 0;
        }

        int32_t lght = t0 + 32 / 2; //128  half light step
        int32_t yy = y;
        uint8_t* pix = dst;

        if (g_isLowMemory) {
            while (num_row > 0) {
                if (yy >= g_clipRect.top) {
                    const uint32_t page = (lght >> 5) * g_brightnessLookupCount;
                    *(uint16_t*)pix = g_brightnessLookup[page + (*(uint16_t*)pix >> 3)];
                }

                pix += g_selDrawBitmap.lPitch;

                lght += lstep;
                yy++;
                num_row--;
            }
        } else {
            while (num_row > 0) {
                if (yy >= g_clipRect.top) {
                    const uint32_t page = (lght >> 5) * g_brightnessLookupCount;
                    *(uint16_t*)pix = g_brightnessLookup[page + *(uint16_t*)pix];
                }

                pix += g_selDrawBitmap.lPitch;

                lght += lstep;
                yy++;
                num_row--;
            }
        }
        

        t0 += tl;
        t2 += bl;

        dst += 2;
    }
}

// 4595fd (client: 45d22b)
void FillDeformedLight(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3, int32_t t0, int32_t t1, int32_t t2, int32_t t3) {

#ifdef A2CLIENT_PATCH
    if (x1 <= g_clipRect.left || x0 >= g_clipRect.right ||
#else
    if (x0 < g_clipRect.left || x1 > g_clipRect.right ||
#endif
        (h0 >= g_clipRect.bottom && h1 >= g_clipRect.bottom) ||
        (h2 < g_clipRect.top && h3 < g_clipRect.top)) {
        return;
    }

    int32_t dtop = abs(h1 - h0);
    int32_t dbtm = abs(h3 - h2);
    int32_t tl = (t1 - t0) * 32 / 32;
    int32_t bl = (t3 - t2) * 32 / 32;

    t0 *= 32;
    t2 *= 32;

    int32_t tidx = 0;
    if (h0 >= h1 && dtop > 0) {
        tidx = dtop - 1;
    }

    int32_t bidx = 0;
    if (h2 < h3 && dbtm > 0) {
        bidx = dbtm - 1;
    }

    bool bVar2 = false;
    bool bVar3 = false;
    for (int i = 0; i < 32; i++) {
        if (!bVar2) {
            if (h0 < h1) {
                while (i >= g_TerrainHelper[dtop][tidx] && tidx < dtop) {
                    h0++;
                    tidx++;
                }
            } else {
                while (i >= 32 - g_TerrainHelper[dtop][tidx] && tidx > -1) {
                    h0--;
                    tidx--;
                }
            }

            if (tidx < 0 || tidx == dtop) {
                bVar2 = true;
            }
        }
        if (!bVar3) {
            if (h2 < h3) {
                while (i >= 32 - g_TerrainHelper[dbtm][bidx] && bidx > -1) {
                    h2++;
                    bidx--;
                }
            } else {
                while (i >= g_TerrainHelper[dbtm][bidx] && bidx < dbtm) {
                    h2--;
                    bidx++;
                }
            }

            if (bidx < 0 || bidx == dbtm) {
                bVar3 = true;
            }
        }

        int32_t num_rows = h2 - h0;
#ifdef A2CLIENT_PATCH
        if (num_rows > 0 && x0 + i >= g_clipRect.left && x0 + i < g_clipRect.right) {
#else
        if (num_rows > 0) {
#endif
            int32_t light_step = (t2 - t0) / num_rows;

            uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + (x0 + i) * 2;

            if (h2 > g_clipRect.bottom) {
                num_rows = g_clipRect.bottom - h0;
            }

            if (num_rows < 0) {
                num_rows = 0;
            }

            int32_t uVar10 = t0 + 32 / 2;
            int32_t yy = h0;
            while (num_rows > 0) {
                if (yy >= g_clipRect.top) {
                    const uint32_t page = (uVar10 >> 5) * g_brightnessLookupCount;
                    if (g_isLowMemory == 1) {
                        *(uint16_t*)dst = g_brightnessLookup[page + ((*(uint16_t*)dst) >> 3)];
                    } else {
                        *(uint16_t*)dst = g_brightnessLookup[page + *(uint16_t*)dst];
                    }
                }

                uVar10 += light_step;
                yy++;
                dst += g_selDrawBitmap.lPitch;
                num_rows--;
            }
        }

        t0 += tl;
        t2 += bl;
    }
}


// 45995e (client: 45d58c)
void FillFlatGrey(int32_t x, int32_t h0, int32_t h2) {
#ifdef A2CLIENT_PATCH
    int32_t x_start = (std::max)(x, (int32_t)g_clipRect.left);
    int32_t x_end = (std::min)(x + 32, (int32_t)g_clipRect.right);
    if (x_start >= x_end || h0 >= g_clipRect.bottom || h2 < g_clipRect.top) {
        return;
    }
#else
    if (h0 >= g_clipRect.bottom || h2 < g_clipRect.top) {
        return;
    }
#endif

    if (h0 < g_clipRect.top) {
        h0 = g_clipRect.top;
    }

    int32_t num_rows = h2 - h0;
    if (g_clipRect.bottom < h2) {
        num_rows = g_clipRect.bottom - h0;
    }

#ifdef A2CLIENT_PATCH
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x_start * 2;
    int32_t span_cols = x_end - x_start;
    while (num_rows > 0) {
        uint16_t* pix = (uint16_t*)dst;
        for (int32_t num = 0; num < span_cols; num++) {
            *pix = (*pix >> 1) & g_ColorAddMask;
            pix++;
        }

        dst += g_selDrawBitmap.lPitch;
        num_rows--;
    }
#else
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x * 2;
    while (num_rows > 0) {
        uint16_t* pix = (uint16_t*)dst;
        for (int num = 0; num < 32; num++) {
            *pix = (*pix >> 1) & g_ColorAddMask;
            pix++;
        }

        dst += g_selDrawBitmap.lPitch;
        num_rows--;
    }
#endif
}

// 459b72 (client: 45d7a0)
void FillDeformedGrey(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3) {
#ifdef A2CLIENT_PATCH
    if (x1 <= g_clipRect.left || x0 >= g_clipRect.right ||
#else
    if (x0 < g_clipRect.left || x1 > g_clipRect.right ||
#endif
        (h0 >= g_clipRect.bottom && h1 >= g_clipRect.bottom) ||
        (h2 < g_clipRect.top && h3 < g_clipRect.top)) {
        return;
    }

    int32_t h_bottom = h3;
    if (h2 < h3) {
        h_bottom = h2;
    }

    int32_t h_top = h1;
    if (h1 < h0) {
        h_top = h0;
    }

    if (h_top < h_bottom) {
        if (h0 < h1) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h0 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h1 - h0;
            int32_t idx = 0;
            while (h0 < h1) {
                if (h0 >= g_clipRect.top && h0 < g_clipRect.bottom) {
#ifdef A2CLIENT_PATCH
                    GreyClippedSpan16(out_dst - x0 * 2, x0, x0 + g_TerrainHelper[hid][idx]);
#else
                    uint16_t* pix = (uint16_t*)out_dst;
                    for (int num = g_TerrainHelper[hid][idx]; num > 0; num--) {
                        *pix = (*pix >> 1) & g_ColorAddMask;
                        pix++;
                    }
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h0++;
                idx++;
            }
        } else if (h1 < h0) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h1 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h0 - h1;
            int32_t idx = 0;
            while (h1 < h0) {
                if (h1 >= g_clipRect.top && h1 < g_clipRect.bottom) {
                    int num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    GreyClippedSpan16(out_dst - x0 * 2, x0 + 32 - num, x0 + 32);
#else
                    uint16_t* pix = (uint16_t*)(out_dst + 32 * 2 - num * 2);
                    for (; num > 0; num--) {
                        *pix = (*pix >> 1) & g_ColorAddMask;
                        pix++;
                    }
#endif
                }
                h1++;
                idx++;
                out_dst += g_selDrawBitmap.lPitch;
            }
        }

        if (h_top < h_bottom) {
            FillFlatGrey(x0, h_top, h_bottom);
        }

        if (h3 < h2) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h3 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h2 - h3;
            int32_t idx = hid - 1;
            while (h3 < h2) {
                if (h3 >= g_clipRect.top && h3 < g_clipRect.bottom) {
#ifdef A2CLIENT_PATCH
                    GreyClippedSpan16(out_dst - x0 * 2, x0, x0 + g_TerrainHelper[hid][idx]);
#else
                    uint16_t* pix = (uint16_t*)out_dst;
                    for (int num = g_TerrainHelper[hid][idx]; num > 0; num--) {
                        *pix = (*pix >> 1) & g_ColorAddMask;
                        pix++;
                    }
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h3++;
                idx--;
            }
        } else if (h2 < h3) {
            uint8_t* out_dst = (uint8_t*)g_selDrawBitmap.lpSurface + h2 * g_selDrawBitmap.lPitch + x0 * 2;
            const int32_t hid = h3 - h2;
            int32_t idx = hid - 1;
            while (h2 < h3) {
                if (h2 >= g_clipRect.top && h2 < g_clipRect.bottom) {
                    int num = g_TerrainHelper[hid][idx];
#ifdef A2CLIENT_PATCH
                    GreyClippedSpan16(out_dst - x0 * 2, x0 + 32 - num, x0 + 32);
#else
                    uint16_t* pix = (uint16_t*)(out_dst + 32 * 2 - num * 2);
                    for (; num > 0; num--) {
                        *pix = (*pix >> 1) & g_ColorAddMask;
                        pix++;
                    }
#endif
                }
                out_dst += g_selDrawBitmap.lPitch;
                h2++;
                idx--;
            }
        }
    } else {
        FillDeformedLight(x0, x1, h0, h1, h2, h3, 8, 8, 8, 8);
    }
}


// 458035 (client: 45bc73)
void __cdecl DrawRectangleFrame(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr) {
    FillRectColor(l, t, r, t, clr); //up
    FillRectColor(l, b, r, b, clr); //down
    FillRectColor(r, t, r, b, clr); //right
    FillRectColor(l, t, l, b, clr); //left
}

// 454a54 (client: 458694)
void __cdecl CopySurface16Rect(uint8_t* src, int32_t sx, int32_t sy, uint8_t* dst, int32_t dx, int32_t dy, int32_t w, int32_t h, int32_t sp, int32_t dp) {
    int32_t t = g_clipRect.left - dx;
    if (t > 0) {
        sx += t;
        dx += t;
        w -= t;
    }
    t = (dx + w) - g_clipRect.right;
    if (t > -1) {
        w -= t;
    }

    t = g_clipRect.top - dy;
    if (t > 0) {
        sy += t;
        dy += t;
        h -= t;
    }

    t = (dy + h) - g_clipRect.bottom;
    if (t > -1) {
        h -= t;
    }

    if (w <= 0 || h <= 0) {
        return;
    }

    uint8_t* s = src + sy * sp + sx * 2;
    uint8_t* d = dst + dy * dp + dx * 2;

    for (; h; h--) {
        memcpy(d, s, w * 2);
        s += sp;
        d += dp;
    }
}

// 454e40 (client: 458a80)
void __cdecl CopyScreenBlock(int32_t sx, int32_t sy, int32_t w, int32_t h) {
    LockTerrainSurface();
    uint8_t* src = (uint8_t*)g_selDrawBitmap.lpSurface;
    int32_t spitch = g_selDrawBitmap.lPitch;
    LockBackSurface();
    uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface;

    int32_t sx32 = sx << 5;
    int32_t sy32 = sy << 5;

    CopySurface16Rect(src, sx32, sy32, dst, 0, 0, w - sx32, h - sy32, spitch, g_selDrawBitmap.lPitch);
    CopySurface16Rect(src, 0, 0, dst, w - sx32, h - sy32, sx32, sy32, spitch, g_selDrawBitmap.lPitch);
    CopySurface16Rect(src, sx32, 0, dst, 0, h - sy32, w - sx32, sy32, spitch, g_selDrawBitmap.lPitch);
    CopySurface16Rect(src, 0, sy32, dst, w - sx32, 0, sx32, h - sy32, spitch, g_selDrawBitmap.lPitch);

    UnlockBackSurface();
    g_selDrawBitmap.lpSurface = src;
    UnlockTerrainSurface();
}

#ifdef A2CLIENT_PATCH
static CRect GetEffectiveDisplaySize() {
    if (g_DisplaySize.right > 0 && g_DisplaySize.bottom > 0) {
        return g_DisplaySize;
    }
    return g_ScreenSize;
}

CPoint PhysicalToLogicalPoint(POINT pt) {
    CRect disp = GetEffectiveDisplaySize();
    int32_t disp_w = disp.right;
    int32_t disp_h = disp.bottom;
    int32_t scr_w = g_ScreenSize.right;
    int32_t scr_h = g_ScreenSize.bottom;
    if (disp_w <= 1 || disp_h <= 1 || scr_w <= 1 || scr_h <= 1 ||
        (disp_w == scr_w && disp_h == scr_h)) {
        return CPoint(pt.x, pt.y);
    }

    int32_t px = (std::max)(0L, (std::min)(pt.x, static_cast<LONG>(disp_w - 1)));
    int32_t py = (std::max)(0L, (std::min)(pt.y, static_cast<LONG>(disp_h - 1)));
    int32_t lx = static_cast<int32_t>((static_cast<int64_t>(px) * (scr_w - 1) + (disp_w - 1) / 2) / (disp_w - 1));
    int32_t ly = static_cast<int32_t>((static_cast<int64_t>(py) * (scr_h - 1) + (disp_h - 1) / 2) / (disp_h - 1));
    return CPoint(lx, ly);
}

CPoint LogicalToPhysicalPoint(int32_t x, int32_t y) {
    CRect disp = GetEffectiveDisplaySize();
    int32_t disp_w = disp.right;
    int32_t disp_h = disp.bottom;
    int32_t scr_w = g_ScreenSize.right;
    int32_t scr_h = g_ScreenSize.bottom;
    if (disp_w <= 1 || disp_h <= 1 || scr_w <= 1 || scr_h <= 1 ||
        (disp_w == scr_w && disp_h == scr_h)) {
        return CPoint(x, y);
    }

    int32_t lx = (std::max)(0, (std::min)(x, scr_w - 1));
    int32_t ly = (std::max)(0, (std::min)(y, scr_h - 1));
    int32_t px = static_cast<int32_t>((static_cast<int64_t>(lx) * (disp_w - 1) + (scr_w - 1) / 2) / (scr_w - 1));
    int32_t py = static_cast<int32_t>((static_cast<int64_t>(ly) * (disp_h - 1) + (scr_h - 1) / 2) / (scr_h - 1));
    return CPoint(px, py);
}

CRect LogicalToPhysicalRect(const CRect& rect) {
    CRect disp = GetEffectiveDisplaySize();
    int32_t disp_w = disp.right;
    int32_t disp_h = disp.bottom;
    int32_t scr_w = g_ScreenSize.right;
    int32_t scr_h = g_ScreenSize.bottom;
    if (disp_w <= 0 || disp_h <= 0 || scr_w <= 0 || scr_h <= 0 ||
        (disp_w == scr_w && disp_h == scr_h)) {
        return rect;
    }

    int32_t pl = static_cast<int32_t>((static_cast<int64_t>(rect.left) * disp_w) / scr_w);
    int32_t pt = static_cast<int32_t>((static_cast<int64_t>(rect.top) * disp_h) / scr_h);
    int32_t pr = static_cast<int32_t>((static_cast<int64_t>(rect.right) * disp_w + scr_w - 1) / scr_w);
    int32_t pb = static_cast<int32_t>((static_cast<int64_t>(rect.bottom) * disp_h + scr_h - 1) / scr_h);
    return CRect(pl, pt, pr, pb);
}

CRect PhysicalToLogicalRect(const CRect& rect) {
    CRect disp = GetEffectiveDisplaySize();
    int32_t disp_w = disp.right;
    int32_t disp_h = disp.bottom;
    int32_t scr_w = g_ScreenSize.right;
    int32_t scr_h = g_ScreenSize.bottom;
    if (disp_w <= 0 || disp_h <= 0 || scr_w <= 0 || scr_h <= 0 ||
        (disp_w == scr_w && disp_h == scr_h)) {
        return rect;
    }

    int32_t ll = static_cast<int32_t>((static_cast<int64_t>(rect.left) * scr_w) / disp_w) - 2;
    int32_t lt = static_cast<int32_t>((static_cast<int64_t>(rect.top) * scr_h) / disp_h) - 2;
    int32_t lr = static_cast<int32_t>((static_cast<int64_t>(rect.right) * scr_w + disp_w - 1) / disp_w) + 2;
    int32_t lb = static_cast<int32_t>((static_cast<int64_t>(rect.bottom) * scr_h + disp_h - 1) / disp_h) + 2;
    CRect out((std::max)(0, ll), (std::max)(0, lt), (std::min)(scr_w, lr), (std::min)(scr_h, lb));
    return out;
}

BOOL GetLogicalCursorPos(LPPOINT lpPoint) {
    if (!GetCursorPos(lpPoint)) {
        return FALSE;
    }
    if (g_IsServer == 0) {
        CWnd* mwnd = AfxGetMainWnd();
        if (mwnd != nullptr && mwnd->GetSafeHwnd() != nullptr && !mwnd->IsIconic()) {
            CRect wnd_rc;
            mwnd->GetWindowRect(&wnd_rc);
            lpPoint->x -= wnd_rc.left;
            lpPoint->y -= wnd_rc.top;
        }
        CPoint lpt = PhysicalToLogicalPoint(*lpPoint);
        lpPoint->x = lpt.x;
        lpPoint->y = lpt.y;
    }
    return TRUE;
}

BOOL SetLogicalCursorPos(int32_t x, int32_t y) {
    if (g_IsServer == 0) {
        CPoint ppt = LogicalToPhysicalPoint(x, y);
        CWnd* mwnd = AfxGetMainWnd();
        if (mwnd != nullptr && mwnd->GetSafeHwnd() != nullptr && !mwnd->IsIconic()) {
            CRect wnd_rc;
            mwnd->GetWindowRect(&wnd_rc);
            ppt.x += wnd_rc.left;
            ppt.y += wnd_rc.top;
        }
        return SetCursorPos(ppt.x, ppt.y);
    }
    return SetCursorPos(x, y);
}

static void SampleRowBilinear32(const uint16_t* srow, int32_t dst_left, int32_t width, uint32_t* out_row) {
    for (int32_t x = 0; x < width; x++) {
        int32_t dx = dst_left + x;
        uint32_t c0 = g_rgb565_to_32[srow[g_scale_sx0[dx]]];
        uint32_t wx = g_scale_wx[dx];
        if (wx == 0) {
            out_row[x] = c0;
            continue;
        }
        uint32_t c1 = g_rgb565_to_32[srow[g_scale_sx1[dx]]];
        if (c0 == c1) {
            out_row[x] = c0;
            continue;
        }
        uint32_t inv_wx = 256 - wx;
        uint32_t rb = (((c0 & 0x00FF00FFu) * inv_wx + (c1 & 0x00FF00FFu) * wx) >> 8) & 0x00FF00FFu;
        uint32_t g = (((c0 & 0x0000FF00u) * inv_wx + (c1 & 0x0000FF00u) * wx) >> 8) & 0x0000FF00u;
        out_row[x] = rb | g;
    }
}

void PresentPrimarySurfaceRect(const CRect& rect) {
    if (g_IsServer != 0 || g_ddsurface == nullptr || g_surf1_buf == nullptr) {
        return;
    }

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd == nullptr || mwnd->has_focus == 0) {
        return;
    }

    CRect r;
    if (!r.IntersectRect(&rect, &g_ScreenSize)) {
        return;
    }

    const int32_t src_w = g_ScreenSize.right;
    const int32_t src_h = g_ScreenSize.bottom;
    const CRect disp_rect = GetEffectiveDisplaySize();
    const int32_t dst_w = disp_rect.right;
    const int32_t dst_h = disp_rect.bottom;

    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        return;
    }

    // Unscaled 1:1 path (e.g. -window mode or matching native resolution).
    if (dst_w == src_w && dst_h == src_h) {
        DDSURFACEDESC ddsd;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC));
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        RECT lock_rc = r;
        HRESULT hr = g_ddsurface->Lock(&lock_rc, &ddsd, DDLOCK_WAIT, nullptr);
        if (hr == DDERR_SURFACELOST) {
            if (g_is_fullscreen != 0) {
                g_ddraw->SetCooperativeLevel(mwnd->m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
                g_ddraw->SetDisplayMode(dst_w, dst_h, gfx_bits);
            }
            if (FAILED(g_ddsurface->Restore())) {
                return;
            }
            r = g_ScreenSize;
            lock_rc = r;
            memset(&ddsd, 0, sizeof(DDSURFACEDESC));
            ddsd.dwSize = sizeof(DDSURFACEDESC);
            hr = g_ddsurface->Lock(&lock_rc, &ddsd, DDLOCK_WAIT, nullptr);
        }
        if (FAILED(hr) || ddsd.lpSurface == nullptr) {
            return;
        }

        const int32_t w = r.Width();
        const int32_t h = r.Height();
        const int32_t src_stride = src_w;
        const uint16_t* src_row = g_surf1_buf + r.top * src_stride + r.left;
        uint8_t* dst_row = (uint8_t*)ddsd.lpSurface;

        if (g_display_bpp == 32) {
            for (int32_t y = 0; y < h; y++) {
                uint32_t* dst32 = (uint32_t*)dst_row;
                for (int32_t x = 0; x < w; x++) {
                    dst32[x] = g_rgb565_to_32[src_row[x]];
                }
                src_row += src_stride;
                dst_row += ddsd.lPitch;
            }
        } else if (g_display_is_rgb565 != 0) {
            for (int32_t y = 0; y < h; y++) {
                memcpy(dst_row, src_row, (size_t)w * 2);
                src_row += src_stride;
                dst_row += ddsd.lPitch;
            }
        } else {
            for (int32_t y = 0; y < h; y++) {
                uint16_t* dst16 = (uint16_t*)dst_row;
                for (int32_t x = 0; x < w; x++) {
                    dst16[x] = g_rgb565_to_16[src_row[x]];
                }
                src_row += src_stride;
                dst_row += ddsd.lPitch;
            }
        }

        g_ddsurface->Unlock(ddsd.lpSurface);
        return;
    }

    // Scaled stretch path (logical g_ScreenSize -> physical g_DisplaySize).
    EnsureScaleTables(src_w, src_h, dst_w, dst_h);

    const int32_t pad_x = dst_w / src_w + 2;
    const int32_t pad_y = dst_h / src_h + 2;
    int32_t dl = static_cast<int32_t>((static_cast<int64_t>(r.left) * dst_w) / src_w) - pad_x;
    int32_t dt = static_cast<int32_t>((static_cast<int64_t>(r.top) * dst_h) / src_h) - pad_y;
    int32_t dr = static_cast<int32_t>((static_cast<int64_t>(r.right) * dst_w + src_w - 1) / src_w) + pad_x;
    int32_t db = static_cast<int32_t>((static_cast<int64_t>(r.bottom) * dst_h + src_h - 1) / src_h) + pad_y;
    CRect dst_r((std::max)(0, dl), (std::max)(0, dt), (std::min)(dst_w, dr), (std::min)(dst_h, db));
    if (dst_r.IsRectEmpty()) {
        return;
    }

    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    RECT lock_rc = dst_r;
    HRESULT hr = g_ddsurface->Lock(&lock_rc, &ddsd, DDLOCK_WAIT, nullptr);
    if (hr == DDERR_SURFACELOST) {
        if (g_is_fullscreen != 0) {
            g_ddraw->SetCooperativeLevel(mwnd->m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
            g_ddraw->SetDisplayMode(dst_w, dst_h, gfx_bits);
        }
        if (FAILED(g_ddsurface->Restore())) {
            return;
        }
        dst_r = disp_rect;
        lock_rc = dst_r;
        memset(&ddsd, 0, sizeof(DDSURFACEDESC));
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        hr = g_ddsurface->Lock(&lock_rc, &ddsd, DDLOCK_WAIT, nullptr);
    }
    if (FAILED(hr) || ddsd.lpSurface == nullptr) {
        return;
    }

    const int32_t w = dst_r.Width();
    const int32_t h = dst_r.Height();
    uint8_t* dst_row = (uint8_t*)ddsd.lpSurface;

    if (g_display_bpp == 32) {
        uint32_t* hrow0 = g_scale_row0;
        uint32_t* hrow1 = g_scale_row1;
        int32_t cached_sy0 = -1;
        int32_t cached_sy1 = -1;

        for (int32_t y = 0; y < h; y++) {
            int32_t dy = dst_r.top + y;
            int32_t sy0 = g_scale_sy0[dy];
            int32_t sy1 = g_scale_sy1[dy];
            uint32_t wy = g_scale_wy[dy];

            if (sy0 != cached_sy0) {
                if (sy0 == cached_sy1) {
                    std::swap(hrow0, hrow1);
                    cached_sy1 = cached_sy0;
                    cached_sy0 = sy0;
                } else {
                    SampleRowBilinear32(g_surf1_buf + sy0 * src_w, dst_r.left, w, hrow0);
                    cached_sy0 = sy0;
                }
            }

            uint32_t* dst32 = (uint32_t*)dst_row;
            if (wy == 0 || sy0 == sy1) {
                memcpy(dst32, hrow0, static_cast<size_t>(w) * sizeof(uint32_t));
            } else {
                if (sy1 != cached_sy1) {
                    SampleRowBilinear32(g_surf1_buf + sy1 * src_w, dst_r.left, w, hrow1);
                    cached_sy1 = sy1;
                }
                uint32_t inv_wy = 256 - wy;
                for (int32_t x = 0; x < w; x++) {
                    uint32_t r0 = hrow0[x];
                    uint32_t r1 = hrow1[x];
                    uint32_t rb = (((r0 & 0x00FF00FFu) * inv_wy + (r1 & 0x00FF00FFu) * wy) >> 8) & 0x00FF00FFu;
                    uint32_t g = (((r0 & 0x0000FF00u) * inv_wy + (r1 & 0x0000FF00u) * wy) >> 8) & 0x0000FF00u;
                    dst32[x] = rb | g;
                }
            }

            dst_row += ddsd.lPitch;
        }
    }

    g_ddsurface->Unlock(ddsd.lpSurface);
}
#endif

// 454c74 (client: 4588b4)
void __cdecl gfxFlushRect(const CRect& rect) {
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

#ifdef A2CLIENT_PATCH
    if (mwnd != nullptr && mwnd->has_focus != 0 && g_IsServer == 0 && g_surf1_buf != nullptr && g_surf2_buf != nullptr) {
        CRect r;
        r.IntersectRect(&rect, &g_ScreenSize);

        CRect ver_rect(0, 0, 0, 0);
        bool stamped_ver = false;
        static std::vector<uint16_t> ver_backup;
        if (g_font2 != nullptr && g_BuildVersion != nullptr && g_BuildVersion[0] != '\0') {
            const int32_t ver_x = (std::max)(4, (int32_t)g_ScreenSize.right - 296);
            const int32_t ver_y = 2;
            const int32_t ver_w = g_font2->GetStrWidth(g_BuildVersion) + 6;
            const int32_t ver_h = g_font2->GetHeight() + 6;
            CRect candidate(ver_x, ver_y, ver_x + ver_w, ver_y + ver_h);
            ver_rect.IntersectRect(&candidate, &g_ScreenSize);
            CRect hit;
            if (!ver_rect.IsRectEmpty() && hit.IntersectRect(&r, &ver_rect)) {
                const int32_t vw = ver_rect.Width();
                const int32_t vh = ver_rect.Height();
                const int32_t stride = g_ScreenSize.right;
                ver_backup.resize(vw * vh);
                for (int32_t y = 0; y < vh; y++) {
                    memcpy(&ver_backup[y * vw], g_surf2_buf + (ver_rect.top + y) * stride + ver_rect.left, vw * sizeof(uint16_t));
                }

                LockBackSurface();
                DDSURFACEDESC prev_desc = g_selDrawBitmap;
                g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
                g_selDrawBitmap.lpSurface = g_surf2_buf;
                g_selDrawBitmap.dwWidth = g_ScreenSize.right;
                g_selDrawBitmap.dwHeight = g_ScreenSize.bottom + 1;
                g_selDrawBitmap.lPitch = g_ScreenSize.right * 2;

                CRect prev_clip;
                GetClipRect(&prev_clip);
                SetClipRect(ver_rect);
                g_font2->DrawTextWithShadow(ver_x, ver_y, g_BuildVersion, CGameFont::ALIGN_LEFT_TOP, clrsh_DullGold, 1);
                SetClipRect(prev_clip);

                g_selDrawBitmap = prev_desc;
                UnlockBackSurface();
                stamped_ver = true;
            }
        }

        g_mousept.PrepareFlushOverlays(r);
        int32_t w = r.Width();
        int32_t h = r.Height();

        if (w < 1 || h < 1) {
            g_mousept.RestoreAfterFlush(r);
        } else {
            LockBackSurface();

            void* sv = g_selDrawBitmap.lpSurface;

            uint8_t* src = (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2;
            int32_t spitch = g_selDrawBitmap.lPitch;

            LockPrimarySurface();

            CopySurface16Rect(src, 0, 0, 
                             (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2, 0, 0, 
                             w, h, 
                             spitch, 
                             g_selDrawBitmap.lPitch);

            UnlockPrimarySurface();
            PresentPrimarySurfaceRect(r);

            g_selDrawBitmap.lpSurface = sv;

            UnlockBackSurface();
            g_mousept.RestoreAfterFlush(r);
        }

        if (stamped_ver) {
            const int32_t vw = ver_rect.Width();
            const int32_t vh = ver_rect.Height();
            const int32_t stride = g_ScreenSize.right;
            for (int32_t y = 0; y < vh; y++) {
                memcpy(g_surf2_buf + (ver_rect.top + y) * stride + ver_rect.left,
                       &ver_backup[static_cast<size_t>(y) * vw],
                       static_cast<size_t>(vw) * sizeof(uint16_t));
            }
        }
    }
#else
    if (mwnd->has_focus != 0 && g_IsServer == 0) {
        CRect r;
        r.IntersectRect(&rect, &g_ScreenSize);

        while (g_ddsurface->IsLost() == DDERR_SURFACELOST) {
            g_ddraw->SetCooperativeLevel(mwnd->m_hWnd, DDSCL_EXCLUSIVE || DDSCL_FULLSCREEN);
            g_ddraw->SetDisplayMode(g_ScreenSize.right, g_ScreenSize.bottom, gfx_bits);
            g_ddsurface->Restore();
        }

        g_mousept.PrepareFlushOverlays(r);
        int32_t w = r.Width();
        int32_t h = r.Height();

        if (w < 1 || h < 1) {
            g_mousept.RestoreAfterFlush(r);
        } else {
            LockBackSurface();

            void* sv = g_selDrawBitmap.lpSurface;

            uint8_t* src = (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2;
            int32_t spitch = g_selDrawBitmap.lPitch;

            LockPrimarySurface();

            CopySurface16Rect(src, 0, 0, 
                             (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2, 0, 0, 
                             w, h, 
                             spitch, 
                             g_selDrawBitmap.lPitch);

            UnlockPrimarySurface();

            g_selDrawBitmap.lpSurface = sv;

            UnlockBackSurface();
            g_mousept.RestoreAfterFlush(r);
        }
    }
#endif
}

// 454e18 (client: 458a58)
void __cdecl FlushScreen() {
    gfxFlushRect(CRect(g_ScreenSize));
}



// 45431c (client: 457f5c)
uint32_t LockPrimarySurface() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer || g_surf1_buf == nullptr) {
        return 0;
    }

    if (g_ddsurf_lock == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        g_selDrawBitmap.lpSurface = g_surf1_buf;
        g_selDrawBitmap.dwWidth = g_ScreenSize.right;
        g_selDrawBitmap.dwHeight = g_ScreenSize.bottom + 1;
        g_selDrawBitmap.lPitch = g_ScreenSize.right * 2;
    }
    g_ddsurf_lock++;
    return 1;
#else
    if (g_IsServer) {
        return 0;
    }

    int res = 0;
    if (g_ddsurf_lock == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        res = SUCCEEDED( g_ddsurface->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr) );
        g_ddsurf_lock++;
    }
    return res;
#endif
}


// 454378 (client: 457fb8)
uint32_t UnlockPrimarySurface() {
    if (g_IsServer) {
        return 0;
    }

#ifdef A2CLIENT_PATCH
    uint32_t res = 0;
    if (g_ddsurf_lock != 0) {
        // imp_MakeScreenshotNet (client: 457ff4): capture any armed HAT
        // screenshot request while the primary draw surface is still mapped.
        tangar_client::ProcLastScreenshot();
        g_ddsurf_lock--;
        res = 1;
    }
    return res;
#else
    int res = 0;
    if (g_ddsurf_lock != 0) {
        res = SUCCEEDED(g_ddsurface->Unlock(g_selDrawBitmap.lpSurface));
        g_ddsurf_lock--;
    }
    return res;
#endif
}


// 45426e (client: 457eae)
uint32_t LockBackSurface() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer || g_surf2_buf == nullptr) {
        return 0;
    }

    if (g_ddsurf_lock2 == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        g_selDrawBitmap.lpSurface = g_surf2_buf;
        g_selDrawBitmap.dwWidth = g_ScreenSize.right;
        g_selDrawBitmap.dwHeight = g_ScreenSize.bottom + 1;
        g_selDrawBitmap.lPitch = g_ScreenSize.right * 2;
    }
    g_ddsurf_lock2++;
    return 1;
#else
    if (g_IsServer) {
        return 0;
    }

    int res = 0;
    if (g_ddsurf_lock2 == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        res = SUCCEEDED(g_ddsurface2->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr));
        g_ddsurf_lock2++;
    }
    return res;
#endif
}

// 4542ca (client: 457f0a)
uint32_t UnlockBackSurface() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer) {
        return 0;
    }

    uint32_t res = 0;
    if (g_ddsurf_lock2 != 0) {
        g_ddsurf_lock2--;
        res = 1;
    }
    return res;
#else
    if (g_IsServer) {
        return 0;
    }

    int res = 0;
    if (g_ddsurf_lock2 != 0) {
        res = SUCCEEDED(g_ddsurface2->Unlock(g_selDrawBitmap.lpSurface));
        g_ddsurf_lock2--;
    }
    return res;
#endif
}

// 4543ca (client: 45800a)
uint32_t LockTerrainSurface() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer || g_surf3_buf == nullptr) {
        return 0;
    }

    if (g_ddsurf_lock3 == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        g_selDrawBitmap.lpSurface = g_surf3_buf;
        g_selDrawBitmap.dwWidth = g_surf3_w;
        g_selDrawBitmap.dwHeight = g_surf3_h;
        g_selDrawBitmap.lPitch = g_surf3_w * 2;
    }
    g_ddsurf_lock3++;
    return 1;
#else
    if (g_IsServer) {
        return 0;
    }

    if (g_ddsurface3->IsLost() == DDERR_SURFACELOST) {
        g_ddsurface3->Restore();
    }

    int res = 0;
    if (g_ddsurf_lock3 == 0) {
        g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
        res = SUCCEEDED(g_ddsurface3->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr));
        g_ddsurf_lock3++;
    }
    return res;
#endif
}


// 45444f (client: 45808f)
uint32_t UnlockTerrainSurface() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer) {
        return 0;
    }

    uint32_t res = 0;
    if (g_ddsurf_lock3 != 0) {
        g_ddsurf_lock3--;
        res = 1;
    }
    return res;
#else
    if (g_IsServer) {
        return 0;
    }

    if (g_ddsurface3->IsLost() == DDERR_SURFACELOST) {
        g_ddsurface3->Restore();
    }

    int res = 0;
    if (g_ddsurf_lock3 != 0) {
        res = SUCCEEDED(g_ddsurface3->Unlock(g_selDrawBitmap.lpSurface));
        g_ddsurf_lock3--;
    }
    return res;
#endif
}

// 4538e1 (client: 457521)
uint32_t GetBackSurfaceLockCount() { return g_ddsurf_lock2; }

// 45455e (client: 45819e)
void __cdecl CopyScreenRectToBmp64(int32_t x, int32_t y, int32_t w, int32_t h, int32_t dw, int32_t dh, uint8_t* dst) {
    int32_t dy = 0;
    int32_t dx = 0;
    if (x < g_clipRect.left) {
        w -= g_clipRect.left - x;
        dx = g_clipRect.left - x;
        x = g_clipRect.left;
    }
    if (y < g_clipRect.top) {
        h -= g_clipRect.top - y;
        dy = g_clipRect.top - y;
        y = g_clipRect.top;
    }
    if (g_clipRect.right <= x + w) {
        w = g_clipRect.right - x;
    }
    if (g_clipRect.bottom <= y + h) {
        h = g_clipRect.bottom - y;
    }

#ifdef A2CLIENT_PATCH
    if (dst == nullptr || g_selDrawBitmap.lpSurface == nullptr || dw <= 0 || dh <= 0 || dx >= dw || dy >= dh) {
        return;
    }
    if (dx + w > dw) {
        w = dw - dx;
    }
    if (dy + h > dh) {
        h = dh - dy;
    }
#endif

    if (w <= 0 || h <= 0) {
        return;
    }

    uint8_t* s = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
    uint8_t* d = (uint8_t*)dst + dx * 2 + ((dh - dy) - 1) * dw * 2;
    for (int32_t yy = y; yy < y + h; yy++) {
        memcpy(d, s, w * 2);
        s += g_selDrawBitmap.lPitch;
        d -= dw * 2;
    }
}

// 453f5c (client: 457b9c)
int BitmaskLowBound(uint32_t bm) {
    for (int32_t i = 0; i < 32; i++) {
        if (bm & (1u << i)) {
            return i;
        }
    }
    return -1;
}

// 453f94 (client: 457bd4)
int BitmaskHighBound(uint32_t bm) {
    int32_t max = 15;
#ifdef A2CLIENT_PATCH
    max = 31;
#endif

    for (int32_t i = max; i >= 0; i--) {
        if (bm & (1u << i)) {
            return i;
        }
    }
    return -1;
}

// 453cf9 (client: 457939)
void FreeBrightnessLookup() {
    if (g_brightnessLookup) {
        delete[] g_brightnessLookup;
    }
    g_brightnessLookup = nullptr;
}

// 4538eb (client: 45752b)
void CalculateBrightness() {
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    
    g_isLowMemory = ms.dwTotalPhys < 24 * 1000 * 1000;

    int32_t maxR = 1 << g_RBits;
    int32_t maxG = 1 << g_GBits;
    int32_t maxB = 1 << g_BBits;

    if (g_isLowMemory == 0) {
        g_brightnessLookupCount = 0x10000;
    } else {
        g_brightnessLookupCount = 0x10000 >> 3;
    }

    g_brightnessLookup = new uint16_t[g_brightnessLookupCount * 17];

    int db = 1;
    int sb = 0;
    if (g_isLowMemory) {
        db = 1 << 3;
        sb = (1 << 3) / 2;
    }

    for (int32_t br = 16; br >= 0; br--) {
        int32_t off = g_brightnessLookupCount * (16 - br);
        for (int32_t ri = 0; ri < maxR; ri++) {
            for (int32_t gi = 0; gi < maxG; gi++) {
                for (int32_t bi = sb; bi < maxB; bi += db) {
                    int32_t r = (ri * br / 16) << (8 - g_RBits);
                    int32_t g = (gi * br / 16) << (8 - g_GBits);
                    int32_t b = (bi * br / 16) << (8 - g_BBits);
                    if (r > 255) {
                        r = 255;
                    }
                    if (g > 255) {
                        g = 255;
                    }
                    if (b > 255) {
                        b = 255;
                    }
                    int32_t idx = (ri << g_RBitShift) | (gi << g_GBitShift) | (bi << g_BBitShift);
                    if (g_isLowMemory) {
                        idx >>= 3;
                    }
                    g_brightnessLookup[off + idx] = GetColorRGB(r, g, b);
                }
            }
        }
    }

    for (int32_t i = 1; i < 128; i++) {
        int32_t val = 0;
        for (int32_t j = 0; j < i; j++) {
            val += 32 * 0x10000 / (i + 1);
            g_TerrainHelper[i][j] = (val + 0x8000) >> 16; //  (val + 0x8000) / 0x10000    --- 0.5 round to ceil
        }
    }

    g_ColorAddMask = GetColorRGB(0x7f, 0x7f, 0x7f);
}

// 46011e (client: 463d30)
void CalcColorsAndPalettes() {
    for (int32_t i = 0; i < 0x10; i++) {
        clrsh_Black[i] = GetColorRGB(8, 8, 8);
        clrsh_ShockingBlack[i] = GetColorRGB(i * 0x11, i * 0x11, i * 0x11);
        clrsh_TechBlack[i] = GetColorRGB(i * 0xe, i * 0xe, i * 0xe);
        clrsh_DullGold[i] = GetColorRGB((i * 0xb9) / 0xf, (i * 0x9f) / 0xf, (i * 0x49) / 0xf);
        clrsh_Oxley[i] = GetColorRGB((i * 0x6b) / 0xf, (i * 0x9a) / 0xf, (i * 0x78) / 0xf);
        clrsh_GoldenFizz[i] = GetColorRGB((i * 0xff) / 0xf, (i * 0xff) / 0xf, (i * 0x40) / 0xf);
        clrsh_CoralRed[i] = GetColorRGB((i * 0xff) / 0xf, (i * 0x40) / 0xf, (i * 0x40) / 0xf);
        clrsh_ScreaminGreen[i] = GetColorRGB((i * 0x40) / 0xf, (i * 0xff) / 0xf, (i * 0x40) / 0xf);
        clrsh_LuxorGold[i] = GetColorRGB((i * 0xa0) / 0xf, (i * 0x78) / 0xf, (i * 0x32) / 0xf);
        clrsh_Cumin[i] = GetColorRGB((i * 0x8b) / 0xf, (i * 0x41) / 0xf, (i * 0x20) / 0xf);
        clrsh_CharlieBrown[i] = GetColorRGB((i * 0x96) / 0xf, (i * 0x5a) / 0xf, 0);
        clrsh_CreteToRed[i] = GetColorRGB(0x8d - (i * 0x3d) / 0xf, (i * 0x7e) / 0xf, (i * 0x31) / 0xf);
        clrsh_ScreamGreenToRed[i] = GetColorRGB(0xff - (i * 0xaf) / 0xf, (i * 0xfc) / 0xf, (i * 0x62) / 0xf);
        clrsh_InvBarleyCorn[i] = GetColorRGB(((0xf - i) * 0xa0) / 0xf + 0x20, ((0xf - i) * 0x94) / 0xf + 0x20, ((0xf - i) * 0x58) / 0xf + 0x20);
    }

    // seems this colors was define static, but we do not want useless memory usage. dynamic
    std::vector<RGBQUAD>clrs_husk(256);
    std::vector<RGBQUAD>clrs_paris_daisy(256);
    std::vector<RGBQUAD>clrs_black(256);
    std::vector<RGBQUAD>clrs_brown_derby(256);
    std::vector<RGBQUAD>clrs_tawny_port(256);

    for (int32_t i = 0; i < 0x100; i++) {
        clrs_husk[i].rgbRed = (i * 0xb9) / 0xff;
        clrs_husk[i].rgbGreen = (i * 0x9f) / 0xff;
        clrs_husk[i].rgbBlue = (i * 0x49) / 0xff;
        clrs_paris_daisy[i].rgbRed = (i * 0xff) / 0xff;
        clrs_paris_daisy[i].rgbGreen = (i * 0xff) / 0xff;
        clrs_paris_daisy[i].rgbBlue = (i * 0x74) / 0xff;
        clrs_black[i].rgbRed = 0;
        clrs_black[i].rgbGreen = 0;
        clrs_black[i].rgbBlue = 0;
        clrs_brown_derby[i].rgbRed = (i * 0x41) / 0xff;
        clrs_brown_derby[i].rgbGreen = (i * 0x2f) / 0xff;
        clrs_brown_derby[i].rgbBlue = (i * 0x14) / 0xff;
        clrs_tawny_port[i].rgbRed = (i * 0x65) / 0xff;
        clrs_tawny_port[i].rgbGreen = (i * 0x27) / 0xff;
        clrs_tawny_port[i].rgbBlue = (i * 0x3d) / 0xff;
    }

    palette_husk = new CGamePalette();
    palette_husk->SetPalette(clrs_husk.data(), 0x10, 4, 0);

    palette_paris_daisy = new CGamePalette();
    palette_paris_daisy->SetPalette(clrs_paris_daisy.data(), 0x10, 4, 0);

    palette_black = new CGamePalette();
    palette_black->SetPalette(clrs_black.data(), 0x10, 4, 0);

    palette_brown_derby = new CGamePalette();
    palette_brown_derby->SetPalette(clrs_brown_derby.data(), 0x10, 4, 0);

    palette_tawny_port = new CGamePalette();
    palette_tawny_port->SetPalette(clrs_tawny_port.data(), 0x10, 4, 0);
}


#ifdef A2CLIENT_PATCH
static uint32_t Scale8ToMaskBits(uint32_t v8, int32_t bits, int32_t shift) {
    if (bits <= 0 || shift < 0) {
        return 0;
    }
    uint32_t scaled = bits <= 8 ? (v8 >> (8 - bits)) : (v8 << (bits - 8));
    return scaled << shift;
}
#endif

// 45488c (client: 4584cc)
uint32_t InitPixelColors() {
#ifdef A2CLIENT_PATCH
    DDPIXELFORMAT pfmt;
    memset(&pfmt, 0, sizeof(DDPIXELFORMAT));
    pfmt.dwSize = sizeof(DDPIXELFORMAT);
    if (FAILED(g_ddsurface->GetPixelFormat(&pfmt))) {
        return 0;
    }

    if ((pfmt.dwFlags & DDPF_RGB) == 0 || pfmt.dwRBitMask == 0 ||
        (pfmt.dwRGBBitCount != 16 && pfmt.dwRGBBitCount != 32)) {
        ((MainWindow*)AfxGetMainWnd())->UpdateClipCursor();
        return 0;
    }

    g_display_bpp = pfmt.dwRGBBitCount;
    int32_t disp_r_shift = BitmaskLowBound(pfmt.dwRBitMask);
    int32_t disp_g_shift = BitmaskLowBound(pfmt.dwGBitMask);
    int32_t disp_b_shift = BitmaskLowBound(pfmt.dwBBitMask);
    int32_t disp_r_bits = BitmaskHighBound(pfmt.dwRBitMask) - disp_r_shift + 1;
    int32_t disp_g_bits = BitmaskHighBound(pfmt.dwGBitMask) - disp_g_shift + 1;
    int32_t disp_b_bits = BitmaskHighBound(pfmt.dwBBitMask) - disp_b_shift + 1;

    if (g_display_bpp == 32) {
        g_display_is_rgb565 = 0;
        for (uint32_t c = 0; c < 65536; c++) {
            uint32_t r5 = (c >> 11) & 0x1F;
            uint32_t g6 = (c >> 5) & 0x3F;
            uint32_t b5 = c & 0x1F;
            uint32_t r8 = (r5 << 3) | (r5 >> 2);
            uint32_t g8 = (g6 << 2) | (g6 >> 4);
            uint32_t b8 = (b5 << 3) | (b5 >> 2);
            g_rgb565_to_32[c] =
                Scale8ToMaskBits(r8, disp_r_bits, disp_r_shift) |
                Scale8ToMaskBits(g8, disp_g_bits, disp_g_shift) |
                Scale8ToMaskBits(b8, disp_b_bits, disp_b_shift);
        }
    } else {
        g_display_is_rgb565 = (pfmt.dwRBitMask == 0xF800 && pfmt.dwGBitMask == 0x07E0 && pfmt.dwBBitMask == 0x001F) ? 1 : 0;
        if (g_display_is_rgb565 == 0) {
            for (uint32_t c = 0; c < 65536; c++) {
                uint32_t r5 = (c >> 11) & 0x1F;
                uint32_t g6 = (c >> 5) & 0x3F;
                uint32_t b5 = c & 0x1F;
                uint32_t r8 = (r5 << 3) | (r5 >> 2);
                uint32_t g8 = (g6 << 2) | (g6 >> 4);
                uint32_t b8 = (b5 << 3) | (b5 >> 2);
                g_rgb565_to_16[c] = (uint16_t)(
                    Scale8ToMaskBits(r8, disp_r_bits, disp_r_shift) |
                    Scale8ToMaskBits(g8, disp_g_bits, disp_g_shift) |
                    Scale8ToMaskBits(b8, disp_b_bits, disp_b_shift));
            }
        }
    }

    // Internal software rendering is pinned to 16-bit RGB565.
    g_RBits = 5;
    g_GBits = 6;
    g_BBits = 5;
    g_RBitShift = 11;
    g_GBitShift = 5;
    g_BBitShift = 0;
    g_RBitMask = 0xF800;
    g_GBitMask = 0x07E0;
    g_BBitMask = 0x001F;

    if (g_brightnessLookup == nullptr) {
        CalculateBrightness();
        CalcColorsAndPalettes();
    }

    ((MainWindow*)AfxGetMainWnd())->UpdateClipCursor();

    return g_RBitMask;
#else
    DDPIXELFORMAT pfmt;
    pfmt.dwSize = sizeof(DDPIXELFORMAT);
    g_ddsurface->GetPixelFormat(&pfmt);

    g_RBitMask = pfmt.dwRBitMask;
    g_GBitMask = pfmt.dwGBitMask;
    g_BBitMask = pfmt.dwBBitMask;

    if (pfmt.dwRBitMask != 0) {
        g_RBitShift = BitmaskLowBound(g_RBitMask);
        g_GBitShift = BitmaskLowBound(g_GBitMask);
        g_BBitShift = BitmaskLowBound(g_BBitMask);
        g_RBits = BitmaskHighBound(g_RBitMask) - g_RBitShift + 1;
        g_GBits = BitmaskHighBound(g_GBitMask) - g_GBitShift + 1;
        g_BBits = BitmaskHighBound(g_BBitMask) - g_BBitShift + 1;

        FreeBrightnessLookup();
        CalculateBrightness();
        CalcColorsAndPalettes();
    }

    ((MainWindow*)AfxGetMainWnd())->UpdateClipCursor();

    return g_RBitMask;
#endif
}

// 453fcc (client: 457c0c)
int32_t InitVideo() {
    if (g_IsServer) {
        CalcColorsAndPalettes();
        return 1;
    }

#ifdef A2CLIENT_PATCH
    g_ddsurface = nullptr;
    g_is_fullscreen = 0;

    g_ddsurf_lock = 0;
    g_ddsurf_lock2 = 0;
    g_ddsurf_lock3 = 0;

    g_ScreenSize.top = 0;
    g_ScreenSize.left = 0;
    g_clipRect = g_ScreenSize;

    gfx_bits = 32;

    delete[] g_surf1_buf;
    delete[] g_surf2_buf;
    delete[] g_surf3_buf;

    g_surf3_w = ((g_ScreenSize.right - 160) + 31) & ~31;
    g_surf3_h = ((g_ScreenSize.bottom + 31) & ~31) + 1;
    size_t surf12_pixels = (size_t)g_ScreenSize.right * (size_t)(g_ScreenSize.bottom + 1);
    size_t surf3_pixels = (size_t)g_surf3_w * (size_t)g_surf3_h;
    g_surf1_buf = new uint16_t[surf12_pixels];
    g_surf2_buf = new uint16_t[surf12_pixels];
    g_surf3_buf = new uint16_t[surf3_pixels];
    memset(g_surf1_buf, 0, surf12_pixels * sizeof(uint16_t));
    memset(g_surf2_buf, 0, surf12_pixels * sizeof(uint16_t));
    memset(g_surf3_buf, 0, surf3_pixels * sizeof(uint16_t));

    memset(&g_selDrawBitmap, 0, sizeof(DDSURFACEDESC));
    g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
    g_selDrawBitmap.lpSurface = g_surf2_buf;
    g_selDrawBitmap.dwWidth = g_ScreenSize.right;
    g_selDrawBitmap.dwHeight = g_ScreenSize.bottom + 1;
    g_selDrawBitmap.lPitch = g_ScreenSize.right * 2;

    if (FAILED(DirectDrawCreate(nullptr, &g_ddraw, nullptr))) {
        return 0;
    }

    if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_NORMAL))) {
        return 0;
    }

    DDSURFACEDESC tmp;
    memset(&tmp, 0, sizeof(DDSURFACEDESC));
    tmp.dwSize = sizeof(DDSURFACEDESC);
    tmp.dwFlags = DDSD_CAPS;
    tmp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    tmp.dwBackBufferCount = 0;

    if (FAILED(g_ddraw->CreateSurface(&tmp, &g_ddsurface, nullptr))) {
        return 0;
    }

    // If we can't init pixel colors in desktop mode, force fullscreen video mode.
    if (InitPixelColors() == 0 && SetVideoMode() == 0) {
        AfxMessageBox("Unable to set video mode", 0, 0);
        exit(-1);
    }

    return 1;
#else
    g_ddsurface = nullptr;
    g_ddsurface2 = nullptr;

    g_ddsurf_lock = 0;
    g_ddsurf_lock2 = 0;

    g_ScreenSize.top = 0;
    g_ScreenSize.left = 0;
    g_clipRect = g_ScreenSize;

    gfx_bits = 16;
    
    if (FAILED(DirectDrawCreate(nullptr, &g_ddraw, nullptr))) {
        return 0;
    }

    if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_NORMAL))) {
        return 0;
    }

    DDSURFACEDESC tmp;
    tmp.dwSize = sizeof(DDSURFACEDESC);
    tmp.dwFlags = DDSD_CAPS;
    tmp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    tmp.dwBackBufferCount = 0;

    if (FAILED(g_ddraw->CreateSurface(&tmp, &g_ddsurface, nullptr))) {
        return 0;

    //if we can't init pixel colors then try force video mode set which also call InitPixelColors
    }
    if (InitPixelColors() == 0 && SetVideoMode() == 0) {
        AfxMessageBox("Unable to set video mode", 0, 0);
        exit(-1);
    }

    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = g_ScreenSize.right;
    desc.dwHeight = g_ScreenSize.bottom + 1;
    if (FAILED(g_ddraw->CreateSurface(&desc, &g_ddsurface2, nullptr))) {
        return 0;
    }

    if (!g_ddsurface2) {
        return 0;
    }

    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = g_ScreenSize.right - 160;
    desc.dwHeight = g_ScreenSize.bottom + 1;
    if (FAILED(g_ddraw->CreateSurface(&desc, &g_ddsurface3, nullptr))) {
        return 0;
    }

    if (!g_ddsurface3) {
        return 0;
    
    /* NOT USED
    DDCAPS g_ddrawCaps; //659a70
    g_ddrawCaps.dwSize = sizeof(DDCAPS);
    g_ddraw->GetCaps(&g_ddrawCaps, nullptr);
    */

    }
    return 1;
#endif
}

// 4546c6 (client: 458306)
int32_t SetVideoMode() {
    if (g_IsServer) {
        return 1;
    }

#ifdef A2CLIENT_PATCH
    // When InitVideo() already initialized the native desktop surface in DDSCL_NORMAL,
    // keep borderless fullscreen at native desktop resolution and stretch g_ScreenSize to it.
    if (g_ddsurface != nullptr && g_RBitMask != 0) {
        ((MainWindow*)AfxGetMainWnd())->UpdateClipCursor();
        return 1;
    }

    CRect disp = GetEffectiveDisplaySize();
    if (g_ddsurface != nullptr) {
        IDirectDrawSurface* old_surf = g_ddsurface;
        g_ddsurface = nullptr;
        old_surf->Release();
    }

    if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) {
        return 0;
    }

    if (SUCCEEDED(g_ddraw->SetDisplayMode(disp.right, disp.bottom, 32))) {
        gfx_bits = 32;
    } else if (SUCCEEDED(g_ddraw->SetDisplayMode(disp.right, disp.bottom, 16))) {
        gfx_bits = 16;
    } else {
        return 0;
    }

    DDSURFACEDESC primary_desc;
    memset(&primary_desc, 0, sizeof(DDSURFACEDESC));
    primary_desc.dwSize = sizeof(DDSURFACEDESC);
    primary_desc.dwFlags = DDSD_CAPS;
    primary_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    primary_desc.dwBackBufferCount = 0;

    if (FAILED(g_ddraw->CreateSurface(&primary_desc, &g_ddsurface, nullptr))) {
        return 0;
    }

    g_is_fullscreen = 1;
    if (InitPixelColors() == 0) {
        return 0;
    }
    return 1;
#else
    if (g_ddsurface) {
        g_ddsurface->Release();
    }
    g_ddsurface = nullptr;

    if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) {
        return 0;
    }

    if (FAILED(g_ddraw->SetDisplayMode(g_ScreenSize.right, g_ScreenSize.bottom, gfx_bits))) {
        return 0;
    }

    g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
    g_selDrawBitmap.dwFlags = DDSD_CAPS;
    g_selDrawBitmap.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    g_selDrawBitmap.dwBackBufferCount = 0;

    if (FAILED(g_ddraw->CreateSurface(&g_selDrawBitmap, &g_ddsurface, nullptr))) {
        return 0;
    }

    InitPixelColors();
    return 1;
#endif
}



// 45e0d0 (client: 461cf0)
CGameFont::CGameFont() {
    this->bitmap = nullptr;
    this->char_widths = nullptr;
}

// 45e0f9 (client: 461d19)
CGameFont::~CGameFont() {
    if (this->char_widths) {
        delete[] this->char_widths;
    }

    if (this->bitmap) {
        delete this->bitmap;
    }
}

// 460f60 (client: 464b40)
void CGameFont::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh) {
}

// 460f70 (client: 464b50)
uint16_t* CGameFont::GetShadowColors() {
    return nullptr;
}

// 45ebda (client: 4627f6)
void CGameFont::DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy) {
    uint16_t* shdow = this->GetShadowColors();
    this->DrawTxt(x + shadow_dxy, y + shadow_dxy, txt, align, shdow);
    this->DrawTxt(x, y, txt, align, colorsh);
}


// 45e1f9 (client: 461e19)
int32_t CGameFont::GetStrWidth(const char* str) {
    int32_t wi = 0;
    int ln = strlen(str);

    for (int i = 0; i < ln; i++) {
        char c = str[i];
        if ((uint8_t)c >= ' ') { // 0x20
            if (c == '~') {
                if ((i != ln - 1) && str[i + 1] == '~') {
                    i++;

                    uint8_t n = DecodeChar('~') - 0x20;
                    wi += this->char_widths[n] + this->space;
                    if (n == 0) {
                        wi += this->bitmap->GetHeight(0) / 2;
                    }
                }
            } else {
                uint8_t n = DecodeChar(c) - 0x20;
                wi += this->char_widths[n] + this->space;
                if (n == 0) {
                    wi += this->bitmap->GetHeight(0) / 2;
                }
            }
        }
    }

    return wi;
}


// 402a90 (client: 402860)
int32_t CGameFont::GetHeight() {
    return this->bitmap->GetHeight(0);
}


// 45e959 (client: 462575) - internal
CStringArray& CGameFont::StrToArray(const char* str) {
    static CStringArray out;  //static for return

    CString s = str;
    out.RemoveAll();

    CString tmp;

    while (!s.IsEmpty()) {
        int pos = s.Find('\r');
        if (pos == -1) {
            break;
        }

        if (s[pos + 1] == '\n') {
            tmp = s.Left(pos + 1);
            tmp.TrimLeft();
            out.Add( tmp );

            s = s.Right(s.GetLength() - (pos + 2));
            s.TrimLeft();
        }
    }

    if (!s.IsEmpty()) {
        out.Add(s);
    }

    return out;
}

// 45e35f (client: 461f7b) - internal
CStringArray& CGameFont::StrSplitToFitWidth(const CRect& r, const char* str) {
    static CStringArray out;  //static for return

    CString s = str;
    CString tmp1;
    CString tmp2;

    out.RemoveAll();

    s.TrimLeft();
    s.TrimRight();

    int32_t split_pos;

    const int32_t w = r.Width();
    while (!s.IsEmpty()) {
        int32_t x = 0;
        if (this->GetStrWidth(s) >= w) {
            tmp2 = s;
            while (true) {
                split_pos = x;

                int32_t pos = tmp2.Find(' ');
                if (pos == -1) {
                    x = s.GetLength();
                } else {
                    x += pos + 1;
                }

                tmp1 = s.Left(x);

                tmp2 = s.Right(s.GetLength() - x);

                if (tmp2.IsEmpty()) {
                    if (this->GetStrWidth(tmp1) < w) {
                        split_pos = x;
                        break;
                    }
                }

                if (this->GetStrWidth(tmp1) >= w) {
                    break;
                }
            }
        } else {
            s += '\r';
            split_pos = s.GetLength();
        }

        if (w < this->GetStrWidth(tmp1) && split_pos == 0) {
            split_pos = x;
        }

        tmp1 = s.Left(split_pos);
        tmp1.TrimLeft();

        out.Add(tmp1);

        s = s.Right(s.GetLength() - split_pos);
        s.TrimLeft();
    }

    return out;
}

// 45eb1d (client: 462739)
CStringArray& CGameFont::StringArrayForRect(const CRect& r, const char* str) {

    static CStringArray out;

    out.RemoveAll();

    CStringArray& lines = this->StrToArray(str);
    
    for (int i = 0; i < lines.GetSize(); i++) {
        CStringArray& spl = this->StrSplitToFitWidth(r, lines[i]);
        out.Append(spl);
    }
    return out;
}

// 45f6c2 (client: 4632dd)
void CGameFont::DrawTextLinesShadow(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t* clr, int32_t dy) {
    if (last > lines.GetSize()) {
        last = lines.GetSize();
    }
    if (dy == 0) {
        dy = this->GetHeight();
    }

    for (int32_t i = first; i < last; i++) {
        uint8_t flags = 0;
        CString line = lines[i];
        if (i != lines.GetSize() - 1) {
            if (line[line.GetLength() - 1] != '\r') {
                flags |= 2;
            }
        }

        if (i == 0) {
            flags |= 1;
            if (this->GetStrWidth(line + ' ') > r.Width()) {
                flags |= 2;
            }
        } else {
            const CString& s = lines[i - 1];

            if (s[s.GetLength() - 1] == '\r') {
                flags |= 1;
                if (this->GetStrWidth(line + ' ') > r.Width()) {
                    flags |= 2;
                }
            }
        }

        if (i == lines.GetSize() - 1) {
            flags |= 4;
        } else {
            if (line[line.GetLength() - 1] != '\r') {
                flags |= 4;
            }
        }

        if (line[line.GetLength() - 1] == '\r') {
            line.SetAt(line.GetLength() - 1, 0);
        }

        if ((flags & 2) == 0) {
            if ((flags & 1) == 0) {
                this->DrawTextWithShadow(r.left, r.top + (i - first) * dy, line, ALIGN_LEFT_TOP, clr, 1);
            } else {
                this->DrawTextWithShadow(r.left + this->char_widths[' '], r.top + (i - first) * dy, line, ALIGN_LEFT_TOP, clr, 1);
            }
        } else if ((flags & 1) == 0) {
            this->DrawTextJustifyShadow(r.left, r.top + (i - first) * dy, r.Width(), line, clr);
        } else {
            this->DrawTextJustifyShadow(r.left + this->char_widths[' '], r.top + (i - first) * dy, r.Width() - this->char_widths[' '], line, clr);
        }
    }
}

// 45f33e (client: 462f5a)
void CGameFont::DrawTextLines(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t* clr, int32_t dy) {
    if (last > lines.GetSize()) {
        last = lines.GetSize();
    }
    if (dy == 0) {
        dy = this->GetHeight();
    }

    for (int32_t i = first; i < last; i++) {
        uint8_t flags = 0;
        CString line = lines[i];
        if (i != lines.GetSize() - 1) {
            if (line[line.GetLength() - 1] != '\r') {
                flags |= 2;
            }
        }

        if (i == 0) {
            flags |= 1;
            if (this->GetStrWidth(line + ' ') > r.Width()) {
                flags |= 2;
            }
        } else {
            const CString& s = lines[i - 1];

            if (s[s.GetLength() - 1] == '\r') {
                flags |= 1;
                if (this->GetStrWidth(line + ' ') > r.Width()) {
                    flags |= 2;
                }
            }
        }

        if (i == lines.GetSize() - 1) {
            flags |= 4;
        } else {
            if (line[line.GetLength() - 1] != '\r') {
                flags |= 4;
            }
        }

        if (line[line.GetLength() - 1] == '\r') {
            line.SetAt(line.GetLength() - 1, 0);
        }

        if ((flags & 2) == 0) {
            if ((flags & 1) == 0) {
                this->DrawTxt(r.left, r.top + (i - first) * dy, line, ALIGN_LEFT_TOP, clr);
            } else {
                this->DrawTxt(r.left + this->char_widths[' '], r.top + (i - first) * dy, line, ALIGN_LEFT_TOP, clr);
            }
        } else if ((flags & 1) == 0) {
            this->DrawTextJustify(r.left, r.top + (i - first) * dy, r.Width(), line, clr);
        } else {
            this->DrawTextJustify(r.left + this->char_widths[' '], r.top + (i - first) * dy, r.Width() - this->char_widths[' '], line, clr);
        }
    }
}


// 45f10b (client: 462d27)
void CGameFont::DrawTextJustifyShadow(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr) {
    int32_t space_width = this->char_widths[' '];

    CStringArray words;

    txt.TrimRight();
    int32_t words_width = 0;
    while (txt.GetLength() != 0) {
        txt.TrimLeft();

        int32_t word_end = txt.Find(' ');
        if (word_end == -1) {
            word_end = txt.GetLength();
        }

        CString wrd = txt.Left(word_end);
        wrd.TrimRight();

        words.Add(wrd);

        words_width += this->GetStrWidth(wrd);

        txt = txt.Right(txt.GetLength() - word_end);
    }

    double word_space = (w - words_width);
    if (words.GetSize() > 1) {
        word_space /= (double)(words.GetSize() - 1);
    }

    double xx = (double)x;
    for (int i = 0; i < words.GetSize(); i++) {
        this->DrawTextWithShadow((int32_t)xx, y, words[i], ALIGN_LEFT_TOP, clr, 1);
        xx += word_space + this->GetStrWidth(words[i]);
    }
}

// 45eed7 (client: 462af3)
void CGameFont::DrawTextJustify(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr) {
    int32_t space_width = this->char_widths[' '];

    CStringArray words;

    txt.TrimRight();
    int32_t words_width = 0;
    while (txt.GetLength() != 0) {
        txt.TrimLeft();

        int32_t word_end = txt.Find(' ');
        if (word_end == -1) {
            word_end = txt.GetLength();
        }

        CString wrd = txt.Left(word_end);
        wrd.TrimRight();

        words.Add(wrd);

        words_width += this->GetStrWidth(wrd);

        txt = txt.Right(txt.GetLength() - word_end);
    }

    double word_space = (w - words_width);
    if (words.GetSize() > 1) {
        word_space /= (double)(words.GetSize() - 1);
    }

    double xx = (double)x;
    for (int i = 0; i < words.GetSize(); i++) {
        this->DrawTxt((int32_t)xx, y, words[i], ALIGN_LEFT_TOP, clr);
        xx += word_space + this->GetStrWidth(words[i]);
    }
}


// 45ed37 (client: 46284f)
void CGameFont::DrawTextJustifyInRectShadow(const CRect& r, const char* str, uint16_t* clr, int32_t dy) {
    CStringArray& array = this->StringArrayForRect(r, str);

    if (dy == 0) {
        dy = this->GetHeight();
    }

    for (int32_t i = 0; i < array.GetSize(); i++) {
        if (i * dy + this->GetHeight() > r.Height()) {
            break;
        }
        this->DrawTextJustifyShadow(r.left, r.top + i * dy, r.Width(), array[i], clr);
    }

    array.RemoveAll();
}




void CGamePalette::operator=(const CGamePalette& b) {
    this->Free();

    this->count = b.count;
    this->colors = new uint16_t[256 * this->count];
    memcpy(this->colors, b.colors, this->count * 256 * sizeof(uint16_t));
}

// 423c6d (client: 425b9d)
CGamePalette::~CGamePalette() {
    this->Free();
}

// 4245a7 (client: 4264d7)
void CGamePalette::Dump(CDumpContext& dc) const {
    //dc << "CGamePalette";
}

// 42457a (client: 4264aa)
void CGamePalette::Free() {
    if (this->colors) {
        delete[] this->colors;
    }

    this->colors = nullptr;

    this->count = 0;
}

// 423cbe (client: 425bee)
void CGamePalette::SetPalette(RGBQUAD* rgb, uint32_t _count, int mode, int useColor) {
    this->count = _count;
    this->colors = new uint16_t[256 * this->count];

    uint32_t dg = g_DeltaCLR.g;
    uint32_t db = g_DeltaCLR.b;
    uint32_t dr = g_DeltaCLR.r;

    if (useColor == 0) {
        db = 0;
        dg = 0;
        dr = 0;
    }

    uint16_t* clrs = this->colors;

    switch (mode) {
    case 0:
        this->colors[0] = 0;
        for (int i = 1; i < 256; i++) {
            this->colors[i] = 0xffff;
        }
        break;

    case 1:
        for (int i = 0; i < 256; i++) {
            uint32_t r = rgb[i].rgbRed + dr;

            if (dr > 255) {
                dr = 255;
            }

            uint32_t g = rgb[i].rgbGreen + dg;
            
            if (dg > 255) {
                dg = 255;
            }

            uint32_t b = rgb[i].rgbBlue + db;
            
            if (db > 255) {
                db = 255;
            }

            this->colors[i] = GetColorRGB(r, g, b);
        }
        break;

    case 2:
        for (int j = this->count; j > 0; j--) {
            for (int i = 0; i < 256; i++) {
                int r = (rgb[i].rgbRed + dr) * j * 2 / this->count;
                if (r < 0) {
                    r = 0;
                } else if (r > 255) {
                    r = 255;
                }

                int g = (rgb[i].rgbGreen + dg) * j * 2 / this->count;
                if (g < 0) {
                    g = 0;
                } else if (g > 255) {
                    g = 255;
                }

                int b = (rgb[i].rgbBlue + db) * j * 2 / this->count;
                if (b < 0) {
                    b = 0;
                } else if (b > 255) {
                    b = 255;
                }

                clrs[i] = GetColorRGB(r, g, b);
            }
            clrs += 256;
        }
        break;

    case 3:
        for (int j = this->count; j > 0; j--) {
            for (int i = 0; i < 256; i++) {
                int r = (rgb[i].rgbRed + dr) * j / 32;
                if (r < 0) {
                    r = 0;
                } else if (r > 255) {
                    r = 255;
                }

                int g = (rgb[i].rgbGreen + dg) * j / 32;
                if (g < 0) {
                    g = 0;
                } else if (g > 255) {
                    g = 255;
                }

                int b = (rgb[i].rgbBlue + db) * j / 32;
                if (b < 0) {
                    b = 0;
                } else if (b > 255) {
                    b = 255;
                }

                clrs[i] = GetColorRGB(r, g, b);
            }
            clrs += 256;
        }
        break;

    case 4:
        for (int j = 1; j < 17; j++) {
            for (int i = 0; i < 256; i++) {
                int r, g, b;
                if (g_isLowMemory == 0) {
                    r = rgb[i].rgbRed * j / 16;
                    g = rgb[i].rgbGreen * j / 16;
                    b = rgb[i].rgbBlue * j / 16;
                } else {
                    r = rgb[i].rgbRed * j / 18;
                    g = rgb[i].rgbGreen * j / 18;
                    b = rgb[i].rgbBlue * j / 18;
                }
                if (r < 0) {
                    r = 0;
                } else if (r > 255) {
                    r = 255;
                }

                if (g < 0) {
                    g = 0;
                } else if (g > 255) {
                    g = 255;
                }

                if (b < 0) {
                    b = 0;
                } else if (b > 255) {
                    b = 255;
                }
                    
                clrs[i] = GetColorRGB(r, g, b);
            }
            clrs += 256;
        }
        break;

    case 5:
        for (int j = this->count; j> 0; j--) {
            for (int i = 0; i < 256; i++) {
                int v = ((rgb[i].rgbRed + rgb[i].rgbGreen + rgb[i].rgbBlue) * j * 2) / 3 / this->count;
                if (v > 255) {
                    v = 255;
                }

                clrs[i] = GetColorRGB(v, v, v);
            }
            clrs += 256;
        }
    }
}

// 41ec40 (client: 420440)
uint16_t* CGamePalette::GetPalette(int32_t idx) {
#ifdef A2CLIENT_PATCH
    // CRASH_fix_420450 (client 420450): invalid palette -> null
    if (this == nullptr || this->colors == nullptr) {
        return nullptr;
    }
#endif
    return this->colors + 256 * idx; 
}





// 4247a6 (client: 4266d6)
CGameBitmap::CGameBitmap() {}

// 4245c1 (client: 4264f1)
CGameBitmap::CGameBitmap(const char* fname) {
    File2 f;
    if (!f.Open(fname, CFile::modeRead)) {
        CString s = "FATAL ERROR: can't load ";
        s += fname;
        AfxMessageBox(s);
        exit(-1);
    }

    f.Seek(-4, CFile::end);
    f.Read(&this->frames_count, 4);
    this->data_size = f.GetLength();

    f.Rewind();

    this->palette_data = nullptr;

    this->pdata = new uint8_t[this->data_size];
    f.Read(this->pdata, this->data_size);
    f.Close();

    this->frames = new void*[this->frames_count];

    uint8_t* dpos = this->pdata;
    for (int i = 0; i < this->frames_count; i++) {
        SpriteFrame* frm = (SpriteFrame*)dpos;
        this->frames[i] = frm;
        dpos = frm->data + frm->datasize;
    }
}

// 4247fd (client: 42672d)
CGameBitmap::CGameBitmap(const CGameBitmap& src) {
    this->frames_count = src.frames_count;
    this->data_size = src.data_size;

    this->frames = new void* [this->frames_count];

    this->pdata = new uint8_t[this->data_size];

    memcpy(this->pdata, src.pdata, this->data_size);

    uint8_t* dpos = this->pdata;
    for (int i = 0; i < this->frames_count; i++) {
        SpriteFrame* frm = (SpriteFrame*)dpos;
        this->frames[i] = frm;
        dpos = frm->data + frm->datasize;
    }

    this->palette = src.palette;

    this->palette_data = src.palette_data; //FIXME
}

// 42496a (client: 42689a)
CGameBitmap::~CGameBitmap() {
    if (this->frames) {
        delete[] this->frames;
    }
    if (this->pdata) {
        delete[] this->pdata;
    }

    this->palette.Free();
}

// 424d3d (client: 426c6d)
void CGameBitmap::Dump(CDumpContext& dc) const {
    //dc << "CGameBitmap";
}

// 428d60 (client: 42a820)
void CGameBitmap::DrawWithPalette(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode) {
}

// 428d50 (client: 42a810)
void CGameBitmap::Draw(int32_t x, int32_t y, int frame, int mode, int palid) {
}

// 428d70 (client: 42a830)
void CGameBitmap::DrawBrightened(int32_t x, int32_t y, int frame, int blevel, int mode) {
}

// 424ce6 (client: 426c16)
int32_t CGameBitmap::GetWidth(int frame) {
    return ((SpriteFrame**)this->frames)[frame]->width;
}

// 424d11 (client: 426c41)
int32_t CGameBitmap::GetHeight(int frame) {
    return ((SpriteFrame**)this->frames)[frame]->height;
}

// 424d57 (client: 426c87)
void CGameBitmap::SelectBitmapForDraw() {
    g_selDrawBitmap.lpSurface = CGameBitmap::GetData();
    g_selDrawBitmap.dwWidth = this->GetWidth(0);
    g_selDrawBitmap.dwHeight = this->GetHeight(0);
    g_selDrawBitmap.lPitch = this->GetWidth(0) * this->GetPixelSize();
}

// 428d80 (client: 42a840)
void CGameBitmap::FlipVertical() {
}

// 428d90 (client: 42a850)
int CGameBitmap::GetPixelSize() {
    return 2;
}

// 41ec60 (client: 420460)
void* CGameBitmap::GetData() {
    return this->pdata + sizeof(BmpFrame); 
}

// 424cb0 (client: 426be0)
void CGameBitmap::ResetPalette(uint32_t count, int mode, int useColor) {
    this->palette.Free();
    this->palette.SetPalette((RGBQUAD*)this->palette_data, count, mode, useColor);
}

// 4387f0 (client: 43ae10)
int32_t CGameBitmap::GetFrameCount() {
    return this->frames_count; 
}










// 425f8c (client: 427eba)
CBmp256::CBmp256(const char* fname) {
    File2 f;
    if (!f.Open(fname, CFile::modeRead)) {
        CString s = "FATAL ERROR: can't load ";
        s += fname;
        AfxMessageBox(s);
        exit(-1);
    }

    BITMAPFILEHEADER fhdr;
    f.Read(&fhdr, sizeof(fhdr)); //0xe
    
    BITMAPINFOHEADER binf;
    f.Read(&binf, sizeof(binf)); //0x28

    f.Seek(0x36, CFile::begin);

    const uint32_t pixsz = binf.biWidth * binf.biHeight; //pixel data size

    this->data_size = pixsz + 256 * 4 + sizeof(BmpFrame);

    this->pdata = new uint8_t[this->data_size];

    this->palette_data = this->pdata + pixsz + sizeof(BmpFrame);
    memset(this->palette_data, 0, 256 * 4);

    if (binf.biClrUsed == 0) {
        f.Read(this->palette_data, 256 * 4);
    } else {
        f.Read(this->palette_data, binf.biClrUsed * 4);
    }

    BmpFrame* frm = (BmpFrame*)this->pdata;
    frm->width = binf.biWidth;
    frm->height = binf.biHeight;

    f.Read(this->pdata + sizeof(BmpFrame), pixsz);
    f.Close();

    CBmp256::FlipVertical();

    this->frames = new void*[1];
    this->frames[0] = this->pdata + sizeof(BmpFrame);
}

// 4262d1 (client: 4281ff)
CBmp256::CBmp256(uint32_t w, uint32_t h) {

    this->data_size = w * h + sizeof(BmpFrame);
    this->pdata = new uint8_t[this->data_size];

    memset(this->pdata, 0, this->data_size);

    this->palette_data = nullptr;

    BmpFrame* frm = (BmpFrame*)this->pdata;
    frm->width = w;
    frm->height = h;

    this->frames = new void* [1];
    this->frames[0] = this->pdata + sizeof(BmpFrame);
}

// 428a40 (client: 42aba0)
CBmp256::~CBmp256() {
}

// 42649a (client: 4283c8)
void CBmp256::Dump(CDumpContext& dc) const {
}

// 4263df (client: 42830d)
void CBmp256::Draw(int32_t x, int32_t y, int frame, int mode, int palid) {
    const uint16_t* pal = this->palette.GetPalette(palid);
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    BlitPaletted(x, y, 0, 0, frm->width, frm->height, this->pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}

// 4264c0 (client: 4283ee)
int32_t CBmp256::GetWidth(int frame) {
    return ((BmpFrame*)this->pdata)->width;
}

// 4264d5 (client: 428403)
int32_t CBmp256::GetHeight(int frame) {
    return ((BmpFrame*)this->pdata)->height;
}

// 4261a6 (client: 4280d4)
void CBmp256::FlipVertical() {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    uint8_t* a = this->pdata + sizeof(BmpFrame);
    uint8_t* b = this->pdata + sizeof(BmpFrame) + (frm->height - 1) * frm->width;
    for (int i = 0; i < frm->height / 2; i++) {
        memcpy(g_GfxTempBuffer, a, frm->width);
        memcpy(a, b, frm->width);
        memcpy(b, g_GfxTempBuffer, frm->width);

        a += frm->width;
        b -= frm->width;
    }
}

// 428da0 (client: 42ab60)
int CBmp256::GetPixelSize() {
    return 1;
}

// 426385 (client: 4282b3)
void CBmp256::DrawSubRect(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid) {
    const uint16_t* pal = this->palette.GetPalette(palid);
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    BlitPaletted(x, y, l, t, r, b, this->pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}

// 426440 (client: 42836e)
void CBmp256::DrawSubRectAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid) {
    const uint16_t* pal = this->palette.GetPalette(palid);
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    BlitFlipPalettedAdd(x, y, l, t, r, b, this->pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}






// 42576d (client: 42769b)
CBmp64::CBmp64(const char* fname) {
    File2 f;
    if (!f.Open(fname, CFile::modeRead)) {
        CString s = "FATAL ERROR: can't load ";
        s += fname;
        AfxMessageBox(s);
        exit(-1);
    }

    BITMAPFILEHEADER fhdr;
    f.Read(&fhdr, sizeof(fhdr)); //0xe

    BITMAPINFOHEADER binf;
    f.Read(&binf, sizeof(binf)); //0x28

    f.Seek(0x36, CFile::begin);

    const uint32_t pixsz = binf.biWidth * binf.biHeight; //pixel data size

    this->data_size = pixsz * 2 + sizeof(BmpFrame);

    this->pdata = new uint8_t[this->data_size];

    this->palette_data = nullptr;

    BmpFrame* frm = (BmpFrame*)this->pdata;
    frm->width = binf.biWidth;
    frm->height = binf.biHeight;

    this->frames = new void*[1];
    this->frames[0] = this->pdata + sizeof(BmpFrame);

    uint16_t* dat = (uint16_t*)this->frames[0];
    for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; ) {
        int32_t rsz = sizeof(g_GfxTempBuffer);
        if (rawdata < rsz) {
            rsz = rawdata;
        }

        f.Read(g_GfxTempBuffer, rsz);
        Convert24To16(g_GfxTempBuffer, dat, rsz / 3);
        dat += rsz / 3;

        rawdata -= rsz;
    }
    f.Close();
}

// 42597e (client: 4278ac)
CBmp64::CBmp64(uint32_t w, uint32_t h) {
    this->data_size = w * h * 2 + sizeof(BmpFrame);
    this->pdata = new uint8_t[this->data_size];
    
    this->palette_data = nullptr;

    BmpFrame* frm = (BmpFrame*)this->pdata;
    frm->width = w;
    frm->height = h;

    this->frames = new void*[1];
    this->frames[0] = this->pdata + sizeof(BmpFrame);
}

// 4289f0 (client: 42ab40)
CBmp64::~CBmp64() {
}

// 425f3b (client: 427e69)
void CBmp64::Dump(CDumpContext& dc) const {
}

// 425e0c (client: 427d3a)
void CBmp64::Draw(int32_t x, int32_t y, int frame, int mode, int palid) {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    Blit16(x, y, 0, 0, frm->width, frm->height, this->pdata + sizeof(BmpFrame), frm->width, frm->height);
}

// 425f61 (client: 427e8f)
int32_t CBmp64::GetWidth(int frame) {
    return ((BmpFrame*)this->pdata)->width;
}

// 425f76 (client: 427ea4)
int32_t CBmp64::GetHeight(int frame) {
    return ((BmpFrame*)this->pdata)->height;
}

// 425cdb (client: 427c09)
void CBmp64::FlipVertical() {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    uint16_t* a = (uint16_t*)((uint8_t*)this->pdata + sizeof(BmpFrame));
    uint16_t* b = (uint16_t*)((uint8_t*)this->pdata + sizeof(BmpFrame)) + (frm->height - 1) * frm->width;
    for (int i = 0; i < frm->height / 2; i++) {
        memcpy(g_GfxTempBuffer, a, frm->width * 2);
        memcpy(a, b, frm->width * 2);
        memcpy(b, g_GfxTempBuffer, frm->width * 2);

        a += frm->width;
        b -= frm->width;
    }
}

// 425e5d (client: 427d8b)
void CBmp64::DrawSubRect(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b) {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    Blit16(x, y, l, t, r, b, this->pdata + sizeof(BmpFrame), frm->width, frm->height);
}

// 425ea7 (client: 427dd5)
void CBmp64::DrawSubRectMasked(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b) {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    Blit16Masked(x, y, l, t, r, b, this->pdata + sizeof(BmpFrame), frm->width, frm->height);
}

// 425ef1 (client: 427e1f)
void CBmp64::DrawSubRectAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b) {
    const BmpFrame* frm = (BmpFrame*)this->pdata;
    Blit16Add(x, y, l, t, r, b, this->pdata + sizeof(BmpFrame), frm->width, frm->height);
}

// 425a17 (client: 427945)
void CBmp64::LoadFile(const char* fname, CGameBitmap* bmp2) {
    File2 f;
    if (!f.Open(fname, CFile::modeRead)) {
        CString s = "FATAL ERROR: can't load ";
        s += fname;
        AfxMessageBox(s);
        exit(-1);
    }

    BITMAPFILEHEADER fhdr;
    f.Read(&fhdr, sizeof(fhdr)); //0xe

    BITMAPINFOHEADER binf;
    f.Read(&binf, sizeof(binf)); //0x28

    uint16_t* dat = (uint16_t*)(this->pdata + sizeof(BmpFrame));
    for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; ) {
        int32_t rsz = sizeof(g_GfxTempBuffer);
        if (rawdata < rsz) {
            rsz = rawdata;
        }

        f.Read(g_GfxTempBuffer, rsz);
        Convert24To16(g_GfxTempBuffer, dat, rsz / 3);
        dat += rsz / 3;

        rawdata -= rsz;
    }

    if (bmp2) {
        f.Read(bmp2->GetData(), binf.biWidth * binf.biHeight);
    }

    f.Close();
}

// 425b95 (client: 427ac3)
void CBmp64::WriteFile(const char* fname, CGameBitmap* bmp2) {
    CFile f;
    f.Open(fname, CFile::modeCreate | CFile::modeWrite);

    BITMAPFILEHEADER fhdr;
    f.Write(&fhdr, sizeof(fhdr)); //0xe

    BITMAPINFOHEADER binf;
    binf.biWidth = this->GetWidth();
    binf.biHeight = this->GetHeight();
    f.Write(&binf, sizeof(binf)); //0x28

    uint16_t* dat = (uint16_t*)(this->pdata + sizeof(BmpFrame));
    for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; ) {
        int32_t rsz = sizeof(g_GfxTempBuffer);
        if (rawdata < rsz) {
            rsz = rawdata;
        }

        Convert16To24(dat, g_GfxTempBuffer, rsz / 3);
        f.Write(g_GfxTempBuffer, rsz);

        dat += rsz / 3;
        rawdata -= rsz;
    }

    if (bmp2) {
        f.Write(bmp2->GetData(), binf.biWidth * binf.biHeight);
    }

    f.Close();
}




// 424e79 (client: 426da9)
CSprite256::CSprite256(const char* fname) {

    File2 f;
    if (!f.Open(fname, CFile::modeRead)) {
        CString s = "FATAL ERROR: can't load ";
        s += fname;
        AfxMessageBox(s);
        exit(-1);
    }

    SpriteTrailer trailer;
    f.Seek(-4, CFile::end);
    f.Read(&trailer.raw, 4);
    this->data_size = f.GetLength();

    f.Rewind();

    bool has_pal = trailer.has_palette != 0;

    this->frames_count = trailer.frames_count;

    this->pdata = new uint8_t[this->data_size];

    this->palette_data = this->pdata + this->data_size - 4 * 256;

    if (has_pal) {
        f.Read(this->palette_data, 4 * 256);
        this->data_size -= 4 * 256;
    }

    f.Read(this->pdata, this->data_size);
    f.Close();

    this->frames = new void*[this->frames_count];

    uint8_t* dpos = this->pdata;
    for (int i = 0; i < this->frames_count; i++) {
        SpriteFrame* frm = (SpriteFrame*)dpos;
        this->frames[i] = frm;
        dpos = frm->data + frm->datasize;
    }
}


// 4250bf (client: 426fef)
CSprite256::CSprite256(const CSprite256& b)
: CGameBitmap(b) {
}

// 4289a0 (client: 42aaf0)
CSprite256::~CSprite256() {
}

// 425558 (client: 427486)
void CSprite256::Dump(CDumpContext& dc) const {
}

// 425210 (client: 427140)
void CSprite256::DrawWithPalette(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode) {
    const uint16_t* pal = ppalette->GetPalette(palid);
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        BlitRle8(x, y, frm->width, frm->height, frm->data, pal);
    } else {
        BlitRle8FlipX(x, y, frm->width, frm->height, frm->data, pal);
    }
}

// 4250e4 (client: 427014)
void CSprite256::Draw(int32_t x, int32_t y, int frame, int palid, int mode) {
    const uint16_t* pal = this->palette.GetPalette(palid);
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        BlitRle8(x, y, frm->width, frm->height, frm->data, pal);
    } else {
        BlitRle8FlipX(x, y, frm->width, frm->height, frm->data, pal);
    }
}

// 425330 (client: 427260)
void CSprite256::DrawBrightened(int32_t x, int32_t y, int frame, int blevel, int mode) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        if (g_isLowMemory == 0) {
            BlitLightRle8(x, y, frm->width, frm->height, frm->data, blevel);
        } else {
            BlitLightRle8Low(x, y, frm->width, frm->height, frm->data, blevel);
        }
    } else {
        if (g_isLowMemory == 0) {
            BlitLightRle8FlipX(x, y, frm->width, frm->height, frm->data, blevel);
        } else {
            BlitLightRle8FlipXLow(x, y, frm->width, frm->height, frm->data, blevel);
        }
    }
}

// 4252a0 (client: 4271d0)
void CSprite256::DrawWithPaletteBlend(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        BlitRle8Blend(x, y, frm->width, frm->height, frm->data, ppalette->GetPalette(palid));
    } else {
        BlitRle8BlendFlipX(x, y, frm->width, frm->height, frm->data, ppalette->GetPalette(palid));
    }
}

// 42517a (client: 4270aa)
void CSprite256::DrawBlend(int32_t x, int32_t y, int frame, int palid, int mode) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        BlitRle8Blend(x, y, frm->width, frm->height, frm->data, this->palette.GetPalette(palid));
    } else {
        BlitRle8BlendFlipX(x, y, frm->width, frm->height, frm->data, this->palette.GetPalette(palid));
    }
}

// 425414 (client: 427344)
void CSprite256::DrawShadowed(int32_t x, int32_t y, int frame, int blevel, int32_t tilt, int mode) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    if (mode == 0) {
        if (g_isLowMemory == 0) {
            BlitLightRle8Tilt(x, y, frm->width, frm->height, frm->data, blevel, tilt);
        } else {
            BlitLightRle8TiltLow(x, y, frm->width, frm->height, frm->data, blevel, tilt);
        }
    } else {
        if (g_isLowMemory == 0) {
            BlitLightRle8TiltFlipX(x, y, frm->width, frm->height, frm->data, blevel, tilt);
        } else {
            BlitLightRle8TiltFlipXLow(x, y, frm->width, frm->height, frm->data, blevel, tilt);
        }
    }
}

// 425508 (client: 427438)
void CSprite256::DrawIndexMask(int32_t x, int32_t y, int frame, uint32_t mv) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    BlitRle8IndexMask(x, y, frm->width, frm->height, frm->data, mv);
}





// 4287ff (client: 42a71b)
CA16::CA16(const char* fname)
: CSprite256(fname)
{}


// 428824 (client: 42a740)
void CA16::Draw(int32_t x, int32_t y, int frame, int palid, int mode) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];

    uint16_t* pal = (uint16_t*)palid;
    if (!pal) {
        pal = this->palette.GetPalette(0);
    }

    if (mode == 0) {
        BlitRle16(x, y, frm->width, frm->height, (uint16_t*)frm->data, pal);
    } else {
        BlitRle16FlipX(x, y, frm->width, frm->height, (uint16_t*)frm->data, pal);
    }
}





// 424db8 (client: 426ce8)
CSprite16::CSprite16(const char* fname)
: CGameBitmap(fname)
{}


// 424ddd (client: 426d0d)
void CSprite16::DrawRle4(int32_t x, int32_t y, int frame, uint16_t* pcolor) {
    SpriteFrame* frm = ((SpriteFrame**)this->frames)[frame];
    BlitRle4(x, y, frm->width, frm->height, frm->data, pcolor);
}





// 45fa44 (client: 46365e)
CSpriteFont16::CSpriteFont16(const char* fname, int32_t _space) {
    CString buf = fname;
    buf += ".16";

    this->bitmap = new CSprite16(buf);

    buf = fname;
    buf += ".dat";

    File2 f;
    f.Open(buf, CFile::modeRead);

    this->char_widths = new int32_t[f.GetLength() / 4];
    f.Read(this->char_widths, f.GetLength());

    f.Close();

    this->space = _space;
}

// 45fbd7 (client: 4637f1)
void CSpriteFont16::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* clr) {
    if ((align & ALIGN_RIGHT) != 0) {
        x -= this->GetStrWidth(txt);
    }

    if ((align & ALIGN_HCENTER) != 0) {
        x -= this->GetStrWidth(txt) / 2;
    }

    if ((align & ALIGN_BOTTOM) != 0) {
        y -= this->bitmap->GetWidth();
    }

    if ((align & ALIGN_VCENTER) != 0) {
        y -= this->bitmap->GetWidth() / 2;
    }

    int32_t tlen = strlen(txt);
    for (int32_t i = 0; i < tlen; i++) {
        uint8_t chr1 = DecodeChar(txt[i]) - 0x20;
        uint8_t chr2 = DecodeChar(txt[i + 1]) - 0x20;

        if (chr1 == 0x5e && chr2 != 0x5e) {
            FillRectColor(x, y + this->bitmap->GetHeight(), x + this->char_widths[chr2], y + this->bitmap->GetHeight(), clr[15]);
        } else {
            if (chr1 == 0) {
                x += this->bitmap->GetHeight() / 2;
            } else {
                ((CSprite16*)this->bitmap)->DrawRle4(x, y, chr1, clr);
            }

            x += this->char_widths[chr1] + this->space;
        }

        if (chr1 == 0x5e && chr2 == 0x5e) {
            i++;
        }
    }
}

// 45fdf2 (client: 463a06)
uint16_t* CSpriteFont16::GetShadowColors() {
    return clrsh_Black;
}



// 45fe02 (client: 463a16)
CSpriteFont16a::CSpriteFont16a(const char* fname, int32_t _space) {
    CString buf = fname;
    buf += ".16a";

    this->bitmap = new CA16(buf);

    buf = fname;
    buf += ".dat";

    File2 f;
    f.Open(buf, CFile::modeRead);

    this->char_widths = new int32_t[f.GetLength() / 4];
    f.Read(this->char_widths, f.GetLength());

    f.Close();

    this->space = _space;

    this->bitmap->ResetPalette(16, 4, 0);
}

// 461000 (client: 464c10)
CSpriteFont16a::~CSpriteFont16a() {
}

// 45ffa9 (client: 463bbd)
void CSpriteFont16a::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* clr) {
    if ((align & ALIGN_RIGHT) != 0) {
        x -= this->GetStrWidth(txt);
    }

    if ((align & ALIGN_HCENTER) != 0) {
        x -= this->GetStrWidth(txt) / 2;
    }

    if ((align & ALIGN_BOTTOM) != 0) {
        y -= this->bitmap->GetWidth();
    }

    if ((align & ALIGN_VCENTER) != 0) {
        y -= this->bitmap->GetWidth() / 2;
    }

    int32_t tlen = strlen(txt);
    for (int32_t i = 0; i < tlen; i++) {
        uint8_t chr1 = DecodeChar(txt[i]) - 0x20;
        if (chr1 == 0) {
            x += this->bitmap->GetHeight() / 2;
        } else {
            ((CA16*)this->bitmap)->Draw(x, y, chr1, (int)clr, 0); //FIXME
        }

        x += this->char_widths[chr1] + this->space;
    }
}

// 460106 (client: 463d18)
uint16_t* CSpriteFont16a::GetShadowColors() {
    return palette_black->GetPalette(0);
}












// 47819d (client: 47bedd)
void LoadGraphics() {
        g_bmp_crystalr = new CBmp64("graphics\\interface\\CrystalR.bmp");
        g_bmp_crystall = new CBmp64("graphics\\interface\\CrystalL.bmp");
        g_bmp_headsr = new CBmp64("graphics\\interface\\HeadsR.bmp");
        g_bmp_headsl = new CBmp64("graphics\\interface\\HeadsL.bmp");

        g_mousept.Update();

        g_bmp_cmdbarr = new CBmp64("graphics\\interface\\CommandBarR.bmp");
        g_bmp_cmdbarl = new CBmp64("graphics\\interface\\CommandBarL.bmp");
        g_bmp_cmddnr = new CBmp64("graphics\\interface\\CommandDnR.bmp");
        g_bmp_cmdempr = new CBmp64("graphics\\interface\\CommandEmpR.bmp");
        g_bmp_humanbackl = new CBmp64("graphics\\interface\\HumanBackL.bmp");

        g_mousept.Update();

        g_bmp_humanbackr = new CBmp64("graphics\\interface\\HumanBackR.bmp");
        g_bmp_textbackl = new CBmp64("graphics\\interface\\TextBackL.bmp");
        g_bmp_textbackr = new CBmp64("graphics\\interface\\TextBackR.bmp");
        g_bmp_bookopened = new CBmp64("graphics\\interface\\BookOpened.bmp");
        g_bmp_bookclosed = new CBmp64("graphics\\interface\\BookClosed.bmp");
        g_bmp_backpackop = new CBmp64("graphics\\interface\\BackPackOp.bmp");
        g_bmp_backpackcl = new CBmp64("graphics\\interface\\BackPackCl.bmp");

        g_mousept.Update();

        g_bmp_humanmode = new CBmp64("graphics\\interface\\HumanMode.bmp");
        g_bmp_textmode = new CBmp64("graphics\\interface\\TextMode.bmp");
        g_bmp_diskette = new CBmp64("graphics\\interface\\diskette.bmp");
        g_bmp_ar1 = new CBmp64("graphics\\interface\\ar1.bmp");
        g_bmp_ar2 = new CBmp64("graphics\\interface\\ar2.bmp");
        g_bmp_ar3 = new CBmp64("graphics\\interface\\ar3.bmp");
        g_bmp_ar4 = new CBmp64("graphics\\interface\\ar4.bmp");

        g_mousept.Update();

        g_bmp_spellbook = new CBmp64("graphics\\interface\\SpellBook.bmp");
        g_bmp_spellback = new CBmp64("graphics\\interface\\SpellBack.bmp");
        gfx_ball = new CBmp64("graphics\\interface\\Ball.bmp");

        g_spr_backpack = new CSprite256("graphics\\backpack\\sprites.256");
        g_spr_backpackb = new CSprite256("graphics\\backpack\\spritesb.256");
        g_spr_backpack->ResetPalette(0x10, 2, 1);

        g_bmp_invframe = new CBmp64("graphics\\interface\\InvFrame.bmp");
        g_bmp_invarrow1 = new CBmp64("graphics\\interface\\InvArrow1.bmp");
        g_bmp_invarrow2 = new CBmp64("graphics\\interface\\InvArrow2.bmp");
        g_bmp_invarrow3 = new CBmp64("graphics\\interface\\InvArrow3.bmp");
        g_bmp_invarrow4 = new CBmp64("graphics\\interface\\InvArrow4.bmp");
        g_bmp_backinv = new CBmp64("graphics\\interface\\backinv.bmp");

        g_mousept.Update();

#ifdef A2CLIENT_PATCH
        if (g_ScreenSize.bottom > 480) {
            g_bmp_extra800l = new CBmp64("graphics\\interface\\extra800l.bmp");
            g_bmp_extra800r = new CBmp64("graphics\\interface\\extra800r.bmp");
            g_bmp_extra1024l = new CBmp64("graphics\\interface\\extra1024l.bmp");
            g_bmp_extra1024r = new CBmp64("graphics\\interface\\extra1024r.bmp");
        }
        if (g_ScreenSize.right > 800) {
            g_bmp_spb1024l = new CBmp64("graphics\\interface\\spb1024l.bmp");
            g_bmp_spb1024r = new CBmp64("graphics\\interface\\spb1024r.bmp");
            g_bmp_inv1024l = new CBmp64("graphics\\interface\\inv1024l.bmp");
            g_bmp_inv1024r = new CBmp64("graphics\\interface\\inv1024r.bmp");
        } else if (g_ScreenSize.right > 640) {
            g_bmp_spb800l = new CBmp64("graphics\\interface\\spb800l.bmp");
            g_bmp_spb800r = new CBmp64("graphics\\interface\\spb800r.bmp");
        }
#else
        if (g_ScreenSize.bottom < 601) {
            if (g_ScreenSize.bottom > 480) {
                g_bmp_spb800l = new CBmp64("graphics\\interface\\spb800l.bmp");
                g_bmp_spb800r = new CBmp64("graphics\\interface\\spb800r.bmp");
                g_bmp_extra800l = new CBmp64("graphics\\interface\\extra800l.bmp");
                g_bmp_extra800r = new CBmp64("graphics\\interface\\extra800r.bmp");
            }
        } else {
            g_bmp_spb1024l = new CBmp64("graphics\\interface\\spb1024l.bmp");
            g_bmp_spb1024r = new CBmp64("graphics\\interface\\spb1024r.bmp");
            g_bmp_extra1024l = new CBmp64("graphics\\interface\\extra1024l.bmp");
            g_bmp_extra1024r = new CBmp64("graphics\\interface\\extra1024r.bmp");
            g_bmp_inv1024l = new CBmp64("graphics\\interface\\inv1024l.bmp");
            g_bmp_inv1024r = new CBmp64("graphics\\interface\\inv1024r.bmp");
        }
#endif

        gfx_interface_lm = new CSprite256("graphics\\interface\\lm.256");
        gfx_interface_lm->ResetPalette(1, 1, 0);

        g_mousept.Update();

        gfx_scrollbars = new CSprite256("graphics\\interface\\scrlbars.256");
        gfx_scrollbars->ResetPalette(1, 1, 0);

        gfx_radiob = new CSprite256("graphics\\interface\\radiob.256");
        gfx_radiob->ResetPalette(1, 1, 0);

        g_ca16_money = new CA16("graphics\\interface\\money\\money.16a");
        g_ca16_money->ResetPalette(0x10, 4, 0);

        g_mousept.Update();

        g_spr_t_border = new CSprite256("graphics\\interface\\t_border.256");
        g_spr_t_border->ResetPalette(1, 1, 0);

        g_bmp_t_back = new CBmp64("graphics\\interface\\t_back.bmp");
        g_spr_backm = new CSprite256("graphics\\interface\\heroback\\backm.256");
        g_spr_backm->ResetPalette(1, 1, 0);

        g_spr_backf = new CSprite256("graphics\\interface\\heroback\\backf.256");
        g_spr_backf->ResetPalette(1, 1, 0);

        g_mousept.Update();

        g_bmp_server = new CBmp64("graphics\\interface\\server.bmp");
        g_bmp_minimapdata = new CBmp64("graphics\\interface\\MiniMapData.bmp");

        g_bmp_testiva = new CBmp256("graphics\\interface\\testiva.bmp");
        g_bmp_testiva->ResetPalette(0x10, 2, 0);
}


// 47961c (client: 47d439)
void UnloadGraphics() {
    if (g_bmp_crystalr) {
        delete g_bmp_crystalr;
    }
    if (g_bmp_crystall) {
        delete g_bmp_crystall;
    }
    if (g_bmp_headsr) {
        delete g_bmp_headsr;
    }
    if (g_bmp_headsl) {
        delete g_bmp_headsl;
    }
    if (g_bmp_cmdbarr) {
        delete g_bmp_cmdbarr;
    }
    if (g_bmp_cmdbarl) {
        delete g_bmp_cmdbarl;
    }
    if (g_bmp_cmddnr) {
        delete g_bmp_cmddnr;
    }
    if (g_bmp_cmdempr) {
        delete g_bmp_cmdempr;
    }
    if (g_bmp_humanbackl) {
        delete g_bmp_humanbackl;
    }
    if (g_bmp_humanbackr) {
        delete g_bmp_humanbackr;
    }
    if (g_bmp_textbackl) {
        delete g_bmp_textbackl;
    }
    if (g_bmp_textbackr) {
        delete g_bmp_textbackr;
    }
    if (g_bmp_bookopened) {
        delete g_bmp_bookopened;
    }
    if (g_bmp_bookclosed) {
        delete g_bmp_bookclosed;
    }
    if (g_bmp_backpackop) {
        delete g_bmp_backpackop;
    }
    if (g_bmp_backpackcl) {
        delete g_bmp_backpackcl;
    }
    if (g_bmp_humanmode) {
        delete g_bmp_humanmode;
    }
    if (g_bmp_textmode) {
        delete g_bmp_textmode;
    }
    if (g_bmp_diskette) {
        delete g_bmp_diskette;
    }
    if (g_bmp_ar1) {
        delete g_bmp_ar1;
    }
    if (g_bmp_ar2) {
        delete g_bmp_ar2;
    }
    if (g_bmp_ar3) {
        delete g_bmp_ar3;
    }
    if (g_bmp_ar4) {
        delete g_bmp_ar4;
    }
    if (g_bmp_spellbook) {
        delete g_bmp_spellbook;
    }
    if (g_bmp_spellback) {
        delete g_bmp_spellback;
    }
    if (g_bmp_invframe) {
        delete g_bmp_invframe;
    }
    if (g_bmp_invarrow1) {
        delete g_bmp_invarrow1;
    }
    if (g_bmp_invarrow2) {
        delete g_bmp_invarrow2;
    }
    if (g_bmp_invarrow3) {
        delete g_bmp_invarrow3;
    }
    if (g_bmp_invarrow4) {
        delete g_bmp_invarrow4;
    }
    if (g_bmp_backinv) {
        delete g_bmp_backinv;
    }
    if (g_bmp_spb1024l) {
        delete g_bmp_spb1024l;
    }
    if (g_bmp_spb1024r) {
        delete g_bmp_spb1024r;
    }
    if (g_bmp_extra1024l) {
        delete g_bmp_extra1024l;
    }
    if (g_bmp_extra1024r) {
        delete g_bmp_extra1024r;
    }
    if (g_bmp_inv1024l) {
        delete g_bmp_inv1024l;
    }
    if (g_bmp_inv1024r) {
        delete g_bmp_inv1024r;
    }
    if (g_bmp_spb800l) {
        delete g_bmp_spb800l;
    }
    if (g_bmp_spb800r) {
        delete g_bmp_spb800r;
    }
    if (g_bmp_extra800l) {
        delete g_bmp_extra800l;
    }
    if (g_bmp_extra800r) {
        delete g_bmp_extra800r;
    }
    if (gfx_interface_lm) {
        delete gfx_interface_lm;
    }
    if (gfx_scrollbars) {
        delete gfx_scrollbars;
    }
    if (gfx_radiob) {
        delete gfx_radiob;
    }
    if (g_ca16_money) {
        delete g_ca16_money;
    }
    if (g_spr_t_border) {
        delete g_spr_t_border;
    }
    if (g_bmp_t_back) {
        delete g_bmp_t_back;
    }
    if (g_spr_backm) {
        delete g_spr_backm;
    }
    if (g_spr_backf) {
        delete g_spr_backf;
    }
    if (g_bmp_minimapdata) {
        delete g_bmp_minimapdata;
    }
    if (g_bmp_server) {
        delete g_bmp_server;
    }
    if (gfx_ball) {
        delete gfx_ball;
    }
    if (g_spr_backpack) {
        delete g_spr_backpack;
    }
    if (g_spr_backpackb) {
        delete g_spr_backpackb;
    }
    if (g_bmp_testiva) {
        delete g_bmp_testiva;
    }

    g_bmp_crystalr = nullptr;
    g_bmp_crystall = nullptr;
    g_bmp_headsr = nullptr;
    g_bmp_headsl = nullptr;
    g_bmp_cmdbarr = nullptr;
    g_bmp_cmdbarl = nullptr;
    g_bmp_cmddnr = nullptr;
    g_bmp_cmdempr = nullptr;
    g_bmp_humanbackl = nullptr;
    g_bmp_humanbackr = nullptr;
    g_bmp_textbackl = nullptr;
    g_bmp_textbackr = nullptr;
    g_bmp_bookopened = nullptr;
    g_bmp_bookclosed = nullptr;
    g_bmp_backpackop = nullptr;
    g_bmp_backpackcl = nullptr;
    g_bmp_humanmode = nullptr;
    g_bmp_textmode = nullptr;
    g_bmp_diskette = nullptr;
    g_bmp_ar1 = nullptr;
    g_bmp_ar2 = nullptr;
    g_bmp_ar3 = nullptr;
    g_bmp_ar4 = nullptr;
    g_bmp_spellbook = nullptr;
    g_bmp_spellback = nullptr;
    g_bmp_invframe = nullptr;
    g_bmp_invarrow1 = nullptr;
    g_bmp_invarrow2 = nullptr;
    g_bmp_invarrow3 = nullptr;
    g_bmp_invarrow4 = nullptr;
    g_bmp_backinv = nullptr;
    g_bmp_spb1024l = nullptr;
    g_bmp_spb1024r = nullptr;
    g_bmp_extra1024l = nullptr;
    g_bmp_extra1024r = nullptr;
    g_bmp_inv1024l = nullptr;
    g_bmp_inv1024r = nullptr;
    g_bmp_spb800l = nullptr;
    g_bmp_spb800r = nullptr;
    g_bmp_extra800l = nullptr;
    g_bmp_extra800r = nullptr;
    gfx_interface_lm = nullptr;
    gfx_scrollbars = nullptr;
    gfx_radiob = nullptr;
    g_ca16_money = nullptr;
    g_spr_t_border = nullptr;
    g_bmp_t_back = nullptr;
    g_spr_backm = nullptr;
    g_spr_backf = nullptr;
    g_bmp_minimapdata = nullptr;
    g_bmp_server = nullptr;
    gfx_ball = nullptr;
    g_spr_backpack = nullptr;
    g_spr_backpackb = nullptr;
    g_bmp_testiva = nullptr;
}

// 47c6cd (client: 480587)
void UnloadUnitGfxFiles() {
    for (int32_t i = 0; i < g_UnitGfxFiles.GetSize(); i++) {
        UnitGfxFile* gfx = g_UnitGfxFiles[i];
        if (gfx != nullptr) {
            delete gfx;
        }
    }
    for (int32_t i = 0; i < g_VFX_info.GetSize(); i++) {
        UnitVFXUnfo* vfx = g_VFX_info[i];
        if (vfx != nullptr) {
            delete vfx;
        }
    }
    g_UnitGfxFiles.RemoveAll();
    g_VFX_info.RemoveAll();
    for (int32_t i = 0; i < g_Human_palettes.GetSize(); i++) {
        CGamePalette* pal = g_Human_palettes[i];
        if (pal != nullptr) {
            delete pal;
        }
    }
    g_Human_palettes.RemoveAll();
}

// 460c37 (client: 464849)
void LoadFonts() {
    g_font1 = new CSpriteFont16("graphics\\font1\\font1", 2);
    g_font2 = new CSpriteFont16("graphics\\font2\\font2", 2);
    g_font3 = new CSpriteFont16("graphics\\font3\\font3", 2);
    g_font4 = new CSpriteFont16a("graphics\\font4\\font4", 2);
}

// 460d7f (client: 464991)
void FreeFontData() {
    if (g_font1) {
        delete g_font1;
    }
    if (g_font2) {
        delete g_font2;
    }
    if (g_font3) {
        delete g_font3;
    }
    if (g_font4) {
        delete g_font4;
    }
    if (palette_paris_daisy) {
        delete palette_paris_daisy;
    }
    if (palette_husk) {
        delete palette_husk;
    }
    if (palette_black) {
        delete palette_black;
    }
    if (palette_brown_derby) {
        delete palette_brown_derby;
    }
    if (palette_tawny_port) {
        delete palette_tawny_port;
    }

    g_font2 = nullptr;
    g_font3 = nullptr;
    g_font4 = nullptr;
    palette_paris_daisy = nullptr;
    palette_husk = nullptr;
    palette_black = nullptr;
    palette_brown_derby = nullptr;
    palette_tawny_port = nullptr;
}

// 454219 (client: 457e59)
void FreeDDraw() {
#ifdef A2CLIENT_PATCH
    if (g_IsServer == 0) {
        IDirectDrawSurface* surf = g_ddsurface;
        g_ddsurface = nullptr;
        if (surf != nullptr) {
            surf->Release();
        }

        IDirectDraw* ddraw = g_ddraw;
        g_ddraw = nullptr;
        if (ddraw != nullptr) {
            ddraw->Release();
        }

        delete[] g_surf1_buf;
        g_surf1_buf = nullptr;
        delete[] g_surf2_buf;
        g_surf2_buf = nullptr;
        delete[] g_surf3_buf;
        g_surf3_buf = nullptr;
        FreeScaleTables();
    }
#else
    if (g_IsServer == 0 && g_ddraw) {
        g_ddraw->Release();
        g_ddraw = nullptr;
    }
#endif
}


#ifdef A2CLIENT_PATCH
// Tangar SCREENSHOT_take (entry takeover of client 457973): save the frame as a
// timestamped PNG (lodepng) instead of the retail numbered BMP. The surface is
// read directly, like the patch does (no lock/unlock).
// 453d33 (client: 457973)
void SaveScreenshot() {
    int32_t si_width = g_ScreenSize.right - g_ScreenSize.left;
    int32_t si_height = g_ScreenSize.bottom - g_ScreenSize.top;

    uint32_t* ndata = new uint32_t[si_width * si_height];
    uint32_t* ndatap = ndata;

    uint8_t* sdata = g_surf1_buf != nullptr ? (uint8_t*)g_surf1_buf : (uint8_t*)g_selDrawBitmap.lpSurface;
    uint32_t spitch = g_surf1_buf != nullptr ? g_ScreenSize.right * 2 : g_selDrawBitmap.lPitch;

    for (int32_t y = 0; y < si_height; y++) {
        uint16_t* row = (uint16_t*)(sdata + y * spitch);
        for (int32_t x = 0; x < si_width; x++) {
            uint16_t clr = *row++;
            uint32_t r = (clr & 0xF800) >> 11;
            uint32_t g = (clr & 0x07E0) >> 5;
            uint32_t b = clr & 0x001F;
            r = r * 255 / 31;
            g = g * 255 / 63;
            b = b * 255 / 31;
            *ndatap++ = 0xFF000000 | (b << 16) | (g << 8) | r;
        }
    }

    unsigned char* buf = nullptr;
    size_t size = 0;
    lodepng_encode32(&buf, &size, (const unsigned char*)ndata, si_width, si_height);
    delete[] ndata;

    CString fname;
    time_t t = time(NULL);
    struct tm tmv;
    if (localtime_s(&tmv, &t) == 0) {
        fname.Format("screenshot_%02u-%02u-%04u_%02u-%02u-%02u", tmv.tm_mday, tmv.tm_mon + 1, tmv.tm_year + 1900, tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    } else {
        fname = "screenshot";
    }
    if (tangar_cfg::z_screendir) {
        fname = CString("screenshots") + CString("\\") + fname;
    }
    fname += ".png";

    FILE* outf = nullptr;
    if (fopen_s(&outf, (const char*)fname, "wb") == 0) {
        fwrite(buf, 1, size, outf);
        fclose(outf);

        const char* fmt = (txt_patch.GetCount() > 226) ? txt_patch.GetLine(226) : "Screenshot saved: %s";
        CString msg;
        msg.Format(fmt, (const char*)fname);
        tangar_cfg::WriteChat(msg);
    }
    free(buf);
}
#else
// 453d33 (client: 457973)
void SaveScreenshot() {
    static int32_t screenshot_num = 0; //65dd24
    CString fname;
    fname.Format("Allods%04d.bmp", screenshot_num);
    screenshot_num++;

    int32_t pixnum = g_ScreenSize.right * g_ScreenSize.bottom;
    uint8_t* data = new uint8_t[pixnum * 3];

    LockPrimarySurface();
    uint8_t* dst = data;
    for (int32_t y = g_ScreenSize.bottom - 1; y >= 0; y--) {
        uint16_t* src = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_ScreenSize.bottom * 2);
        for (int32_t x = 0; x < g_ScreenSize.right; x++) {
            uint16_t clr = *src;
            dst[0] = ((clr & g_BBitMask) >> g_BBitShift) << (8 - g_BBits);
            dst[1] = ((clr & g_GBitMask) >> g_GBitShift) << (8 - g_GBits);
            dst[2] = ((clr & g_RBitMask) >> g_RBitShift) << (8 - g_RBits);

            src++;
            dst += 3;
        }
    }
    UnlockPrimarySurface();

    CFile f(fname, CFile::modeWrite | CFile::modeCreate);

    BITMAPFILEHEADER bhdr;
    bhdr.bfType = 0x4d42; //BM
    bhdr.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixnum * 3;
    bhdr.bfReserved2 = 0;
    bhdr.bfReserved1 = 0;
    bhdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    f.Write(&bhdr, sizeof(BITMAPFILEHEADER));

    BITMAPINFOHEADER binf;
    binf.biSize = 0x28;
    binf.biWidth = g_ScreenSize.right;
    binf.biHeight = g_ScreenSize.bottom;
    binf.biPlanes = 1;
    binf.biBitCount = 24;
    binf.biCompression = 0;
    binf.biSizeImage = 0;
    binf.biXPelsPerMeter = 0;
    binf.biYPelsPerMeter = 0;
    binf.biClrUsed = 0;
    binf.biClrImportant = 0;

    f.Write(&binf, sizeof(BITMAPINFOHEADER));

    f.Write(data, pixnum * 3);

    f.Close();

    delete[] data;
}
#endif
