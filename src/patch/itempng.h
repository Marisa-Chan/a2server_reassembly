#pragma once

// Tangar PNG item/character sprite replacements, ported from my-a2mgr
// itempng.cpp (hooks ITEMPNG_loadPng @ client 46E014, ITEMPNG_loadPngChar @
// client 46E1B0 and ITEMPNG_cachePng @ client 42AAC0). When a PNG twin of a
// retail .256 equipment/character sprite exists under data\, it replaces the
// sprite in CUnit::RenderHeroPortrait.

#include <stdint.h>

#include "../gfx.h"
#include "image.h"

// CSprite256-compatible sprite backed by a PatchImage. The manager faked a
// 17-slot vtable with only three live entries (??_G no-op, Draw, DrawIndexMask
// — the only slots RenderHeroPortrait dispatches on); this port gets the same
// behavior from a real subclass.
class PatchPngSprite : public CSprite256 {
public:
	explicit PatchPngSprite(PatchImage* image);

	virtual void Draw(int32_t x, int32_t y, int frame, int palid, int mode) override;
	virtual void DrawIndexMask(int32_t x, int32_t y, int frame, uint32_t mv) override;

	// Sprites live in a permanent cache, exactly as in the manager patch (its
	// IImage_Destruct no-ops and the CSprite256 ??_G guard skips deletion for
	// cached pointers): `delete sprite` therefore does nothing.
	static void operator delete(void*) {}

private:
	PatchImage* image;
};

// Returns a cached PNG-backed sprite for a retail ".256" sprite path (by
// checking the "data\<...>.png" twin), or nullptr when no PNG replacement
// exists — the caller then falls back to the retail CSprite256 load.
CSprite256* TangarLoadItemSprite(const char* retail_256_path);
