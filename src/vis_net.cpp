#include <winsock2.h>
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
#include "vis_net.h"
#include "gameplay.h"
#include "vis_playing_area.h"
#include "vis_fame.h"

char g_EmptyStrSrvScreenText[4]; //659a48 zeroed buffer used to clear the server screen text box
char g_EmptyStrMapTooltip[4]; //659a30 "" placeholder tooltip for net map entries with an unprintable tooltip

// Still referenced from ASM: ostream::operator<< passes it as the "" pad string and a
// RegQueryValueExA caller passes it as the "" value name, so it keeps C linkage.
extern "C" {
    char g_EmptyStrNetChatText[4]; //659a5c zeroed buffer used to clear the net chat text box
}

static char* netdlg_selected_name = nullptr;    // 659960 "-COMPUTERNAME-" anchor used to sort sessions


// 44791E
static int __cdecl CompareSessionsByName(const void* elem1, const void* elem2) {
    if (strcmp((const char*)elem1, netdlg_selected_name) == 0) {
        return -1;
    }
    if (strcmp((const char*)elem2, netdlg_selected_name) == 0) {
        return 1;
    }
    return strcmp((const char*)elem1, (const char*)elem2);
}




// 447105
static int EnumSessionsPump() {
    MSG msg;
    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
        if (msg.message == WM_QUIT) {
            return 0;
        }
        if (msg.message == VIS_MSG_CLOSE_SCREEN) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    g_mousept.Update();
    return 1;
}




// 447842
static char* ExtractFirstWord(const char* str) {
    const char* word = str;
    while (isalpha(*word) == 0) {
        if (*word == 0) {
            return nullptr;
        }
        word++;
    }
    const char* end = word;
    while (isalpha(*end) != 0) {
        end++;
    }
    int32_t len = (int32_t)(end - word);
    char* buf = new char[len + 1];
    memcpy(buf, word, len);
    buf[len] = 0;    // original writes buf[len + 1] (off-by-one); the caller only null-checks the result
    return buf;
}

// 44D8F0
int32_t VisNetMapSelection::OnKeyDown(uint32_t wparam) {
    return this->VisWindow::OnKeyDown(wparam);
}

// 44AAE5
void VisNetMapSelection::ScanMapsThread() {
    char path[MAX_PATH];
    WIN32_FIND_DATAA find_data;
    AppGetWorkingDir(0x103, path);
    strcat(path, "\\*.alm");
    HANDLE find_handle = FindFirstFileA(path, &find_data);
    if (find_handle != INVALID_HANDLE_VALUE) {
        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                NetMapInfo* info = new NetMapInfo;
                info->mapid = find_data.cFileName;
                Scenario* scenario = new Scenario;
                if (scenario->ReadAlmInfo(info)) {
                    bool name_printable = false;
                    const char* psz = (LPCTSTR)info->name;
                    if (psz != nullptr) {
                        for (; *psz != '\0'; psz++) {
                            if (isprint(*psz) || *psz < 0) {
                                name_printable = true;
                                break;
                            }
                        }
                    }
                    if (!name_printable) {
                        info->name = find_data.cFileName;
                    }
                    bool tooltip_printable = false;
                    psz = (LPCTSTR)info->tooltip;
                    if (psz != nullptr) {
                        for (; *psz != '\0'; psz++) {
                            if (isprint(*psz) || *psz < 0) {
                                tooltip_printable = true;
                                break;
                            }
                        }
                    }
                    if (!tooltip_printable) {
                        info->tooltip = g_EmptyStrMapTooltip;
                    }
                    info->name.OemToAnsi();
                    info->tooltip.OemToAnsi();
                    if (info->tooltip.IsEmpty()) {
                        info->tooltip = info->name;
                    }
                    delete scenario;
                    PostMessageA(g_MainWndHWND, 0x461, (WPARAM)info, 0);
                } else {
                    delete scenario;
                    delete info;
                }
            }
        } while (FindNextFileA(find_handle, &find_data) && WaitForSingleObject(this->stop_event, 0) != 0);
    }
    FindClose(find_handle);
}


// 44AF45
VisNetMapSelection::VisNetMapSelection(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* pMapName)
: VisWindow(_id, l, t, r, b, nullptr), thread(VisNetMapThreadProc, this) {
    this->p_mapname = pMapName;
}


// 44AFD8 (scalar deleting dtor ??_G at 44FEA0)
VisNetMapSelection::~VisNetMapSelection() {
    for (int32_t i = 0; i < this->avail_maps.GetSize(); i++) {
        delete this->avail_maps.GetAt(i);
    }
}


// 44AEEF
void VisNetMapSelection::StopThread() {
    SetEvent(this->stop_event);
    if (this->thread.m_hThread != nullptr) {
        WaitForSingleObject(this->thread.m_hThread, INFINITE);
    }
    CloseHandle(this->stop_event);
    this->stop_event = nullptr;
}


// 44D13D
void VisNetMapSelection::AddNetMapInfo(NetMapInfo* info) {
    this->avail_maps.Add(info);
    VisListBox* map_list = (VisListBox*)this->FindChild(1);
    VisScrollBar* scrollbar = (VisScrollBar*)this->FindChild(0xA);
    int32_t prev_sel = map_list->GetSelectedIndex();
    char buf[1024];
    sprintf(buf, "%s#%dx%d#%d#%d", (LPCTSTR)info->name, info->width - 0x10, info->height - 0x10,
            info->players, info->maplevel);
    map_list->AddItem(buf);
    if (prev_sel < 0) {
        prev_sel = prev_sel + 1;
        map_list->SetSelectedIndex(prev_sel);
    }
    scrollbar->SetPos(prev_sel, this->avail_maps.GetSize());
    map_list->VMethod9();
    if (prev_sel == 0) {
        PostMessageA(g_MainWndHWND, 0x46E, 1, 0);
    }
}


// 44C788
void VisNetMapSelection::VMethod26() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    this->mode = (main_wnd->sessionMode != SESSION_SINGLEPLAYER);

    this->AddChild(new VisLabel(0x19, 0x28, 0x16, this->rect.Width() - 0x28, 0x38,
                                txt_dialogs.GetLine(0x8A), g_font1, p_clrsh_Black, 2));
    this->AddChild(new VisLabel(0x1A, 0x28, 0x2C, this->rect.Width() - 0x28, 0x44,
                                txt_dialogs.GetLine(0x8B), g_font1, p_clrsh_Black, 0));
    VisNetMapList* map_list = new VisNetMapList(1, 0x28, 0x44, this->rect.Width() - 0x40, this->rect.Height() / 2,
                                                g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xA,
                                                txt_dialogs.GetLine(0x76), &this->selected_map_index);
    this->AddChild(map_list);
    map_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1A));
    CRect list_rc = map_list->GetRect();
    this->AddChild(new VisScrollBar(0xA, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    list_rc.top = 0x11C;
    list_rc.bottom = 0x180;
    list_rc.right = 0x15E;
    this->AddChild(new VisLabel(0x1B, list_rc.left, list_rc.top - 0x18, list_rc.right, list_rc.top - 6,
                                txt_dialogs.GetLine(0x8C), g_font1, p_clrsh_Black, 0));
    VisNetChatList* chat_list = new VisNetChatList(3, list_rc, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 0xB,
                                                   txt_dialogs.GetLine(0x84));
    this->AddChild(chat_list);
    chat_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1B));
    list_rc = chat_list->GetRect();
    this->AddChild(new VisScrollBar(0xB, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    CPoint btn_center(this->rect.Width() / 2, this->rect.Height() - 0x30);
    CRect btn_rc(btn_center.x - 0x30, btn_center.y - 0xC, btn_center.x + 0x30, btn_center.y + 0xC);
    OffsetRect(&btn_rc, -0x48, 0);
    VisButton* ok_btn = new VisButton(0x14, btn_rc, txt_dialogs.GetLine(0), g_font1, nullptr, 0x445, 0, "");
    this->AddChild(ok_btn);
    if (this->mode == 0) {
        ok_btn->ChangeFlags(1, 0);
    }

    list_rc.top = list_rc.bottom + 0xC;
    list_rc.bottom = list_rc.bottom + 0x24;
    VisNetChatTextBox* chat_txt = new VisNetChatTextBox(4, list_rc, g_font1, p_clrsh_Black,
                                                        txt_dialogs.GetLine(0x85));
    this->AddChild(chat_txt);
    chat_txt->SetUpObj(chat_list);

    list_rc.left = 0x181;
    list_rc.right = 0x20C;
    list_rc.top = 0x11C;
    list_rc.bottom = 0x1A4;
    this->AddChild(new VisLabel(0x1C, list_rc.left, list_rc.top - 0x18, list_rc.right, list_rc.top - 6,
                                txt_dialogs.GetLine(0x8D), g_font1, p_clrsh_Black, 0));
    VisNetPlayerList* player_list = new VisNetPlayerList(5, list_rc, g_font1, p_clrsh_Black, p_clrsh_Black, 0xC,
                                                         txt_dialogs.GetLine(0x89));
    this->AddChild(player_list);
    player_list->SetCaptionLabel((VisLabel*)this->FindChild(0x1C));
    PostMessageA(g_MainWndHWND, 0x460, 0, 0);
    list_rc = player_list->GetRect();
    this->AddChild(new VisScrollBar(0xC, list_rc.right, list_rc.top, list_rc.right + 0x18, list_rc.bottom, nullptr));

    OffsetRect(&btn_rc, 0x90, 0);
    VisButton* cancel_btn = new VisButton(0x15, btn_rc, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(cancel_btn);
    cancel_btn->SetLeftObj(this->FindChild(0x14));

    this->stop_event = CreateEventA(nullptr, 0, 0, nullptr);
    if (this->stop_event == nullptr) {
        AfxThrowMemoryException();
        return;
    }
    this->map_context = main_wnd->vis_map_context;
    this->chat_log = &this->map_context->msglog;
    this->selected_map_index = 0;
    this->thread.CreateThread(0, 0, nullptr);
    this->thread.m_bAutoDelete = 0;
}


// 44D275
int32_t VisNetMapSelection::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (wparam != 1) {
            return 0;
        }
        // fallthrough
    case VIS_MSG_HIDE_SCREEN:
        if (this->FindChild(0x14)->TestFlags(1)) {
            this->StopThread();
            this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            VisListBox* map_list = (VisListBox*)this->FindChild(1);
            NetMapInfo* info = this->avail_maps.GetAt(map_list->GetSelectedIndex());
            *this->p_mapname = info->mapid;
        }
        return 1;
    case VIS_MSG_CLOSE_SCREEN:
        this->StopThread();
        this->VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        return 1;
    case VIS_MSG_NETMAP_APPLY:
        if (main_wnd->sessionMode == SESSION_SINGLEPLAYER) {
            this->StopThread();
            if (this->mode == 0) {
                VisListBox* map_list = (VisListBox*)this->FindChild(1);
                CString row = map_list->GetItem(this->selected_map_index);
                int32_t level = row[row.GetLength() - 1] - '1';
                if (level < 0) {
                    level = 0;
                }
                if (level > 3) {
                    level = 3;
                }
                bool can_join = (level + 1 >= main_wnd->m_GameSession.GetSkillTier1())
                    && (main_wnd->m_GameSession.GetSkillTier2() >= level + 1);
                this->VisScreen::MsgProc(can_join ? 0x445 : 0x446, 0, 0);
            } else {
                this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            }
        }
        return 1;
    case VIS_MSG_NETMAP_CHAT_SYNC: {
        VisNetChatList* chat_list = (VisNetChatList*)this->FindChild(3);
        int32_t count = chat_list->GetItemCount();
        int32_t sel = chat_list->GetSelectedIndex();
        VisScrollBar* scrollbar = (VisScrollBar*)this->FindChild(0xB);
        scrollbar->SetPos(sel, count);
        int32_t size = this->chat_log->text.GetSize();
        chat_list->AddItem(this->chat_log->text[size - 1]);
        chat_list->colors.Add(this->chat_log->color.GetAt(size - 1));
        if (chat_list->GetItemCount() - 2 == chat_list->selected_index) {
            chat_list->SelectItem(chat_list->vis_start_index + chat_list->num_vis_entry);
        }
        chat_list->VMethod9();
        return 1;
    }
    case VIS_MSG_ROSTER_CHANGED: {
        VisPlayingArea* map_context = main_wnd->vis_map_context;
        VisNetPlayerList* player_list = (VisNetPlayerList*)this->FindChild(5);
        player_list->entries.RemoveAll();
        player_list->selected_index = -1;
        player_list->colors.RemoveAll();
        for (int32_t i = 0x10; i < map_context->map_players.GetSize(); i++) {
            MapPlayerData* pd = map_context->map_players.GetAt(i);
            if (pd != nullptr) {
                player_list->AddItem(pd->name);
                player_list->colors.Add(g_colors_human_pals + pd->color);
            }
        }
        player_list->VMethod9();
        return 1;
    }
    case VIS_MSG_NETMAP_ADD_INFO:
        this->AddNetMapInfo((NetMapInfo*)wparam);
        return 1;
    case VIS_MSG_SET_OPTION:
    case VIS_MSG_LIST_SELECT:
        if (wparam == 1) {
            if (lparam >= 0 && this->mode != 0) {
                this->selected_map_index = lparam;
                VisListBox* map_list = (VisListBox*)this->FindChild(1);
                map_list->VMethod9();
                NetMapInfo* info = this->avail_maps.GetAt(this->selected_map_index);
                this->map_context->SendMapAnnouncement((LPCTSTR)info->name);
                CString row = map_list->GetItem(lparam);
                int32_t level = row[row.GetLength() - 1] - '1';
                if (level < 0) {
                    level = 0;
                }
                if (level > 3) {
                    level = 3;
                }
                bool can_join = (level + 1 >= main_wnd->m_GameSession.GetSkillTier1())
                    && (main_wnd->m_GameSession.GetSkillTier2() >= level + 1);
                this->FindChild(0x14)->ChangeFlags(1, can_join);
                this->FindChild(0x14)->VMethod9();
            }
            return 1;
        }
        return 0;
    case VIS_MSG_NETMAP_FIND_MAP:
        if (this->mode == 0) {
            if (lparam >= 0 && lparam < (uint32_t)this->avail_maps.GetSize()) {
                for (int32_t i = 0; i < this->avail_maps.GetSize(); i++) {
                    if (_strcmpi((const char*)wparam, (LPCTSTR)this->avail_maps.GetAt(i)->name) == 0) {
                        this->selected_map_index = i;
                        break;
                    }
                }
            }
            this->FindChild(1)->VMethod9();
        }
        return 1;
    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 450080 (the CPtrArray `colors` member at 0x94 is member-constructed by the compiler)
VisNetChatList::VisNetChatList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}

// 44c63d
void VisNetChatList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    const char* txt = this->entries[idx];

    CRect client;
    this->ClientRectToScreen(&client, this->rect);

    CRect saved;
    GetClipRect(&saved);

    CRect clip(pos.x, pos.y, client.right - 6, pos.y + this->font->GetHeight() + 2);
    SetClipRect(clip);

    this->font->DrawTextWithShadow(pos.x, pos.y, txt, 0, (uint16_t*)this->colors[idx], 1);

    SetClipRect(saved);
}


// 450180 (the CPtrArray `colors` member at 0x94 is member-constructed by the compiler)
VisNetPlayerList::VisNetPlayerList(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint)
    : VisListBox(_id, r, _font, _clr1, _clr2, _scrollid, hint) {
}

// 44c71d
void VisNetPlayerList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    const char* txt = this->entries[idx];
    this->font->DrawTextWithShadow(pos.x, pos.y, txt, 0, (uint16_t*)this->colors[idx], 1);
}


// 44ffd0
VisNetMapList::VisNetMapList(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _scrollid, const char* hint, int32_t* p_selected_map_index)
    : VisListBox(_id, l, t, r, b, _font, _clr1, _clr2, _scrollid, hint) {
    this->p_selected_map_index = p_selected_map_index;
}

// 44b09e
void VisNetMapList::DrawItem(int32_t idx, CPoint pos, uint16_t* clr) {
    if (!this->IsValidIndex(idx)) {
        return;
    }

    if (idx == *this->p_selected_map_index) {
        g_bmp_server->Draw(pos.x, pos.y, 0, 0, 0);
    }

    CString& item = this->entries[idx];
    char* txt = (char*)(const char*)item;

    int32_t level = item[item.GetLength() - 1] - '1';
    if (level < 0) {
        level = 0;
    }
    if (level > 3) {
        level = 3;
    }

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->m_GameSession.GetSkillTier1() > level + 1
        || main_wnd->m_GameSession.GetSkillTier2() < level + 1) {
        clr = g_colors_human_pals[15];
    }

    pos.x += 0x1E;

    char* field = txt;
    char* sep = strchr(field, '#');
    *sep = '\0';
    this->font->DrawTextWithShadow(pos.x, pos.y, field, 0, clr, 1);
    *sep = '#';

    pos.x += 0x10E;
    field = sep + 1;
    sep = strchr(field, '#');
    *sep = '\0';
    this->font->DrawTextWithShadow(pos.x, pos.y, field, 0, clr, 1);
    *sep = '#';

    pos.x += 0x54;
    field = sep + 1;
    sep = strchr(field, '#');
    *sep = '\0';
    *sep = '#';
    this->font->DrawTextWithShadow(pos.x, pos.y, txt_patch.GetLine(level + 0x62), 0, clr, 1);
}

// 44b2bf
const char* VisNetMapList::GetHint() {
    CRect client;
    this->ClientRectToScreen(&client, this->rect);

    if (g_mousept.x < client.left + 0x12C) {
        int32_t idx = this->vis_start_index + this->YToIndex(g_mousept.y);
        if (!this->IsValidIndex(idx)) {
            return nullptr;
        }

        VisNetMapSelection* screen = (VisNetMapSelection*)this->parent;
        NetMapInfo* info = screen->avail_maps[idx];
        return info->tooltip;
    }

    if (g_mousept.x < client.left + 0x180) {
        return txt_dialogs.GetLine(0x86);
    }

    return txt_dialogs.GetLine(0x88);
}


// 450920
VisServerScreenRadio::VisServerScreenRadio(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint)
    : VisRadioType1(_id, l, t, r, b, _font, _clr, hint) {
}

// 4db3ad
void VisServerScreenRadio::VMethod7() {
    CRect r;
    this->ClientRectToScreen(&r, this->rect);
    r.bottom += 4;
    r.right += 4;

    int32_t txt_x = r.left + gfx_radiob->GetWidth(4) + 6;
    int32_t y = r.top;

    POINT cursor;
    GetCursorPos(&cursor);

    LockSurface2();

    this->parent->VMethod8(&r);

    r.bottom -= 4;
    r.right -= 4;

    for (int32_t i = 0; i < this->entries.GetSize(); i++) {
        int32_t flag = 1 << i;
        if (this->selection & flag) {
            gfx_radiob->DrawBrightened(r.left + 5, y + 4, 5, 4, 0);
            gfx_radiob->Draw(r.left + 1, y, 5, 0, 0);
        } else {
            gfx_radiob->DrawBrightened(r.left + 5, y + 4, 4, 4, 0);
            gfx_radiob->Draw(r.left + 1, y, 4, 0, 0);
        }

        CRect entry(r.left, y, r.right, y + gfx_radiob->GetHeight(4));
        if (entry.PtInRect(cursor)) {
            this->clr = clrsh_CharlieBrown;
        } else {
            this->clr = clrsh_DullGold;
        }

        this->font->DrawTextWithShadow(txt_x, y + 3, this->entries[i], 0, this->clr, 1);

        y += gfx_radiob->GetHeight(4);
    }

    if (!this->TestFlags(1)) {
        CRect r2;
        this->ClientRectToScreen(&r2, this->rect);
        ShadowRect(CRect(r2.left - 1, r2.top - 1, r2.right + 1, r2.bottom + 1), 3);
    }

    UnlockSurface2();
}

// 4db615
int32_t VisServerScreenRadio::GetIndex(int32_t y) {
    CRect client;
    this->ClientRectToScreen(&client, this->rect);
    return (y - client.top) / gfx_radiob->GetHeight(4);
}


// 450280
VisNetChatTextBox::VisNetChatTextBox(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr, const char* hint)
    : VisTextBox(_id, r, _font, _clr, hint) {
}

// 4502c0
int32_t VisNetChatTextBox::OnKeyDown(uint32_t wparam) {
    if (wparam == 0xD) {
        char buf[256];
        this->WriteData(buf);
        this->ReadData(g_EmptyStrNetChatText);
        this->VMethod9();

        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->vis_map_context->SendChatPacket(buf, CHAT_TYPE_GLOBAL, 0);
        return 1;
    }

    return VisTextBox::OnKeyDown(wparam);
}

// 446567
void VisNetDlg::CreateSessionList() {
    int32_t x = (this->rect.Width() - 0xB0) / 2;
    int32_t y = this->rect.Height() - 0x90;
    CRect rc_list(0x28, 0x50, x + 0x28, y + 0x30);

    this->AddChild(new VisLabel(0x19, 0x28, 0x20, this->rect.Width() - 0x28, 0x38, txt_dialogs.GetLine(0x8E), g_font1, p_clrsh_Black, 2));

    VisListBox* server_list = new VisListBox(1, rc_list, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 2, txt_dialogs.GetLine(0x52));
    this->AddChild(server_list);

    VisScrollBar* server_scroll = new VisScrollBar(2, server_list->GetRect().left, server_list->GetRect().top, server_list->GetRect().right + 0x18, server_list->GetRect().bottom, nullptr);
    this->AddChild(server_scroll);

    this->AddChild(new VisLabel(4, server_list->GetRect().left, server_list->GetRect().top - 0x18, server_list->GetRect().right, server_list->GetRect().top, txt_dialogs.GetLine(0x53), g_font1, p_clrsh_Black, 0));
    server_list->SetCaptionLabel((VisLabel*)this->FindChild(4));

    CRect rc_players(server_scroll->GetRect().right + 0x18, server_list->GetRect().top, server_scroll->GetRect().right + x + 0x18, server_list->GetRect().bottom);
    VisListBox* player_list = new VisListBox(7, rc_players, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 8, txt_dialogs.GetLine(0x54));
    this->AddChild(player_list);

    this->AddChild(new VisLabel(6, player_list->GetRect().left, player_list->GetRect().top - 0x18, player_list->GetRect().right, player_list->GetRect().top, txt_dialogs.GetLine(0x55), g_font1, p_clrsh_Black, 0));
    player_list->SetCaptionLabel((VisLabel*)this->FindChild(6));

    VisScrollBar* player_scroll = new VisScrollBar(8, player_list->GetRect().left, player_list->GetRect().top, player_list->GetRect().right + 0x18, player_list->GetRect().bottom, nullptr);
    this->AddChild(player_scroll);

    int32_t btn_top = this->rect.Height() - 0x3C;
    int32_t btn_bottom = this->rect.Height() - 0x24;
    int32_t w = this->rect.Width();
    CRect rc_refresh(w * 3 / 26, btn_top, w * 7 / 26, btn_bottom);
    CRect rc_join(w * 8 / 26, btn_top, w * 12 / 26, btn_bottom);
    CRect rc_create(w * 13 / 26, btn_top, w * 17 / 26, btn_bottom);
    CRect rc_exit(w * 18 / 26, btn_top, w * 22 / 26, btn_bottom);

    VisButton* refresh_btn = new VisButton(0x11, rc_refresh, txt_dialogs.GetLine(0x53), g_font1, nullptr, 0x450, 0, "");
    this->AddChild(refresh_btn);

    VisButton* join_btn = new VisButton(0xF, rc_join, txt_dialogs.GetLine(0x57), g_font1, nullptr, 0x44F, 0, "");
    join_btn->ChangeFlags(0x10, true);
    this->AddChild(join_btn);
    join_btn->ChangeFlags(1, false);
    join_btn->SetLeftObj(this->FindChild(0x11));

    VisButton* create_btn = new VisButton(0xE, rc_create, txt_dialogs.GetLine(0x56), g_font1, nullptr, 0x44E, 0, "");
    this->AddChild(create_btn);
    if (g_IsCdPresent == 0) {
        create_btn->ChangeFlags(1, false);
    }
    create_btn->SetLeftObj(this->FindChild(0xF));

    VisButton* exit_btn = new VisButton(0x10, rc_exit, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(exit_btn);
    exit_btn->SetLeftObj(this->FindChild(0xE));
}

// 447C27
void VisNetDlg::CachePlayerRows() {
    for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
        CStringArray* row = this->cached_player_rows.GetAt(i);
        row->RemoveAll();
        if (row != nullptr) {
            delete row;
        }
    }
    this->cached_player_rows.RemoveAll();

    char* comp_name = new char[0x14];
    netdlg_selected_name = comp_name;
    comp_name[0] = '-';
    unsigned long name_size = 0x11;
    GetComputerNameA(comp_name + 1, &name_size);
    strcat(comp_name, "-");

    if (this->sessions->sessions != nullptr && this->sessions->num_sessions != 0) {
        qsort(this->sessions->sessions, this->sessions->num_sessions, sizeof(CLlNetSession), CompareSessionsByName);
    }
    delete[] comp_name;

    this->cached_player_rows.SetSize(this->sessions->num_sessions, -1);
    for (int32_t i = 0; i < this->sessions->num_sessions; i++) {
        CLlName* names = nullptr;
        int32_t num_names = 0;
        g_CLlDriver.EnumPlayers(&this->sessions->sessions[i], &names, &num_names);

        CStringArray* row = new CStringArray();
        row->SetSize(num_names, -1);
        for (int32_t j = 0; j < num_names; j++) {
            CString name(names[j].name);
            row->ElementAt(j) = name;
        }
        this->cached_player_rows.ElementAt(i) = row;
    }
}


// 44797E
void VisNetDlg::FillServerList() {
    VisListBox* server_list = (VisListBox*)this->FindChild(1);
    int32_t count = server_list->GetItemCount();

    CString sel_name;
    if (count > this->selected) {
        sel_name = server_list->GetItem(this->selected);
    } else {
        sel_name = "When night are cold and friends are few I sit alone and think of you";
    }
    while (count != 0) {
        count--;
        server_list->RemoveItem(count);
    }

    int32_t num_sessions = this->sessions->num_sessions;
    this->selected = 0;
    for (int32_t i = 0; i < num_sessions; i++) {
        server_list->AddItem(this->sessions->sessions[i].name);

        CString item(this->sessions->sessions[i].name);
        if (item == sel_name) {
            this->selected = i;
            server_list->SetSelectedIndex(i);
        }
    }

    VisButton* join_btn = (VisButton*)this->FindChild(0xF);
    if (num_sessions == 0) {
        join_btn->ChangeFlags(1, false);
        join_btn->SetDowned(false);
    } else {
        join_btn->ChangeFlags(1, true);
    }
    join_btn->VMethod9();
    server_list->VMethod9();
}


// 447B3C
void VisNetDlg::UpdatePlayerList() {
    VisListBox* player_list = (VisListBox*)this->FindChild(7);
    int32_t count = player_list->GetItemCount();
    while (count > 0) {
        count--;
        player_list->RemoveItem(count);
    }

    if (this->cached_player_rows.GetSize() > 0) {
        CStringArray* row = this->cached_player_rows.GetAt(this->selected);
        int32_t size = row->GetSize();
        int32_t i;
        for (i = 0; i < size; i++) {
            player_list->AddItem(row->ElementAt(i));
        }
        ((VisScrollBar*)this->FindChild(8))->SetPos(0, i);
    }
    player_list->VMethod9();
}

// 447173
void VisNetDlg::RefreshSessions() {
    uint32_t timeout = 0;
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == PROVIDER_DP_TCPIP) {
        timeout = 0x5DC;
    } else if (provider == PROVIDER_IPX) {
        timeout = 0x12C;
    } else if (provider == PROVIDER_MODEM) {
        timeout = 0xEA60;
    }

    g_CLlDriver.EnumSessions(&this->sessions->sessions, &this->sessions->num_sessions, EnumSessionsPump, timeout);
    this->CachePlayerRows();
    PostMessageA(g_MainWndHWND, 0x471, 0, 0);
}

// 447214
int32_t VisNetDlg::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        if (this->FindChild(0xF)->TestFlags(1) == 0) {
            return 0;
        }
        // fall through
    case VIS_MSG_NET_CREATE:
    case VIS_MSG_NET_JOIN: {
        this->sessions->character_name = main_wnd->m_GameSession.character_name;
        VisListBox* server_list = (VisListBox*)this->FindChild(1);
        this->sessions->selected_index = server_list->GetSelectedIndex();
        this->DoClose(msg);
        main_wnd->PostMessage(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);

        for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
            CStringArray* row = this->cached_player_rows.GetAt(i);
            if (row != nullptr) {
                delete row;
            }
        }
        this->cached_player_rows.RemoveAll();
        return 1;
    }
    case VIS_MSG_CLOSE_SCREEN: {
        VisScreen::MsgProc(msg, wparam, lparam);
        main_wnd->dialogsMask = DIALOG_MASK_NONE;

        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == PROVIDER_MODEM) {
            g_CLlDriver.Close();
            PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
        } else {
            provider = g_CLlDriver.GetProvider();
            if (provider == PROVIDER_SERIAL) {
                g_CLlDriver.Close();
                PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_SERIAL_SETTINGS, 0, 0);
            } else {
                provider = g_CLlDriver.GetProvider();
                if (provider == PROVIDER_DP_TCPIP) {
                    g_CLlDriver.Close();
                    if (main_wnd->net_session.internet_game == 0) {
                        PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_TCP_SETTINGS, 0, 0);
                    } else {
                        PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_HAT_BROWSER, 0, 0);
                    }
                } else {
                    g_CLlDriver.Close();
                    PostMessageA(g_MainWndHWND, MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
                }
            }
        }

        for (int32_t i = 0; i < this->cached_player_rows.GetSize(); i++) {
            CStringArray* row = this->cached_player_rows.GetAt(i);
            if (row != nullptr) {
                delete row;
            }
        }
        this->cached_player_rows.RemoveAll();
        return 1;
    }
    case VIS_MSG_NET_REFRESH:
        this->RefreshSessions();
        return 1;
    case VIS_MSG_SET_OPTION:
        if (wparam == 1) {
            this->selected = ((int32_t)lparam < 0) ? 0 : (int32_t)lparam;
            this->UpdatePlayerList();
        }
        return 1;
    case VIS_MSG_NET_APPLY_PROVIDER: {
        int32_t provider = g_CLlDriver.GetProvider();
        if (provider == PROVIDER_SERIAL || provider == PROVIDER_MODEM) {
            this->rect = this->net_rect;
            this->sessions->character_name = main_wnd->m_GameSession.character_name;
            this->sessions->selected_index = 0;

            if (this->sessions->num_sessions == 0) {
                this->RemoveAllChilds();

                this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x7C), g_font1, clrsh_TechBlack, 0));

                int32_t h = this->rect.Height();
                int32_t w = this->rect.Width();
                this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, clrsh_TechBlack, 0x446, 0, ""));
            } else {
                if (g_CLlDriver.GetProvider() == PROVIDER_MODEM) {
                    char* word = ExtractFirstWord(main_wnd->phone_book.phones.ElementAt(0));
                    if (word != nullptr) {
                        delete[] word;
                    }
                }
                this->DoClose(VIS_MSG_NET_JOIN);
                main_wnd->PostMessage(MAIN_MSG_CHILD_CLOSED, (WPARAM)this, 0);
                AfxGetMainWnd()->PostMessage(0x44F, 0, 0);
            }
        } else {
            this->FillServerList();
            this->UpdatePlayerList();
            this->sessions->selected_row = this->selected;
        }
        return 1;
    }
    case VIS_MSG_NET_NEW_SESSION:
        g_CLlDriver.SetEventNewSession();
        return 1;
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 446E97
void VisNetDlg::VMethod26() {
    int32_t provider = g_CLlDriver.GetProvider();
    if (provider == PROVIDER_SERIAL || provider == PROVIDER_MODEM) {
        this->net_rect = this->rect;
        this->rect = CRect(0, 0, 0x17C, 0xC4);
        this->UpdateWinRect();

        if (g_CLlDriver.GetProvider() == PROVIDER_MODEM) {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0x77), g_font1, p_clrsh_Black, 0));
        } else {
            this->AddChild(new VisLabel(1, 0x28, 0x30, 0xDC, 0x4A, txt_dialogs.GetLine(0xA3), g_font1, p_clrsh_Black, 0));
        }

        int32_t h = this->rect.Height();
        int32_t w = this->rect.Width();
        this->AddChild(new VisButton(2, w / 2 - 0x30, h / 2 + 0xC, w / 2 + 0x30, h / 2 + 0x24, txt_dialogs.GetLine(1), g_font1, nullptr, 0x47C, 0, ""));
    } else {
        this->CreateSessionList();
    }

    this->selected = 0;
    PostMessageA(g_MainWndHWND, 0x450, 0, 0);
}


// 4478E9
int32_t VisNetDlg::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x1B) {
        return this->MsgProc(VIS_MSG_NET_NEW_SESSION, 0, 0);
    }
    return VisWindow::OnKeyDown(wparam);
}


// 4464C7
VisNetDlg::VisNetDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, AvailNetSession* _sessions)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->sessions = _sessions;
    this->sessions->sessions = nullptr;
    this->sessions->num_sessions = 0;
}


// 44FA90
VisNetDlg::~VisNetDlg()
{}


// 4ADAB6
void VisFameDocument::VMethod26() {
    this->fame = nullptr;
    this->bmp_sheet = nullptr;
    this->rect_prev = CRect(CPoint(0, 200), CSize(0x38, 0x28));
    this->rect_next = CRect(CPoint(0x240, 200), CSize(0x3C, 0x28));
    this->rect_ok = CRect(CPoint(0x230, 0x1A0), CSize(0x2C, 0x20));
    this->bmp_cur_left = nullptr;
    this->bmp_cur_right = nullptr;
    this->bmp_cur_ok = nullptr;
    this->AddChild(new VisButton(4, 0, 0, 0, 0, "", g_font1, clrsh_TechBlack, 0x445, 0, nullptr));
    this->visible_flag = 0;
}

// 44DBE0
void VisServerScreen::VMethod7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    CString str;
    LockSurface2();
    FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    this->CVisualObject::VMethod7();
    g_font2->DrawTextWithShadow(0, 0, txt_patch.GetLine(0x39), 0, clrsh_ShockingBlack, 1);
    FillRectColorSimple(0, 0x10, this->rect.right, 0x11, 0xffff);
    FillRectColorSimple(0, 0x20, this->rect.right, 0x21, 0xffff);
    FillRectColorSimple(0, 0xC8, this->rect.right, 0xC9, 0xffff);
    FillRectColorSimple(0, 0xD8, this->rect.right, 0xD9, 0xffff);

    int32_t row_top = 0x10;
    int32_t row_bottom = 0xC8;
    int32_t y = 0x24;
    int32_t i = 0;
    POSITION it = g_NetStru1_main.client_stat.GetStartPosition();
    while (it != nullptr && i < this->scroll_pos + 0x10) {
        int32_t key;
        ConnStatInfo* stat;
        g_NetStru1_main.client_stat.GetNextAssoc(it, key, stat);
        if (i < this->scroll_pos) {
            i++;
            continue;
        }
        NetStru2* client = g_NetStru1_main.GetClientByLowUid((uint32_t)key >> 16);
        Player* player = g_PlayersList->FindById(client->player_id);
        if (player == nullptr) {
            i++;
            continue;
        }
        uint16_t* colosh = g_colors_human_pals[player->player_id & 0xF];
        int32_t bytes_per_sec = 0;
        if (stat->time != 0) {
            bytes_per_sec = stat->total_bytes / stat->time;
        }
        int32_t x = 0;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0xC;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3A), 2, clrsh_ShockingBlack);
        str.Format("%d", (int16_t)player->player_id);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0xC;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x30;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3B), 2, clrsh_ShockingBlack);
        g_font2->DrawTxt(x, y, player->name, 2, colosh);
        x += 0x30;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x24;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3C), 2, clrsh_ShockingBlack);
        str.Format("%d:%02d:%02d", stat->time / 0xE10, (stat->time % 0xE10) / 0x3C, stat->time % 0x3C);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x24;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3D), 2, clrsh_ShockingBlack);
        str.Format("%d", stat->cur_bs);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3E), 2, clrsh_ShockingBlack);
        str.Format("%d", bytes_per_sec);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x3F), 2, clrsh_ShockingBlack);
        str.Format("%d", stat->max_bs);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0x18;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x40), 2, clrsh_ShockingBlack);
        str.Format("%d", player->monster_kills);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        x += 0x18;
        if (i == this->scroll_pos) {
            FillRectColor(x, row_top, x, row_bottom, 0xffff);
        }
        x += 0xC;
        g_font2->DrawTxt(x, 0x14, txt_patch.GetLine(0x41), 2, clrsh_ShockingBlack);
        str.Format("%d", player->player_kills);
        g_font2->DrawTxt(x, y, str, 2, colosh);
        if (i == this->scroll_pos) {
            FillRectColor(this->rect.right - 1, row_top, this->rect.right - 1, row_bottom, 0xffff);
        }
        y += 10;
    }

    int32_t avg_units_per_10_ticks = 0;
    if (g_Server->tic16 != 0) {
        avg_units_per_10_ticks = g_Server->total_ms / g_Server->tic16 / 10;
    }
    if (g_Server->srv_stru1->sack_list != nullptr) {
        str.Format(txt_patch.GetLine(0x42),
                   main_wnd->current_map_name,
                   g_PlayersList->CountHumanPlayers(),
                   g_ServerUnits->unit_list.GetCount(),
                   g_Server->srv_stru1->building_list->GetCount(),
                   g_Server->srv_stru1->units_list->unit_list.GetCount(),
                   g_Server->srv_stru1->sack_list->list.GetCount(),
                   g_Server->last_window_ms / 10,
                   avg_units_per_10_ticks);
        g_font2->DrawTxt(0, row_bottom + 4, str, 0, clrsh_ShockingBlack);
    }
    this->msg_log->Draw();
    UnlockSurface2();
}


// 44D909
void VisServerScreen::VMethod26() {
    this->scroll_pos = 0;

    this->AddChild(new VisButton(0x63, 0x140, g_ScreenSize.bottom - 0x1E, 0x1A4, g_ScreenSize.bottom - 0x0C,
                                 txt_dialogs.GetLine(0x0E), g_font2, nullptr, 0x445, 0, nullptr));
    this->AddChild(new VisButton(0x64, 0x1B8, g_ScreenSize.bottom - 0x1E, 0x21C, g_ScreenSize.bottom - 0x0C,
                                 txt_dialogs.GetLine(0x2A), g_font2, nullptr, 0x446, 0, nullptr));

    VisServerScreenRadio* save_on_server = new VisServerScreenRadio(0x65, 0, g_ScreenSize.bottom - 0x1C, 0x12C,
                                                                    g_ScreenSize.bottom - 0x0C, g_font2, nullptr, nullptr);
    save_on_server->AddEntry(txt_patch.GetLine(0x38));
    this->AddChild(save_on_server);

    int32_t checked = 1;
    if (strstr(afxCurrentWinApp->m_lpCmdLine, "-saveonserver") != nullptr
        || strstr(afxCurrentWinApp->m_lpCmdLine, "-internetserver") != nullptr) {
        g_Server->save_on_server = 0;
    }
    if (g_Server->save_on_server != 0) {
        checked = 0;
    }
    save_on_server->ReadData(&checked);

    this->AddChild(new VisTextBox(0x68, 4, g_ScreenSize.bottom - 0x38, g_ScreenSize.right - 4, g_ScreenSize.bottom - 0x28,
                                  g_font2, clrsh_ShockingBlack, nullptr));
}

// 44E6DE
int32_t VisServerScreen::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x0D) {  // Enter: send the chat text box content
        PacketJoin& pkt = PacketJoin::Inst;
        pkt.id = CHAT;
        pkt.to_player_id = 0;
        pkt.__as_dword = 0;

        CVisualObject* textbox = this->FindChild(0x68);
        textbox->WriteData(pkt.name);

        if (pkt.name[0] == '#') {
            ExecuteAdminChatCommand(&pkt.name[1]);
        } else {
            LogMessage(pkt.name);
            g_NetStru1_main.QueuePacketSend(&pkt);
        }

        CVisualObject* textbox_clear = this->FindChild(0x68);
        textbox_clear->ReadData(g_EmptyStrSrvScreenText);
        return 1;
    }

    CVisualObject* textbox = this->FindChild(0x68);
    return textbox->OnKeyDown(wparam);
}


// 44E7C8
int32_t VisServerScreen::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_CLOSE_SCREEN:  // "disconnect" button
        this->VisScreen::MsgProc(VIS_MSG_CLOSE_SCREEN, 0, 0);
        PostMessageA(g_MainWndHWND, MAIN_MSG_LEAVE_MISSION, 0, 0);
        return 1;

    case VIS_MSG_SET_OPTION:  // "save on server" checkbox changed
        if (wparam == 0x65) {
            g_Server->save_on_server = (lparam == 0) ? 1 : 0;
            return 1;
        }
        return this->VisScreen::MsgProc(msg, wparam, lparam);

    case 0x100:  // WM_KEYDOWN
        this->OnKeyDown(wparam);
        return 1;

    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 44DBAE
void VisServerScreen::VMethod8(CRect* rect) {
    FillRectColorSimple(rect->left, rect->top, rect->right, rect->bottom, 0);
}


// 44e469
VisServerScreen::VisServerScreen(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, GM_a28* log)
: VisScreen(_id, l, t, r, b, nullptr) {
    this->msg_log = log;
}

// 44878A
int32_t VisNetPhoneBook::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_LIST_ACTIVATE:
        PostMessageA(g_MainWndHWND, 0x46e, wparam, lparam);
        PostMessageA(g_MainWndHWND, 0x47d, 0, 0);
        return 1;
    case VIS_MSG_CLOSE_SCREEN: {
        int32_t res = this->VisScreen::MsgProc(msg, wparam, lparam);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        main_wnd->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
        return res;
    }
    case VIS_MSG_SET_OPTION:
        if (wparam == 1) {
            CVisualObject* edit = this->FindChild(3);
            const char* str;
            if ((int32_t)lparam < 0) {
                str = "";
            } else {
                str = this->phones->phones[lparam];
            }
            this->UpdateConnectButton(str);
            edit->ReadData(str);
            edit->VMethod9();
        }
        if (wparam == 3) {
            CVisualObject* edit = this->FindChild(3);
            char buf[64];
            edit->WriteData(buf);
            this->UpdateConnectButton(buf);
        }
        return 1;
    case VIS_MSG_PHONE_CONNECT: {
        CVisualObject* connect_btn = this->FindChild(10);
        if (connect_btn->TestFlags(1) == 0) {
            return 1;
        }
        char string[64];
        CVisualObject* edit = this->FindChild(3);
        edit->WriteData(string);
        this->phones->dial = string;
        CString dial_str(string);
        bool found = false;
        for (int32_t i = 0; i < this->phones->phones.GetSize(); i++) {
            CString name = this->phones->phones[i];
            if (name == this->phones->dial) {
                found = true;
            }
        }
        if (!found) {
            this->phones->phones.InsertAt(0, dial_str, 1);
        }
        VisListBox* list = (VisListBox*)this->FindChild(7);
        int32_t sel = list->GetSelectedIndex();
        CLlAddress* addr = this->enum_addresses + sel;
        if (g_CLlDriver.PrepareForConnect(this->phones->dial, addr)) {
            MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
            main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_PHONE_DIAL: {
        this->VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
        VisListBox* list = (VisListBox*)this->FindChild(7);
        int32_t sel = list->GetSelectedIndex();
        CLlAddress* addr = this->enum_addresses + sel;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, addr) == 0) {
            VisMessageBoxWithList* msgbox = new VisMessageBoxWithList(
                1, 100, 100, 0x21c, 0x17c, TxtFile::AllLines.GetAt(0x9a), nullptr, 0);
            main_wnd->ModalScreen(msgbox);
            main_wnd->PostMessage(MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
        } else {
            main_wnd->PostMessage(MAIN_MSG_ENTER_CLIENT_MODE, 0, 0);
        }
        return 1;
    }
    default:
        return this->VisScreen::MsgProc(msg, wparam, lparam);
    }
}


// 447EF2
void VisNetPhoneBook::VMethod26() {
    g_CLlDriver.EnumAddresses(&this->enum_addresses, &this->enum_addresses_num);

    VisLabel* title = new VisLabel(-1, 0x28, 0x14, this->rect.Width() - 0x28, 0x2c,
        txt_dialogs.GetLine(0x93), g_font1, p_clrsh_Black, 2);
    this->AddChild(title);

    int32_t third_width = this->rect.Width() / 3;
    CRect rc(0x28, 0x50, third_width + 0x28, this->rect.Height() - 0x40);

    VisListBoxPhoneBook* phone_list = new VisListBoxPhoneBook(1, rc, g_font1, p_clrsh_Black,
        p_clrsh_ShockingBlack, 2, txt_dialogs.GetLine(0x5a), &this->phones->phones);
    this->AddChild(phone_list);

    for (int32_t i = 0; i < this->phones->phones.GetSize(); i++) {
        phone_list->AddItem(this->phones->phones[i]);
    }

    CRect rc2(phone_list->GetRect());

    VisScrollBar* scrollbar = new VisScrollBar(2, rc2.right, rc2.top, rc2.right + 0x18, rc2.bottom, nullptr);
    this->AddChild(scrollbar);

    VisLabel* phone_caption = new VisLabel(4, rc2.left + 0xa, rc2.top - 0x1e, rc2.right + 0xa, rc2.top - 0xa,
        txt_dialogs.GetLine(0x5c), g_font1, p_clrsh_Black, 2);
    this->AddChild(phone_caption);

    phone_list->SetCaptionLabel((VisLabel*)this->FindChild(4));

    VisLabel* addr_caption = new VisLabel(-2, rc2.right + 0x30, rc2.top - 0x1e, rc2.right + third_width * 3, rc2.top - 0xa,
        txt_dialogs.GetLine(0x98), g_font1, p_clrsh_Black, 0);
    this->AddChild(addr_caption);

    VisTextBox* edit = new VisTextBox(3, rc2.right + 0x30, rc2.top, rc2.right + (third_width * 3) / 2, rc2.top + 0x18,
        g_font1, p_clrsh_Black, txt_dialogs.GetLine(0x5b));
    this->AddChild(edit);

    CRect rc3(edit->GetRect());
    rc3.OffsetRect(0, rc3.Height() + 4);
    rc3.bottom = rc3.top + 0x30;

    VisListBox* addr_list = new VisListBox(7, rc3, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 8, txt_dialogs.GetLine(0x5d));
    this->AddChild(addr_list);

    for (int32_t i = 0; i < this->enum_addresses_num; i++) {
        addr_list->AddItem(this->enum_addresses[i].name);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);
    rc3.bottom = rc3.top + 0x18;

    VisButton* connect_btn = new VisButton(0xa, rc3, txt_dialogs.GetLine(0x78), g_font1, nullptr, 0x47d, 0, txt_dialogs.GetLine(0x7e));
    this->AddChild(connect_btn);
    if (g_IsCdPresent == 0 || this->enum_addresses_num == 0) {
        connect_btn->ChangeFlags(1, false);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);

    VisButton* host_btn = new VisButton(0xb, rc3, txt_dialogs.GetLine(0x79), g_font1, nullptr, 0x47e, 0, txt_dialogs.GetLine(0x7f));
    this->AddChild(host_btn);
    if (g_IsCdPresent == 0 || this->enum_addresses_num == 0) {
        host_btn->ChangeFlags(1, false);
    }

    rc3.OffsetRect(0, rc3.Height() + 4);

    VisButton* back_btn = new VisButton(0xc, rc3, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(back_btn);

    const char* str = "";
    if (this->phones->phones.GetSize() != 0) {
        str = this->phones->phones[0];
        edit->ReadData(str);
    }
    this->UpdateConnectButton(str);
}


// 447ea8
VisNetPhoneBook::VisNetPhoneBook(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, PhoneBook* _book)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->phones = _book;
    this->enum_addresses = nullptr;
}

// 44FB30
VisListBoxPhoneBook::VisListBoxPhoneBook(int32_t _id, const RECT& r, CGameFont* _font, uint16_t* _clr1, uint16_t* _clr2, int32_t _entry_height, const char* hint, CStringArray* _bound_entries)
    : VisListBox(_id, r, _font, _clr1, _clr2, _entry_height, hint) {
    this->bound_entries = _bound_entries;
}

// 44fca0 (deleting dtor ??_G at 44FC70; the complete dtor only calls the base dtor)
VisListBoxPhoneBook::~VisListBoxPhoneBook() = default;

// 44FB80
int32_t VisListBoxPhoneBook::OnKeyDown(uint32_t wparam) {
    if (wparam == 0x2E) {
        if (this->selected_index >= 0) {
            this->RemoveItem(this->selected_index);
            this->bound_entries->RemoveAt(this->selected_index, 1);
            if (this->selected_index == this->entries.GetSize()) {
                this->selected_index--;
            }
            VisScrollBar* scrollbar = (VisScrollBar*)this->parent->FindChild(this->scrollbox_id);
            if (scrollbar != nullptr && this->selected_index != 0) {
                scrollbar->SetPos(this->selected_index, this->entries.GetSize());
            }
            this->VMethod9();
        }
        return 1;
    }
    return VisListBox::OnKeyDown(wparam);
}


// 448708
void VisNetPhoneBook::UpdateConnectButton(const char* str) {
    VisButton* connect_btn = (VisButton*)this->FindChild(10);
    if (str != nullptr && this->enum_addresses_num != 0) {
        connect_btn->ChangeFlags(1, true);
    } else {
        connect_btn->ChangeFlags(1, false);
        connect_btn->SetDowned(false);
    }
    connect_btn->VMethod9();
}


// 448cd5
VisNetSerialSettings::VisNetSerialSettings(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, ComSettings* _com)
: VisWindow(_id, l, t, r, b, nullptr) {
    this->pcom = _com;
}

// 449903
void VisNetSerialSettings::SaveComSettings() {
    char buffer[64];

    CVisualObject* obj = this->FindChild(1);
    obj->WriteData(buffer);
    this->pcom->index = atoi(buffer + 3);

    obj = this->FindChild(2);
    obj->WriteData(buffer);
    this->pcom->speed = atoi(buffer);

    obj = this->FindChild(3);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x62)) == 0) {
        this->pcom->stop_bits = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x63)) == 0) {
        this->pcom->stop_bits = 1;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x64)) == 0) {
        this->pcom->stop_bits = 2;
    }

    obj = this->FindChild(4);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x5e)) == 0) {
        this->pcom->parity = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x5f)) == 0) {
        this->pcom->parity = 1;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x60)) == 0) {
        this->pcom->parity = 2;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x61)) == 0) {
        this->pcom->parity = 3;
    }

    obj = this->FindChild(5);
    obj->WriteData(buffer);
    if (strcmp(buffer, txt_dialogs.GetLine(0x69)) == 0) {
        this->pcom->flow_control = 4;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x65)) == 0) {
        this->pcom->flow_control = 2;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x66)) == 0) {
        this->pcom->flow_control = 3;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x67)) == 0) {
        this->pcom->flow_control = 0;
    } else if (strcmp(buffer, txt_dialogs.GetLine(0x68)) == 0) {
        this->pcom->flow_control = 1;
    }
}


// 448d15
void VisNetSerialSettings::VMethod26() {
    VisLabel* title = new VisLabel(-1, 0x28, 0x14, this->rect.Width() - 0x28, 0x2c,
        txt_dialogs.GetLine(0x94), g_font1, p_clrsh_Black, 2);
    this->AddChild(title);

    CRect rc(0x28, 0x38, 0xd0, 0x3c);

    VisLabel* com_label = new VisLabel(0x1e, rc, txt_dialogs.GetLine(0x6a), g_font1, p_clrsh_Black, 0);
    this->AddChild(com_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* speed_label = new VisLabel(0x1f, rc, txt_dialogs.GetLine(0x6b), g_font1, p_clrsh_Black, 0);
    this->AddChild(speed_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* parity_label = new VisLabel(0x20, rc, txt_dialogs.GetLine(0x6c), g_font1, p_clrsh_Black, 0);
    this->AddChild(parity_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* flow_label = new VisLabel(0x21, rc, txt_dialogs.GetLine(0x6d), g_font1, p_clrsh_Black, 0);
    this->AddChild(flow_label);
    rc.OffsetRect(0, 0x28);

    VisLabel* stop_label = new VisLabel(0x22, rc, txt_dialogs.GetLine(0x6e), g_font1, p_clrsh_Black, 0);
    this->AddChild(stop_label);

    VisComboBox* com_combo = new VisComboBox(1, CRect(0xe4, 0x38, this->rect.Width() - 0x2c, 0xb0),
        txt_dialogs.GetLine(0x6f));
    this->AddChild(com_combo);
    com_combo->AddItem("COM1");
    com_combo->AddItem("COM2");
    com_combo->AddItem("COM3");
    com_combo->AddItem("COM4");
    com_combo->SelectItem(this->pcom->index - 1);

    CRect rc2(com_combo->GetRect());
    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0xc0;

    VisComboBox* speed_combo = new VisComboBox(2, rc2, txt_dialogs.GetLine(0x70));
    this->AddChild(speed_combo);
    speed_combo->AddItem("14400");
    speed_combo->AddItem("19200");
    speed_combo->AddItem("38400");
    speed_combo->AddItem("56000");
    speed_combo->AddItem("57600");
    speed_combo->AddItem("115200");
    speed_combo->AddItem("128000");
    speed_combo->AddItem("256000");
    switch (this->pcom->speed) {
    case 14400:
        speed_combo->SelectItem(0);
        break;
    case 19200:
        speed_combo->SelectItem(1);
        break;
    case 38400:
        speed_combo->SelectItem(2);
        break;
    case 56000:
        speed_combo->SelectItem(3);
        break;
    case 57600:
        speed_combo->SelectItem(4);
        break;
    case 115200:
        speed_combo->SelectItem(5);
        break;
    case 128000:
        speed_combo->SelectItem(6);
        break;
    case 256000:
        speed_combo->SelectItem(7);
        break;
    default:
        speed_combo->SelectItem(0);
        break;
    }

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x78;

    VisComboBox* parity_combo = new VisComboBox(4, rc2, txt_dialogs.GetLine(0x71));
    this->AddChild(parity_combo);
    parity_combo->AddItem(txt_dialogs.GetLine(0x5e));
    parity_combo->AddItem(txt_dialogs.GetLine(0x5f));
    parity_combo->AddItem(txt_dialogs.GetLine(0x60));
    parity_combo->AddItem(txt_dialogs.GetLine(0x61));
    parity_combo->SelectItem(this->pcom->parity);

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x78;

    VisComboBox* flow_combo = new VisComboBox(5, rc2, txt_dialogs.GetLine(0x72));
    this->AddChild(flow_combo);
    flow_combo->AddItem(txt_dialogs.GetLine(0x67));
    flow_combo->AddItem(txt_dialogs.GetLine(0x68));
    flow_combo->AddItem(txt_dialogs.GetLine(0x65));
    flow_combo->AddItem(txt_dialogs.GetLine(0x66));
    flow_combo->AddItem(txt_dialogs.GetLine(0x69));
    flow_combo->SelectItem(this->pcom->flow_control);

    rc2.OffsetRect(0, 0x28);
    rc2.bottom = rc2.top + 0x48;

    VisComboBox* stop_combo = new VisComboBox(3, rc2, txt_dialogs.GetLine(0x73));
    this->AddChild(stop_combo);
    stop_combo->AddItem(txt_dialogs.GetLine(0x62));
    stop_combo->AddItem(txt_dialogs.GetLine(0x63));
    stop_combo->AddItem(txt_dialogs.GetLine(0x64));
    stop_combo->SelectItem(this->pcom->stop_bits);

    CRect btn_rc(60, 272, 321, 296);

    VisButton* join_btn = new VisButton(10, btn_rc, txt_dialogs.GetLine(0x7a), g_font1, nullptr, 0x480, 0,
        txt_dialogs.GetLine(0x80));
    this->AddChild(join_btn);
    btn_rc.OffsetRect(0, 0x1e);

    VisButton* host_btn = new VisButton(0xb, btn_rc, txt_dialogs.GetLine(0x7b), g_font1, nullptr, 0x481, 0,
        txt_dialogs.GetLine(0x81));
    this->AddChild(host_btn);
    if (g_IsCdPresent == 0) {
        host_btn->ChangeFlags(1, false);
    }
    btn_rc.OffsetRect(0, 0x1e);

    this->FindChild(0xb)->SetUpObj(this->FindChild(10));

    VisButton* reconnect_btn = new VisButton(0xc, btn_rc, txt_dialogs.GetLine(0x82), g_font1, nullptr, 0x483, 0,
        txt_dialogs.GetLine(0x83));
    this->AddChild(reconnect_btn);
    btn_rc.OffsetRect(0, 0x1e);

    this->FindChild(0xc)->SetUpObj(this->FindChild(0xb));

    VisButton* back_btn = new VisButton(0xd, btn_rc, txt_dialogs.GetLine(1), g_font1, nullptr, 0x446, 0, "");
    this->AddChild(back_btn);

    this->FindChild(0xd)->SetUpObj(this->FindChild(0xc));
}


// 449c98
int32_t VisNetSerialSettings::MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) {
    switch (msg) {
    case VIS_MSG_CLOSE_SCREEN: {
        int32_t result = VisScreen::MsgProc(msg, wparam, lparam);
        AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
        return result;
    }
    case VIS_MSG_SERIAL_CONNECT: {
        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.PrepareForConnect("", &addr) != 0) {
            AfxGetMainWnd()->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_SERIAL_HOST: {
        MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

        this->SaveComSettings();

        CLlAddress addr;
        addr.com = *this->pcom;
        if (g_CLlDriver.StartServer(2, main_wnd->m_GameSession.character_name, &addr) != 0) {
            VisScreen::MsgProc(VIS_MSG_HIDE_SCREEN, 0, 0);
            main_wnd->PostMessage(MAIN_MSG_ENTER_CLIENT_MODE2, 0, 0);
        }
        return 1;
    }
    case VIS_MSG_SERIAL_CLEAR: {
        this->DestroyAllChilds();
        this->cursor_over_obj_last = nullptr;
        this->cursor_over_obj = nullptr;
        this->last_focus_obj = nullptr;
        this->focus_obj = nullptr;
        this->VMethod26();

        this->FocusTo(this->FindChild(0xc), false);
        this->VMethod9();
        return 1;
    }
    default:
        return VisScreen::MsgProc(msg, wparam, lparam);
    }
}

// 449F06
VisNetTcpIp::VisNetTcpIp(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CString* _ip)
    : VisMessageBox(_id, l, t, r, b, (const void*)_ip, txt_dialogs.GetLine(0x74), 1, txt_dialogs.GetLine(0x95)) {
    this->ip = _ip;
}

// 44FD10 (this address is the deleting dtor ??_G; the real dtor 44FD40 only calls the base dtor)
VisNetTcpIp::~VisNetTcpIp() = default;

// 449F67
void VisNetTcpIp::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        char buffer[256];
        this->FindChild(2)->WriteData(buffer);
        *this->ip = buffer;
        main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
        main_wnd->net_session.connect_address = buffer;
    } else {
        g_CLlDriver.Close();
        main_wnd->PostMessage(MAIN_MSG_SHOW_CONNECT_DLG, 0, 0);
    }
}

// 44A014
CVisualObject* VisNetTcpIp::VMethod30(const void* data, const RECT& r) {
    VisTextBox* textbox = new VisTextBox(2, r.left, r.top, r.right, r.top + 0x18, g_font1, clrsh_TechBlack, txt_dialogs.GetLine(0x75));
    this->AddChild(textbox);
    textbox->ReadData(*(CString*)data);
    return textbox;
}

// 44604C
CVisualObject* VisConnectionDlg::VMethod30(const void* data, const RECT& r) {
    RECT& rect = const_cast<RECT&>(r);
    rect.right -= 0x18;

    VisListBox* list = new VisListBox(2, r, g_font1, p_clrsh_Black, p_clrsh_ShockingBlack, 10, txt_dialogs.GetLine(0x51));
    this->AddChild(list);

    VisScrollBar* scrollbar = new VisScrollBar(10, list->GetRect().right, list->GetRect().top, list->GetRect().right + 0x18, list->GetRect().bottom, nullptr);
    this->AddChild(scrollbar);

    CStringArray* names = new CStringArray();

    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    int32_t no_serial = (main_wnd->sessionMode == SESSION_SERVER) ? 1 : 0;

    int32_t count = 0;
    g_CLlDriver.EnumConnections(&this->protocols, &count);
    for (int32_t i = 0; i < count; i++) {
        if (this->protocols[i].typ == 4 || (no_serial != 0 && (this->protocols[i].typ == 1 || this->protocols[i].typ == 0))) {
            memmove(this->protocols + i, this->protocols + i + 1, (count - i - 1) * sizeof(CLlConn));
            count--;
            i--;
        } else {
            char enc[256];
            const char* name = this->protocols[i].name;
            uint32_t j = 0;
            for (; j < strlen(name); j++) {
                enc[j] = EncodeChar(name[j]);
            }
            enc[j] = '\0';
            names->Add(enc);
        }
    }
    list->ReadData(&names);

    if ((uint32_t)*this->pSelected >= (uint32_t)(list->GetItemCount() - 1)) {
        *this->pSelected = list->GetItemCount() - 1;
    }
    list->SetSelectedIndex(*this->pSelected);
    list->SetCaptionLabel((VisLabel*)this->FindChild(-1));

    delete names;
    return list;
}

// 445EBD
void VisConnectionDlg::VMethod31(int32_t code) {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (code == VIS_MSG_HIDE_SCREEN) {
        VisListBox* list = (VisListBox*)this->FindChild(2);
        *this->pSelected = list->GetSelectedIndex();
        int32_t typ = this->protocols[*this->pSelected].typ;
        if (g_CLlDriver.ResetProvider(typ) == 0) {
            VisMessageBoxWithList* msg = new VisMessageBoxWithList(1, 100, 100, 0x21c, 0x17c, TxtFile::AllLines[0x98], nullptr, 0);
            main_wnd->ModalScreen(msg);
            main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
        } else {
            if (typ == 3) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_TCP_SETTINGS, 0, 0);
            } else if (typ == 0) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_SERIAL_SETTINGS, 0, 0);
            } else if (typ == 1) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_PHONE_BOOK, 0, 0);
            } else if (typ == 2) {
                main_wnd->PostMessage(MAIN_MSG_SHOW_NET_SESSIONS, 0, 0);
            }
        }
    }
    if (code == VIS_MSG_CLOSE_SCREEN) {
        main_wnd->PostMessage(MAIN_MSG_MAIN_MENU, 0, 0);
    }
}

// 445E52
VisConnectionDlg::VisConnectionDlg(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, int32_t* _payload)
    : VisMessageBox(_id, l, t, r, b, _payload, txt_dialogs.GetLine(0x50), 1, txt_dialogs.GetLine(0x92)) {
    this->protocols = nullptr;
    this->pSelected = _payload;
}

// 44FA40
VisConnectionDlg::~VisConnectionDlg() {
}




