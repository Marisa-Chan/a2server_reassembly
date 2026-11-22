#ifndef A2_VIS_NET_H
#define A2_VIS_NET_H

#pragma once
#include "vis_core.h"

// 60adf8, radio checkbox on the server screen ("save on server" toggle)
class VisServerScreenRadio : public VisRadioType1 {
public:
    virtual ~VisServerScreenRadio(); // 4509a0 (client: 454760) - (complete dtor 4509a0)

    VisServerScreenRadio(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); // 450920 (client: 4546e0)

    virtual void VMethod7() override; // 4db3ad (client: 4dfd32)
    virtual int32_t GetIndex(int32_t y) override; // 4db615 (client: 4dff9a)
};
ASSERT_SIZE(VisServerScreenRadio, 0x8c);

//60aef0
class VisServerScreen : public VisScreen {
public:
    virtual ~VisServerScreen(); // 450ae0 (client: 4547e0)

    virtual void VMethod7() override; // 44dbe0 (client: 450f0b)
    virtual void VMethod8(CRect* rect) override; // 44dbae (client: 450ed9)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44e7c8 (client: 451aed)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 44e6de (client: 451a03)
    virtual void VMethod26() override; // 44d909 (client: 450c31)

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

    VisConnectionDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, int32_t* _payload); // 445e52 (client: 448523)

    virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44604c (client: 44871d)
    virtual void VMethod31(int32_t code) override; // 445ebd (client: 44858e)
public:
    CLlConn* protocols;
    int32_t* pSelected;
};
ASSERT_SIZE(VisConnectionDlg, 0x80);

//60a5e8
class VisNetDlg : public VisWindow {
public:
    virtual ~VisNetDlg(); // 44fa90 (client: 453980)

    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 447214 (client: 4498e3)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 4478e9 (client: 4e4833)
    virtual void VMethod26() override; // 446e97 (client: 449566)

    VisNetDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, AvailNetSession* _sessions); // 4464c7 (client: 448b96)

private:
    void CreateSessionList(); // 446567 (client: 448c36)
    void RefreshSessions(); // 447173 (client: 449842)
    void FillServerList(); // 44797e (client: 44a04d)
    void UpdatePlayerList(); // 447b3c (client: 44a20b)
    void CachePlayerRows(); // 447c27 (client: 44a2f6)

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
    virtual ~VisNetTcpIp(); // 44fd40 (client: 453c40)

    VisNetTcpIp(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* _ip); // 449f06 (client: 44c5d8)

    virtual CVisualObject* VMethod30(const void* data, const RECT& r) override; // 44a014 (client: 44c6e6)
    virtual void VMethod31(int32_t code) override; // 449f67 (client: 44c639)
public:
    CString* ip;
};
ASSERT_SIZE(VisNetTcpIp, 0x7c);

//60a788
class VisNetSerialSettings : public VisWindow {
public:
    virtual ~VisNetSerialSettings(); // 44fcf0 (client: 453bf0)

    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 449c98 (client: 44c36a)
    virtual void VMethod26() override; // 448d15 (client: 44b3e7)

    void SaveComSettings(); // 449903 (client: 44bfd5)

    VisNetSerialSettings(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, ComSettings* _com); // 448cd5 (client: 44b3a7)

public:
    ComSettings* pcom;
};
ASSERT_SIZE(VisNetSerialSettings, 0x6c);

//60a6f8
class VisListBoxPhoneBook : public VisListBox {
public:
    virtual ~VisListBoxPhoneBook(); // 44fca0 (client: 453ba0)

    virtual int32_t OnKeyDown(uint32_t wparam) override; // 44fb80 (client: 453a80)

    VisListBoxPhoneBook(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height, const char* hint, CStringArray* _bound_entries); // 44fb30 (client: 453a30)

public:
    CStringArray* bound_entries; //0x94
};
ASSERT_SIZE(VisListBoxPhoneBook, 0x98);

//60a670
class VisNetPhoneBook : public VisWindow {
public:
    virtual ~VisNetPhoneBook(); // 44fb10 (client: 453a10)

    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44878a (client: 44ae5c)
    virtual void VMethod26() override; // 447ef2 (client: 44a5c1)

    VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book); // 447ea8 (client: 44a577)

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
    virtual ~VisNetMapList(); // 450060 (client: 454280) - (complete dtor 450060)

    VisNetMapList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint, int32_t* p_selected_map_index); // 44ffd0 (client: 4541f0)

    virtual const char* GetHint() override; // 44b2bf (client: 44d9ec)
    virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44b09e (client: 44d7cb)

    int32_t* p_selected_map_index; // 0x94
};
ASSERT_SIZE(VisNetMapList, 0x98);

//60ab60 — chat log list box
class VisNetChatList : public VisListBox {
public:
    virtual ~VisNetChatList(); // 450130 (client: 454350) - (complete dtor 450130)

    VisNetChatList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450080 (client: 4542a0)

    virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c63d (client: 44f962)

    CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetChatList, 0xa8);

//60abf0 — player list box
class VisNetPlayerList : public VisListBox {
public:
    virtual ~VisNetPlayerList(); // 450230 (client: 454450) - (complete dtor 450230)

    VisNetPlayerList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint); // 450180 (client: 4543a0)

    virtual void DrawItem(int32_t idx, CPoint pos, uint16_t* clr) override; // 44c71d (client: 44fa42)

    CPtrArray colors; // 0x94
};
ASSERT_SIZE(VisNetPlayerList, 0xa8);

//60ac80 — chat input line
class VisNetChatTextBox : public VisTextBox {
public:
    virtual ~VisNetChatTextBox(); // 450380 (client: 454600) - (complete dtor 450380)

    VisNetChatTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint); // 450280 (client: 4544a0)

    virtual int32_t OnKeyDown(uint32_t wparam) override; // 4502c0 (client: 454540)
};
#ifdef A2CLIENT
ASSERT_SIZE(VisNetChatTextBox, 0x80);
#else
ASSERT_SIZE(VisNetChatTextBox, 0x7c);

#endif

class VisNetMapSelection : public VisWindow {
public:
    virtual ~VisNetMapSelection(); // 44afd8 (client: 44d705)

    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 44d275 (client: 45059d)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 44d8f0 (client: 450c18)
    virtual void VMethod26() override; // 44c788 (client: 44faad)

    VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName); // 44af45 (client: 44d672)

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
    VisPlayingArea* map_context;
    int32_t mode;
};


//60a930
UINT AFX_CDECL VisNetMapThreadProc(LPVOID pParam); // 44aad6 (client: 44d203)

// binary 0xfc; the embedded CWinThread differs between VC6 and MFC140,
// so pin the composition: 5 tail members (0x14) follow the thread object
ASSERT_SIZE(VisNetMapSelection, offsetof(VisNetMapSelection, thread) + sizeof(CWinThread) + 0x14);
#endif


