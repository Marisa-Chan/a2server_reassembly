#pragma once

#include <afx.h>
#include <afxtempl.h>
#include "assert_offset.h"
#include "2darray.h"

#include <ddraw.h>

extern DDSURFACEDESC g_selDrawBitmap; //659c98



class CGameBitmap;
class CSprite256;
class CGameFont;
class CBmp64;
class CBmp256;
class CA16;
class CGamePalette;


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

extern CGamePalette* palette_husk; //65f628
extern CGamePalette* palette_paris_daisy; //65f52c
extern CGamePalette* palette_brown_derby; //65f5e8
extern CGamePalette* palette_tawny_port; //65f5a0



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
extern uint16_t clrsh_Oxley[16]; //65f4d8
extern uint16_t clrsh_TechBlack[16]; //65f090
extern uint16_t clrsh_ShockingBlack[16]; //65e1e8
extern uint16_t clrsh_GoldenFizz[16]; //65f578
extern uint16_t clrsh_CharlieBrown[16]; //65f4b8
extern uint16_t clrsh_LuxorGold[16]; //65e848
extern uint16_t clrsh_CoralRed[16]; //65f608
extern uint16_t clrsh_InvBarleyCorn[16]; //65e208

extern uint16_t g_colors_human_pals[16][16]; //65e230

extern int32_t g_MessageColors; //660f80
extern uint16_t g_ColorAddMask; //660f60

extern uint16_t* g_brightnessLookup; //65dd20
extern uint32_t g_brightnessLookupCount; //659bf0

extern uint32_t g_isLowMemory; //660f58

extern CGamePalette* g_TerPalettes[32]; //661018
extern RGBQUAD g_human_pals_data[16][256]; //661118
extern CArray<CGamePalette*> g_Human_palettes; //6610c8

extern Array2D<CBmp256*, 4, 32> g_ter_tiles; //665130
extern CGameBitmap* g_TerDirtBmp; //665344


void GetClipRect(CRect* r); // 454a0c (client: 45864c)
void SetClipRect(CRect& r); // 4549ba (client: 4585fa)

void __cdecl SetClipRect(int32_t l, int32_t t, int32_t r, int32_t b); // 4549e5 (client: 458625)
void __cdecl CopyScreenBlock(int32_t sx, int32_t sy, int32_t w, int32_t h); // 454e40 (client: 458a80)

void CopyIndexedSkip0(uint8_t* dst, uint8_t* src, int32_t size); // 45424d (client: 457e8d)
void DrawFlatTile(int32_t x, int32_t y, int32_t l1, int32_t l2, int32_t l3, int32_t l4, uint8_t* src, uint16_t* pal); // 458b29 (client: 45c757)
void DrawDeformedTile(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3, int32_t l1, int32_t l2, int32_t l3, int32_t l4, uint8_t* src, uint16_t* pal); // 458ca0 (client: 45c8ce)

void FillFlatTerrain(int32_t x, int32_t h0, int32_t h2); // 458fe6 (client: 45cc14)
void FillDeformedTerrain(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3); // 4590ef (client: 45cd1d)
void FillFlatLight(int32_t x, int32_t h0, int32_t t0, int32_t t1, int32_t t2, int32_t t3); // 459449 (client: 45d077)
void FillDeformedLight(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3, int32_t t0, int32_t t1, int32_t t2, int32_t t3); // 4595fd (client: 45d22b)
void FillFlatGrey(int32_t x, int32_t h0, int32_t h2); // 45995e (client: 45d58c)
void FillDeformedGrey(int32_t x0, int32_t x1, int32_t h0, int32_t h1, int32_t h2, int32_t h3); // 459b72 (client: 45d7a0)


inline uint32_t GetColorRGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> (8 - g_RBits)) << g_RBitShift) | ((g >> (8 - g_GBits)) << g_GBitShift) | ((b >> (8 - g_BBits)) << g_BBitShift);
}



namespace shadow_level {
enum : int32_t {
    disabled      = 3,
    char_selected = 8,
    list_selected = 10,
    text_select   = 12,
};
} // namespace shadow_level

void __cdecl SetPixelColor(int32_t x, int32_t y, uint32_t clr); // 4586a4 (client: 45c2d2)
void __cdecl FillRectColor(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); // 457dcc (client: 45ba0c)
void __cdecl FillRectColorSimple(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); // 4579d8 (client: 45b618)
void __cdecl FillRectClipped(int32_t l, int32_t t, int32_t r, int32_t b, uint16_t clr); // 457aa6 (client: 45b6e6)
void __cdecl ShadowRect(CRect rect, int shadow); // 457b6f (client: 45b7af)

void __cdecl FillRectBrightness(int32_t l, int32_t t, int32_t r, int32_t b, int32_t brightness); // 457c5d (client: 45b89d)
void __cdecl PlotPixelBlend(int32_t x, int32_t y, int32_t r, int32_t g, int32_t b, int32_t alpha); // 4586fe (client: 45c32c)
void __cdecl PlotGlowDot(int32_t x, int32_t y, int32_t r, int32_t g, int32_t b, int32_t alpha); // 4588ec (client: 45c51a)

void __cdecl gfxFlushRect(const CRect& rect); // 454c74 (client: 4588b4)
void __cdecl DrawRectangleFrame(int32_t l, int32_t t, int32_t r, int32_t b, uint32_t clr); // 458035 (client: 45bc73)

void __cdecl FlushScreen(); // 454e18 (client: 458a58)

void __cdecl CopyScreenRectToBmp64(int32_t x, int32_t y, int32_t w, int32_t h, int32_t dw, int32_t dh, uint8_t* dst); // 45455e (client: 45819e)

uint32_t LockSurface2(); // 45426e (client: 457eae)
uint32_t UnlockSurface2(); // 4542ca (client: 457f0a)
uint32_t LockSurface3(); // 4543ca (client: 45800a)
uint32_t UnlockSurface3(); // 45444f (client: 45808f)
uint32_t LockSurface1(); // 45431c (client: 457f5c)
uint32_t UnlockSurface1(); // 454378 (client: 457fb8)
#ifdef A2CLIENT_PATCH
extern CRect g_DisplaySize;
void PresentSurface1Rect(const CRect& rect);
CPoint PhysicalToLogicalPoint(POINT pt);
CPoint LogicalToPhysicalPoint(int32_t x, int32_t y);
CRect LogicalToPhysicalRect(const CRect& rect);
CRect PhysicalToLogicalRect(const CRect& rect);
BOOL GetLogicalCursorPos(LPPOINT lpPoint);
BOOL SetLogicalCursorPos(int32_t x, int32_t y);
#endif

uint32_t GetLockCountSurf2(); // 4538e1 (client: 457521)

int32_t InitVideo(); // 453fcc (client: 457c0c)
int32_t SetVideoMode(); // 4546c6 (client: 458306)

void LoadGraphics(); // 47819d (client: 47bedd)
void UnloadGraphics(); // 47961c (client: 47d439)

void FreeFontData(); // 460d7f (client: 464991)
void FreeDDraw(); // 454219 (client: 457e59)
void FreeBrightnessLookup(); // 453cf9 (client: 457939)
void SaveScreenshot(); // 453d33 (client: 457973)
void UnloadUnitGfxFiles(); // 47c6cd (client: 480587)
void FlushRect(int32_t l, int32_t t, int32_t r, int32_t b); // 454a1e (client: 45865e)
extern IDirectDraw* g_ddraw; //659bec


struct ColorThing {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t __pd1;
    int32_t brightness;
    int32_t contrast;
    int32_t shade_delta;
    int32_t lit_delta;
    int32_t __pd2;
    double sun_angle;
};
ASSERT_SIZE(ColorThing, 0x20);

extern ColorThing g_DeltaCLR; //660e28



class CGameFont : public CObject {
public:
    enum AlignFlags : uint32_t {
        ALIGN_LEFT_TOP = 0,
        ALIGN_RIGHT    = 0x01,
        ALIGN_HCENTER  = 0x02,
        ALIGN_BOTTOM   = 0x04,
        ALIGN_VCENTER  = 0x08,
        ALIGN_CENTER   = ALIGN_HCENTER | ALIGN_VCENTER,
    };

    virtual ~CGameFont(); // 45e0f9 (client: 461d19)

    virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh); // 460f60 (client: 464b40)
    virtual uint16_t* GetShadowColors(); // 460f70 (client: 464b50)

public:
    CGameFont(); // 45e0d0 (client: 461cf0)
    void DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy); // 45ebda (client: 4627f6)

    int32_t GetStrWidth(const char* str); // 45e1f9 (client: 461e19)

    int32_t GetHeight(); // 402a90 (client: 402860)

    CStringArray& StrToArray(const char* str); // 45e959 (client: 462575) - internal
    CStringArray& StrSplitToFitWidth(const CRect& r, const char* str); // 45e35f (client: 461f7b) - internal

    CStringArray& StringArrayForRect(const CRect& r, const char* str); // 45eb1d (client: 462739)
    void DrawTextLinesShadow(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t *clr, int32_t dy); // 45f6c2 (client: 4632dd)
    void DrawTextLines(const CRect& r, int32_t first, int32_t last, const CStringArray& lines, uint16_t* clr, int32_t dy); // 45f33e (client: 462f5a)

    void DrawTextJustifyInRectShadow(const CRect& r, const char* str, uint16_t* clr, int32_t dy); // 45ed37 (client: 46284f)

    void DrawTextJustifyShadow(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr); // 45f10b (client: 462d27)
    void DrawTextJustify(int32_t x, int32_t y, int32_t w, CString txt, uint16_t* clr); // 45eed7 (client: 462af3)
public:
    CGameBitmap* bitmap;
    int32_t* char_widths;
    int32_t space;
};

ASSERT_SIZE(CGameFont, 0x10);


class CGamePalette : public CObject {
    DECLARE_DYNAMIC(CGamePalette);
public:
    virtual ~CGamePalette(); // 423c6d (client: 425b9d)
    virtual void Dump(CDumpContext& dc) const override; // 4245a7 (client: 4264d7)

    void operator=(const CGamePalette& b);

    CGamePalette() = default; // 423bdf (client: 425b0f)
    void Free(); // 42457a (client: 4264aa)
    void SetPalette(RGBQUAD* rgb, uint32_t count, int mode, int useColor); // 423cbe (client: 425bee)
    uint16_t* GetPalette(int32_t idx); // 41ec40 (client: 420440)

public:
    uint32_t count = 0;
    uint16_t* colors = nullptr;
};

ASSERT_SIZE(CGamePalette, 0xC);

struct BmpFrame {
    uint32_t width;
    uint32_t height;
    uint8_t data[];
};
ASSERT_SIZE(BmpFrame, 8);

struct SpriteFrame {
    uint32_t width;
    uint32_t height;
    uint32_t datasize;
    uint8_t data[];
};
ASSERT_SIZE(SpriteFrame, 0xc);

struct SpriteTrailer {
    union {
        uint32_t raw;
        struct {
            uint32_t frames_count : 31; // [30:0] 0x7FFFFFFF: frame count
            uint32_t has_palette  : 1;  // [31]   0x80000000: 256-entry RGBQUAD palette precedes frame data
        };
    };

    constexpr SpriteTrailer(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
};
ASSERT_SIZE(SpriteTrailer, 4);

struct Rle8Cmd {
    enum Op : uint8_t {
        OP_PIXELS      = 0, // 0x00: run of `count` source pixels (or nibble pairs in Rle4)
        OP_SKIP_LINES  = 1, // 0x40: advance down `count` scanlines
        OP_SKIP_PIXELS = 2, // 0x80: skip `count` transparent pixels horizontally
    };

    union {
        uint8_t raw;
        struct {
            uint8_t count : 6; // [5:0] 0x3F
            uint8_t op    : 2; // [7:6] 0xC0 (Op)
        };
    };

    constexpr Rle8Cmd(uint8_t v = 0) : raw(v) {}
    constexpr operator uint8_t() const { return this->raw; }
};
ASSERT_SIZE(Rle8Cmd, 1);

struct Rle16Cmd {
    enum Op : uint16_t {
        OP_PIXELS      = 0, // 0x0000: run of `count` Rle16Pixel entries
        OP_SKIP_LINES  = 1, // 0x4000: advance down `count` scanlines
        OP_SKIP_PIXELS = 2, // 0x8000: skip `count` transparent pixels horizontally
    };

    union {
        uint16_t raw;
        struct {
            uint16_t count : 14; // [13:0]  0x3FFF
            uint16_t op    : 2;  // [15:14] 0xC000 (Op)
        };
    };

    constexpr Rle16Cmd(uint16_t v = 0) : raw(v) {}
    constexpr operator uint16_t() const { return this->raw; }
};
ASSERT_SIZE(Rle16Cmd, 2);

struct Rle16Pixel {
    union {
        uint16_t raw;
        struct {
            uint16_t unused_lsb  : 1; // [0]     0 (word byte-offset alignment)
            uint16_t color_index : 8; // [8:1]   palette entry (0..255)
            uint16_t alpha_level : 4; // [12:9]  0x1E00: destination shade/alpha level (0..15)
            uint16_t unused_high : 3; // [15:13]
        };
        struct {
            uint16_t align_bit   : 1;  // [0]
            uint16_t clut_index  : 15; // [15:1]  (alpha_level << 8) | color_index == raw / 2
        };
    };

    constexpr Rle16Pixel(uint16_t v = 0) : raw(v) {}
    constexpr operator uint16_t() const { return this->raw; }
};
ASSERT_SIZE(Rle16Pixel, 2);


class CGameBitmap : public CObject {
    DECLARE_DYNAMIC(CGameBitmap);
public:
    virtual ~CGameBitmap(); // 42496a (client: 42689a)
    virtual void Dump(CDumpContext& dc) const override; // 424d3d (client: 426c6d)

    virtual void DrawWithPalette(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode); // 428d60 (client: 42a820)
    virtual void Draw(int32_t x, int32_t y, int frame, int mode, int palid); // 428d50 (client: 42a810)
    virtual void DrawBrightened(int32_t x, int32_t y, int frame, int blevel, int mode); // 428d70 (client: 42a830)
    virtual int32_t GetWidth(int frame = 0); // 424ce6 (client: 426c16)
    virtual int32_t GetHeight(int frame = 0); // 424d11 (client: 426c41)
    virtual void SelectBitmapForDraw(); // 424d57 (client: 426c87)
    virtual void FlipVertical(); // 428d80 (client: 42a840)
    virtual int GetPixelSize(); // 428d90 (client: 42a850)


    CGameBitmap(); // 4247a6 (client: 4266d6)
    CGameBitmap(const char* fname); // 4245c1 (client: 4264f1)
    CGameBitmap(const CGameBitmap& source); // 4247fd (client: 42672d)

    void* GetData(); // 41ec60 (client: 420460)
    void ResetPalette(uint32_t count, int mode, int useColor); // 424cb0 (client: 426be0)

    int32_t GetFrameCount(); // 4387f0 (client: 43ae10)

public:
    uint32_t frames_count = 0;
    uint32_t data_size = 0;
    void** frames = nullptr;
    uint8_t* pdata = nullptr;
    CGamePalette palette;
    uint8_t* palette_data = nullptr;
};

ASSERT_SIZE(CGameBitmap, 0x24);


class CBmp256 : public CGameBitmap {
    DECLARE_DYNAMIC(CBmp256);
public:
    virtual ~CBmp256(); // 428a40 (client: 42aba0)
    virtual void Dump(CDumpContext& dc) const override; // 42649a (client: 4283c8)

    virtual void Draw(int32_t x, int32_t y, int frame, int mode, int palid) override; // 4263df (client: 42830d)
    virtual int32_t GetWidth(int frame = 0) override; // 4264c0 (client: 4283ee)
    virtual int32_t GetHeight(int frame = 0) override; // 4264d5 (client: 428403)
    virtual void FlipVertical() override; // 4261a6 (client: 4280d4)
    virtual int GetPixelSize() override; // 428da0 (client: 42ab60)

    virtual void DrawSubRect(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid); // 426385 (client: 4282b3)
    virtual void DrawSubRectAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b, int palid); // 426440 (client: 42836e)

    CBmp256(const char* fname); // 425f8c (client: 427eba)
    CBmp256(uint32_t w, uint32_t h); // 4262d1 (client: 4281ff)
};


class CBmp64 : public CGameBitmap {
    DECLARE_DYNAMIC(CBmp64);
public:
    virtual ~CBmp64(); // 4289f0 (client: 42ab40)
    virtual void Dump(CDumpContext& dc) const override; // 425f3b (client: 427e69)

    virtual void Draw(int32_t x, int32_t y, int frame, int mode, int palid) override; // 425e0c (client: 427d3a)
    virtual int32_t GetWidth(int frame = 0) override; // 425f61 (client: 427e8f)
    virtual int32_t GetHeight(int frame = 0) override; // 425f76 (client: 427ea4)
    virtual void FlipVertical() override; // 425cdb (client: 427c09)

    virtual void DrawSubRect(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b); // 425e5d (client: 427d8b)
    virtual void DrawSubRectMasked(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b); // 425ea7 (client: 427dd5)
    virtual void DrawSubRectAdd(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b); // 425ef1 (client: 427e1f)


    CBmp64(const char* fname); // 42576d (client: 42769b)
    CBmp64(uint32_t w, uint32_t h); // 42597e (client: 4278ac)

    void LoadFile(const char* fname, CGameBitmap* bmp2 = nullptr); // 425a17 (client: 427945)
    void WriteFile(const char* fname, CGameBitmap* bmp2 = nullptr); // 425b95 (client: 427ac3)
};

class CSprite256 : public CGameBitmap {
    DECLARE_DYNAMIC(CSprite256);
public:
    virtual ~CSprite256(); // 4289a0 (client: 42aaf0)
    virtual void Dump(CDumpContext& dc) const override; // 425558 (client: 427486)

    virtual void DrawWithPalette(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode) override; // 425210 (client: 427140)
    virtual void Draw(int32_t x, int32_t y, int frame, int palid, int mode) override; // 4250e4 (client: 427014)
    virtual void DrawBrightened(int32_t x, int32_t y, int frame, int blevel, int mode) override; // 425330 (client: 427260)

    virtual void DrawWithPaletteBlend(int32_t x, int32_t y, int frame, int palid, CGamePalette* ppalette, int mode); // 4252a0 (client: 4271d0)
    virtual void DrawBlend(int32_t x, int32_t y, int frame, int palid, int mode); // 42517a (client: 4270aa)
    virtual void DrawShadowed(int32_t x, int32_t y, int frame, int blevel, int32_t tilt, int mode); // 425414 (client: 427344)
    virtual void DrawIndexMask(int32_t x, int32_t y, int frame, uint32_t mv); // 425508 (client: 427438)

    CSprite256(const char* fname); // 424e79 (client: 426da9)
    CSprite256(const CSprite256& b); // 4250bf (client: 426fef)

#ifdef A2CLIENT_PATCH
protected:
    // Patch subclasses (patch/itempng.h) construct without loading a .256 file.
    CSprite256() = default;
public:
#endif
};

class CA16 : public CSprite256 {
    DECLARE_DYNAMIC(CA16);
public:
    virtual ~CA16() {}; // 428b10 (client: 42acb0)

    virtual void Draw(int32_t x, int32_t y, int frame, int palid, int mode) override; // 428824 (client: 42a740)

    CA16(const char* fname); // 4287ff (client: 42a71b)
};

class CSprite16 : public CGameBitmap {
    DECLARE_DYNAMIC(CSprite16);
public:
    virtual ~CSprite16() {}; // 428950 (client: 42aaa0)
    virtual void Dump(CDumpContext& dc) const override {}; // 424e2b (client: 426d5b)

    virtual void DrawRle4(int32_t x, int32_t y, int frame, uint16_t* pcolor); // 424ddd (client: 426d0d)


    CSprite16(const char* fname); // 424db8 (client: 426ce8)
};




//CSprite16 based
class CSpriteFont16 : public CGameFont {
public:
    virtual ~CSpriteFont16() {}; // 460fb0 (client: 464bc0)
    virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* clr) override; // 45fbd7 (client: 4637f1)
    virtual uint16_t* GetShadowColors() override; // 45fdf2 (client: 463a06)

public:
    CSpriteFont16(const char* fname, int32_t _space); // 45fa44 (client: 46365e)
};

//CA16 based
class CSpriteFont16a : public CGameFont {
public:
    virtual ~CSpriteFont16a(); // 461000 (client: 464c10)
    virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh) override; // 45ffa9 (client: 463bbd)
    virtual uint16_t* GetShadowColors() override; // 460106 (client: 463d18)

public:
    CSpriteFont16a(const char* fname, int32_t _space); // 45fe02 (client: 463a16)
};

ASSERT_SIZE(CSpriteFont16a, 0x10);
