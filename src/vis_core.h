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
extern char g_HintBuf[223]; //4c9a6f GetHint static buffer (defined in vis_core.cpp)

class BigStruct2;
class GM_a28;
class VisInvExtType1;
class VisInvExtType2;
class VisInvExtType3;
class VisSpellBook;
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



























class VisStartGame;




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
