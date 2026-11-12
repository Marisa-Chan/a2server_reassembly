// Tangar patch image loader, ported from my-a2mgr Image.cpp. Decode via
// lodepng (PNG) and a built-in BMP reader instead of SDL_image; everything
// else (blending, clipping, fatal-error handling) is behavior-identical.

#include "image.h"

#include <afxwin.h>

#include <cstring>

#include "lodepng/lodepng.h"
#include "../gfx.h"
#include "../server.h"

// Manager: MessageBoxA(GetHWND(), ...) + zxmgr::AfxAbort(). The main window
// exists whenever the client renders, so GetSafeHwnd() is the same HWND.
static void TangarImageFatal(const char* filename) {
	char msg[512];
	sprintf_s(msg, "FATAL ERROR: can't load %s", filename);
	MessageBoxA(AfxGetMainWnd()->GetSafeHwnd(), msg, "Allods2", MB_ICONWARNING | MB_OK);
	AfxAbort();
}

PatchImage::PatchImage(const char* filename) {
	this->myPixels = nullptr;
	this->myWidth = 0;
	this->myHeight = 0;

	FILE* fil = nullptr;
	if (fopen_s(&fil, filename, "rb") != 0 || fil == nullptr) {
		TangarImageFatal(filename);
		return;
	}
	fseek(fil, 0, SEEK_END);
	long length = ftell(fil);
	fseek(fil, 0, SEEK_SET);

	uint8_t* buffer = new uint8_t[length];
	memset(buffer, 0, length);
	size_t count_read = fread(buffer, 1, length, fil);
	fclose(fil);
	(void)count_read;

	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t* decoded = nullptr;
	bool ok = false;

	if (length >= 8 && buffer[0] == 0x89 && buffer[1] == 'P' && buffer[2] == 'N' && buffer[3] == 'G') {
		// lodepng_decode32 outputs R,G,B,A bytes per pixel, which is exactly
		// the layout this class expects (0xAABBGGRR little-endian dwords).
		ok = (lodepng_decode32(&decoded, &width, &height, buffer, length) == 0);
	} else if (length >= 54 && buffer[0] == 'B' && buffer[1] == 'M') {
		// Uncompressed bottom-up 24/32-bit BMP, expanded to opaque RGBA.
		uint32_t data_offset = buffer[10] | (buffer[11] << 8) | (buffer[12] << 16) | ((uint32_t)buffer[13] << 24);
		uint32_t header_size = buffer[14] | (buffer[15] << 8) | (buffer[16] << 16) | ((uint32_t)buffer[17] << 24);
		int32_t bmp_w = buffer[18] | (buffer[19] << 8) | (buffer[20] << 16) | ((int32_t)((uint32_t)buffer[21] << 24));
		int32_t bmp_h = buffer[22] | (buffer[23] << 8) | (buffer[24] << 16) | ((int32_t)((uint32_t)buffer[25] << 24));
		uint16_t bpp = buffer[28] | ((uint16_t)buffer[29] << 8);
		uint32_t compression = buffer[30] | (buffer[31] << 8) | (buffer[32] << 16) | ((uint32_t)buffer[33] << 24);

		if (header_size >= 40 && bmp_w > 0 && bmp_h > 0 && compression == 0 && (bpp == 24 || bpp == 32)) {
			uint32_t src_stride = ((uint32_t)bmp_w * bpp / 8u + 3u) & ~3u;
			if ((uint64_t)data_offset + (uint64_t)src_stride * (uint64_t)bmp_h <= (uint64_t)length) {
				width = (uint32_t)bmp_w;
				height = (uint32_t)bmp_h;
				decoded = (uint8_t*)malloc((size_t)width * height * 4);
				if (decoded != nullptr) {
					ok = true;
					for (uint32_t y = 0; y < height; y++) {
						const uint8_t* src_row = buffer + data_offset + (size_t)(height - 1 - y) * src_stride;
						uint8_t* dst_row = decoded + (size_t)y * width * 4;
						for (uint32_t x = 0; x < width; x++) {
							dst_row[x * 4 + 0] = src_row[x * (bpp / 8) + 2]; // R
							dst_row[x * 4 + 1] = src_row[x * (bpp / 8) + 1]; // G
							dst_row[x * 4 + 2] = src_row[x * (bpp / 8) + 0]; // B
							dst_row[x * 4 + 3] = 0xFF;                       // A (BMP has no alpha)
						}
					}
				}
			}
		}
	}

	delete[] buffer;

	if (!ok || decoded == nullptr || width == 0 || height == 0) {
		free(decoded);
		TangarImageFatal(filename);
		return;
	}

	this->myWidth = width;
	this->myHeight = height;
	this->myPixels = new uint32_t[(size_t)width * height];
	memcpy(this->myPixels, decoded, (size_t)width * height * 4);
	free(decoded);
}

PatchImage::~PatchImage() {
	if (this->myPixels) {
		delete[] this->myPixels;
	}
	this->myPixels = nullptr;
	this->myWidth = 0;
	this->myHeight = 0;
}

void PatchImage::Display(int16_t x, int16_t y) {
	this->DisplayEx(x, y, 0, 0, (int16_t)this->myWidth, (int16_t)this->myHeight, false);
}

void PatchImage::DisplayEx(int16_t x, int16_t y, int16_t inX, int16_t inY, int16_t w, int16_t h, bool no_alpha) {
	if (this->myPixels == nullptr) {
		return;
	}
	if (g_IsServer != 0) {
		return;
	}

	uint16_t* pPixels = (uint16_t*)g_selDrawBitmap.lpSurface;
	uint32_t pPitch = g_selDrawBitmap.lPitch;

	if (x < 0) {
		int16_t oldX = x;
		x = 0;
		inX = -oldX;
	}

	if (y < 0) {
		int16_t oldY = y;
		y = 0;
		inY = -oldY;
	}

	if (inX + w > (int16_t)this->myWidth) {
		w = (int16_t)this->myWidth - inX;
	}
	if (inY + h > (int16_t)this->myHeight) {
		h = (int16_t)this->myHeight - inY;
	}

	const CRect& pRect = g_ScreenSize;
	if (x > pRect.right || y > pRect.bottom) {
		return;
	}
	if (x + w < pRect.left || y + h < pRect.top) {
		return;
	}

	int32_t pTotalWidth = pPitch / 2;
	int32_t inBorderLeft = x;
	int32_t inBorderRight = pTotalWidth - (w + inBorderLeft);

	if (inBorderLeft < 0 || inBorderRight < 0) {
		return;
	}

	int32_t rBorderLeft = inX;
	int32_t rBorderRight = (int32_t)this->myWidth - (w + rBorderLeft);
	uint32_t* rPixels = this->myPixels;

	pPixels += (size_t)y * pTotalWidth;
	rPixels += (size_t)inY * this->myWidth;
	for (int32_t row = 0; row < h; row++) {
		rPixels += rBorderLeft;
		pPixels += inBorderLeft;
		for (int32_t col = 0; col < w; col++) {
			uint32_t src_color = *rPixels++;
			// lodepng byte order R,G,B,A packed little-endian: A<<24|B<<16|G<<8|R.
			uint8_t src_r = (src_color & 0x000000FF) >> 3;
			uint8_t src_g = (src_color & 0x0000FF00) >> 10;
			uint8_t src_b = (src_color & 0x00FF0000) >> 19;
			uint8_t src_a = (src_color & 0xFF000000) >> 24;
			if (no_alpha) {
				if (src_a > 127) {
					src_a = 255;
				} else {
					src_a = 0;
				}
			}

			if (src_a == 0) {
				// The manager forgot to advance pPixels on transparent pixels
				// (its `continue` skipped the only `*pPixels++`), desyncing the
				// destination for the rest of the row. All shipped patch assets
				// are fully opaque, so this never fired; kept correct here.
				pPixels++;
				continue;
			}

			uint16_t dst_color = *pPixels;
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

				out_r = min((uint8_t)Fout_r, 0x1F);
				out_g = min((uint8_t)Fout_g, 0x3F);
				out_b = min((uint8_t)Fout_b, 0x1F);
			} else {
				out_r = src_r;
				out_g = src_g;
				out_b = src_b;
			}

			dst_color = ((uint16_t)(out_r & 0x1F) << 11);
			dst_color |= ((uint16_t)(out_g & 0x3F) << 5);
			dst_color |= out_b;
			*pPixels++ = dst_color;
		}
		rPixels += rBorderRight;
		pPixels += inBorderRight;
	}
}

uint32_t* PatchImage::GetPixels() {
	return this->myPixels;
}

uint32_t PatchImage::GetWidth() {
	return this->myWidth;
}

uint32_t PatchImage::GetHeight() {
	return this->myHeight;
}
