#ifndef A2_GAMEPLAY_H
#define A2_GAMEPLAY_H

// HUD children of the in-game screen, the chat/console family and the
// shared message-log helper (GM_a28). The playing-area screen itself
// (VisPlayingArea, ex VisPlayingArea) lives in vis_playing_area.{h,cpp}.
#pragma once
#include "vis_core.h"
#include "take_damage.h"

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
	int32_t dirty; //unk type
};


//60ce80
class VisRightPanel : public CVisualObject {
public:
	virtual ~VisRightPanel(); // 4B46E0 (client: 4b90b0)

	virtual void VMethod7() override; // 4AEE31 (client: 4b3771)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AEE44 (client: 4b3784)

	VisRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aedcf (client: 4b370f) in asm
};
ASSERT_SIZE(VisRightPanel, 0x5c);


//60cef8
class VisMiniMap : public CVisualObject {
public:
	virtual ~VisMiniMap(); // 4B4730 (client: 4b9100)

	virtual void VMethod7() override; // 4AFB8E (client: 4b44c4)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AF006 (client: 4b3946)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B0C2F (client: 4b5561)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B0788 (client: 4b50be)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B0C84 (client: 4b5b7e)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4B0AAD (client: 4b53df)

	VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aeec3 (client: 4b3803)

	void RebuildMap(); // 4af0dc (client: 4b3a1c) in asm
	void UpdateCursor(); // 4b042e (client: 4b4d64) in asm

public:
	VisPlayingArea* map_context;
	CBmp64* bitmap1;
	CBmp64* bitmap2;
	int32_t zoom;
	uint32_t viewer;
};
ASSERT_SIZE(VisMiniMap, 0x70);


//60cf70
class VisOrderToolbar : public CVisualObject {
public:
	virtual ~VisOrderToolbar(); // 4b4790 (client: 4b9160)

	virtual const char* GetHint() override; // 4B0D9A (client: 4b56cc)
	virtual void VMethod7() override; // 4B12E3 (client: 4b5c15)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B0ED1 (client: 4b5803)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B12B3 (client: 4b5be5)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B107B (client: 4b59ad)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B124C (client: 4b5b7e)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B1213 (client: 4b5b45)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b1201 (client: 4b5b33)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B1287 (client: 4b5bb9)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b123a (client: 4b5b6c)

	void UpdateCursor(); // 4b1468 (client: 4b5d9a)

	VisOrderToolbar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b0cfc (client: 4b562e) in asm
public:
	VisPlayingArea* map_context;
	uint32_t selected_order;
	uint32_t avail_orders_mask;
	uint32_t enabled;
	uint32_t dirty;
};
ASSERT_SIZE(VisOrderToolbar, 0x70);

//609a58 - console-style input line used by Vis1200
class Vis1200obj : public CVisualObject {
public:
	virtual ~Vis1200obj();  // client: 43e3c0

	virtual void VMethod7() override; // 43b32f (client: 43d73e)
	virtual void WriteData(void* buf) override; // 43ad35 (client: 43d144)
	virtual uint32_t DataSize() override; // 43c020 (client: 43e3b0)
	virtual void ReadData(const void* buf) override; // 43acca (client: 43d0d9)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43b283 (client: 43d692)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43b159 (client: 43d564)
	virtual int32_t OnChar(uint32_t wparam) override; // 43b240 (client: 43d64b)

	void AppendInputChar(uint8_t ch); // 43ae1c (client: 43d22b)
	void BackspaceInputChar(); // 43b03a (client: 43d445)
	void CompletePlayerName(); // 43b724 (client: 43db33)
	void DeletePreviousWord(); // 43ba75 (client: 43de84)
	void DrawInputLine(CString* str); // 43b504 (client: 43d913)

	Vis1200obj(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); //43ab28 (client: 43cf38)

	void ResetInputState(); //43ac8c (client: 43d09b) - reset input/history state
	void SaveHistoryBackup(); //43ad86 (client: 43d195) - commit current line to history
	void RecallHistoryPrev(); //43ada3 (client: 43d1b2) - recall previous line from history
	int32_t GetMatchPlayerIndex(); //4972f0 (client: 401780)
	int32_t GetMatchNameLength(); //497330 (client: 401780)

public:
	CStringArray history_backup;
	CStringArray history; // history
	CString input_line; // current line
	CGameFont* font; // 0x88
	uint16_t* clr; // 0x8c
	int32_t match_player_index; // 0x90
	int32_t match_name_length; // 0x94
	uint32_t blink_last_tick; // 0x98 - last input tick
	int32_t blink_now_tick; // 0x9c
	int32_t cursor_visible; // 0xa0
};
ASSERT_SIZE(Vis1200obj, 0xa4);

	int32_t IsWordSeparator(char ch); // 43aae0 in asm - chat separator test (' ', ',', '.', ':', ';')
	void MakeLowerDosCp(CString* str); // 476236 - lowercase respecting DOS CP

//609ad0
class Vis1200 : public VisScreen {
public:
	virtual ~Vis1200(); // 43c000 (client: 43e420)

	virtual void VMethod7() override; // 43BE80 (client: 43e28f)
	virtual void VMethod8(CRect* rect) override; // 43be73 (client: 43e282)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43bdcb (client: 43e1da)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43BDEC (client: 43e1fb)

	virtual void VMethod26() override; // 43BC3C (client: 43e04b)
	virtual void VMethod28() override; // 43BD2E (client: 43e13d)
	virtual void DoClose(uint32_t code) override; // 43BD96 (client: 43e1a5)

	int32_t GetMatchPlayerIndex(); //4972d0 (client: 430ac8)
	int32_t GetMatchNameLength(); //497310 (client: 430ac8)
	void RepositionAboveDialogs(); //43be9f (client: 43e2ae)

	Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 43bb70 (client: 433fda) in asm

public:
	char buffer[256];
	Vis1200obj* text_block;
	uint32_t active;
};
ASSERT_SIZE(Vis1200, 0x170);

//60d070
class VisSideStatus : public CVisualObject {
public:
	virtual ~VisSideStatus(); // 4B4810 (client: 4b91e0)

	virtual const char* GetHint() override; // 4B45BB (client: 4b8f8a)
	virtual void VMethod7() override; // 4B4190 (client: 4b8b61)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B4490 (client: 4b8e5f)

	void UpdateCursor(); //4b4019 (client: 4b89ea) in asm

	VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b3fa3 (client: 4b8974) in asm
public:
	uint32_t dirty;
};
ASSERT_SIZE(VisSideStatus, 0x60);


#endif
