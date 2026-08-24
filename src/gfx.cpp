#include <inttypes.h>
#include <ddraw.h>
#include <stdio.h>
#include "asm_mfc.h"
#include "gfx.h"
#include "util.h"
#include "game_app.h"
#include "file.h"

IMPLEMENT_DYNAMIC(CGamePalette, CObject);

IMPLEMENT_DYNAMIC(CGameBitmap, CObject);
IMPLEMENT_DYNAMIC(CBmp256, CGameBitmap);
IMPLEMENT_DYNAMIC(CBmp64, CGameBitmap);
IMPLEMENT_DYNAMIC(CSprite256, CGameBitmap);
IMPLEMENT_DYNAMIC(CA16, CSprite256);
IMPLEMENT_DYNAMIC(CSprite16, CGameBitmap);


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


uint16_t clrsh_CoralRed[16]; //65f608
uint16_t clrsh_DullGold[16]; //65f4f8
uint16_t clrsh_TechBlack[16]; //65f090
uint16_t clrsh_ShockingBlack[16]; //65e1e8
uint16_t clrsh_CharlieBrown[16]; //65f4b8
uint16_t clrsh_LuxorGold[16]; //65e848
uint16_t* p_clrsh_Black = clrsh_TechBlack; //62e584
uint16_t* p_clrsh_Gold = clrsh_DullGold; //62e58c
uint16_t* p_clrsh_ShockingBlack = clrsh_ShockingBlack; //62e588

uint16_t g_colors_human_pals[16][16]; //65e230
int32_t g_MessageColors; //660f80

uint16_t* g_brightnessLookup = nullptr; //65dd20
uint32_t g_brightnessLookupCount = 0; //659bf0

ColorThing g_DeltaCLR; //660e28

uint8_t g_GfxTempBuffer[92160]; //642d10
uint16_t g_ColorAddMask; //660f60


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



void __cdecl FlushScreen()
{
	//454e18
	gfxFlushRect(CRect(g_ScreenSize));
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
		free(char_widths);

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