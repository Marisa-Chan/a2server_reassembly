#include <winsock2.h>
#ifdef A2CLIENT_PATCH
#include "patch/stats_level.h"
#endif
#include <ws2tcpip.h>
#include <timeapi.h>

#include "server.h"
#include "main_window.h"
#include "gfx.h"
#include "mouse.h"
#include "file.h"
#include "game_app.h"
#include "quest_map.h"
#include "quest.h"
#include "ingame.h"
#include "util.h"
#include "resource.h"
#include "spell.h"
#include "players_list.h"
#include "player.h"
#include "unit.h"
#include "unit_list.h"
#include "buildings_list.h"
#include "packet.h"
#include "vis_hat.h"
#include "gameplay.h"
#include "vis_playing_area.h"

// HatBrowserProgress implementation (struct declared in net.h)

HatBrowserProgress::HatBrowserProgress() { //4b18c0
    recv_addr_len = sizeof(send_addr);
    timeout = 1000;
    addr.S_un.S_addr = 0;
    send_serial = 0;
    sock = INVALID_SOCKET;
    wsa_started = 0;
    timestamp = 0;
    min_rtt = 10000;
    last_rtt = 0;
    max_rtt = 0;
    last_rtt = 0;
    total_rtt = 0;
    recv_serial = 0;
    timeouts = 0;
    error = 0;

    InitializeCriticalSection(&cs);

    if (WSAStartup(0x101, &wsa) == 0) {
        send_buf.resize(0x400);
        recv_buf.resize(0x400);
        if (send_buf.empty() || recv_buf.empty()) {
            error = -5;
        }
        else {
            wsa_started = 1;
        }
    }
    else {
        error = -1;
    }
}

HatBrowserProgress::~HatBrowserProgress() {
    if (wsa_started != 0) {
        if (sock != INVALID_SOCKET)
            closesocket(sock);
        WSACleanup();
    }
    DeleteCriticalSection(&cs);
}


void HatBrowserProgress::Init(const char* hst) { //4b1a9b
    if (wsa_started == 0)
        return;

    active = 0;

    EnterCriticalSection(&cs);

    min_rtt = 1000;
    last_rtt = 0;
    max_rtt = 0;
    last_rtt = 0;
    total_rtt = 0;
    recv_serial = 0;
    timeouts = 0;
    error = 0;

    LeaveCriticalSection(&cs);

    if (sock != INVALID_SOCKET)
        closesocket(sock);
    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        error = -2;
        return;
    }

    err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, 4);
    if (err == -1) {
        error = -3;
        return;
    }

    timeout = 1000;
    err = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, 4);
    if (err == -1) {
        error = -3;
        return;
    }

    memset(&send_addr, 0, sizeof(send_addr));
    //host_ent = gethostbyname(hst);

    if (GetAddrInfo(hst, nullptr, nullptr, &host_ent) == 0 || host_ent == nullptr) {
        inet_pton(AF_INET, hst, &addr.S_un); // addr.S_un.S_addr = inet_addr(hst);
    }

    if ((host_ent == nullptr) && addr.S_un.S_addr == 0xffffffff) {
        error = -4;
        return;
    }

    if (host_ent == nullptr)
        send_addr.sin_addr.S_un = addr.S_un;
    else
        memcpy(&send_addr.sin_addr, host_ent->ai_addr, host_ent->ai_addrlen);

    if (host_ent == nullptr)
        send_addr.sin_family = 2;
    else
        send_addr.sin_family = host_ent->ai_family;

    static char addrbuf[INET_ADDRSTRLEN+1];
    inet_ntop(AF_INET, &(send_addr.sin_addr), addrbuf, INET_ADDRSTRLEN); // inet_ntoa(send_addr.sin_addr);
    addr_str = addrbuf;
    send_len = 0x20;
    send_len += 0xc;

    memset(send_buf.data(), 0, send_buf.size());
    InitBuf(send_buf.data(), send_len);

    active = 1;
    timestamp = GetTickCount();
}


void HatBrowserProgress::PingServer() { //4b1dbb
    if (active == 0)
        return;

    error = 0;

    send_len = 0x20;
    send_len += 0xc;

    send_buf[2] = 0;
    send_buf[3] = 0;

    *(uint32_t *)(send_buf.data() + 8) = GetTickCount();
    *(uint16_t*)(send_buf.data() + 6) = send_serial;

    send_serial++;

    *(uint16_t*)(send_buf.data() + 2) = CalcCrc(send_buf.data(), send_len);

    int32_t res = sendto(sock, (char*)send_buf.data(), send_len, 0, (sockaddr*)&send_addr, 0x10);
    timestamp = GetTickCount();

    if (res == -1) {
        if (WSAGetLastError() == WSAETIMEDOUT)
            error = -6;
        else
            error = -7;
        return;
    }

    err = recvfrom(sock, (char*)recv_buf.data(), recv_buf.size(), 0, (sockaddr*)&recv_addr, &recv_addr_len);
    timestamp = GetTickCount();

    if (err == -1) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            EnterCriticalSection(&cs);
            timeouts++;
            error = -8;
            LeaveCriticalSection(&cs);
        }
        else
            error = -9;
        return;
    }
        
    uint32_t* rdat = (uint32_t*)(recv_buf.data() + (recv_buf[0] & 0xf) * 4);
    if (*(uint8_t*)rdat != 0) {
        error = -10;
        return;
    }

    if ((rdat[1] & 0xffff) != (GetCurrentProcessId() & 0xffff)) {
        error = -10;
        return;
    }

    EnterCriticalSection(&cs);
    last_rtt = timestamp - rdat[2];
    recv_serial++;
    total_rtt += last_rtt;

    if (min_rtt >= last_rtt)
        min_rtt = last_rtt;

    if (last_rtt >= max_rtt)
        max_rtt = last_rtt;
    LeaveCriticalSection(&cs);
}

int32_t HatBrowserProgress::GetPingError() { //453df0
    return error;
}

// 44b460
int32_t VisHatBrowserList::IsItemEnabled(int32_t idx) {
    if (this->state == 0) {
        return 0;
    }

    if (!this->IsValidIndex(idx)) {
        return 0;
    }

    CString& item = this->entries[idx];

    int32_t pos = GetFieldPos(item, 5);
    if (pos == -1) {
        return 0;
    }

    int32_t level = item[pos] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }

    pos = GetFieldPos(item, 6);
    if (pos == -1) {
        return 0;
    }

    int32_t players = atoi(item.Mid(pos, 2));

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
#ifdef A2CLIENT_PATCH
    // GUI_gmEnter hook (44DB8D): a full GM (session key part 2 carries the
    // GMF flags) may enter any server
    if ((main_wnd->m_GameSession.sessionKeyPart2 & 0x3F000000) == 0x3F000000) {
        return 1;
    }
#endif
    if (main_wnd->net_session.internet_game == 0 || main_wnd->hat_settings.ishat == 0 || main_wnd->hat_settings.deathmatch == 0) {
#ifdef A2CLIENT_PATCH
        // Tangar GUI_softcoreEnter (44DD0F): stats-based server-level selection
        // replaces the retail skill-tier brackets (and the players == 16 check).
        return tangar_stats::StatsBasedLevelSelection((const char*)item) ? 1 : 0;
#else
        if (main_wnd->m_GameSession.GetSkillTier1() > level + 1
            || main_wnd->m_GameSession.GetSkillTier2() < level + 1
            || players == 16) {
            return 0;
        }
        return 1;
#endif
    }

    return players != 16 ? 1 : 0;
}

// 44ff20
VisHatBrowserList::VisHatBrowserList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
: VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
    this->state = -1;
}

// 44ffb0
VisHatBrowserList::~VisHatBrowserList() {
}

// 44b60c
void VisHatBrowserList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (this->state == 0 && idx == 0) {
        this->font->DrawTextWithShadow(pos.x + 5, pos.y, txt_patch.GetLine(0x71), 0, clrsh_ShockingBlack, 1);
        return;
    }

    if (this->state == -1 && idx == 0) {
        this->font->DrawTextWithShadow(pos.x + 5, pos.y, txt_patch.GetLine(0x72), 0, clrsh_ShockingBlack, 1);
        return;
    }

    if (this->state == 0 || !this->IsValidIndex(idx)) {
        return;
    }

    if (!this->IsItemEnabled(idx)) {
        clr = g_colors_human_pals[15];
    }

    CString& item = this->entries[idx];

    CRect saved_clip;
    GetClipRect(&saved_clip);

    pos.x += 5;

    CRect clip(saved_clip);
    clip.right = vis_scr_rect.left + 0xD9;
    SetClipRect(clip);

    CString text = item.Mid(GetFieldPos(item, 1));
    text = text.Left(text.Find('|'));
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 3));
    text = text.Left(text.Find('|'));
    clip.right = vis_scr_rect.left + 0x160;
    pos.x = vis_scr_rect.left + 0xDC;
    SetClipRect(clip);
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 4));
    text = text.Left(text.Find('|'));
    SetClipRect(saved_clip);
    pos.x = vis_scr_rect.left + 0x163;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    int32_t level = item[GetFieldPos(item, 5)] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }
    text = txt_patch.GetLine(level + 0x62);
    pos.x = vis_scr_rect.left + 0x1AE;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);

    text = item.Mid(GetFieldPos(item, 6));
    text = text.Left(text.Find('|'));
    pos.x = vis_scr_rect.left + 0x203;
    this->font->DrawTextWithShadow(pos.x, pos.y, text, 0, clr, 1);
}


// 44bc6c
// client: 44e774
void VisHatBrowserDlg::VMethod26() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    VisLabel* title_label = new VisLabel(0x19, 0x28, 0x16, this->rect.Width() - 0x28, 0x38, txt_patch.GetLine(0x6a), g_font1, p_clrsh_Black, 2);
    this->AddChild(title_label);

    const char* ip_text = main_wnd->hat_settings.hatip;
#ifdef A2CLIENT
    if (main_wnd->hat_settings.ishat != 0) {
        ip_text = main_wnd->hat_settings.hatprogip;
    }
#endif
    VisLabel* ip_label = new VisLabel(0x7e, 0x28, 0x2c, this->rect.Width() - 0x28, 0x44, ip_text, g_font1, p_clrsh_Black, 2);
    this->AddChild(ip_label);

#ifdef A2CLIENT
    int32_t cap_top = 0x30;
    int32_t cap_bottom = 0x44;
#else
    int32_t cap_top = 0x44;
    int32_t cap_bottom = 0x5c;
#endif

    VisLabel* caption_label = new VisLabel(0x1a, 0x28, cap_top, this->rect.Width() - 0x28, cap_bottom, txt_patch.GetLine(0x6b), g_font1, p_clrsh_Black, 0);
    this->AddChild(caption_label);

    VisHatBrowserList* list = nullptr;
    if (main_wnd->hat_settings.ishat != 0) {
        int32_t list_top = 0x44;
        int32_t list_bottom = this->rect.Height() - 0x60;
        if (main_wnd->hat_settings.deathmatch == 2) {
            list_bottom = this->rect.Height() - 0xc0;
        }
        list = new VisHatBrowserList(1, 0x28, list_top, this->rect.Width() - 0x40, list_bottom, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xa, nullptr);
    } else {
#ifdef A2CLIENT
        int32_t list_bottom = this->rect.Height() - 0x50;
#else
        int32_t list_bottom = this->rect.Height() - 0x60;
#endif
        list = new VisHatBrowserList(1, 0x28, 0x5c, this->rect.Width() - 0x40, list_bottom, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xa, nullptr);
    }

#ifdef A2CLIENT
    if (main_wnd->hat_settings.ishat != 0) {
        if (main_wnd->hat_settings.deathmatch == 2) {
            VisRadioType2* store_radio = new VisRadioType2(0x31, 0x28, this->rect.Height() - 0xb6, this->rect.Width() - 0x40, this->rect.Height() - 0x80, g_font1, p_clrsh_Black, nullptr);
            store_radio->AddEntry(txt_patch.GetLine(0x8b));
            store_radio->AddEntry(txt_patch.GetLine(0x8c));
            this->AddChild(store_radio);
            store_radio->ReadData(&main_wnd->hat_settings.store);
        }
    
        VisLabel* status1 = new VisLabel(0x2d, 0x28, this->rect.Height() - 0x64, this->rect.Width() - 0x28, this->rect.Height() - 0x50, " ", g_font1, p_clrsh_Black, 2);
        this->AddChild(status1);
        VisLabel* status2 = new VisLabel(0x2e, 0x28, this->rect.Height() - 0x54, this->rect.Width() - 0x28, this->rect.Height() - 0x40, " ", g_font1, p_clrsh_Black, 2);
        this->AddChild(status2);
    }
#endif

    this->AddChild(list);

    list->SetCaptionLabel((VisLabel*)this->FindChild(0x1a));

    CRect list_rect = list->GetRect();
    VisScrollBar* scrollbar = new VisScrollBar(0xa, list_rect.right, list_rect.top, list_rect.right + 0x18, list_rect.bottom, nullptr);
    this->AddChild(scrollbar);

    CPoint btn_center(0xc0, this->rect.Height() - 0x30);
    CRect btn_rect(btn_center.x - 0x30, btn_center.y - 0xc, btn_center.x + 0x30, btn_center.y + 0xc);
    btn_rect.OffsetRect(-0x48, 0);

    VisButton* ok_btn = new VisButton(0x14, btn_rect, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    this->AddChild(ok_btn);
    ok_btn->ChangeFlags(1, false);

    btn_rect.OffsetRect(0x90, 0);
    VisButton* cancel_btn = new VisButton(0x15, btn_rect, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(cancel_btn);
    cancel_btn->SetLeftObj(this->FindChild(0x14));

    btn_rect.OffsetRect(0x90, 0);
    VisButton* refresh_btn = new VisButton(0x16, btn_rect, txt_patch.GetLine(0x6c), g_font1, nullptr, 0x48b, 0, "");
    this->AddChild(refresh_btn);
    refresh_btn->SetLeftObj(this->FindChild(0x15));
}

// 44c1a9
// client: 44f11d
int32_t VisHatBrowserDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (wparam != 1) {
            return VisScreen::MsgProc(msg, wparam, lparam);
        }
        // fallthrough
    case VIS_MSG_HIDE_SCREEN: {
        if (this->FindChild(0x14)->TestFlags(1) != 0) {
            VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
            CString item = list->GetItem(list->GetSelectedIndex());
            int32_t field_pos = GetFieldPos(item, 7);
            main_wnd->net_session.connect_address = item.Mid(field_pos);
            main_wnd->net_session.connect_address = main_wnd->net_session.connect_address.Left(main_wnd->net_session.connect_address.Find('|'));
#ifdef A2CLIENT
            if (main_wnd->hat_settings.deathmatch == 2) {
                ((VisRadioType2*)this->FindChild(0x31))->WriteData(&main_wnd->hat_settings.store);
                main_wnd->m_GameSession.color = (main_wnd->hat_settings.store != 0) ? 1 : 3;
            }
#endif
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
#ifdef A2CLIENT
            if (this->browser != nullptr) {
                SetEvent(this->exit_event);
            }
#endif
        }
        return 1;
    }
    case VIS_MSG_CLOSE_SCREEN:
        VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
#ifdef A2CLIENT
        if (this->browser != nullptr) {
            SetEvent(this->exit_event);
        }
#endif
        return 1;
    case VIS_MSG_SET_OPTION:
    case VIS_MSG_LIST_SELECT:
        if (wparam == 1) {
            if ((int32_t)lparam >= 0) {
                VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
                list->VMethod9();
                this->FindChild(0x14)->ChangeFlags(1, list->IsItemEnabled(list->GetSelectedIndex()));
                this->FindChild(0x14)->VMethod9();
#ifdef A2CLIENT
                if (this->browser) {
                    EnterCriticalSection(&this->connect_cs);
                    CString item = list->GetItem(list->GetSelectedIndex());
                    int32_t field_pos = GetFieldPos(item, 7);
                    this->connect_addr = item.Mid(field_pos);
                    this->connect_addr = this->connect_addr.Left(this->connect_addr.Find('|'));
                    LeaveCriticalSection(&this->connect_cs);
                }
#endif
            }
            return 1;
        }
        return 0;
    case VIS_MSG_HATBROWSER_REFRESH: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        int32_t sel_index = list->GetSelectedIndex();
        int32_t vis_start = list->vis_start_index;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->state = wparam;
        if (list->state == 0) {
            list->vis_start_index = 0;
            list->SetSelectedIndex(0);
        } else {
            for (int32_t i = 0; i < g_WwwServerList.GetSize(); i++) {
                list->AddItem(g_WwwServerList[i]);
            }
            if (g_WwwServerList.GetSize() <= sel_index) {
                sel_index = g_WwwServerList.GetSize() - 1;
            }
#ifdef A2CLIENT
            if (g_WwwServerList.GetSize() > 0 && sel_index < 0) {
                sel_index = 0;
            }
#endif
            if (g_WwwServerList.GetSize() <= vis_start) {
                vis_start = g_WwwServerList.GetSize() - 1;
            }
            list->vis_start_index = vis_start;
            list->SetSelectedIndex(sel_index);
        }
        this->MsgProc(VIS_MSG_SET_OPTION, 1, list->GetSelectedIndex());
        list->VMethod9();
        return 1;
    }
    case VIS_MSG_HATBROWSER_ABORT: {
        VisHatBrowserList* list = (VisHatBrowserList*)this->FindChild(1);
        list->state = -1;
        list->entries.RemoveAll();
        list->selected_index = -1;
        list->vis_start_index = 0;
        list->SetSelectedIndex(0);
        list->VMethod9();

        int32_t res = 1; // 44c55e: left uninitialized when hat_settings.ishat == 0
#ifdef A2CLIENT
        if (main_wnd->hat_settings.ishat == 0) {
            res = main_wnd->CheckWebVersion(main_wnd->hat_settings.hatip);
        } else if (main_wnd->LeaveServerMode() == 0) {
            g_CLlDriver.Close();
            main_wnd->PostMessage(0x446, 0, 0);
            return VisScreen::MsgProc(msg, wparam, lparam);
        }
#else
        if (main_wnd->hat_settings.ishat != 0) {
            if (main_wnd->LeaveServerMode() == 0) {
                g_CLlDriver.Close();
                main_wnd->PostMessage(0x446, 0, 0);
                return VisScreen::MsgProc(msg, wparam, lparam);
            }
            res = main_wnd->CheckWebVersion(main_wnd->hat_settings.hatip);
        }
#endif
        this->MsgProc(VIS_MSG_HATBROWSER_REFRESH, res, 0);
        return VisScreen::MsgProc(msg, wparam, lparam);
    }

#ifdef A2CLIENT
    case VIS_MSG_HATBROWSER_PROGRESS: {
        // client-only: progress/status pump posted by the worker thread. Requires the
        // a2mgr-installed browser object; without it the retail client passthroughs too.
        if (main_wnd->hat_settings.ishat && this->browser) {
            HatBrowserProgress* prog = (HatBrowserProgress*)this->browser;
            VisLabel* lbl1 = (VisLabel*)this->FindChild(0x2d);
            VisLabel* lbl2 = (VisLabel*)this->FindChild(0x2e);
            CString s1;
            CString s2;
            EnterCriticalSection(&prog->cs);
            int32_t step = (int32_t)wparam;
            if (prog->recv_serial + prog->timeouts != 0) {
                int32_t total = prog->recv_serial + prog->timeouts;
                s1.Format(txt_patch.GetLine(0x7f), prog->min_rtt, (prog->recv_serial != 0) ? prog->total_rtt / prog->recv_serial : 0, prog->max_rtt, (total != 0) ? prog->timeouts * 100 / total : 0);
                if (step == 0) {
                    s2.Format(txt_patch.GetLine(0x80), prog->last_rtt, total);
                }
                else {
                    s2 = txt_patch.GetLine(0x80 - step);
                }
            }
            else {
                if (step != 0) {
                    s1 = " ";
                    s2 = txt_patch.GetLine(0x80 - step);
                }
                else {
                    s1 = " ";
                    s2 = " ";
                }
            }
            LeaveCriticalSection(&prog->cs);
            lbl1->text = s1;
            lbl1->VMethod9();
            lbl2->text = s2;
            lbl2->VMethod9();
        }
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
#endif
    
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}

// 44bc35
VisHatBrowserDlg::VisHatBrowserDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
    : VisWindow(_id, l, t, r, b, nullptr) {

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->hat_settings.ishat)
    {
        browser = new HatBrowserProgress();
        exit_event = CreateEventA(nullptr, 0, 0, nullptr);
        ack_event = CreateEventA(nullptr, 0, 0, nullptr);
        InitializeCriticalSection(&connect_cs);
        worker_thread = AfxBeginThread(ThreadProc, this, 0, 0, 0, 0);
        worker_thread->m_bAutoDelete = 1;
    }
    else
        browser = nullptr;
}

#ifdef A2CLIENT
// client: 4527dd
HatPlayerListWindow::HatPlayerListWindow(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, const void* _payload, int32_t _mode)
    : VisMessageBox(_id, l, t, r, b, _payload, nullptr, 0, txt_patch.GetLine(_mode + 0xd1)) {
    this->mode = _mode;
}

// client: 452837
HatPlayerListWindow::~HatPlayerListWindow() {
    // retail writes MainWindow+0x634 - the client layout's shutdown_pending
    // slot, repurposed as the hat player-list visibility flag
    ((MainWindow*)AfxGetMainWnd())->shutdown_pending = 0;
}

// client: 452a02
int32_t HatPlayerListWindow::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg == VIS_MSG_HIDE_SCREEN && this->mode != 0) {
        MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
        wnd->vis_map_context->msglog.AddWithCheck(txt_patch.GetLine(0x45), clr_log_sblack, 10000);
    }
    return VisMessageBox::MsgProc(msg, wparam, lparam);
}

// client: 452897
CVisualObject* HatPlayerListWindow::VMethod30(const void* payload, const RECT& r) {
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    // hat player-list visibility flag (client MainWindow+0x634, see dtor above)
    wnd->shutdown_pending = 1;

    RECT list_r = r;
    list_r.right -= 0x18;

    HatPlayerScoreList* list = new HatPlayerScoreList(2, list_r, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0x29b, nullptr);
    list->ReadData(&payload);
    list->SetSelectedIndex(g_hat_selected_index);
    list->scores.SetSize(wnd->vis_map_context->hat_score_list.GetSize(), -1);
    for (int32_t i = 0; i < list->scores.GetSize(); i++) {
        list->scores[i] = wnd->vis_map_context->hat_score_list[i];
    }
    this->AddChild(list);

    this->AddChild(new VisScrollBar(0x29b, list_r.right, list_r.top, list_r.right + 0x18, list_r.bottom, nullptr));
    return list;
}

// client: 454970
HatPlayerScoreList::HatPlayerScoreList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}

// client: 454a20
HatPlayerScoreList::~HatPlayerScoreList() {
}

// client: 45265b
void HatPlayerScoreList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    (void)clr;
    if (!this->IsValidIndex(idx)) {
        return;
    }

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();
    bool is_me = (this->entries[idx] == wnd->vis_map_context->my_main_unit->name);
    uint16_t* color = is_me ? (uint16_t*)clrsh_ShockingBlack : g_colors_human_pals[15];

    CString str;
    str.Format("%d. %s", idx + 1, (const char*)this->entries[idx]);
    this->font->DrawTextWithShadow(pos.x + 5, pos.y, str, 0, color, 1);

    str.Format("%d", this->scores[idx]);
    int w = this->font->GetStrWidth(str);
    this->font->DrawTextWithShadow(pos.x + this->rect.Width() - 10 - w, pos.y, str, 1, color, 1);
}
#endif

// 44fed0
VisHatBrowserDlg::~VisHatBrowserDlg() {
}

UINT __cdecl VisHatBrowserDlg::ThreadProc(void* pv) { //44e391
    VisHatBrowserDlg* dlg = (VisHatBrowserDlg*)pv;

    while (WaitForSingleObject(dlg->exit_event, 0) != WAIT_OBJECT_0) {
        ResetEvent(dlg->ack_event);
        EnterCriticalSection(&dlg->connect_cs);

        CString addr = dlg->connect_addr;

        if (!addr.IsEmpty()) {
            dlg->connect_addr.Empty();
            LeaveCriticalSection(&dlg->connect_cs);

            int padr = addr.ReverseFind(':');
            if (padr != -1)
                addr = addr.Left(padr);

            dlg->browser->Init(addr);

            if (WaitForSingleObject(dlg->exit_event, 0) == 0) {
                SetEvent(dlg->ack_event);
                break;
            }

            PostMessageA(g_MainWndHWND, 0x48c, dlg->browser->GetPingError(), 0);
        }
        else
            LeaveCriticalSection(&dlg->connect_cs);

        dlg->browser->PingServer();
        PostMessageA(g_MainWndHWND, 0x48c, dlg->browser->GetPingError(), 0);
        SetEvent(dlg->ack_event);

        if (WaitForSingleObject(dlg->exit_event, 500) == 0)
            break;
    }
    return 0;
}



// 44A140
// client: 44c815
void VisHatServerListDlg::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        this->FindChild(4)->WriteData(&this->hat_settings->ishat);
        char buffer[256];
        this->FindChild(2)->WriteData(buffer);
        if (this->hat_settings->ishat == 0) {
            this->hat_settings->hatip = buffer;
        } else {
            this->hat_settings->hatprogip = buffer;
        }
        this->FindChild(6)->WriteData(buffer);
        this->hat_settings->login = buffer;
        this->FindChild(8)->WriteData(buffer);
        this->hat_settings->password = buffer;
        this->FindChild(3)->WriteData(&this->hat_settings->deathmatch);
#ifdef A2CLIENT
        if (this->hat_settings->deathmatch == 1) {
            this->hat_settings->deathmatch = 3;
        }
#endif
        this->FindChild(9)->WriteData(&this->hat_settings->store);
#ifndef A2CLIENT
            main_wnd->net_session.internet_game = (this->hat_settings->ishat == 0) ? 1 : 0;
#else
            main_wnd->net_session.internet_game = 1;
#endif
        if (this->hat_settings->ishat == 0) {
            main_wnd->PostMessage(MAIN_MSG_SHOW_CHAR_LOADER, 0, 0);
        } else {
            main_wnd->PostMessage(MAIN_MSG_CONNECT_DONE, 0, 0);
        }
    } else {
        main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
    }
}

// 44A8A8
int32_t VisHatServerListDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    if (msg != VIS_MSG_SET_OPTION) {
        return VisMessageBox::MsgProc(msg, wparam, lparam);
    }
    if (wparam != 4) {
        return 0;
    }

    CVisualObject* store_child = this->FindChild(9);
    if (store_child->TestFlags(1) == lparam || (int32_t)lparam > 1 || (int32_t)lparam < 0) {
        return 1;
    }

    store_child->ChangeFlags(1, lparam != 0);
    this->FindChild(3)->ChangeFlags(1, lparam != 0);
    this->FindChild(6)->ChangeFlags(1, lparam != 0);
    this->FindChild(8)->ChangeFlags(1, lparam != 0);

    char buffer[256];
    this->FindChild(2)->WriteData(buffer);
    if (lparam == 1) {
        this->FindChild(2)->ReadData(this->hat_settings->hatprogip);
        this->hat_settings->hatip = buffer;
    } else {
        this->FindChild(2)->ReadData(this->hat_settings->hatip);
        this->hat_settings->hatprogip = buffer;
    }
    this->VMethod9();
    return 1;
}

// 44A34D
// client: 44ca33
CVisualObject* VisHatServerListDlg::VMethod30(const void* data, const RECT& r) {
    HatSettings* hat = (HatSettings*)data;
    // the client lays out everything below the ip box 0x18 lower
#ifdef A2CLIENT
    const int32_t dy = 0x18;
#else
    const int32_t dy = 0;
#endif

    VisTextBox* ip_box = new VisTextBox(2, r.left, r.top, r.right, r.top + 0x18, g_font1, clrsh_TechBlack, txt_patch.GetLine(0x70));
    this->AddChild(ip_box);
    if (hat->ishat == 0) {
        ip_box->ReadData(hat->hatip);
    } else {
        ip_box->ReadData(hat->hatprogip);
    }

    VisRadioType2* deathmatch_radio = new VisRadioType2(3, r.left, 0x6c, r.right, 0x9c + dy, g_font1, p_clrsh_Black, nullptr);
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x68));
#ifdef A2CLIENT
    deathmatch_radio->AddEntry(txt_patch.GetLine(0xd5));
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x6d));
#else
    deathmatch_radio->AddEntry(txt_patch.GetLine(0x69));
#endif

    this->AddChild(deathmatch_radio);
    int32_t dm_display = hat->deathmatch;
#ifdef A2CLIENT
    if (dm_display == 3) {
        dm_display = 1;
    }
#endif

    deathmatch_radio->ReadData(&dm_display);
    if (hat->ishat == 0) {
        deathmatch_radio->ChangeFlags(1, false);
    }

    VisRadioType2* ishat_radio = new VisRadioType2(4, r.left, 0xa8 + dy, r.right, 0xd8 + dy, g_font1, p_clrsh_Black, nullptr);
    ishat_radio->AddEntry(txt_patch.GetLine(0x78));
    ishat_radio->AddEntry(txt_patch.GetLine(0x79));
    this->AddChild(ishat_radio);
    ishat_radio->ReadData(&hat->ishat);

    this->AddChild(new VisLabel(5, r.left, 0xe4 + dy, r.right, 0xfc + dy, txt_patch.GetLine(0x6e), g_font1, p_clrsh_Black, 0));

    VisTextBox* login_box = new VisTextBox(6, r.left, 0xfc + dy, r.right, 0x114 + dy, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(login_box);
    login_box->ReadData(hat->login);
    if (hat->ishat == 0) {
        login_box->ChangeFlags(1, false);
    }

    this->AddChild(new VisLabel(7, r.left, 0x12c + dy, r.right, 0x144 + dy, txt_patch.GetLine(0x6f), g_font1, p_clrsh_Black, 0));

    VisTextBox* password_box = new VisTextBox(8, r.left, 0x144 + dy, r.right, 0x15c + dy, g_font1, p_clrsh_Black, nullptr);
    this->AddChild(password_box);
    password_box->ReadData(hat->password);
#ifdef A2CLIENT
    password_box->is_password = 1;
#endif
    if (hat->ishat == 0) {
        password_box->ChangeFlags(1, false);
    }

    VisRadioType1* store_radio = new VisRadioType1(9, r.left, 0x168 + dy, r.right, 0x180 + dy, g_font1, p_clrsh_Black, txt_patch.GetLine(0x7b));
    store_radio->AddEntry(txt_patch.GetLine(0x7a));
    this->AddChild(store_radio);
    store_radio->ReadData(&hat->store);
    if (hat->ishat == 0) {
        store_radio->ChangeFlags(1, false);
    }

    return password_box;
}

// 44A0E2
VisHatServerListDlg::VisHatServerListDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, HatSettings* hat)
    : VisMessageBox(_id, l, t, r, b, hat, txt_patch.GetLine(0x67), 1, txt_patch.GetLine(0x66)) {
    this->hat_settings = hat;
}

// 44FD90
VisHatServerListDlg::~VisHatServerListDlg() {
}

#ifdef A2CLIENT
VisHatSession::VisHatSession(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b)
: VisScreen(_id, l, t, r, b, nullptr) {} //4e850e

void VisHatSession::VMethod7() { // 4e8545
    CDWordArray scores;
    CDWordArray colors;
    CStringArray names;
    int num = 0;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->hat_settings.deathmatch == 3) {
        CString st = mwnd->vis_map_context->my_main_unit->name;
        for (int i = 0; i < mwnd->vis_map_context->hat_name_list.GetSize(); i++) {
            if (st == mwnd->vis_map_context->hat_name_list[i]) {
                scores.Add(mwnd->vis_map_context->hat_score_list[i]);
                colors.Add(mwnd->vis_map_context->my_main_unit->color);
                st.Format("%d/%d %s", i + 1, mwnd->vis_map_context->hat_name_list.GetSize(), mwnd->vis_map_context->my_main_unit->name);
                names.Add(st);
                num = 1;
                break;
            }
        }
    }
    else {
        for (int i = 0; i < 16; i++) {
            if (mwnd->vis_map_context->hat_cursor_slots[i] != 0xf0000003 && i + 16 < mwnd->vis_map_context->map_players.GetSize()) {
                MapPlayerData* pl = mwnd->vis_map_context->map_players[i + 16];
                if (pl) {
                    num++;

                    int j = 0;
                    while (j < scores.GetSize() && mwnd->vis_map_context->hat_cursor_slots[i] < scores[j])
                        j++;

                    scores.InsertAt(j, mwnd->vis_map_context->hat_cursor_slots[i]);
                    names.InsertAt(j, pl->name);
                    colors.InsertAt(j, pl->color);
                }
            }
        }
    }

    int32_t idoff = 0;
    if (mwnd->hat_settings.deathmatch == 2 && mwnd->vis_map_context->hat_item_slot12 != 0)
        idoff = 2;

    int32_t hsz = idoff + 2 + num;

    rect.bottom = rect.top + 72 + ((hsz + 2) / 3) * 32;
    CRect local_20 = ClientRectToScreen(rect);

    local_20.bottom -= 8;
    local_20.right -= 8;

    gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.top + 8,     12, 6, 0);
    gfx_interface_lm->DrawBrightened(local_20.left  + 8,  local_20.bottom - 24, 15, 6, 0);
    gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.bottom - 24, 17, 6, 0);
    
    for (int i = 0; i < (local_20.Width() - 64) / 48; i++)
        gfx_interface_lm->DrawBrightened(local_20.left + 40 + i * 48, local_20.bottom - 24, 16, 6, 0);

    for (int i = 0; i < (local_20.Height() - 64) / 32; i++)
        gfx_interface_lm->DrawBrightened(local_20.right - 24, local_20.top + 40 + i * 32, 14, 6, 0);

    gfx_interface_lm->Draw(local_20.left, local_20.top, 10, 0, 0);
    gfx_interface_lm->Draw(local_20.right - 32, local_20.top, 12, 0, 0);
    gfx_interface_lm->Draw(local_20.left, local_20.bottom - 32, 15, 0, 0);
    gfx_interface_lm->Draw(local_20.right - 32, local_20.bottom - 32, 17, 0, 0);

    for (int i = 0; i < (local_20.Width() - 64) / 48; i++) {
        gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.top, 11, 0, 0);
        gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.bottom - 32, 16, 0, 0);
    }

    for (int i = 0; i < (local_20.Height() - 64) / 32; i++) {
        gfx_interface_lm->Draw(local_20.left, local_20.top + 32 + i * 32, 13, 0, 0);
        gfx_interface_lm->Draw(local_20.right - 32, local_20.top + 32 + i * 32, 14, 0, 0);
    }

    for (int i = 0; i < (local_20.Width() - 64) / 48; i++) {
        for (int j = 0; j < (local_20.Height() - 64) / 32; j++) {
            gfx_interface_lm->Draw(local_20.left + 32 + i * 48, local_20.top + 32 + j * 32, 9, 0, 0);
        }
    }

    int32_t xoff = rect.left + 32;
    int32_t yoff = rect.top + 32;
    LockSurface2();
    if (mwnd->vis_map_context->hat_item_slot12 == 0) {
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, txt_patch.GetLine(181), 2, clrsh_TechBlack, 1);
    }
    else if (mwnd->vis_map_context->hat_player_count < 0x7fffffff) {
        CString local_88;
        local_88.Format(txt_patch.GetLine(186), mwnd->vis_map_context->hat_player_count);
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, local_88, 2, clrsh_TechBlack, 1);
    }
    else {
        g_font2->DrawTextWithShadow(rect.left + rect.Width() / 2, yoff, txt_patch.GetLine(180), 2, clrsh_TechBlack, 1);
    }

    yoff += 20;
    if (mwnd->hat_settings.deathmatch == 2 && mwnd->vis_map_context->hat_item_slot12 != 0) {
        CString local_8c;
        CString local_90;
        local_8c.Format("%d", mwnd->vis_map_context->hat_item_slot13);
        local_90.Format("%d", mwnd->vis_map_context->hat_item_slot14);

        int32_t vy1;
        int32_t vy2;

        if (mwnd->vis_map_context->hat_item_slot13 < mwnd->vis_map_context->hat_item_slot14) {
            vy2 = yoff;
            vy1 = yoff + 11;
        }
        else {
            vy1 = yoff;
            vy2 = yoff + 11;
        }

        g_font2->DrawTextWithShadow(xoff, vy1, txt_patch.GetLine(184), 0, g_colors_human_pals[2], 1);
        g_font2->DrawTextWithShadow(xoff, vy2, txt_patch.GetLine(185), 0, g_colors_human_pals[0], 1);
        g_font2->DrawTextWithShadow(rect.right - 32, vy1, local_8c, 1, g_colors_human_pals[2], 1);
        g_font2->DrawTextWithShadow(rect.right - 32, vy2, local_90, 1, g_colors_human_pals[0], 1);
    }

    for (int i = 0; i < num; i++) {
        g_font2->DrawTextWithShadow(xoff, yoff, names[i], 0, g_colors_human_pals[colors[i]], 1);
        CString st;
        if (mwnd->vis_map_context->hat_item_slot12 == 0) {
            st = txt_patch.GetLine(183 - scores[i]);
        }
        else {
            st.Format("%d", scores[i]);
        }
        g_font2->DrawTextWithShadow(rect.right - 32, yoff, st, 1, g_colors_human_pals[colors[i]], 1);

        yoff += 11;
    }
    UnlockSurface2();
}
#endif







