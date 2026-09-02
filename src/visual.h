#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include <array>
#include "assert_offset.h"
#include "asm_mfc.h"
#include "mfc_templ.h"
#include "sound.h"
#include "gameobj.h"
#include "txtfile.h"


class CGameSession;
class VisLabel;
class CGameBitmap;
class CGameObject;
class CGameFont;
class CBmp64;
class CBmp256;
class CA16;
class BigStruct2;
class VisCharSellectStats;
class VisCharSellectButtons;
class VisCharSellectList;
class Item;
class QuestMap;
class Scenario;

class CUnit;

class CFameHall;

struct SaveFileInfo;



class CVisualObject : public CObject
{
public:
	enum
	{
		MSG_409 = 0x409,
		MSG_40a = 0x40a,
		MSG_410 = 0x410,
	};

	enum
	{
		FLAG_ENABLED = 1,
		FLAG_NOTFOCUS = 2,
		FLAG_FOCUS = 4,
		FLAG_OVERCURSOR = 8,
		FLAG_10 = 0x10,
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
	void SetRect(int32_t l, int32_t t, int32_t r, int32_t b);

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
	void RemoveItem(int32_t idx); //450830

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
	virtual ~VisMessageBoxWithList() {}; //44f930
	virtual CVisualObject* VMethod30(const char* str, const RECT& r); //4450d4

	VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes); //445091
};
ASSERT_SIZE(VisMessageBoxWithList, 0x78);

//60a3a8
class QuestObjectivesHeaderDialogVisualObject : public VisMessageBoxWithList
{
public:
	QuestObjectivesHeaderDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b);  //445173

	virtual void VMethod26() override; //4451e1
};
ASSERT_SIZE(QuestObjectivesHeaderDialogVisualObject, 0x78);


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

	VisCharSelect(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //431eac in asm

public:
	CGameSession* session;
	CVisualObject* info_panel;
	BigStruct2* map_context;
	VisCharSellectStats* vis_stats;
	VisCharSellectButtons* buttons;
	VisCharSellectList* roster_list;
	CVisualObject* rename_txt;
	CUnit* selected_unit;
	SfxSample* snd_ok;
	SfxSample* snd_rename;
	SfxSample* snd_delete;
	SfxSample* snd_cancel;
	uint32_t active_flag;
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
	BigStruct2(int32_t l, int32_t t, int32_t r, int32_t b); //402af6 in asm

	void UpdateSelectionState(); //416cf7
	void UpdateSpellEffects(CUnit* unit); //from 416cf7
	void UpdateSpellModifiers(CUnit* unit); //from 416cf7

	void FUN_0041c4a1(const char* name); //41c4a1

	CUnit* GetUnit_3f6c(); //41f830
	void SetUnit_3f6c(CUnit* uni) { field_0x3f6c = uni; };

	int32_t ProcessPackets(uint8_t breakid); //40da14

	void FUN_0041b7b7(int32_t xpos, int32_t ypos, int32_t* pvol, int32_t* ppan); //41b7b7
	void FUN_0041c74b(int32_t x, int32_t y, int32_t w, int32_t h, int32_t unk1, uint8_t* dat, int32_t unk2); //41c74b

	CUnit* FUN_0041df23(int32_t serv_id); //41df23

	void FUN_0041c630(CWordArray *data); //41c630

	void FUN_0041cb67(int32_t val); //41cb67
	void FUN_0041cb21(); //41cb21

	void FUN_0041b2a4(const char* nam, int32_t token, int32_t playerid); //41b2a4
	void FUN_0041a8cc(); //41a8cc

	int32_t FUN_0040d7f3(); //40d7f3
	int ConnectAndJoinSession(); //40d4e2

	void FUN_0041cbb8(); //0041cbb8
	void FUN_0041cc78(const CString& str); //41cc78
	void FUN_0041d2da(int32_t arg); //41d2da
	int32_t IsBookOpen(); //41b4b0
	int32_t IsBagOpen(); //41b495

	void FUN_0041afcf(CString str); //41afcf
	void FUN_0041a735(); //41a735

	void FUN_0041cad0(int32_t m); //41cad0
	void FUN_0041aaaa(int32_t wim); //41aaaa
	void FUN_0041abd2(int32_t form); // 41abd2
	void FUN_0041ab74(); //41ab74

	void FUN_0041b6e0(); //41b6e0
	int32_t FUN_0041f6b0() { return (field_0x88 - 8) - field_0x68; } //41f6b0

	void FUN_0041b636(); //41b636

	int FUN_0041b495() { return FindChild(2) != nullptr; } //41b495
	void FUN_0041b509(); //41b509
	void FUN_0041b40e(); //41b40e
	void FUN_0041b381(); //41b381
	void FUN_0041b10f(); //41b10f
	void FUN_0041b064(int32_t arg1, int32_t arg2); //41b064
	void FUN_0041c39c(); //41c39c

	void CleanupCompletedMissionMapState(); //41ce5a

	void FUN_00403ca0(CGameObject* obj); //403ca0 in asm
	void FUN_0041d97e(int32_t); //41d97e in asm

	uint32_t AvailableOrderMask() const; // 417af6
	void SendAdjustPlayerGoldAction(int32_t goldDelta) const; //41b0be

	int32_t MapMinX() const { return 8; } //41f670
	int32_t MapMinY() const { return 8; } //41f680

	int32_t MapMaxX() const { return field_0x88 - 8 - field_0x68; } //41f6b0

	void ScrollMapX(int32_t dx); //403f4f
	void ScrollMapY(int32_t dy); //403fc5

public:
	int32_t view_x;
	int32_t view_y;
	int32_t field_0x64; //unk type
	int32_t field_0x68; //unk type
	int32_t field_0x6c; //unk type
	int32_t field_0x70; //unk type
	int32_t field_0x74; //unk type
	int32_t field_0x78; //unk type
	int32_t field_0x7c; //unk type
	Scenario* field_0x80;
	int32_t field_0x84; //unk type
	int32_t field_0x88; //unk type
	uint32_t* field_0x8c; //unk type
	uint32_t* field_0x90; //unk type
	uint32_t* field_0x94; //unk type
	uint32_t* field_0x98; //unk type
	uint32_t* field_0x9c; //unk type
	uint32_t* field_0xa0; //unk type
	uint32_t* field_0xa4; //unk type
	uint8_t* field_0xa8; //unk type
	uint8_t* field_0xac; //unk type
	uint8_t* field_0xb0; //unk type
	uint32_t* field_0xb4; //unk type
	uint32_t* field_0xb8; //unk type
	uint32_t* field_0xbc; //unk type
	uint32_t* field_0xc0; //unk type
	int32_t field_0xc4; //unk type
	int32_t field_0xc8; //unk type
	int32_t field_0xcc; //unk type
	int32_t field_0xd0;
	int32_t field_0xd4;
	int32_t field_0xd8;
	int32_t field_0xdc;
	int32_t field_0xe0;
	int32_t field_0xe4;//unk type
	int32_t field_0xe8;//unk type
	int32_t field_0xec;//unk type
	int32_t field_0xf0;//unk type
	CRect field_0xf4;
	int32_t field_0x104;//unk type
	int32_t field_0x108;//unk type
	CArray<CRect> field_0x10c;
	CWordArray field_0x120;
	int32_t field_0x134;//unk type
	CGameObject* field_0x138; //primarySelectedObject
	POSITION field_0x13c; //primarySelectedAssoc
	int32_t field_0x140; //selectedObjectCount
	uint32_t field_0x144;
	uint32_t field_0x148; //selectedAvailableSpellMask
	int32_t field_0x14c; //unk type
	uint32_t field_0x150; //activeSpellEffectMask
	int32_t spell_damage_min[24];
	int32_t spell_damage_max[24];
	int32_t spell_range_min[24];
	int32_t spell_range_max[24];
	int32_t spell_mana_cost1[24];
	int32_t spell_mana_cost2[24];
	int32_t spell_power_min[24];
	int32_t spell_power_max[24];
	int32_t spell_val1_min[24];
	int32_t spell_val1_max[24];
	int32_t spell_val2_min[24];
	int32_t spell_val2_max[24];
	int32_t spell_val3_min[24];
	int32_t spell_val3_max[24];
	int32_t spell_val4_min[24];
	int32_t spell_val4_max[24];
	int32_t spell_val5_min[24];
	int32_t spell_val5_max[24];
	int32_t spell_val6_min[24];
	int32_t spell_val6_max[24];
	int32_t spell_val7_min[24];
	int32_t spell_val7_max[24];
	CUnit* field_0x994;
	uint32_t quest_some_id_2;
	uint32_t quest_landmark_some_id;
	uint32_t quest_building_some_id;
	uint32_t quest_some_id;
	uint32_t field_0x9a8;
	int32_t field_0x9ac;
	int32_t field_0x9b0;
	int32_t field_0x9b4;
	CArray<MapPlayerData*> field_0x9b8;
	MapPlayerData* my_main_unit;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> field_0x9d0;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> field_0x9ec;
	CMap<uint16_t, uint16_t, uint32_t, uint32_t> field_0xa08; //terrainLightOverrideCells
	int32_t field_0xa24; //unk type
	GM_a28 msglog;
	int32_t field_0xa88;
	int32_t field_0xa8c;
	int32_t field_0xa90;
	CMap<uint16_t, uint16_t, uint32_t, uint32_t> field_0xa94;
	int32_t formation;
	int32_t wimpy;
	int32_t show_hp;
	int32_t flying_hp;
	
	uint8_t field_0xac0[41][41][2];
	uint8_t ____padding1[2];

	uint32_t field_0x17e4[41][41];
	uint16_t field_3228[41][41];

	uint16_t ____padding2;

	int32_t field_0x3f4c; //unk type
	int32_t field_0x3f50; //unk type
	CArray<TakeDamage> damage_labels;
	CBmp256* field_0x3f68;
	CUnit* field_0x3f6c;
	std::array<uint8_t, 0xa00> kill_stats;
	QuestMap* field_0x4970;
	CArray<Item*> field_0x4974;
	std::array<Item*, 12> field_0x4988;
	int32_t field_0x49b8; //unk type
	int32_t field_0x49bc; //unk type
	int32_t field_0x49c0; //unk type
	int32_t field_0x49c4; //unk type
};

ASSERT_OFFSET(BigStruct2, field_0x80, 0x80);
ASSERT_OFFSET(BigStruct2, field_0x9d0, 0x9d0);
ASSERT_OFFSET(BigStruct2, damage_labels, 0x3f54);
ASSERT_SIZE(BigStruct2, 0x49c8);

//60ce80
class VisRightPanel : public CVisualObject
{
public:
	VisRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aedcf in asm
};
ASSERT_SIZE(VisRightPanel, 0x5c);


//60cef8
class VisMiniMap : public CVisualObject
{
public:
	VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aeec3 in asm

public:
	BigStruct2* map_context;
	CBmp64* bitmap1;
	CBmp64* bitmap2;
	int32_t zoom;
	uint32_t viewer;
};
ASSERT_SIZE(VisMiniMap, 0x70);


//60cf70
class VisOrderToolbar : public CVisualObject
{
public:
	VisOrderToolbar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b0cfc in asm
public:
	BigStruct2* map_context;
	uint32_t selected_order;
	uint32_t avail_orders_mask;
	uint32_t enabled;
	uint32_t dirty;
};
ASSERT_SIZE(VisOrderToolbar, 0x70);






//60c9b0
class VisInvBase : public CVisualObject
{
public:
	virtual ~VisInvBase();

	virtual const char* GetHint() override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx); //4a554f
	virtual int32_t VMethod27(TokenEntry* o); //4a56c3
	virtual TokenEntry* VMethod28(uint32_t id); //4a57a1
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num); //4a58dc
	virtual int32_t VMethod30(int32_t x, int32_t y); //4a79a0
	virtual int32_t VMethod31(const CPoint* pt); //4a7990
	virtual void VMethod32(CArray<TokenEntry*>* arr); //4a4da9
	virtual void VMethod33(CGameObject* uni); //4a4d05
	virtual void VMethod34(); //4a4e28
	virtual void VMethod35(); //4a4c2f
	virtual TokenEntry* VMethod36(int32_t idx, int32_t num); //4a5aae
	virtual int32_t VMethod37(int32_t idx); //4a5c39
	virtual int32_t VMethod38(); //4a79b0

public:
	CArray<CSprite256*> spr_cells;
	CDWordArray cell_update_counter;
	CArray<TokenEntry*>* grid_source;
	int32_t visible_columns;
	int32_t visible_rows;
	int32_t* visible_startref;
	int32_t random_offsets1[1024];
	int32_t random_offsets2[1024];
	CDWordArray anim_frames;
	int32_t has_anim_visible_cells;
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

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override;
	virtual int32_t VMethod27(TokenEntry* o) override;
	virtual int32_t VMethod30(int32_t x, int32_t y) override;
	virtual int32_t VMethod38() override;

	VisInvType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a630b in asm

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
	virtual void VMethod32(CArray<TokenEntry*>* arr) override;
	virtual int32_t VMethod38() override;

	virtual void VMethod39();
	virtual void VMethod40(CArray<TokenEntry*>* arr);
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

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override;
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num) override;

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
	CArray<TokenEntry*> field_0x2100[4];
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

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override;
	virtual int32_t VMethod27(TokenEntry* o) override;

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

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override;
	virtual int32_t VMethod27(TokenEntry* o) override;

	virtual TokenEntry* VMethod36(int32_t idx, int32_t num) override;
	virtual int32_t VMethod37(int32_t idx) override;
	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	virtual void VMethod41() override;
	virtual void VMethod42() override;

	virtual TokenEntry* VMethod43(int32_t id1, int32_t id2);
public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
	CBmp64* shoptable;
	int32_t field_0x20f0; //unk type
	CArray<TokenEntry*> field_0x20f4;
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

	void FUN_0049edec();

	VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //49db79 in asm

public:
	BigStruct2* map_context;
	CVisualObject* info_panel;
	CVisualObject* left_panel;
	CVisualObject* right_panel;
	int32_t field_0x78; //unk type
	CVisualObject* scene;
	int32_t tips;
	CSound sounds[13];
	int32_t selection_index;
	int32_t select_party;
	CArray <CUnit*> avail_entries;
	CArray <CUnit*> selected_entries;
	CArray <CUnit*> reserved_entries;
	CDWordArray entrie_id;
	int32_t dialog_active;
	QuestMap* quest_map;
	int32_t quest_id;
	int32_t quest_selection;
	int32_t quest_mirror_selection;
	CArray<TokenEntry*> rewards;
	int32_t field_0x138;
	int32_t field_0x13c;
};
ASSERT_SIZE(VisTav, 0x140);

//60c768
class VisTavDruid : public VisTav
{
public:
	VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49f4e6 in asm
public:
	SfxSample* snd_druid[11];
};
ASSERT_SIZE(VisTavDruid, 0x16c);

//60c880
class VisTavKaarg : public VisTav
{
public:
	VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4a18d8 in asm
public:
	SfxSample* snd_kaarg[10];
};
ASSERT_SIZE(VisTavKaarg, 0x168);


class VisSpellBook : public CVisualObject
{
public:
	virtual ~VisSpellBook();

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
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	void FUN_004caa69();

	VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4c99c7 in asm

public:
	int32_t field_0x5c;
	int32_t pressed;
	int32_t selected;
	int32_t spell;
};
ASSERT_SIZE(VisSpellBook, 0x6c);


//60d3e8
class VisShop : public VisScreen
{
public:
	virtual ~VisShop();

	virtual void VMethod8(CRect* rect) override;
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	virtual void VMethod26() override;
	virtual void VMethod28() override;
	virtual void DoClose(uint32_t code) override;

	virtual void VMethod30();
	virtual void VMethod31();
	virtual void VMethod32();


	void FUN_004bcd02();

	VisShop(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4ba342 in asm

public:
	VisInvExtType1* assortiment;
	VisInvExtType2* to_sell;
	VisInvExtType3* to_buy;
	CVisualObject* shop_compass;
	CVisualObject* buttons;
	CVisualObject* select_info_panel;
	BigStruct2* gameplay;
	VisSpellBook* spell_panel;
	CVisualObject* tips;
	int32_t tips_update_flag;
	SfxSample* snd_notif;
	SfxSample* snd_step1;
	SfxSample* snd_step2;
	SfxSample* snd_breath;
	SfxSample* snd_depart;
	SfxSample* snd_buy;
	SfxSample* snd_sell;
	SfxSample* snd_enter;
	SfxSample* snd_start;
	SfxSample* snd_pov1;
	SfxSample* snd_pov2;
	SfxSample* snd_inshop;
	SfxSample* snd_out;
	SfxSample* snd_undo;
	CBmp64* bmp_backinvg;
	CBmp64* bmp_backinvb;
	CBmp64* bmp_backinvs;
	CSprite256* spr_myitem;
	CSprite256* spr_shopitem;
	CArray<CBmp64*> bmp_cost_medium;
	CArray<CBmp64*> bmp_cost_small;
	int32_t hovered_region;
	CArray<CUnit*> selected_units;
	CWordArray units_order;
	int16_t select_index;
	int16_t select_category;
	int32_t field_0x134;
	int32_t field_0x138;
	int32_t field_0x13c;
	int32_t field_0x140;
	int32_t placement_lock;
	int32_t dialog_active;
	int32_t dirty;
	int32_t current_gold;
	int32_t buy_gold;
	int32_t sell_gold;
	int32_t result_gold;
	int32_t scenario_talk_target;
};
ASSERT_SIZE(VisShop, 0x164);


//60d5c0
class VisShopDruid : public VisShop
{
public:
	VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c15ad in asm
public:
	SfxSample* snd_bird[3];
	SfxSample* snd_tool[4];
	int32_t bird_tick;
	int32_t tool_tick;
	int32_t next_bird;
	int32_t next_tool;
};
ASSERT_SIZE(VisShopDruid, 0x190);


//60d708
class VisShopKaarg : public VisShop
{
public:
	VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c37c5 in asm
public:
	SfxSample* snd_voice[3];
	SfxSample* snd_tool[4];
	int32_t voice_tick;
	int32_t tool_tick;
	int32_t next_voice;
	int32_t next_tool;
};
ASSERT_SIZE(VisShopKaarg, 0x190);




//60dbb0
class VisTown : public VisScreen
{
public:
	VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cda5c in asm
public:
	CBmp64* bmp_bkg;
	CBmp256* bmp_hover_mask;
	CBmp64* bmp_bird_overlay;
	SfxSample* sounds[14];
	int32_t hover_snd_shop;
	int32_t hover_snd_second;
	int32_t hovered_action_mask;
	int32_t last_bird;
	int32_t bird_group_index;
	int32_t active_bird;
	CArray<CSprite256*> spr_birds;
	int32_t bird_frame0;
	int32_t bird_frame1;
	int32_t bird_frame2;
	CA16* spr_guard;
	int32_t guard_frame;
	uint32_t guard_frame_step;
	int32_t guard_sound;
	CA16* spr_cur_bbird;
	CArray<CSprite256*> spr_bbird;
	CPoint bbird_position;
	int32_t bbird_frame;
	uint32_t bbird_last_tick;
	int32_t bbird_delay;
	CA16* spr_cur_horse;
	CArray<CSprite256*> spr_horse;
	CPoint horse_position;
	int32_t horse_frame;
	int32_t horse_anim_index;
	uint32_t horse_last_tick;
	int32_t horse_delay;
	CA16* spr_dervish;
	CPoint dervish_position;
	int32_t dervish_frame;
	CA16* spr_tavern;
	CBmp64* bmp_tavern_hover;
	int32_t tavern_frame;
	CArray<CBmp64*> bmp_sign;
	CBmp64* bmp_cur_sign;
	int32_t sign_frame;
	CArray<CBmp64*> bmp_door;
	CBmp64* bmp_cur_door;
	int32_t door_frame;
	uint32_t door_open_flag;
	CArray<CBmp64*> bmp_stars;
	CBmp64* bmp_cur_stars;
	int32_t stars_frame;
	CBmp64* bmp_trainer_hover;
	CA16* spr_fighter;
	int32_t fighter_frame;
	CA16* spr_mage;
	int32_t mage_frame;
	CA16* spr_shop;
	CBmp64* bmp_shop_hover;
	int32_t shop_frame;
	CArray<CBmp64*> bmp_flugel;
	CBmp64* bmp_cur_flugel;
	int32_t flugel_frame;
	CVisualObject* tips;
	uint32_t dialog_active;
	uint32_t town_anim;
};
ASSERT_SIZE(VisTown, 0x20c);


//60dc60
class VisTownDruid : public VisTown
{
public:
	VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d1af8 in asm
public:
	SfxSample* snd_lizard[4];
	SfxSample* snd_bug[3];
	SfxSample* snd_bird[3];
	SfxSample* snd_tree[4];
	SfxSample* snd_forest;
	SfxSample* snd_shop;
	SfxSample* snd_tavern;
	SfxSample* snd_shop_enter;
	SfxSample* snd_tavern_enter;
	SfxSample* snd_town_exit;
	SfxSample* snd_wolf;
	uint32_t tavern_hover_snd_flag;
	uint32_t mission_exit_hover_snd_flag;
	CArray<CBmp64*> tavern_frame_group[3];
	int32_t tavern_group;
	uint32_t tavern_tick;
	int32_t tavern_delay;
	CArray<CBmp64*> shop_frame_group[3];
	int32_t shop_group;
	uint32_t shop_tick;
	int32_t shop_delay;
	uint32_t tree_tick;
	uint32_t wolf_tick;
	CA16* spr_bug;
	uint32_t bug_tick;
	int32_t bug_delay;
	int32_t bug_variant;
	int32_t bug_frame;
	CA16* spr_lizard;
	int32_t lizard_variant;
	int32_t lizard_frame;
};
ASSERT_SIZE(VisTownDruid, 0x320);


//60dd10
class VisTownKaarg : public VisTown
{
public:
	VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d423a in asm
public:
	SfxSample* snd_voice[3];
	SfxSample* snd_bird[4];
	SfxSample* snd_voice1;
	SfxSample* snd_shop_enter;
	SfxSample* snd_tavern_enter;
	SfxSample* snd_dervish;
	SfxSample* snd_guard[6];
	uint32_t tavern_snd_flag;
	uint32_t exit_snd_flag;
	uint32_t guard_snd_flag;
	uint32_t bird_snd_tick;
	uint32_t dervish_snd_tick;
	CArray<CBmp64*> bmp_guard;
	uint32_t guard_tick;
	int32_t guard_delay;
	CArray<CBmp64*> bmp_dervish;
	uint32_t dervish_tick;
	int32_t dervish_delay;
	CArray<CBmp64*> bmp_girl1[2];
	int32_t girl1_group;
	CArray<CBmp64*> bmp_girl2[2];
	int32_t girl2_group;
	int32_t girl2_frame;
	uint32_t girl2_tick;
	int32_t girl2_delay;
	CArray<CBmp64*> bmp_gate;
};
ASSERT_SIZE(VisTownKaarg, 0x314);



class VisCredits : public VisScreen
{
public:
	VisCredits(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43c337 in asm
public:
	CMapStringToOb bitmaps;
	TxtFile text;
	uint32_t flag;
	int32_t first_visible;
	int32_t scroll;
};
ASSERT_SIZE(VisCredits, 0xa0);

//6096a8
class VisCharGen : public VisScreen
{
public:
	VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //42dcd9 in asm
public:
	CVisualObject* info_panel;
	CVisualObject* map_visual;
	CVisualObject* stats_panel;
	CVisualObject* fullstats_panel;
	CVisualObject* action_panel;
	CVisualObject* skills_panel;
	CVisualObject* tips;
	SfxSample* snd_stat;
	SfxSample* snd_ok;
	SfxSample* snd_reset;
	SfxSample* snd_back;
	CUnit* current_char;
	int32_t fwd_btn;
	int32_t field14_0x9c;
	CWordArray face_sets[4];
	uint32_t mage_flag;
	int32_t female_face;
	int32_t selected_face;
	int32_t selected_face_set;
	int32_t tips_step;
	uint32_t active_flag;
};
ASSERT_SIZE(VisCharGen, 0x108);




//609ad0
class Vis1200 : public VisScreen
{
public:

	int32_t FUN_004972d0(); //4972d0
	int32_t FUN_00497310(); //497310
	void FUN_0043be9f(); //43be9f

	Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 43bb70 in asm

public:
	char buffer[256];
	CVisualObject* text_block;
	uint32_t active;
};
ASSERT_SIZE(Vis1200, 0x170);



//60a170
class VisDropGold : public VisScreen
{
public:
	VisDropGold(int32_t _id, int32_t _x, int32_t _y); // 60a170 in asm
public:
	int32_t selection;
	char amount_text[100];
};
ASSERT_SIZE(VisDropGold, 0xd0);




//60cfe8
class VisCharInfo : public CVisualObject
{
public:
	VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b16c3 in asm
public:
	BigStruct2* map_context;
	int32_t dirty;
	int32_t field_64;
	int32_t selection_panel_state;
	int32_t spell_panel_state;
	int32_t info_mode;
	CBmp64* bitmap;
	CBmp256* hitmap;
	char picturename[256];
};
ASSERT_SIZE(VisCharInfo, 0x17c);


//60d070
class VisSideStatus : public CVisualObject
{
public:
	VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b3fa3 in asm
public:
	uint32_t dirty;
};
ASSERT_SIZE(VisSideStatus, 0x60);


//60cd30
class VisMainMenu : public VisScreen
{
public:
	VisMainMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aa926 in asm
public:
	CArray<CBmp64*> bmp_button;
	CArray<CBmp64*> bmp_pressed;
	CArray<CBmp64*> bmp_labels;
	CArray<CRect> rect_buttons;
	CArray<CRect> rect_labels;
	CBmp64* bmp_menu;
	CBmp256* bmp_menu_mask;
	SfxSample* snd_btn_click;
	CBmp64* bmp_active_button;
	CRect rect_active_button;
	uint32_t over_button;
	uint32_t active_button;
	uint32_t pressed_button;
	uint32_t disable_mask;
	CA16* sprite;
};
ASSERT_SIZE(VisMainMenu, 0x100);




class VisStartGame;

class VisStartGameTextBox : public CVisualObject
{
public:

public:
	VisStartGame* screen;
	CString text;
	uint16_t* clr;
	uint16_t* clr_active;
	uint16_t* clr_inactive;
	int32_t is_active;
	uint32_t ts;
};
ASSERT_SIZE(VisStartGameTextBox, 0x78);


//609998
class VisStartGame : public VisScreen
{
public:
	VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43305b in asm

	CString GetName() { return char_name; } // 4973f0
	CString GetClan() { return clan_name; } // 497430
	int32_t GetSelectedDifficulty() { return selectedDifficultyIndex; } //4973d0
	void SetSelectedDifficulty(int diff) { selectedDifficultyIndex = diff; } //497370
	int32_t GetSelectedPortraitPlayerType() { return DWORD_0060bd60[selectedPortraitIndex]; } //4973b0
	void SetSelectedPortraitPlayerType(int id) { selectedPortraitIndex = id; } //497350
	void SetCharName(const CString& name) { char_name = name; } //497390

public:
	CBmp64* mainAreaBitmap; //0x68
	CBmp256* hotspotMaskBitmap; // 0x6c;
	CBmp64* blindAnimation; // 0x70;
	int32_t blindAnimationFrame; // 0x74;
	CPoint blindAnimationPosition; // 0x78;
	CArray<CRect> blindSpawnRects; // 0x80;
	CArray<CBmp64*> portraitSelectedBitmaps; // 0x94;
	CArray<CBmp64*> portraitHoverBitmaps; // 0xa8;
	CArray<CBmp64*> portraitSelectedHoverBitmaps; // 0xbc;
	CArray<CBmp64*> difficultySelectedBitmaps; // 0xd0;
	CArray<CBmp64*> difficultyHoverBitmaps; // 0xe4;
	CArray<CBmp64*> difficultySelectedHoverBitmaps; // 0xf8;
	CArray<CRect> portraitRects; // 0x10c;
	CArray<CRect> difficultyRects; // 0x120;
	CUIntArray portraitStateFlags; // 0x134;
	CUIntArray difficultyStateFlags; // 0x148;
	CBmp64* returnToGameButtonBitmap; // 0x15c;
	CBmp64* acceptButtonBitmap; // 0x160;
	CBmp64* tableauBitmap; // 0x164;
	CArray<CBmp64*> leftTorchFrames; // 0x168;
	CArray<CBmp64*> rightTorchFrames; // 0x17c;
	CRect returnToGameButtonRect; // 0x190;
	CRect acceptButtonRect; // 0x1a0;
	int32_t torchFrameTick; // 0x1b0;
	CBmp64* returnToGameHoverBitmap; // 0x1b4;
	CBmp64* acceptHoverBitmap; // 0x1b8;
	int32_t field_0x1bc;
	int32_t field_0x1c0;
	CSound difficultyLevel1Sound; // 0x1c4;
	CSound difficultyLevel2Sound; // 0x1c8;
	CSound difficultyLevel3Sound; // 0x1cc;
	CSound portraitSelectSound; // 0x1d0;
	CSound acceptSound; // 0x1d4;
	CSound returnSound; // 0x1d8;
	CSound labelInputSound1; // 0x1dc;
	CSound labelInputSound2; // 0x1e0;
	CSound labelInputSound3; // 0x1e4;
	int32_t labelInputSoundIndex;
	VisStartGameTextBox* networkNameLabel; // 0x1ec;
	VisStartGameTextBox* nameLabel; // 0x1f0;
	VisStartGameTextBox* clanLabel; // 0x1f4;
	VisScreen* tipsPrompt; // 0x1f8;
	CString char_name; // 0x1fc
	CString clan_name; // 0x200
	int32_t field_0x204;
	int32_t selectedPortraitIndex; // 0x208;
	int32_t selectedDifficultyIndex; // 0x20c;
	int32_t committedDifficultyIndex; // 0x210;
	int32_t committedPortraitIndex; // 0x214;
	int32_t tipsProgress; // 0x218;
	int32_t dialogActiveFlag; // 0x21c;

	static const int32_t DWORD_0060bd60[4];
};
ASSERT_SIZE(VisStartGame, 0x220);

//60b180
class VisMenuWnd : public VisWindow
{
public:
	virtual ~VisMenuWnd();

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440e54
	virtual int32_t OnKeyDown(uint32_t wparam) override; //440ece

	VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect &_r); //451990

	void AddElement(CVisualObject *obj, int32_t height); //440e11
public:
	uint32_t field_0x68;
	CRect field_0x6c;
};
ASSERT_SIZE(VisMenuWnd, 0x7c);


//609f50
class IngameMenu : public VisMenuWnd
{
public:
	IngameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r); //451990
};
ASSERT_SIZE(IngameMenu, 0x7c);


//60ae78
class MenuButton : public VisButton
{
public:
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; //4dd933
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } //4dda31
	virtual int32_t OnChar(uint32_t wparam); //4dd9b3

	MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //4509c0
	MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //450bf0
};
ASSERT_SIZE(MenuButton, 0x78);


//609dc8
class LoadGameWindow : public VisWindow
{
public:
	virtual ~LoadGameWindow() {} //44f4b0
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43f63c
	virtual void VMethod26() override; //43ed94

	LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43ecfb
public:
	SaveFileInfo* field_0x68;
	CStringArray file_names;
	CStringArray save_times;
	int32_t field_0x94;
};
ASSERT_SIZE(LoadGameWindow, 0x98);

//609e50
class SaveGameWindow : public VisWindow
{
public:
	virtual ~SaveGameWindow() {} //44f540
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440726
	virtual void VMethod26() override; //43fb2c

	SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43fa42
	void CheckInput(); //440cc4

	static int __cdecl Compare(void const* a, void const* b); //43faf4
public:
	SaveFileInfo* field_0x68;
	CStringArray file_names;
	CStringArray save_times;
	int32_t field_0x94;
	char* field_0x98;
};
ASSERT_SIZE(SaveGameWindow, 0x9c);


//609d38
class HintedListBox : public VisListBox
{
public:
	virtual ~HintedListBox() {} //44f410

	virtual const char* GetHint() override; //43ec67

	HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); //43ec05

	void SetHints(CStringArray* names) { hints = names; } //43ec4e
public:
	CStringArray* hints = nullptr;
};
ASSERT_SIZE(HintedListBox, 0x98);


//60a2000
class GameOptionsWindow : public VisWindow
{
public:
	virtual ~GameOptionsWindow() {} //443cbb

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //443cd7
	virtual void VMethod26() override; //442646

	GameOptionsWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b) : VisWindow(_id, l, t, r, b, nullptr) {}; //44260f
};
ASSERT_SIZE(GameOptionsWindow, 0x68);


class EndGameMenu : public VisMenuWnd
{
public:
	EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2); //441357
};
ASSERT_SIZE(EndGameMenu, 0x7c);

class ExitGameMenu : public VisMenuWnd
{
public:
	ExitGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44186e
};
ASSERT_SIZE(ExitGameMenu, 0x7c);


class TownMenuListDialogVisualObject : public VisMenuWnd
{
public:
	TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44160e
};
ASSERT_SIZE(TownMenuListDialogVisualObject, 0x7c);


//60aef0
class VisServerScreen : public VisScreen
{
public:
	VisServerScreen(int32_t _id, int32_t l, int32_t, int32_t r, int32_t b, GM_a28 *log); // 44e469

public:
	char dummy[16];
};
ASSERT_SIZE(VisServerScreen, 0x78);


class GlobalMapRouteArray : public CWordArray
{
public:
public:
	int32_t routeCost = 0;
};
ASSERT_SIZE(GlobalMapRouteArray, 0x18);

//60b760
class VisGlobalMap : public VisScreen
{
public:
	void SetTravelOrigin(const CPoint& pt) { currentLocationPoint = pt; } //4972a0

	void RebuildScenarioLocations(); //47024a
	void PopulateScenarioLocationFlags(); //47025d

	VisGlobalMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 46fd7b in asm
public:
	CBmp64* gmap;
	CBmp64* hero_bmp;
	CBmp64* ballmap;
	CA16* flag1_spr;
	CA16* flag_spr;
	CA16* cross_spr;
	CA16* mission_flg;
	CA16* flg_on_map;
	CA16* yflag_spr;
	CBmp64* scroll1_bmp;
	CBmp64* scroll2_bmp;
	CBmp64* scroll3_bmp;
	CBmp64* scrollp1_bmp;
	CBmp64* scrollp3_bmp;
	CBmp64* scrollp2_bmp;
	CWordArray locationAvailabilityFlags;
	CArray<CRect> locationHitRects;
	CArray<CPoint> locationPoints;
	CArray<CPoint> travelRoutePoints;
	CSize heroBitmapSize;
	CPoint heroDrawPoint;
	CPoint currentLocationPoint;
	CPoint targetLocationPoint;
	CRect partDetailsRect;
	int32_t hoveredLocationIndex;
	int32_t mapFlagAnimationFrame;
	int32_t travelProgress;
	int32_t partyFlagAnimationFrame;
	int32_t targetCrossAnimationFrame;
	uint32_t bestRouteCost;
	GlobalMapRouteArray routeNodeIndices;
	void* routeAdjacencyMatrix;
	CArray<CPoint> graphNodePoints;
	CArray<DWORD> locationMetadata;
	CSound snd_scrollup;
	CSound snd_scrolldn;
	CSound snd_point1;
	CSound snd_point2;
	int32_t routePointSoundIndex;
	int32_t renderActiveFlag;
	CString hoveredLocationTitle;
	CStringArray hoveredLocationLines;
	int32_t umoirMapMode;
};
ASSERT_SIZE(VisGlobalMap, 0x1b4);

//609cb0
class SoundPreferencesDialogVisualObject : public VisWindow
{
public:
	SoundPreferencesDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SoundSettings* pset); //43d917

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43e6f0
	virtual void VMethod26() override; //43d959

	int32_t ConvVolumeToSlider(int32_t vol, int32_t max_val) { return max_val - (sqrt((float)-vol / (float)max_val) * (float)max_val); } //45ca10
	int32_t ConvSliderToVolume(int32_t pos, int32_t max_val) //45ca45
	{
		float v = (float)(pos - max_val) / (float)max_val;
		return max_val * (v * v);
	}
public:
	MusicPlayer* music_player;
	int32_t track_index;
	SoundSettings* settings;
};
ASSERT_SIZE(SoundPreferencesDialogVisualObject, 0x74);


//60db28
class VisLogoWnd : public VisScreen
{
public:
	VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cd4d0
	~VisLogoWnd(); //4cd53c

	virtual void VMethod7() override; //4cd8d1
	virtual void VMethod8(CRect* rect) override {}; //4cd91e
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4cd72e
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4cd6c2
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4cd6f8

	virtual void VMethod26(); //4cd5be
	virtual void DoClose(uint32_t code); //4cd666 29 method   

public:
	CBmp256* logoBitmap;
	int32_t step;
	uint32_t timeoutStart;
	uint32_t timeoutTicks;
	uint32_t drawPendingFlag;
};
ASSERT_SIZE(VisLogoWnd, 0x7c);


//60b288
class VisFameHall : public VisScreen
{
public:
	VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 45cd35 in asm
public:
	CFameHall* fame;
	CArray<CRect> name_rects;
	CArray<CRect> rank_rects;
	CArray<CRect> score_rects;
	CRect close_rect;
	CBmp64* bmp_bkg;
	CBmp64* bmp_close_off;
	CBmp64* bmp_close_on;
	CBmp64* bmp_cur_close;
	SfxSample* snd_close;
	uint32_t visible_flag;
};
ASSERT_SIZE(VisFameHall, 0xd0);

//60cdf8
class VisFameDocument : public VisScreen
{
public:
	VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4ad961 in asm
public:
	CFameHall* fame;
	CBmp64* bmp_sheet;
	CArray<CBmp64*> bmp_leftarrow;
	CArray<CBmp64*> bmp_rightarrow;
	CArray<CBmp64*> bmp_okbutton;
	CBmp64* bmp_cur_left;
	CBmp64* bmp_cur_right;
	CBmp64* bmp_cur_ok;
	int32_t selected_doc;
	uint32_t field10_0xbc;
	CRect rect_prev;
	CRect rect_next;
	CRect rect_ok;
	uint32_t field17_0xf0;
	uint32_t visible_flag;
};
ASSERT_SIZE(VisFameDocument, 0xf8);



#endif
