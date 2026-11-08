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









































void __cdecl SortTokenEntries(CArray<TokenEntry*>* arr); //4ba1cc (client: 4beb9c) in asm
void __cdecl InsertThousandsSeparators(CString* str); //476987 (client: 47a6d3) - insert thousands separators









































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









// Chat command handler for the server screen text box input ("#disconnect N", "#curse N"). 44e4ce
void ExecuteAdminChatCommand(const char* cmd);  // client: 4517f9












































// Returns the start position of the field-th (0-based) '|'-separated field of str, or -1.
int32_t GetFieldPos(CString str, int32_t field); // 44b37a (client: 44daa7)


struct HatBrowserProgress;

#ifdef A2CLIENT



#endif

#endif


