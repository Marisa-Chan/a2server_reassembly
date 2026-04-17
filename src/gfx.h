#pragma once

#include "asm_mfc.h"



class CGameBitmap;
class CSprite256;
class CGameFont;
class CBmp64;


extern CRect g_ScreenSize; //65fb78
extern CSprite256* gfx_interface_lm; //6653ec
extern CSprite256* gfx_scrollbars; //6653f0
extern CSprite256* gfx_radiob; //6653f4

extern CGameFont* g_font1; //65e228
extern CGameFont* g_font2; //65ec68

extern CBmp64* gfx_ball; //665414


extern int32_t g_RBits;
extern int32_t g_GBits;
extern int32_t g_BBits;
extern int32_t g_RBitShift;
extern int32_t g_GBitShift;
extern int32_t g_BBitShift;
extern int32_t g_RBitMask;
extern int32_t g_GBitMask;
extern int32_t g_BBitMask;


extern uint16_t* p_clrsh_Black; //62e584
extern uint16_t* p_clrsh_ShockingBlack; //62e588
extern uint16_t* p_clrsh_Gold; //62e58c

extern uint16_t clrsh_DullGold[16]; //65f4f8
extern uint16_t clrsh_TechBlack[16]; //65f090
extern uint16_t clrsh_ShockingBlack[16]; //65e1e8
extern uint16_t clrsh_CharlieBrown[16]; //65f4b8
extern uint16_t clrsh_LuxorGold[16]; //65e848

extern uint16_t g_colors_human_pals[16][16]; //65e230

extern uint16_t* g_brightnessLookup; //65dd20
extern uint32_t g_brightnessLookupCount; //659bf0

extern uint32_t g_isLowMemory; //660f58



void GetClipRect(CRect* r); //454a0c
void SetClipRect(CRect& r); //4549ba


inline uint32_t GetColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r >> (8 - g_RBits)) << g_RBitShift) | ((g >> (8 - g_GBits)) << g_GBitShift) | ((b >> (8 - g_BBits)) << g_BBitShift);
}



void __cdecl SetPixelColor(int32_t x, int32_t y, uint32_t clr); //4586a4
void __cdecl FillRectColor(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); //457dcc
void __cdecl FillRectColorSimple(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); //4579d8
void __cdecl ShadowRect(CRect rect, int shadow); //457b6f
void __cdecl gfxFlushRect(const CRect& rect); //454c74
void __cdecl DrawRectangleFrame(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); //458035

void __cdecl CopyScreenRectToBmp64(int32_t x, int32_t y, int32_t w, int32_t h, int32_t width, int32_t height, void* dst); //45455e

extern void LockSurface2(); // 45426e
extern void UnlockSurface2(); // 4542ca
extern void LockSurface1(); // 45431c
extern void UnlockSurface1(); // 454378

extern int32_t GetLockCountSurf2(); //4538e1





class CGameFont : public CObject
{
public:
	virtual ~CGameFont();

	virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh);
	virtual uint16_t* GetShadowColors();

public:
	CGameFont();
	void DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy);

	int32_t GetStrWidth(const char* str); //45e1f9

	int32_t GetHeight(); //402a90

	CStringArray& StrToArray(const char* str); //45e959  internal
	CStringArray& StrSplitToFitWidth(const CRect& r, const char* str); //45e35f  internal

	CStringArray& StringArrayForRect(const CRect& r, const char* str); //45eb1d
	void DrawTextLines(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t *clr, int32_t dy); //45f6c2

public:
	CGameBitmap* bitmap;
	int32_t* char_widths;
	int32_t space;
};

ASSERT_SIZE(CGameFont, 0x10);


class CGamePalette : public CObject
{
	DECLARE_DYNAMIC(CGamePalette);
public:
	virtual ~CGamePalette();
	virtual void Dump(CDumpContext& dc) const override;

	CGamePalette();
	void Free();
	void SetPalette(RGBQUAD* rgb, uint32_t count, int mode, int useColor);

public:
	uint32_t count;
	uint16_t* colors;
};

ASSERT_SIZE(CGamePalette, 0xC);

struct BtmapFrame
{
	uint32_t width;
	uint32_t height;
};

class CGameBitmap : public CObject
{
	DECLARE_DYNAMIC(CGameBitmap);
public:
	virtual ~CGameBitmap();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode);
	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode);
	virtual void VMethod3(int32_t x, int32_t y, int frame, int blevel, int mode);
	virtual int32_t GetWidth(int frame);
	virtual int32_t GetHeight(int frame);
	virtual void SelectBitmapForDraw();
	virtual void VMethod7();
	virtual int GetPixelSize();


	CGameBitmap();
	CGameBitmap(const char* fname);
	CGameBitmap(const CGameBitmap* source);

	void* GetData() { return pdata + 8; };
	void ResetPalette(uint32_t count, int mode, int useColor);

	int32_t GetFrameCount() { return frames_count; } //4387f0

public:
	uint32_t frames_count;
	uint32_t data_size;
	BtmapFrame** frames;
	uint8_t* pdata;
	CGamePalette palette;
	uint8_t* palette_data;
};

ASSERT_SIZE(CGameBitmap, 0x24);


class CBmp256 : public CGameBitmap
{
	DECLARE_DYNAMIC(CBmp256);
public:
	virtual ~CBmp256();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode) override;
	virtual int32_t GetWidth(int frame) override;
	virtual int32_t GetHeight(int frame) override;
	virtual void VMethod7() override;
	virtual int GetPixelSize() override;

	virtual void VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid);
	virtual void VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid);

	CBmp256(const char* fname);
	CBmp256(uint32_t w, uint32_t h);
};


class CBmp64 : public CGameBitmap
{
	DECLARE_DYNAMIC(CBmp64);
public:
	virtual ~CBmp64();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode) override;
	virtual int32_t GetWidth(int frame) override;
	virtual int32_t GetHeight(int frame) override;
	virtual void VMethod7() override;

	virtual void VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod11(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);


	CBmp64(const char* fname);
	CBmp64(uint32_t w, uint32_t h);
};

class CSprite256 : public CGameBitmap
{
	DECLARE_DYNAMIC(CSprite256);
public:
	virtual ~CSprite256();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode) override;
	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode) override;
	virtual void VMethod3(int32_t x, int32_t y, int frame, int blevel, int mode) override;

	virtual void VMethod9(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode);
	virtual void VMethod10(int32_t x, int32_t y, int frame, int palid, int mode);
	virtual void VMethod11(int32_t x, int32_t y, int frame, int blevel, int arg5, int mode);
	virtual void VMethod12(int32_t x, int32_t y, int frame, uint32_t arg4);

	CSprite256(const char* fname);
	CSprite256(uint32_t w, uint32_t h);
};

class CA16 : public CSprite256
{
	DECLARE_DYNAMIC(CA16);
public:
	virtual ~CA16();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode) override;

	CA16(const char* fname);
};

class CSprite16 : public CGameBitmap
{
	DECLARE_DYNAMIC(CSprite16);
public:
	virtual ~CSprite16();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod9(int32_t x, int32_t y, int frame, uint16_t* pcolor);


	CSprite16(const char* fname);
};




//CSprite16 based
class CSpriteFont16 : public CGameFont
{
public:
	virtual ~CSpriteFont16();
	virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh) override;
	virtual uint16_t* GetShadowColors() override;

public:
	CSpriteFont16(const char* fname, int32_t _space);
};

//CA16 based
class CSpriteFont16a : public CGameFont
{
public:
	virtual ~CSpriteFont16a();
	virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh) override;
	virtual uint16_t* GetShadowColors() override;

public:
	CSpriteFont16a(const char* fname, int32_t _space);
};

ASSERT_SIZE(CSpriteFont16a, 0x10);




