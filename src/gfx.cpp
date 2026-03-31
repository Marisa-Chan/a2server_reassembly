#include <inttypes.h>
#include <ddraw.h>
#include <stdio.h>
#include "asm_mfc.h"

extern CRect g_clipRect;
extern DDSURFACEDESC g_selDrawBitmap;

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
				uint8_t b = *src;
				src++;
				uint8_t flags = b & 0xc0;
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
				uint8_t b = *src;
				src++;
				uint8_t flags = b & 0xc0;
				uint8_t count = b & 0x3f;
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
