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


uint32_t IsDefaultNpcName(const char* name); // 4386e0 (client: 43ad20)

// Visual-object UI event codes routed through CVisualObject::MsgProc. Every
// screen class overrides MsgProc and switches on these; screens also close
// themselves with VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN / VIS_MSG_CLOSE_SCREEN)
// and parents compare GetCloseCode() against the button codes. NOT the same id
// space as the MainWindow window messages (MAIN_MSG_*) even where the numeric
// ranges overlap - forwarded ids (e.g. 0x467 music position) cross over.
enum VisMsg : uint32_t {
    VIS_MSG_MOUSE                = 0x400, // mouse message forwarded from the window
    VIS_MSG_WORLD_TICK           = 0x401, // per-frame world/logic tick (BigStruct2)
    VIS_MSG_FRAME_TICK           = 0x402, // per-frame redraw tick (VMethod9 / RenderWorldFrame)
    VIS_MSG_SET_MAP_CONTEXT      = 0x403, // wparam = BigStruct2* map context
    VIS_MSG_MINIMAP_REBUILD      = 0x404, // rebuild the minimap bitmap
    VIS_MSG_CANCEL_ORDER         = 0x405, // leave order/targeting mode
    VIS_MSG_SCROLL_VIEW          = 0x406, // scroll the world view (wparam/lparam = tile x/y)
    VIS_MSG_DATA_REFRESH         = 0x408, // backing data changed - redraw
    VIS_MSG_ORDERS_ENABLE        = 0x409, // order toolbar: wparam = available-orders mask
    VIS_MSG_ORDERS_DISABLE       = 0x40a, // order toolbar: gray out
    VIS_MSG_ORDERS_CLEAR         = 0x40b, // order toolbar: clear selection
    VIS_MSG_ORDER_MODE_CHANGED   = 0x40c, // map: order mode selected (wparam = order, plays sfx)
    VIS_MSG_TOOLBAR_ORDER_SELECT = 0x40d, // order toolbar: select order wparam
    VIS_MSG_TOGGLE_SELECT_PANEL  = 0x40e, // toggle char-info selection panel / close bag
    VIS_MSG_TOGGLE_SPELL_PANEL   = 0x40f, // toggle spell panel / close spell book
    VIS_MSG_STATUS_REFRESH       = 0x410, // status panels: dirty redraw (no sender left in tree)
    VIS_MSG_SHORTCUT_RELEASE     = 0x411, // spell book: clear pressed shortcut
    VIS_MSG_TOGGLE_INFO_MODE     = 0x412, // char info: toggle info mode (mode button)
    VIS_MSG_REFRESH_ITEM_GRIDS   = 0x413, // item grids: wparam = ItemListMode, lparam = &list
    VIS_MSG_SELECT_PREV          = 0x414, // select previous party unit / face
    VIS_MSG_SELECT_NEXT          = 0x415, // select next party unit / face
    VIS_MSG_SHORTCUT_KEY         = 0x417, // hotbar hotkey pressed (wparam = slot, lparam = kb mods)
    VIS_MSG_LOGO_SET_TIMEOUT     = 0x439, // logo window: wparam = timeout ms
    VIS_MSG_LOGO_SET_STEP        = 0x43a, // logo window: wparam = step (bitmap index)
    VIS_MSG_LIST_ACTIVATE        = 0x444, // list box double-click (wparam = list id, lparam = index)
    VIS_MSG_HIDE_SCREEN          = 0x445, // hide this screen (DoClose reply convention)
    VIS_MSG_CLOSE_SCREEN         = 0x446, // close this screen / child dialog
    VIS_MSG_BOX_OK               = 0x447, // message box OK button code
    VIS_MSG_NET_CLOSE_SERVER     = 0x44d, // net dialog close-as-server code (no producer left in tree)
    VIS_MSG_NET_CREATE           = 0x44e, // net dialog: create-session button
    VIS_MSG_NET_JOIN             = 0x44f, // net dialog: join button
    VIS_MSG_NET_REFRESH          = 0x450, // net dialog: refresh session list button
    VIS_MSG_COMBO_TOGGLE         = 0x456, // combo box: toggle the drop-down list
    VIS_MSG_CURSOR_BLINK         = 0x462, // text box: blink the caret (idle-loop ping)
    VIS_MSG_CLOSE_TIPS           = 0x45A, // remove the tips child from the screen
    VIS_MSG_NETMAP_APPLY         = 0x45E, // net map selection: apply the selected map
    VIS_MSG_NETMAP_CHAT_SYNC     = 0x45F, // net map selection: sync chat list scroll
    VIS_MSG_ROSTER_CHANGED       = 0x460, // net screens: player roster changed
    VIS_MSG_NETMAP_ADD_INFO      = 0x461, // net map selection: wparam = NetMapInfo*
    VIS_MSG_MUSIC_POSITION       = 0x467, // sound thread: wparam = track, lparam = seconds
    VIS_MSG_SCROLL_SELECT        = 0x469, // scrollbox: select item (lparam = index)
    VIS_MSG_SCROLL_UP            = 0x46a, // scrollbox: line up
    VIS_MSG_SCROLL_DOWN          = 0x46b, // scrollbox: line down
    VIS_MSG_SCROLL_PAGE_UP       = 0x46c, // scrollbox: page up
    VIS_MSG_SCROLL_PAGE_DOWN     = 0x46d, // scrollbox: page down
    VIS_MSG_SET_OPTION           = 0x46e, // multiplexed per-dialog option set (wparam = option)
    VIS_MSG_SCROLLBAR_RESIZE     = 0x46f, // scrollbar: recompute rects
    VIS_MSG_SPEECH_ADVANCE       = 0x470, // role-key screen: advance speech line
    VIS_MSG_NET_APPLY_PROVIDER   = 0x471, // net dialog: relayout for the current provider
    VIS_MSG_LIST_SELECT          = 0x473, // list selection (wparam = list id, lparam = index)
    VIS_MSG_SFX_TEST             = 0x474, // sound prefs: play test sample (wparam = which)
    VIS_MSG_LIST_SECONDARY       = 0x475, // load/save dialog: second button (details/save)
    VIS_MSG_ACKNOWLEDGE          = 0x476, // sound prefs: write acknowledgement + hide
    VIS_MSG_MUSIC_ENABLE         = 0x477, // sound prefs: enable music
    VIS_MSG_MUSIC_DISABLE        = 0x478, // sound prefs: disable/fade out music
    VIS_MSG_CONFIRM              = 0x479, // load/save dialog: OK button
    VIS_MSG_CANCEL               = 0x47a, // dialog: cancel button
    VIS_MSG_MENU_BUTTON_CLICK    = 0x47b, // menu button clicked (wparam = target window message)
    VIS_MSG_NET_NEW_SESSION      = 0x47c, // net dialog: a new session appeared (wake event)
    VIS_MSG_PHONE_CONNECT        = 0x47d, // phone book: connect via edited entry
    VIS_MSG_PHONE_DIAL           = 0x47e, // phone book: dial the selected entry
    VIS_MSG_SERIAL_CONNECT       = 0x480, // serial settings: connect
    VIS_MSG_SERIAL_HOST          = 0x481, // serial settings: host a session
    VIS_MSG_SERIAL_CLEAR         = 0x483, // serial settings: clear children/focus
    VIS_MSG_NETMAP_FIND_MAP      = 0x484, // net map selection: wparam = map name to select
    VIS_MSG_AMBIENT_STOP         = 0x485, // stop ambient view (music thread request)
    VIS_MSG_HATBROWSER_REFRESH   = 0x48a, // hat browser: refresh the current page
    VIS_MSG_HATBROWSER_ABORT     = 0x48b, // hat browser: abort the scan
    VIS_MSG_HATBROWSER_PROGRESS  = 0x48c, // hat browser: worker-thread progress pump
};

class CGameSession;
class VisLabel;
class CGameBitmap;
class CGameObject;
class CGameFont;
class CBmp64;
class CBmp256;
class CA16;
class BigStruct2;
class GM_a28;
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



// Child-window IDs of the in-game screen tree: the map context (BigStruct2)
// itself, its children, and the tips-dialog internal controls all draw from one
// shared ID allocation. The client build allocates one extra ID early in that
// space (the map context itself sits at 2 instead of 1), which shifts every
// following ID up by one. Values verified against a2serv6.exe / allods2.exe.
enum MapChildId {
#ifdef A2CLIENT
	MAP_CONTEXT_ID = 2,
	MAP_CHILD_BAG = 3,
	MAP_CHILD_SPELL_BOOK = 4,
	MAP_CHILD_TIPS_TEXT = 14,
	MAP_CHILD_TIPS_OK_BUTTON = 15,
	MAP_CHILD_TIPS_SHOW_RADIO = 16,
	MAP_CHILD_TIPS_DIALOG = 17,
	MAP_CHILD_QUEST_STATUS = 19, // shared with the hat-session panel below
	MAP_CHILD_HAT_SESSION = 19,
	MAP_CHILD_CHAT_LINE = 1200, // Vis1200 input strip; not part of the shifted range
#else
	MAP_CONTEXT_ID = 1,
	MAP_CHILD_BAG = 2,
	MAP_CHILD_SPELL_BOOK = 3,
	MAP_CHILD_TIPS_TEXT = 13,
	MAP_CHILD_TIPS_OK_BUTTON = 14,
	MAP_CHILD_TIPS_SHOW_RADIO = 15,
	MAP_CHILD_TIPS_DIALOG = 16,
	MAP_CHILD_QUEST_STATUS = 18,
	MAP_CHILD_CHAT_LINE = 1200,
#endif
};



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
	~CVisualObject();  // client: 4db98a
	virtual void Dump(CDumpContext& dc) const override;  // client: 4dcbc3

	virtual const char* GetHint();  // client: 4dc525
	virtual void SetHint(const char *_hint);  // client: 4dc509
	virtual void ChangeFlags(uint32_t _flags, bool setunset);  // client: 4dc313
	virtual uint32_t TestFlags(uint32_t _flags);  // client: 4dc2fd
	virtual void SetCursorOver(bool isOver);  // client: 4dc34a
	virtual void SetFocus(bool isFocus);  // client: 4dc3b9
	virtual void VMethod7();  // client: 4dc550
	virtual void VMethod8(CRect* rect);  // client: 420190
	virtual void VMethod9();  // client: 4dc5d8
	virtual void VMethod10();  // client: 4dc613
	virtual void WriteData(void* buf);  // client: 4dc6a9
	virtual uint32_t DataSize();  // client: 4dc647
	virtual void ReadData(const void* buf);  // client: 4dc712
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam);  // client: 4dc8d9
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos);  // client: 420950
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos);  // client: 4201a0
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos);  // client: 43a7c0
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos);  // client: 43a7d0
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos);  // client: 43a7e0
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos);  // client: 43a600
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos);  // client: 43a610
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos);  // client: 4201b0
	virtual int32_t OnKeyDown(uint32_t wparam);  // client: 43a620
	virtual int32_t OnKeyUp(uint32_t wparam);  // client: 4201c0
	virtual int32_t OnChar(uint32_t wparam);  // client: 43a630

public:
	CVisualObject();  // client: 4db650
	CVisualObject(int32_t _id, const RECT& r, const char* str);  // client: 4db760
	CVisualObject(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str);  // client: 4db86f

	int32_t MsgProcOnChilds(uint32_t msg, uint32_t wparam, uint32_t lparam);  // client: 4dc77b

	void AddChild(CVisualObject* obj);  // client: 4dbbd4

	void RemoveChild(CVisualObject* obj);  // client: 4dbbf9
	void RemoveChildById(int32_t _id);  // client: 4dbca3
	void RemoveAllChilds();  // client: 4dc1e4

	void DestroyChild(CVisualObject* obj);  // client: 4dbd40
	void DestroyChildById(int32_t _id);  // client: 4dbddf
	void DestroyAllChilds();  // client: 4dc260

	CVisualObject* FindChild(int32_t _id);  // client: 4dc183
	CVisualObject* GetChildAt(POINT pt);  // client: 4dc458

	void ClientPtToScreen(CPoint* out, const CPoint& _point);  // client: 4dbb03
	void ClientRectToScreen(CRect* out, const CRect& _rect);  // client: 4dbb93

	CPoint ClientPtToScreen(const CPoint& _point);  // client: 4dbb03
	CRect ClientRectToScreen(const CRect& _rect);  // client: 4dbb93

	void SetLeftObj(CVisualObject* obj);  // client: 4dcbdd
	void SetRightObj(CVisualObject* obj);  // client: 4dcbfc
	void SetUpObj(CVisualObject* obj);  // client: 4dcc1b
	void SetDownObj(CVisualObject* obj);  // client: 4dcc3a

	void SetCaptionLabel(VisLabel* obj);  // client: 4dcc59

	void FocusTo(CVisualObject* obj, bool update);  // client: 4dc126
	void TabFocus(bool forward, bool update);  // client: 4dbf84

	void SetRect(RECT r);  // client: 4dba88
	void SetRect(const RECT* r);  // client: 4dba50
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
	~VisLabel();  // client: 453f10

	// VTable at 60de38.
	virtual void VMethod7() override;  // client: 4dcdf4

public:
	VisLabel();  // client: 4dcc6f
	VisLabel(int32_t _id, const RECT& r, const char* _text, CGameFont* _font, uint16_t* colorsh, uint32_t align);  // client: 4dcd63
	VisLabel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _text, CGameFont *_font, uint16_t *colorsh, uint32_t align);  // client: 4dccc6

	void SetActiveColor(bool isActive);  // client: 4dcef6
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
	virtual ~VisButton();  // client: 453330

	virtual void VMethod7() override;  // client: 4dd88a
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4dde8b
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4ddf9d
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4ddfe4
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4de07f
	virtual int32_t OnChar(uint32_t wparam) override;  // client: 4de0d0

	VisButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint);  // client: 4dd73b
	VisButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint);  // client: 4dd5e0

	void SetDowned(bool down);  // client: 4dddb1
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
	virtual ~VisScrollBar();  // client: 4e9260

	virtual void SetCursorOver(bool isOver) override;  // client: 4e3060
	virtual void VMethod7() override;
	virtual void VMethod9() override;  // client: 4e2a51
	virtual void WriteData(void* buf) override;  // client: 4e9bc0
	virtual void ReadData(const void* buf) override;  // client: 4e9bf0
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;  // client: 4e39d6
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4e34b3
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;  // client: 4e383e
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4e3082
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4e3948
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;  // client: 4e9c20
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4e3a08

	VisScrollBar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* hint);  // client: 4e27eb


	void SetPos(int32_t pos, int32_t pos_max);  // client: 4e2a07
	void UpdateRects();  // client: 4e2450
	void UpdateHBoxes();  // client: 4e25c2

	int32_t GetValHPos(int32_t x);  // client: 4e273f

public:
	int32_t dirty;
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

	virtual void VMethod7() override;  // client: 4e0aa0
	virtual void WriteData(void* buf) override;  // client: 4e0a3f
	virtual uint32_t DataSize() override;  // client: 452ef0
	virtual void ReadData(const void* buf) override;  // client: 4e0a6d
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;  // client: 4e0fb2
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4e1227
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4e10c2
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4e11b5
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;  // client: 4e11ee
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4e1279

	virtual int32_t IsValidIndex(int32_t idx); //26 (client: 4e0eab)
	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr); //27 (client: 4e0ee3)
	virtual void SelectItem(int32_t idx); //28 (client: 4e0802)
	virtual int32_t GetItemCount(); //29 (client: 452ed0)
	virtual void VMethod30(CPoint pos, const CRect& r); //30 (client: 4e0e71)

	VisListBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);  // client: 4e06b5
	VisListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);  // client: 4e0596

	void Down();  // client: 4e093e
	void Up();  // client: 4e090c
	void PageDown();  // client: 4e09be
	void PageUp();  // client: 4e0964

	int32_t YToIndex(int32_t y); //Get index of y coordinate (client: 4e07c8)

	void AddItem(const char* str);  // client: 452ce0

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

	virtual void VMethod7() override;  // client: 4de337
	virtual void WriteData(void* buf) override;  // client: 454510
	virtual uint32_t DataSize() override;  // client: 454500
	virtual void ReadData(const void* buf) override;  // client: 4544e0
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;  // client: 4df3b0
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4debaf
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4decfd
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4ded6e
	virtual int32_t OnChar(uint32_t wparam) override;  // client: 4df2eb

	VisTextBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 4de26c
	VisTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 4de195

	void DelSelection();  // client: 4de9e8
	void InsertChar(int32_t chr);  // client: 4de875
	void ResetBlink();  // client: 4df438
	int32_t GetCursorPosByX(int32_t x);  // client: 4dead4

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

	virtual void SetCursorOver(bool isOver) override;  // client: 4df652
	virtual void WriteData(void* buf) override;  // client: 452bb0
	virtual uint32_t DataSize() override;  // client: 452ba0
	virtual void ReadData(const void* buf) override;  // client: 453740
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4df674

	virtual int32_t GetIndex(int32_t y); //26 (client: 4df606)

	VisRadioBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 4df4b0

	void AddEntry(const char* etext);  // client: 452b30
public:
	int32_t mouse_over;
	int32_t gap_0x60;
	CStringArray entries;
	CGameFont* font;
	uint16_t* clr;
	int32_t selection;
	int32_t gap_0x84;
	int32_t selected;
};
ASSERT_SIZE(VisRadioBase, 0x8c);

//60acf8
class VisRadioType1 : public VisRadioBase {
public:
	virtual ~VisRadioType1();
	virtual void VMethod7() override;  // client: 4df77d
	virtual void ReadData(const void* buf) override;  // client: 452bd0
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4dfab9
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4df9e7
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override;  // client: 452bf0
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4dfc58
	virtual int32_t OnChar(uint32_t wparam) override;  // client: 4dfb92

	VisRadioType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 452b50
};

//60ad78
class VisRadioType2 : public VisRadioBase {
public:
	virtual ~VisRadioType2();
	virtual void VMethod7() override;  // client: 4e0005
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4e0500
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4e026c
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4e0393
	virtual int32_t OnChar(uint32_t wparam) override;  // client: 4e0318

	VisRadioType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 452b50
};


//60e248
class VisBitmap : public CVisualObject {
public:
	virtual ~VisBitmap();  // client: 4d1e8c
	virtual void VMethod7() override;  // client: 4e3cc6

	VisBitmap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t dtype);  // client: 4e3bbe
	VisBitmap(int32_t _id, const RECT& r, CGameBitmap* _bitmap, int32_t dtype);  // client: 4e3c07

	CGameBitmap* GetBitmap();  // client: 4e9c80
	void SetBitmap(CGameBitmap* bmp);  // client: 4e9ca0
public:
	CGameBitmap* bitmap;
	int32_t draw_type;
};
ASSERT_SIZE(VisBitmap, 0x64);


//60e4d0
class VisComboBox : public CVisualObject {
public:
	virtual ~VisComboBox() = default;

	virtual void VMethod8(CRect* rect) override;  // client: 4e6e05
	virtual void VMethod9() override;  // client: 4e69a2
	virtual void WriteData(void* buf) override;  // client: 4e69b5
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;  // client: 4e6c5c
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4e6d94
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4e6da6

	VisComboBox(int32_t _id, CRect r, const char* hint);  // client: 4e6791

	void AddItem(const char* str);  // client: 4e69d7
	void ToggleList();  // client: 4e6ad4
	void HideList();  // client: 4e6a5f
	void ProcSelectList();  // client: 4e6bae
	void SelectItem(int32_t index);  // client: 4e6a1b

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
	virtual void VMethod7() override;  // client: 4e9510

	VisComboBoxText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint);  // client: 4e94c0
};

//60e548
class VisComboBoxList : public VisListBox {
public:
	virtual ~VisComboBoxList() = default;
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4e9430

	VisComboBoxList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint);  // client: 4e93e0
};

//60e458
class VisComboBoxButton : public VisButton {
public:
	virtual ~VisComboBoxButton() = default;

	virtual void SetCursorOver(bool isOver) override;  // client: 4e6637
	virtual void VMethod7() override;  // client: 4e66fe
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4e66dd
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4e66bc
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4e6597

	VisComboBoxButton(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, int32_t _frm, int32_t _msgid, int32_t _charid, const char* hint);  // client: 4e64dd
public:
	CGameBitmap* bitmap;
	int32_t frm;
};
ASSERT_SIZE(VisComboBoxButton, 0x80);

//60deb0
class VisMultiText : public VisListBox {
public:
	virtual ~VisMultiText() = default; //4e3c20 (client: 4e90f0)

	virtual void VMethod7() override;  // client: 4dd2a4
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override;  // client: 4dd4a9
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override;  // client: 4dd4ca
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override;  // client: 4dd4eb
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override;  // client: 4dd54f
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 4dd570

	virtual void SelectItem(int32_t idx) override;  // client: 4dd1ad

	VisMultiText(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy);
	VisMultiText(int32_t _id, const RECT& r, const char* str, CGameFont* _font, uint16_t* _clr, int32_t dy);

	void SetText(const char* text);  // client: 4dd126
	void SizesCheck();  // client: 4dd34b

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

	virtual void VMethod26(); //450900 (client: 44e774)
	virtual void VMethod27(); //4388c0 (client: 43a9e0)
	virtual void VMethod28(); //4dfb4f (client: 4e44d0)
	virtual void DoClose(uint32_t code); //4dfb8a (client: 4e450b) 29 method   

	VisScreen();  // client: 425b0f
	VisScreen(int32_t _id, const RECT& r, CGameBitmap* _bitmap);  // client: 4e3e7b
	VisScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap);  // client: 4e3eee

	int32_t GetCloseCode(); // (client: 43ab60)
	void CloseOk();  // client: 4348e6
	void CloseCancel();  // client: 430a8a
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
	virtual int32_t OnKeyDown(uint32_t wparam) override;  // client: 449fb8

	VisWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap);  // client: 4e472c

	void UpdateWinRect(); //4dfe19 (client: 4e479a)
};
ASSERT_SIZE(VisWindow, 0x68);


//60e3d0 — role/NPC speech dialog screen shown by ShowRoleKeyDialog
class VisRoleKeyScreen : public VisWindow {
public:
	virtual ~VisRoleKeyScreen(); //4e3f60 (client: 4e9340)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4e0071 (client: 4e49f2)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4e0027 (client: 4e49a8)
	virtual void VMethod28() override; //4e012d (client: 4e4aae)
	virtual void DoClose(uint32_t code) override; //4e0148 (client: 4e4ac9)

	VisRoleKeyScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& text); //4dff5c (client: 4e48dd)

	bool AdvanceSpeechLine(); //4e0169 (client: 4e4aea) in asm - advance to next talk line
	bool ParseSpeechLine(int32_t line_no, int32_t* out_id, CString* out_text, char** out_name, char** out_name_end, int32_t* out_music); //4e0b43 (client: 4e54c4)
	void LoadText(); //4e0978 (client: 4e52f9)
public:
	int32_t line_no; // 0x68
	CString role_name; // 0x6c
	SfxSample* speech_sample; // 0x70 - speech sample currently playing
	char* text_buf; // 0x74
	int32_t text_len; // 0x78
	int32_t is_npc; // 0x7c
	int32_t tips_id; // 0x80
	int32_t spoken_by_player; // 0x84
};
ASSERT_SIZE(VisRoleKeyScreen, 0x88);


//60a288
class VisMessageBox : public VisWindow {
public:
	virtual ~VisMessageBox(); // 444ff3 (client: 4476c1)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44500f (client: 4476dd)
	virtual void VMethod26() override; // 444086 (client: 446656)

	virtual CVisualObject* VMethod30(const void* str, const RECT& r) = 0;
	virtual void VMethod31(int32_t code); // 445084 (client: 447752)

	VisMessageBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, const char* str2, int32_t btypes, const char* str3); // 44402b (client: 4465fb)

public:
	const void* payload;
	const char* text;
	const char* title;
	int32_t button_types;
};
ASSERT_SIZE(VisMessageBox, 0x78);

//60a318
class VisMessageBoxWithList : public VisMessageBox {
public:
	virtual ~VisMessageBoxWithList() {}; //44f930 (client: 453820)
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
	virtual ~VisCharSelect(); // 438650 (client: 43ac00)

	virtual void VMethod8(CRect* rect) override; // 438d50 (client: 43abf0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4326A4 (client: 4347d5)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4326EA (client: 43481b)

	virtual void VMethod26() override; // 431FDF (client: 43410d)
	virtual void VMethod28() override; // 43233E (client: 43446f)
	virtual void DoClose(uint32_t code) override; // 43251C (client: 43464d)


	void OpenRenameWindow(); //4327f3 (client: 434924)

	void SelectCharacter(CUnit* unit); // client: 434786 (client: 434786)

	void LoadSfx(); // 432280 (client: 430eb0)
	void FreeSfx(); // 4322ed (client: 430f1d)
	void CloseRenameWindow(); // 432933 (client: 434a64)
	CUnit* GetSelectedMapUnit(); // 438d30 (client: 430ac8)

	VisCharSelect(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //431eac (client: 433fda) in asm

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
	virtual ~VisCharSellectButtons(); // 4385B0 (client: 43ab30)

	virtual void VMethod7() override; // 43005F (client: 431f52)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4302F5 (client: 4321e8)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 430318 (client: 43220b)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override;  // client: 4322bb

	VisCharSellectButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42fd5c (client: 431c50)

	void LoadBitmaps(); // 430850 (client: 43291c)
	void FreeBitmaps(); // 430a1a (client: 432ae6)
	void Init(); // 42fe62 (client: 431d56)
	void ResetMouseBoxes(); // 43072b (client: 4327f7)

public:
	VisCharSelect* parent_screen;
	CStringArray captions;
	CBmp64* buttons_bmp[4];
	void* gap_0x84[4];
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
	virtual ~VisCharSellectList(); // 4385E0 (client: 43ab80)

	virtual void VMethod7() override; // 430FEC (client: 433143)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 431609 (client: 433760)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 431764 (client: 4338bb)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 43182C (client: 433983)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4316A0 (client: 4337f7)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 431B95 (client: 433cec)

	VisCharSellectList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 430bbb (client: 432c87)

	void ResetListState(); // 430fad (client: 433104)
	void Init(); // 430cb3 (client: 432d7f)
	void SelectRow(int32_t row); // 4316c4 (client: 43381b)
	int32_t SelectPrevRow(); // 431a05 (client: 433b5c)
	int32_t SelectNextRow(); // 431a93 (client: 433bea)
	void UpdateArrows(CPoint pos, bool is_down); // 431c97 (client: 433dee)
	int32_t HitTest(CPoint pos); // 431872 (client: 4339c9)
	void LoadBitmaps(); // 4312b7 (client: 43291c)
	void FreeBitmaps(); // 43148a (client: 4335e1)

public:
	VisCharSelect* parent_screen;
	CRect row_rect;
	CRect list_area;
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	int32_t top_row;
	CBmp64* bmp_center_area;
	CBmp64* bmp_up_shine;
	CBmp64* bmp_up_shine_on;
	CBmp64* bmp_down_shine;
	CBmp64* bmp_down_shine_on;
	CBmp64* bmp_arrow_up;
	CBmp64* bmp_arrow_down;
	uint32_t gap_0xc0;
	uint32_t gap_0xc4;
	uint32_t gap_0xc8;
	uint32_t gap_0xcc;
	int32_t selected_row;
	int32_t hovered_row;
	int32_t renaming;
};
ASSERT_SIZE(VisCharSellectList, 0xdc);

//609730
class VisCharSellectStats : public CVisualObject {
public:
	virtual ~VisCharSellectStats(); // 438580 (client: 43aac0)

	virtual const char* GetHint() override; // 42FC61 (client: 431b55)
	virtual void VMethod7() override; // 42F7B9 (client: 4316ad)


	void ReadUnitStats(); // 42f6f3 (client: 4315ef)

	VisCharSellectStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharSelect* parent_screen); // 42f0ec (client: 430fe8)

	void Init(); // 42f1e8 (client: 4310e4)
	void LoadBitmaps(); // 42f4df (client: 4313db)
	void FreeBitmaps(); // 42f61a (client: 431516)

public:
	VisCharSelect* parent_screen;
	CBmp64* bmp_full_stats;
	CBmp64* bmp_left_up;
	CStringArray stat_labels;
	uint32_t shadow_body;
	uint32_t shadow_reaction;
	uint32_t shadow_mind;
	uint32_t shadow_spirit;
	CA16* cube_sprites;
	CRect stat_icon_rects[4];
};
ASSERT_SIZE(VisCharSellectStats, 0xd0);








//60c9b0
class VisInvBase : public CVisualObject {
public:
	VisInvBase(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4a4938 (client: 4a8968)

	virtual ~VisInvBase(); // 4A77D0 (client: 4ab830)
	virtual const char* GetHint() override; // 4a4cf8 (client: 4a8d28)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A5DE0 (client: 4a9e0f)
	virtual int32_t VMethod26(TokenEntry* o, int32_t idx); //4a554f (client: 4a957e)
	virtual int32_t VMethod27(TokenEntry* o); //4a56c3 (client: 4a96f2)
	virtual TokenEntry* VMethod28(uint32_t id); //4a57a1 (client: 4a97d0)
	virtual TokenEntry* VMethod29(TokenEntry* o, int32_t num); //4a58dc (client: 4a990b)
	virtual int32_t VMethod30(int32_t x, int32_t y); //4a79a0 (client: 4ab800)
	virtual int32_t VMethod31(const CPoint* pt); //4a7990 (client: 4ab810)
	virtual void VMethod32(CArray<TokenEntry*>* arr); //4a4da9 (client: 4a8dd9)
	virtual void VMethod33(CGameObject* uni); //4a4d05 (client: 4a8d35)
	virtual void VMethod34(); //4a4e28 (client: 4a8e58)
	virtual void VMethod35(); //4a4c2f (client: 4a8c5f)
	virtual TokenEntry* VMethod36(int32_t idx, int32_t num); //4a5aae (client: 4a9add)
	virtual int32_t VMethod37(int32_t idx); //4a5c39 (client: 4a9c68)
	virtual int32_t VMethod38(); //4a79b0 (client: 4ab820)

	int32_t GetVisibleStart(); //46fb90 (client: 472fd0)
	int32_t HasAnimatingCells(); //41f940 (client: 4207b0)
	void RefreshCellSprites(); //4a4ebc (client: 4a8eec)
	void InitGlowOffsets(); //4a4bcb (client: 4a8bfb)
	void ScrollCellsBack(); //4a5350 (client: 4a937f) in asm
	void ScrollCellsForward(); //4a51c9 (client: 4a91f8) in asm
	void TickCellAnims(); //4a5e12 (client: 4a9e41) in asm
	void DrawGlowTrail(int32_t x, int32_t y, int32_t idx); //4a5fab (client: 4a9fda) in asm

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
	virtual ~VisInvType1(); // 4A7830 (client: 4aba40)

	virtual const char* GetHint() override; // 4A765D (client: 4ab68b)
	virtual void VMethod7() override; // 4A6449 (client: 4aa478)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4A7441 (client: 4ab46f)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4A716F (client: 4ab19d)
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 4A741D (client: 4ab44b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4A6E57 (client: 4aae86)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4A70EC (client: 4ab11a)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4A6F44 (client: 4aaf73)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4a6f32 (client: 4aaf61)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4a715d (client: 4ab18b)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4a70da (client: 4ab108)

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
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B5F3F (client: 4ba90d)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b62a2 (client: 4bac6c)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4b62b4 (client: 4bac7e)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b62c6 (client: 4bac90)

	virtual int32_t VMethod30(int32_t x, int32_t y) override; // 4B4FA3 (client: 4b9973)
	virtual int32_t VMethod31(const CPoint* pt) override; // 4B4E88 (client: 4b9858)
	virtual void VMethod32(CArray<TokenEntry*>* arr) override;  // client: 4b9761
	virtual int32_t VMethod38() override;  // client: 4be810

	virtual void VMethod39();  // client: 4be800
	virtual void VMethod40(CArray<TokenEntry*>* arr);  // client: 4b977a
	virtual void VMethod41();  // client: 4be820
	virtual void VMethod42();  // client: 4be830

	int32_t EndItemDrag(); //4b5b81 (client: 4ba550) in asm
	int32_t StartItemDrag(int32_t idx); //4b5a6c (client: 4ba43b) in asm
	void FreeSpeechSfx(); //4b4ba9 (client: 461193) in asm
	void FreeGridSfx(); //4b4c1c (client: 4b95ec)
	void LoadGridSfx(); //4b4bc5 (client: 4b9595)
	void RefreshGrid(); //4b4d33 (client: 4b9703)
	void ClampVisibleStart(); //4b4fd1 (client: 4b99a1)

public:
	VisShop* shop;
	CSound snd_speech;
	CSound snd_put_on;
	CSound snd_put_off;
	CSound snd_scroll;
	int32_t gap_0x20c0; //unk type
	CRect* cell_rects;
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

	virtual int32_t VMethod37(int32_t idx) override;  // client: 4bc6e6
	virtual int32_t VMethod38() override;  // client: 4be840
	virtual void VMethod39() override;  // client: 4bb710

	virtual void VMethod41() override;  // client: 4bb006
	virtual void VMethod42() override;  // client: 4bb591

	void ClearCategoryGrids(); //4b7859 (client: 4bc223)
	void SelectCategoryGrid(int32_t category); //4b73e4 (client: 4bbdae)
	void DrawUpArrowHover(); //4b7264 (client: 4bbc2e) in asm
	void DrawUpArrowIdle(); //4b72c1 (client: 4bbc8b) in asm
	void DrawDownArrowHover(); //4b731e (client: 4bbce8) in asm
	void DrawDownArrowIdle(); //4b7381 (client: 4bbd4b) in asm

	VisInvExtType1(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b63f7 (client: 4badc1) in asm

public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	CBmp64* arrow1;
	CBmp64* arrow3;
	CBmp64* arrow2;
	CBmp64* arrow4;
	CBmp64* shop_inv;
	CArray<TokenEntry*> category_grids[4];
};
ASSERT_SIZE(VisInvExtType1, 0x2150);


//60d910
class VisInvExtType1Kaarg : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b69ee (client: 4bb3b8) in asm

	VisInvExtType1Kaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6cd0 (client: 4cb670) in asm
};
ASSERT_SIZE(VisInvExtType1Kaarg, 0x2150);


//60d850
class VisInvExtType1Druid : public VisInvExtType1 {
public:
	virtual void VMethod41() override; //4b6815 (client: 4bb1df) in asm

	VisInvExtType1Druid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c6c40 (client: 4cb5e0) in asm
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

	virtual int32_t VMethod38() override;  // client: 4be8d0
	virtual void VMethod39() override;  // client: 4bcacc

	void DrawUpArrowHover(); //4b860e (client: 4bcfd8) in asm
	void DrawUpArrowIdle(); //4b8666 (client: 4bd030) in asm
	void DrawDownArrowHover(); //4b86be (client: 4bd088) in asm
	void DrawDownArrowIdle(); //4b871c (client: 4bd0e6) in asm

	VisInvExtType2(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b7fe5 (client: 4bc9af) in asm
public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
};
ASSERT_SIZE(VisInvExtType2, 0x20ec);


//60d328
class VisInvExtType3 : public VisInvExtBase {
public:
	virtual ~VisInvExtType3(); // 4B9D10 (client: 4be6e0)

	virtual const char* GetHint() override; // 4B90CF (client: 4bda99)
	virtual void VMethod7() override; // 4B9155 (client: 4bdb1f)

	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B92F9 (client: 4bdcc3)

	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b93ae (client: 4bdd78)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B93CF (client: 4bdd99)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b9436 (client: 4bde00)

	virtual int32_t VMethod26(TokenEntry* o, int32_t idx) override; // 4B9457 (client: 4bde21)
	virtual int32_t VMethod27(TokenEntry* o) override; // 4B949A (client: 4bde64)

	virtual TokenEntry* VMethod36(int32_t idx, int32_t num) override;  // client: 4be606
	virtual int32_t VMethod37(int32_t idx) override;  // client: 4be15a
	virtual int32_t VMethod38() override;  // client: 4be8e0
	virtual void VMethod39() override;  // client: 4bd864

	virtual void VMethod41() override;  // client: 4bd70d
	virtual void VMethod42() override;  // client: 4bd80d

	virtual TokenEntry* VMethod43(int32_t id1, int32_t id2);  // client: 4bdf8f

	int32_t IsBuyGridFull(TokenEntry* entry); //4b91f9 (client: 4bdbc3) in asm
	VisInvExtType3(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4b8bba (client: 4bd584) in asm

	void ClearBuyEntries(); //4b970e (client: 4be0d8)
	int32_t DropDraggedItem(int32_t idx); //4b9b2f (client: 4be4f9) in asm
public:
	int32_t gap_0x20c8; //unk type
	CRect up_arrow_rect;
	CRect down_arrow_rect;
	CBmp64* shoptable;
	int32_t gap_0x20f0; //unk type
	CArray<TokenEntry*> entries;
};
ASSERT_SIZE(VisInvExtType3, 0x2108);


class VisTavSceneAnim {
public:
	void LoadAnimFrames(CStringArray* names); //4010ee (client: 4010ee) in asm
	void FreeAnimFrames(); //4014f2 (client: 4014f1) in asm
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
	virtual ~VisTavRightPanel(); // 4A3E40 (client: 4a7ec0)

	virtual const char* GetHint() override; // 4a4710 (client: 4a7eb0) in asm
	virtual void VMethod7() override; // 499dfa (client: 49de1a) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49a26e (client: 49e28e) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49a291 (client: 49e2b1) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 49a30a (client: 49e32a) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49a2e6 (client: 49e306) in asm

	VisTavRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4997c6 (client: 42cc93) in asm

	void LoadRightPanelBmps(); //499a67 (client: 49da87) in asm
	void ResetButtonStates(); //49a84e (client: 49e87d) in asm
	void UpdateButtonLabels(); //49a973 (client: 49e9a2) in asm
	void FreeRightPanelBmps(); //499cdf in asm - client FreeBitmaps restructured away (both client 283b FreeBitmaps fns belong to VisCharGenAction 42d704 / VisCharSellectButtons 432ae6)
	void InitRightPanel(); //4998c2 (client: 42cd99) in asm
	int32_t HitTestButton(CPoint pos); //49a873 (client: 43281c) in asm
	void UpdateHoveredButton(uint32_t wparam, CPoint pos); //49a8fa in asm - client FreeBitmaps restructured away (42d41b is VisCharGenAction::UpdateMouseOver)

public:
	VisTav* vis_tav; // 0x5c
	CStringArray texts; // 0x60
	CBmp64* bmp_btn_on[3]; // 0x74
	CBmp64* bmp_btn_off[3]; // 0x80
	CBmp64* bmp_area; // 0x8c
	CRect btn_rects[3]; // 0x90
	int32_t pressed_btn; // 0xc0
	int32_t hovered_btn; // 0xc4
};
ASSERT_SIZE(VisTavRightPanel, 0xc8);


// 60c560
class VisTavLeftPanel : public CVisualObject {
public:
	virtual ~VisTavLeftPanel(); // 4A3E10 (client: 4a7e40)

	virtual const char* GetHint() override; // 49934d (client: 49d36d) in asm
	virtual void VMethod7() override; // 497ed0 (client: 49bef0) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 497ec1 (client: 49bee1) in asm

	VisTavLeftPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //497990 (client: 49b9b0) in asm

	void LoadLeftPanelBmps(); //4995d1 (client: 49d5f1) in asm
	void FreeLeftPanelBmps(); //4996ab (client: 49d6cb) in asm
	void InitLeftPanel(); //497ace (client: 49baee) in asm
	void DrawUnitPanel(CUnit* unit); //497f82 (client: 49bfa2) in asm
	void SelectRosterEntry(int32_t idx); //497e06 (client: 49be26) in asm

public:
	VisTav* vis_tav; // 0x5c
	char picture_name[0x100]; // 0x60
	CBmp64* portrait_bmp; // 0x160
	CBmp256* hitmap; // 0x164
	CBmp64* bmp_left_stats; // 0x168
	CBmp64* bmp_left_picture; // 0x16c
	CRect left_btn_rect; // 0x170
	CRect right_btn_rect; // 0x180
};
ASSERT_SIZE(VisTavLeftPanel, 0x190);


//60c650
class VisTavScene : public CVisualObject {
public:
	virtual ~VisTavScene(); // 4A3E70 (client: 4a7ef0)

	virtual const char* GetHint() override; // 49cd2b (client: 4a0d59) in asm
	virtual void VMethod7() override; // 49b22f (client: 49f25e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49d8d9 (client: 4a1907) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 49da98 (client: 4a1ac6) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 49d8e8 (client: 4a1916) in asm

	virtual void VMethod26(); //49ced4 (client: 4a0f02) in asm
	virtual void VMethod27(); //49d1b0 (client: 4a11de) in asm
	virtual void VMethod28(); //49d2bb (client: 4a12e9) in asm
	virtual void VMethod29(); //49d5f9 (client: 4a1627) in asm

	VisTavScene(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //49ad56 (client: 49ed85) in asm

	void DrawQuestRoster(); //49bc23 (client: 49fc52) in asm
	int32_t HitTestRoster(CPoint* pos); //49cab8 (client: 4a0ae6) in asm
	void InitSceneRects(); //49af8c (client: 49efbb) in asm

public:
	VisTav* vis_tav; // 0x5c
	CRect roster_rects[3][6]; // 0x60
	CRect party_rects[3][3]; // 0x180
	CArray<CSprite256*> roster_sprites; // 0x210
	CArray<CSprite256*> avail_sprites; // 0x224
	CArray<CSprite256*> reserved_sprites; // 0x238
	CArray<uint32_t> avail_frames; // 0x24c
	CArray<uint32_t> reserved_frames; // 0x260
	CBmp64* bmp_man_back; // 0x274
	CBmp64* bmp_man_back_talk; // 0x278
	CBmp64* bmp_background; // 0x27c
	VisTavSceneAnim anims[4]; // 0x280
	CBmp64* bmp_lu_over; // 0x340
	CBmp64* bmp_ld_over; // 0x344
	CBmp64* bmp_ru_over; // 0x348
	CBmp64* quest_bmps[13]; // 0x34c
	uint8_t __gap_0x380[76];
};
ASSERT_SIZE(VisTavScene, 0x3cc);


//60c7f8
class VisTavSceneDruid : public VisTavScene {
public:
	virtual ~VisTavSceneDruid(); // 4A3F90 (client: 4a80b0)

	virtual void VMethod7() override; // 4a0673 (client: 4a46a1) in asm
	virtual void VMethod28() override; // 4a11e3 (client: 4a5211) in asm
	virtual void VMethod29() override; // 4a158f (client: 4a55bd) in asm

	VisTavSceneDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a05a0 (client: 4a45ce) in asm

public:
	CBmp64* bmp_flash;
	VisTavSceneAnim anims_druid[3]; // 0x3d0
	uint32_t ambient_last_tick;
	uint32_t ambient_interval;
};
ASSERT_SIZE(VisTavSceneDruid, 0x468);


//60c910
class VisTavSceneKaarg : public VisTavScene {
public:
	virtual ~VisTavSceneKaarg(); // 4A4090 (client: 4a81b0)

	virtual void VMethod7() override; // 4a2bb8 (client: 4a6be6) in asm
	virtual void VMethod28() override; // 4a37e7 (client: 4a7815) in asm
	virtual void VMethod29() override; // 4a3b25 (client: 4a7b53) in asm

	VisTavSceneKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisTav* tav); //4a2aa4 (client: 4a6ad2) in asm

public:
	VisTavSceneAnim anims_kaarg[5]; // 0x3cc
	uint32_t script_frame_index;
	uint32_t ambient1_last_tick;
	uint32_t ambient1_interval;
	uint32_t ambient2_last_tick;
	uint32_t ambient2_interval;
};
ASSERT_SIZE(VisTavSceneKaarg, 0x4d0);


//60c6d8
class VisTav : public VisScreen {
public:
	virtual ~VisTav(); // 4A3F10 (client: 4a7fd0)

	virtual void VMethod7() override; // 49E044 (client: 4a2072)
	virtual void VMethod8(CRect* rect) override; // 49e075 (client: 4a20a3)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 49E082 (client: 4a20b0)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 49E261 (client: 4a228f)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 49E233 (client: 4a2261)

	virtual void VMethod26() override; // 49DDAF (client: 4a3549)
	virtual void VMethod28() override; // 49E34F (client: 4a237d)
	virtual void DoClose(uint32_t code) override; // 49EB6C (client: 4a2b9a)

	virtual void VMethod30(); // 49F298 (client: 4a32c6)
	virtual void VMethod31(); // 49F3D7 (client: 4a3405)

	void CloseDialog(); // client: 4a2e1a (client: 4a2e1a)
	void SelectNextPartyUnit(); //49ee36 (client: 4a2e64) in asm
	void SelectPrevPartyUnit(); //49eecd (client: 4a2efb) in asm
	int32_t FindEntryIndexForUnit(CUnit* unit); //49e2e3 (client: 4a2311) in asm
	void ShowHireResponseDialog(); //49ef63 (client: 4a2f91) in asm
	void RetalkAvailNpc(); //49f0da (client: 4a3108) in asm
	void ShowReservedNpcTalk(); //49f179 (client: 4a31a7) in asm

	VisTav(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //49db79 (client: 4a1ba7) in asm

public:
	BigStruct2* map_context;
	VisCharInfo* info_panel;
	VisTavLeftPanel* left_panel;
	VisTavRightPanel* right_panel;
	int32_t gap_0x78; //unk type
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
	int32_t inn_id;
	int32_t quest_alt_close;
};
ASSERT_SIZE(VisTav, 0x140);

//60c768
class VisTavDruid : public VisTav {
public:
	virtual ~VisTavDruid(); // 4A3F40 (client: 4a8040)

	virtual void VMethod26() override; // 49F51B (client: 4a8040)
	virtual void VMethod28() override; // 49FAAA (client: 4a3ad8)
	virtual void DoClose(uint32_t code) override; // 4A0278 (client: 4a42a6)
	virtual void VMethod30() override; // 49F7B0 (client: 4a37de)
	virtual void VMethod31() override; // 49F961 (client: 4a398f)

	VisTavDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 49f4e6 (client: 4544a0) in asm
public:
	SfxSample* snd_druid[11];
};
ASSERT_SIZE(VisTavDruid, 0x16c);

//60c880
class VisTavKaarg : public VisTav {
public:
	virtual ~VisTavKaarg(); // 4A4040 (client: 4a8160)

	virtual void VMethod26() override; // 4A190D (client: 4a1ddd)
	virtual void VMethod28() override; // 4A1BA2 (client: 4a5bd0)
	virtual void DoClose(uint32_t code) override; // 4A2448 (client: 4a6476)
	virtual void VMethod30() override; // 4A26D9 (client: 4a6707)
	virtual void VMethod31() override; // 4A2873 (client: 4a68a1)

	VisTavKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4a18d8 (client: 4544a0) in asm
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
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4CADD2 (client: 4cf758)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4CAF70 (client: 4cf8f6)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4cae94 (client: 4cf81a)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CAC9B (client: 4cf621)

	void ClearSpellSelection(); // client: 4cf3f7 (client: 4cf3f7)

	int32_t HitTestSpellCell(CPoint* pos); //4ca7c2 (client: 4cf150) in asm
	void SelectScrollEntry(int32_t idx); //4ca925 (client: 4cf2b3) in asm
	int32_t IsSpellAvailable(int32_t id); //4ca89b (client: 4cf229)
	int32_t IsSpellEffectActive(int32_t id); //4ca8e0 (client: 4cf26e)
	int32_t GetActiveSlot(); //41f9b0 (client: 4208e0)
	int32_t HasSelectedSpell(); //41f9e0 (client: 420910)
	int32_t NeedsUnitTarget(); //4caaa7 (client: 4cf435)

	VisSpellBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4c99c7 (client: 4ce357)

public:
	int32_t dirty;
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
	virtual void VMethod28(); // 4be5de (client: 4c2fa3)
	virtual void VMethod29(); // 4be6df (client: 4c30a4)
	virtual void VMethod30(); // 4be7e0 (client: 4c31a5)
	virtual void VMethod31(int32_t arg); // 4be45d (client: 4c2e22)
	virtual void VMethod32(int32_t arg); // 4be541 (client: 4c2f06)
	virtual void VMethod33(); // 4be8b0 (client: 4c3275)
	virtual void VMethod34(); // 4be92d (client: 4c32f2)
	virtual void VMethod35(); // 4be9aa (client: 4c336f)
	virtual void VMethod36(); // 4bea01 (client: 4c33c6)
	virtual int32_t VMethod37(int32_t arg); // 4bf4e8 (client: 4c3eab)
	virtual int32_t VMethod38(); // 4c6bc0 (client: 4cb560)

	VisShopCompass(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4bd495 (client: 4c1e5a)

	void AdvanceDirectionFrames(int32_t dir); // 4bf63f (client: 4c4002)

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
	virtual ~VisShopCompassDruid(); // 4C6A30 (client: 4cb3d0)

	virtual const char* GetHint() override; // 4C3690 (client: 4c8047)
	virtual void VMethod7() override; // 4C30B4 (client: 4c7a6d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4C359D (client: 4c7f56)
	virtual void VMethod26() override; // 4C2882 (client: 4c723b)
	virtual void VMethod27() override; // 4C2B29 (client: 4c74e2)
	virtual void VMethod28() override; // 4C2D8A (client: 4c775c)
	virtual void VMethod29() override; // 4C2DA3 (client: 4c775c)
	virtual void VMethod30() override; // 4C2DBC (client: 4c7775)
	virtual void VMethod31(int32_t arg) override; // 4C2CCE (client: 4c7687)
	virtual void VMethod32(int32_t arg) override; // 4C2CED (client: 4c76a6)
	virtual void VMethod33() override; // 4C2F00 (client: 4c78b9)
	virtual void VMethod34() override; // 4C2F7D (client: 4c7936)
	virtual void VMethod35() override; // 4C2FFA (client: 4c79b3)
	virtual void VMethod36() override; // 4C3051 (client: 4c7a0a)
	virtual int32_t VMethod37(int32_t arg) override; // 4C34E6 (client: 4c7e9f)

	VisShopCompassDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c247b (client: 4c6e34) in asm

	int32_t IsDruidShopAvailable(); // 4c2842 (client: 4c71fb)

public:
	int32_t gap_0x258; // 0x258
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
	virtual ~VisShopCompassKaarg(); // 4C6AD0 (client: 4cb470)

	virtual const char* GetHint() override; // 4C6800 (client: 4cb19b)
	virtual void VMethod7() override; // 4C608E (client: 4caa2b)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4C671F (client: 4cb0bc)
	virtual void VMethod26() override; // 4C4A78 (client: 4c942f)
	virtual void VMethod27() override; // 4C5A80 (client: 4ca41d)
	virtual void VMethod28() override; // 4C5D64 (client: 4ca701)
	virtual void VMethod29() override; // 4C5D7D (client: 4ca71a)
	virtual void VMethod30() override; // 4C5D96 (client: 4ca733)
	virtual void VMethod31(int32_t arg) override; // 4C5CA8 (client: 4ca645)
	virtual void VMethod32(int32_t arg) override; // 4C5CC7 (client: 4ca664)
	virtual void VMethod33() override; // 4C5EDA (client: 4ca877)
	virtual void VMethod34() override; // 4C5F57 (client: 4ca8f4)
	virtual void VMethod35() override; // 4C5FD4 (client: 4ca971)
	virtual void VMethod36() override; // 4C602B (client: 4ca9c8)
	virtual int32_t VMethod37(int32_t arg) override; // 4C65BA (client: 4caf57)

	VisShopCompassKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4c4693 (client: 4c904a) in asm

public:
	int32_t gap_0x258; // 0x258
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
	virtual const char* GetHint() override { return nullptr; } //4c04a4 (client: 4c4e5d)
	virtual void VMethod7() override; //4c04b1 (client: 4c4e6a)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; //4c10c4 (client: 4c5a7d)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4c1134 (client: 4c5aed)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; //4c11c2 (client: 4c5b7b)
	virtual int32_t VMethod30(); //4c6bd0 (client: 4cb570)

	void ReleaseBmp(); //4c0352 (client: 4c4d0b)
	void UpdateHoveredState(uint32_t wparam, CPoint pos); //4c14c4 (client: 4c5e7d)
	int32_t GetButtonAt(CPoint pos); //4c137d (client: 4c5d36)
	void LoadBmp(); //4c0088 (client: 4c4a43)
	void ResetSelected(); //4c1358 (client: 4c5d11)

	VisShopButtons(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisShop* shop); //4bfa5a (client: 4c4415)

public:
	VisShop* shop; // 0x5c
	int32_t gap_0x60;
	CBmp64* button_bmps[4]; // 0x64
	CBmp64* menu_bmp; // 0x74
	CRect outer_rects[4]; // 0x78
	CRect inner_rects[4]; // 0xb8
	int32_t pressed_btn; // 0xf8
	int32_t hovered_btn; // 0xfc
};
ASSERT_SIZE(VisShopButtons, 0x100);


void __cdecl SortTokenEntries(CArray<TokenEntry*>* arr); //4ba1cc (client: 4beb9c) in asm
void __cdecl InsertThousandsSeparators(CString* str); //476987 (client: 47a6d3) - insert thousands separators


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
	virtual void VMethod32();  // client: 4c1c16
	virtual CString VMethod33(); // 4c6b90 (client: 4cb530)


	void CloseShopDialog(); // client: 4c16d3 (client: 4c16d3)
	void DrawInfoPanelEdge(); //4bad1d (client: 4bf6ea) in asm
	void FreeShopGfx(); //4bb4fb (client: 4bfec8)
	void SelectNextPartyUnit(); //4bb895 (client: 4c0262)
	void SelectPrevPartyUnit(); //4bba06 (client: 4c03d3)
	void LoadShopGfx(); //4bb102 (client: 4bfacf)
	void SetAssortmentCategory(int32_t category, CArray<TokenEntry*>* arr); //4bbbd6 (client: 4c05a3)
	void UpdateShopTips(); //4bc8d7 (client: 4c12a4)
	void ReturnPendingBuyItems(); //4bcc38 (client: 4c1604)
	void CancelItemDrag(); //4bc97b (client: 4c1348)
	void ShowSpellPanel(); //4bcd4b (client: 4c171c)
	void ShowBuyPanel(); //4bcd79 (client: 4c174a)
	void RecalcShopGold(); //4bcda0 (client: 4c1771)
	void ResetInfoPanelButtons(); //4bcea4 (client: 4c1875)
	void CommitSell(); //4bcaf1 (client: 4c14bd)
	int CommitBuy(); //4bcb63 (client: 4c152f)
	void CancelAllPendingBuys(); //4bcce1 (client: 4c16b2)

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
	int32_t gap_0x134;
	int32_t gap_0x138;
	int32_t gap_0x13c;
	int32_t gap_0x140;
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
	virtual ~VisShopDruid(); // 4C69B0 (client: 4cb350)

	virtual void VMethod26() override; // 4C15E6 (client: 4c5f9f)
	virtual void VMethod28() override; // 4C1E5C (client: 4c6815)
	virtual void VMethod30() override; // 4C1958 (client: 4c6311)
	virtual void VMethod31() override; // 4C1B8B (client: 4c6544)
	virtual void VMethod32() override; // 4C1D25 (client: 4c66de)
	virtual CString VMethod33() override; // 4C6BE0 (client: 4cb580)

	VisShopDruid(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c15ad (client: 4c5f66)
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
	virtual ~VisShopKaarg(); // 4C6A50 (client: 4cb3f0)

	virtual void VMethod26() override; // 4C37FE (client: 4c81b5)
	virtual void VMethod28() override; // 4C3B70 (client: 4c8527)
	virtual void VMethod30() override; // 4C4170 (client: 4c8b27)
	virtual void VMethod31() override; // 4C43A3 (client: 4c8d5a)
	virtual void VMethod32() override; // 4C453D (client: 4c8ef4)
	virtual CString VMethod33() override; // 4C6C10 (client: 4cb5b0)

	VisShopKaarg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* btm = nullptr); //4c37c5 (client: 4c5f66)
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
	virtual ~VisTown(); // 4D6B80 (client: 4db490)

	virtual const char* GetHint() override; // 4D1602 (client: 4d5f33)
	virtual void VMethod7() override; // 4CE886 (client: 4d31cd)
	virtual void VMethod8(CRect* rect) override; // 4ce719 (client: 4d3063)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4CE3B8 (client: 4d2d05)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4CE4E5 (client: 4d2e32)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4CE50A (client: 4d2e57)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4CE4AF (client: 4d2dfc)

	virtual void VMethod26() override; // 4CDDAD (client: 4d26fd)
	virtual void VMethod28() override; // 4CE002 (client: 4d2952)
	virtual void DoClose(uint32_t code) override; // 4CE310 (client: 4d2c5d)

	virtual void VMethod30(); // 4D1723 (client: 4d6054)
	virtual void VMethod31(); // 4D184A (client: 4d617b)

	virtual void VMethod32(); // 4D1709 (client: 4d603a)
	virtual int32_t VMethod33(CPoint pos); // 4D10F0 (client: 4d5a27)
	virtual void VMethod34(CPoint pos); // 4D1429 (client: 4d5d5a)
	virtual void VMethod35(); // 4D12CB (client: 4d5c02)
	virtual void VMethod36(); // 4CEEEF (client: 4d3836)
	virtual void VMethod37(); // 4CFD0E (client: 4d4655)
	virtual void VMethod38(); // 4D0E31 (client: 4d5776)
	virtual void VMethod39(); // 4D15E3 (client: 4d5f14)

	int32_t StartBirdFlight(); //4ce64b (client: 4d2f98) in asm
	void DrawBirdFlock(); //4ce726 (client: 4d3070) in asm

	void AdvanceGuardAnim(); //4d06aa (client: 4d4ff1) in asm
	void AdvanceTavernAnim(); //4d07e8 (client: 4d512f) in asm
	void AdvanceSignAnim(); //4d0884 (client: 4d56e7) in asm
	void AdvanceGateAnim(); //4d0913 (client: 4d525a) in asm
	void AdvanceStarsAnim(); //4d0ad7 (client: 4d541c) in asm
	void AdvanceFighterAnim(); //4d0b95 (client: 4d54da) in asm
	void AdvanceMageAnim(); //4d0c6e (client: 4d55b3) in asm
	void AdvanceShopAnim(); //4d0d47 (client: 4d568c) in asm
	void AdvanceFlugelAnim(); //4d0da2 (client: 4d51cb) in asm
	void AdvanceBlackBirdAnim(); //4d0f66 (client: 4d58ab) in asm
	void AdvanceHorseAnim(); //4d1002 (client: 4d5940) in asm
	void AdvanceDervishAnim(); //4d109e (client: 4d59d5) in asm

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
	virtual ~VisTownDruid(); // 4D6BB0 (client: 4db4c0)

	virtual void VMethod7() override; // 4D2E99 (client: 4d77ca)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D3F18 (client: 4d883c)
	virtual void VMethod28() override; // 4D1E80 (client: 4d67b1)
	virtual void VMethod30() override; // 4D2AFF (client: 4d7430)
	virtual void VMethod31() override; // 4D2D1B (client: 4d764c)
	virtual void VMethod32() override; // 4D2AE3 (client: 4d7414)
	virtual void VMethod34(CPoint pos) override; // 4D359C (client: 4d7ec8)
	virtual void VMethod35() override; // 4D3922 (client: 4d824e)
	virtual void VMethod36() override; // 4D2153 (client: 4d6a84)
	virtual void VMethod37() override; // 4D27FF (client: 4d7130)
	virtual void VMethod38() override; // 4D3EA7 (client: 4d87cb)

	void AdvanceTavernAnim(); //4d3384 (client: 4d7cb5) in asm
	void AdvanceShopAnim(); //4d3435 (client: 4d7d66) in asm
	void AdvanceBugAnim(); //4d34b6 (client: 4d7de7) in asm
	void AdvanceLizardAnim(); //4d3520 (client: 4d7e4e) in asm

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
	virtual ~VisTownKaarg(); // 4D6C60 (client: 4db570)

	virtual const char* GetHint() override; // 4D6A1C (client: 4db32d)
	virtual void VMethod7() override; // 4D57B9 (client: 4da0dd)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4D6802 (client: 4db113)
	virtual void VMethod28() override; // 4D4592 (client: 4d8eb6)
	virtual void VMethod30() override; // 4D54CF (client: 4d9df3)
	virtual void VMethod31() override; // 4D5682 (client: 4d9fa6)
	virtual void VMethod32() override; // 4D54B5 (client: 4d9dd9)
	virtual void VMethod34(CPoint pos) override; // 4D5C9A (client: 4da5be)
	virtual void VMethod35() override; // 4D5EF0 (client: 4da814)
	virtual void VMethod36() override; // 4D4866 (client: 4d918a)
	virtual void VMethod37() override; // 4D5112 (client: 4d9a36)
	virtual void VMethod38() override; // 4D6786 (client: 4db097)

	void AdvanceGateAnim(); //4d6480 (client: 4dad9c) in asm
	void AdvanceGirl1Anim(); //4d65dd (client: 4daef7) in asm
	void AdvanceGirl2Anim(); //4d6655 (client: 4daf6c) in asm
	void AdvanceGuardAnim(); //4d66ca (client: 4dafe1) in asm
	void AdvanceDervishAnim(); //4d6728 (client: 4db03c) in asm

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
	virtual void VMethod8(CRect* rect) override; // 43c9a8 (client: 43edb8)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43C595 (client: 43e9a5)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 43C5F5 (client: 43ea05)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43C5D4 (client: 43e9e4)

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
	virtual ~VisCharGenStats(); // 428f12 (client: 42ae32) in asm

	virtual const char* GetHint() override; // 42a43b (client: 42c351) in asm
	virtual void VMethod7() override; // 429d03 (client: 42bc1b) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42a312 (client: 42d1f6) in asm
	virtual int32_t OnWmUser(uint32_t wparam, CPoint pos) override; // 42a3f2 (client: 42c308) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42a335 (client: 42c24b) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42a3a8 (client: 42c2be) in asm
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 42a3ce (client: 42c2e4) in asm

	VisCharGenStats(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //428e51 (client: 42ad71) in asm

	void ReadUnitStats(); // 429c39 (client: 42bb59) in asm
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
	CRect label_rects[4];
	CRect points_rect;
	CRect areas[4][3]; // 0xb4: per-stat rects [value, plus_btn, minus_btn]
	CBmp64* stat_btn_pairs[4][2]; // 0x174: per-stat [plus, minus] button bitmap pair
	CBmp64* stat_button_bmps[10]; // 0x194: plon/ploff/pnlon/pnloff/pdisable/mlon/mloff/mnlon/mnloff/mdisable
	CStringArray texts; // 0x1bc
	int32_t stat_body; // 0x1d0
	int32_t stat_reaction; // 0x1d4
	int32_t stat_mind; // 0x1d8
	int32_t stat_spirit; // 0x1dc
	int32_t gap_0x1e0[4];
	int32_t stat_points;
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
	void FreeBitmaps(); // 42ab9c (client: 42cab2) in asm
public:
	VisCharGen* parent_screen; // 0x5c
	CBmp64* bmp; // 0x60
	uint8_t gap_0x64[0x44];
};
ASSERT_SIZE(VisCharGenFullStats, 0xa8);

//6095b8
class VisCharGenAction : public CVisualObject {
public:
	virtual ~VisCharGenAction(); // 42ae19 (client: 42ae32) in asm

	virtual void VMethod7() override; // 42b028 (client: 42cf3e) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42b2e0 (client: 42d1f6) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42b303 (client: 42d219) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42b392 (client: 42d2a8) in asm

	VisCharGenAction(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, VisCharGen* parent); //42ad7d (client: 42cc93) in asm

	void LoadBitmaps(); // 42b57e (client: 42d494) in asm
	void FreeBitmaps(); // 42b7ee (client: 42d704) in asm
	void ResetMouseBoxes(); // 42b455 (client: 42d36b) in asm
private:
	int32_t HitTest(CPoint pt); // 42b47a (client: 42d390) in asm
	void UpdateMouseOver(uint32_t wparam, CPoint pos); // 42b505 (client: 42d41b) in asm
	void Init(); // 42ae83 (client: 42cd99) in asm
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
	virtual ~VisCharGenSkills(); // 42ba30 (client: 422210) in asm

	virtual const char* GetHint() override; // 42dbce (client: 42fae2) in asm
	virtual void VMethod7() override; // 42bb5a (client: 42da70) in asm
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42bfcc (client: 42dee1) in asm
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 42c072 (client: 42df86) in asm
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42c1ab (client: 43e1da) in asm (Ghidra-labeled accHitTest)

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
	CPoint slot_positions[5]; // 0xb4: slot positions (parent-screen coords)
	CPoint slot_sizes[5]; // 0xdc: slot sizes
	uint8_t color_keys[8]; // 0x104: per-slot mask color keys
	int32_t slot_states[5]; // 0x10c: per-slot state (bit0 selected, bit1 hover)
	SfxSample* skill_sounds[5]; // 0x120: skill sounds
	int32_t selected_slot; // 0x134
};
ASSERT_SIZE(VisCharGenSkills, 0x138);

//6096a8
class VisCharGen : public VisScreen {
public:
	virtual ~VisCharGen(); // 4383F0 (client: 43a9f0)

	virtual void VMethod7() override; // 42EB29 (client: 430a27)
	virtual void VMethod8(CRect* rect) override; // 42eb7f (client: 430a7d)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 42E058 (client: 42ff6c)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 42E196 (client: 4300aa)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 42EA01 (client: 4308ff)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 42EA4F (client: 43094d)

	virtual void VMethod26() override; // 42DDEC (client: 42fd00)
	virtual void VMethod28() override; // 42E218 (client: 43012c)
	virtual void DoClose(uint32_t code) override; // 42E7BC (client: 4306d0)

	VisCharGen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //42dcd9 (client: 42fbed) in asm

	void OnPrevFace(); // 42edc2 (client: 430cbf) in asm
	void OnNextFace(); // 42ebe0 (client: 430ade) in asm
	void RollStats(); // 42e92a (client: 43083e) in asm
	void OnClickFwd(); // 42eb8c (client: 430a8a) in asm
	void OnClickBack(); // 42ebab (client: 430a8a) in asm
	void ShowTipHint(); // 42ea87 (client: 430985) in asm
	void LoadSamples(); // 42efb4 (client: 430eb0) in asm
	void FreeSamples(); // 42f021 (client: 430f1d) in asm
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
	int32_t gap_0x9c;
	CArray<uint16_t> face_sets[4];
	uint32_t mage_flag;
	int32_t female_face;
	int32_t selected_face;
	int32_t selected_face_set;
	int32_t tips_step;
	uint32_t active_flag;
};
ASSERT_SIZE(VisCharGen, 0x108);







//60a170
class VisDropGold : public VisScreen {
public:
	virtual ~VisDropGold(); // 441c63 (client: 444232) (44F870 is the ??_G thunk)

	virtual void VMethod9() override; // 441F1F (client: 4444f1)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 441F3C (client: 44450e)

	virtual void VMethod26() override; // 441C7F (client: 44424e)
	virtual void VMethod28() override; // 441FA3 (client: 444575)
	virtual void DoClose(uint32_t code) override; // 441FEE (client: 4445c0)

	void SetSelection(int32_t idx); //4a7a30 (client: 4aba60) in asm

	VisDropGold(int32_t _id, int32_t _x, int32_t _y); // 441bec (client: 4441bb) in asm
public:
	int32_t selection;
	char amount_text[100];
};
ASSERT_SIZE(VisDropGold, 0xd0);




//60cfe8
class VisCharInfo : public CVisualObject {
public:
	virtual ~VisCharInfo(); // 4B47B0 (client: 4b9180)

	virtual const char* GetHint() override; // 4B19B0 (client: 4b62e2)
	virtual void VMethod7() override; // 4B2B3D (client: 4b746f)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B201C (client: 4b694e)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B289E (client: 4b71d0)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4b2215 (client: 4b6b47)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B2346 (client: 4b6c78)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B2239 (client: 4b6b6b)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b2227 (client: 4b6b59)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B2857 (client: 4b7189)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b2334 (client: 4b6b6b)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4B2AD5 (client: 4b7407)

	virtual int32_t VMethod26(int32_t a); //4b38af (client: 4b81df)
	virtual int32_t VMethod27(int32_t a); //4b3a0d (client: 4b8374)
	virtual int32_t VMethod28(); // 4b4830 (client: 4b9200)

	void UpdateCursor(); //4b36b4 (client: 4b7fe4)

	VisCharInfo(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b16c3 (client: 4b5ff5) in asm
public:
	BigStruct2* map_context;
	int32_t dirty;
	int32_t gap_64;
	int32_t selection_panel_state;
	int32_t spell_panel_state;
	int32_t info_mode;
	CBmp64* bitmap;
	CBmp256* hitmap;
	char picturename[256];
};
ASSERT_SIZE(VisCharInfo, 0x17c);




//60cd30
class VisMainMenu : public VisScreen {
public:
	virtual ~VisMainMenu(); // 4AAAB6 (client: 4aeae6)

	virtual void VMethod7() override; // 4AB28F (client: 4af2bf)
	virtual void VMethod8(CRect* rect) override; // 4ab411 (client: 4af441)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AB41E (client: 4af44e)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4AB88C (client: 4af8bc)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4AB8AF (client: 4af8df)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4AB8F3 (client: 4af923)

	virtual void VMethod26() override; // 4AAB71 (client: 4aeba1)
	virtual void VMethod28() override; // 4ABA8D (client: 4afabd)
	virtual void DoClose(uint32_t code) override; // 4ABB14 (client: 4afb44)

	void UpdateButtonState(uint32_t wparam, CPoint mouse); // 4ab45d (client: 4af48d)
	void LoadGraphics(); // 4aada7 (client: 4aedd7)
	void LoadSfx(); // 4abb5e (client: 4afb8e)
	void FreeGraphics(); // 4ab07b (client: 4af0ab)
	void FreeSfx(); // 4abb87 (client: 4afbb7)

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
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 432CE3 (client: 434e1a)
	virtual int32_t OnChar(uint32_t wparam) override; // 432D1B (client: 434e52)

	CString GetText(); // 438d80 (client: 43add0)
	void SetText(const CString& text); // 432c03 (client: 434d3a)

	void BackspaceChar(); //432b80 (client: 434cb7) in asm
	void AcceptChar(uint8_t ch); //432b33 (client: 434c63) in asm
	void InitTextBox(); //432ad1 (client: 434c02) in asm

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
	virtual ~VisStartGame(); // 4386B0 (client: 43ac90)

	virtual const char* GetHint() override; // 437E13 (client: 439f45)
	virtual void VMethod7() override; // 435D72 (client: 437ea4)
	virtual void VMethod8(CRect* rect) override; // 438dc0 (client: 43ac80)
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

	void PlayLabelInputSound(); //4382d9 (client: 43a5d4) in asm

	void ClearDifficultySelectedFlags(); // 435743 (client: 43787b)
	void ClearPortraitSelectedFlags(); // 43578c (client: 4378c3)
	void ClearDifficultyHoverFlags(); // 4357d5 (client: 43790b)
	void ClearPortraitHoverFlags(); // 43581e (client: 437953)

	void LoadBitmaps(); // 43438b (client: 4364c3)
	void LoadSamples(); // 437ff4 (client: 43a126)
	void FreeSamples(); // 4380d3 (client: 43a205)
	void ResetHoverState(); // 435d26 (client: 437e58)
	void FreeBitmaps(); // 435097 (client: 4371cf)

	VisStartGame(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //43305b (client: 435193) in asm

	CString GetName() { return this->char_name; } // 4973f0 (client: 43add0)
	CString GetClan() { return this->clan_name; } // 497430 (client: 49aa50)
	int32_t GetSelectedDifficulty() { return this->selectedDifficultyIndex; } //4973d0 (client: 49a9f0)
	void SetSelectedDifficulty(int diff) { this->selectedDifficultyIndex = diff; } //497370 (client: 49ab80)
	int32_t GetSelectedPortraitPlayerType() { return PortraitPlayerTypes[this->selectedPortraitIndex]; } //4973b0 (client: 49a9d0)
	void SetSelectedPortraitPlayerType(int id) { this->selectedPortraitIndex = id; } //497350 (client: 49ab60)
	void SetCharName(const CString& name) { this->char_name = name; } //497390 (client: 434d3a)

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
	int32_t gap_0x1bc;
	int32_t gap_0x1c0;
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
	int32_t gap_0x204;
	int32_t selectedPortraitIndex; // 0x208;
	int32_t selectedDifficultyIndex; // 0x20c;
	int32_t committedDifficultyIndex; // 0x210;
	int32_t committedPortraitIndex; // 0x214;
	int32_t tipsProgress; // 0x218;
	int32_t dialogActiveFlag; // 0x21c;

	static const int32_t PortraitPlayerTypes[4];
};
ASSERT_SIZE(VisStartGame, 0x220);

//60b180
class VisMenuWnd : public VisWindow {
public:
	virtual ~VisMenuWnd();  // client: 453670

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440e54 (client: 4432bb)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //440ece (client: 443335)

	VisMenuWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, uint32_t unk, const CRect &_r); //451990 (client: 453430)

	void AddElement(CVisualObject *obj, int32_t height); //440e11 (client: 443278)
public:
	uint32_t gap_0x68;
	CRect element_rect;
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
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } //4dda31 (client: 4e23b2)
	virtual int32_t OnChar(uint32_t wparam); //4dd9b3 (client: 4e2336)

	MenuButton(int32_t _id, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //4509c0 (client: 453390)
	MenuButton(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //450bf0 (client: 452b50)
};
ASSERT_SIZE(MenuButton, 0x78);


//609dc8
class LoadGameWindow : public VisWindow {
public:
	virtual ~LoadGameWindow() {} //44f4b0 (client: 452fd0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //43f63c (client: 441aa0)
	virtual void VMethod26() override; //43ed94 (client: 4411f8)

	LoadGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43ecfb (client: 44115f)
public:
	SaveFileInfo* out_info;
	CStringArray file_names;
	CStringArray save_times;
	int32_t selected_index;
};
ASSERT_SIZE(LoadGameWindow, 0x98);

//609e50
class SaveGameWindow : public VisWindow {
public:
	virtual ~SaveGameWindow() {} //44f540 (client: 452fd0)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //440726 (client: 442b8d)
	virtual void VMethod26() override; //43fb2c (client: 441f90)

	SaveGameWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitmap, SaveFileInfo* unk); //43fa42 (client: 441ea6)
	void CheckInput(); //440cc4 (client: 44312b)

	static int __cdecl Compare(void const* a, void const* b); //43faf4 (client: 441f58)
public:
	SaveFileInfo* out_info;
	CStringArray file_names;
	CStringArray save_times;
	int32_t selected_index;
	char* empty_cell_text;
};
ASSERT_SIZE(SaveGameWindow, 0x9c);

//609ed8
class VisButtonSaveGame : public VisButton {
public:
	virtual ~VisButtonSaveGame(); //44f650 (client: 453310)

	virtual int32_t OnKeyDown(uint32_t wparam) override; //44f5f0 (client: 4532b0)

	VisButtonSaveGame(int32_t _id, const RECT& r, const char* _caption, CGameFont* _font, uint16_t* _clr, int32_t _msgid, int32_t _charid, const char* hint); //44f5a0 (client: 452b50)
};
ASSERT_SIZE(VisButtonSaveGame, 0x78);


//609d38
class HintedListBox : public VisListBox {
public:
	virtual ~HintedListBox() {} //44f410 (client: 452f30)

	virtual const char* GetHint() override; //43ec67 (client: 4410cb)

	HintedListBox(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); //43ec05 (client: 441069)

	void SetHints(CStringArray* names) { this->hints = names; } //43ec4e (client: 4410b2)
public:
	CStringArray* hints = nullptr;
};
ASSERT_SIZE(HintedListBox, 0x98);


//60a200
class GameOptionsWindow : public VisWindow {
public:
	virtual ~GameOptionsWindow() {} //443cbb (client: 406f89)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //443cd7 (client: 4462a9)
	virtual void VMethod26() override; //442646 (client: 444c18)

	GameOptionsWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b) : VisWindow(_id, l, t, r, b, nullptr) {}; //44260f (client: 444be1)
};
ASSERT_SIZE(GameOptionsWindow, 0x68);


//609fd8
class EndGameMenu : public VisMenuWnd {
public:
	EndGameMenu(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameBitmap* _bitm, uint32_t unk, const CRect& _r, int32_t unk2); //441357 (client: 453430)
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
void ExecuteAdminChatCommand(const char* cmd);  // client: 4517f9

// 60adf8, radio checkbox on the server screen ("save on server" toggle)
class VisServerScreenRadio : public VisRadioType1 {
public:
	virtual ~VisServerScreenRadio(); // 450970 (client: 454730) (complete dtor 4509a0)

	VisServerScreenRadio(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); // 450920 (client: 452b50)

	virtual void VMethod7() override; // 4db3ad (client: 4dfd32) in asm
	virtual int32_t GetIndex(int32_t y) override; // 4db615 (client: 4dff9a) in asm
};
ASSERT_SIZE(VisServerScreenRadio, 0x8c);

//60aef0
class VisServerScreen : public VisScreen {
public:
	virtual ~VisServerScreen(); // 450AB0 (client: 4547b0)

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
	void FreePathmap(); // 4735e2 (client: 42cab2)
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
	virtual ~VisGlobalMap(); // 473D70 (client: 477c20)

	virtual const char* GetHint() override; // 472729 (client: 476487)
	virtual void VMethod7() override; // 4714E7 (client: 475245)
	virtual void VMethod8(CRect* rect) override; // 471f08 (client: 475c66)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 472CC5 (client: 476a15)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 472D99 (client: 476ae9)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 473038 (client: 476d88)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 474f10 (client: 477c10)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 473077 (client: 476dc7)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 473175 (client: 476ec5)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4726E6 (client: 47642a)
	virtual int32_t OnChar(uint32_t wparam) override; // 4726CC (client: 476444)

	virtual void VMethod26() override; // 47031D (client: 47407b)
	virtual void VMethod28() override; // 47115D (client: 474ebb)
	virtual void DoClose(uint32_t code) override; // 471367 (client: 4750c5)

	void SetTravelOrigin(const CPoint& pt) { this->currentLocationPoint = pt; } //4972a0 (client: 4210f0)

	void RebuildScenarioLocations(); //47024a (client: 473faa)
	void PopulateScenarioLocationFlags(); //47025d (client: 473fbd)
	void ComputeTravelRoute(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY); // 472820 (client: 47657e)
	void OnMapClick(); // 472dca (client: 476b1a)
	void UpdateHoveredLocation(CPoint pos); // 472d4b (client: 476a9b)
	void FreeBitmaps(); // 470cd9 (client: 474a37)
	void FreeSamples(); // 470571 (client: 4742cf)
	void LoadBitmaps(); // 4705c2 (client: 474320)
	void LoadSamples(); // 470504 (client: 474262)
	void FinishTravel(); // 472684 (client: 4763e2)
	void SearchRoute(uint16_t from, uint16_t to, GlobalMapRouteArray* route); // 472ada (client: 476832)

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
	VisLogoWnd(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4cd4d0 (client: 4d1e20)
	~VisLogoWnd(); //4cd53c (client: 4d1e8c)

	virtual void VMethod7() override; //4cd8d1 (client: 4d2221)
	virtual void VMethod8(CRect* rect) override {}; //4cd91e (client: 4d226e)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; //4cd72e (client: 4d207e)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; //4cd6c2 (client: 4d2012)
	virtual int32_t OnKeyDown(uint32_t wparam) override; //4cd6f8 (client: 4d2048)

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
	virtual void VMethod8(CRect* rect) override; // 45da6a (client: 461692)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 45D57F (client: 4611af)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 45D5F0 (client: 461220)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 45D6C3 (client: 4612f3)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 45D773 (client: 4613a3)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 45D5BE (client: 4611ee)
	virtual int32_t OnChar(uint32_t wparam) override; // 45D5D7 (client: 461207)

	virtual void VMethod26() override; // 45CE6E (client: 460a9e)
	virtual void VMethod28() override; // 45D222 (client: 460e52)
	virtual void DoClose(uint32_t code) override; // 45D2D9 (client: 460f09)

	void LoadBitmaps(); // 45d32e (client: 460f5e) in asm
	void LoadSounds(); // 45d542 (client: 461172) in asm
	void UpdateRects(); // 45cfae (client: 460bde) in asm
	void FreeBitmaps(); // 45d44a (client: 46107a) in asm
	void FreeSounds(); // 45d563 (client: 461193) in asm

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
	virtual ~VisFameDocument(); // 4AED50 (client: 4b3690)

	virtual const char* GetHint() override; // 4aeda0 (client: 4b36e0)
	virtual void VMethod7() override; // 4AE7A3 (client: 4b30e3)
	virtual void VMethod8(CRect* rect) override; // 4ae926 (client: 4b3266)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AE57F (client: 4b2ebf)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4AE5BE (client: 4b2efe)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4AE607 (client: 4b2f47)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4AE717 (client: 4b3057)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4ae791 (client: 4b30d1)

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
	uint32_t gap_0xbc;
	CRect rect_prev;
	CRect rect_next;
	CRect rect_ok;
	uint32_t gap_0xf0;
	uint32_t visible_flag;
};
ASSERT_SIZE(VisFameDocument, 0xf8);


//60b098
class Vis2Action : public VisMenuWnd {
public:
	virtual ~Vis2Action(); // 450bd0 (client: 4548d0)

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
	virtual ~VisMissionFailed(); // 450B00 (client: 454800)
	virtual void VMethod26() override; // 44E8CA (client: 451bef)

	VisMissionFailed(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const CString& _txt); //44e875 (client: 451b9a)
};
ASSERT_SIZE(VisMissionFailed, 0x78);

//60b008
class VisCutScenesDlg : public VisMessageBox {
public:
	virtual ~VisCutScenesDlg(); // 450B50 (client: 454850)

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
	virtual void VMethod30(CPoint pos, const CRect& r) override; // 4e3d30 (client: 4e9cc0)

	VisListBoxDiplomacy(int32_t _id, const RECT& r, CArray<DiplomacyEntry*>* _diplomacy, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height); //4dc9fb (client: 4e137e)

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
	virtual ~VisDiplomacy(); // 44F9E0 (client: 4538d0)

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
	virtual ~VisConnectionDlg(); // 44fa40 (client: 453930)

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
	virtual ~VisNetDlg(); // 44FA60 (client: 453950)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 447214 (client: 4498e3)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4478E9 (client: 4e4833)
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
	virtual ~VisNetTcpIp(); // 44FD10 (client: 453c10)

	VisNetTcpIp(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* _ip); //449f06 (client: 44c5d8)

	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44A014 (client: 44c6e6)
	virtual void VMethod31(int32_t code) override; // 449F67 (client: 44c639)
public:
	CString* ip;
};
ASSERT_SIZE(VisNetTcpIp, 0x7c);


//60a788
class VisNetSerialSettings : public VisWindow {
public:
	virtual ~VisNetSerialSettings(); // 44FCC0 (client: 453bc0)

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
	virtual ~VisListBoxPhoneBook(); // 44fca0 (client: 453ba0)

	virtual int32_t OnKeyDown(uint32_t wparam) override; // 44fb80 (client: 453a80)

	VisListBoxPhoneBook(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height, const char* hint, CStringArray* _bound_entries); //44fb30 (client: 453a30)

public:
	CStringArray* bound_entries; //0x94
};
ASSERT_SIZE(VisListBoxPhoneBook, 0x98);

//60a670
class VisNetPhoneBook : public VisWindow {
public:
	virtual ~VisNetPhoneBook(); // 44FAE0 (client: 4539e0)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44878A (client: 44ae5c)
	virtual void VMethod26() override; // 447EF2 (client: 44a5c1)

	VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book); //447ea8 (client: 44a577)

	void UpdateConnectButton(const char* str); // 448708 (client: 44adda)

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

	VisNetChatList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450080 (client: 4542a0) in asm

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c63d (client: 44f962) in asm

	CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetChatList, 0xa8);

//60abf0 — player list box
class VisNetPlayerList : public VisListBox {
public:
	virtual ~VisNetPlayerList(); // 450200 (client: 454420) (complete dtor 450230)

	VisNetPlayerList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450180 (client: 4543a0) in asm

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c71d (client: 44fa42) in asm

	CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetPlayerList, 0xa8);

//60ac80 — chat input line
class VisNetChatTextBox : public VisTextBox {
public:
	virtual ~VisNetChatTextBox(); // 450350 (client: 4545d0) (complete dtor 450380)

	VisNetChatTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint); // 450280 (client: 4544a0) in asm

	virtual int32_t OnKeyDown(uint32_t wparam) override; // 4502c0 (client: 454540) in asm
};
#ifdef A2CLIENT
ASSERT_SIZE(VisNetChatTextBox, 0x80);
#else
ASSERT_SIZE(VisNetChatTextBox, 0x7c);
#endif

//60a930
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam); // 44aad6 (client: 44d203)

class VisNetMapSelection : public VisWindow {
public:
	virtual ~VisNetMapSelection(); // 44FEA0 (client: 453dc0)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44D275 (client: 45059d)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 44D8F0 (client: 450c18)
	virtual void VMethod26() override; // 44C788 (client: 44faad)

	VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName); //44af45 (client: 44d672)

	void StopThread(); // 44aeef (client: 44d61c)
	void AddNetMapInfo(NetMapInfo* info); // 44d13d (client: 450465)
	void ScanMapsThread(); // 44aae5 (client: 44d212)

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
	virtual ~VisTipsDialog(); // 4e4510 (client: 4e98b0)

	virtual void VMethod8(CRect* rect) override; // 4E27CD (client: 4e7151)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4E2B95 (client: 4e7519)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4E2C42 (client: 4e75c6)
	virtual int32_t OnKeyDown(uint32_t wparam) override { return 0; } // 4e2c33 (client: 4e75b7)

	VisTipsDialog(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); //4e2541 (client: 4e6ec5)

	void InitTipsDialog(int32_t l, int32_t t, int32_t r, int32_t b, const char* txt); // 4e25c1 (client: 4e6f45)
	void SetText(const char* text); // 4e2b73 (client: 4e74f7)

public:
	uint8_t __unused__[12];
};
ASSERT_SIZE(VisTipsDialog, 0x74);


//60e6d8
class VisQuestStatus : public VisScreen {
public:
	virtual ~VisQuestStatus(); // 4E4540 (client: 4e98d0)

	virtual void VMethod7() override; // 4E2F76 (client: 4e78fa)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4e4530 (client: 4e9cd0)

	VisQuestStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4e2e3d (client: 4e77c1)

public:
	CSprite256* icon;
};
ASSERT_SIZE(VisQuestStatus, 0x6c);

//60a8a0
class VisHatServerListDlg : public VisMessageBox {
public:
	virtual ~VisHatServerListDlg(); // 44fd90 (client: 453c90)

	VisHatServerListDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, HatSettings* hat); //44a0e2 (client: 44c5d8)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44A8A8 (client: 44cfd5)
	virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44A34D (client: 44ca33)
	virtual void VMethod31(int32_t code) override; // 44A140 (client: 44c815)
public:
	HatSettings* hat_settings;
};
ASSERT_SIZE(VisHatServerListDlg, 0x7c);


//60aa40
class VisHatBrowserList : public VisListBox {
public:
	virtual ~VisHatBrowserList(); // 44ffb0 (client: 453ff0)

	virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44b60c (client: 44dd68)

	VisHatBrowserList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 44ff20 (client: 453f60)

	int32_t IsItemEnabled(int32_t idx); // 44b460 (client: 44db8d)

public:
	int32_t state; //0x94. -1 = refreshing, 0 = empty/failed, 1 = loaded
};
ASSERT_SIZE(VisHatBrowserList, 0x98);

// Returns the start position of the field-th (0-based) '|'-separated field of str, or -1.
int32_t GetFieldPos(CString str, int32_t field); // 44b37a (client: 44daa7)


struct HatBrowserProgress;
//60a9b8
class VisHatBrowserDlg : public VisWindow {
public:
	virtual ~VisHatBrowserDlg(); // 44FED0 (client: 453e10)

	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44C1A9 (client: 44f11d)
	virtual void VMethod26() override; // 44BC6C (client: 44e774)

	VisHatBrowserDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //44bc35 (client: 44e568)

	static UINT __cdecl ThreadProc(void*); //44e391

	// client-only web-browser glue. The retail client's browser object is installed by the
	// a2mgr.dll mod layer; every use is gated on browser != 0, so an un-hooked client (and
	// this port, which leaves it null) no-ops through all of them.
	HatBrowserProgress* browser = nullptr; // 0x68 — 0x21c-byte browser object (client ctor 4b18c0)
	CWinThread* worker_thread; // 0x6c — worker thread driving browser_step (client 44e391)
	HANDLE exit_event; // 0x70 — auto-reset; SetEvent from MsgProc 0x445/0x446
	HANDLE ack_event; // 0x74 — auto-reset, initially signaled
	CRITICAL_SECTION connect_cs; // 0x78 — guards connect_addr
	CString connect_addr; // 0x90 — address handed to the worker
};
ASSERT_SIZE(VisHatBrowserDlg, 0x94);

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

// client 5da840
class VisHatSession : public VisScreen {
public:
	VisHatSession(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4e850e (client: 4e850e)

	virtual void VMethod7() override; // 4e8545 (client: 4e8545)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override { return 0; }; // 4e9ce0 (client: 4e9ce0)
};

#endif

#endif
