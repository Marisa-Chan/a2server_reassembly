#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"
#include "sound.h"
#include "gameobj.h"


extern void LockSurface2();
extern void UnlockSurface2();



class SomeMainStructure;
class VisLabel;
class CGameBitmap;
class CGameObject;
class CGameFont;
class CBmp64;
class BigStruct2;
class VisCharSellectStats;
class VisCharSellectButtons;
class VisCharSellectList;

class CUnit;



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
	virtual void VMethod8(CRect* rect);
	virtual void VMethod9();
	virtual void VMethod10();
	virtual void WriteData(void* buf);
	virtual uint32_t DataSize();
	virtual void ReadData(const void* buf);
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

	CPoint ClientPtToScreen(const CPoint& _point);
	CRect ClientRectToScreen(const CRect& _rect);

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


//60df40
class VisButton : public CVisualObject
{
public:
	virtual ~VisButton();

	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;
	virtual int32_t OnChar(uint32_t wparam) override;

	VisButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint);
	VisButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint);

	void SetDowned(bool down);
public:
	CString caption;
	CGameFont* font;
	uint16_t* clr;
	uint32_t mouse_on;
	uint32_t downed;
	int32_t msgid;
	int32_t charid;
};
ASSERT_SIZE(VisButton, 0x78);


//60e1d0
class VisScrollBar : public CVisualObject
{
public:
	virtual ~VisScrollBar();

	virtual void SetCursorOver(bool isOver) override;
	virtual void VMethod7() override;
	virtual void VMethod9() override;
	virtual void WriteData(void* buf) override;
	virtual void ReadData(const void* buf) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisScrollBar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* hint);


	void SetPos(int32_t pos, int32_t pos_max);
	void UpdateRects();
	void UpdateHBoxes();

	int32_t GetValHPos(int32_t x);

public:
	int32_t field_0x5c;
	int32_t is_mouse_over;
	int32_t mouse_on_minus;
	int32_t mouse_on_plus;
	int32_t mousedown_on_hbox;
	int32_t down_on_part;
	CRect btn_minus;
	CRect btn_plus;
	CRect horiz_box_pos;
	CRect rect4;
	CRect rect5;
	int32_t val;
	int32_t val_max;

public:
	struct Data
	{
		int32_t v;
		int32_t vmax;
	};
};
ASSERT_SIZE(VisScrollBar, 0xcc);

//60e0b0
class VisListBox : public CVisualObject
{
public:
	~VisListBox();

	virtual void VMethod7() override;
	virtual void WriteData(void* buf) override;
	virtual uint32_t DataSize() override;
	virtual void ReadData(const void* buf) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	virtual int32_t IsValidIndex(int32_t idx); //26
	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr); //27
	virtual void SelectItem(int32_t idx); //28
	virtual int32_t GetItemCount(); //29
	virtual void VMethod30(CPoint pos, const CRect& r); //30 

	VisListBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);
	VisListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);

	void Down();
	void Up();
	void PageDown();
	void PageUp();

	int32_t YToIndex(int32_t y); //Get index of y coordinate

public:
	int32_t entry_height;
	int32_t entry_height_full;
	CStringArray entries;
	CGameFont* font;
	uint16_t* clr1;
	uint16_t* clr2;
	int32_t vis_start_index;
	int32_t selected_index;
	int32_t num_vis_entry;
	int32_t scrollbox_id;
};
ASSERT_SIZE(VisListBox, 0x94);

//60e2c0
class VisScreen : public CVisualObject
{
public:
	virtual ~VisScreen();
	virtual void VMethod7() override;
	virtual void VMethod8(CRect* rect) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4dfc43

	virtual void VMethod26(); //450900
	virtual void VMethod27(); //4388c0
	virtual void VMethod28(); //4dfb4f
	virtual void DoClose(uint32_t code); //4dfb8a 29 method   

	VisScreen();
	VisScreen(int32_t _id, const RECT& r, CGameBitmap* _bitmap);
	VisScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap);

	int32_t GetCloseCode(); //
	void CloseOk();
	void CloseCancel();
public:
	int32_t is_active = 0;
	int32_t exit_code = 0;
	CGameBitmap* bitmap = nullptr;
};
ASSERT_SIZE(VisScreen, 0x68);


//60e348
class VisWindow : public VisScreen
{
public:
	virtual ~VisWindow();
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap);

	void UpdateWinRect(); //4dfe19
};
ASSERT_SIZE(VisWindow, 0x68);


//44402b
class VisMessageBox : public VisWindow
{
public:
	virtual ~VisMessageBox();
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual void VMethod26() override;

	virtual CVisualObject* VMethod30(const char* str, const RECT& r) = 0;
	virtual void VMethod31(int32_t code);

	VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes, const char* str3);

public:
	const char* field_0x68;
	const char* field_0x6c;
	const char* field_0x70;
	int32_t button_types;
};
ASSERT_SIZE(VisMessageBox, 0x78);

//60a318
class VisMessageBoxWithList : public VisMessageBox
{
public:
	virtual ~VisMessageBoxWithList();
	virtual CVisualObject* VMethod30(const char* str, const RECT& r);

	VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes);
};
ASSERT_SIZE(VisMessageBoxWithList, 0x78);


//609898
class VisCharSelect : public VisScreen
{
public:
	virtual void VMethod8(CRect* rect) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;

	virtual void VMethod26() override;
	virtual void VMethod28() override;
	virtual void DoClose(uint32_t code) override;


	void OpenRenameWindow(); //4327f3

	void FUN_00432655(CUnit* unit);

public:
	SomeMainStructure* pCharacters;
	CVisualObject* field_0x6c;
	BigStruct2* gameinterface;
	VisCharSellectStats* vis_stats;
	VisCharSellectButtons* buttons;
	VisCharSellectList* vis_list;
	CVisualObject* field_0x80;
	CUnit* field_0x84;
	int32_t field_0x88;
	CSound field_0x8c;
	int32_t field_0x90;
	int32_t field_0x94;
	int32_t field_0x98;
};
ASSERT_SIZE(VisCharSelect, 0x9c);

//6097a8
class VisCharSellectButtons : public CVisualObject
{
public:
	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;

public:
	VisCharSelect* parent_screen;
	CStringArray field_0x60;
	CBmp64* buttons_bmp[4];
	void* field_0x84[4];
	CBmp64* bmp_area;
	CRect areas[4];
	int32_t mouse_down_box;
	int32_t mouse_over_box;

public:
	int GetMouseOnBox(CPoint pos);
	void UpdateMouseOverBox(uint32_t wparam, CPoint pos);
};
ASSERT_SIZE(VisCharSellectButtons, 0xe0);

//609820
class VisCharSellectList : public CVisualObject
{
public:
	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

public:
	VisCharSelect* parent_screen;
	CRect field_0x60;
	CRect field_0x70;
	CRect field_0x80;
	CRect field_0x90;
	uint32_t field_0xa0;
	uint32_t field_0xa4;
	uint32_t field_0xa8;
	uint32_t field_0xac;
	uint32_t field_0xb0;
	uint32_t field_0xb4;
	uint32_t field_0xb8;
	uint32_t field_0xbc;
	uint32_t field_0xc0;
	uint32_t field_0xc4;
	uint32_t field_0xc8;
	uint32_t field_0xcc;
	uint32_t field_0xd0;
	uint32_t field_0xd4;
	uint32_t field_0xd8;
};
ASSERT_SIZE(VisCharSellectList, 0xdc);

//609730
class VisCharSellectStats : public CVisualObject
{
public:
	virtual const char* GetHint() override;
	virtual void VMethod7() override;


	void FUN_0042f6f3();

public:
	VisCharSelect* parent_screen;
	uint32_t field_0x60;
	uint32_t field_0x64;
	CStringArray field_0x68;
	uint32_t field_0x7c;
	uint32_t field_0x80;
	uint32_t field_0x84;
	uint32_t field_0x88;
	uint32_t field_0x8c;
	CRect field_0x90[4];
};
ASSERT_SIZE(VisCharSellectStats, 0xd0);


class BigStruct2 : CVisualObject
{
public:

	void FUN_00416cf7();
public:
	uint8_t _unk1[0x974];
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> field_0x9d0;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> field_0x9ec;
	uint8_t _unk2[0x3fc0];
};

ASSERT_OFFSET(BigStruct2, field_0x9d0, 0x9d0);
ASSERT_SIZE(BigStruct2, 0x49c8);











#endif
