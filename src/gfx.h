#pragma once

#include "asm_mfc.h"



class CGameBitmap;
class CSprite256;
class CGameFont;
class CBmp64;
class CBmp256;
class CA16;


extern CRect g_ScreenSize; //65fb78


extern CBmp64* g_bmp_t_back; //0066512c
extern CBmp64* g_bmp_crystalr; //00665348
extern CBmp64* g_bmp_crystall; //0066534c
extern CBmp64* g_bmp_headsr; //00665350
extern CBmp64* g_bmp_headsl; //00665354
extern CBmp64* g_bmp_cmdbarr; //00665358
extern CBmp64* g_bmp_cmdbarl; //0066535c
extern CBmp64* g_bmp_cmddnr; //00665360
extern CBmp64* g_bmp_cmdempr; //00665364
extern CBmp64* g_bmp_humanbackl; //00665368
extern CBmp64* g_bmp_humanbackr; //0066536c
extern CBmp64* g_bmp_textbackl; //00665370
extern CBmp64* g_bmp_textbackr; //00665374
extern CBmp64* g_bmp_bookopened; //00665378
extern CBmp64* g_bmp_bookclosed; //0066537c
extern CBmp64* g_bmp_backpackop; //00665380
extern CBmp64* g_bmp_backpackcl; //00665384
extern CBmp64* g_bmp_humanmode; //00665388
extern CBmp64* g_bmp_textmode; //0066538c
extern CBmp64* g_bmp_diskette; //00665390
extern CBmp64* g_bmp_ar1; //00665394
extern CBmp64* g_bmp_ar2; //00665398
extern CBmp64* g_bmp_ar3; //0066539c
extern CBmp64* g_bmp_ar4; //006653a0
extern CBmp64* g_bmp_spellbook; //006653a4
extern CBmp64* g_bmp_spellback; //006653a8
extern CBmp64* g_bmp_invframe; //006653ac
extern CBmp64* g_bmp_invarrow1; //006653b0
extern CBmp64* g_bmp_invarrow2; //006653b4
extern CBmp64* g_bmp_invarrow3; //006653b8
extern CBmp64* g_bmp_invarrow4; //006653bc
extern CBmp64* g_bmp_backinv; //006653c0
extern CBmp64* g_bmp_spb1024l; //006653c4
extern CBmp64* g_bmp_spb1024r; //006653c8
extern CBmp64* g_bmp_extra1024l; //006653cc
extern CBmp64* g_bmp_extra1024r; //006653d0
extern CBmp64* g_bmp_inv1024l; //006653d4
extern CBmp64* g_bmp_inv1024r; //006653d8
extern CBmp64* g_bmp_spb800l; //006653dc
extern CBmp64* g_bmp_spb800r; //006653e0
extern CBmp64* g_bmp_extra800l; //006653e4
extern CBmp64* g_bmp_extra800r; //006653e8
extern CSprite256* gfx_interface_lm; //006653ec
extern CSprite256* gfx_scrollbars; //006653f0
extern CSprite256* gfx_radiob; //006653f4
extern CSprite256* g_spr_t_border; //006653f8
extern CSprite256* g_spr_backm; //006653fc
extern CSprite256* g_spr_backf; //00665400
extern CA16* g_ca16_money; //00665404
extern CBmp64* g_bmp_server; //00665408
extern CSprite256* g_spr_backpack; //0066540c
extern CSprite256* g_spr_backpackb; //00665410
extern CBmp64* gfx_ball; //00665414
extern CBmp64* g_bmp_minimapdata; //00665418
extern CBmp256* g_bmp_testiva; //0066541c

extern CGameFont* g_font1; //65e228
extern CGameFont* g_font2; //65ec68
extern CGameFont* g_font3; //65f598
extern CGameFont* g_font4; //65f530



extern int32_t g_Shadows; //62f880
extern int32_t g_Animation; //62f884
extern int32_t g_Lightning; //62f888

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
extern uint16_t clrsh_CoralRed[16]; //65f608

extern uint16_t g_colors_human_pals[16][16]; //65e230

extern int32_t g_MessageColors; //660f80

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

void __cdecl FlushScreen(); //454e18

void __cdecl CopyScreenRectToBmp64(int32_t x, int32_t y, int32_t w, int32_t h, int32_t dw, int32_t dh, uint8_t* dst); //45455e

uint32_t LockSurface2(); // 45426e
uint32_t UnlockSurface2(); // 4542ca
uint32_t LockSurface1(); // 45431c
uint32_t UnlockSurface1(); // 454378

uint32_t GetLockCountSurf2(); //4538e1

int32_t InitVideo(); //453fcc
int32_t SetVideoMode(); //4546c6

void LoadGraphics(); //47819d
void UnloadGraphics(); //47961c

void FreeFontData(); //460d7f
void FreeDDraw(); //454219
void FreeBrightnessLookup(); //453cf9


struct ColorThing
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t __pd1;
	int32_t field_4;
	int32_t field_8;
	int32_t field_c;
	int32_t field_10;
	int32_t __pd2;
	double field_18;
};
ASSERT_SIZE(ColorThing, 0x20);

extern ColorThing g_DeltaCLR; //660e28



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
	virtual ~CGamePalette(); //423c6d
	virtual void Dump(CDumpContext& dc) const override;

	void operator=(const CGamePalette& b);

	CGamePalette() = default; // 423bdf
	void Free();
	void SetPalette(RGBQUAD* rgb, uint32_t count, int mode, int useColor); //423cbe
	uint16_t* GetPalette(int32_t idx); //41ec40

public:
	uint32_t count = 0;
	uint16_t* colors = nullptr;
};

ASSERT_SIZE(CGamePalette, 0xC);

struct BmpFrame
{
	uint32_t width;
	uint32_t height;
	uint8_t data[];
};
ASSERT_SIZE(BmpFrame, 8);

struct SpriteFrame
{
	uint32_t width;
	uint32_t height;
	uint32_t datasize;
	uint8_t data[];
};
ASSERT_SIZE(SpriteFrame, 0xc);


class CGameBitmap : public CObject
{
	DECLARE_DYNAMIC(CGameBitmap);
public:
	virtual ~CGameBitmap();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode);
	virtual void VMethod2(int32_t x, int32_t y, int frame, int mode, int palid);
	virtual void VMethod3(int32_t x, int32_t y, int frame, int blevel, int mode);
	virtual int32_t GetWidth(int frame = 0);
	virtual int32_t GetHeight(int frame = 0);
	virtual void SelectBitmapForDraw();
	virtual void VMethod7();
	virtual int GetPixelSize();


	CGameBitmap();
	CGameBitmap(const char* fname);
	CGameBitmap(const CGameBitmap& source);

	void* GetData();
	void ResetPalette(uint32_t count, int mode, int useColor);

	int32_t GetFrameCount(); //4387f0

public:
	uint32_t frames_count = 0;
	uint32_t data_size = 0;
	void** frames = nullptr;
	uint8_t* pdata = nullptr;
	CGamePalette palette;
	uint8_t* palette_data = nullptr;
};

ASSERT_SIZE(CGameBitmap, 0x24);


class CBmp256 : public CGameBitmap
{
	DECLARE_DYNAMIC(CBmp256);
public:
	virtual ~CBmp256();
	virtual void Dump(CDumpContext& dc) const override;

	virtual void VMethod2(int32_t x, int32_t y, int frame, int mode, int palid) override;
	virtual int32_t GetWidth(int frame = 0) override;
	virtual int32_t GetHeight(int frame = 0) override;
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

	virtual void VMethod2(int32_t x, int32_t y, int frame, int mode, int palid) override;
	virtual int32_t GetWidth(int frame = 0) override;
	virtual int32_t GetHeight(int frame = 0) override;
	virtual void VMethod7() override;

	virtual void VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod11(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);


	CBmp64(const char* fname);
	CBmp64(uint32_t w, uint32_t h);

	void LoadFile(const char* fname, CGameBitmap* bmp2 = nullptr);
	void WriteFile(const char* fname, CGameBitmap* bmp2 = nullptr);
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
	virtual void VMethod11(int32_t x, int32_t y, int frame, int blevel, int32_t arg5, int mode);
	virtual void VMethod12(int32_t x, int32_t y, int frame, uint32_t arg4);

	CSprite256(const char* fname);
	CSprite256(const CSprite256& b);
};

class CA16 : public CSprite256
{
	DECLARE_DYNAMIC(CA16);
public:
	virtual ~CA16() {}; //428ae0

	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode) override;

	CA16(const char* fname);
};

class CSprite16 : public CGameBitmap
{
	DECLARE_DYNAMIC(CSprite16);
public:
	virtual ~CSprite16() {}; //428950
	virtual void Dump(CDumpContext& dc) const override {}; //424e2b

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




