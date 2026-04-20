#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"
#include "sound.h"
#include "gameobj.h"


class SomeMainStructure;
class VisLabel;
class CGameBitmap;
class CGameObject;
class CGameFont;
class CBmp64;
class CBmp256;
class BigStruct2;
class VisCharSellectStats;
class VisCharSellectButtons;
class VisCharSellectList;
class Item;
class QuestMap;

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

	int32_t GetId() { return id; } //4503f0
	CRect& GetRect() { return rect; } //41ed40

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

	void AddItem(const char* str);

	void SetSelectedIndex(int32_t idx); //4507a0
	int32_t GetSelectedIndex() { return selected_index; } //450780
	CString& GetItem(int32_t idx); //4508b0

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

//60dfb8
class VisTextBox : public CVisualObject
{
public:
	virtual ~VisTextBox();

	virtual void VMethod7() override;
	virtual void WriteData(void* buf) override;
	virtual uint32_t DataSize() override;
	virtual void ReadData(const void* buf) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;
	virtual int32_t OnChar(uint32_t wparam) override;

	VisTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);
	VisTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint);

	void DelSelection();
	void InsertChar(int32_t chr);
	void ResetBlink();
	int32_t GetCursorPosByX(int32_t x);

public:
	CString text;
	CGameFont* font;
	uint16_t* clr;
	int32_t select_start;
	int32_t select_end;
	int32_t cursor_pos;
	int32_t cursor_blink;
	uint32_t cursor_blink_ts;
};
ASSERT_SIZE(VisTextBox, 0x7c);


//60e030
class VisRadioBase : public CVisualObject
{
public:
	virtual ~VisRadioBase();

	virtual void SetCursorOver(bool isOver) override;
	virtual void WriteData(void* buf) override;
	virtual uint32_t DataSize() override;
	virtual void ReadData(const void* buf) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;

	virtual int32_t GetIndex(int32_t y); //26

	VisRadioBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);

	void AddEntry(const char* etext);
public:
	int32_t mouse_over;
	int32_t field_0x60;
	CStringArray entries;
	CGameFont* font;
	uint16_t* clr;
	int32_t selection;
	int32_t field_0x84;
	int32_t selected;
};
ASSERT_SIZE(VisRadioBase, 0x8c);

//60acf8
class VisRadioType1 : public VisRadioBase
{
public:
	virtual ~VisRadioType1();
	virtual void VMethod7() override;
	virtual void ReadData(const void* buf) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;
	virtual int32_t OnChar(uint32_t wparam) override;

	VisRadioType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);
};

//60ad78
class VisRadioType2 : public VisRadioBase
{
public:
	virtual ~VisRadioType2();
	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;
	virtual int32_t OnChar(uint32_t wparam) override;

	VisRadioType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);
};


//60e248
class VisBitmap : public CVisualObject
{
public:
	virtual ~VisBitmap();
	virtual void VMethod7() override;

	VisBitmap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t dtype);
	VisBitmap(int32_t _id, const RECT& r, CGameBitmap* _bitmap, int32_t dtype);

	CGameBitmap* GetBitmap();
	void SetBitmap(CGameBitmap* bmp);
public:
	CGameBitmap* bitmap;
	int32_t draw_type;
};
ASSERT_SIZE(VisBitmap, 0x64);


//60e4d0
class VisComboBox : public CVisualObject
{
public:
	virtual ~VisComboBox() = default;

	virtual void VMethod8(CRect* rect) override;
	virtual void VMethod9() override;
	virtual void WriteData(void* buf) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisComboBox(int32_t _id, CRect r, const char* hint);

	void AddItem(const char* str);
	void ToggleList();
	void HideList();
	void ProcSelectList();
	void SelectItem(int32_t index);

public:
	VisListBox* listbox;
	VisTextBox* textbox;
	int32_t isEmpty;
	int32_t list_showed;
};
ASSERT_SIZE(VisComboBox, 0x6c);

//60e5d8
class VisComboBoxText : public VisTextBox
{
public:
	virtual ~VisComboBoxText() = default;
	virtual void VMethod7() override;

	VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);
};

//60e548
class VisComboBoxList : public VisListBox
{
public:
	virtual ~VisComboBoxList() = default;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);
};

//60e458
class VisComboBoxButton : public VisButton
{
public:
	virtual ~VisComboBoxButton() = default;

	virtual void SetCursorOver(bool isOver) override;
	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;

	VisComboBoxButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t _frm, int32_t _msgid, int32_t _charid, const char* hint);
public:
	CGameBitmap* bitmap;
	int32_t frm;
};
ASSERT_SIZE(VisComboBoxButton, 0x80);

//60deb0
class VisMultiText : public VisListBox
{
public:
	virtual ~VisMultiText() = default; //4e3c20

	virtual void VMethod7() override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	virtual void SelectItem(int32_t idx) override;

	VisMultiText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy);
	VisMultiText(int32_t _id, const RECT& r, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy);

	void SetText(const char* text);
	void SizesCheck();

public:
	CString text;
};
ASSERT_SIZE(VisMultiText, 0x98);





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


struct TakeDamage
{
	CString txt;
	int32_t dmg = 0;
	uint16_t* color = nullptr;
	int32_t dx = 0;
	int32_t dy = 0;
	uint32_t timestamp = 0;
	int32_t field_0x18 = 1;
	CUnit* cunit = nullptr;
	int32_t field_0x20 = 0;

	TakeDamage(); //45db60
	TakeDamage(int32_t dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct); //45db8a
	TakeDamage(const TakeDamage& ref);
	~TakeDamage(); //45dd08

	void Update(); //45dd1b
	int Draw(); //45dd6d

	static void __cdecl AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg); //45dea1
	static void __cdecl DrawDamages(CArray<TakeDamage>* arr); //45ddf6
	static void __cdecl UpdateDamages(CArray<TakeDamage>* arr); //45de65
};
ASSERT_SIZE(TakeDamage, 0x24);



class GM_a28 : public CObject
{
public:
	virtual ~GM_a28(); //4020ad
	GM_a28(); //401f50

	void Add(const char* msg, uint16_t* clr, uint32_t t); //4021fe
	void AddWithCheck(const char* msg, uint16_t* clr, uint32_t t); //4023aa
	void Clear(); //4021d2
	void Draw(); //40261a
	void SetRect(const CRect& r); //40212b
	void Update(); //402585
public:
	CStringArray text;
	CPtrArray color;
	CDWordArray times;
	uint32_t timestamp;
	uint32_t elapsed_time;
	int32_t max_size;
	CRect out_area;
	int32_t field_0x5c; //unk type
};

class BigStruct2 : public CVisualObject
{
public:

	void FUN_00416cf7();

	void FUN_0041c4a1(const char* name); //41c4a1

	CUnit* GetUnit_3f6c(); //41f830

	int32_t ProcessPackets(uint8_t breakid); //40da14
public:
	int32_t view_tile_x;
	int32_t view_tile_y;
	int32_t field_0x64; //unk type
	int32_t field_0x68; //unk type
	int32_t field_0x6c; //unk type
	int32_t field_0x70; //unk type
	int32_t field_0x74; //unk type
	int32_t field_0x78; //unk type
	int32_t field_0x7c; //unk type
	void* field_0x80;  //unk type

	uint8_t _unk1[0xb4];

	CUnit* field_0x138;
	int32_t field_0x13c; //unk type
	int32_t field_0x140;

	uint8_t _unk1_1[0x874];

	CArray<MapPlayerData*> field_0x9b8;
	MapPlayerData* my_main_unit;
	CMap<uint16_t, uint16_t, CUnit*, CUnit*> field_0x9d0;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> field_0x9ec;
	CMap<uint32_t, uint32_t, uint32_t, uint32_t> field_0xa08; //unk type
	int32_t field_0xa24; //unk type
	GM_a28 msglog;
	int32_t field_0xa88;
	int32_t field_0xa8c;
	int32_t field_0xa90;
	CMap<uint32_t, uint32_t, uint32_t, uint32_t> field_0xa94; //unk type
	int32_t field_0xab0;
	int32_t field_0xab4;
	int32_t field_0xab8;
	int32_t field_0xabc;

	uint8_t _unk2[0x3494];

	CArray<TakeDamage> damage_labels;
	CBmp256* field_0x3f54;
	CUnit* field_0x3f6c;

	uint8_t _unk3[0xA04];

	CArray<Item*> field_0x4974;
	Item* field_0x4988[12];
	int32_t field_0x49b8; //unk type
	int32_t field_0x49bc; //unk type
	int32_t field_0x49c0; //unk type
	int32_t field_0x49c4; //unk type
};

ASSERT_OFFSET(BigStruct2, field_0x80, 0x80);
ASSERT_OFFSET(BigStruct2, field_0x9d0, 0x9d0);
ASSERT_OFFSET(BigStruct2, damage_labels, 0x3f54);
ASSERT_SIZE(BigStruct2, 0x49c8);




//60c9b0
class VisInvBase : public CVisualObject
{
public:
	virtual ~VisInvBase();

	virtual const char* GetHint() override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;

	virtual int32_t VMethod26(GO_d0* o, int32_t idx); //4a554f
	virtual int32_t VMethod27(GO_d0* o); //4a56c3
	virtual GO_d0* VMethod28(uint32_t id); //4a57a1
	virtual GO_d0* VMethod29(GO_d0* o, int32_t num); //4a58dc
	virtual int32_t VMethod30(int32_t x, int32_t y); //4a79a0
	virtual int32_t VMethod31(const CPoint* pt); //4a7990
	virtual void VMethod32(CArray<GO_d0*>* arr); //4a4da9
	virtual void VMethod33(CUnit* uni); //4a4d05
	virtual void VMethod34(); //4a4e28
	virtual void VMethod35(); //4a4c2f
	virtual GO_d0* VMethod36(int32_t idx, int32_t num); //4a5aae
	virtual int32_t VMethod37(int32_t idx); //4a5c39
	virtual int32_t VMethod38(); //4a79b0

public:
	CArray<CSprite256*> field_0x5c;
	CDWordArray field_0x70;
	CArray<GO_d0*>* unit_d0;
	int32_t field_0x88;
	int32_t field_0x8c;
	int32_t* unit_e4;
	int32_t field_0x94[1024];
	int32_t field_0x1094[1024];
	CDWordArray field_0x2094;
	int32_t field_0x20a8;
};
ASSERT_SIZE(VisInvBase, 0x20ac);

//60ca60
class VisInvType1 : public VisInvBase
{
public:
	virtual ~VisInvType1();

	virtual const char* GetHint() override;
	virtual void VMethod7() override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override;

	virtual int32_t VMethod26(GO_d0* o, int32_t idx) override;
	virtual int32_t VMethod27(GO_d0* o) override;
	virtual int32_t VMethod30(int32_t x, int32_t y) override;
	virtual int32_t VMethod38() override;

public:
};
ASSERT_SIZE(VisInvType1, 0x20ac);

//60d0e8
class VisInvExtBase : public VisInvBase
{
public:
	virtual ~VisInvExtBase();

	virtual const char* GetHint() override;
	virtual void VMethod7() override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override;

	virtual int32_t VMethod30(int32_t x, int32_t y) override;
	virtual int32_t VMethod31(const CPoint* pt) override;
	virtual void VMethod32(CArray<GO_d0*>* arr) override;
	virtual int32_t VMethod38() override;

	virtual void VMethod39();
	virtual void VMethod40(CArray<GO_d0*>* arr);
	virtual void VMethod41();
	virtual void VMethod42();

public:
	int32_t field_0x20ac; //unk type
	CSound field_0x20b0;
	CSound field_0x20b4;
	CSound field_0x20b8;
	CSound field_0x20bc;
	int32_t field_0x20c0; //unk type
	CRect* field_0x20c4;
};
ASSERT_SIZE(VisInvExtBase, 0x20c8);

//60d1a8
class VisInvExtType1 : public VisInvExtBase
{
public:
	virtual ~VisInvExtType1();

	virtual const char* GetHint() override;
	virtual void VMethod7() override;

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;

	virtual int32_t VMethod26(GO_d0* o, int32_t idx) override;
	virtual GO_d0* VMethod29(GO_d0* o, int32_t num) override;

	virtual int32_t VMethod37(int32_t idx) override;
	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	virtual void VMethod41() override;
	virtual void VMethod42() override;

public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
	CBmp64* arrow1;
	CBmp64* arrow3;
	CBmp64* arrow2;
	CBmp64* arrow4;
	CBmp64* shop_inv;
	CArray<GO_d0*> field_0x2100[4];
};
ASSERT_SIZE(VisInvExtType1, 0x2150);


//60d268
class VisInvExtType2 : public VisInvExtBase
{
public:
	virtual ~VisInvExtType2();

	virtual const char* GetHint() override;
	virtual void VMethod7() override;

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;

	virtual int32_t VMethod26(GO_d0* o, int32_t idx) override;
	virtual int32_t VMethod27(GO_d0* o) override;

	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;
public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
};
ASSERT_SIZE(VisInvExtType2, 0x20ec);


//60d328
class VisInvExtType3 : public VisInvExtBase
{
public:
	virtual ~VisInvExtType3();

	virtual const char* GetHint() override;
	virtual void VMethod7() override;

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;

	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;

	virtual int32_t VMethod26(GO_d0* o, int32_t idx) override;
	virtual int32_t VMethod27(GO_d0* o) override;

	virtual GO_d0* VMethod36(int32_t idx, int32_t num) override;
	virtual int32_t VMethod37(int32_t idx) override;
	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	virtual void VMethod41() override;
	virtual void VMethod42() override;

	virtual GO_d0* VMethod43(int32_t id1, int32_t id2);
public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
	CBmp64* shoptable;
	int32_t field_0x20f0; //unk type
	CArray<GO_d0*> field_0x20f4;
};
ASSERT_SIZE(VisInvExtType3, 0x2108);


//60c6d8
class VisTav : public VisScreen
{
public:
	virtual ~VisTav();

	virtual void VMethod7() override;
	virtual void VMethod8(CRect* rect) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	virtual void VMethod26() override;
	virtual void VMethod28() override;
	virtual void DoClose(uint32_t code) override;

	virtual void VMethod30();
	virtual void VMethod31();

public:
	BigStruct2* p_bigstru2;
	CVisualObject* field_0x6c;
	CVisualObject* tav_44d;
	CVisualObject* tav_44e;
	int32_t field_0x78; //unk type
	CVisualObject* tav_450;
	int32_t field_0x80; //unk type
	CSound sounds[13];
	int32_t field_0xb8; //unk type
	int32_t field_0xbc; //unk type
	CArray <CUnit*> field_0xc0;
	CArray <CUnit*> field_0xd4;
	CArray <CUnit*> field_0xe8;
	CDWordArray field_0xfc;
	int32_t field_0x110; //unk type
	QuestMap* quest_map;
	int32_t quest_id;
	int32_t field_0x11c; //unk type
	int32_t field_0x120; //unk type
	CArray<GO_d0*> field_0x124;
	int32_t field_0x138; //unk type
	int32_t field_0x13c; //unk type
};
ASSERT_SIZE(VisTav, 0x140);

#endif
