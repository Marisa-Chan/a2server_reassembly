#include <inttypes.h>
#include <ddraw.h>
#include <stdio.h>
#include "asm_mfc.h"
#include "gfx.h"
#include "util.h"
#include "game_app.h"

extern CRect g_clipRect;
extern DDSURFACEDESC g_selDrawBitmap;

uint32_t g_isLowMemory = 0; //660f58


int32_t g_RBits = 5; //62f870
int32_t g_GBits = 6; //62f874
int32_t g_BBits = 5; //62f878
int32_t g_RBitShift = 11; //62f868
int32_t g_GBitShift = 5; //62f86c
int32_t g_BBitShift = 0; //660f5c
int32_t g_RBitMask = 0xf800; //62f85c
int32_t g_GBitMask = 0x7e0; //62f860
int32_t g_BBitMask = 0x1f; //62f864


uint16_t clrsh_DullGold[16]; //65f4f8
uint16_t clrsh_TechBlack[16]; //65f090
uint16_t clrsh_ShockingBlack[16]; //65e1e8
uint16_t clrsh_CharlieBrown[16]; //65f4b8
uint16_t* p_clrsh_Black = clrsh_TechBlack; //62e584
uint16_t* p_clrsh_Gold = clrsh_DullGold; //62e58c
uint16_t* p_clrsh_ShockingBlack = clrsh_ShockingBlack; //62e588

uint16_t* g_brightnessLookup = nullptr; //65dd20
uint32_t g_brightnessLookupCount = 0; //659bf0



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


void __cdecl FUN_004559d4(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* src, uint16_t* clr)
{
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

	return wi;
}


int32_t CGameFont::GetHeight()
{
	//402a90
	return bitmap->GetHeight(0);
}
