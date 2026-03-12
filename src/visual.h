#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"



extern void FUN_00454c74(CRect *r);
extern void LockSurface2();
extern void UnlockSurface2();

extern uint16_t clrsh_DullGold[16];
extern uint16_t clrsh_TechBlack[16];




class VisLabel;
class CGameBitmap;

class CGameFont : public CObject
{
public:
	virtual ~CGameFont();
	
	virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh);
	virtual uint16_t* GetShadowColors();

public:
	CGameFont();
	void DrawTextWithShadow(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colorsh, int32_t shadow_dxy);

public:
	CGameBitmap* bitmap;
	int32_t* char_widths;
	int32_t space;
};

ASSERT_SIZE(CGameFont, 0x10);


class CVisualObject : public CObject
{
public:
	enum
	{
		FLAG_ENABLED = 1,
		FLAG_NOTFOCUS = 2,
		FLAG_FOCUS = 4,
		FLAG_OVERCURSOR = 8,
		FLAG_20 = 0x20,
	};
public:
	~CVisualObject();
	virtual void Dump(CDumpContext& dc) const override;

	virtual const char* GetHint();
	virtual void SetHint(const char *_hint);
	virtual void ChangeFlags(uint32_t _flags, bool setunset);
	virtual uint32_t TestFlags(uint32_t _flags);
	virtual void SetCursorOver(bool isOver);
	virtual void SetFocus(bool isFocus);
	virtual void VMethod7();
	virtual void VMethod8(RECT* rect);
	virtual void VMethod9();
	virtual void VMethod10();
	virtual void WriteData(void* buf);
	virtual uint32_t DataSize();
	virtual void ReadData(void* buf);
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam);
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos);
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos);
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos);
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos);
	virtual int32_t OnKeyDown(uint32_t wparam);
	virtual int32_t OnKeyUp(uint32_t wparam);
	virtual int32_t OnChar(uint32_t wparam);

public:
	CVisualObject();
	CVisualObject(int32_t _id, const RECT& r, const char* str);
	CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str);

	int32_t MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam);

	void AddChild(CVisualObject* obj);

	void RemoveChild(CVisualObject* obj);
	void RemoveChildById(int32_t _id);
	void RemoveAllChilds();

	void DestroyChild(CVisualObject* obj);
	void DestroyChildById(int32_t _id);
	void DestroyAllChilds();

	CVisualObject* FindChild(int32_t _id);
	CVisualObject* GetChildAt(POINT pt);
	
	void ClientPtToScreen(CPoint* out, const CPoint& _point);
	void ClientRectToScreen(CRect* out, const CRect& _rect);

	void SetLeftObj(CVisualObject* obj);
	void SetRightObj(CVisualObject* obj);
	void SetUpObj(CVisualObject* obj);
	void SetDownObj(CVisualObject* obj);

	void SetCaptionLabel(VisLabel* obj);

	void FocusTo(CVisualObject* obj, bool update);
	void TabFocus(bool forward, bool update);

	void SetRect(RECT r);
	void SetRect(const RECT* r);

public:
	int32_t id;
	CRect rect;
	uint32_t flags;
	CArray<CVisualObject*> childs;
	CVisualObject* parent;
	CVisualObject* cursor_over_obj;
	CVisualObject* focus_obj;
	CString hint;
	CVisualObject* cursor_over_obj_last;
	CVisualObject* last_focus_obj;
	CVisualObject* up_obj;
	CVisualObject* down_obj;
	CVisualObject* left_obj;
	CVisualObject* right_obj;
	VisLabel* caption_obj;
};

ASSERT_SIZE(CVisualObject, 0x5C);



class VisLabel : public CVisualObject
{
public:
	~VisLabel();

	virtual void VMethod7() override;

public:
	VisLabel();
	VisLabel(int32_t _id, const RECT& r, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align);
	VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _text, CGameFont *_font, uint16_t *colorsh, uint32_t align);

	void SetActiveColor(bool isActive);
public:

	CString text;
	CGameFont* font;
	uint16_t* color_sh;
	uint32_t align_flags;
};

ASSERT_SIZE(VisLabel, 0x6C);



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

	virtual void VMethod1(int32_t x, int32_t y, int frame, int palid, CGamePalette *ppalette, int mode);
	virtual void VMethod2(int32_t x, int32_t y, int frame, int palid, int mode);
	virtual void VMethod3(int32_t x, int32_t y, int frame, uint16_t *palette, int mode);
	virtual int32_t GetWidth();
	virtual int32_t GetHeight();
	virtual void SelectBitmapForDraw();
	virtual void VMethod7();
	virtual int GetPixelSize();


	CGameBitmap();
	CGameBitmap(const char* fname);
	CGameBitmap(const CGameBitmap* source);

	void* GetData();
	void ResetPalette(uint32_t count, int mode, int useColor);


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
	virtual int32_t GetWidth() override;
	virtual int32_t GetHeight() override;
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
	virtual int32_t GetWidth() override;
	virtual int32_t GetHeight() override;
	virtual void VMethod7() override;

	virtual void VMethod9(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod10(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);
	virtual void VMethod11(int32_t x, int32_t y, int32_t l, int32_t t, int32_t r, int32_t b);


	CBmp64(const char *fname);
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
	virtual void VMethod3(int32_t x, int32_t y, int frame, uint16_t* palette, int mode) override;

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



class CMousePointer : public CObject
{
	DECLARE_DYNAMIC(CMousePointer);
public:
	virtual ~CMousePointer();

public:
	CSprite256* field_0x4;
	CBmp64* field_0x8;
	CBmp64* field_0xc;
	uint32_t field_0x10;
	uint32_t field_0x14;
	uint32_t field_0x18;
	uint32_t field_0x1c;
	uint32_t field_0x20;
	uint32_t field_0x24;
	uint32_t field_0x28;
	uint32_t field_0x2c;
	uint32_t field_0x30;
	uint32_t field_0x34;
	uint32_t field_0x38;
	CString  field_0x3c;
	uint32_t field_0x40;
	uint32_t field_0x44;
	CRect	 field_0x48;
	uint32_t field_0x58;
	uint32_t field_0x5c;
	uint32_t field_0x60;
	CBmp64* field_0x64;
	CBmp64* field_0x68;
	CRect    field_0x6c;
	CBmp64* field_0x7c[8];
	uint32_t field_0x9c;
	uint32_t field_0xa0;
};

ASSERT_SIZE(CMousePointer, 0xa4);


//CSprite16 based
class CSpriteFont16 : public CGameFont
{
public:
	virtual ~CSpriteFont16();
	virtual void DrawTxt(int32_t x, int32_t y, const char* txt, uint32_t align, uint16_t* colosh) override;
	virtual uint16_t* GetShadowColors() override;

public:
	CSpriteFont16(const char *fname, int32_t _space);
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

#endif
