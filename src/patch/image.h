#pragma once

// Tangar patch image loader, ported from my-a2mgr Image.{h,cpp} (menu.cpp,
// itempng.cpp and add_info.cpp all render through it). The manager decoded
// files with SDL_image; this port uses the vendored lodepng for PNG and a
// built-in decoder for uncompressed 24/32-bit BMP (the only BMP assets the
// patch ships). Pixels are kept in lodepng byte order (R,G,B,A per pixel,
// i.e. 0xAABBGGRR as a little-endian dword) and blended onto the game's
// RGB565 draw surface (g_selDrawBitmap).

#include <stdint.h>

class PatchImage {
public:
	PatchImage(const char* filename);
	~PatchImage();

	void Display(int16_t x, int16_t y);
	void DisplayEx(int16_t x, int16_t y, int16_t inX, int16_t inY, int16_t w, int16_t h, bool no_alpha);

	uint32_t* GetPixels();
	uint32_t GetWidth();
	uint32_t GetHeight();

private:
	uint32_t* myPixels = nullptr;
	uint32_t myWidth = 0;
	uint32_t myHeight = 0;
};
