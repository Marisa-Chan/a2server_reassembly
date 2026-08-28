#include <inttypes.h>
#include <ddraw.h>
#include <stdio.h>
#include "asm_mfc.h"
#include "gfx.h"
#include "util.h"
#include "game_app.h"
#include "file.h"
#include "main_window.h"

IMPLEMENT_DYNAMIC(CGamePalette, CObject);

IMPLEMENT_DYNAMIC(CGameBitmap, CObject);
IMPLEMENT_DYNAMIC(CBmp256, CGameBitmap);
IMPLEMENT_DYNAMIC(CBmp64, CGameBitmap);
IMPLEMENT_DYNAMIC(CSprite256, CGameBitmap);
IMPLEMENT_DYNAMIC(CA16, CSprite256);
IMPLEMENT_DYNAMIC(CSprite16, CGameBitmap);

int32_t gfx_bits = 16; //65fb74

IDirectDraw* g_ddraw = nullptr; //659bec
IDirectDrawSurface* g_ddsurface = nullptr; //659c90
int32_t g_ddsurf_lock = 0; //659bfc
IDirectDrawSurface* g_ddsurface2 = nullptr; //659c8c
int32_t g_ddsurf_lock2 = 0; //659bf4
IDirectDrawSurface* g_ddsurface3 = nullptr; //659a68
int32_t g_ddsurf_lock3 = 0; //659bf8

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



//454a0c
void GetClipRect(CRect* r)
{
	*r = g_clipRect;
}

//4549ba
void SetClipRect(CRect& r)
{
	g_clipRect = r;
}


void __cdecl Convert24To16(uint8_t* src, uint16_t* dst, int32_t num)
{ //4255a6
	for (; num; num--)
	{
		*dst = (uint16_t)((src[2] >> (8 - g_RBits)) << g_RBitShift) |
			(uint16_t)((src[1] >> (8 - g_GBits)) << g_GBitShift) |
			(uint16_t)((src[0] >> (8 - g_BBits)) << g_BBitShift);

		src += 3;
		dst++;
	}
}

void __cdecl Convert16To24(uint16_t* src, uint8_t* dst, int32_t num)
{ //425688
	for (; num; num--)
	{
		uint16_t c = *src;
		dst[0] = (c >> g_BBitShift) << (8 - g_BBits);
		dst[1] = (c >> g_GBitShift) << (8 - g_GBits);
		dst[2] = (c >> g_RBitShift) << (8 - g_RBits);

		src++;
		dst += 3;
	}
}

void BlitPaletted(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height, const uint16_t *palette)
{ //4553db
	int32_t w = r - l;
	int32_t h = b - t;

	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		l += g_clipRect.left - x;
		x = g_clipRect.left;
	}

	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		t += g_clipRect.top - y;
		y = g_clipRect.top;
	}

	if (x + w >= g_clipRect.right)
		w = g_clipRect.right - x;

	if (y + h >= g_clipRect.bottom)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	//MMX optimise omitted

	w = l + w;
	h = t + h;

	uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2);
	uint8_t* src = src_data + l + t * src_width;

	while (t < h)
	{
		for (int32_t n = w - l; n; n--)
		{
			*dst = palette[*src];
			src++;
			dst++;
		}
		dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch - (w - l) * 2);
		src = src + src_width - (w - l);
	}
}

void BlitFlipPalettedAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height, const uint16_t* palette)
{ //4555f9
	int32_t w = r - l;
	int32_t h = b - t;

	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		l += g_clipRect.left - x;
		x = g_clipRect.left;
	}

	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		t += g_clipRect.top - y;
		y = g_clipRect.top;
	}

	if (x + w >= g_clipRect.right)
		w = g_clipRect.right - x;

	if (y + h >= g_clipRect.bottom)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	h = t + h;

	uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2);
	uint8_t* src = src_data + l + (src_height - 1 - t) * src_width;

	while (t < h)
	{
		for (int32_t n = w; n; n--)
		{
			*dst += palette[*src];
			src++;
			dst++;
		}
		dst = (uint16_t*)((uint8_t*)dst + g_selDrawBitmap.lPitch - w * 2);
		src = src - src_width - w;
	}
}


void Blit16(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height)
{ //454f6d
	int32_t w = r - l;
	int32_t h = b - t;

	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		l += g_clipRect.left - x;
		x = g_clipRect.left;
	}
	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		t += g_clipRect.top - y;
		y = g_clipRect.top;
	}
	if (x + w >= g_clipRect.right)
		w = g_clipRect.right - x;
	
	if (y + h >= g_clipRect.bottom)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
	uint8_t* src = src_data + (((src_height - 1) - t) * src_width + l) * 2;
	for (int32_t yy = t; yy < t + h; yy++)
	{
		memcpy(dst, src, w * 2);
		dst += g_selDrawBitmap.lPitch;
		src -= src_width * 2;
	}
}

void Blit16Masked(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height)
{ //45525a
	int32_t w = r - l;
	int32_t h = b - t;

	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		l += g_clipRect.left - x;
		x = g_clipRect.left;
	}
	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		t += g_clipRect.top - y;
		y = g_clipRect.top;
	}
	if (x + w >= g_clipRect.right)
		w = g_clipRect.right - x;

	if (y + h >= g_clipRect.bottom)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
	uint16_t* src = (uint16_t*)(src_data + (((src_height - 1) - t) * src_width + l) * 2);
	for (; h; h--)
	{
		for (int32_t ww = w; ww; ww--)
		{
			if (*src != 0)
				*(uint16_t*)dst = *src;

			src++;
			dst += 2;
		}

		dst += g_selDrawBitmap.lPitch - w * 2;
		src -= src_width + w;
	}
}

void Blit16Add(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, uint8_t* src_data, int32_t src_width, int32_t src_height)
{ //4550e8
	int32_t w = r - l;
	int32_t h = b - t;

	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		l += g_clipRect.left - x;
		x = g_clipRect.left;
	}
	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		t += g_clipRect.top - y;
		y = g_clipRect.top;
	}
	if (x + w >= g_clipRect.right)
		w = g_clipRect.right - x;

	if (y + h >= g_clipRect.bottom)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
	uint16_t* src = (uint16_t*)(src_data + (((src_height - 1) - t) * src_width + l) * 2);
	for (; h; h--)
	{
		for (int32_t ww = w; ww; ww--)
		{
			*(uint16_t*)dst += *src;
			src++;
			dst += 2;
		}

		dst += g_selDrawBitmap.lPitch - w * 2;
		src -= src_width + w;
	}
}



void __cdecl BlitRle8(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr)
{ // 4559d4
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t *)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = clr[*src];
							src++;
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					for (int i = count / 2; i > 0; i--)
					{
						*(uint32_t*)dst = (clr[src[1]] << 16) | clr[src[0]];
						src += 2;
						dst += 2;
					}
					if (count & 1)
					{
						*dst = clr[*src];
						src++;
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}


void __cdecl BlitRle8FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr)
{ //456a0d
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = clr[*src];
							src++;
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx -= count;
				}
			}
			xx = x + w - 1;
			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					for (int i = count; i > 0; i--)
					{
						*dst = clr[ *src ];
						src++;
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			h--;
		}
	}
}


void __cdecl BlitLightRle8(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl)
{ //45614c
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = g_brightnessLookup[bl + *dst];
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + *dst];
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}


void __cdecl BlitLightRle8Low(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl)
{ //455f1c
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = g_brightnessLookup[bl + (*dst >> 3)];
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + (*dst >> 3)];
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}

void __cdecl BlitLightRle8FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl)
{ //457141
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = g_brightnessLookup[bl + *dst];
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx -= count;
				}
			}
			xx = x + w - 1;
			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + *dst];
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			h--;
		}
	}
}

void __cdecl BlitLightRle8FlipXLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl)
{ //457141
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = g_brightnessLookup[bl + (*dst >> 3)];
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx -= count;
				}
			}
			xx = x + w - 1;
			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + (*dst >> 3)];
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			h--;
		}
	}
}


void __cdecl BlitRle8Blend(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr)
{ // 455ce6
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
							src++;
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					for (int i = count; i > 0; i--)
					{
						*dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
						src++;
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}


void __cdecl BlitRle8BlendFlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr)
{ // 456ca7
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						while (count)
						{
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
							src++;
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx -= count;
				}
			}
			xx = x + w - 1;
			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					for (int i = count; i > 0; i--)
					{
						*dst = ((clr[*src] >> 1) & g_ColorAddMask) + ((*dst >> 1) & g_ColorAddMask);
						src++;
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst -= count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			h--;
		}
	}
}



void __cdecl BlitLightRle8Tilt(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt)
{ //4566ff
	int32_t off = (int16_t)((tilt * h) >> 16);
	int32_t aoff = abs(off);

	if (x + w + aoff <= g_clipRect.left)
		return;
	if (x - aoff >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	int32_t tl = 0;

	if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right)
								*dst = g_brightnessLookup[bl + *dst];
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);
					off -= tmp;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + *dst];
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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

void __cdecl BlitLightRle8TiltLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt)
{ //4563e7
	int32_t off = (int16_t)((tilt * h) >> 16);
	int32_t aoff = abs(off);

	if (x + w + aoff <= g_clipRect.left)
		return;
	if (x - aoff >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	int32_t tl = 0;

	if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right)
								*dst = g_brightnessLookup[bl + (*dst >> 3)];
							dst++;
							xx++;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);
					off -= tmp;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + (*dst >> 3)];
						dst++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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


void __cdecl BlitLightRle8TiltFlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt)
{ //4576b1
	int32_t off = (int16_t)((tilt * h) >> 16);
	int32_t aoff = abs(off);

	if (x + w + aoff <= g_clipRect.left)
		return;
	if (x - aoff >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	int32_t tl = 0;

	if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right)
								*dst = g_brightnessLookup[bl + *dst];
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);
					off -= tmp;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + *dst];
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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

void __cdecl BlitLightRle8TiltFlipXLow(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, int32_t blvl, int32_t tilt)
{ //457380
	int32_t off = (int16_t)((tilt * h) >> 16);
	int32_t aoff = abs(off);

	if (x + w + aoff <= g_clipRect.left)
		return;
	if (x - aoff >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;
	int32_t bl = blvl * g_brightnessLookupCount;

	int32_t tl = 0;

	if (x - aoff < g_clipRect.left || x + w + aoff > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						src += count;
						while (count)
						{
							if (xx + off >= g_clipRect.left && xx + off < g_clipRect.right)
								*dst = g_brightnessLookup[bl + (*dst >> 3)];
							dst--;
							xx--;
							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);
					off -= tmp;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (off + x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					xx += count;
					src += count;
					for (int i = count; i > 0; i--)
					{
						*dst = g_brightnessLookup[bl + (*dst >> 3)];
						dst--;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					int32_t tmp = tl + count * tilt;
					tl = tmp & 0xffff;
					tmp = (int16_t)(tmp >> 16);

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch - tmp * 2);
					h -= count;
				}
				else
				{
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



void __cdecl BlitRle8IndexMask(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, uint8_t mv)
{ // 455c55
	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	uint8_t* dst = (uint8_t*)g_selDrawBitmap.lpSurface + x + y * dstpitch;
	while (h > 0)
	{
		int32_t xx = 0;
		while (xx < w)
		{
			const uint8_t b = *src;
			src++;
			const uint8_t flags = b & 0xc0;
			const uint8_t count = b & 0x3f;
			if (flags == 0)
			{
				xx += count;
				src += count;

				for (int i = count; i > 0; i--)
				{
					*dst = mv;
					dst++;
				}
			}
			else if (flags == 0x40)
			{
				if (h <= count)
					return;

				dst = (uint8_t*)dst + count * dstpitch;
				h -= count;
			}
			else
			{
				dst += count;
				xx += count;
			}
		}

		dst = (uint8_t*)dst - w + dstpitch;
		h--;
	}
}


void __cdecl BlitRle16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* src, const uint16_t* clr)
{ // 
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint16_t b = *src;
				src++;
				const uint16_t flags = b & 0xc000;
				uint16_t count = b & 0x3fff;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count;
						xx += count;
					}
					else
					{
						if (g_isLowMemory == 0)
						{
							while (count)
							{
								if (xx >= g_clipRect.left && xx < g_clipRect.right)
								{
									const uint16_t px = *src;
									*dst = clr[px / 2] + g_brightnessLookup[g_brightnessLookupCount + ((px & 0x1e00) << (8 - 1)) + *dst];
								}
								src++;
								dst++;
								xx++;
								count--;
							}
						}
						else
						{
							while (count)
							{
								if (xx >= g_clipRect.left && xx < g_clipRect.right)
								{
									const uint16_t px = *src;
									*dst = clr[px / 2] + g_brightnessLookup[((px & 0x1e00) << (8 - 1 - 3)) + (*dst >> 3)];
								}
								src++;
								dst++;
								xx++;
								count--;
							}
						}
					}
				}
				else if (flags == 0x4000)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else if (flags == 0x8000)
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint16_t b = *src;
				src++;
				const uint16_t flags = b & 0xc000;
				const uint16_t count = b & 0x3fff;
				if (flags == 0)
				{
					xx += count;
					if (g_isLowMemory == 0)
					{
						for (int i = count; i > 0; i--)
						{
							const uint16_t px = *src;
							*dst = clr[px / 2] + g_brightnessLookup[g_brightnessLookupCount + ((px & 0x1e00) << (8 - 1)) + *dst];
							src++;
							dst++;
						}
					}
					else
					{
						for (int i = count; i > 0; i--)
						{
							const uint16_t px = *src;
							*dst = clr[px / 2] + g_brightnessLookup[((px & 0x1e00) << (8 - 1 - 3)) + (*dst >> 3)];
							src++;
							dst++;
						}
					}
				}
				else if (flags == 0x4000)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else if (flags == 0x8000)
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}

void __cdecl BlitRle16FlipX(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* src, const uint16_t* clr)
{ //
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x + w - 1;
		while (h != 0)
		{
			while (xx >= x)
			{
				const uint16_t b = *src;
				src++;
				const uint16_t flags = b & 0xc000;
				uint16_t count = b & 0x3fff;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst -= count;
						xx -= count;
					}
					else
					{
						if (g_isLowMemory == 0)
						{
							while (count)
							{
								if (xx >= g_clipRect.left && xx < g_clipRect.right)
								{
									const uint16_t px = *src;
									*dst = clr[px / 2] + g_brightnessLookup[g_brightnessLookupCount + ((px & 0x1e00) << (8 - 1)) + *dst];
								}
								src++;
								dst--;
								xx--;
								count--;
							}
						}
						else
						{
							while (count)
							{
								if (xx >= g_clipRect.left && xx < g_clipRect.right)
								{
									const uint16_t px = *src;
									*dst = clr[px / 2] + g_brightnessLookup[((px & 0x1e00) << (8 - 1 - 3)) + (*dst >> 3)];
								}
								src++;
								dst--;
								xx--;
								count--;
							}
						}
					}
				}
				else if (flags == 0x4000)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else if (flags == 0x8000)
				{
					dst -= count;
					xx -= count;
				}
			}
			xx = x + w - 1;
			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + (x + w - 1) * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint16_t b = *src;
				src++;
				const uint16_t flags = b & 0xc000;
				const uint16_t count = b & 0x3fff;
				if (flags == 0)
				{
					xx += count;
					if (g_isLowMemory == 0)
					{
						for (int i = count; i > 0; i--)
						{
							const uint16_t px = *src;
							*dst = clr[px / 2] + g_brightnessLookup[g_brightnessLookupCount + ((px & 0x1e00) << (8 - 1)) + *dst];
							src++;
							dst--;
						}
					}
					else
					{
						for (int i = count; i > 0; i--)
						{
							const uint16_t px = *src;
							*dst = clr[px / 2] + g_brightnessLookup[((px & 0x1e00) << (8 - 1 - 3)) + (*dst >> 3)];
							src++;
							dst--;
						}
					}
				}
				else if (flags == 0x4000)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else if(flags == 0x8000)
				{
					dst -= count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst + w * 2 + dstpitch);
			h--;
		}
	}
}

void __cdecl BlitRle4(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, const uint16_t* clr)
{ // 455761
	if (x + w <= g_clipRect.left)
		return;
	if (x >= g_clipRect.right)
		return;
	if (y + h <= g_clipRect.top)
		return;
	if (y >= g_clipRect.bottom)
		return;

	const uint32_t dstpitch = g_selDrawBitmap.lPitch;

	if (x < g_clipRect.left || x + w > g_clipRect.right || y < g_clipRect.top || y + h > g_clipRect.bottom)
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		int32_t yy = y;
		int32_t xx = x;
		while (h != 0)
		{
			while (xx - x < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					if (yy < g_clipRect.top || yy >= g_clipRect.bottom)
					{
						src += count;
						dst += count * 2;
						xx += count * 2;

						if ((src[-1] & 0xf0) == 0)
						{
							dst--;
							xx--;
						}
					}
					else
					{
						while (count)
						{
							uint8_t bb = *src;
							src++;

							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = clr[bb & 0xf];
							dst++;
							xx++;

							bb >>= 4;
							if (bb == 0)
								break;
							if (xx >= g_clipRect.left && xx < g_clipRect.right)
								*dst = clr[bb];
							dst++;
							xx++;

							count--;
						}
					}
				}
				else if (flags == 0x40)
				{
					yy += count;
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}
			xx = x;
			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			yy++;
			h--;
		}
	}
	else
	{
		uint16_t* dst = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + x * 2 + y * dstpitch);
		while (h > 0)
		{
			int32_t xx = 0;
			while (xx < w)
			{
				const uint8_t b = *src;
				src++;
				const uint8_t flags = b & 0xc0;
				const uint8_t count = b & 0x3f;
				if (flags == 0)
				{
					for (int i = count; i > 0; i--)
					{
						uint8_t bb = *src;
						src++;

						*dst = clr[bb & 0xf];
						dst++;
						xx++;

						bb >>= 4;
						if (bb == 0)
							break;
						*dst = clr[bb];
						dst++;
						xx++;
					}
				}
				else if (flags == 0x40)
				{
					if (h <= count)
						return;

					dst = (uint16_t*)((uint8_t*)dst + count * dstpitch);
					h -= count;
				}
				else
				{
					dst += count;
					xx += count;
				}
			}

			dst = (uint16_t*)((uint8_t*)dst - w * 2 + dstpitch);
			h--;
		}
	}
}



void __cdecl SetPixelColor(int32_t x, int32_t y, uint32_t clr)
{
	//4586a4
	if (x >= g_clipRect.left && x < g_clipRect.right &&
		y >= g_clipRect.top && y < g_clipRect.bottom)
		((uint16_t*)g_selDrawBitmap.lpSurface)[x + y * (g_selDrawBitmap.lPitch / 2)] = clr;
}

void __cdecl FillRectColor(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr)
{
	//457dcc
	uint16_t* sp = (uint16_t*)g_selDrawBitmap.lpSurface;

	int32_t w = abs(r - l);
	int32_t h = abs(b - t);

	int32_t bb;
	int32_t rr;
	int32_t dl;
	int32_t dd;
	int32_t dp;

	if (h < w)
	{
		dp = h * 2 - w;
		dl = h - w;
		rr = r;
		dd = h;
	}
	else
	{
		dp = w * 2 - h;
		dl = w - h;
		bb = b;
		dd = w;
	}

	dd *= 2;
	dl *= 2;

	int32_t dx = 1;
	if (r < l)
		dx = -1;

	int32_t dy = 1;
	if (b < t)
		dy = -1;

	int32_t xx = l;
	int32_t yy = t;

	if (l >= g_clipRect.left && l < g_clipRect.right &&
		t >= g_clipRect.top && t < g_clipRect.bottom)
		sp[l + t * (g_selDrawBitmap.lPitch / 2)] = clr;

	if (h < w)
	{
		while (xx != rr)
		{
			xx += dx;
			w = dd;
			if (dp > -1)
			{
				yy += dy;
				w = dl;
			}
			dp += w;

			if (xx >= g_clipRect.left && xx < g_clipRect.right &&
				yy >= g_clipRect.top && yy < g_clipRect.bottom)
				sp[xx + yy * (g_selDrawBitmap.lPitch / 2)] = clr;
		}
	}
	else
	{
		while (yy != bb)
		{
			yy += dy;
			w = dd;
			if (dp > -1)
			{
				xx += dx;
				w = dl;
			}
			dp += w;

			if (xx >= g_clipRect.left && xx < g_clipRect.right &&
				yy >= g_clipRect.top && yy < g_clipRect.bottom)
				sp[xx + yy * (g_selDrawBitmap.lPitch / 2)] = clr;
		}
	}
}

void __cdecl FillRectColorSimple(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr)
{
	//4579d8
	if (g_IsServer != 0)
		return;

	if (l < g_clipRect.left)
		l = g_clipRect.left;
	if (r > g_clipRect.right)
		r = g_clipRect.right;
	if (t < g_clipRect.top)
		t = g_clipRect.top;
	if (b > g_clipRect.bottom)
		b = g_clipRect.bottom;

	const uint32_t dcolor = (clr << 16) | clr;

	for (int32_t y = t; y < b; y++)
	{
		uint16_t* dout = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + l * 2);

		int32_t num = r - l;

		while (num >= 2)
		{
			*(uint32_t*)dout = dcolor;
			dout += 2;
			num -= 2;
		}

		if (num == 1)
			*dout = clr;
	}
}

void __cdecl ShadowRect(CRect rect, int shadow)
{
	//457b6f
	uint16_t* lookup = g_brightnessLookup + shadow * g_brightnessLookupCount;

	rect.IntersectRect(rect, g_clipRect);

	if (rect.Width() == 0 || rect.Height() == 0)
		return;

	uint16_t* px = (uint16_t*)g_selDrawBitmap.lpSurface + (g_selDrawBitmap.lPitch / 2) * rect.top + rect.left;
	for (int32_t y = rect.top; y < rect.bottom; y++)
	{
		uint16_t* row = px;
		for (int32_t x = rect.left; x < rect.right; x++)
		{
			if (g_isLowMemory)
				*row = lookup[(*row) >> 3];
			else
				*row = lookup[*row];
			row++;
		}
		px += g_selDrawBitmap.lPitch / 2;
	}
}


void __cdecl DrawRectangleFrame(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr)
{
	//458035
	FillRectColor(l, t, r, t, clr); //up
	FillRectColor(l, b, r, b, clr); //down
	FillRectColor(r, t, r, b, clr); //right
	FillRectColor(l, t, l, b, clr); //left
}

void __cdecl CopySurface16Rect(uint8_t* src, int32_t sx, int32_t sy, uint8_t* dst, int32_t dx, int32_t dy, int32_t w, int32_t h, int32_t sp, int32_t dp)
{ //454a54
	int32_t t = g_clipRect.left - dx;
	if (t > 0)
	{
		sx += t;
		dx += t;
		w -= t;
	}
	t = (dx + w) - g_clipRect.right;
	if (t > -1)
		w -= t;

	t = g_clipRect.top - dy;
	if (t > 0)
	{
		sy += t;
		dy += t;
		h -= t;
	}

	t = (dy + h) - g_clipRect.bottom;
	if (t > -1)
		h -= t;

	if (w <= 0 || h <= 0)
		return;

	uint8_t* s = src + sy * sp + sx * 2;
	uint8_t* d = dst + dy * dp + dx * 2;

	for (; h; h--)
	{
		memcpy(d, s, w * 2);
		s += sp;
		d += dp;
	}
}

void __cdecl gfxFlushRect(const CRect& rect)
{ //454c74
	MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

	if (mwnd->field_0xbc != 0 && g_IsServer == 0)
	{
		CRect r;
		r.IntersectRect(&rect, &g_ScreenSize);

		while (g_ddsurface->IsLost() == DDERR_SURFACELOST)
		{
			g_ddraw->SetCooperativeLevel(mwnd->m_hWnd, DDSCL_EXCLUSIVE || DDSCL_FULLSCREEN);
			g_ddraw->SetDisplayMode(g_ScreenSize.right, g_ScreenSize.bottom, gfx_bits);
			g_ddsurface->Restore();
		}

		g_mousept.SaveScreenParts2(r);
		int32_t w = r.Width();
		int32_t h = r.Height();

		if (w < 1 || h < 1)
		{
			g_mousept.SaveScreenParts(r);
		}
		else
		{
			LockSurface2();

			void* sv = g_selDrawBitmap.lpSurface;

			uint8_t* src = (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2;
			int32_t spitch = g_selDrawBitmap.lPitch;

			LockSurface1();

			CopySurface16Rect(src, 0, 0, 
				             (uint8_t*)g_selDrawBitmap.lpSurface + r.top * g_selDrawBitmap.lPitch + r.left * 2, 0, 0, 
				             w, h, 
							 spitch, 
				             g_selDrawBitmap.lPitch);

			UnlockSurface1();

			g_selDrawBitmap.lpSurface = sv;

			UnlockSurface2();
			g_mousept.SaveScreenParts(r);
		}
	}
}

void __cdecl FlushScreen()
{
	//454e18
	gfxFlushRect(CRect(g_ScreenSize));
}



uint32_t LockSurface1()
{ //45431c
	if (g_IsServer)
		return 0;

	int res = 0;
	if (g_ddsurf_lock == 0)
	{
		g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
		res = SUCCEEDED( g_ddsurface->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr) );
		g_ddsurf_lock++;
	}
	return res;
}


uint32_t UnlockSurface1()
{ //454378
	if (g_IsServer)
		return 0;

	int res = 0;
	if (g_ddsurf_lock != 0)
	{
		res = SUCCEEDED(g_ddsurface->Unlock(g_selDrawBitmap.lpSurface));
		g_ddsurf_lock--;
	}
	return res;
}


uint32_t LockSurface2()
{ //45431c
	if (g_IsServer)
		return 0;

	int res = 0;
	if (g_ddsurf_lock2 == 0)
	{
		g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
		res = SUCCEEDED(g_ddsurface2->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr));
		g_ddsurf_lock2++;
	}
	return res;
}

uint32_t UnlockSurface2()
{ //4542ca
	if (g_IsServer)
		return 0;

	int res = 0;
	if (g_ddsurf_lock2 != 0)
	{
		res = SUCCEEDED(g_ddsurface2->Unlock(g_selDrawBitmap.lpSurface));
		g_ddsurf_lock2--;
	}
	return res;
}

uint32_t LockSurface3()
{ //4543ca
	if (g_IsServer)
		return 0;

	if (g_ddsurface3->IsLost() == DDERR_SURFACELOST)
		g_ddsurface3->Restore();

	int res = 0;
	if (g_ddsurf_lock3 == 0)
	{
		g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
		res = SUCCEEDED(g_ddsurface3->Lock(nullptr, &g_selDrawBitmap, DDLOCK_WAIT, nullptr));
		g_ddsurf_lock3++;
	}
	return res;
}


uint32_t UnlockSurface3()
{ //45444f
	if (g_IsServer)
		return 0;

	if (g_ddsurface3->IsLost() == DDERR_SURFACELOST)
		g_ddsurface3->Restore();

	int res = 0;
	if (g_ddsurf_lock3 != 0)
	{
		res = SUCCEEDED(g_ddsurface3->Unlock(g_selDrawBitmap.lpSurface));
		g_ddsurf_lock3--;
	}
	return res;
}

uint32_t GetLockCountSurf2() { return g_ddsurf_lock2; }//4538e1

void __cdecl CopyScreenRectToBmp64(int32_t x, int32_t y, int32_t w, int32_t h, int32_t dw, int32_t dh, uint8_t* dst)
{ //45455e
	int32_t dy = 0;
	int32_t dx = 0;
	if (x < g_clipRect.left)
	{
		w -= g_clipRect.left - x;
		dx = g_clipRect.left - x;
		x = g_clipRect.left;
	}
	if (y < g_clipRect.top)
	{
		h -= g_clipRect.top - y;
		dy = g_clipRect.top - y;
		y = g_clipRect.top;
	}
	if (g_clipRect.right <= x + w)
		w = g_clipRect.right - x;
	if (g_clipRect.bottom <= y + h)
		h = g_clipRect.bottom - y;

	if (w <= 0 || h <= 0)
		return;

	uint8_t* s = (uint8_t*)g_selDrawBitmap.lpSurface + y * g_selDrawBitmap.lPitch + x * 2;
	uint8_t* d = (uint8_t*)dst + dx * 2 + ((dh - dy) - 1) * dw * 2;
	for (int32_t yy = y; yy < y + h; yy++)
	{
		memcpy(d, s, w * 2);
		s += g_selDrawBitmap.lPitch;
		d -= dw * 2;
	}
}

int BitmaskLowBound(uint32_t bm)
{ //453f5c
	for (int32_t i = 0; i < 32; i++)
	{
		if (bm & (1 << i))
			return i;
	}
	return -1;
}

int BitmaskHighBound(uint32_t bm)
{ //453f94
	for (int32_t i = 15; i >= 0; i--)
	{
		if (bm & (1 << i))
			return i;
	}
	return -1;
}

void FreeBrightnessLookup()
{ //453cf9
	if (g_brightnessLookup)
		delete[] g_brightnessLookup;
	g_brightnessLookup = nullptr;
}

void CalculateBrightness()
{ //4538eb
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	
	g_isLowMemory = ms.dwTotalPhys < 24 * 1000 * 1000;

	int32_t maxR = 1 << g_RBits;
	int32_t maxG = 1 << g_GBits;
	int32_t maxB = 1 << g_BBits;

	if (g_isLowMemory == 0)
		g_brightnessLookupCount = 0x10000;
	else
		g_brightnessLookupCount = 0x10000 >> 3;

	g_brightnessLookup = new uint16_t[g_brightnessLookupCount * 17];

	int db = 1;
	int sb = 0;
	if (g_isLowMemory)
	{
		db = 1 << 3;
		sb = (1 << 3) / 2;
	}

	for (int32_t br = 16; br >= 0; br--)
	{
		int32_t off = g_brightnessLookupCount * (16 - br);
		for (int32_t ri = 0; ri < maxR; ri++)
		{
			for (int32_t gi = 0; gi < maxG; gi++)
			{
				for (int32_t bi = sb; bi < maxB; bi += db)
				{
					int32_t r = (ri * br / 16) << (8 - g_RBits);
					int32_t g = (gi * br / 16) << (8 - g_GBits);
					int32_t b = (bi * br / 16) << (8 - g_BBits);
					if (r > 255)
						r = 255;
					if (g > 255)
						g = 255;
					if (b > 255)
						b = 255;
					int32_t idx = (ri << g_RBitShift) | (gi << g_GBitShift) | (bi << g_BBitShift);
					if (g_isLowMemory)
						idx >>= 3;
					g_brightnessLookup[off + idx] = GetColorRGB(r, g, b);
				}
			}
		}
	}

	for (int32_t i = 1; i < 128; i++)
	{
		int32_t val = 0;
		for (int32_t j = 0; j < i; j++)
		{
			val += 32 * 0x10000 / (i + 1);
			g_TerrainHelper[i][j] = (val + 0x8000) >> 16; //  (val + 0x8000) / 0x10000    --- 0.5 round to ceil
		}
	}

	g_ColorAddMask = GetColorRGB(0x7f, 0x7f, 0x7f);
}

void CalcColorsAndPalettes()
{ //46011e
	for (int32_t i = 0; i < 0x10; i++)
	{
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

	for (int32_t i = 0; i < 0x100; i++)
	{
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


uint32_t InitPixelColors()
{ //45488c
	DDPIXELFORMAT pfmt;
	pfmt.dwSize = sizeof(DDPIXELFORMAT);
	g_ddsurface->GetPixelFormat(&pfmt);

	g_RBitMask = pfmt.dwRBitMask;
	g_GBitMask = pfmt.dwGBitMask;
	g_BBitMask = pfmt.dwBBitMask;

	if (pfmt.dwRBitMask != 0)
	{
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
}

int32_t InitVideo()
{ //453fcc
	if (g_IsServer)
	{
		CalcColorsAndPalettes();
		return 1;
	}

	g_ddsurface = nullptr;
	g_ddsurface2 = nullptr;

	g_ddsurf_lock = 0;
	g_ddsurf_lock2 = 0;

	g_ScreenSize.top = 0;
	g_ScreenSize.left = 0;
	g_clipRect = g_ScreenSize;

	gfx_bits = 16;
	
	if (FAILED(DirectDrawCreate(nullptr, &g_ddraw, nullptr)))
		return 0;

	if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_NORMAL)))
		return 0;

	DDSURFACEDESC tmp;
	tmp.dwSize = sizeof(DDSURFACEDESC);
	tmp.dwFlags = DDSD_CAPS;
	tmp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	tmp.dwBackBufferCount = 0;

	if (FAILED(g_ddraw->CreateSurface(&tmp, &g_ddsurface, nullptr)))
		return 0;

	//if we can't init pixel colors then try force video mode set which also call InitPixelColors
	if (InitPixelColors() == 0 && SetVideoMode() == 0)
	{
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
	if (FAILED(g_ddraw->CreateSurface(&desc, &g_ddsurface2, nullptr)))
		return 0;

	if (!g_ddsurface2)
		return 0;

	memset(&desc, 0, sizeof(DDSURFACEDESC));
	desc.dwSize = sizeof(DDSURFACEDESC);
	desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	desc.dwWidth = g_ScreenSize.right - 160;
	desc.dwHeight = g_ScreenSize.bottom + 1;
	if (FAILED(g_ddraw->CreateSurface(&desc, &g_ddsurface3, nullptr)))
		return 0;

	if (!g_ddsurface3)
		return 0;
	
	/* NOT USED
	DDCAPS g_ddrawCaps; //659a70
	g_ddrawCaps.dwSize = sizeof(DDCAPS);
	g_ddraw->GetCaps(&g_ddrawCaps, nullptr);
	*/

	return 1;
}

int32_t SetVideoMode()
{ //4546c6
	if (g_IsServer)
		return 1;

	if (g_ddsurface)
		g_ddsurface->Release();
	g_ddsurface = nullptr;

	if (FAILED(g_ddraw->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
		return 0;

	if (FAILED(g_ddraw->SetDisplayMode(g_ScreenSize.right, g_ScreenSize.bottom, gfx_bits)))
		return 0;

	g_selDrawBitmap.dwSize = sizeof(DDSURFACEDESC);
	g_selDrawBitmap.dwFlags = DDSD_CAPS;
	g_selDrawBitmap.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	g_selDrawBitmap.dwBackBufferCount = 0;

	if (FAILED(g_ddraw->CreateSurface(&g_selDrawBitmap, &g_ddsurface, nullptr)))
		return 0;

	InitPixelColors();
	return 1;
}



CGameFont::CGameFont()
{
	//45e0d0
	bitmap = nullptr;
	char_widths = nullptr;
}

CGameFont::~CGameFont()
{
	//45e0f9
	if (char_widths)
		delete[] char_widths;

	if (bitmap)
		delete bitmap;
}

void CGameFont::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh)
{
	//460f60
}

uint16_t* CGameFont::GetShadowColors()
{
	//460f70
	return nullptr;
}

void CGameFont::DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy)
{
	//45ebda
	uint16_t* shdow = GetShadowColors();
	DrawTxt(x + shadow_dxy, y + shadow_dxy, txt, align, shdow);
	DrawTxt(x, y, txt, align, colorsh);
}


int32_t CGameFont::GetStrWidth(const char* str)
{
	//45e1f9
	int32_t wi = 0;
	int ln = strlen(str);

	for (int i = 0; i < ln; i++)
	{
		char c = str[i];
		if ((uint8_t)c >= ' ') // 0x20
		{
			if (c == '~')
			{
				if ((i != ln - 1) && str[i + 1] == '~')
				{
					i++;

					uint8_t n = DecodeChar('~') - 0x20;
					wi += char_widths[n] + space;
					if (n == 0)
						wi += bitmap->GetHeight(0) / 2;
				}
			}
			else
			{
				uint8_t n = DecodeChar(c) - 0x20;
				wi += char_widths[n] + space;
				if (n == 0)
					wi += bitmap->GetHeight(0) / 2;
			}
		}
	}

	return wi;
}


int32_t CGameFont::GetHeight()
{
	//402a90
	return bitmap->GetHeight(0);
}


CStringArray& CGameFont::StrToArray(const char* str)
{
	//45e959
	static CStringArray out;  //static for return

	CString s = str;
	out.RemoveAll();

	CString tmp;

	while (!s.IsEmpty())
	{
		int pos = s.Find('\r');
		if (pos == -1)
			break;

		if (s[pos + 1] == '\n')
		{
			tmp = s.Left(pos + 1);
			tmp.TrimLeft();
			out.Add( tmp );

			s = s.Right(s.GetLength() - (pos + 2));
			s.TrimLeft();
		}
	}

	if (!s.IsEmpty())
		out.Add(s);

	return out;
}

CStringArray& CGameFont::StrSplitToFitWidth(const CRect& r, const char* str)
{
	//45e35f
	static CStringArray out;  //static for return

	CString s = str;
	CString tmp1;
	CString tmp2;

	out.RemoveAll();

	s.TrimLeft();
	s.TrimRight();

	int32_t split_pos;

	const int32_t w = r.Width();
	while (!s.IsEmpty())
	{
		int32_t x = 0;
		if (GetStrWidth(s) >= w)
		{
			tmp2 = s;
			while (true)
			{
				split_pos = x;

				int32_t pos = tmp2.Find(' ');
				if (pos == -1)
					x = s.GetLength();
				else
					x += pos + 1;

				tmp1 = s.Left(x);

				tmp2 = s.Right(s.GetLength() - x);

				if (tmp2.IsEmpty())
				{
					if (GetStrWidth(tmp1) < w)
					{
						split_pos = x;
						break;
					}
				}

				if (GetStrWidth(tmp1) >= w)
					break;
			}
		}
		else
		{
			s += '\r';
			split_pos = s.GetLength();
		}

		if (w < GetStrWidth(tmp1) && split_pos == 0)
			split_pos = x;

		tmp1 = s.Left(split_pos);
		tmp1.TrimLeft();

		out.Add(tmp1);

		s = s.Right(s.GetLength() - split_pos);
		s.TrimLeft();
	}

	return out;
}

CStringArray& CGameFont::StringArrayForRect(const CRect& r, const char* str)
{
	//45eb1d

	static CStringArray out;

	out.RemoveAll();

	CStringArray& lines = StrToArray(str);
	
	for (int i = 0; i < lines.GetSize(); i++)
	{
		CStringArray& spl = StrSplitToFitWidth(r, lines[i]);
		out.Append(spl);
	}
	return out;
}

void CGameFont::DrawTextLinesShadow(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t* clr, int32_t dy)
{ //45f6c2
	if (last > lines.GetSize())
		last = lines.GetSize();
	if (dy == 0)
		dy = GetHeight();

	for (int32_t i = first; i < last; i++)
	{
		uint8_t flags = 0;
		CString line = lines[i];
		if (i != lines.GetSize() - 1)
		{
			if (line[line.GetLength() - 1] != '\r')
				flags |= 2;
		}

		if (i == 0)
		{
			flags |= 1;
			if (GetStrWidth(line + ' ') > r.Width())
				flags |= 2;
		}
		else
		{
			const CString& s = lines[i - 1];

			if (s[s.GetLength() - 1] == '\r')
			{
				flags |= 1;
				if (GetStrWidth(line + ' ') > r.Width())
					flags |= 2;
			}
		}

		if (i == lines.GetSize() - 1)
			flags |= 4;
		else
		{
			if (line[line.GetLength() - 1] != '\r')
				flags |= 4;
		}

		if (line[line.GetLength() - 1] == '\r')
			line.SetAt(line.GetLength() - 1, 0);

		if ((flags & 2) == 0)
		{
			if ((flags & 1) == 0)
				DrawTextWithShadow(r.left, r.top + (i - first) * dy, line, 0, clr, 1);
			else
				DrawTextWithShadow(r.left + char_widths[' '], r.top + (i - first) * dy, line, 0, clr, 1);
		}
		else if ((flags & 1) == 0)
			DrawTextJustifyShadow(r.left, r.top + (i - first) * dy, r.Width(), line, clr);
		else
			DrawTextJustifyShadow(r.left + char_widths[' '], r.top + (i - first) * dy, r.Width() - char_widths[' '], line, clr);
	}
}

void CGameFont::DrawTextLines(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t* clr, int32_t dy)
{ //45f33e
	if (last > lines.GetSize())
		last = lines.GetSize();
	if (dy == 0)
		dy = GetHeight();

	for (int32_t i = first; i < last; i++)
	{
		uint8_t flags = 0;
		CString line = lines[i];
		if (i != lines.GetSize() - 1)
		{
			if (line[line.GetLength() - 1] != '\r')
				flags |= 2;
		}

		if (i == 0)
		{
			flags |= 1;
			if (GetStrWidth(line + ' ') > r.Width())
				flags |= 2;
		}
		else
		{
			const CString& s = lines[i - 1];

			if (s[s.GetLength() - 1] == '\r')
			{
				flags |= 1;
				if (GetStrWidth(line + ' ') > r.Width())
					flags |= 2;
			}
		}

		if (i == lines.GetSize() - 1)
			flags |= 4;
		else
		{
			if (line[line.GetLength() - 1] != '\r')
				flags |= 4;
		}

		if (line[line.GetLength() - 1] == '\r')
			line.SetAt(line.GetLength() - 1, 0);

		if ((flags & 2) == 0)
		{
			if ((flags & 1) == 0)
				DrawTxt(r.left, r.top + (i - first) * dy, line, 0, clr);
			else
				DrawTxt(r.left + char_widths[' '], r.top + (i - first) * dy, line, 0, clr);
		}
		else if ((flags & 1) == 0)
			DrawTextJustify(r.left, r.top + (i - first) * dy, r.Width(), line, clr);
		else
			DrawTextJustify(r.left + char_widths[' '], r.top + (i - first) * dy, r.Width() - char_widths[' '], line, clr);
	}
}


void CGameFont::DrawTextJustifyShadow(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr)
{ //45f10b
	int32_t space_width = char_widths[' '];

	CStringArray words;
	CDWordArray wordssz;

	txt.TrimRight();
	int32_t words_width = 0;
	while (txt.GetLength() != 0)
	{
		txt.TrimLeft();

		int32_t word_end = txt.Find(' ');
		if (word_end == -1)
			word_end = txt.GetLength();

		CString wrd = txt.Left(word_end);
		wrd.TrimRight();

		words.Add(wrd);

		int32_t sz = GetStrWidth(wrd);
		words_width += sz;

		wordssz.Add(sz);

		txt = txt.Right(txt.GetLength() - word_end);		
	} 

	int32_t word_space = (w - words_width) << 8; //replace it with int, because no sense of float. Use << 8
	if (words.GetSize() >= 2)
		word_space /= (words.GetSize() - 1);

	int32_t xx = x << 8;
	for (int i = 0; i < words.GetSize(); i++)
	{
		DrawTextWithShadow(xx >> 8, y, words[i], 0, clr, 1);
		xx += (wordssz[i] << 8) + word_space;
	}
}

void CGameFont::DrawTextJustify(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr)
{ //45eed7
	int32_t space_width = char_widths[' '];

	CStringArray words;
	CDWordArray wordssz;

	txt.TrimRight();
	int32_t words_width = 0;
	while (txt.GetLength() != 0)
	{
		txt.TrimLeft();

		int32_t word_end = txt.Find(' ');
		if (word_end == -1)
			word_end = txt.GetLength();

		CString wrd = txt.Left(word_end);
		wrd.TrimRight();

		words.Add(wrd);

		int32_t sz = GetStrWidth(wrd);
		words_width += sz;

		wordssz.Add(sz);

		txt = txt.Right(txt.GetLength() - word_end);
	}

	int32_t word_space = (w - words_width) << 8; //replace it with int, because no sense of float. Use << 8
	if (words.GetSize() >= 2)
		word_space /= (words.GetSize() - 1);

	int32_t xx = x << 8;
	for (int i = 0; i < words.GetSize(); i++)
	{
		DrawTxt(xx >> 8, y, words[i], 0, clr);
		xx += (wordssz[i] << 8) + word_space;
	}
}


void CGameFont::DrawTextJustifyInRectShadow(const CRect& r, const char* str, uint16_t* clr, int32_t dy)
{ //45ed37
	CStringArray& array = StringArrayForRect(r, str);

	if (dy == 0)
		dy = GetHeight();

	for (int32_t i = 0; i < array.GetSize(); i++)
	{
		if (i * dy + GetHeight() > r.Height())
			break;
		DrawTextJustifyShadow(r.left, r.top + i * dy, r.Width(), array[i], clr);
	}

	array.RemoveAll();
}




void CGamePalette::operator=(const CGamePalette& b)
{
	Free();

	count = b.count;
	colors = new uint16_t[256 * count];
	memcpy(colors, b.colors, count * 256 * sizeof(uint16_t));
}

CGamePalette::~CGamePalette()
{ //423c6d
	Free();
}

void CGamePalette::Dump(CDumpContext& dc) const
{ //4245a7
	//dc << "CGamePalette";
}

void CGamePalette::Free()
{ //42457a
	if (colors)
		delete[] colors;

	colors = nullptr;

	count = 0;
}

void CGamePalette::SetPalette(RGBQUAD* rgb, uint32_t _count, int mode, int useColor)
{ //423cbe
	count = _count;
	colors = new uint16_t[256 * count];

	uint32_t dg = g_DeltaCLR.g;
	uint32_t db = g_DeltaCLR.b;
	uint32_t dr = g_DeltaCLR.r;

	if (useColor == 0)
	{
		db = 0;
		dg = 0;
		dr = 0;
	}

	uint16_t* clrs = colors;

	switch (mode)
	{
	case 0:
		colors[0] = 0;
		for (int i = 1; i < 256; i++)
			colors[i] = 0xffff;
		break;

	case 1:
		for (int i = 0; i < 256; i++)
		{
			uint32_t r = rgb[i].rgbRed + dr;

			if (dr > 255)
				dr = 255;

			uint32_t g = rgb[i].rgbGreen + dg;
			
			if (dg > 255)
				dg = 255;

			uint32_t b = rgb[i].rgbBlue + db;
			
			if (db > 255)
				db = 255;

			colors[i] = GetColorRGB(r, g, b);
		}
		break;

	case 2:
		for (int j = count; j > 0; j--)
		{
			for (int i = 0; i < 256; i++)
			{
				int r = (rgb[i].rgbRed + dr) * j * 2 / count;
				if (r < 0)
					r = 0;
				else if (r > 255)
					r = 255;

				int g = (rgb[i].rgbGreen + dg) * j * 2 / count;
				if (g < 0)
					g = 0;
				else if (g > 255)
					g = 255;

				int b = (rgb[i].rgbBlue + db) * j * 2 / count;
				if (b < 0)
					b = 0;
				else if (b > 255)
					b = 255;

				clrs[i] = GetColorRGB(r, g, b);
			}
			clrs += 256;
		}
		break;

	case 3:
		for (int j = count; j > 0; j--)
		{
			for (int i = 0; i < 256; i++)
			{
				int r = (rgb[i].rgbRed + dr) * j / 32;
				if (r < 0)
					r = 0;
				else if (r > 255)
					r = 255;

				int g = (rgb[i].rgbGreen + dg) * j / 32;
				if (g < 0)
					g = 0;
				else if (g > 255)
					g = 255;

				int b = (rgb[i].rgbBlue + db) * j / 32;
				if (b < 0)
					b = 0;
				else if (b > 255)
					b = 255;

				clrs[i] = GetColorRGB(r, g, b);
			}
			clrs += 256;
		}
		break;

	case 4:
		for (int j = 1; j < 17; j++)
		{
			for (int i = 0; i < 256; i++)
			{
				int r, g, b;
				if (g_isLowMemory == 0)
				{
					r = rgb[i].rgbRed * j / 16;
					g = rgb[i].rgbGreen * j / 16;
					b = rgb[i].rgbBlue * j / 16;
				}
				else
				{
					r = rgb[i].rgbRed * j / 18;
					g = rgb[i].rgbGreen * j / 18;
					b = rgb[i].rgbBlue * j / 18;
				}
				if (r < 0)
					r = 0;
				else if (r > 255)
					r = 255;

				if (g < 0)
					g = 0;
				else if (g > 255)
					g = 255;

				if (b < 0)
					b = 0;
				else if (b > 255)
					b = 255;
					
				clrs[i] = GetColorRGB(r, g, b);
			}
			clrs += 256;
		}
		break;

	case 5:
		for (int j = count; j> 0; j--)
		{
			for (int i = 0; i < 256; i++)
			{
				int v = ((rgb[i].rgbRed + rgb[i].rgbGreen + rgb[i].rgbBlue) * j * 2) / 3 / count;
				if (v > 255)
					v = 255;

				clrs[i] = GetColorRGB(v, v, v);
			}
			clrs += 256;
		}
	}
}

uint16_t* CGamePalette::GetPalette(int32_t idx)
{ //41ec40
	return colors + 256 * idx; 
}





CGameBitmap::CGameBitmap() {} //4247a6

CGameBitmap::CGameBitmap(const char* fname)
{ //4245c1
	File2 f;
	if (!f.Open(fname, CFile::modeRead))
	{
		CString s = "FATAL ERROR: can't load ";
		s += fname;
		AfxMessageBox(s);
		exit(-1);
	}

	f.Seek(-4, CFile::end);
	f.Read(&frames_count, 4);
	data_size = f.GetLength();

	f.Rewind();

	palette_data = nullptr;

	pdata = new uint8_t[data_size];
	f.Read(pdata, data_size);
	f.Close();

	frames = new void*[frames_count];

	uint8_t* dpos = pdata;
	for (int i = 0; i < frames_count; i++)
	{
		SpriteFrame* frm = (SpriteFrame*)dpos;
		frames[i] = frm;
		dpos = frm->data + frm->datasize;
	}
}

CGameBitmap::CGameBitmap(const CGameBitmap& src)
{ // 4247fd
	frames_count = src.frames_count;
	data_size = src.data_size;

	frames = new void* [frames_count];

	pdata = new uint8_t[data_size];

	memcpy(pdata, src.pdata, data_size);

	uint8_t* dpos = pdata;
	for (int i = 0; i < frames_count; i++)
	{
		SpriteFrame* frm = (SpriteFrame*)dpos;
		frames[i] = frm;
		dpos = frm->data + frm->datasize;
	}

	palette = src.palette;

	palette_data = src.palette_data; //FIXME
}

CGameBitmap::~CGameBitmap()
{ //42496a
	if (frames)
		delete[] frames;
	if (pdata)
		delete[] pdata;

	palette.Free();
}

void CGameBitmap::Dump(CDumpContext& dc) const
{ //424d3d
	//dc << "CGameBitmap";
}

void CGameBitmap::VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode)
{
	//428d60
}

void CGameBitmap::VMethod2(int32_t x, int32_t y, int frame, int mode, int palid)
{
	//428d50
}

void CGameBitmap::VMethod3(int32_t x, int32_t y, int frame, int blevel, int mode)
{
	//428d70
}

int32_t CGameBitmap::GetWidth(int frame)
{ //424ce6
	return ((SpriteFrame**)frames)[frame]->width;
}

int32_t CGameBitmap::GetHeight(int frame)
{ //424d11
	return ((SpriteFrame**)frames)[frame]->height;
}

void CGameBitmap::SelectBitmapForDraw()
{ //424d57
	g_selDrawBitmap.lpSurface = CGameBitmap::GetData();
	g_selDrawBitmap.dwWidth = GetWidth(0);
	g_selDrawBitmap.dwHeight = GetHeight(0);
	g_selDrawBitmap.lPitch = GetWidth(0) * GetPixelSize();
}

void CGameBitmap::VMethod7()
{
	//428d80
}

int CGameBitmap::GetPixelSize()
{ //428d90
	return 2;
}

void* CGameBitmap::GetData() 
{ //41ec60
	return pdata + sizeof(BmpFrame); 
}

void CGameBitmap::ResetPalette(uint32_t count, int mode, int useColor)
{ //424cb0
	palette.Free();
	palette.SetPalette((RGBQUAD*)palette_data, count, mode, useColor);
}

int32_t CGameBitmap::GetFrameCount() 
{//4387f0
	return frames_count; 
}










CBmp256::CBmp256(const char* fname)
{ //425f8c
	File2 f;
	if (!f.Open(fname, CFile::modeRead))
	{
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

	data_size = pixsz + 256 * 4 + sizeof(BmpFrame);

	pdata = new uint8_t[data_size];

	palette_data = pdata + pixsz + sizeof(BmpFrame);
	memset(palette_data, 0, 256 * 4);

	if (binf.biClrUsed == 0)
		f.Read(palette_data, 256 * 4);
	else
		f.Read(palette_data, binf.biClrUsed * 4);

	BmpFrame* frm = (BmpFrame*)pdata;
	frm->width = binf.biWidth;
	frm->height = binf.biHeight;

	f.Read(pdata + sizeof(BmpFrame), pixsz);
	f.Close();

	CBmp256::VMethod7();

	frames = new void*[1];
	frames[0] = pdata + sizeof(BmpFrame);
}

CBmp256::CBmp256(uint32_t w, uint32_t h)
{ //4262d1

	data_size = w * h + sizeof(BmpFrame);
	pdata = new uint8_t[data_size];

	memset(pdata, 0, data_size);

	palette_data = nullptr;

	BmpFrame* frm = (BmpFrame*)pdata;
	frm->width = w;
	frm->height = h;

	frames = new void* [1];
	frames[0] = pdata + sizeof(BmpFrame);
}

CBmp256::~CBmp256()
{ //428a40
}

void CBmp256::Dump(CDumpContext& dc) const
{ //42649a
}

void CBmp256::VMethod2(int32_t x, int32_t y, int frame, int mode, int palid)
{ //4263df
	const uint16_t* pal = palette.GetPalette(palid);
	const BmpFrame* frm = (BmpFrame*)pdata;
	BlitPaletted(x, y, 0, 0, frm->width, frm->height, pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}

int32_t CBmp256::GetWidth(int frame)
{ //4264c0
	return ((BmpFrame*)pdata)->width;
}

int32_t CBmp256::GetHeight(int frame)
{ //4264d5
	return ((BmpFrame*)pdata)->height;
}

void CBmp256::VMethod7()
{ //4261a6
	const BmpFrame* frm = (BmpFrame*)pdata;
	uint8_t* a = pdata + sizeof(BmpFrame);
	uint8_t* b = pdata + sizeof(BmpFrame) + (frm->height - 1) * frm->width;
	for (int i = 0; i < frm->height / 2; i++)
	{
		memcpy(g_GfxTempBuffer, a, frm->width);
		memcpy(a, b, frm->width);
		memcpy(b, g_GfxTempBuffer, frm->width);

		a += frm->width;
		b -= frm->width;
	}
}

int CBmp256::GetPixelSize()
{ //428da0
	return 1;
}

void CBmp256::VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid)
{ //426385
	const uint16_t* pal = palette.GetPalette(palid);
	const BmpFrame* frm = (BmpFrame*)pdata;
	BlitPaletted(x, y, l, t, r, b, pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}

void CBmp256::VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid)
{ //426440
	const uint16_t* pal = palette.GetPalette(palid);
	const BmpFrame* frm = (BmpFrame*)pdata;
	BlitFlipPalettedAdd(x, y, l, t, r, b, pdata + sizeof(BmpFrame), frm->width, frm->height, pal);
}






CBmp64::CBmp64(const char* fname)
{ //42576d
	File2 f;
	if (!f.Open(fname, CFile::modeRead))
	{
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

	data_size = pixsz * 2 + sizeof(BmpFrame);

	pdata = new uint8_t[data_size];

	palette_data = nullptr;

	BmpFrame* frm = (BmpFrame*)pdata;
	frm->width = binf.biWidth;
	frm->height = binf.biHeight;

	frames = new void*[1];
	frames[0] = pdata + sizeof(BmpFrame);

	uint16_t* dat = (uint16_t*)frames[0];
	for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; )
	{
		int32_t rsz = sizeof(g_GfxTempBuffer);
		if (rawdata < rsz)
			rsz = rawdata;

		f.Read(g_GfxTempBuffer, rsz);
		Convert24To16(g_GfxTempBuffer, dat, rsz / 3);
		dat += rsz / 3;

		rawdata -= rsz;
	}
	f.Close();
}

CBmp64::CBmp64(uint32_t w, uint32_t h)
{ //42597e
	data_size = w * h * 2 + sizeof(BmpFrame);
	pdata = new uint8_t[data_size];
	
	palette_data = nullptr;

	BmpFrame* frm = (BmpFrame*)pdata;
	frm->width = w;
	frm->height = h;

	frames = new void*[1];
	frames[0] = pdata + sizeof(BmpFrame);
}

CBmp64::~CBmp64()
{ //4289f0
}

void CBmp64::Dump(CDumpContext& dc) const
{ //425f3b
}

void CBmp64::VMethod2(int32_t x, int32_t y, int frame, int mode, int palid)
{ //425e0c
	const BmpFrame* frm = (BmpFrame*)pdata;
	Blit16(x, y, 0, 0, frm->width, frm->height, pdata + sizeof(BmpFrame), frm->width, frm->height);
}

int32_t CBmp64::GetWidth(int frame)
{ //425f61
	return ((BmpFrame*)pdata)->width;
}

int32_t CBmp64::GetHeight(int frame)
{ //425f76
	return ((BmpFrame*)pdata)->height;
}

void CBmp64::VMethod7()
{ //425cdb
	const BmpFrame* frm = (BmpFrame*)pdata;
	uint16_t* a = (uint16_t*)((uint8_t*)pdata + sizeof(BmpFrame));
	uint16_t* b = (uint16_t*)((uint8_t*)pdata + sizeof(BmpFrame)) + (frm->height - 1) * frm->width;
	for (int i = 0; i < frm->height / 2; i++)
	{
		memcpy(g_GfxTempBuffer, a, frm->width * 2);
		memcpy(a, b, frm->width * 2);
		memcpy(b, g_GfxTempBuffer, frm->width * 2);

		a += frm->width;
		b -= frm->width;
	}
}

void CBmp64::VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b)
{ //425e5d
	const BmpFrame* frm = (BmpFrame*)pdata;
	Blit16(x, y, l, t, r, b, pdata + sizeof(BmpFrame), frm->width, frm->height);
}

void CBmp64::VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b)
{ //425ea7
	const BmpFrame* frm = (BmpFrame*)pdata;
	Blit16Masked(x, y, l, t, r, b, pdata + sizeof(BmpFrame), frm->width, frm->height);
}

void CBmp64::VMethod11(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b)
{ //425ef1
	const BmpFrame* frm = (BmpFrame*)pdata;
	Blit16Add(x, y, l, t, r, b, pdata + sizeof(BmpFrame), frm->width, frm->height);
}

void CBmp64::LoadFile(const char* fname, CGameBitmap* bmp2)
{ //425a17
	File2 f;
	if (!f.Open(fname, CFile::modeRead))
	{
		CString s = "FATAL ERROR: can't load ";
		s += fname;
		AfxMessageBox(s);
		exit(-1);
	}

	BITMAPFILEHEADER fhdr;
	f.Read(&fhdr, sizeof(fhdr)); //0xe

	BITMAPINFOHEADER binf;
	f.Read(&binf, sizeof(binf)); //0x28

	uint16_t* dat = (uint16_t*)(pdata + sizeof(BmpFrame));
	for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; )
	{
		int32_t rsz = sizeof(g_GfxTempBuffer);
		if (rawdata < rsz)
			rsz = rawdata;

		f.Read(g_GfxTempBuffer, rsz);
		Convert24To16(g_GfxTempBuffer, dat, rsz / 3);
		dat += rsz / 3;

		rawdata -= rsz;
	}

	if (bmp2)
		f.Read(bmp2->GetData(), binf.biWidth * binf.biHeight);

	f.Close();
}

void CBmp64::WriteFile(const char* fname, CGameBitmap* bmp2)
{ //425b95
	CFile f;
	f.Open(fname, CFile::modeCreate | CFile::modeWrite);

	BITMAPFILEHEADER fhdr;
	f.Write(&fhdr, sizeof(fhdr)); //0xe

	BITMAPINFOHEADER binf;
	binf.biWidth = GetWidth();
	binf.biHeight = GetHeight();
	f.Write(&binf, sizeof(binf)); //0x28

	uint16_t* dat = (uint16_t*)(pdata + sizeof(BmpFrame));
	for (int32_t rawdata = binf.biWidth * binf.biHeight * 3; rawdata; )
	{
		int32_t rsz = sizeof(g_GfxTempBuffer);
		if (rawdata < rsz)
			rsz = rawdata;

		Convert16To24(dat, g_GfxTempBuffer, rsz / 3);
		f.Write(g_GfxTempBuffer, rsz);

		dat += rsz / 3;
		rawdata -= rsz;
	}

	if (bmp2)
		f.Write(bmp2->GetData(), binf.biWidth * binf.biHeight);

	f.Close();
}




CSprite256::CSprite256(const char* fname)
{ //424e79

	File2 f;
	if (!f.Open(fname, CFile::modeRead))
	{
		CString s = "FATAL ERROR: can't load ";
		s += fname;
		AfxMessageBox(s);
		exit(-1);
	}

	f.Seek(-4, CFile::end);
	f.Read(&frames_count, 4);
	data_size = f.GetLength();

	f.Rewind();

	bool has_pal = (frames_count & 0x80000000) != 0;

	frames_count &= 0x7fffffff;

	pdata = new uint8_t[data_size];

	palette_data = pdata + data_size - 4 * 256;

	if (has_pal)
	{
		f.Read(palette_data, 4 * 256);
		data_size -= 4 * 256;
	}

	f.Read(pdata, data_size);
	f.Close();

	frames = new void*[frames_count];

	uint8_t* dpos = pdata;
	for (int i = 0; i < frames_count; i++)
	{
		SpriteFrame* frm = (SpriteFrame*)dpos;
		frames[i] = frm;
		dpos = frm->data + frm->datasize;
	}
}


CSprite256::CSprite256(const CSprite256& b)
: CGameBitmap(b)
{ //4250bf
}

CSprite256::~CSprite256()
{ //4289a0
}

void CSprite256::Dump(CDumpContext& dc) const
{ //425558
}

void CSprite256::VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode)
{ //425210
	const uint16_t* pal = ppalette->GetPalette(palid);
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
		BlitRle8(x, y, frm->width, frm->height, frm->data, pal);
	else
		BlitRle8FlipX(x, y, frm->width, frm->height, frm->data, pal);
}

void CSprite256::VMethod2(int32_t x, int32_t y, int frame, int palid, int mode)
{ //4250e4
	const uint16_t* pal = palette.GetPalette(palid);
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
		BlitRle8(x, y, frm->width, frm->height, frm->data, pal);
	else
		BlitRle8FlipX(x, y, frm->width, frm->height, frm->data, pal);
}

void CSprite256::VMethod3(int32_t x, int32_t y, int frame, int blevel, int mode)
{ //425330
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
	{
		if (g_isLowMemory == 0)
			BlitLightRle8(x, y, frm->width, frm->height, frm->data, blevel);
		else
			BlitLightRle8Low(x, y, frm->width, frm->height, frm->data, blevel);
	}
	else
	{
		if (g_isLowMemory == 0)
			BlitLightRle8FlipX(x, y, frm->width, frm->height, frm->data, blevel);
		else
			BlitLightRle8FlipXLow(x, y, frm->width, frm->height, frm->data, blevel);
	}
}

void CSprite256::VMethod9(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode)
{ //4252a0
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
		BlitRle8Blend(x, y, frm->width, frm->height, frm->data, ppalette->GetPalette(palid));
	else
		BlitRle8BlendFlipX(x, y, frm->width, frm->height, frm->data, ppalette->GetPalette(palid));
}

void CSprite256::VMethod10(int32_t x, int32_t y, int frame, int palid, int mode)
{ //42517a
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
		BlitRle8Blend(x, y, frm->width, frm->height, frm->data, palette.GetPalette(palid));
	else
		BlitRle8BlendFlipX(x, y, frm->width, frm->height, frm->data, palette.GetPalette(palid));
}

void CSprite256::VMethod11(int32_t x, int32_t y, int frame, int blevel, int32_t tilt, int mode)
{ //425414
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	if (mode == 0)
	{
		if (g_isLowMemory == 0)
			BlitLightRle8Tilt(x, y, frm->width, frm->height, frm->data, blevel, tilt);
		else
			BlitLightRle8TiltLow(x, y, frm->width, frm->height, frm->data, blevel, tilt);
	}
	else
	{
		if (g_isLowMemory == 0)
			BlitLightRle8TiltFlipX(x, y, frm->width, frm->height, frm->data, blevel, tilt);
		else
			BlitLightRle8TiltFlipXLow(x, y, frm->width, frm->height, frm->data, blevel, tilt);
	}
}

void CSprite256::VMethod12(int32_t x, int32_t y, int frame, uint32_t mv)
{ //425508
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	BlitRle8IndexMask(x, y, frm->width, frm->height, frm->data, mv);
}





CA16::CA16(const char* fname)
: CSprite256(fname)
{} //4287ff


void CA16::VMethod2(int32_t x, int32_t y, int frame, int palid, int mode)
{ //428824
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];

	uint16_t* pal = (uint16_t*)palid;
	if (!pal)
		pal = palette.GetPalette(0);

	if (mode == 0)
		BlitRle16(x, y, frm->width, frm->height, (uint16_t*)frm->data, pal);
	else
		BlitRle16FlipX(x, y, frm->width, frm->height, (uint16_t*)frm->data, pal);
}





CSprite16::CSprite16(const char* fname)
: CGameBitmap(fname)
{} //424db8


void CSprite16::VMethod9(int32_t x, int32_t y, int frame, uint16_t* pcolor)
{ //424ddd
	SpriteFrame* frm = ((SpriteFrame**)frames)[frame];
	BlitRle4(x, y, frm->width, frm->height, frm->data, pcolor);
}





CSpriteFont16::CSpriteFont16(const char* fname, int32_t _space)
{ //45fa44
	CString buf = fname;
	buf += ".16";

	bitmap = new CSprite16(buf);

	buf = fname;
	buf += ".dat";

	File2 f;
	f.Open(buf, CFile::modeRead);

	char_widths = new int32_t[f.GetLength() / 4];
	f.Read(char_widths, f.GetLength());

	f.Close();

	space = _space;
}

void CSpriteFont16::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* clr)
{ //45fbd7
	if ((align & 1) != 0)
		x -= GetStrWidth(txt);

	if ((align & 2) != 0)
		x -= GetStrWidth(txt) / 2;

	if ((align & 4) != 0)
		y -= bitmap->GetWidth();

	if ((align & 8) != 0)
		y -= bitmap->GetWidth() / 2;

	int32_t tlen = strlen(txt);
	for (int32_t i = 0; i < tlen; i++)
	{
		uint8_t chr1 = DecodeChar(txt[i]) - 0x20;
		uint8_t chr2 = DecodeChar(txt[i + 1]) - 0x20;

		if (chr1 == 0x5e && chr2 != 0x5e)
		{
			FillRectColor(x, y + bitmap->GetHeight(), x + char_widths[chr2], y + bitmap->GetHeight(), clr[15]);
		}
		else
		{
			if (chr1 == 0)
				x += bitmap->GetHeight() / 2;
			else
				((CSprite16*)bitmap)->VMethod9(x, y, chr1, clr);

			x += char_widths[chr1] + space;
		}

		if (chr1 == 0x5e && chr2 == 0x5e)
			i++;
	}
}

uint16_t* CSpriteFont16::GetShadowColors()
{ // 45fdf2
	return clrsh_Black;
}



CSpriteFont16a::CSpriteFont16a(const char* fname, int32_t _space)
{ // 45fe02
	CString buf = fname;
	buf += ".16a";

	bitmap = new CA16(buf);

	buf = fname;
	buf += ".dat";

	File2 f;
	f.Open(buf, CFile::modeRead);

	char_widths = new int32_t[f.GetLength() / 4];
	f.Read(char_widths, f.GetLength());

	f.Close();

	space = _space;

	bitmap->ResetPalette(16, 4, 0);
}

CSpriteFont16a::~CSpriteFont16a()
{ // 461000
}

void CSpriteFont16a::DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* clr)
{ // 45ffa9
	if ((align & 1) != 0)
		x -= GetStrWidth(txt);

	if ((align & 2) != 0)
		x -= GetStrWidth(txt) / 2;

	if ((align & 4) != 0)
		y -= bitmap->GetWidth();

	if ((align & 8) != 0)
		y -= bitmap->GetWidth() / 2;

	int32_t tlen = strlen(txt);
	for (int32_t i = 0; i < tlen; i++)
	{
		uint8_t chr1 = DecodeChar(txt[i]) - 0x20;
		if (chr1 == 0)
			x += bitmap->GetHeight() / 2;
		else
			((CA16*)bitmap)->VMethod2(x, y, chr1, (int)clr, 0); //FIXME

		x += char_widths[chr1] + space;
	}
}

uint16_t* CSpriteFont16a::GetShadowColors()
{ // 460106
	return palette_black->GetPalette(0);
}












void LoadGraphics()
{ //47819d
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

		if (g_ScreenSize.bottom < 601)
		{
			if (g_ScreenSize.bottom > 480)
			{
				g_bmp_spb800l = new CBmp64("graphics\\interface\\spb800l.bmp");
				g_bmp_spb800r = new CBmp64("graphics\\interface\\spb800r.bmp");
				g_bmp_extra800l = new CBmp64("graphics\\interface\\extra800l.bmp");
				g_bmp_extra800r = new CBmp64("graphics\\interface\\extra800r.bmp");
			}
		}
		else
		{
			g_bmp_spb1024l = new CBmp64("graphics\\interface\\spb1024l.bmp");
			g_bmp_spb1024r = new CBmp64("graphics\\interface\\spb1024r.bmp");
			g_bmp_extra1024l = new CBmp64("graphics\\interface\\extra1024l.bmp");
			g_bmp_extra1024r = new CBmp64("graphics\\interface\\extra1024r.bmp");
			g_bmp_inv1024l = new CBmp64("graphics\\interface\\inv1024l.bmp");
			g_bmp_inv1024r = new CBmp64("graphics\\interface\\inv1024r.bmp");
		}

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


void UnloadGraphics()
{ //47961c
	if (g_bmp_crystalr)
		delete g_bmp_crystalr;
	if (g_bmp_crystall)
		delete g_bmp_crystall;
	if (g_bmp_headsr)
		delete g_bmp_headsr;
	if (g_bmp_headsl)
		delete g_bmp_headsl;
	if (g_bmp_cmdbarr)
		delete g_bmp_cmdbarr;
	if (g_bmp_cmdbarl)
		delete g_bmp_cmdbarl;
	if (g_bmp_cmddnr)
		delete g_bmp_cmddnr;
	if (g_bmp_cmdempr)
		delete g_bmp_cmdempr;
	if (g_bmp_humanbackl)
		delete g_bmp_humanbackl;
	if (g_bmp_humanbackr)
		delete g_bmp_humanbackr;
	if (g_bmp_textbackl)
		delete g_bmp_textbackl;
	if (g_bmp_textbackr)
		delete g_bmp_textbackr;
	if (g_bmp_bookopened)
		delete g_bmp_bookopened;
	if (g_bmp_bookclosed)
		delete g_bmp_bookclosed;
	if (g_bmp_backpackop)
		delete g_bmp_backpackop;
	if (g_bmp_backpackcl)
		delete g_bmp_backpackcl;
	if (g_bmp_humanmode)
		delete g_bmp_humanmode;
	if (g_bmp_textmode)
		delete g_bmp_textmode;
	if (g_bmp_diskette)
		delete g_bmp_diskette;
	if (g_bmp_ar1)
		delete g_bmp_ar1;
	if (g_bmp_ar2)
		delete g_bmp_ar2;
	if (g_bmp_ar3)
		delete g_bmp_ar3;
	if (g_bmp_ar4)
		delete g_bmp_ar4;
	if (g_bmp_spellbook)
		delete g_bmp_spellbook;
	if (g_bmp_spellback)
		delete g_bmp_spellback;
	if (g_bmp_invframe)
		delete g_bmp_invframe;
	if (g_bmp_invarrow1)
		delete g_bmp_invarrow1;
	if (g_bmp_invarrow2)
		delete g_bmp_invarrow2;
	if (g_bmp_invarrow3)
		delete g_bmp_invarrow3;
	if (g_bmp_invarrow4)
		delete g_bmp_invarrow4;
	if (g_bmp_backinv)
		delete g_bmp_backinv;
	if (g_bmp_spb1024l)
		delete g_bmp_spb1024l;
	if (g_bmp_spb1024r)
		delete g_bmp_spb1024r;
	if (g_bmp_extra1024l)
		delete g_bmp_extra1024l;
	if (g_bmp_extra1024r)
		delete g_bmp_extra1024r;
	if (g_bmp_inv1024l)
		delete g_bmp_inv1024l;
	if (g_bmp_inv1024r)
		delete g_bmp_inv1024r;
	if (g_bmp_spb800l)
		delete g_bmp_spb800l;
	if (g_bmp_spb800r)
		delete g_bmp_spb800r;
	if (g_bmp_extra800l)
		delete g_bmp_extra800l;
	if (g_bmp_extra800r)
		delete g_bmp_extra800r;
	if (gfx_interface_lm)
		delete gfx_interface_lm;
	if (gfx_scrollbars)
		delete gfx_scrollbars;
	if (gfx_radiob)
		delete gfx_radiob;
	if (g_ca16_money)
		delete g_ca16_money;
	if (g_spr_t_border)
		delete g_spr_t_border;
	if (g_bmp_t_back)
		delete g_bmp_t_back;
	if (g_spr_backm)
		delete g_spr_backm;
	if (g_spr_backf)
		delete g_spr_backf;
	if (g_bmp_minimapdata)
		delete g_bmp_minimapdata;
	if (g_bmp_server)
		delete g_bmp_server;
	if (gfx_ball)
		delete gfx_ball;
	if (g_spr_backpack)
		delete g_spr_backpack;
	if (g_spr_backpackb)
		delete g_spr_backpackb;
	if (g_bmp_testiva)
		delete g_bmp_testiva;

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

void LoadFonts()
{ //460c37
	g_font1 = new CSpriteFont16("graphics\\font1\\font1", 2);
	g_font2 = new CSpriteFont16("graphics\\font2\\font2", 2);
	g_font3 = new CSpriteFont16("graphics\\font3\\font3", 2);
	g_font4 = new CSpriteFont16a("graphics\\font4\\font4", 2);
}

void FreeFontData()
{ //460d7f
	if (g_font1)
		delete g_font1;
	if (g_font2)
		delete g_font2;
	if (g_font3)
		delete g_font3;
	if (g_font4)
		delete g_font4;
	if (palette_paris_daisy)
		delete palette_paris_daisy;
	if (palette_husk)
		delete palette_husk;
	if (palette_black)
		delete palette_black;
	if (palette_brown_derby)
		delete palette_brown_derby;
	if (palette_tawny_port)
		delete palette_tawny_port;

	g_font2 = nullptr;
	g_font3 = nullptr;
	g_font4 = nullptr;
	palette_paris_daisy = nullptr;
	palette_husk = nullptr;
	palette_black = nullptr;
	palette_brown_derby = nullptr;
	palette_tawny_port = nullptr;
}

void FreeDDraw()
{ //454219
	if (g_IsServer == 0 && g_ddraw)
	{
		g_ddraw->Release();
		g_ddraw = nullptr;
	}
}


void SaveScreenshot()
{ //453d33
	static int32_t screenshot_num = 0; //65dd24
	CString fname;
	fname.Format("Allods%04d.bmp", screenshot_num);
	screenshot_num++;

	int32_t pixnum = g_ScreenSize.right * g_ScreenSize.bottom;
	uint8_t* data = new uint8_t[pixnum * 3];

	LockSurface1();
	uint8_t* dst = data;
	for (int32_t y = g_ScreenSize.bottom - 1; y >= 0; y--)
	{
		uint16_t* src = (uint16_t*)((uint8_t*)g_selDrawBitmap.lpSurface + y * g_ScreenSize.bottom * 2);
		for (int32_t x = 0; x < g_ScreenSize.right; x++)
		{
			uint16_t clr = *src;
			dst[0] = ((clr & g_BBitMask) >> g_BBitShift) << (8 - g_BBits);
			dst[1] = ((clr & g_GBitMask) >> g_GBitShift) << (8 - g_GBits);
			dst[2] = ((clr & g_RBitMask) >> g_RBitShift) << (8 - g_RBits);

			src++;
			dst += 3;
		}
	}
	UnlockSurface1();

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