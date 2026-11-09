#ifndef A2_VIS_HAT_H
#define A2_VIS_HAT_H

#pragma once
#include "vis_core.h"
#include <vector>

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

// client vtable 5d7238 — hat-DM score dialog popped by VisPlayingArea packet cases 0x5c/0xc3.
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


struct HatBrowserProgress {
    WSAData wsa;
    SOCKET sock;
    sockaddr_in send_addr;
    sockaddr_in recv_addr;
    addrinfo* host_ent;
    int32_t err;
    int32_t send_len;
    int32_t recv_addr_len;
    int32_t timeout;
    char* addr_str;
    std::vector<uint8_t> send_buf;
    std::vector<uint8_t> recv_buf;
    in_addr addr;
    int32_t send_serial;
    int32_t min_rtt;
    int32_t max_rtt;
    int32_t last_rtt;
    int32_t recv_serial;
    int32_t total_rtt;
    int32_t timeouts;
    int32_t error;
    int32_t active;
    int32_t wsa_started;
    uint32_t timestamp;
    CRITICAL_SECTION cs;

    static void InitBuf(uint8_t* buf, int32_t ln) { //4b2187
        buf[0] = 8;
        buf[1] = 0;
        *(uint16_t*)(buf + 2) = 0;
        *(uint16_t*)(buf + 4) = GetCurrentProcessId();
        *(uint16_t*)(buf + 6) = 0;
        memset(buf + 0xc, 'E', ln - 0xc);
    }

    static uint32_t CalcCrc(void* ibuf, int32_t ln) { //4b2113
        uint16_t* buf = (uint16_t*)ibuf;
        uint32_t crc = 0;
        for (; ln > 1; ln -= 2) {
            crc += *buf;
            buf++;
        }

        if (ln != 0)
            crc += *(uint8_t*)buf;

        crc = (crc >> 16) + (crc & 0xffff);
        return ~(crc + (crc >> 0x10));
    }
    HatBrowserProgress(); // 4b18c0
    ~HatBrowserProgress();
    void Init(const char* hst); // 4b1a9b - resolve host, open the ICMP socket, arm the session
    void PingServer(); // 4b1dbb - send one ICMP echo and read the reply
    int32_t GetPingError(); // 453df0 - returns error
};
#endif


