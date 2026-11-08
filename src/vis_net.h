#ifndef A2_VIS_NET_H
#define A2_VIS_NET_H

#pragma once
#include "vis_core.h"

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


//60a930
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam); // 44aad6 (client: 44d203)

// binary 0xfc; the embedded CWinThread differs between VC6 and MFC140,
// so pin the composition: 5 tail members (0x14) follow the thread object
ASSERT_SIZE(VisNetMapSelection, offsetof(VisNetMapSelection, thread) + sizeof(CWinThread) + 0x14);
#endif


