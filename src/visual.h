#ifndef VISUALOBJ_H
#define VISUALOBJ_H

#include <winsock2.h>
#include <array>
#include "assert_offset.h"
#include <afx.h>
#include <afxtempl.h>
#include <afxwin.h>
#include "sound.h"
#include "gameobj.h"
#include "txtfile.h"


uint32_t IsDefaultNpcName(const char* name); // 4386e0


class CGameSession;
class VisLabel;
class CGameBitmap;
class CGameObject;
class CGameFont;
class CBmp64;
class CBmp256;
class CA16;
class BigStruct2;
class VisCharInfo;
class VisCharSellectStats;
class VisCharSellectButtons;
class VisCharSellectList;
class VisShop;
class VisTav;
class Item;
class QuestMap;
class Scenario;

class CUnit;

class SfxSample;

class CFameHall;

struct SaveFileInfo;
struct DiplomacyEntry;
struct AvailNetSession;
struct ComSettings;
struct PhoneBook;
struct CLlAddress;
struct CLlConn;
struct HatSettings;



class CVisualObject : public CObject {
public:
	enum {
		MSG_409 = 0x409,
		MSG_40a = 0x40a,
		MSG_410 = 0x410,
	};

	enum {
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

	int32_t GetId() { return this->id; } //4503f0 (client: 4530f0)
	CRect& GetRect() { return this->rect; } //41ed40 (client: 4205f0)

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



class VisLabel : public CVisualObject {
public:
	~VisLabel();

	// VTable at 60de38.
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
class VisButton : public CVisualObject {
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
class VisScrollBar : public CVisualObject {
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
	struct Data {
		int32_t v;
		int32_t vmax;
	};
};
ASSERT_SIZE(VisScrollBar, 0xcc);

//60e0b0
class VisListBox : public CVisualObject {
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

	void SetSelectedIndex(int32_t idx); //4507a0 (client: 452cc0)
	int32_t GetSelectedIndex() { return this->selected_index; } //450780 (client: 453110)
	CString& GetItem(int32_t idx); //4508b0 (client: 453180)
	void RemoveItem(int32_t idx); //450830 (client: 453130)

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
class VisTextBox : public CVisualObject {
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
#ifdef A2CLIENT
	int32_t is_password; // client-only (retail client class is 4 bytes larger)
#endif
};
#ifdef A2CLIENT
ASSERT_SIZE(VisTextBox, 0x80);
#else
ASSERT_SIZE(VisTextBox, 0x7c);
#endif


//60e030
class VisRadioBase : public CVisualObject {
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
class VisRadioType1 : public VisRadioBase {
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
class VisRadioType2 : public VisRadioBase {
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
class VisBitmap : public CVisualObject {
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
class VisComboBox : public CVisualObject {
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
class VisComboBoxText : public VisTextBox {
public:
	virtual ~VisComboBoxText() = default;
	virtual void VMethod7() override;

	VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);
};

//60e548
class VisComboBoxList : public VisListBox {
public:
	virtual ~VisComboBoxList() = default;
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);
};

//60e458
class VisComboBoxButton : public VisButton {
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
class VisMultiText : public VisListBox {
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
class VisScreen : public CVisualObject {
public:
	virtual ~VisScreen(); // 4df5ec (client: 4e3f6d)
	virtual void VMethod7() override; // 4df63d (client: 4e3fbe)
	virtual void VMethod8(CRect* rect) override; // 4df67c (client: 4e3ffd)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4dfbcd (client: 4e454e)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4dfee7 (client: 4e4868)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4dfc43 (client: 4e45c4)

	virtual void VMethod26(); //450900
	virtual void VMethod27(); //4388c0
	virtual void VMethod28(); //4dfb4f (client: 4e44d0)
	virtual void DoClose(uint32_t code); //4dfb8a (client: 4e450b) 29 method   

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
class VisWindow : public VisScreen {
public:
	virtual ~VisWindow();
	virtual int32_t OnKeyDown(uint32_t wparam) override;

	VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap);

	void UpdateWinRect(); //4dfe19 (client: 4e479a)
};
ASSERT_SIZE(VisWindow, 0x68);


//60e3d0 — role/NPC speech dialog screen shown by ShowRoleKeyDialog
class VisRoleKeyScreen : public VisWindow {
public:
	virtual ~VisRoleKeyScreen(); //4e3f60
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4e0071 (client: 4e49f2)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4e0027 (client: 4e49a8)
	virtual void VMethod28() override; //4e012d (client: 4e4aae)
	virtual void DoClose(uint32_t code) override; //4e0148

	VisRoleKeyScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& text); //4dff5c (client: 4e48dd)

	bool sub_4E0169(); //4e0169 (client: 4e4aea) in asm - advance to next talk line
	bool FUN_004e0b43(int32_t line_no, int32_t* out_id, CString* out_text, char** out_name, char** out_name_end, int32_t* out_music); //4e0b43 (client: 4e54c4)
	void LoadText(); //4e0978 (client: 4e52f9)
public:
	int32_t field_0x68; // 0x68
	CString role_name; // 0x6c
	SfxSample* field_0x70; // 0x70 - speech sample currently playing
	char* text_buf; // 0x74
	int32_t text_len; // 0x78
	int32_t is_npc; // 0x7c
	int32_t field_0x80; // 0x80
	int32_t field_0x84; // 0x84
};
ASSERT_SIZE(VisRoleKeyScreen, 0x88);


//60a288
class VisMessageBox : public VisWindow {
public:
	virtual ~VisMessageBox(); // 444ff3
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44500f (client: 4476dd)
	virtual void VMethod26() override; // 444086 (client: 446656)

	virtual CVisualObject* VMethod30(const void* str, const RECT& r) = 0;
	virtual void VMethod31(int32_t code); // 445084

	VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, const char* str2, int32_t btypes, const char* str3); // 44402b (client: 4465fb)

public:
	const void* payload;
	const char* field_0x6c;
	const char* field_0x70;
	int32_t button_types;
};
ASSERT_SIZE(VisMessageBox, 0x78);

//60a318
class VisMessageBoxWithList : public VisMessageBox {
public:
	virtual ~VisMessageBoxWithList() {}; //44f930
	virtual CVisualObject* VMethod30(const void* str, const RECT& r); //4450d4 (client: 4477a2)

	VisMessageBoxWithList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str1, const char* str2, int32_t btypes); //445091 (client: 44775f)
};
ASSERT_SIZE(VisMessageBoxWithList, 0x78);

//60a3a8
class QuestObjectivesHeaderDialogVisualObject : public VisMessageBoxWithList {
public:
	QuestObjectivesHeaderDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b);  //445173 (client: 447841)

	virtual void VMethod26() override; //4451e1 (client: 4478af)
};
ASSERT_SIZE(QuestObjectivesHeaderDialogVisualObject, 0x78);


//609898
class VisCharSelect : public VisScreen {
public:
	virtual ~VisCharSelect(); // 438650

	virtual void VMethod8(CRect* rect) override; // 438d50
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4326A4 (client: 4347d5)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4326EA (client: 43481b)

	virtual void VMethod26() override; // 431FDF (client: 43410d)
	virtual void VMethod28() override; // 43233E (client: 43446f)
	virtual void DoClose(uint32_t code) override; // 43251C (client: 43464d)


	void OpenRenameWindow(); //4327f3 (client: 434924)

	void FUN_00432655(CUnit* unit); // client: 434786 (client: 434786)

	void LoadSfx(); // 432280
	void FreeSfx(); // 4322ed
	void CloseRenameWindow(); // 432933 (client: 434a64)
	CUnit* GetSelectedMapUnit(); // 438d30

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
class VisCharSellectButtons : public CVisualObject {
public:
	virtual ~VisCharSellectButtons(); // 4385B0

	virtual void VMethod7() override; // 43005F (client: 431f52)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4302F5
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 430318 (client: 43220b)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;

	VisCharSellectButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42fd5c

	void LoadBitmaps(); // 430850
	void FreeBitmaps(); // 430a1a
	void Init(); // 42fe62 (client: 431d56)
	void ResetMouseBoxes(); // 43072b

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
class VisCharSellectList : public CVisualObject {
public:
	virtual ~VisCharSellectList(); // 4385E0

	virtual void VMethod7() override; // 430FEC (client: 433143)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 431609 (client: 433760)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 431764 (client: 4338bb)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 43182C (client: 433983)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4316A0
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 431B95 (client: 433cec)

	VisCharSellectList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 430bbb (client: 432c87)

	void FUN_00430fad(); // 430fad (client: 433104)
	void Init(); // 430cb3 (client: 432d7f)
	void SelectRow(int32_t row); // 4316c4 (client: 43381b)
	int32_t SelectPrevRow(); // 431a05 (client: 433b5c)
	int32_t SelectNextRow(); // 431a93 (client: 433bea)
	void UpdateArrows(CPoint pos, bool is_down); // 431c97 (client: 433dee)
	int32_t HitTest(CPoint pos); // 431872 (client: 4339c9)
	void LoadBitmaps(); // 4312b7
	void FreeBitmaps(); // 43148a (client: 4335e1)

public:
	VisCharSelect* parent_screen;
	CRect field_0x60;
	CRect field_0x70;
	CRect field_0x80;
	CRect field_0x90;
	int32_t field_0xa0;
	CBmp64* field_0xa4;
	CBmp64* field_0xa8;
	CBmp64* field_0xac;
	CBmp64* field_0xb0;
	CBmp64* field_0xb4;
	CBmp64* field_0xb8;
	CBmp64* field_0xbc;
	uint32_t field_0xc0;
	uint32_t field_0xc4;
	uint32_t field_0xc8;
	uint32_t field_0xcc;
	int32_t field_0xd0;
	int32_t field_0xd4;
	int32_t field_0xd8;
};
ASSERT_SIZE(VisCharSellectList, 0xdc);

//609730
class VisCharSellectStats : public CVisualObject {
public:
	virtual ~VisCharSellectStats(); // 438580

	virtual const char* GetHint() override; // 42FC61 (client: 431b55)
	virtual void VMethod7() override; // 42F7B9 (client: 4316ad)


	void ReadUnitStats(); // 42f6f3 (client: 4315ef)

	VisCharSellectStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42f0ec

	void Init(); // 42f1e8 (client: 4310e4)
	void LoadBitmaps(); // 42f4df (client: 4313db)
	void FreeBitmaps(); // 42f61a (client: 431516)

public:
	VisCharSelect* parent_screen;
	CBmp64* field_0x60;
	CBmp64* field_0x64;
	CStringArray field_0x68;
	uint32_t field_0x7c;
	uint32_t field_0x80;
	uint32_t field_0x84;
	uint32_t field_0x88;
	CA16* field_0x8c;
	CRect field_0x90[4];
};
ASSERT_SIZE(VisCharSellectStats, 0xd0);


struct TakeDamage {
	CString txt;
	int32_t dmg = 0;
	uint16_t* color = nullptr;
	int32_t dx = 0;
	int32_t dy = 0;
	uint32_t timestamp = 0;
	int32_t field_0x18 = 1;
	CUnit* cunit = nullptr;
	int32_t field_0x20 = 0;

	TakeDamage(); //45db60 (client: 461780)
	TakeDamage(int32_t dmg, uint16_t* clr, int32_t unk, int32_t _dx, int32_t _dy, CUnit* ct); //45db8a (client: 4617aa)
	TakeDamage(const TakeDamage& ref);
	~TakeDamage(); //45dd08

	void Update(); //45dd1b (client: 46193b)
	int Draw(); //45dd6d (client: 46198d)

	int32_t FUN_0041f620(); //41f620 (client: 4203d0) in asm
	int32_t FUN_0041f640(); //41f640 (client: 4203f0) in asm

	static void __cdecl AddDamage(CArray<TakeDamage>* arr, const TakeDamage& dmg); //45dea1 (client: 461ac1)
	static void __cdecl DrawDamages(CArray<TakeDamage>* arr); //45ddf6 (client: 461a16)
	static void __cdecl UpdateDamages(CArray<TakeDamage>* arr); //45de65 (client: 461a85)
};
ASSERT_SIZE(TakeDamage, 0x24);



class GM_a28 : public CObject {
public:
	virtual ~GM_a28(); //4020ad (client: 4020ad)
	GM_a28(); //401f50 (client: 401f50)

	void Add(const char* msg, uint16_t* clr, uint32_t t); //4021fe (client: 4021fe)
	void AddWithCheck(const char* msg, uint16_t* clr, uint32_t t); //4023aa (client: 4023aa)
	void Clear(); //4021d2 (client: 4021d2)
	void Draw(); //40261a (client: 40261a)
	void SetRect(const CRect& r); //40212b (client: 40212b)
	void Update(); //402585 (client: 402585)
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

class BigStruct2 : public CVisualObject {
public:
	BigStruct2(int32_t l, int32_t t, int32_t r, int32_t b); // 402af6 (client: 402af6)
	~BigStruct2(); // 403395

	// VTable at 609088.
	virtual void VMethod7() override; // 406f1a (client: 406f89)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 40c232 (client: 40c2b9)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 40c83f (client: 40c8c6)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 40c510 (client: 40c597)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 40c633 (client: 40c6ba)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 40c60f
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 40c56c (client: 40c5f3)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 40c59c (client: 40c623)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 40c902 (client: 40c989)
	virtual int32_t OnChar(uint32_t wparam) override; // 40d4b2 (client: 40d539)

public:
	void sub_406F7B(); //406f7b
	void sub_40403B(); //40403b (client: 4040e2)
	void sub_404E1A(); //404e1a (client: 404eb5)
	void sub_405399(int32_t arg); //405399
	void sub_40587B(int32_t arg); //40587b (client: 405908)
	void sub_405D0E(); //405d0e
	void sub_404DA3(CRect* rect, uint16_t unit_id); //404da3
	void sub_40AFC5(int32_t x, int32_t y, uint32_t flags, int32_t flag); //40afc5 (client: 40b010)
	int32_t sub_40B173(int32_t gfx_id, int32_t x, int32_t y, int32_t height); //40b173 (client: 40b1be)
	void sub_40328E(); //40328e (client: 403311)
	void sub_404912(); //404912 (client: 4049b7)
	void sub_404A2C(); //404a2c
	void sub_403F08(Scenario* scen); //403f08 (client: 403faf)
	void sub_40B314(); //40b314
	uint32_t sub_41864D(); //41864d
	int32_t sub_4184B8(int32_t x, int32_t y); //4184b8 (client: 419c7d)
	void sub_417B42(CPoint* pt); //417b42
	void sub_417F09(CRect* rect); //417f09
	void sub_419154(uint16_t x, uint16_t y); //419154
	void sub_419246(uint16_t id); //419246
	void sub_41930D(uint16_t x, uint16_t y); //41930d
	void sub_41955C(uint16_t id); //41955c
	void sub_41965D(uint16_t x, uint16_t y); //41965d
	void sub_41972F(uint16_t x, uint16_t y, int32_t spell); //41972f
	void sub_419BA1(uint16_t id, int32_t spell); //419ba1
	void sub_41A33F(uint16_t x, uint16_t y); //41a33f
	void sub_41A66E(uint16_t id); //41a66e
	void sub_41AC2D(int32_t id); //41ac2d in asm
	void sub_41AC88(int32_t arg1, int32_t arg2); //41ac88 (client: 41c514)
	void sub_41A7C7(uint8_t arg1, uint16_t arg2, uint8_t arg3, uint16_t arg4, uint16_t arg5); //41a7c7 (client: 41c053)
	void sub_40BD34(); //40bd34
	void sub_40C112(); //40c112 (client: 40c199)
	void sub_40C18B(); //40c18b (client: 40c212)
	int32_t sub_403B8F(int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4); //403b8f (client: 403c36)
	void sub_402FB8(); //402fb8 (client: 40303b)
	void sub_403730(); //403730 (client: 4037d7)
	void sub_4168BD(); //4168bd (client: 4180d4)
	void sub_4167F7(int32_t wparam, int32_t lparam); //4167f7 (client: 41800e)
	void sub_418F93(int32_t arg); //418f93 (client: 41a74e)
	void sub_4193E8(); //4193e8
	void sub_4194A2(); //4194a2
	void sub_41A425(); //41a425
	void sub_41B8D0(); //41b8d0
	void sub_416983(int32_t digit); //416983 (client: 41819a)
	void sub_416A02(int32_t digit); //416a02
	void sub_416AA6(int32_t digit); //416aa6
	void sub_416B4A(int32_t digit); //416b4a (client: 418361)
	void sub_41A5B4(); //41a5b4
	CUnit* sub_41DFDB(); //41dfdb (client: 41f85d)
	void sub_41A4DF(); //41a4df
	void sub_41B155(); //41b155
	void sub_416C4C(); //416c4c (client: 418463)
	void sub_41F6D0(); //41f6d0
	void sub_41F770(); //41f770
	void sub_41F7B0(); //41f7b0
	void sub_41F720(); //41f720
	int32_t sub_41F7F0(); //41f7f0 (client: 420c60)
	int32_t sub_41F810(); //41f810
	void sub_41AB05(); //41ab05 (client: 41c391)
	void sub_41A9F6(); //41a9f6
	void sub_41A99C(); //41a99c
	void sub_41CE14(); //41ce14
	void sub_41A942(); //41a942

	void UpdateSelectionState(); //416cf7 (client: 41850e)
	void FUN_0041cda3(const char* mapname); // 41cda3 (client: 41e627)
	uint32_t sub_40C0A8(int32_t x, int32_t y); // 40c0a8 (client: 40c12f)
	void FUN_0041ace2(int32_t spell_id); //41ace2 (client: 41c56e)
	void FUN_0041a001(int32_t idx); //41a001
	void UpdateSpellEffects(CUnit* unit); //from 416cf7 (client: 41850e)
	void UpdateSpellModifiers(CUnit* unit); //from 416cf7 (client: 41850e)

	void FUN_0041c4a1(const char* name); //41c4a1 (client: 41dd29)

	CUnit* GetUnit_3f6c(); //41f830
	void SetUnit_3f6c(CUnit* uni) { this->field_0x3f6c = uni; };

	int32_t ProcessPackets(uint8_t breakid); //40da14 (client: 40dac9)

	void FUN_0041b7b7(int32_t xpos, int32_t ypos, int32_t* pvol, int32_t* ppan); //41b7b7
	void FUN_0041c74b(int32_t x, int32_t y, int32_t w, int32_t h, int32_t unk1, uint8_t* dat, int32_t unk2); //41c74b

	CUnit* FUN_0041df23(int32_t serv_id); //41df23 (client: 41f7a5)
	int32_t FUN_0041e2af(int32_t id, CUnit** out); //41e2af (client: 41fb31)
	CUnit* FUN_0041dfa6(int32_t serv_id); //41dfa6 (client: 41f828)
	CGameBitmap* FUN_0041da52(int32_t serv_id); //41da52 (client: 41f2d4) in asm
	CUnit* sub_41DE80(int32_t serv_id); //41de80 (client: 41f702)
	void FUN_0041ae1c(int32_t serv_id); //41ae1c

	void FUN_0041c630(CWordArray *data); //41c630

	void FUN_0041cb67(int32_t val); //41cb67
	void FUN_0041cb21(); //41cb21

	void FUN_0041b2a4(const char* nam, int32_t token, int32_t playerid); //41b2a4 (client: 41cb2c)
	void FUN_0041a8cc(); //41a8cc (client: 41c158)

	int32_t FUN_0040d7f3(); //40d7f3
	int ConnectAndJoinSession(); //40d4e2 (client: 40d569)

	void FUN_0041cbb8(); //0041cbb8 (client: 41e43c)
	void FUN_0041cc78(const CString& str); //41cc78 (client: 41e4fc)
	void FUN_0041d2da(int32_t arg); //41d2da (client: 41eb5c)
	int32_t IsBookOpen(); //41b4b0 (client: 41cd38)
	int32_t IsBagOpen(); //41b495 (client: 41cd1d)

	void FUN_0041afcf(CString str); //41afcf (client: 41c857)
	void FUN_0041a735(); //41a735 (client: 41bfc1)

	void FUN_0041cad0(int32_t m); //41cad0
	void FUN_0041aaaa(int32_t wim); //41aaaa
	void FUN_0041abd2(int32_t form); // 41abd2
	void FUN_0041ab74(); //41ab74 (client: 41c400)

	void FUN_0041b6e0(); //41b6e0 (client: 41cf68)
	int32_t MapMaxY() const { return (this->map_height - 8) - this->view_tiles_y; } //41f6b0

	void FUN_0041b636(); //41b636 (client: 41cebe)

	int FUN_0041b495() { return this->FindChild(2) != nullptr; } //41b495 (client: 41cd1d)
	void FUN_0041b509(); //41b509 (client: 41cd91)
	void FUN_0041b40e(); //41b40e (client: 41cc96)
	void FUN_0041b381(); //41b381 (client: 41cc09)
	void FUN_0041b10f(); //41b10f
	void FUN_0041b064(int32_t arg1, int32_t arg2); //41b064 (client: 41c8ec)
	void FUN_0041c39c(); //41c39c (client: 41dc24)

	void CleanupCompletedMissionMapState(); //41ce5a

	void FUN_00403ca0(CGameObject* obj); //403ca0 (client: 403d47)
	void FUN_0041d97e(int32_t); //41d97e (client: 41f200)

	uint32_t AvailableOrderMask() const; // 417af6 (client: 4192c7)
	void SendAdjustPlayerGoldAction(int32_t goldDelta) const; //41b0be

	int32_t MapMinX() const { return 8; } //41f670
	int32_t MapMinY() const { return 8; } //41f680

	int32_t MapMaxX() const { return (this->map_width - 8) - this->view_tiles_x; } //41f690

	void ScrollMapX(int32_t dx); //403f4f
	void ScrollMapY(int32_t dy); //403fc5

	void OnOpenShopDialog(); //41a856 (client: 41c0e2)
	void NetOnOpenInnDialog(); //41adbb (client: 41c647)

	void UpdateAmbientSounds(); //41b8d0
	void MakeHeightCells(); //4065d3 (client: 406658)
	void ApplyDynamicLight(int32_t tx, int32_t ty, int32_t radius, int32_t brightness); //406340 (client: 4063c5)

public:
	int32_t view_x;
	int32_t view_y;
	int32_t view_tiles_x; // 0x64 - visible map width in tiles
	int32_t view_tiles_y; // 0x68 - visible map height in tiles
	int32_t tile_grid_bytes; // 0x6c - byte size of the per-tile arrays ((view_tiles_x + 6) * (view_tiles_y + 10) dwords)
	int32_t corner_grid_bytes; // 0x70 - byte size of the per-corner arrays ((view_tiles_x + 7) * (view_tiles_y + 11) dwords)
	int32_t force_full_redraw; // 0x74
	int32_t scroll_dx; // 0x78
	int32_t scroll_dy; // 0x7c
	Scenario* field_0x80;
	int32_t map_width; // 0x84 - scenario width in cells
	int32_t map_height; // 0x88 - scenario height in cells
	CGameObject** tile_units; // 0x8c - per-tile ground units
	CGameObject** tile_air_units; // 0x90 - per-tile air units
	CGameObject** tile_structures; // 0x94 - per-tile structures
	CGameObject** tile_backpacks; // 0x98 - per-tile backpacks
	CGameObject** tile_special_units; // 0x9c - per-tile special units (CUnit::VMethod10 bucket 4)
	uint32_t* field_0xa0; //unk type
	uint32_t* field_0xa4; //unk type
	uint8_t* field_0xa8; //unk type
	uint8_t* field_0xac; //unk type
	uint8_t* tile_light; // 0xb0 - per-tile light level
	uint32_t* tile_corner_y; // 0xb4 - screen Y per tile corner (corner grid)
	uint32_t* tile_top_y; // 0xb8 - screen Y of tile top edge (tile grid)
	uint32_t* tile_bottom_y; // 0xbc - screen Y of tile bottom edge (tile grid)
	uint32_t* height_cells; // 0xc0 - per-corner averaged terrain heights (stride view_tiles_x + 8)
	int32_t field_0xc4; //unk type
	int32_t field_0xc8; //unk type
	int32_t field_0xcc; //unk type
	double field_0xd0;
	int32_t field_0xd8;
	int32_t field_0xdc;
	int32_t field_0xe0;
	int32_t tile_scroll_x; // 0xe4
	int32_t tile_scroll_y; // 0xe8
	int32_t prev_view_x; // 0xec
	int32_t prev_view_y; // 0xf0
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
#ifdef A2CLIENT
	// client tail layout (retail class is 0x4a48 bytes, client ctor 402af6): the
	// server's 12-slot item array extends to 32 hat-DM slots (entries 12..31 are
	// the fields below), and the server's 40000 timer triple (field_0x49b8/
	// field_0x49bc/field_0x49c0 at the end of this block) moves behind the hat
	// containers to 0x4a38..0x4a40
	int32_t hat_item_slot12; // client 0x49b8 (ctor sets 0)
	int32_t hat_item_slot13; // client 0x49bc (ctor sets 0xF0000001)
	int32_t hat_item_slot14; // client 0x49c0 (ctor sets 0xF0000001)
	int32_t hat_cursor_slots[16]; // client 0x49c4..0x4a03 = slots 15..30 (ctor 0xF0000003; packet 0xc2 bulk state)
	int32_t field_0x4a04; // client 0x4a04 = slot 31 (ctor 0x7fffffff; hat player-list state, packets 0x5c/0xc2)
	int32_t cursor_over_obj_last_index; // client 0x4a08
	int32_t last_focus_obj_index; // client 0x4a0c
	CStringArray hat_name_list; // client 0x4a10 — hat DM player names (packets 0x5a/0x5b/0x5c)
	CArray<uint32_t> hat_score_list; // client 0x4a24 — parallel scores (plain dwords), sorted descending
	int32_t field_0x49b8; //unk type — at client 0x4a38 (40000)
	int32_t field_0x49bc; //unk type — at client 0x4a3c (40000)
	int32_t field_0x49c0; //unk type — at client 0x4a40 (0)

	void CloseHatOverlays(); // client 41748d/417902 — close shop/tavern/popup/1200 overlays
#else
	int32_t field_0x49b8; //unk type
	int32_t field_0x49bc; //unk type
	int32_t field_0x49c0; //unk type
	int32_t field_0x49c4; //unk type
#endif
};

ASSERT_OFFSET(BigStruct2, field_0x80, 0x80);
ASSERT_OFFSET(BigStruct2, field_0x9d0, 0x9d0);
ASSERT_OFFSET(BigStruct2, damage_labels, 0x3f54);
#ifdef A2CLIENT
ASSERT_OFFSET(BigStruct2, hat_cursor_slots, 0x49c4);
ASSERT_OFFSET(BigStruct2, field_0x4a04, 0x4a04);
ASSERT_OFFSET(BigStruct2, hat_name_list, 0x4a10);
ASSERT_OFFSET(BigStruct2, hat_score_list, 0x4a24);
ASSERT_OFFSET(BigStruct2, field_0x49b8, 0x4a38);
ASSERT_SIZE(BigStruct2, 0x4a48);
#else
ASSERT_SIZE(BigStruct2, 0x49c8);
#endif

//60ce80
class VisRightPanel : public CVisualObject {
public:
	virtual ~VisRightPanel(); // 4B46E0

	virtual void VMethod7() override; // 4AEE31
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AEE44 (client: 4b3784)

	VisRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aedcf in asm
};
ASSERT_SIZE(VisRightPanel, 0x5c);


//60cef8
class VisMiniMap : public CVisualObject {
public:
	virtual ~VisMiniMap(); // 4B4730

	virtual void VMethod7() override; // 4AFB8E (client: 4b44c4)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AF006 (client: 4b3946)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B0C2F (client: 4b5561)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B0788
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B0C84
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4B0AAD (client: 4b53df)

	VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aeec3 (client: 4b3803)

	void RebuildMap(); // 4af0dc in asm
	void UpdateCursor(); // 4b042e (client: 4b4d64) in asm

public:
	BigStruct2* map_context;
	CBmp64* bitmap1;
	CBmp64* bitmap2;
	int32_t zoom;
	uint32_t viewer;
};
ASSERT_SIZE(VisMiniMap, 0x70);


//60cf70
class VisOrderToolbar : public CVisualObject {
public:
	virtual ~VisOrderToolbar(); // 4b4790

	virtual const char* GetHint() override; // 4B0D9A (client: 4b56cc)
	virtual void VMethod7() override; // 4B12E3 (client: 4b5c15)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B0ED1 (client: 4b5803)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B12B3 (client: 4b5be5)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B107B (client: 4b59ad)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B124C
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B1213 (client: 4b5b45)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b1201
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B1287 (client: 4b5bb9)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b123a

	void UpdateCursor(); // 4b1468 (client: 4b5d9a)

	VisOrderToolbar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b0cfc (client: 4b562e) in asm
public:
	BigStruct2* map_context;
	uint32_t selected_order;
	uint32_t avail_orders_mask;
	uint32_t enabled;
	uint32_t dirty;
};
ASSERT_SIZE(VisOrderToolbar, 0x70);






//60c9b0
class VisInvBase : public CVisualObject {
public:
	VisInvBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a4938 (client: 4a8968)

	virtual ~VisInvBase(); // 4A77D0
	virtual const char* GetHint() override; // 4a4cf8
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A5DE0
	virtual int32_t VMethod26(TokenEntry* o, int32_t idx); //4a554f (client: 4a957e)
	virtual int32_t VMethod27(TokenEntry* o); //4a56c3 (client: 4a96f2)
	virtual TokenEntry* VMethod28(uint32_t id); //4a57a1 (client: 4a97d0)
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num); //4a58dc (client: 4a990b)
	virtual int32_t VMethod30(int32_t x, int32_t y); //4a79a0
	virtual int32_t VMethod31(const CPoint* pt); //4a7990
	virtual void VMethod32(CArray<TokenEntry*>* arr); //4a4da9 (client: 4a8dd9)
	virtual void VMethod33(CGameObject* uni); //4a4d05 (client: 4a8d35)
	virtual void VMethod34(); //4a4e28 (client: 4a8e58)
	virtual void VMethod35(); //4a4c2f (client: 4a8c5f)
	virtual TokenEntry* VMethod36(int32_t idx, int32_t num); //4a5aae (client: 4a9add)
	virtual int32_t VMethod37(int32_t idx); //4a5c39 (client: 4a9c68)
	virtual int32_t VMethod38(); //4a79b0

	int32_t FUN_0046fb90(); //46fb90
	int32_t sub_41F940(); //41f940
	void FUN_004a4ebc(); //4a4ebc (client: 4a8eec)
	void sub_4A4BCB(); //4a4bcb (client: 4a8bfb)
	void sub_4A5350(); //4a5350 (client: 4a937f) in asm
	void sub_4A51C9(); //4a51c9 (client: 4a91f8) in asm
	void FUN_004a5e12(); //4a5e12 (client: 4a9e41) in asm
	void sub_4A5FAB(int32_t x, int32_t y, int32_t idx); //4a5fab (client: 4a9fda) in asm

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
class VisInvType1 : public VisInvBase {
public:
	virtual ~VisInvType1(); // 4A7830

	virtual const char* GetHint() override; // 4A765D (client: 4ab68b)
	virtual void VMethod7() override; // 4A6449 (client: 4aa478)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A7441 (client: 4ab46f)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4A716F (client: 4ab19d)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4A741D (client: 4ab44b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4A6E57 (client: 4aae86)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4A70EC (client: 4ab11a)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4A6F44 (client: 4aaf73)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4a6f32
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4a715d
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4a70da

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4A79F0 (client: 4ab9d0)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4A79C0 (client: 4ab9a0)
	virtual int32_t VMethod30(int32_t x, int32_t y) override; // 4A6D11 (client: 4aad40)
	virtual int32_t VMethod38() override; // 4a7a20 (client: 4aba00)

	VisInvType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a630b (client: 4aa33a) in asm

public:
};
ASSERT_SIZE(VisInvType1, 0x20ac);

//60d0e8
class VisInvExtBase : public VisInvBase {
public:
	VisInvExtBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b4a0c (client: 4b93dc) in asm
	virtual ~VisInvExtBase(); // 4B4AEC (client: 4b94bc)

	virtual const char* GetHint() override; // 4B4C64 (client: 4b9634)
	virtual void VMethod7() override; // 4B5072 (client: 4b9a42)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B5BBE (client: 4ba58d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B5D19 (client: 4ba6e8)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B5D5C (client: 4ba72b)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B5F3F
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b62a2
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4b62b4
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b62c6

	virtual int32_t VMethod30(int32_t x, int32_t y) override; // 4B4FA3 (client: 4b9973)
	virtual int32_t VMethod31(const CPoint* pt) override; // 4B4E88 (client: 4b9858)
	virtual void VMethod32(CArray<TokenEntry*>* arr) override;
	virtual int32_t VMethod38() override;

	virtual void VMethod39();
	virtual void VMethod40(CArray<TokenEntry*>* arr);
	virtual void VMethod41();
	virtual void VMethod42();

	int32_t sub_4B5B81(); //4b5b81 (client: 4ba550) in asm
	int32_t sub_4B5A6C(int32_t idx); //4b5a6c (client: 4ba43b) in asm
	void sub_4B4BA9(); //4b4ba9 in asm
	void sub_4B4C1C(); //4b4c1c (client: 4b95ec)
	void sub_4B4BC5(); //4b4bc5 (client: 4b9595)
	void sub_4B4D33(); //4b4d33 (client: 4b9703)
	void sub_4B4FD1(); //4b4fd1 (client: 4b99a1)

public:
	VisShop* field_0x20ac;
	CSound field_0x20b0;
	CSound field_0x20b4;
	CSound field_0x20b8;
	CSound field_0x20bc;
	int32_t field_0x20c0; //unk type
	CRect* field_0x20c4;
};
ASSERT_SIZE(VisInvExtBase, 0x20c8);

//60d1a8
class VisInvExtType1 : public VisInvExtBase {
public:
	virtual ~VisInvExtType1(); // 4B9CB0 (client: 4be680)

	virtual const char* GetHint() override; // 4B6F8D (client: 4bb957)
	virtual void VMethod7() override; // 4B70C0 (client: 4bba8a)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B7690 (client: 4bc05a)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4B7433 (client: 4bbdfd)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B7562 (client: 4bbf2c)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4b7412 (client: 4bbddc)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4B777C (client: 4bc146)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B790D (client: 4bc2d7)
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num) override; // 4B7A9F (client: 4bc469)

	virtual int32_t VMethod37(int32_t idx) override;
	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	virtual void VMethod41() override;
	virtual void VMethod42() override;

	void sub_4B7859(); //4b7859 (client: 4bc223)
	void sub_4B73E4(int32_t category); //4b73e4 (client: 4bbdae)
	void sub_4B7264(); //4b7264 in asm
	void sub_4B72C1(); //4b72c1 in asm
	void sub_4B731E(); //4b731e in asm
	void sub_4B7381(); //4b7381 in asm

	VisInvExtType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b63f7 (client: 4badc1) in asm

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


//60d910
class VisInvExtType1Kaarg : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b69ee (client: 4bb3b8) in asm

	VisInvExtType1Kaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6cd0 in asm
};
ASSERT_SIZE(VisInvExtType1Kaarg, 0x2150);


//60d850
class VisInvExtType1Druid : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b6815 (client: 4bb1df) in asm

	VisInvExtType1Druid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6c40 in asm
};
ASSERT_SIZE(VisInvExtType1Druid, 0x2150);


//60d268
class VisInvExtType2 : public VisInvExtBase {
public:
	virtual ~VisInvExtType2(); // 4B9CE0 (client: 4be6b0)

	virtual const char* GetHint() override; // 4B8339 (client: 4bcd03)
	virtual void VMethod7() override; // 4B846C (client: 4bce36)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B88EC (client: 4bd2b6)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4B877A (client: 4bd144)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B87CE (client: 4bd198)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4b87ad (client: 4bd177)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B9ED0 (client: 4be8a0)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4B9EA0 (client: 4be870)

	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	void sub_4B860E(); //4b860e in asm
	void sub_4B8666(); //4b8666 in asm
	void sub_4B86BE(); //4b86be in asm
	void sub_4B871C(); //4b871c in asm

	VisInvExtType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b7fe5 (client: 4bc9af) in asm
public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
};
ASSERT_SIZE(VisInvExtType2, 0x20ec);


//60d328
class VisInvExtType3 : public VisInvExtBase {
public:
	virtual ~VisInvExtType3(); // 4B9D10

	virtual const char* GetHint() override; // 4B90CF (client: 4bda99)
	virtual void VMethod7() override; // 4B9155 (client: 4bdb1f)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B92F9 (client: 4bdcc3)

	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b93ae
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B93CF (client: 4bdd99)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b9436

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B9457 (client: 4bde21)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4B949A (client: 4bde64)

	virtual TokenEntry* VMethod36(int32_t idx, int32_t num) override;
	virtual int32_t VMethod37(int32_t idx) override;
	virtual int32_t VMethod38() override;
	virtual void VMethod39() override;

	virtual void VMethod41() override;
	virtual void VMethod42() override;

	virtual TokenEntry* VMethod43(int32_t id1, int32_t id2);

	int32_t sub_4B91F9(TokenEntry* entry); //4b91f9 (client: 4bdbc3) in asm
	VisInvExtType3(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b8bba (client: 4bd584) in asm

	void sub_4B970E(); //4b970e (client: 4be0d8)
	int32_t sub_4B9B2F(int32_t idx); //4b9b2f (client: 4be4f9) in asm
public:
	int32_t field_0x20c8; //unk type
	CRect field_0x20cc;
	CRect field_0x20dc;
	CBmp64* shoptable;
	int32_t field_0x20f0; //unk type
	CArray<TokenEntry*> field_0x20f4;
};
ASSERT_SIZE(VisInvExtType3, 0x2108);


class VisTavSceneAnim {
public:
	void FUN_004010ee(CStringArray* names); //4010ee (client: 4010ee) in asm
	void FUN_004014f2(); //4014f2 (client: 4014f1) in asm
	bool StepForward(); //4015c6 (client: 4015c5) in asm
	bool StepBackward(); //401613 (client: 401612) in asm
	void NextFrame(); //401659 (client: 401658) in asm
	void Draw(int32_t x, int32_t y); //4016ef (client: 4016ee) in asm

public:
	CArray<CBmp64*> frames;
	CBmp64* current_frame; // 0x14
	int32_t frame_idx; // 0x18
	CStringArray names;
};
ASSERT_SIZE(VisTavSceneAnim, 0x30);


//60c5d8
class VisTavRightPanel : public CVisualObject {
public:
	virtual ~VisTavRightPanel(); // 4A3E40

	virtual const char* GetHint() override; // 4a4710 in asm
	virtual void VMethod7() override; // 499dfa (client: 49de1a) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49a26e in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49a291 (client: 49e2b1) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 49a30a in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49a2e6 in asm

	VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4997c6 in asm

	void FUN_00499a67(); //499a67 (client: 49da87) in asm
	void FUN_0049a84e(); //49a84e in asm
	void FUN_0049a973(); //49a973 (client: 49e9a2) in asm
	void FUN_00499cdf(); //499cdf in asm
	void FUN_004998c2(); //4998c2 in asm
	int32_t FUN_0049a873(CPoint pos); //49a873 in asm
	void FUN_0049a8fa(uint32_t wparam, CPoint pos); //49a8fa in asm

public:
	VisTav* vis_tav; // 0x5c
	CStringArray texts; // 0x60
	CBmp64* field_0x74[3]; // 0x74
	CBmp64* field_0x80[3]; // 0x80
	CBmp64* field_0x8c; // 0x8c
	CRect field_0x90[3]; // 0x90
	int32_t field_0xc0; // 0xc0
	int32_t field_0xc4; // 0xc4
};
ASSERT_SIZE(VisTavRightPanel, 0xc8);


// 60c560
class VisTavLeftPanel : public CVisualObject {
public:
	virtual ~VisTavLeftPanel(); // 4A3E10

	virtual const char* GetHint() override; // 49934d (client: 49d36d) in asm
	virtual void VMethod7() override; // 497ed0 (client: 49bef0) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 497ec1 in asm

	VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //497990 (client: 49b9b0) in asm

	void FUN_004995d1(); //4995d1 (client: 49d5f1) in asm
	void FUN_004996ab(); //4996ab (client: 49d6cb) in asm
	void FUN_00497ace(); //497ace (client: 49baee) in asm
	void FUN_00497f82(CUnit* unit); //497f82 (client: 49bfa2) in asm
	void FUN_00497e06(int32_t idx); //497e06 (client: 49be26) in asm

public:
	VisTav* vis_tav; // 0x5c
	char field_0x60[0x100]; // 0x60
	CBmp64* field_0x160; // 0x160
	CBmp256* field_0x164; // 0x164
	CBmp64* field_0x168; // 0x168
	CBmp64* field_0x16c; // 0x16c
	CRect field_0x170; // 0x170
	CRect field_0x180; // 0x180
};
ASSERT_SIZE(VisTavLeftPanel, 0x190);


//60c650
class VisTavScene : public CVisualObject {
public:
	virtual ~VisTavScene(); // 4A3E70

	virtual const char* GetHint() override; // 49cd2b (client: 4a0d59) in asm
	virtual void VMethod7() override; // 49b22f (client: 49f25e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49d8d9 in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49da98 (client: 4a1ac6) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49d8e8 (client: 4a1916) in asm

	virtual void VMethod26(); //49ced4 (client: 4a0f02) in asm
	virtual void VMethod27(); //49d1b0 (client: 4a11de) in asm
	virtual void VMethod28(); //49d2bb in asm
	virtual void VMethod29(); //49d5f9 in asm

	VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //49ad56 (client: 49ed85) in asm

	void FUN_0049bc23(); //49bc23 (client: 49fc52) in asm
	int32_t FUN_0049cab8(CPoint* pos); //49cab8 (client: 4a0ae6) in asm
	void FUN_0049af8c(); //49af8c (client: 49efbb) in asm

public:
	VisTav* vis_tav; // 0x5c
	CRect field_0x60[18]; // 0x60
	CRect field_0x180[9]; // 0x180
	CArray<CSprite256*> field_0x210; // 0x210
	CArray<CSprite256*> field_0x224; // 0x224
	CArray<CSprite256*> field_0x238; // 0x238
	CArray<uint32_t> field_0x24c; // 0x24c
	CArray<uint32_t> field_0x260; // 0x260
	CBmp64* field_0x274; // 0x274
	CBmp64* field_0x278; // 0x278
	CBmp64* field_0x27c; // 0x27c
	VisTavSceneAnim anims[4]; // 0x280
	CBmp64* field_0x340; // 0x340
	CBmp64* field_0x344; // 0x344
	CBmp64* field_0x348; // 0x348
	CBmp64* field_0x34c[13]; // 0x34c
	uint8_t __gap_0x380[76];
};
ASSERT_SIZE(VisTavScene, 0x3cc);


//60c7f8
class VisTavSceneDruid : public VisTavScene {
public:
	virtual ~VisTavSceneDruid(); // 4A3F90

	virtual void VMethod7() override; // 4a0673 (client: 4a46a1) in asm
	virtual void VMethod28() override; // 4a11e3 (client: 4a5211) in asm
	virtual void VMethod29() override; // 4a158f (client: 4a55bd) in asm

	VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a05a0 (client: 4a45ce) in asm

public:
	CBmp64* field_0x3cc;
	VisTavSceneAnim anims_druid[3]; // 0x3d0
	uint32_t field_0x460;
	uint32_t field_0x464;
};
ASSERT_SIZE(VisTavSceneDruid, 0x468);


//60c910
class VisTavSceneKaarg : public VisTavScene {
public:
	virtual ~VisTavSceneKaarg(); // 4A4090

	virtual void VMethod7() override; // 4a2bb8 (client: 4a6be6) in asm
	virtual void VMethod28() override; // 4a37e7 in asm
	virtual void VMethod29() override; // 4a3b25 in asm

	VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a2aa4 (client: 4a6ad2) in asm

public:
	VisTavSceneAnim anims_kaarg[5]; // 0x3cc
	uint32_t field_0x4bc;
	uint32_t field_0x4c0;
	uint32_t field_0x4c4;
	uint32_t field_0x4c8;
	uint32_t field_0x4cc;
};
ASSERT_SIZE(VisTavSceneKaarg, 0x4d0);


//60c6d8
class VisTav : public VisScreen {
public:
	virtual ~VisTav(); // 4A3F10

	virtual void VMethod7() override; // 49E044 (client: 4a2072)
	virtual void VMethod8(CRect* rect) override; // 49e075
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 49E082 (client: 4a20b0)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49E261
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 49E233 (client: 4a2261)

	virtual void VMethod26() override; // 49DDAF
	virtual void VMethod28() override; // 49E34F (client: 4a237d)
	virtual void DoClose(uint32_t code) override; // 49EB6C (client: 4a2b9a)

	virtual void VMethod30(); // 49F298 (client: 4a32c6)
	virtual void VMethod31(); // 49F3D7 (client: 4a3405)

	void FUN_0049edec(); // client: 4a2e1a (client: 4a2e1a)
	void FUN_0049ee36(); //49ee36 (client: 4a2e64) in asm
	void FUN_0049eecd(); //49eecd (client: 4a2efb) in asm
	int32_t FUN_0049e2e3(CUnit* unit); //49e2e3 (client: 4a2311) in asm
	void FUN_0049ef63(); //49ef63 (client: 4a2f91) in asm
	void FUN_0049f0da(); //49f0da (client: 4a3108) in asm
	void FUN_0049f179(); //49f179 (client: 4a31a7) in asm

	VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //49db79 (client: 4a1ba7) in asm

public:
	BigStruct2* map_context;
	VisCharInfo* info_panel;
	VisTavLeftPanel* left_panel;
	VisTavRightPanel* right_panel;
	int32_t field_0x78; //unk type
	VisTavScene* scene;
	CVisualObject* tips;
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
class VisTavDruid : public VisTav {
public:
	virtual ~VisTavDruid(); // 4A3F40

	virtual void VMethod26() override; // 49F51B
	virtual void VMethod28() override; // 49FAAA (client: 4a3ad8)
	virtual void DoClose(uint32_t code) override; // 4A0278 (client: 4a42a6)
	virtual void VMethod30() override; // 49F7B0 (client: 4a37de)
	virtual void VMethod31() override; // 49F961 (client: 4a398f)

	VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49f4e6 in asm
public:
	SfxSample* snd_druid[11];
};
ASSERT_SIZE(VisTavDruid, 0x16c);

//60c880
class VisTavKaarg : public VisTav {
public:
	virtual ~VisTavKaarg(); // 4A4040

	virtual void VMethod26() override; // 4A190D
	virtual void VMethod28() override; // 4A1BA2 (client: 4a5bd0)
	virtual void DoClose(uint32_t code) override; // 4A2448 (client: 4a6476)
	virtual void VMethod30() override; // 4A26D9 (client: 4a6707)
	virtual void VMethod31() override; // 4A2873

	VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4a18d8 in asm
public:
	SfxSample* snd_kaarg[10];
};
ASSERT_SIZE(VisTavKaarg, 0x168);


// VTable at 60da00
class VisSpellBook : public CVisualObject {
public:
	virtual ~VisSpellBook(); // 4CB020 (client: 4cf9a0)

	virtual const char* GetHint() override; // 4C9A6F (client: 4ce3ff)
	virtual void VMethod7() override; // 4CA2A1 (client: 4cec31)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4CAAD7 (client: 4cf465)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4CAF8C (client: 4cf912)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4CAFF5 (client: 4cf97b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4CAD6A (client: 4cf6f0)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4CAEA6 (client: 4cf82c)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4CADEE (client: 4cf774)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4CADD2
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4CAF70
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4cae94 (client: 4cf81a)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CAC9B (client: 4cf621)

	void FUN_004caa69(); // client: 4cf3f7 (client: 4cf3f7)

	int32_t FUN_004ca7c2(CPoint* pos); //4ca7c2 (client: 4cf150) in asm
	void sub_4CA925(int32_t idx); //4ca925 (client: 4cf2b3) in asm
	int32_t sub_4CA89B(int32_t id); //4ca89b
	int32_t sub_4CA8E0(int32_t id); //4ca8e0
	int32_t sub_41F9B0(); //41f9b0 (client: 4208e0)
	int32_t sub_41F9E0(); //41f9e0 (client: 420910)
	int32_t sub_4CAAA7(); //4caaa7 (client: 4cf435)

	VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4c99c7 (client: 4ce357)

public:
	int32_t field_0x5c;
	int32_t pressed;
	int32_t selected;
	int32_t spell;
};
ASSERT_SIZE(VisSpellBook, 0x6c);


//60d480
class VisShopCompass : public CVisualObject {
public:
	virtual ~VisShopCompass(); //4be1df (client: 4c2ba4) in asm
	virtual const char* GetHint() override; //4bea64 (client: 4c3429) in asm
	virtual void VMethod7() override; //4beb77 (client: 4c353c) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; //4bf6ce (client: 4c4091) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4bf748 (client: 4c410b) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; //4bfa01 (client: 4c43bc) in asm

	virtual void VMethod26(); // 4be238 (client: 4c2bfd)
	virtual void VMethod27(); // 4be372 (client: 4c2d37)
	virtual void VMethod28(); // 4be5de
	virtual void VMethod29(); // 4be6df
	virtual void VMethod30(); // 4be7e0 (client: 4c31a5)
	virtual void VMethod31(int32_t arg); // 4be45d (client: 4c2e22)
	virtual void VMethod32(int32_t arg); // 4be541 (client: 4c2f06)
	virtual void VMethod33(); // 4be8b0
	virtual void VMethod34(); // 4be92d
	virtual void VMethod35(); // 4be9aa
	virtual void VMethod36(); // 4bea01 (client: 4c33c6)
	virtual int32_t VMethod37(int32_t arg); // 4bf4e8 (client: 4c3eab)
	virtual int32_t VMethod38(); // 4c6bc0

	VisShopCompass(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4bd495 (client: 4c1e5a)

	void sub_4BF63F(int32_t dir); // 4bf63f (client: 4c4002)

public:
	VisShop* shop; // 0x5c
	CRect outer_rects[4]; // 0x60
	CRect inner_rects[4]; // 0xa0
	CRect center_rect; // 0xe0
	CRect confirm_rect; // 0xf0
	CRect confirm_yes_rect; // 0x100
	CRect confirm_no_rect; // 0x110
	CSprite256* frame_sprite; // 0x120
	CBmp64* bkg_bmp; // 0x124
	CBmp64* idle_bmp; // 0x128
	CBmp64* trigger_bmp; // 0x12c
	CBmp64* direction_frames[4][11]; // 0x130
	CBmp64* fwd_frames[12]; // 0x1e0
	CBmp64* ret_frames[12]; // 0x210
	int32_t state; // 0x240
	int32_t dir_frm[4]; // 0x244
	int32_t center_frm; // 0x254
};
ASSERT_SIZE(VisShopCompass, 0x258);


//60d658
class VisShopCompassDruid : public VisShopCompass {
public:
	virtual ~VisShopCompassDruid(); // 4C6A30

	virtual const char* GetHint() override; // 4C3690 (client: 4c8047)
	virtual void VMethod7() override; // 4C30B4 (client: 4c7a6d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4C359D (client: 4c7f56)
	virtual void VMethod26() override; // 4C2882 (client: 4c723b)
	virtual void VMethod27() override; // 4C2B29 (client: 4c74e2)
	virtual void VMethod28() override; // 4C2D8A
	virtual void VMethod29() override; // 4C2DA3
	virtual void VMethod30() override; // 4C2DBC (client: 4c7775)
	virtual void VMethod31(int32_t arg) override; // 4C2CCE (client: 4c7687)
	virtual void VMethod32(int32_t arg) override; // 4C2CED (client: 4c76a6)
	virtual void VMethod33() override; // 4C2F00
	virtual void VMethod34() override; // 4C2F7D
	virtual void VMethod35() override; // 4C2FFA (client: 4c79b3)
	virtual void VMethod36() override; // 4C3051 (client: 4c7a0a)
	virtual int32_t VMethod37(int32_t arg) override; // 4C34E6 (client: 4c7e9f)

	VisShopCompassDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c247b (client: 4c6e34) in asm

	int32_t FUN_004c2842(); // 4c2842 (client: 4c71fb)

public:
	int32_t field_0x258; // 0x258
	CBmp64* armor_bmp; // 0x25c
	CBmp64* magic_bmp; // 0x260
	CBmp64* potions_bmp; // 0x264
	CBmp64* sel_goods_bmp; // 0x268
	CBmp64* category_bmp; // 0x26c
};
ASSERT_SIZE(VisShopCompassDruid, 0x270);


//60d7a0
class VisShopCompassKaarg : public VisShopCompass {
public:
	virtual ~VisShopCompassKaarg(); // 4C6AD0

	virtual const char* GetHint() override; // 4C6800 (client: 4cb19b)
	virtual void VMethod7() override; // 4C608E (client: 4caa2b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4C671F (client: 4cb0bc)
	virtual void VMethod26() override; // 4C4A78 (client: 4c942f)
	virtual void VMethod27() override; // 4C5A80 (client: 4ca41d)
	virtual void VMethod28() override; // 4C5D64
	virtual void VMethod29() override; // 4C5D7D
	virtual void VMethod30() override; // 4C5D96 (client: 4ca733)
	virtual void VMethod31(int32_t arg) override; // 4C5CA8 (client: 4ca645)
	virtual void VMethod32(int32_t arg) override; // 4C5CC7 (client: 4ca664)
	virtual void VMethod33() override; // 4C5EDA
	virtual void VMethod34() override; // 4C5F57
	virtual void VMethod35() override; // 4C5FD4 (client: 4ca971)
	virtual void VMethod36() override; // 4C602B (client: 4ca9c8)
	virtual int32_t VMethod37(int32_t arg) override; // 4C65BA (client: 4caf57)

	VisShopCompassKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c4693 (client: 4c904a) in asm

public:
	int32_t field_0x258; // 0x258
	CBmp64* armor_bmp; // 0x25c
	CBmp64* magic_bmp; // 0x260
	CBmp64* scrolls_bmp; // 0x264
	CBmp64* weapons_bmp; // 0x268
	CBmp64* tail_frames[18]; // 0x26c
	CBmp64* wpn_tail_frames[18]; // 0x2b4
	int tail_frame; // 0x2fc
	int wpn_tail_frame; // 0x300
};
ASSERT_SIZE(VisShopCompassKaarg, 0x304);


//60d540
class VisShopButtons : public CVisualObject {
public:
	virtual ~VisShopButtons(); //4c0020 (client: 4c49db)
	virtual const char* GetHint() override { return nullptr; } //4c04a4
	virtual void VMethod7() override; //4c04b1 (client: 4c4e6a)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; //4c10c4 (client: 4c5a7d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4c1134 (client: 4c5aed)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; //4c11c2 (client: 4c5b7b)
	virtual int32_t VMethod30(); //4c6bd0

	void ReleaseBmp(); //4c0352 (client: 4c4d0b)
	void UpdateHoveredState(uint32_t wparam, CPoint pos); //4c14c4 (client: 4c5e7d)
	int32_t GetButtonAt(CPoint pos); //4c137d (client: 4c5d36)
	void LoadBmp(); //4c0088 (client: 4c4a43)
	void ResetSelected(); //4c1358

	VisShopButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4bfa5a (client: 4c4415)

public:
	VisShop* shop; // 0x5c
	int32_t field_0x60;
	CBmp64* button_bmps[4]; // 0x64
	CBmp64* menu_bmp; // 0x74
	CRect outer_rects[4]; // 0x78
	CRect inner_rects[4]; // 0xb8
	int32_t pressed_btn; // 0xf8
	int32_t hovered_btn; // 0xfc
};
ASSERT_SIZE(VisShopButtons, 0x100);


void __cdecl FUN_004ba1cc(CArray<TokenEntry*>* arr); //4ba1cc in asm
void __cdecl FUN_00476987(CString* str); //476987 - insert thousands separators


//60d3e8
class VisShop : public VisScreen {
public:
	virtual ~VisShop(); // 4BA5A0 (client: 4bef70)

	virtual void VMethod8(CRect* rect) override; // 4BB79A (client: 4c0167)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4BA892 (client: 4bf262)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4BADCB (client: 4bf798)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4BB045 (client: 4bfa12)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4BA832 (client: 4bf202)

	virtual void VMethod26() override; // 4BBD75 (client: 4c0742)
	virtual void VMethod28() override; // 4BC32B (client: 4c0cf8)
	virtual void DoClose(uint32_t code) override; // 4BC0E7 (client: 4c0ab4)

	virtual void VMethod30(); // 4BCF2F (client: 4c18f4)
	virtual void VMethod31(); // 4BD0FB (client: 4c1ac0)
	virtual void VMethod32();
	virtual CString VMethod33(); // 4c6b90 (client: 4cb530)


	void FUN_004bcd02(); // client: 4c16d3 (client: 4c16d3)
	void sub_4BAD1D(); //4bad1d (client: 4bf6ea) in asm
	void sub_4BB4FB(); //4bb4fb (client: 4bfec8)
	void sub_4BB895(); //4bb895 (client: 4c0262)
	void sub_4BBA06(); //4bba06 (client: 4c03d3)
	void sub_4BB102(); //4bb102 (client: 4bfacf)
	void sub_4BBBD6(int32_t category, CArray<TokenEntry*>* arr); //4bbbd6
	void sub_4BC8D7(); //4bc8d7 (client: 4c12a4)
	void sub_4BCC38(); //4bcc38
	void sub_4BC97B(); //4bc97b (client: 4c1348)
	void sub_4BCD4B(); //4bcd4b (client: 4c171c)
	void sub_4BCD79(); //4bcd79 (client: 4c174a)
	void sub_4BCDA0(); //4bcda0 (client: 4c1771)
	void sub_4BCEA4(); //4bcea4 (client: 4c1875)
	void sub_4BCAF1(); //4bcaf1 (client: 4c14bd)
	int sub_4BCB63(); //4bcb63 (client: 4c152f)
	void sub_4BCCE1(); //4bcce1 (client: 4c16b2)

	VisShop(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4ba342 (client: 4bed12) in asm

public:
	VisInvExtType1* assortiment;
	VisInvExtType2* to_sell;
	VisInvExtType3* to_buy;
	VisShopCompass* shop_compass;
	VisShopButtons* buttons;
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
class VisShopDruid : public VisShop {
public:
	virtual ~VisShopDruid(); // 4C69B0

	virtual void VMethod26() override; // 4C15E6 (client: 4c5f9f)
	virtual void VMethod28() override; // 4C1E5C (client: 4c6815)
	virtual void VMethod30() override; // 4C1958 (client: 4c6311)
	virtual void VMethod31() override; // 4C1B8B (client: 4c6544)
	virtual void VMethod32() override; // 4C1D25 (client: 4c66de)
	virtual CString VMethod33() override; // 4C6BE0 (client: 4cb580)

	VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c15ad
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
class VisShopKaarg : public VisShop {
public:
	virtual ~VisShopKaarg(); // 4C6A50

	virtual void VMethod26() override; // 4C37FE (client: 4c81b5)
	virtual void VMethod28() override; // 4C3B70 (client: 4c8527)
	virtual void VMethod30() override; // 4C4170 (client: 4c8b27)
	virtual void VMethod31() override; // 4C43A3 (client: 4c8d5a)
	virtual void VMethod32() override; // 4C453D (client: 4c8ef4)
	virtual CString VMethod33() override; // 4C6C10 (client: 4cb5b0)

	VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c37c5
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
class VisTown : public VisScreen {
public:
	virtual ~VisTown(); // 4D6B80

	virtual const char* GetHint() override; // 4D1602 (client: 4d5f33)
	virtual void VMethod7() override; // 4CE886 (client: 4d31cd)
	virtual void VMethod8(CRect* rect) override; // 4ce719
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4CE3B8 (client: 4d2d05)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4CE4E5 (client: 4d2e32)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4CE50A (client: 4d2e57)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CE4AF (client: 4d2dfc)

	virtual void VMethod26() override; // 4CDDAD (client: 4d26fd)
	virtual void VMethod28() override; // 4CE002 (client: 4d2952)
	virtual void DoClose(uint32_t code) override; // 4CE310 (client: 4d2c5d)

	virtual void VMethod30(); // 4D1723 (client: 4d6054)
	virtual void VMethod31(); // 4D184A (client: 4d617b)

	virtual void VMethod32(); // 4D1709
	virtual int32_t VMethod33(CPoint pos); // 4D10F0 (client: 4d5a27)
	virtual void VMethod34(CPoint pos); // 4D1429 (client: 4d5d5a)
	virtual void VMethod35(); // 4D12CB (client: 4d5c02)
	virtual void VMethod36(); // 4CEEEF (client: 4d3836)
	virtual void VMethod37(); // 4CFD0E (client: 4d4655)
	virtual void VMethod38(); // 4D0E31 (client: 4d5776)
	virtual void VMethod39(); // 4D15E3 (client: 4d5f14)

	int32_t FUN_004ce64b(); //4ce64b (client: 4d2f98) in asm
	void FUN_004ce726(); //4ce726 (client: 4d3070) in asm

	void FUN_004d06aa(); //4d06aa (client: 4d4ff1) in asm
	void FUN_004d07e8(); //4d07e8 (client: 4d512f) in asm
	void FUN_004d0884(); //4d0884 in asm
	void FUN_004d0913(); //4d0913 in asm
	void FUN_004d0ad7(); //4d0ad7 (client: 4d541c) in asm
	void FUN_004d0b95(); //4d0b95 in asm
	void FUN_004d0c6e(); //4d0c6e in asm
	void FUN_004d0d47(); //4d0d47 (client: 4d568c) in asm
	void FUN_004d0da2(); //4d0da2 in asm
	void FUN_004d0f66(); //4d0f66 in asm
	void FUN_004d1002(); //4d1002 in asm
	void FUN_004d109e(); //4d109e (client: 4d59d5) in asm

	VisTown(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cda5c (client: 4d23ac) in asm
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
	CArray<CA16*> spr_birds;
	int32_t bird_frame0;
	int32_t bird_frame1;
	int32_t bird_frame2;
	CA16* spr_guard;
	int32_t guard_frame;
	int32_t guard_frame_step; // signed: compared against -1
	int32_t guard_sound;
	CA16* spr_cur_bbird;
	CArray<CA16*> spr_bbird;
	CPoint bbird_position;
	int32_t bbird_frame;
	uint32_t bbird_last_tick;
	int32_t bbird_delay;
	CA16* spr_cur_horse;
	CArray<CA16*> spr_horse;
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
class VisTownDruid : public VisTown {
public:
	virtual ~VisTownDruid(); // 4D6BB0

	virtual void VMethod7() override; // 4D2E99 (client: 4d77ca)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D3F18 (client: 4d883c)
	virtual void VMethod28() override; // 4D1E80 (client: 4d67b1)
	virtual void VMethod30() override; // 4D2AFF (client: 4d7430)
	virtual void VMethod31() override; // 4D2D1B (client: 4d764c)
	virtual void VMethod32() override; // 4D2AE3
	virtual void VMethod34(CPoint pos) override; // 4D359C (client: 4d7ec8)
	virtual void VMethod35() override; // 4D3922 (client: 4d824e)
	virtual void VMethod36() override; // 4D2153 (client: 4d6a84)
	virtual void VMethod37() override; // 4D27FF (client: 4d7130)
	virtual void VMethod38() override; // 4D3EA7 (client: 4d87cb)

	void FUN_004d3384(); //4d3384 (client: 4d7cb5) in asm
	void FUN_004d3435(); //4d3435 (client: 4d7d66) in asm
	void FUN_004d34b6(); //4d34b6 (client: 4d7de7) in asm
	void FUN_004d3520(); //4d3520 (client: 4d7e4e) in asm

	VisTownDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d1af8 (client: 4d6429) in asm
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
class VisTownKaarg : public VisTown {
public:
	virtual ~VisTownKaarg(); // 4D6C60

	virtual const char* GetHint() override; // 4D6A1C (client: 4db32d)
	virtual void VMethod7() override; // 4D57B9 (client: 4da0dd)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D6802 (client: 4db113)
	virtual void VMethod28() override; // 4D4592 (client: 4d8eb6)
	virtual void VMethod30() override; // 4D54CF (client: 4d9df3)
	virtual void VMethod31() override; // 4D5682
	virtual void VMethod32() override; // 4D54B5
	virtual void VMethod34(CPoint pos) override; // 4D5C9A (client: 4da5be)
	virtual void VMethod35() override; // 4D5EF0 (client: 4da814)
	virtual void VMethod36() override; // 4D4866 (client: 4d918a)
	virtual void VMethod37() override; // 4D5112 (client: 4d9a36)
	virtual void VMethod38() override; // 4D6786 (client: 4db097)

	void FUN_004d6480(); //4d6480 in asm
	void FUN_004d65dd(); //4d65dd in asm
	void FUN_004d6655(); //4d6655 in asm
	void FUN_004d66ca(); //4d66ca in asm
	void FUN_004d6728(); //4d6728 in asm

	VisTownKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4d423a (client: 4d8b5e) in asm
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


// VTable at 609b88
class VisCredits : public VisScreen {
public:
	virtual ~VisCredits(); // 43c3c6 (client: 43e7d6)

	virtual void VMethod7() override; // 43C60C (client: 43ea1c)
	virtual void VMethod8(CRect* rect) override; // 43c9a8
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43C595
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 43C5F5 (client: 43ea05)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43C5D4

	virtual void VMethod26() override; // 43C438 (client: 43e848)
	virtual void VMethod28() override; // 43C4D6 (client: 43e8e6)
	virtual void DoClose(uint32_t code) override; // 43C553 (client: 43e963)

	void LoadContent(); // 43c9b5 (client: 43edc5)
	void FreeContent(); // 43cb77 (client: 43ef87)

	VisCredits(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43c337 (client: 43e747) in asm
public:
	CMapStringToOb bitmaps;
	TxtFile text;
	uint32_t flag;
	int32_t first_visible;
	int32_t scroll;
};
ASSERT_SIZE(VisCredits, 0xa0);

class VisCharGen;
class VisTipsDialog;

//6094c8
class VisCharGenStats : public CVisualObject {
public:
	virtual ~VisCharGenStats(); // 428f12 in asm

	virtual const char* GetHint() override; // 42a43b (client: 42c351) in asm
	virtual void VMethod7() override; // 429d03 (client: 42bc1b) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42a312 in asm
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 42a3f2 (client: 42c308) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42a335 (client: 42c24b) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42a3a8 (client: 42c2be) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 42a3ce in asm

	VisCharGenStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //428e51 (client: 42ad71) in asm

	void ReadUnitStats(); // 429c39 in asm
	void LoadBitmaps(); // 429475 (client: 42b395) in asm
	void FreeBitmaps(); // 4298b6 (client: 42b7d6) in asm
	uint32_t HandleClick(int32_t mode, CPoint pt); // 42a161 (client: 42c077) in asm
private:
	int32_t StatUpCost(int32_t stat); // 42a96d (client: 42c883) in asm
	int32_t StatDownRefund(int32_t stat); // 42a99d (client: 42c8b3) in asm
	int32_t HitTest(CPoint pt); // 42a02a (client: 42bf42) in asm
	int32_t OnStatUp(int32_t idx); // 42a778 (client: 42c68e) in asm
	int32_t OnStatDown(int32_t idx); // 42a87b (client: 42c791) in asm
	void Init(); // 428f7f (client: 42ae9f) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp; // 0x60
	CRect field_0x64[4];
	CRect field_0xa4;
	CRect areas[12]; // 0xb4
	CBmp64* field_0x174[8]; // 0x174: 4 x [plus, minus] button bitmap pairs
	CBmp64* field_0x194[10]; // 0x194: plon/ploff/pnlon/pnloff/pdisable/mlon/mloff/mnlon/mnloff/mdisable
	CStringArray texts; // 0x1bc
	int32_t stat_body; // 0x1d0
	int32_t stat_reaction; // 0x1d4
	int32_t stat_mind; // 0x1d8
	int32_t stat_spirit; // 0x1dc
	int32_t field_0x1e0[4];
	int32_t field_0x1f0;
};
ASSERT_SIZE(VisCharGenStats, 0x1f4);

//609540
class VisCharGenFullStats : public CVisualObject {
public:
	virtual ~VisCharGenFullStats(); // 42aaa0 (client: 42c9b6) in asm

	virtual const char* GetHint() override; // 42ac85 (client: 42cb9b) in asm
	virtual void VMethod7() override; // 42abea (client: 42cb00) in asm

	VisCharGenFullStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42aa2b (client: 42c941) in asm

	void LoadBitmaps(); // 42ab18 (client: 42ca2e) in asm
	void FreeBitmaps(); // 42ab9c in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp; // 0x60
	uint8_t field_0x64[0x44];
};
ASSERT_SIZE(VisCharGenFullStats, 0xa8);

//6095b8
class VisCharGenAction : public CVisualObject {
public:
	virtual ~VisCharGenAction(); // 42ae19 in asm

	virtual void VMethod7() override; // 42b028 (client: 42cf3e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42b2e0 in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42b303 (client: 42d219) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42b392 (client: 42d2a8) in asm

	VisCharGenAction(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42ad7d in asm

	void LoadBitmaps(); // 42b57e (client: 42d494) in asm
	void FreeBitmaps(); // 42b7ee in asm
	void ResetMouseBoxes(); // 42b455 in asm
private:
	int32_t HitTest(CPoint pt); // 42b47a (client: 42d390) in asm
	void UpdateMouseOver(uint32_t wparam, CPoint pos); // 42b505 in asm
	void Init(); // 42ae83 in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CStringArray texts; // 0x60
	CBmp64* btn_on[3]; // 0x74
	CBmp64* btn_off[3]; // 0x80
	CBmp64* area_bmp; // 0x8c
	CRect areas[3]; // 0x90
	int32_t mouse_down_box; // 0xc0
	int32_t mouse_over_box; // 0xc4
};
ASSERT_SIZE(VisCharGenAction, 0xc8);

//609630
class VisCharGenSkills : public CVisualObject {
public:
	virtual ~VisCharGenSkills(); // 42ba30 in asm

	virtual const char* GetHint() override; // 42dbce (client: 42fae2) in asm
	virtual void VMethod7() override; // 42bb5a (client: 42da70) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42bfcc (client: 42dee1) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42c072 (client: 42df86) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42c1ab in asm (Ghidra-labeled accHitTest)

	VisCharGenSkills(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42b98c (client: 42d8a2) in asm

	void LoadBitmaps(uint32_t mage_flag); // 42c1cc (client: 42e0e0) in asm
	void LoadSamples(uint32_t mage_flag); // 42d963 (client: 42f877) in asm
	void FreeBitmaps(); // 42d6ed (client: 42f601) in asm
	void FreeSamples(); // 42da5a (client: 42f96e) in asm
private:
	int32_t HitTest(CPoint pt); // 42dadf (client: 42f9f3) in asm
	void DrawBlinkSlot(); // 42bd41 (client: 42dc56) in asm
	void Init(); // 42ba81 (client: 42d997) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp_column; // 0x60: "chrgen\<class>\column.bmp", background drawn in VMethod7 (owned)
	CBmp256* bmp_mask; // 0x64: "chrgen\<class>\mask.bmp", HitTest color-key source (owned)
	CBmp64* bmp_roll_stats; // 0x68: "chrgen\\RollStatsR.bmp" (owned, never drawn)
	CBmp64* bmp_full_stats; // 0x6c: "chrgen\\FullStatsR.bmp" (owned, never drawn)
	CBmp64* bmp_ru_over; // 0x70: "inn\\RUOver.bmp" (owned, never drawn)
	CBmp64* bmp_human_back; // 0x74: alias to g_bmp_humanbackl (not owned; vanilla never frees it)
	CBmp64* bmp_on[5]; // 0x78
	CBmp64* bmp_shine_off[5]; // 0x8c
	CBmp64* bmp_shine_on[5]; // 0xa0
	CPoint field_0xb4[5]; // 0xb4: slot positions (parent-screen coords)
	CPoint field_0xdc[5]; // 0xdc: slot sizes
	uint8_t color_keys[8]; // 0x104: per-slot mask color keys
	int32_t field_0x10c[5]; // 0x10c: per-slot state (bit0 selected, bit1 hover)
	SfxSample* field_0x120[5]; // 0x120: skill sounds
	int32_t selected_slot; // 0x134
};
ASSERT_SIZE(VisCharGenSkills, 0x138);

//6096a8
class VisCharGen : public VisScreen {
public:
	virtual ~VisCharGen(); // 4383F0

	virtual void VMethod7() override; // 42EB29 (client: 430a27)
	virtual void VMethod8(CRect* rect) override; // 42eb7f
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 42E058 (client: 42ff6c)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42E196
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42EA01 (client: 4308ff)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 42EA4F

	virtual void VMethod26() override; // 42DDEC (client: 42fd00)
	virtual void VMethod28() override; // 42E218 (client: 43012c)
	virtual void DoClose(uint32_t code) override; // 42E7BC (client: 4306d0)

	VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //42dcd9 (client: 42fbed) in asm

	void OnPrevFace(); // 42edc2 (client: 430cbf) in asm
	void OnNextFace(); // 42ebe0 (client: 430ade) in asm
	void RollStats(); // 42e92a in asm
	void OnClickFwd(); // 42eb8c in asm
	void OnClickBack(); // 42ebab in asm
	void ShowTipHint(); // 42ea87 (client: 430985) in asm
	void LoadSamples(); // 42efb4 in asm
	void FreeSamples(); // 42f021 in asm
public:
	VisCharInfo* info_panel;
	BigStruct2* map_visual;
	VisCharGenStats* stats_panel;
	VisCharGenFullStats* fullstats_panel;
	VisCharGenAction* action_panel;
	VisCharGenSkills* skills_panel;
	VisTipsDialog* tips;
	SfxSample* snd_stat;
	SfxSample* snd_ok;
	SfxSample* snd_reset;
	SfxSample* snd_back;
	CUnit* current_char;
	int32_t fwd_btn;
	int32_t field14_0x9c;
	CArray<uint16_t> face_sets[4];
	uint32_t mage_flag;
	int32_t female_face;
	int32_t selected_face;
	int32_t selected_face_set;
	int32_t tips_step;
	uint32_t active_flag;
};
ASSERT_SIZE(VisCharGen, 0x108);




//609a58 - console-style input line used by Vis1200
class Vis1200obj : public CVisualObject {
public:
	virtual ~Vis1200obj();

	virtual void VMethod7() override; // 43b32f (client: 43d73e)
	virtual void WriteData(void* buf) override; // 43ad35 (client: 43d144)
	virtual uint32_t DataSize() override; // 43c020
	virtual void ReadData(const void* buf) override; // 43acca (client: 43d0d9)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43b283 (client: 43d692)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43b159 (client: 43d564)
	virtual int32_t OnChar(uint32_t wparam) override; // 43b240

	void FUN_0043ae1c(uint8_t ch); // 43ae1c
	void FUN_0043b03a(); // 43b03a (client: 43d445)
	void FUN_0043b724(); // 43b724 (client: 43db33)
	void FUN_0043ba75(); // 43ba75 (client: 43de84)
	void FUN_0043b504(CString* str); // 43b504 (client: 43d913)

	Vis1200obj(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); //43ab28 (client: 43cf38)

	void FUN_0043ac8c(); //43ac8c (client: 43d09b) - reset input/history state
	void FUN_0043ad86(); //43ad86 (client: 43d195) - commit current line to history
	void FUN_0043ada3(); //43ada3 (client: 43d1b2) - recall previous line from history
	int32_t FUN_004972f0(); //4972f0
	int32_t FUN_00497330(); //497330

public:
	CStringArray field_0x5c;
	CStringArray field_0x70; // history
	CString field_0x84; // current line
	CGameFont* font; // 0x88
	uint16_t* clr; // 0x8c
	int32_t field_0x90; // 0x90
	int32_t field_0x94; // 0x94
	uint32_t field_0x98; // 0x98 - last input tick
	int32_t field_0x9c; // 0x9c
	int32_t field_0xa0; // 0xa0
};
ASSERT_SIZE(Vis1200obj, 0xa4);

	int32_t FUN_0043aae0(char ch); // 43aae0 in asm - chat separator test (' ', ',', '.', ':', ';')
	void FUN_00476236(CString* str); // 476236 - lowercase respecting DOS CP

//609ad0
class Vis1200 : public VisScreen {
public:
	virtual ~Vis1200(); // 43c000

	virtual void VMethod7() override; // 43BE80 (client: 43e28f)
	virtual void VMethod8(CRect* rect) override; // 43be73
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43bdcb
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43BDEC (client: 43e1fb)

	virtual void VMethod26() override; // 43BC3C (client: 43e04b)
	virtual void VMethod28() override; // 43BD2E (client: 43e13d)
	virtual void DoClose(uint32_t code) override; // 43BD96 (client: 43e1a5)

	int32_t FUN_004972d0(); //4972d0
	int32_t FUN_00497310(); //497310
	void FUN_0043be9f(); //43be9f (client: 43e2ae)

	Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 43bb70 in asm

public:
	char buffer[256];
	Vis1200obj* text_block;
	uint32_t active;
};
ASSERT_SIZE(Vis1200, 0x170);



//60a170
class VisDropGold : public VisScreen {
public:
	virtual ~VisDropGold(); // 441c63 (44F870 is the ??_G thunk)

	virtual void VMethod9() override; // 441F1F (client: 4444f1)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 441F3C (client: 44450e)

	virtual void VMethod26() override; // 441C7F (client: 44424e)
	virtual void VMethod28() override; // 441FA3 (client: 444575)
	virtual void DoClose(uint32_t code) override; // 441FEE (client: 4445c0)

	void FUN_004a7a30(int32_t idx); //4a7a30 in asm

	VisDropGold(int32_t _id, int32_t _x, int32_t _y); // 441bec (client: 4441bb) in asm
public:
	int32_t selection;
	char amount_text[100];
};
ASSERT_SIZE(VisDropGold, 0xd0);




//60cfe8
class VisCharInfo : public CVisualObject {
public:
	virtual ~VisCharInfo(); // 4B47B0

	virtual const char* GetHint() override; // 4B19B0 (client: 4b62e2)
	virtual void VMethod7() override; // 4B2B3D (client: 4b746f)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B201C (client: 4b694e)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B289E (client: 4b71d0)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b2215
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B2346 (client: 4b6c78)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B2239 (client: 4b6b6b)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b2227
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B2857 (client: 4b7189)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b2334
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4B2AD5 (client: 4b7407)

	virtual int32_t VMethod26(int32_t a); //4b38af
	virtual int32_t VMethod27(int32_t a); //4b3a0d
	virtual int32_t VMethod28(); // 4b4830

	void sub_4B36B4(); //4b36b4 (client: 4b7fe4)

	VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b16c3 (client: 4b5ff5) in asm
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
class VisSideStatus : public CVisualObject {
public:
	virtual ~VisSideStatus(); // 4B4810

	virtual const char* GetHint() override; // 4B45BB (client: 4b8f8a)
	virtual void VMethod7() override; // 4B4190 (client: 4b8b61)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B4490 (client: 4b8e5f)

	void UpdateCursor(); //4b4019 (client: 4b89ea) in asm

	VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b3fa3 (client: 4b8974) in asm
public:
	uint32_t dirty;
};
ASSERT_SIZE(VisSideStatus, 0x60);


//60cd30
class VisMainMenu : public VisScreen {
public:
	virtual ~VisMainMenu(); // 4AAAB6 (client: 4aeae6)

	virtual void VMethod7() override; // 4AB28F (client: 4af2bf)
	virtual void VMethod8(CRect* rect) override; // 4ab411
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AB41E
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4AB88C
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4AB8AF (client: 4af8df)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4AB8F3 (client: 4af923)

	virtual void VMethod26() override; // 4AAB71 (client: 4aeba1)
	virtual void VMethod28() override; // 4ABA8D (client: 4afabd)
	virtual void DoClose(uint32_t code) override; // 4ABB14 (client: 4afb44)

	void UpdateButtonState(uint32_t wparam, CPoint mouse); // 4ab45d (client: 4af48d)
	void LoadGraphics(); // 4aada7 (client: 4aedd7)
	void LoadSfx(); // 4abb5e (client: 4afb8e)
	void FreeGraphics(); // 4ab07b (client: 4af0ab)
	void FreeSfx(); // 4abb87

	VisMainMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aa926 (client: 4ae956) in asm
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
	int32_t over_button;
	int32_t active_button;
	int32_t pressed_button;
	uint32_t disable_mask;
	CA16* sprite;
};
ASSERT_SIZE(VisMainMenu, 0x100);




class VisStartGame;

class VisStartGameTextBox : public CVisualObject {
public:
	VisStartGameTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisStartGame* screen); // 4329f9 (client: 434b2a)

	virtual const char* GetHint() override; // 432EA3 (client: 434fdb)
	virtual void VMethod7() override; // 432D58 (client: 434e90)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 432C1F (client: 434d56)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 432C9E (client: 434dd5)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 432CE3
	virtual int32_t OnChar(uint32_t wparam) override; // 432D1B (client: 434e52)

	CString GetText(); // 438d80 (client: 43add0)
	void SetText(const CString& text); // 432c03

	void FUN_00432b80(); //432b80 (client: 434cb7) in asm
	void FUN_00432b33(uint8_t ch); //432b33 in asm
	void FUN_00432ad1(); //432ad1 (client: 434c02) in asm

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
class VisStartGame : public VisScreen {
public:
	virtual ~VisStartGame(); // 4386B0

	virtual const char* GetHint() override; // 437E13 (client: 439f45)
	virtual void VMethod7() override; // 435D72 (client: 437ea4)
	virtual void VMethod8(CRect* rect) override; // 438dc0
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 437571 (client: 4396a3)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 437664 (client: 439796)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 43769C (client: 4397ce)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 437617 (client: 439749)

	virtual void VMethod26() override; // 4333B9 (client: 4354f1)
	virtual void VMethod28() override; // 433E2D (client: 435f65)
	virtual void DoClose(uint32_t code) override; // 4341EB (client: 436323)

	uint32_t GetHotspotId(int32_t x, int32_t y); // 4356c2 (client: 4377fa)
	uint32_t DrawTipsHighlight(uint32_t hotspot); // 436e4b (client: 438f7d)
	uint32_t UpdateHotspots(int32_t x, int32_t y, uint32_t pressed); // 435867 (client: 43799b)
	void UpdateTipsProgress(uint32_t hotspot); // 437c25 (client: 439d57)
	void Accept(); // 43817d (client: 43a2af)
	void Cancel(); // 4382a9 (client: 43a5a4)

	void FUN_004382d9(); //4382d9 (client: 43a5d4) in asm

	void ClearDifficultySelectedFlags(); // 435743
	void ClearPortraitSelectedFlags(); // 43578c
	void ClearDifficultyHoverFlags(); // 4357d5
	void ClearPortraitHoverFlags(); // 43581e

	void LoadBitmaps(); // 43438b (client: 4364c3)
	void LoadSamples(); // 437ff4 (client: 43a126)
	void FreeSamples(); // 4380d3 (client: 43a205)
	void ResetHoverState(); // 435d26 (client: 437e58)
	void FreeBitmaps(); // 435097 (client: 4371cf)

	VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43305b (client: 435193) in asm

	CString GetName() { return this->char_name; } // 4973f0
	CString GetClan() { return this->clan_name; } // 497430
	int32_t GetSelectedDifficulty() { return this->selectedDifficultyIndex; } //4973d0
	void SetSelectedDifficulty(int diff) { this->selectedDifficultyIndex = diff; } //497370
	int32_t GetSelectedPortraitPlayerType() { return DWORD_0060bd60[this->selectedPortraitIndex]; } //4973b0
	void SetSelectedPortraitPlayerType(int id) { this->selectedPortraitIndex = id; } //497350
	void SetCharName(const CString& name) { this->char_name = name; } //497390

public:
	CBmp64* mainAreaBitmap; //0x68
	CBmp256* hotspotMaskBitmap; // 0x6c;
	CA16* blindAnimation; // 0x70;
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
class VisMenuWnd : public VisWindow {
public:
	virtual ~VisMenuWnd();

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440e54 (client: 4432bb)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //440ece (client: 443335)

	VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect &_r); //451990 (client: 453430)

	void AddElement(CVisualObject *obj, int32_t height); //440e11 (client: 443278)
public:
	uint32_t field_0x68;
	CRect field_0x6c;
};
ASSERT_SIZE(VisMenuWnd, 0x7c);


//609f50
class IngameMenu : public VisMenuWnd {
public:
	IngameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect& _r); //451990 (client: 453430)
};
ASSERT_SIZE(IngameMenu, 0x7c);


//60ae78
class MenuButton : public VisButton {
public:
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; //4dd933 (client: 4e22b6)
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } //4dda31
	virtual int32_t OnChar(uint32_t wparam); //4dd9b3

	MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //4509c0 (client: 453390)
	MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //450bf0
};
ASSERT_SIZE(MenuButton, 0x78);


//609dc8
class LoadGameWindow : public VisWindow {
public:
	virtual ~LoadGameWindow() {} //44f4b0
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43f63c (client: 441aa0)
	virtual void VMethod26() override; //43ed94 (client: 4411f8)

	LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43ecfb (client: 44115f)
public:
	SaveFileInfo* field_0x68;
	CStringArray file_names;
	CStringArray save_times;
	int32_t field_0x94;
};
ASSERT_SIZE(LoadGameWindow, 0x98);

//609e50
class SaveGameWindow : public VisWindow {
public:
	virtual ~SaveGameWindow() {} //44f540
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440726 (client: 442b8d)
	virtual void VMethod26() override; //43fb2c (client: 441f90)

	SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43fa42 (client: 441ea6)
	void CheckInput(); //440cc4 (client: 44312b)

	static int __cdecl Compare(void const* a, void const* b); //43faf4 (client: 441f58)
public:
	SaveFileInfo* field_0x68;
	CStringArray file_names;
	CStringArray save_times;
	int32_t field_0x94;
	char* field_0x98;
};
ASSERT_SIZE(SaveGameWindow, 0x9c);

//609ed8
class VisButtonSaveGame : public VisButton {
public:
	virtual ~VisButtonSaveGame(); //44f650

	virtual int32_t OnKeyDown(uint32_t wparam) override; //44f5f0 (client: 4532b0)

	VisButtonSaveGame(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //44f5a0
};
ASSERT_SIZE(VisButtonSaveGame, 0x78);


//609d38
class HintedListBox : public VisListBox {
public:
	virtual ~HintedListBox() {} //44f410

	virtual const char* GetHint() override; //43ec67 (client: 4410cb)

	HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); //43ec05

	void SetHints(CStringArray* names) { this->hints = names; } //43ec4e (client: 4410b2)
public:
	CStringArray* hints = nullptr;
};
ASSERT_SIZE(HintedListBox, 0x98);


//60a2000
class GameOptionsWindow : public VisWindow {
public:
	virtual ~GameOptionsWindow() {} //443cbb

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //443cd7 (client: 4462a9)
	virtual void VMethod26() override; //442646 (client: 444c18)

	GameOptionsWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b) : VisWindow(_id, l, t, r, b, nullptr) {}; //44260f
};
ASSERT_SIZE(GameOptionsWindow, 0x68);


class EndGameMenu : public VisMenuWnd {
public:
	EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2); //441357
};
ASSERT_SIZE(EndGameMenu, 0x7c);

//60a0e8
class ExitGameMenu : public VisMenuWnd {
public:
	ExitGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44186e (client: 443e3d)
};
ASSERT_SIZE(ExitGameMenu, 0x7c);


//60a060
class TownMenuListDialogVisualObject : public VisMenuWnd {
public:
	TownMenuListDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CRect& _r); //44160e (client: 443bdd)
};
ASSERT_SIZE(TownMenuListDialogVisualObject, 0x7c);


// Chat command handler for the server screen text box input ("#disconnect N", "#curse N"). 44e4ce
void sub_44E4CE(const char* cmd);

// Radio checkbox on the server screen ("save on server" toggle). 60adf8
class VisServerScreenRadio : public VisRadioType1 {
public:
	virtual ~VisServerScreenRadio(); // 450970 (complete dtor 4509a0)

	VisServerScreenRadio(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); // 450920

	virtual void VMethod7() override; // 4db3ad (client: 4dfd32) in asm
	virtual int32_t GetIndex(int32_t y) override; // 4db615 in asm
};
ASSERT_SIZE(VisServerScreenRadio, 0x8c);

//60aef0
class VisServerScreen : public VisScreen {
public:
	virtual ~VisServerScreen(); // 450AB0

	virtual void VMethod7() override; // 44DBE0 (client: 450f0b)
	virtual void VMethod8(CRect* rect) override; // 44DBAE (client: 450ed9)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44E7C8 (client: 451aed)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 44E6DE (client: 451a03)
	virtual void VMethod26() override; // 44D909 (client: 450c31)

	VisServerScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, GM_a28* log); // 44e469 (client: 451794)

public:
	GM_a28* msg_log;      // 0x68
	uint8_t gap_0x6c[8];
	int32_t scroll_pos;   // 0x74, first shown client row
};
ASSERT_SIZE(VisServerScreen, 0x78);


class GlobalMapRouteArray : public CWordArray {
public:
public:
	int32_t routeCost = 0;
};
ASSERT_SIZE(GlobalMapRouteArray, 0x18);

// Pathmap/graph builder state allocated by VisGlobalMap::VMethod28.
class GMapThing {
public:
	GMapThing(); // 473195 (client: 476ee5)
	~GMapThing(); // 4733cf (client: 47711f)
	void* TakeAdjacency(void* nodePoints); // 473b4a (client: 47789a)

	void LoadPathmap(); // 473508 (client: 477258)
	void FreePathmap(); // 4735e2
	void BuildAdjacency(); // 47362d (client: 47737d)
	int32_t FloodFill(int32_t x, int32_t y, uint16_t mark, CArray<CPoint>* foundPoints); // 47391f (client: 47766f)

public:
	CBmp256* pathmap; //0x0
	int32_t nodeCount; //0x4
	CArray<CPoint>*** adjacencyMatrix; //0x8
	CArray<CPoint> nodePoints; //0xc
	uint16_t visitedGrid[640][480]; //0x20
	uint8_t* bmp_data; // 0x96020
	CPoint floodPos; // 0x96024
};
ASSERT_SIZE(GMapThing, 0x9602c);

//60b760
class VisGlobalMap : public VisScreen {
public:
	virtual ~VisGlobalMap(); // 473D70

	virtual const char* GetHint() override; // 472729 (client: 476487)
	virtual void VMethod7() override; // 4714E7 (client: 475245)
	virtual void VMethod8(CRect* rect) override; // 471f08
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 472CC5 (client: 476a15)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 472D99 (client: 476ae9)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 473038 (client: 476d88)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 474f10
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 473077 (client: 476dc7)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 473175 (client: 476ec5)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4726E6
	virtual int32_t OnChar(uint32_t wparam) override; // 4726CC

	virtual void VMethod26() override; // 47031D (client: 47407b)
	virtual void VMethod28() override; // 47115D (client: 474ebb)
	virtual void DoClose(uint32_t code) override; // 471367 (client: 4750c5)

	void SetTravelOrigin(const CPoint& pt) { this->currentLocationPoint = pt; } //4972a0

	void RebuildScenarioLocations(); //47024a
	void PopulateScenarioLocationFlags(); //47025d
	void ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY); // 472820 (client: 47657e)
	void OnMapClick(); // 472dca (client: 476b1a)
	void UpdateHoveredLocation(CPoint pos); // 472d4b (client: 476a9b)
	void FreeBitmaps(); // 470cd9 (client: 474a37)
	void FreeSamples(); // 470571
	void LoadBitmaps(); // 4705c2 (client: 474320)
	void LoadSamples(); // 470504
	void FinishTravel(); // 472684 (client: 4763e2)
	void SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route); // 472ada

	VisGlobalMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); // 46fd7b (client: 473adb) in asm
public:
	CBmp64* gmap; //0x68
	CBmp64* hero_bmp; //0x6c
	CBmp64* ballmap; //0x70
	CA16* flag1_spr; //0x74
	CA16* flag_spr; //0x78
	CA16* cross_spr; //0x7c
	CA16* mission_flg; //0x80
	CA16* flg_on_map; //0x84
	CA16* yflag_spr; //0x88
	CBmp64* scroll1_bmp; //0x8c
	CBmp64* scroll2_bmp; //0x90
	CBmp64* scroll3_bmp; //0x94
	CBmp64* scrollp1_bmp; //0x98
	CBmp64* scrollp3_bmp; //0x9c
	CBmp64* scrollp2_bmp; //0xa0
	CWordArray locationAvailabilityFlags; //0xa4
	CArray<CRect> locationHitRects; //0xb8
	CArray<CPoint> locationPoints; //0xcc
	CArray<CPoint> travelRoutePoints; //0xe0
	CSize heroBitmapSize; //0xf4
	CPoint heroDrawPoint; //0xfc
	CPoint currentLocationPoint; //0x104
	CPoint targetLocationPoint; //0x10c
	CRect partDetailsRect; //0x114
	int32_t hoveredLocationIndex; //0x124
	int32_t mapFlagAnimationFrame; //0x128
	int32_t travelProgress; //0x12c
	int32_t partyFlagAnimationFrame; //0x130
	int32_t targetCrossAnimationFrame; //0x134
	uint32_t bestRouteCost; //0x138
	GlobalMapRouteArray routeNodeIndices; //0x13c
	void* routeAdjacencyMatrix; //0x154
	CArray<CPoint> graphNodePoints; //0x158
	CArray<DWORD> locationMetadata; //0x16c
	CSound snd_scrollup; //0x180
	CSound snd_scrolldn; //0x184
	CSound snd_point1; //0x188
	CSound snd_point2; //0x18c
	int32_t routePointSoundIndex; //0x190
	int32_t renderActiveFlag; //0x194
	CString hoveredLocationTitle; //0x198
	CStringArray hoveredLocationLines; //0x19c
	int32_t umoirMapMode; //0x1b0
};
ASSERT_SIZE(VisGlobalMap, 0x1b4);

//609cb0
class SoundPreferencesDialogVisualObject : public VisWindow {
public:
	SoundPreferencesDialogVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SoundSettings* pset); //43d917 (client: 43fd7b)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43e6f0 (client: 440b54)
	virtual void VMethod26() override; //43d959 (client: 43fdbd)

	int32_t ConvVolumeToSlider(int32_t vol, int32_t max_val) { return max_val - (sqrt((float)-vol / (float)max_val) * (float)max_val); } //45ca10 (client: 46063e)
	int32_t ConvSliderToVolume(int32_t pos, int32_t max_val) { //45ca45 (client: 460673)
		float v = (float)(pos - max_val) / (float)max_val;
		return max_val * -(v * v);
	}
public:
	MusicPlayer* music_player;
	int32_t track_index;
	SoundSettings* settings;
};
ASSERT_SIZE(SoundPreferencesDialogVisualObject, 0x74);


//60db28
class VisLogoWnd : public VisScreen {
public:
	VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cd4d0
	~VisLogoWnd(); //4cd53c

	virtual void VMethod7() override; //4cd8d1 (client: 4d2221)
	virtual void VMethod8(CRect* rect) override {}; //4cd91e
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4cd72e (client: 4d207e)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4cd6c2
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4cd6f8

	virtual void VMethod26(); //4cd5be (client: 4d1f0e)
	virtual void DoClose(uint32_t code); //4cd666 (client: 4d1fb6) 29 method   

public:
	CBmp256* logoBitmap;
	int32_t step;
	uint32_t timeoutStart;
	uint32_t timeoutTicks;
	uint32_t drawPendingFlag;
};
ASSERT_SIZE(VisLogoWnd, 0x7c);


//60b288
class VisFameHall : public VisScreen {
public:
	virtual ~VisFameHall(); // 45CDE2 body (??_G deleting dtor at 45DA90) (client: 460a12)

	virtual const char* GetHint() override; // 45db50 (client: 4616c0)
	virtual void VMethod7() override; // 45D7F9 (client: 461429)
	virtual void VMethod8(CRect* rect) override; // 45da6a
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 45D57F (client: 4611af)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 45D5F0 (client: 461220)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 45D6C3 (client: 4612f3)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 45D773 (client: 4613a3)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 45D5BE
	virtual int32_t OnChar(uint32_t wparam) override; // 45D5D7

	virtual void VMethod26() override; // 45CE6E (client: 460a9e)
	virtual void VMethod28() override; // 45D222 (client: 460e52)
	virtual void DoClose(uint32_t code) override; // 45D2D9 (client: 460f09)

	void LoadBitmaps(); // 45d32e (client: 460f5e) in asm
	void LoadSounds(); // 45d542 (client: 461172) in asm
	void UpdateRects(); // 45cfae in asm
	void FreeBitmaps(); // 45d44a (client: 46107a) in asm
	void FreeSounds(); // 45d563 in asm

	VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 45cd35 (client: 460965) in asm
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
class VisFameDocument : public VisScreen {
public:
	virtual ~VisFameDocument(); // 4AED50

	virtual const char* GetHint() override; // 4aeda0
	virtual void VMethod7() override; // 4AE7A3 (client: 4b30e3)
	virtual void VMethod8(CRect* rect) override; // 4ae926
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AE57F
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4AE5BE (client: 4b2efe)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4AE607 (client: 4b2f47)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4AE717 (client: 4b3057)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4ae791

	virtual void VMethod26() override; // 4ADAB6 (client: 4b23f6)
	virtual void VMethod28() override; // 4AE473 (client: 4b2db3)
	virtual void DoClose(uint32_t code) override; // 4AE534 (client: 4b2e74)

	VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4ad961 (client: 4b22a1)

	void LoadBitmaps(); // 4add24 (client: 4b2664) in asm
	void FreeBitmaps(); // 4ae17a (client: 4b2aba) in asm
	void ClearDocs(); // 4ae41b (client: 4b2d5b) in asm
	void LoadDocs(); // 4ae3c6 (client: 4b2d06) in asm
	int32_t UpdateButtons(CPoint pos, bool is_down); // 4ae952 (client: 4b3292) in asm
	void UpdateArrowStates(); // 4aec7a (client: 4b35ba) in asm
	void NextDoc(); // 4ae638 (client: 4b2f78) in asm
	void PrevDoc(); // 4ae6b3 (client: 4b2ff3) in asm
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


//60b098
class Vis2Action : public VisMenuWnd {
public:
	virtual ~Vis2Action(); // 450bd0

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44F30D (client: 452632)
	virtual void VMethod26() override; // 44F0E8 (client: 45240d)

	Vis2Action(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _txt, const CRect& _r); //44f0a2 (client: 4523c7)

public:
	const char* txt;
};
ASSERT_SIZE(Vis2Action, 0x80);


//60af78
class VisMissionFailed : public VisMessageBoxWithList {
public:
	virtual ~VisMissionFailed(); // 450B00
	virtual void VMethod26() override; // 44E8CA (client: 451bef)

	VisMissionFailed(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& _txt); //44e875 (client: 451b9a)
};
ASSERT_SIZE(VisMissionFailed, 0x78);

//60b008
class VisCutScenesDlg : public VisMessageBox {
public:
	virtual ~VisCutScenesDlg(); // 450B50

	VisCutScenesDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CStringArray& _payload); //44eabb (client: 451de0)


	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44EB08 (client: 451e2d)
};
ASSERT_SIZE(VisCutScenesDlg, 0x78);

//60e140
class VisListBoxDiplomacy : public VisListBox {
public:
	virtual ~VisListBoxDiplomacy(); // 4dceba (client: 4e183d)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4dd7f3 (client: 4e2176)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4dd44a (client: 4e1dcd)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4dd8c5 (client: 4e2248)
	virtual int32_t IsValidIndex(int32_t idx) override; // 4dd098 (client: 4e1a1b)
	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 4dd0bd (client: 4e1a40)
	virtual void SelectItem(int32_t idx) override; // 4dcf98 (client: 4e191b)
	virtual void VMethod30(CPoint pos, const CRect& r) override; // 4e3d30

	VisListBoxDiplomacy(int32_t _id, const RECT& r, CArray<DiplomacyEntry*>* _diplomacy, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height); //4dc9fb

	void WriteRadioState(); //4dd1b2 (client: 4e1b35)
	void UpdateScrollBar(); //4dd2c7 (client: 4e1c4a)
	void RestoreRect(); //4dd424 (client: 4e1da7)
	void UpdateRadioPositions(int32_t vis_start_index); //4dd54c (client: 4e1ecf)

public:
	CArray<DiplomacyEntry*>* diplomacy; //0x94
	CArray<VisRadioType1*>* enemy_radios; //0x98
	CArray<VisRadioType1*>* ally_radios; //0x9c
	CArray<VisRadioType1*>* see_radios; //0xa0
	CArray<VisRadioType1*>* mute_radios; //0xa4
};
ASSERT_SIZE(VisListBoxDiplomacy, 0xa8);

//60a4c8
class VisDiplomacy : public VisMessageBox {
public:
	virtual ~VisDiplomacy(); // 44F9E0

	VisDiplomacy(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CArray<DiplomacyEntry*>* _payload); //445bcb (client: 44829c)

	virtual void ReadData(const void* buf) override; // 445D34 (client: 448405)
	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 445C2E (client: 4482ff)
	virtual void VMethod31(int32_t code) override; // 445CF1 (client: 4483c2)
public:
	// Points at MainWindow's CArray<DiplomacyEntry*>* variable (the binary puns it
	// through a const CArray* ctor parameter), so ReadData re-reads the current array.
	CArray<DiplomacyEntry*>** diplomacy; //0x78
};
ASSERT_SIZE(VisDiplomacy, 0x7c);


//60a558
class VisConnectionDlg : public VisMessageBox {
public:
	virtual ~VisConnectionDlg(); // 44fa40

	VisConnectionDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, int32_t* _payload); //445e52 (client: 448523)

	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44604C (client: 44871d)
	virtual void VMethod31(int32_t code) override; // 445EBD (client: 44858e)
public:
	CLlConn* protocols;
	int32_t* pSelected;
};
ASSERT_SIZE(VisConnectionDlg, 0x80);


//60a5e8
class VisNetDlg : public VisWindow {
public:
	virtual ~VisNetDlg(); // 44FA60

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 447214 (client: 4498e3)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4478E9
	virtual void VMethod26() override; // 446E97 (client: 449566)

	VisNetDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, AvailNetSession* _sessions); //4464c7 (client: 448b96)

private:
	void CreateSessionList(); //446567 (client: 448c36)
	void RefreshSessions(); //447173 (client: 449842)
	void FillServerList(); //44797E (client: 44a04d)
	void UpdatePlayerList(); //447B3C (client: 44a20b)
	void CachePlayerRows(); //447C27 (client: 44a2f6)

public:
	int32_t selected;
	AvailNetSession* sessions;
	CRect net_rect;
	CArray<CStringArray*> cached_player_rows;
};
ASSERT_SIZE(VisNetDlg, 0x94);


//60a810
class VisNetTcpIp : public VisMessageBox {
public:
	virtual ~VisNetTcpIp(); // 44FD10

	VisNetTcpIp(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* _ip); //449f06

	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44A014 (client: 44c6e6)
	virtual void VMethod31(int32_t code) override; // 449F67 (client: 44c639)
public:
	CString* ip;
};
ASSERT_SIZE(VisNetTcpIp, 0x7c);


//60a788
class VisNetSerialSettings : public VisWindow {
public:
	virtual ~VisNetSerialSettings(); // 44FCC0

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 449C98 (client: 44c36a)
	virtual void VMethod26() override; // 448D15 (client: 44b3e7)

	void SaveComSettings(); //449903 (client: 44bfd5)

	VisNetSerialSettings(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, ComSettings* _com); //448cd5 (client: 44b3a7)

public:
	ComSettings* pcom;
};
ASSERT_SIZE(VisNetSerialSettings, 0x6c);

//60a6f8
class VisListBoxPhoneBook : public VisListBox {
public:
	virtual ~VisListBoxPhoneBook(); // 44fca0

	virtual int32_t OnKeyDown(uint32_t wparam) override; // 44fb80 (client: 453a80)

	VisListBoxPhoneBook(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height, const char* hint, CStringArray* _bound_entries); //44fb30 (client: 453a30)

public:
	CStringArray* bound_entries; //0x94
};
ASSERT_SIZE(VisListBoxPhoneBook, 0x98);

//60a670
class VisNetPhoneBook : public VisWindow {
public:
	virtual ~VisNetPhoneBook(); // 44FAE0

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44878A (client: 44ae5c)
	virtual void VMethod26() override; // 447EF2 (client: 44a5c1)

	VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book); //447ea8 (client: 44a577)

	void sub_448708(const char* str); // 448708 (client: 44adda)

public:
	int32_t enum_addresses_num;
	PhoneBook* phones;
	CLlAddress* enum_addresses;
};
ASSERT_SIZE(VisNetPhoneBook, 0x74);


struct NetMapInfo {
	CString mapid;
	CString name;
	CString tooltip;
	int32_t players;
	int32_t maplevel;
	int32_t width;
	int32_t height;
};
ASSERT_SIZE(NetMapInfo, 0x1c);

//60aad0 — map list box on the net map-selection screen
class VisNetMapList : public VisListBox {
public:
	virtual ~VisNetMapList(); // 450030 (client: 454250) (complete dtor 450060)

	VisNetMapList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint, int32_t* p_selected_map_index); // 44ffd0 (client: 4541f0) in asm

	virtual const char* GetHint() override; // 44b2bf (client: 44d9ec) in asm
	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44b09e (client: 44d7cb) in asm

	int32_t* p_selected_map_index; // 0x94
};
ASSERT_SIZE(VisNetMapList, 0x98);

//60ab60 — chat log list box
class VisNetChatList : public VisListBox {
public:
	virtual ~VisNetChatList(); // 450100 (client: 454320) (complete dtor 450130)

	VisNetChatList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450080 in asm

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c63d (client: 44f962) in asm

	CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetChatList, 0xa8);

//60abf0 — player list box
class VisNetPlayerList : public VisListBox {
public:
	virtual ~VisNetPlayerList(); // 450200 (client: 454420) (complete dtor 450230)

	VisNetPlayerList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450180 in asm

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c71d (client: 44fa42) in asm

	CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetPlayerList, 0xa8);

//60ac80 — chat input line
class VisNetChatTextBox : public VisTextBox {
public:
	virtual ~VisNetChatTextBox(); // 450350 (complete dtor 450380)

	VisNetChatTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint); // 450280 in asm

	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4502c0 (client: 454540) in asm
};
#ifdef A2CLIENT
ASSERT_SIZE(VisNetChatTextBox, 0x80);
#else
ASSERT_SIZE(VisNetChatTextBox, 0x7c);
#endif

//60a930
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam); // 44aad6

class VisNetMapSelection : public VisWindow {
public:
	virtual ~VisNetMapSelection(); // 44FEA0

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44D275 (client: 45059d)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 44D8F0
	virtual void VMethod26() override; // 44C788 (client: 44faad)

	VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName); //44af45 (client: 44d672)

	void StopThread(); // 44aeef (client: 44d61c)
	void AddNetMapInfo(NetMapInfo* info); // 44d13d (client: 450465)
	void FUN_0044aae5(); // 44aae5 (client: 44d212)

public:
	CString* p_mapname;
	CArray<NetMapInfo*> avail_maps;
	CWinThread thread;
	HANDLE stop_event;
	int32_t selected_map_index;
	GM_a28* chat_log;
	BigStruct2* map_context;
	int32_t mode;
};
// binary 0xfc; the embedded CWinThread differs between VC6 and MFC140,
// so pin the composition: 5 tail members (0x14) follow the thread object
ASSERT_SIZE(VisNetMapSelection, offsetof(VisNetMapSelection, thread) + sizeof(CWinThread) + 0x14);

//60e650
class VisTipsDialog : public VisScreen {
public:
	virtual ~VisTipsDialog(); // 4e4510

	virtual void VMethod8(CRect* rect) override; // 4E27CD (client: 4e7151)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4E2B95 (client: 4e7519)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4E2C42 (client: 4e75c6)
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } // 4e2c33

	VisTipsDialog(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); //4e2541 (client: 4e6ec5)

	void FUN_004e25c1(int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); // 4e25c1 (client: 4e6f45)
	void SetText(const char* text); // 4e2b73 (client: 4e74f7)

public:
	uint8_t __unused__[12];
};
ASSERT_SIZE(VisTipsDialog, 0x74);


//60e6d8
class VisQuestStatus : public VisScreen {
public:
	virtual ~VisQuestStatus(); // 4E4540

	virtual void VMethod7() override; // 4E2F76 (client: 4e78fa)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4e4530

	VisQuestStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4e2e3d (client: 4e77c1)

public:
	CSprite256* icon;
};
ASSERT_SIZE(VisQuestStatus, 0x6c);

//60a8a0
class VisHatServerListDlg : public VisMessageBox {
public:
	virtual ~VisHatServerListDlg(); // 44fd90

	VisHatServerListDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, HatSettings* hat); //44a0e2

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44A8A8 (client: 44cfd5)
	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44A34D (client: 44ca33)
	virtual void VMethod31(int32_t code) override; // 44A140
public:
	HatSettings* hat_settings;
};
ASSERT_SIZE(VisHatServerListDlg, 0x7c);


//60aa40
class VisHatBrowserList : public VisListBox {
public:
	virtual ~VisHatBrowserList(); // 44ffb0

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44b60c (client: 44dd68)

	VisHatBrowserList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 44ff20 (client: 453f60)

	int32_t IsItemEnabled(int32_t idx); // 44b460

public:
	int32_t state; //0x94. -1 = refreshing, 0 = empty/failed, 1 = loaded
};
ASSERT_SIZE(VisHatBrowserList, 0x98);

// Returns the start position of the field-th (0-based) '|'-separated field of str, or -1.
int32_t GetFieldPos(CString str, int32_t field); // 44b37a

//60a9b8
class VisHatBrowserDlg : public VisWindow {
public:
	virtual ~VisHatBrowserDlg(); // 44FED0

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44C1A9
	virtual void VMethod26() override; // 44BC6C

	VisHatBrowserDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //44bc35

#ifdef A2CLIENT
	// client-only web-browser glue. The retail client's browser object is installed by the
	// a2mgr.dll mod layer; every use is gated on browser != 0, so an un-hooked client (and
	// this port, which leaves it null) no-ops through all of them.
	void* browser; // 0x68 — 0x21c-byte browser object (client ctor 4b18c0)
	void* worker_thread; // 0x6c — worker thread driving browser_step (client 44e391)
	HANDLE exit_event; // 0x70 — auto-reset; SetEvent from MsgProc 0x445/0x446
	HANDLE ack_event; // 0x74 — auto-reset, initially signaled
	CRITICAL_SECTION connect_cs; // 0x78 — guards connect_addr
	CString connect_addr; // 0x90 — address handed to the worker
#endif
};
#ifndef A2CLIENT
ASSERT_SIZE(VisHatBrowserDlg, 0x68);
#else
ASSERT_SIZE(VisHatBrowserDlg, 0x94);
#endif

#ifdef A2CLIENT
// client vtable 5d7238 — hat-DM score dialog popped by BigStruct2 packet cases 0x5c/0xc3.
// (The client label "MusicPlayer" on this class is a mislabel; the real MusicPlayer is in sound.h.)
class HatPlayerListWindow : public VisMessageBox {
public:
	~HatPlayerListWindow(); // client: 452837 (deleting dtor 454940)
	int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // client: 452a02
	CVisualObject* VMethod30(const void* payload, const RECT& r) override; // client: 452897

	HatPlayerListWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, int32_t _mode); // client: 4527dd

public:
	int32_t mode; // 0x78 — 0 = plain dialog, !=0 = GM ack line on OK
};
ASSERT_SIZE(HatPlayerListWindow, 0x7c);

// client vtable 5d72c8 — the score list created by HatPlayerListWindow::VMethod30.
// (The client label "CSpriteFont16" on this class is a mislabel; the real font class has vtable 5d74f0.)
class HatPlayerScoreList : public VisListBox {
public:
	~HatPlayerScoreList(); // client: 454a20 (deleting dtor 4549f0)
	void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // client: 45265b

	HatPlayerScoreList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // client: 454970

public:
	CArray<uint32_t> scores; // 0x94 — parallel to entries, one dword per player
};
ASSERT_SIZE(HatPlayerScoreList, 0xa8);
#endif

#endif
