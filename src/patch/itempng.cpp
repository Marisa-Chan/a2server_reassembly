// Tangar PNG item/character sprite replacements, ported from my-a2mgr
// itempng.cpp. Draw/DrawIndexMask carry over the manager's IImage_Display /
// IImage_CopyTo logic (only full-size 160x240 portraits render; all draw
// parameters are ignored — the image always blits at the origin of the
// currently selected draw bitmap).

#include "itempng.h"

#include <cstdio>
#include <map>
#include <string>

PatchPngSprite::PatchPngSprite(PatchImage* image)
    : CSprite256() {
	this->image = image;
}

void PatchPngSprite::Draw(int32_t x, int32_t y, int frame, int palid, int mode) {
	(void)x;
	(void)y;
	(void)frame;
	(void)palid;
	(void)mode;

	if (this->image->GetWidth() != 160 || this->image->GetHeight() != 240) {
		return;
	}

	uint16_t* upx = (uint16_t*)g_selDrawBitmap.lpSurface;
	uint32_t* px = this->image->GetPixels();
	for (uint32_t i = 0; i < 160 * 240; i++) {
		uint32_t src_color = px[i];
		// lodepng byte order R,G,B,A packed little-endian: A<<24|B<<16|G<<8|R.
		uint8_t src_r = (src_color & 0x000000FF) >> 3;
		uint8_t src_g = (src_color & 0x0000FF00) >> 10;
		uint8_t src_b = (src_color & 0x00FF0000) >> 19;
		uint8_t src_a = (src_color & 0xFF000000) >> 24;

		if (src_a < 1) {
			continue;
		}

		uint16_t dst_color = upx[i];
		uint8_t dst_r = (dst_color & 0xF800) >> 11;
		uint8_t dst_g = (dst_color & 0x07E0) >> 5;
		uint8_t dst_b = (dst_color & 0x001F);
		// max for blue&red = 31.0
		// max for green = 63.0

		uint8_t out_r;
		uint8_t out_g;
		uint8_t out_b;

		if (src_a != 255) {
			double alpha = (double)src_a / 255.0;
			double Fout_r = (src_r * alpha) + (dst_r * (1.0 - alpha));
			double Fout_g = (src_g * alpha) + (dst_g * (1.0 - alpha));
			double Fout_b = (src_b * alpha) + (dst_b * (1.0 - alpha));

			// Unlike PatchImage::DisplayEx, the manager clamps to >= 1 here so
			// a blended pixel can never become color 0 (transparent).
			out_r = max(min((uint8_t)Fout_r, 0x1F), 1);
			out_g = max(min((uint8_t)Fout_g, 0x3F), 1);
			out_b = max(min((uint8_t)Fout_b, 0x1F), 1);
		} else {
			out_r = max(src_r, 1);
			out_g = max(src_g, 1);
			out_b = max(src_b, 1);
		}

		dst_color = ((uint16_t)(out_r & 0x1F) << 11);
		dst_color |= ((uint16_t)(out_g & 0x3F) << 5);
		dst_color |= out_b;
		upx[i] = dst_color;
	}
}

void PatchPngSprite::DrawIndexMask(int32_t x, int32_t y, int frame, uint32_t mv) {
	(void)x;
	(void)y;
	(void)frame;

	if (this->image->GetWidth() != 160 || this->image->GetHeight() != 240) {
		return;
	}

	uint8_t* upx = (uint8_t*)g_selDrawBitmap.lpSurface;
	uint32_t* px = this->image->GetPixels();
	for (uint32_t i = 0; i < 160 * 240; i++) {
		uint32_t src_color = px[i];
		uint8_t src_a = (src_color & 0xFF000000) >> 24;

		if (src_a < 127) {
			continue;
		}

		upx[i] = (uint8_t)mv;
	}
}

CSprite256* TangarLoadItemSprite(const char* retail_256_path) {
	// The manager passed the sprite name without extension and built both the
	// .png twin and the .256 fallback from it; here the retail path (with
	// extension) comes in, so swap the extension for ".png".
	static std::map<std::string, CSprite256*> cache;

	std::string png_path = std::string("data\\") + retail_256_path;
	size_t dot = png_path.rfind('.');
	if (dot == std::string::npos) {
		return nullptr;
	}
	png_path.replace(dot, std::string::npos, ".png");

	std::map<std::string, CSprite256*>::iterator it = cache.find(png_path);
	if (it != cache.end()) {
		return it->second;
	}

	// nullptr entries are negative cache: a missing PNG is re-checked on every
	// call in the manager too (FileExistsEx), but caching the miss avoids one
	// file probe per portrait render.
	FILE* fil = nullptr;
	if (fopen_s(&fil, png_path.c_str(), "rb") != 0) {
		cache[png_path] = nullptr;
		return nullptr;
	}
	fclose(fil);

	CSprite256* sprite = new PatchPngSprite(new PatchImage(png_path.c_str()));
	cache[png_path] = sprite;
	return sprite;
}
