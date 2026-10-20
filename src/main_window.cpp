#include "main_window.h"
#include <cstdio>
#include <timeapi.h>
#include "game_app.h"
#include "net.h"
#include "players_list.h"
#include "server.h"
#include <stdio.h>

#include "mouse.h"
#include "ingame.h"
#include "player_file.h"
#include "packet.h"
#include "player.h"
#include "unit.h"
#include "buildings_list.h"
#include "resource.h"
#include "map_stuff.h"
#include "gfx.h"
#include "file.h"
#include "inventory.h"
#include "group.h"
#include "world.h"
#include <wininet.h>

uint32_t g_RemoteTimestamp = 0;
int32_t DAT_00660f88 = 0;
int32_t g_LButtonDown; //660f4c
uint32_t g_using_vxd = 1; // 62fa9c
CDWordArray DAT_006658d8;

CRect vis_scr_rect; //65fb88



//495033
CListBox2::CListBox2() = default;

//495052
CListBox2::~CListBox2() = default;

const AFX_MSGMAP* CListBox2::GetMessageMap() const {
    //49506e
    return &CListBox2::messageMap;
}

//60bfa8
AFX_DATADEF const AFX_MSGMAP CListBox2::messageMap =
{ &CListBox::GetThisMessageMap, &CListBox2::_messageEntries[0] };

const AFX_MSGMAP_ENTRY CListBox2::_messageEntries[] =
{
    {WM_KEYDOWN, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnKeyDown},
    {WM_LBUTTONDBLCLK, 0, 0, 0, AfxSig_vwp, (AFX_PMSG)&OnLButtonDblClk},
    {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
};

void CListBox2::OnKeyDown(UINT, UINT, UINT) {
	this->Default();
}

void CListBox2::OnLButtonDblClk(UINT, CPoint) {
	this->Default();
}












CEdit2::CEdit2() = default;
CEdit2::~CEdit2() = default;

const AFX_MSGMAP* CEdit2::GetMessageMap() const {
	//494efb
	return &CEdit2::messageMap;
}

//60bf70
AFX_DATADEF const AFX_MSGMAP CEdit2::messageMap =
{ &CEdit::GetThisMessageMap, &CEdit2::_messageEntries[0] };

const AFX_MSGMAP_ENTRY CEdit2::_messageEntries[] =
{
    {WM_KEYDOWN, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnKeyDown},
	{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0}
};

void CEdit2::OnKeyDown(UINT nChar, UINT nRepcnt, UINT nFlags) {
	this->Default();

	if (nChar == VK_RETURN) {
		CString txt;
		CWnd::GetWindowText(txt);

		char buf[1024];
		CharToOemA(txt, buf);

		g_NetStru1_main.AnnouncePlayerName(buf, nullptr);

		LogMessage(buf);

		SetSel(0xffff0000);
	}
}






Fame2::Fame2() {
    //4abdab
    this->doc_id = -1;
    this->is_text = 1;
    this->picture = nullptr;
    this->str.Empty();
    this->field_x34 = 1;
    this->top_line = 0;
}

void Fame2::Clear() {
    //4ac27f
    if (this->picture) {
        delete this->picture;
    }

    this->picture = nullptr;
    this->str.Empty();
    this->str_arr.RemoveAll();
}

void Fame2::SetDocument(int32_t arg1, uint32_t arg2) {
    //4abf0f
    this->Clear();
    this->doc_id = arg1;
    this->is_text = arg2;
}


int32_t Fame2::GetDocId() {
    //4ad880
    return this->doc_id;
}

uint32_t Fame2::GetIsText() {
    //4ad890
    return this->is_text;
}

void Fame2::operator=(const Fame2& b) {
    //4abd30
    this->doc_id = b.doc_id;
    this->is_text = b.is_text;
    this->picture = b.picture;
    this->str = b.str;
    this->rect = b.rect;
    this->field_x34 = b.field_x34;
    this->top_line = b.top_line;
}

bool Fame2::ScrollDown() { //4abf35
    int32_t next = this->top_line + 0x15;
    if (next < this->str_arr.GetSize()) {
        this->top_line = next;
        return true;
    }
    return false;
}

int32_t Fame2::ScrollUp() { //4abf72
    if (this->top_line == 0) {
        return 0;
    }
    this->top_line -= 0x15;
    if ((int32_t)this->top_line < 0) {
        this->top_line = 0;
    }
    return 1;
}

void Fame2::Draw(int32_t x, int32_t y) { //4abfb1
    if (this->is_text == 0) {
        if (this->picture) {
            this->picture->Draw(x + this->rect.left, y + this->rect.top, 0, 0, 0);
        }
        return;
    }

    uint16_t* clr = palette_brown_derby->GetPalette(0);
    CRect r = this->rect + CPoint(x, y);
    g_font4->DrawTextLines(r, this->top_line, this->top_line + 0x15, this->str_arr, clr, 0);
}

void Fame2::LoadDocument() { //4ac0af
    this->Clear();
    this->rect = CRect(CPoint(0x5c, 0x48), CSize(0x1c8, 0x158));

    char buffer[1024];
    if (this->is_text == 0) {
        sprintf(buffer, "graphics\\interface\\Docs\\%d.bmp", this->doc_id);
        this->picture = new CBmp64(buffer);
    } else {
        sprintf(buffer, "main\\text\\Docs\\%d.txt", this->doc_id);
        ReadFileToString(buffer, &this->str);
        this->str_arr.Copy(g_font4->StringArrayForRect(this->rect, this->str));
    }
    this->top_line = 0;
}








CFameHall::CFameHall() {
    //4ac2e3
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
    this->m_nMaxEntries = 10;
}

CFameHall::~CFameHall() {
    //4ac352
}

void CFameHall::ZeroFactors() {
    //4ac3af
    this->m_nTotalTime = 0;
    this->m_nDifficultyWeight = 0;
}

void CFameHall::SetDifficulty(uint32_t arg) {
    //497270
    this->difficulty = arg;
}

uint32_t CFameHall::AddTextDocument(int32_t arg) {
    //4ac566
    for (uint32_t i = 0; i < this->m_Documents.GetSize(); i++) {
        Fame2& fame = this->m_Documents[i];
        if (fame.GetDocId() == arg && fame.GetIsText() != 0) {
            return 0;
        }
    }

    Fame2 f;
    f.SetDocument(arg, 1);
    this->m_Documents.Add(f);

    return 1;
}


void CFameHall::SeedDefaultEntries() {
    //4acafa
    Fame1 fame;
    int32_t tval = 70000;
    for (int i = 0; i < 9; i++) {
        fame.score = tval + GetRandS16(5000);
        fame.str = TxtFile::AllLines[i + 0x107];
        this->AddEntry(fame);
        tval -= 7000;
    }

    fame.score = 0;
    fame.str = TxtFile::AllLines[0x110];
    this->AddEntry(fame);
}



// 4ac839
void CFameHall::SaveEntries(CFile* file) {
    int32_t count = this->m_Entries.GetSize();
    file->Write(&count, 4);

    for (int32_t i = 0; i < this->m_Entries.GetSize(); i++) {
        Fame1& fame = this->m_Entries[i];

        int32_t len = fame.str.GetLength() + 1;
        file->Write(&len, 4);
        file->Write(fame.str, len);
        file->Write(&fame.score, 4);
        file->Write(&fame.field_x8, 4);
        file->Write(&fame.field_xc, 4);
    }
}

void CFameHall::LoadEntries(CFile* file) {
    //4ac945
    uint32_t num;
    file->Read(&num, 4);
    
    if (num == 0) {
        this->m_Entries.RemoveAll();
    } else {
        this->m_Entries.SetSize(num);
        for (uint32_t i = 0; i < num; i++) {
            uint32_t t;
            file->Read(&t, 4);

            char buf[1024];
            file->Read(buf, t);

            Fame1& fame = this->m_Entries[i];

            fame.str = buf;

            file->Read(&fame.score, 4);
            file->Read(&fame.field_x8, 4);
            file->Read(&fame.field_xc, 4);
        }
    }
}

void CFameHall::SaveDocuments(CFile* file) { //  4ac706
    int32_t num = this->m_Documents.GetSize();
    file->Write(&num, 4);

    for (uint32_t i = 0; i < num; i++) {
        Fame2& fm = this->m_Documents[i];
        
        //inline 4ac04b
        file->Write(&fm.doc_id, 4);
        file->Write(&fm.is_text, 4);
    }
    file->Write(&this->m_nTotalTime, 4);
    file->Write(&this->m_nDifficultyWeight, 4);
}

void CFameHall::LoadDocuments(CFile* file) { // 4ac7a2
    int32_t cnt;
    file->Read(&cnt, 4);
    this->m_Documents.SetSize(cnt);

    for (int i = 0; i < cnt; i++) {
        Fame2& fame = this->m_Documents[i];
        file->Read(&fame.doc_id, 4);
        file->Read(&fame.is_text, 4);
    }

    file->Read(&this->m_nTotalTime, 4);
    file->Read(&this->m_nDifficultyWeight, 4);
}


void CFameHall::AddEntry(const Fame1& fame) {
    //4ac3ce
    if (this->m_Entries.GetSize() == 0) {
        this->m_Entries.Add(fame);
    } else {
        int32_t i = 0;
        for (i = 0; i < this->m_Entries.GetSize(); i++) {
            Fame1& f = this->m_Entries[i];
            if (f.score <= fame.score) {
                this->m_Entries.InsertAt(i, fame);
                break;
            }
        }
        if (i == this->m_Entries.GetSize()) {
            this->m_Entries.Add(fame);
        }
    }

    if (this->m_Entries.GetSize() > this->m_nMaxEntries) {
        this->m_Entries.SetSize(this->m_nMaxEntries);
    }
}

void CFameHall::AddDifficultyPoint() {
    //420050
    this->m_nDifficultyWeight++;
}

void CFameHall::SubmitScore() { //4ac498
    Fame1 fm;
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();
    if (unit) {
        fm.str = unit->str1;
        if (this->m_nTotalTime == 0) {
            fm.score = unit->exp_summary / 500000.0 * this->m_nDifficultyWeight;
        } else {
            fm.score = unit->exp_summary / (this->m_nTotalTime * 10.0) * this->m_nDifficultyWeight;
        }

        this->AddEntry(fm);
    }
}


// 4951bd
ServerControlDialog::ServerControlDialog(CWnd* parent_window)
    : CDialog(0x6b, parent_window) {
}

ServerControlDialog::~ServerControlDialog() {
}


//60bff8
AFX_DATADEF const AFX_MSGMAP ServerControlDialog::messageMap =
{ &CDialog::GetThisMessageMap, &ServerControlDialog::_messageEntries[0] };

const AFX_MSGMAP_ENTRY ServerControlDialog::_messageEntries[] =
{
    { WM_DESTROY, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&OnDestroy },
    { WM_COMMAND, CN_COMMAND, 0x3f5, 0x3f5, AfxSigCmd_v, (AFX_PMSG)&OnCommand3f5},
    { WM_COMMAND, CN_COMMAND, 0x3f6, 0x3f6, AfxSigCmd_v, (AFX_PMSG)&OnCommand3f6},
    { WM_COMMAND, CN_COMMAND, 0x3f4, 0x3f4, AfxSigCmd_v, (AFX_PMSG)&OnCommandUnknown},
    { WM_CLOSE, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&OnClose },
    { WM_COMMAND, CN_COMMAND, 0x3ff, 0x3ff, AfxSigCmd_v, (AFX_PMSG)&OnCommand3ff},
    { WM_COMMAND, CN_COMMAND, 0x3f7, 0x3f7, AfxSigCmd_v, (AFX_PMSG)&OnCommand3f7},
    { WM_COMMAND, CN_COMMAND, 0x3fb, 0x3fb, AfxSigCmd_v, (AFX_PMSG)&OnCommand3fb},
    { WM_COMMAND, CN_COMMAND, 0x3fe, 0x3fe, AfxSigCmd_v, (AFX_PMSG)&OnCommand3fe},
    {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
};

// 4952fa
void ServerControlDialog::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, 0x3f9, this->edit_5c);
    DDX_Control(pDX, 0x400, this->static_98);
    DDX_Control(pDX, 0x3f6, this->button_d4);
    DDX_Control(pDX, 0x3f5, this->button_110);
    DDX_Control(pDX, 0x3f4, this->button_14c);
    DDX_Control(pDX, 0x3f7, this->button_188);
    DDX_Control(pDX, 0x3ff, this->button_1c4);
    DDX_Control(pDX, 0x3fb, this->button_200);
    DDX_Control(pDX, 0x3fe, this->button_23c);
    DDX_Text(pDX, 0x400, this->str_278);
    DDX_Text(pDX, 0x3f9, this->str_27c);
    DDV_MaxChars(pDX, this->str_27c, 2);
}

//495429
const AFX_MSGMAP* ServerControlDialog::GetMessageMap() const {
    return &ServerControlDialog::messageMap;
}

// 495439
void ServerControlDialog::OnDestroy() {
    CWnd::OnDestroy();
}

// 49544C
void ServerControlDialog::OnCommand3f5() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->SetSpeed(main_wnd->game_speed - 1)) {
        g_ServerConfig.game_speed = main_wnd->game_speed;
        g_NetStru1_main.SendGameInfo(g_ServerConfig.game_speed, nullptr);
    }
}

// 49549B
void ServerControlDialog::OnCommand3f6() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->SetSpeed(main_wnd->game_speed + 1)) {
        g_ServerConfig.game_speed = main_wnd->game_speed;
        g_NetStru1_main.SendGameInfo(g_ServerConfig.game_speed, nullptr);
    }
}

// 4956d5
void ServerControlDialog::OnClose() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    main_wnd->server_ctrl_shown = 0;
    this->UpdateData(1);

    int32_t minutes = atoi(this->str_27c);
    if (minutes > 0 && minutes < 100) {
        g_ServerConfig.shutdown_delay = minutes;
    }

    this->Default();
    this->DestroyWindow();
}

// 495774
void ServerControlDialog::OnCommand3ff() {
    if (g_Server != nullptr && !g_HatLLDriver.IsListen()) {
        g_Server->field51_0x1d8 = 1;
    }
    this->UpdateControlStates();
}

// 4957ad
void ServerControlDialog::OnCommand3f7() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    g_Server->KickAllPlayers();
    main_wnd->shutdown_pending = 1;
    this->UpdateControlStates();
}

// 4957e2
void ServerControlDialog::OnCommand3fb() {
    this->UpdateData(1);

    int32_t minutes = atoi(this->str_27c);
    if (minutes > 0 && minutes < 100) {
        g_ServerConfig.shutdown_delay = minutes;
        g_ShutdownIn = minutes * 60000;
    }
    this->UpdateControlStates();
}

// 49583e
void ServerControlDialog::OnCommand3fe() {
    g_ShutdownIn = 0x7fffffff;
    this->UpdateControlStates();
}

// 49585B
void ServerControlDialog::UpdateControlStates() {
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();

    if (g_Server == nullptr) {
        this->button_188.EnableWindow(0);
        this->button_200.EnableWindow(0);
        this->edit_5c.EnableWindow(0);
        this->button_23c.EnableWindow(0);
        this->button_110.EnableWindow(0);
        this->button_d4.EnableWindow(0);
        this->button_14c.EnableWindow(0);
        this->button_1c4.EnableWindow(0);
        return;
    }

    int32_t enable = 0;
    if (!g_HatLLDriver.IsListen() && g_Server->field51_0x1d8 != 1) {
        enable = 1;
    }
    this->button_1c4.EnableWindow(enable);

    this->button_14c.EnableWindow(main_wnd->list_box2.GetCurSel() != -1);
    this->button_110.EnableWindow(g_Server->field18_0x94);
    this->button_d4.EnableWindow(g_Server->field18_0x94);
    this->button_188.EnableWindow(main_wnd->shutdown_pending == 0);
    this->edit_5c.EnableWindow(g_ShutdownIn == 0x7FFFFFFF);
    this->button_23c.EnableWindow(g_ShutdownIn < 0x7FFFFFFF);

    this->UpdateData(1);

    int32_t minutes = atoi(this->str_27c);
    this->button_200.EnableWindow(g_ShutdownIn == 0x7FFFFFFF && minutes > 0 && minutes < 100);

    if (g_ShutdownIn == 0x7FFFFFFF) {
        this->str_278 = "";
    } else {
        this->str_278.Format("T-%d:%02d", g_ShutdownIn / 60000, (g_ShutdownIn / 1000) % 60);
    }

    this->UpdateData(0);
}



const AFX_MSGMAP* MainWindow::GetMessageMap() const {
    return &MainWindow::messageMap;
}

AFX_DATADEF const AFX_MSGMAP MainWindow::messageMap =
{ &CFrameWnd::GetThisMessageMap, &MainWindow::_messageEntries[0] };

const AFX_MSGMAP_ENTRY MainWindow::_messageEntries[] =
{
    {WM_DESTROY, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&OnDestroy},
	{WM_CREATE, 0, 0, 0, AfxSig_is, (AFX_PMSG)&OnCreate},
    {WM_PAINT, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&OnPaint},
    {WM_SETCURSOR, 0, 0, 0, AfxSig_bWww, (AFX_PMSG)&OnSetCursor},
    {WM_SETFOCUS, 0, 0, 0, AfxSig_vW, (AFX_PMSG)&OnSetFocus},
    {WM_KEYDOWN, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnKeyDown},
    {WM_KEYDOWN, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnKeyDown},
    {WM_SYSKEYDOWN, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnSysKeyDown},
    {WM_KEYUP, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnKeyUp},
    {WM_SYSKEYUP, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnSysKeyUp},
    {WM_CHAR, 0, 0, 0, AfxSig_vwww, (AFX_PMSG)&OnChar},
    {WM_KILLFOCUS, 0, 0, 0, AfxSig_vW, (AFX_PMSG)&OnKillFocus},
    {WM_CLOSE, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&OnClose},
    {WM_SIZE, 0, 0, 0, AfxSig_vwii, (AFX_PMSG)&OnSize},
    {WM_COMMAND, CN_COMMAND, 0x9c4e, 0x9c4e, AfxSigCmd_v, (AFX_PMSG)&BeginShutdown},
    {WM_COMMAND, CN_COMMAND, 0x9c4f, 0x9c4f, AfxSigCmd_v, (AFX_PMSG)&ShowServerControlDialog},
    {WM_COMMAND, CN_COMMAND, 0x9c50, 0x9c50, AfxSigCmd_v, (AFX_PMSG)&MarkHatSessionEnded},
    {WM_SIZING, 0, 0, 0, AfxSig_vwSIZING, (AFX_PMSG)&OnSizing},
    {WM_NCMOUSEMOVE, 0, 0, 0, AfxSig_vwp, (AFX_PMSG)&OnNcMouseMove},
    {WM_NCLBUTTONDOWN, 0, 0, 0, AfxSig_vwp, (AFX_PMSG)&OnNcLButtonDown},

    {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0}
};

// 486a2f
void MainWindow::OnSysKeyDown(UINT key, UINT repeat, UINT flags) {
    if (this->input_locked == 0) {
        if (key == 0x79) {
            this->vis_root->MsgProc(0x100, key, 0);
            this->Default();
            g_mousept.ForceDisableHint();
            return;
        }

        if (key == 0x12) {
            g_kbMenuState = 1;
        }

        if ((flags & 0x2000) != 0) {
            if (key >= 0x31 && key <= 0x39) {
                g_kbMenuState = 1;
                this->vis_root->MsgProc(0x100, key, 0);
            } else if (key >= 0x60 && key <= 0x69) {
                g_kbMenuState = 1;
                this->vis_root->MsgProc(0x100, key - 0x30, 0);
            } else if (key == 0x53) {
                SaveScreenshot();
            } else if (key > 0x41 && key < 0x5A && (this->dialogsMask & 1) != 0) {
                this->vis_map_context->sub_41AC2D(key - 0x41);
            }
        }
    }

    this->Default();
    g_mousept.ForceDisableHint();
}

// 483ea8
void MainWindow::OnDestroy() {
    if (g_IsServer != 0 && this->server_ctrl_dlg != nullptr) {
        this->server_ctrl_dlg->DestroyWindow();
        delete this->server_ctrl_dlg;
        this->server_ctrl_dlg = nullptr;
    }

    CFile file;
    if (file.Open("famehall.dat", 0x1001, nullptr) != 0) {
        this->m_FameHall.SaveEntries(&file);
        file.Close();
    }

    if ((this->dialogsMask & 1) != 0) {
        this->vis_map_context->FUN_0041d2da(1);
        this->UnloadMapAssets();
        this->DestroyServer();
    }

    UnloadUnitGfxFiles();
    UnloadAllSfx();
    SaveSettings();
    this->DestroyServer();
    this->ClearVisualTree();

    if (this->music_player != nullptr) {
        delete this->music_player;
    }

    CFrameWnd::OnDestroy();
}

// 494c91
void MainWindow::PlayDefaultMusic() {
    if (g_SoundSettings.music_allowed != 0) {
        CStringArray* plist = new CStringArray();
        plist->Add("music\\B00.wav");
        plist->Add("music\\B01.wav");
        plist->Add("music\\B02.wav");
        plist->Add("music\\B03.wav");
        plist->Add("music\\B04.wav");
        plist->Add("music\\B05.wav");
        plist->Add("music\\B06.wav");
        plist->Add("music\\B07.wav");
        plist->Add("music\\B08.wav");
        plist->Add("music\\B09.wav");
        plist->Add("music\\B10.wav");
        plist->Add("music\\B11.wav");
        plist->Add("music\\B12.wav");
        plist->Add("music\\B13.wav");
        plist->Add("music\\B14.wav");
        plist->Add("music\\B15.wav");
        plist->Add("music\\B16.wav");
        this->music_player->SetPlayList(*plist);
        delete plist;

        if (this->vis_map_context->field_0x3f6c != nullptr) {
            UpdatePositionalMusic(this->vis_map_context->field_0x3f6c->x_pos, this->vis_map_context->field_0x3f6c->y_pos);
        }

        if (g_CurrentMusicTrack >= 0) {
            this->music_player->StartPlayTrack(g_CurrentMusicTrack);
        }

        this->music_player->Play();
    }
}

// 490f6c
int32_t MainWindow::WaitHatConnection() {
    uint32_t start_time = timeGetTime();
    INT_00660f8c = 0;

    while (true) {
        MSG msg;
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (msg.message == 0x12) { // WM_QUIT
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (g_NetStru1_local.GetClientsPktNum() != 0) {
            if (this->vis_map_context->ProcessPackets(0x64) == 0) {
                VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(0x11), nullptr, 0);
                this->ModalScreen(mbox);
                this->PostMessage(0x10, 0, 0);
                return 0;
            }
            return 1;
        }

        if (g_CmdTimeout < timeGetTime() - start_time) {
            VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(0x12), nullptr, 0);
            this->ModalScreen(mbox);
            this->PostMessage(0x10, 0, 0);
            return 0;
        }

        g_mousept.Update();
    }
}

// 494a9e
void MainWindow::ClearDiplomacyEntries() {
    for (int32_t i = 0; i < this->diplomacy_entries->GetSize(); i++) {
        DiplomacyEntry* entry = (*this->diplomacy_entries)[i];
        if (entry != nullptr) {
            delete entry;
        }
    }
    this->diplomacy_entries->RemoveAll();

    for (int32_t i = 0; i < this->vis_map_context->field_0x9b8.GetSize(); i++) {
        MapPlayerData* mpd = this->vis_map_context->field_0x9b8[i];
        if (mpd != nullptr && (mpd->flags & 1) == 0) {
            uint16_t dipl_flags = this->vis_map_context->my_main_unit->diplomacy[mpd->index];
            DiplomacyEntry* entry = new DiplomacyEntry(mpd->name, dipl_flags & 1, dipl_flags & 2, dipl_flags & 0x10, dipl_flags & 4);
            this->diplomacy_entries->Add(entry);
        }
    }
}

// 49152f
void MainWindow::WaitConnectionCharacterJoin() {
    if (this->vis_map_context->FUN_0040d7f3() == 0) {
        VisMessageBoxWithList* mbox;
        if ((INT_00660f8c & 0xff) < 0xB) {
            mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, TxtFile::AllLines[(INT_00660f8c & 0xff) + 0xC0], nullptr, 0);
        } else {
            mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0);
        }
        this->ModalScreen(mbox);
        this->PostMessage(0x10, 0, 0);
    } else {
        if (this->m_GameSession.phased_in == 0) {
            VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x1ac, 0x252, TxtFile::AllLines[0xD2], nullptr, 4);
            this->ModalScreen(mbox);
            if (mbox->GetCloseCode() != 0x447) {
                this->PostMessage(0x10, 0, 0);
                return;
            }
        }
        this->PostMessage(0x457, 0, 0);
    }
}

// 48d815
void MainWindow::ShowCurrentTownDialog() {
    CStringArray* plist = new CStringArray();
    ReadFileToString("main\\text\\town.txt", &g_MissionText);
    g_Cursors[0x1a]->Use();
    this->ShowRightPanelInfo();

    int loc_id = ScenarioGetCurrentLocation()->GetId();
    if (loc_id == 1) {
        plist->Add("music\\b14.wav");
        this->vis_root->AddChild(this->vis_town);
        this->vis_town->VMethod28();
    } else if (loc_id == 2) {
        plist->Add("music\\b16.wav");
        this->vis_root->AddChild(this->vis_townkaarg);
        this->vis_townkaarg->VMethod28();
    } else if (loc_id == 3) {
        plist->Add("music\\b15.wav");
        this->vis_root->AddChild(this->vis_towndruid);
        this->vis_towndruid->VMethod28();
    }

    this->vis_root->VMethod9();
    this->field_0x460 = 0;

    if (g_SoundSettings.music_allowed != 0) {
        if (this->music_player->GetPlaylist().GetSize() == 0 || this->music_player->GetPlaylist()[0] != (*plist)[0]) {
            this->music_player->SetPlayList(*plist);
        }
        this->music_player->Play();
    }

    delete plist;
    g_Cursors[0]->Use();
}

// 491142
int32_t MainWindow::ConnectToServerAddress() {
    LockSurface2();
    FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    g_font1->DrawTextWithShadow(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2, txt_patch.GetLine(0x7e), 10, clrsh_ShockingBlack, 1);
    UnlockSurface2();
    FlushScreen();

    g_CLlDriver.Close();
    this->sessionMode = 0;

    CLlAddress addr;
    strcpy(addr.address, "0.0.0.0");
    int result = g_CLlDriver.PrepareForConnect(this->field_0x3e0.connect_address, &addr);
    if (result != 0) {
        result = g_CLlDriver.Connect(this->m_GameSession.character_name, nullptr);
    }

    if (result == 0) {
        g_CLlDriver.Close();
        VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(3), nullptr, 0);
        this->ModalScreen(mbox);
        this->PostMessage(0x10, 0, 0);
        return 0;
    }

    g_NetStru1_local.ProcessConnections();
    uint32_t start_time = timeGetTime();
    INT_00660f8c = 0;

    while (true) {
        MSG msg;
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
            if (msg.message == 0x12) { // WM_QUIT
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (g_NetStru1_local.GetClientsPktNum() != 0) {
            if (this->vis_map_context->ProcessPackets(0x64) == 0) {
                VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0);
                this->ModalScreen(mbox);
                this->PostMessage(0x10, 0, 0);
                return 0;
            }
            return 1;
        }

        if (g_CmdTimeout < timeGetTime() - start_time) {
            break;
        }
        g_mousept.Update();
    }

    VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x40, 100, 0x17c, 0x252, txt_patch.GetLine(0), nullptr, 0);
    this->ModalScreen(mbox);
    this->PostMessage(0x10, 0, 0);
    return 0;
}

// 4863f6
void MainWindow::OnKeyDown(UINT key, UINT repeat, UINT flags) {
    if (this->input_locked == 0) {
        switch (key) {
        case 0x08:
        case 0x09:
        case 0x0d:
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x2d:
        case 0x2e:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x7a:
        case 0x7b:
        case 0xc0:
            this->vis_root->MsgProc(0x100, key, 0);
            break;

        case 0x10:
            g_kbShiftState = 1;
            break;

        case 0x11:
            g_kbControlState = 1;
            break;

        case 0x13:
            if (this->sessionMode == 2 && this->dialogsMask == 1) {
                VisMessageBoxWithList* mbox = new VisMessageBoxWithList(1, 0x20, 0x30, 0x260, 0x1b0, TxtFile::AllLines[0x77], nullptr, 0);
                this->PopUpScreen(mbox);
            }
            break;

        case 0x1b:
            if (this->dialogsMask == 1) {
                this->PostMessage(0x416, 0, 0);
                if (this->sessionMode != 2) {
                    this->m_GameSession.AutosaveCharacter();
                }
            } else if (this->dialogsMask == 0) {
                if (!this->field_0x3e0.field_08.IsEmpty()) {
                    this->vis_root->MsgProc(0x100, key, 0);
                } else if (this->sessionMode == 2) {
                    this->PostMessage(0x41f, 0, 0);
                }
            } else {
                this->vis_root->MsgProc(0x100, key, 0);
            }
            break;

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
            this->vis_root->MsgProc(0x100, key - 0x30, 0);
            break;

        case 0x6b:
            if (g_kbControlState == 0) {
                if (this->sessionMode != 0 && this->dialogsMask == 1) {
                    if (this->SetSpeed(this->game_speed + 1) == 0) {
                        this->vis_map_context->msglog.AddWithCheck(TxtFile::AllLines[this->game_speed + 0x6c], clr_log_sblack, 5000);
                    } else {
                        g_NetStru1_main.AnnouncePlayerName(TxtFile::AllLines[this->game_speed + 0x6c], nullptr);
                    }
                }
            } else {
                this->unlimited_speed = 1;
            }
            break;

        case 0x6d:
            if (g_kbControlState == 0) {
                if (this->sessionMode != 0 && this->dialogsMask == 1) {
                    if (this->SetSpeed(this->game_speed - 1) == 0) {
                        this->vis_map_context->msglog.AddWithCheck(TxtFile::AllLines[this->game_speed + 0x6c], clr_log_sblack, 5000);
                    } else {
                        g_NetStru1_main.AnnouncePlayerName(TxtFile::AllLines[this->game_speed + 0x6c], nullptr);
                    }
                }
            } else {
                this->unlimited_speed = 0;
                this->field_0x438 = timeGetTime();
                this->game_tic_counter = 0;
                this->field_0x43c = 0;
            }
            break;

        case 0x70:
            this->PostMessage(0x434, 0, 0);
            break;

        case 0x71:
            if ((this->dialogsMask == 1 || this->dialogsMask == 0) && this->sessionMode == 2) {
                this->PostMessage(0x41a, 0, 0);
            }
            break;

        case 0x72:
            if (g_kbShiftState == 0 || g_kbControlState == 0) {
                if (this->sessionMode == 2) {
                    if (this->dialogsMask == 1 || this->dialogsMask == 0) {
                        this->PostMessage(0x418, 0, 0);
                    }
                } else if (this->dialogsMask == 1) {
                    this->PostMessage(0x43c, 0, 0);
                }
            }
            break;

        default:
            if (key > 0x40 && key < 0x5b) {
                this->vis_root->MsgProc(0x100, key, 0);
            }
            break;
        }
    }

    this->Default();
    g_mousept.ForceDisableHint();
}

// 495010
void MainWindow::BeginShutdown() {
    g_Server->KickAllPlayers();
    this->shutdown_pending = 1;
}

// 4863c6
void MainWindow::OnSetFocus(CWnd* pOldWnd) {
    this->has_focus = 1;
    g_kbMenuState = 0;
    CFrameWnd::OnSetFocus(pOldWnd);
}

// 48cc87
void MainWindow::ApplyCursorClip() {
    ClipCursor(&this->clip_cursor_rect);
}

// 48b01d
void MainWindow::OnKillFocus(CWnd* pNewWnd) {
    this->Default();
    this->has_focus = 0;
    g_kbMenuState = 0;
    g_kbControlState = 0;
    g_kbShiftState = 0;
}

// 4949f9
void MainWindow::OnClose() {
    this->ProcessPlayersTick();
    this->dialogsMask = 0;
    g_Cursors[0]->Use();
    FreeDistortMap();
    CFrameWnd::OnClose();
}

// 495743
void MainWindow::MarkHatSessionEnded() {
    if (g_Server != nullptr) {
        if (g_HatLLDriver.IsListen() == 0) {
            g_Server->field51_0x1d8 = 1;
        }
    }
}

// 494e8e
void MainWindow::OnSize(UINT nType, int cx, int cy) {
    CFrameWnd::OnSize(nType, cx, cy);
    if (g_IsServer != 0) {
        this->SetControlPositions();
    }
}

// 486391
BOOL MainWindow::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
    if (g_IsServer == 0) {
        SetCursor((HCURSOR)nullptr);
    } else {
        SetCursor(this->cursor_arrow);
    }
    return 1;
}

// 486c25
void MainWindow::OnSysKeyUp(UINT key, UINT repeat, UINT flags) {
    if (this->input_locked == 0 && key == 0x12) {
        g_kbMenuState = 0;
    }
    this->Default();
    g_mousept.ForceDisableHint();
}

// 48a35f
void MainWindow::OnChar(UINT ch, UINT repeat, UINT flags) {
    this->vis_root->MsgProc(0x102, ch, 0);
    this->Default();
}

// 4919c7
void MainWindow::OpenConsoleInput() {
    this->vis_map_context->field_0xe0 = 1;
    this->vis_map_context->AddChild(this->vis_1200);
    this->vis_1200->FUN_0043be9f();
    this->vis_1200->VMethod28();
    this->vis_root->VMethod9();
    this->console_active = 1;
}

// 478130
void MainWindow::FUN_00478130() {
    LoadTerrainTiles(0);
    if (DAT_00665344 != nullptr) {
        delete DAT_00665344;
        DAT_00665344 = nullptr;
    }
}

// 486bb0
void MainWindow::OnKeyUp(UINT key, UINT repeat, UINT flags) {
    if (this->input_locked == 0) {
        if (key == 0x10) {
            g_kbShiftState = 0;
        } else if (key == 0x11) {
            g_kbControlState = 0;
        } else if (key == 0x12) {
            g_kbMenuState = 0;
        }
    }
    this->Default();
    g_mousept.ForceDisableHint();
}

// 48cb3c
void MainWindow::DestroyServer() {
    if (g_Server != nullptr) {
        delete g_Server;
        g_Server = nullptr;
    }

    g_CLlDriver.Close();
    g_NetStru1_main.ProcessConnections();
    g_HatLLDriver.Close();
    NetStru1::HatConnector.ProcessConnections();

    this->field_0x438 = 0x7fffffff;
    this->last_tic_time = 0x7fffffff;
}

// 48d125
void MainWindow::ShowFameHallDocDlg() {
    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
        this->ApplyCursorClip();
    }

    g_Cursors[0x1a]->Use();
    this->vis_root->AddChild(this->vis_famedocument);
    this->vis_famedocument->VMethod28();
    this->vis_root->VMethod9();
    this->field_0x460 = 0;
    this->dialogsMask = this->dialogsMask | 0x4000;
    g_Cursors[0]->Use();
}

// 494a31
void MainWindow::ModalScreen(VisScreen* scr) {
    this->PopUpScreen(scr);

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0) != 0) {
        if (msg.message == 0x44c) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        g_mousept.Update();
    }

    this->PostMessage(0x44c, (WPARAM)scr, 0);
}

// 48cbe2
void MainWindow::UpdateMapRect() {
    CRect rect(*this->vis_map_context->GetRect());

    if (this->vis_map_context->IsBookOpen() != 0) {
        rect.BottomRight().y = this->vis_spellbook->GetRect().TopLeft().y;
    } else if (this->vis_map_context->IsBagOpen() != 0) {
        rect.BottomRight().y = this->vis_invtype1->GetRect().TopLeft().y;
    }

    ClipCursor(rect);
}

// 495b9c
void MainWindow::OnNcMouseMove(UINT nHitTest, CPoint point) {
    switch (nHitTest) {
    case 4:
    case 0xd:
    case 0x11:
        SetCursor(this->cursor_sizenwse);
        break;
    default:
        SetCursor(this->cursor_arrow);
        break;
    case 10:
    case 0xb:
        SetCursor(this->cursor_sizewe);
        break;
    case 0xc:
    case 0xf:
        SetCursor(this->cursor_sizens);
        break;
    case 0xe:
    case 0x10:
        SetCursor(this->cursor_sizenesw);
        break;
    }

    this->Default();
}

// 48cca1
void MainWindow::SetItemCursor(TokenEntry* entry, int32_t idx, const char* str, int32_t tab) {
    this->item_cursor = new CCursor(str, 0x28, 0x28, 1000000000);
    this->drag_src_slot = idx;
    this->drag_src_tab = tab;
    this->drag_entry = entry;
}

// 484a1a
void MainWindow::OnPaint() {
    if (g_ddraw != nullptr) {
        CRect rect;
        CPaintDC dc(this);
        rect = dc.m_ps.rcPaint;
        FlushRect(rect.left, rect.top, rect.right, rect.bottom);
    } else if (g_IsServer != 0) {
        this->Default();
    }
}

// 495ab2
void MainWindow::OnSizing(UINT nSide, LPRECT rect) {
    switch (nSide) {
    case 1:
    case 2:
        SetCursor(this->cursor_sizewe);
        break;
    case 3:
    case 6:
        SetCursor(this->cursor_sizens);
        break;
    case 4:
    case 8:
        SetCursor(this->cursor_sizenwse);
        break;
    case 5:
    case 7:
        SetCursor(this->cursor_sizenesw);
        break;
    }

    if (rect->right - rect->left < 0x110) {
        rect->right = rect->left + 0x110;
    }
    if (rect->bottom - rect->top < 0x1ae) {
        rect->bottom = rect->top + 0x1ae;
    }
}

// 4950c0
void MainWindow::ShowServerControlDialog() {
    if (this->server_ctrl_dlg == nullptr) {
        this->server_ctrl_dlg = new ServerControlDialog(nullptr);
    }

    if (this->server_ctrl_shown == 0) {
        this->server_ctrl_dlg->Create(MAKEINTRESOURCE(0x6b), nullptr);
        this->server_ctrl_dlg->str_27c.Format("%d", g_ServerConfig.shutdown_delay);
        this->server_ctrl_dlg->UpdateData(0);
    } else {
        this->server_ctrl_dlg->SetActiveWindow();
    }

    this->server_ctrl_shown = 1;
    this->server_ctrl_dlg->UpdateControlStates();
}

// 4919b1
void MainWindow::ProcessPlayersTick() {
    g_Server->ProcessPlayers();
}

// 485aed
void MainWindow::ClearVisualTree() {
    this->vis_right_panel->RemoveAllChilds();
    this->vis_root->RemoveAllChilds();
    this->vis_map_context->RemoveAllChilds();

    delete this->vis_root;
    this->vis_root = nullptr;

    delete this->vis_right_panel;

    this->vis_tav->RemoveChild(this->vis_charinfo);
    this->vis_shop->RemoveChild(this->vis_charinfo);
    this->vis_tavdruid->RemoveChild(this->vis_charinfo);
    this->vis_shopdruid->RemoveChild(this->vis_charinfo);
    this->vis_tavkaarg->RemoveChild(this->vis_charinfo);
    this->vis_shopkaarg->RemoveChild(this->vis_charinfo);
    this->vis_chargen->RemoveChild(this->vis_charinfo);

    delete this->vis_charinfo;
    delete this->vis_sidestatus;
    delete this->vis_ordertoolbar;
    delete this->vis_minimap;
    delete this->vis_map_context;
    delete this->vis_invtype1;
    delete this->vis_spellbook;
    delete this->vis_shop;
    delete this->vis_shopdruid;
    delete this->vis_shopkaarg;
    delete this->vis_globalmap;
    delete this->vis_mainmenu;
    delete this->vis_tav;
    delete this->vis_tavdruid;
    delete this->vis_tavkaarg;
    delete this->vis_chargen;
    delete this->vis_charsel;
    delete this->vis_startgame;
    delete this->vis_dropgold;
    delete this->vis_town;
    delete this->vis_towndruid;
    delete this->vis_townkaarg;
    delete this->vis_credits;
    delete this->vis_logownd;
    delete this->vis_1200;
    delete this->vis_famehall;
    delete this->vis_famedocument;
}

// 484022
int MainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    if (g_IsServer != 0) {
        this->status_bar.Create(this, 0x50008200, 0xe801);

        CRect rect;
        this->list_box1.Create(0x50600040, rect, this, 0x7dde);
        // The original passed a pointer to 4 uninitialized bytes at 665d20 as the initial text.
        this->static1.Create("", 0x5040000c, rect, this, 0x7ddf);
        this->static2.Create("Map not loaded", 0x50400000, rect, this, 0x7de0);
        this->edit.Create(0x50400480, rect, this, 0x7de1);
        this->list_box2.Create(0x50700040, rect, this, 0x7de2);
    }

    return 0;
}

// 495c6e
void MainWindow::OnNcLButtonDown(UINT nHitTest, CPoint point) {
    switch (nHitTest) {
    case 4:
    case 0xd:
    case 0x11:
        SetCursor(this->cursor_sizenwse);
        break;
    default:
        SetCursor(this->cursor_arrow);
        break;
    case 10:
    case 0xb:
        SetCursor(this->cursor_sizewe);
        break;
    case 0xc:
    case 0xf:
        SetCursor(this->cursor_sizens);
        break;
    case 0xe:
    case 0x10:
        SetCursor(this->cursor_sizenesw);
        break;
    }

    this->Default();
}


MainWindow::MainWindow() { //4837e1
    this->diplomacy_entries = new CArray<DiplomacyEntry*>; //avoid struct with only one this field

    this->vis_root = nullptr;

    CPoint resolution;

    if (g_IsServer == 0) {
        const char* cmdline = afxCurrentWinApp->m_lpCmdLine;

        if (strstr(cmdline, "-800")) {
            resolution.x = 800;
            resolution.y = 600;
        } else if (strstr(cmdline, "-1024")) {
            resolution.x = 1024;
            resolution.y = 768;
        } else if (strstr(cmdline, "-640")) {
            resolution.x = 640;
            resolution.y = 480;
        } else if(strstr(g_resolution, "-800")) {
            resolution.x = 800;
            resolution.y = 600;
        } else if (strstr(g_resolution, "-1024")) {
            resolution.x = 1024;
            resolution.y = 768;
        } else {
            resolution.x = 640;
            resolution.y = 480;
        }

    } else {
        resolution.x = 640;
        resolution.y = 480;
    }

    g_ScreenSize.right = resolution.x;
    g_ScreenSize.bottom = resolution.y;

    vis_scr_rect.left = (resolution.x - 640) / 2;
    vis_scr_rect.right = resolution.x - (resolution.x - 640) / 2;
    vis_scr_rect.top = (resolution.y - 480) / 2;
    vis_scr_rect.bottom = resolution.y - (resolution.y - 480) / 2;

    if (g_IsServer == 0) {
        const char* cls = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, NULL, NULL, NULL);
        CFrameWnd::Create(cls, "Allods", WS_POPUP, CRect(0, 0, resolution.x, resolution.y), NULL, 0, 0, NULL);
    } else {
        CFrameWnd::LoadFrame(0x65, WS_TILEDWINDOW, NULL, NULL);
        SetWindowPos(0, 0, 0, resolution.x, resolution.y, SWP_NOREPOSITION | SWP_NOMOVE);
        this->SetControlPositions();
        ModifyStyle(0, 0, 0);
    }

    this->server_ctrl_dlg = 0;
    this->server_ctrl_shown = 0;
    this->music_player = nullptr;
    this->drag_entry = nullptr;
    this->drag_src_slot = -1;
    this->item_cursor = nullptr;
    this->music_update_proc = nullptr;
    this->dialogsMask = 0;
    this->field_0x44c = 0;
    this->game_speed = 4;
    this->field_0x438 = 0x7fffffff;
    this->last_tic_time = 0x7fffffff;
    this->unlimited_speed = 0;
    this->serverBootstrapEnabled = 0;
    this->pending_game_mode = 0;
    this->vis_msgbox = nullptr;
    this->inside_building = 0;
    this->field_0x34c = nullptr;
    this->vis_msgbox_list = nullptr;
    this->vis_netmapsel = nullptr;
    this->vis_hatbrowser = nullptr;
    this->vis_map_context = nullptr;

    this->field_0x3e0.field_08.Empty();
    this->field_0x3e0.field_00.Empty();
    this->field_0x3e0.hat_id = 0;
    this->field_0x3e0.field_10 = 0;

    this->shutdown_pending = 0;

    this->cursor_sizewe = LoadCursorA(0, IDC_SIZEWE);
    this->cursor_sizens = LoadCursorA(0, IDC_SIZENS);
    this->cursor_sizenwse = LoadCursorA(0, IDC_SIZENWSE);
    this->cursor_sizenesw = LoadCursorA(0, IDC_SIZENESW);
    this->cursor_arrow = LoadCursorA(0, IDC_ARROW);
}


MainWindow::~MainWindow() { // 4961b0
    delete this->diplomacy_entries;
}


void MainWindow::CreateUI() { //484ab0
    this->vis_root = new CVisualObject(0, 0, 0, g_ScreenSize.right, g_ScreenSize.bottom, nullptr);
    this->vis_map_context = new BigStruct2(0, 0, g_ScreenSize.right - 160, g_ScreenSize.bottom);
    this->vis_right_panel = new VisRightPanel(4, g_ScreenSize.right - 160, 0, g_ScreenSize.right, g_ScreenSize.bottom);
    this->vis_minimap = new VisMiniMap(5, 0, 0, 160, 158);
    this->vis_ordertoolbar = new VisOrderToolbar(6, 0, 158, 160, 238);
    this->vis_charinfo = new VisCharInfo(7, 0, 238, 160, 480);
    this->vis_sidestatus = new VisSideStatus(8, 0, 480, 160, g_ScreenSize.bottom);
    this->vis_invtype1 = new VisInvType1(2, 0, this->vis_map_context->GetRect().bottom - 90, g_ScreenSize.right - 160, this->vis_map_context->GetRect().bottom);

    this->vis_spellbook = new VisSpellBook(3, 0, this->vis_map_context->GetRect().bottom - 90, g_ScreenSize.right - 160, this->vis_map_context->GetRect().bottom);
    this->vis_spellbook->MsgProc(0x411, 0, 0);

    this->vis_shop = new VisShop(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_shopdruid = new VisShopDruid(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_shopkaarg = new VisShopKaarg(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_globalmap = new VisGlobalMap(1050, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_mainmenu = new VisMainMenu(1090, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_tav = new VisTav(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_tavdruid = new VisTavDruid(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_tavkaarg = new VisTavKaarg(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_town = new VisTown(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_towndruid = new VisTownDruid(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_townkaarg = new VisTownKaarg(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_credits = new VisCredits(1110, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_logownd = new VisLogoWnd(1200, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_chargen = new VisCharGen(1110, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_charsel = new VisCharSelect(1120, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_startgame = new VisStartGame(1126, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_famehall = new VisFameHall(1200, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_famedocument = new VisFameDocument(1210, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    this->vis_1200 = new Vis1200(1200, 0, 0, g_ScreenSize.right - 160, 30);

    this->console_active = 0;

    this->vis_dropgold = new VisDropGold(1000000000, 100, g_ScreenSize.bottom - 200);

    this->vis_shop->VMethod26();
    this->vis_shopdruid->VMethod26();
    this->vis_shopkaarg->VMethod26();
    this->vis_tav->VMethod26();
    this->vis_tavdruid->VMethod26();
    this->vis_tavkaarg->VMethod26();
}

void MainWindow::LoadData() { //483d64

    g_ter_tiles.fill(nullptr);
    LoadCursors();
    LoadGraphics();
    LoadVfxData();
    LoadGameSounds();
    g_Cursors[CURSOR_DEFAULT]->Use();
    g_mousept.Paint();

    if (InitSound(AfxGetMainWnd()->m_hWnd, 16) != 0) {
        g_SoundSettings.music_allowed = 0;
    }

    this->music_player = new MusicPlayer(g_isLowMemory == 0 ? 0xac000 : 0x56000);
    this->music_player->SetVolume(g_SoundSettings.mus_pos);
}



void GameSettings::Init() { // 442386
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    this->pGameSpeed = &mwnd->game_speed;
    this->pFormationMode = &mwnd->vis_map_context->formation;
    this->pWimpyMode = &mwnd->vis_map_context->wimpy;
    this->pShowAllHitPoints = &mwnd->vis_map_context->show_hp;
    this->pShowFlyingHP = &mwnd->vis_map_context->flying_hp;
    this->pShadows = &g_Shadows;
    this->pLightning = &g_Lightning;
    this->pAnimation = &g_Animation;
    this->pMessageColors = &g_MessageColors;
}

void GameSettings::Load(HKEY& rkey) { //442420
    DWORD sz = 4;
    RegQueryValueExA(rkey, "GameSpeed", nullptr, nullptr, (LPBYTE)this->pGameSpeed, &sz);
    RegQueryValueExA(rkey, "FormationMode", nullptr, nullptr, (LPBYTE)this->pFormationMode, &sz);
    RegQueryValueExA(rkey, "WimpyMode", nullptr, nullptr, (LPBYTE)this->pWimpyMode, &sz);
    RegQueryValueExA(rkey, "ShowAllHitPoints", nullptr, nullptr, (LPBYTE)this->pShowAllHitPoints, &sz);
    RegQueryValueExA(rkey, "Smoothing", nullptr, nullptr, (LPBYTE)&g_settings.Smoothing, &sz);
    RegQueryValueExA(rkey, "ShowFlyingHP", nullptr, nullptr, (LPBYTE)this->pShowFlyingHP, &sz);
    RegQueryValueExA(rkey, "ShowTimeFlow", nullptr, nullptr, (LPBYTE)&g_settings.ShowTimeFlow, &sz);
    RegQueryValueExA(rkey, "TipsMode", nullptr, nullptr, (LPBYTE)&g_settings.TipsMode, &sz);
    RegQueryValueExA(rkey, "AutoCasting", nullptr, nullptr, (LPBYTE)&this->AutoCasting, &sz);
    RegQueryValueExA(rkey, "Acknowledgement", nullptr, nullptr, (LPBYTE)&this->Acknowledgement, &sz);
    RegQueryValueExA(rkey, "Shadows", nullptr, nullptr, (LPBYTE)this->pShadows, &sz);
    RegQueryValueExA(rkey, "Lighting", nullptr, nullptr, (LPBYTE)this->pLightning, &sz);
    RegQueryValueExA(rkey, "Animation", nullptr, nullptr, (LPBYTE)this->pAnimation, &sz);
    RegQueryValueExA(rkey, "ClanNames", nullptr, nullptr, (LPBYTE)&this->ClanNames, &sz);
    RegQueryValueExA(rkey, "MessageColors", nullptr, nullptr, (LPBYTE)this->pMessageColors, &sz);
}

// 4421be
void GameSettings::Save(HKEY& rkey) {
    DWORD cbData = 4;

    RegSetValueExA(rkey, "GameSpeed", 0, REG_DWORD, (const BYTE*)this->pGameSpeed, cbData);
    RegSetValueExA(rkey, "FormationMode", 0, REG_DWORD, (const BYTE*)this->pFormationMode, cbData);
    RegSetValueExA(rkey, "WimpyMode", 0, REG_DWORD, (const BYTE*)this->pWimpyMode, cbData);
    RegSetValueExA(rkey, "ShowAllHitPoints", 0, REG_DWORD, (const BYTE*)this->pShowAllHitPoints, cbData);
    RegSetValueExA(rkey, "Smoothing", 0, REG_DWORD, (const BYTE*)&this->Smoothing, cbData);
    RegSetValueExA(rkey, "ShowFlyingHP", 0, REG_DWORD, (const BYTE*)this->pShowFlyingHP, cbData);
    RegSetValueExA(rkey, "ShowTimeFlow", 0, REG_DWORD, (const BYTE*)&this->ShowTimeFlow, cbData);
    RegSetValueExA(rkey, "TipsMode", 0, REG_DWORD, (const BYTE*)&this->TipsMode, cbData);
    RegSetValueExA(rkey, "AutoCasting", 0, REG_DWORD, (const BYTE*)&this->AutoCasting, cbData);
    RegSetValueExA(rkey, "Acknowledgement", 0, REG_DWORD, (const BYTE*)&this->Acknowledgement, cbData);
    RegSetValueExA(rkey, "Shadows", 0, REG_DWORD, (const BYTE*)this->pShadows, cbData);
    RegSetValueExA(rkey, "Lighting", 0, REG_DWORD, (const BYTE*)this->pLightning, cbData);
    RegSetValueExA(rkey, "Animation", 0, REG_DWORD, (const BYTE*)this->pAnimation, cbData);
    RegSetValueExA(rkey, "ClanNames", 0, REG_DWORD, (const BYTE*)&this->ClanNames, cbData);
    RegSetValueExA(rkey, "MessageColors", 0, REG_DWORD, (const BYTE*)this->pMessageColors, cbData);
}

void PhoneBook::Load(HKEY& rkey) { //43ce7d
    DWORD sz = 4;
    int32_t num = 0;
    RegQueryValueExA(rkey, "phonebooksize", nullptr, nullptr, (LPBYTE)&num, &sz);

    for (int32_t i = 0; i < num; i++) {
        char name[64];
        sprintf(name, "phone%d", i);

        char data[64];
        sz = sizeof(data);
        RegQueryValueExA(rkey, name, nullptr, nullptr, (LPBYTE)data, &sz);
        this->phones.Add(data);
    }
}

// 43cf29
void PhoneBook::Save(HKEY& rkey) {
    char data[64];
    char name[64];
    int32_t count = this->phones.GetSize();

    RegSetValueExA(rkey, "phonebooksize", 0, REG_DWORD, (const BYTE*)&count, 4);

    for (int32_t i = 0; i < count; i++) {
        strcpy(data, this->phones[i]);
        sprintf(name, "phone%d", i);
        RegSetValueExA(rkey, name, 0, REG_SZ, (const BYTE*)data, strlen(data) + 1);
    }
}

void HatSettings::Load(HKEY& rkey) { //43d249
    char buf[256];

    buf[0] = '\0';
    DWORD sz = 255;
    RegQueryValueExA(rkey, "hatip", nullptr, nullptr, (LPBYTE)buf, &sz);

    this->hatip = buf;
    if (this->hatip.IsEmpty()) {
        this->hatip = txt_patch.GetLine(118);
    }

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "hatprogip", nullptr, nullptr, (LPBYTE)buf, &sz);

    this->hatprogip = buf;
    if (this->hatprogip.IsEmpty()) {
        this->hatprogip = txt_patch.GetLine(119);
    }

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "login", nullptr, nullptr, (LPBYTE)buf, &sz);

    this->login = XorRegString(buf);

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "password", nullptr, nullptr, (LPBYTE)buf, &sz);

    this->password = XorRegString(buf);

    sz = 4;
    this->ishat = 0;
    RegQueryValueExA(rkey, "ishat", nullptr, nullptr, (LPBYTE)&this->ishat, &sz);

    this->deathmatch = 0;
    RegQueryValueExA(rkey, "deathmatch", nullptr, nullptr, (LPBYTE)&this->deathmatch, &sz);

    this->store = 1;
    RegQueryValueExA(rkey, "store", nullptr, nullptr, (LPBYTE)&this->store, &sz);
}

// 43d51b
void HatSettings::Save(HKEY& rkey) {
    char buf[256];
    DWORD cbData;

    strcpy(buf, this->hatip);
    RegSetValueExA(rkey, "hatip", 0, REG_SZ, (const BYTE*)buf, strlen(buf) + 1);

    strcpy(buf, this->hatprogip);
    RegSetValueExA(rkey, "hatprogip", 0, REG_SZ, (const BYTE*)buf, strlen(buf) + 1);

    cbData = 4;
    RegSetValueExA(rkey, "ishat", 0, REG_DWORD, (const BYTE*)&this->ishat, cbData);
    RegSetValueExA(rkey, "deathmatch", 0, REG_DWORD, (const BYTE*)&this->deathmatch, cbData);
    RegSetValueExA(rkey, "store", 0, REG_DWORD, (const BYTE*)&this->store, cbData);

    if (this->store != 0) {
        strcpy(buf, XorRegString(this->login));
    } else {
        buf[0] = '\0';
    }
    RegSetValueExA(rkey, "login", 0, REG_SZ, (const BYTE*)buf, strlen(buf) + 1);

    if (this->store != 0) {
        strcpy(buf, XorRegString(this->password));
    } else {
        buf[0] = '\0';
    }
    RegSetValueExA(rkey, "password", 0, REG_SZ, (const BYTE*)buf, strlen(buf) + 1);
}


void MainWindow::LoadSettings() { //441a03
    g_settings.Init();
    this->last_protocol = 0;
    this->com_settings.Reset();

    HKEY regkey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\1C\\Allods 2", 0, KEY_READ, &regkey) != 0) {
        return;
    }

    g_SoundSettings.Load(regkey);
    g_settings.Load(regkey);

    DWORD sz = 4;
    this->last_protocol = 0;
    RegQueryValueExA(regkey, "lastprotocol", nullptr, nullptr, (LPBYTE)&this->last_protocol, &sz); //inline 43d091

    this->phone_book.Load(regkey);

    sz = 0x14;
    RegQueryValueExA(regkey, "comportsetting", nullptr, nullptr, (LPBYTE)&this->com_settings, &sz); //inline 43d02e

    char buf[256];
    sz = 255;
    buf[0] = 0;
    RegQueryValueExA(regkey, "lastip", nullptr, nullptr, (LPBYTE)buf, &sz); //inline 43d0f5
    this->last_ip = buf;

    this->hat_settings.Load(regkey);

    sz = 4;
    RegQueryValueExA(regkey, "Using VxD", nullptr, nullptr, (LPBYTE)&g_using_vxd, &sz);
    if (g_using_vxd == 0) {
        g_using_vxd = 1;
    }

    RegCloseKey(regkey);
}


// 441b1e
void SaveSettings() {
    HKEY regkey;
    RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\1C\\Allods 2", 0, KEY_READ, &regkey);

    g_SoundSettings.Save(regkey);
    g_settings.Save(regkey);

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    RegSetValueExA(regkey, "lastprotocol", 0, REG_DWORD, (const BYTE*)&wnd->last_protocol, 4); //inline 43d0cd
    wnd->phone_book.Save(regkey);
    RegSetValueExA(regkey, "comportsettings", 0, REG_BINARY, (const BYTE*)&wnd->com_settings, 0x14); //inline 43d069

    char buf[256];
    strcpy(buf, wnd->last_ip);
    RegSetValueExA(regkey, "lastip", 0, REG_SZ, (const BYTE*)buf, strlen(buf) + 1); //inline 43d14a

    wnd->hat_settings.Save(regkey);

    RegSetValueExA(regkey, "Using VxD", 0, REG_DWORD, (const BYTE*)&g_using_vxd, 4);
    RegCloseKey(regkey);
}


void MainWindow::SetControlPositions() { //484127
    CRect r;
    GetClientRect(&r);
    this->static2.SetWindowPos(NULL, 0, 0, r.right, 40, SWP_NOREPOSITION);
    this->list_box2.SetWindowPos(NULL, 0, 40, 200, 240, SWP_NOREPOSITION);

    CRect rl2;
    this->list_box2.GetClientRect(&rl2);
    rl2.bottom += 6;

    this->static1.SetWindowPos(NULL, 200, 40, r.right - 200, rl2.bottom, SWP_NOREPOSITION);
    this->list_box1.SetWindowPos(NULL, 0, rl2.bottom + 40, r.right, r.bottom - rl2.bottom - 80, SWP_NOREPOSITION);

    CRect rl1;
    this->list_box1.GetClientRect(&rl1);
    rl1.bottom += 6;

    this->edit.SetWindowPos(NULL, 0, rl2.bottom + 40 + rl1.bottom, r.right, r.bottom - rl2.bottom - rl1.bottom - 60, SWP_NOREPOSITION);
}


// Game loop tick processing function
// Called from GameApp::OnIdle when server is active
void MainWindow::IdleLoop() {
    try {
        g_mousept.Update();

        // Calculate next tick time
        int32_t next_tick_time = this->last_tic_time + (this->game_tic_counter + 1) * this->game_tic_time;
        DWORD current_time = GetTickCount();

        if (next_tick_time <= (int32_t)current_time) {
            // Initialize timestamp on first tick
            if (this->game_tic_counter == 0) {
                this->last_tic_time = GetTickCount();
            }
            
            // Increment and wrap tick counter (0-15)
            this->game_tic_counter = (this->game_tic_counter + 1) & 0x0F;
            
            // Process server
            g_Server->ServerTic();
            
            g_mousept.Update();
            
            // Server mode processing (g_IsServer != 0 means server mode)
            if (g_IsServer != 0 && this->game_tic_counter == 1) {
                // Process network packets?
                this->ServerTickPump();
                    
                // Server shutdown notifications (message 0x0D).
                if (g_ShutdownIn != 0x7FFFFFFF) {
                    if (g_ShutdownIn < 6000000 && g_ShutdownIn > 1000) { // > 1 second
                        int32_t tick_interval = this->game_tic_time * 16;
                        if ((g_ShutdownIn % 60000) < tick_interval) {
                            if (g_ServerConfig.gameType == 0) {
                                g_NetStru1_main.SendTeamNotify(0x0D, g_ShutdownIn / 1000, nullptr);
                            }
                        }

                        if (g_ShutdownIn < 60000) {
                            int32_t tick_interval = this->game_tic_time * 16;
                            if ((g_ShutdownIn % 15000) < tick_interval) {
                                if (g_ServerConfig.gameType == 0) {
                                    g_NetStru1_main.SendTeamNotify(0x0D, g_ShutdownIn / 1000, nullptr);
                                }
                            }
                        }
                    }
                        
                    int32_t tick_interval = this->game_tic_time * 16;
                    g_ShutdownIn -= tick_interval;
                        
                    if (g_ShutdownIn < 0) {
                        g_ShutdownIn = 0;
                        g_Server->KickAllPlayers();
                        this->shutdown_pending = 1;
                    }
                }
                
                if (this->server_ctrl_shown != 0) {
                    this->server_ctrl_dlg->UpdateControlStates();
                }
                
                // Check if should shut down.
                if (this->shutdown_pending != 0) {
                    if (g_Server->FileList.GetSize() == 0) {
                        PostMessage(WM_CLOSE, 0, 0);
                    }
                }
                
                if (g_Server->field59_0x208 != 0) {
                    if (g_PlayersList->CountHumanPlayers() == 0) {
                        if (g_Server->FileList.GetSize() == 0) {
                            g_Server->ProcessPlayers();

                            this->vis_serverscreen->MsgProc(0x445, 0, 0);
                            return;
                        }
                    }
                }
                
                g_Server->map_elapsed_time += this->game_tic_time * 16;
                g_Server->map_elapsed_time2 += this->game_tic_time * 16;

                int32_t map_duration = g_ServerConfig.map_durations.GetAt(g_ServerConfig.current_map_index);
                
                // Send "map will change soon" notifications (message 0x0C).
                if (map_duration != 0x7FFFFFFF) {
                    int32_t target_time = map_duration * 60000;
                    int32_t time_diff = target_time - g_Server->map_elapsed_time;
                    
                    if (time_diff < 660000 && time_diff > 1000) {
                        int32_t tick_interval = this->game_tic_time * 16;
                        
                        if ((time_diff % 60000) < tick_interval) {
                            if (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3) {
                                g_NetStru1_main.SendTeamNotify(0x0C, time_diff / 1000, nullptr);
                            }
                        }
                        
                        if (time_diff < 60000) {
                            if ((time_diff % 15000) < tick_interval) {
                                if (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3) {
                                    g_NetStru1_main.SendTeamNotify(0x0C, time_diff / 1000, nullptr);
                                }
                            }
                        }
                    }
                    
                    // Check if map should change now.
                    if (time_diff <= (this->game_tic_time * 16)) {
                        if (g_Server->field59_0x208 == 0) {
                            g_ServerConfig.current_map_index++;
                            
                            int32_t size = g_ServerConfig.map_durations.GetSize();
                            if (g_ServerConfig.current_map_index >= size) {
                                g_ServerConfig.current_map_index = 0;
                            }
                            
                            if (g_ServerConfig.gameType == 3) {
                                g_Server->SendArenaResults();
                            }
                            
                            g_Server->SignalGameStart(0);

                            return;
                        }
                    }
                }
                
                // Some kind of timeout?
                int32_t timeout_threshold = g_ServerConfig.arena_time_limit * 60000;
                if (timeout_threshold <= g_Server->map_elapsed_time2) {
                    if (g_ServerConfig.gameType == 3) {
                        if (g_Server->field59_0x208 == 0) {
                            g_Server->ResetArenaRound(2);
                        }
                    }
                }
            }
            
            // Client mode processing (g_IsServer == 0 means client mode).
            if (g_IsServer == 0) {
                if (this->has_focus != 0) {
                    if (this->vis_serverscreen != nullptr) {
                        if (this->game_tic_counter == 1) {
                            this->vis_serverscreen->VMethod9();
                        }
                    }
                }
            }
            
            // Calculate sleep time for timing control
            int32_t desired_time = this->last_tic_time + (this->game_tic_counter + 1) * this->game_tic_time;
            int32_t sleep_time = desired_time - GetTickCount();
            
            if (sleep_time > 1) {
                Sleep(sleep_time - 1);
            }
        }
        
        g_mousept.Update();
    } catch (...) {
        #ifdef A2SERVER_PATCH
        _exit(-1);
        #endif
    }
}

void MainWindow::ServerTickPump() { //484259
    if (!g_Server) {
        return;
    }

    int32_t sredne = 0;
    if (g_Server->tic16) {
        sredne = g_Server->field44_0x1bc / g_Server->tic16 / 10;
    }

    char buf[2560];

    if (g_Server->srv_stru1->sack_list) {
        CString str;
        str.Format(txt_patch.GetLine(66), g_PlayersList->CountHumanPlayers(),
                                            dword_6CDB3C->unit_list.GetCount(),
                                            g_Server->srv_stru1->building_list->GetCount(),
                                            g_Server->srv_stru1->units_list->unit_list.GetCount(),
                                            g_Server->srv_stru1->sack_list->list.GetCount(),
                                            g_Server->field42_0x1b4 / 10,
                                            sredne,
                                            this->current_map_name,
                                            txt_main.GetLine(108 + this->game_speed));
        OemToCharA(str, buf);
        SetDlgItemText(0x7de0, buf);
    }

    CString t;
    int sel = this->list_box2.GetCurSel();
    if (sel >= 0) {
        this->list_box2.GetText(sel, t);
    }

    this->list_box2.ResetContent();

    for (POSITION it = g_PlayersList->list.GetHeadPosition(); it; ) {
        Player* pl = g_PlayersList->list.GetNext(it);
        if (pl->is_ai == 0) {
            CString s = pl->name;
            if (pl->is_logged_in == 0) {
                s = "(-)" + s;
            }
            OemToCharA(s, buf);
            this->list_box2.AddString(buf);
        }
    }

    if (!t.IsEmpty()) {
        this->list_box2.SelectString(0, t);
    }

    sel = this->list_box2.GetCurSel();

    Player* selpl = nullptr;
    
    if (sel >= 0) {
        int i = 0;
        for (POSITION it = g_PlayersList->list.GetHeadPosition(); it; ) {
            Player* pl = g_PlayersList->list.GetNext(it);
            if (pl->is_ai == 0) {
                if (i == sel) {
                    selpl = pl;
                    break;
                }
                i++;
            }
        }
    }

    if (!selpl) {
        SetDlgItemText(0x7ddf, "");
    } else {
        CString local_a54;
        CString local_a44;
        CString stat1;
        CString stat2;

        NetStru2* con = g_NetStru1_main.GetClientByPlayerID(selpl->player_id);
        if (con) {
            uint32_t conid = con->GetUid();
            if (g_CLlDriver.GetProvider() == 4) {
                con = g_NetStru1_main.GetClientByLowUid(conid & 0x3fff);
                local_a54 = con->str;
                local_a44 = con->buf;
            }
            ConnStatInfo* stat;
            if (g_NetStru1_main.client_stat.Lookup(conid, stat)) {
                stat1.Format("%d:%02d:%02d", stat->time / 3600, (stat->time % 3600) / 60, stat->time % 60);

                int bs = 0;
                if (stat->time) {
                    bs = stat->total_bytes / stat->time;
                }

                stat2.Format("%d/%d/%d", stat->cur_bs, bs, stat->max_bs);
            }
        }

        int32_t xpos = 0;
        int32_t ypos = 0;
        if (selpl->main_unit) {
            xpos = selpl->main_unit->position->GetX();
            ypos = selpl->main_unit->position->GetY();
        }
        CString txt;
        txt.Format(txt_patch.GetLine(67),
            selpl->name, "\n",
            local_a54, "\n",
            local_a44, "\n",
            selpl->hat_player_id, selpl->flags, "\n",
            selpl->player_id, "\n",
            selpl->monster_kills, "\n",
            selpl->player_kills, "\n",
            selpl->frags, "\n",
            selpl->deaths, "\n",
            selpl->money, "\n",
            ypos, xpos, "\n",
            stat1, "\n",
            stat2);

        OemToCharA(txt, buf);
        SetDlgItemText(0x7ddf, buf);
    }
}


int MainWindow::SetSpeed(int speed) {
    //48dc02
    int32_t old_speed = this->game_speed;

    if (speed < 0) {
        speed = 0;
    }
  
    if (speed > 8) {
        speed = 8;
    }

    this->game_speed = speed;

    int ticks_per_second = 16;
    switch (this->game_speed) {
    case 0:
        ticks_per_second = 8;
        break;
    case 1:
        ticks_per_second = 10;
        break;
    case 2:
        ticks_per_second = 12;
        break;
    case 3:
        ticks_per_second = 14;
        break;
    case 4:
        ticks_per_second = 16;
        break;
    case 5:
        ticks_per_second = 20;
        break;
    case 6:
        ticks_per_second = 24;
        break;
    case 7:
        ticks_per_second = 28;
        break;
    case 8:
        ticks_per_second = 32;
    }

    this->game_tic_time = 1000 / ticks_per_second;
    this->game_tic_counter = 0;
    this->field_0x43c = 0;
    this->field_0x438 = timeGetTime();
    this->last_tic_time = timeGetTime(); //GetTickCount??
    this->field_0x440 = 0;
    return old_speed != this->game_speed;
}



BOOL MainWindow::UpdateClipCursor() {
    //48cbc8
    return GetClipCursor(&this->clip_cursor_rect);
}



#ifdef A2CLIENT
void MainWindow::Proc_421() {
#pragma message("REVERSE IT, it's not FULL");

    //48d49b Allods2.exe
    this->field_0x3e0.field_10 = 0;
    this->field_0x3e0.field_14.Empty();
    this->field_0x3e0.field_0c = 0;

    this->m_FameHall.ZeroFactors();

    CVisualObject* elm = this->vis_root->FindChild(0x3fc);
    if (elm) {
        elm->MsgProc(0x445, 0, 0);
    }

    this->ShowRightPanelInfo();

    this->vis_root->AddChild(this->vis_mainmenu);
    this->vis_mainmenu->VMethod28();
    this->vis_root->VMethod9();
    this->field_0x460 = 0;
    this->dialogsMask |= 0x80;
    this->DestroyServer();

    elm = this->vis_map_context->FindChild(0x13);
    if (elm) {
        this->vis_map_context->RemoveChild(elm);
        delete elm;
    }

}
#else
void MainWindow::Proc_421()
{
    //48d49b

    field_0x3e0.field_10 = 0;
    field_0x3e0.connect_address.Empty();
    DestroyServer();
    PostMessage(0x440, 0, 0);
}
#endif


void MainWindow::ShowRightPanelInfo() {
    //485a41
    this->vis_right_panel->RemoveAllChilds();
    this->vis_right_panel->AddChild(this->vis_charinfo);

    this->vis_root->RemoveAllChilds();
    this->vis_root->AddChild(this->vis_map_context);
    this->vis_root->AddChild(this->vis_right_panel);

    this->dialogsMask &= ~1;

    this->vis_right_panel->MsgProc(0x403, (uint32_t)this->vis_map_context, 0);
}


int32_t CGameSession::GetStringArray1Size() {
    //438d10
    return this->characterRosterNames.GetSize();
}


void MainWindow::ResetItemCursor() {
    //48cd44
    if (!this->drag_entry) {
        return;
    }

    if (this->item_cursor) {
        delete this->item_cursor;
    }

    this->item_cursor = nullptr;
    this->drag_entry = nullptr;
    this->drag_src_slot = -1;
    this->drag_src_tab = -1;
}


// 445a46
DiplomacyEntry::DiplomacyEntry(const char* _name, int e, int a, int s, int m) {
    this->name = new char[strlen(_name) + 1];
    strcpy(this->name, _name);
    this->enemy = e;
    this->ally = a;
    this->see = s;
    this->mute = m;
}

//445aa9
DiplomacyEntry::~DiplomacyEntry() {
    if (this->name) {
        delete this->name;
    }
}

void MainWindow::Proc_44c(CVisualObject* obj) {
    this->field_0x460 = 1;

    this->vis_root->RemoveChild(obj);

    this->vis_map_context->field_0xe0 = 1;

    if (obj == this->vis_shop || obj == this->vis_shopdruid || obj == this->vis_shopkaarg) {
        this->vis_map_context->FUN_0041a8cc();
        this->vis_map_context->UpdateSelectionState();
        if (this->sessionMode != 2) {
            this->PlayDefaultMusic();
        }
    } else if (obj == this->vis_1200) {
        this->vis_map_context->RemoveChild(this->vis_1200);

        if (this->vis_1200->GetCloseCode() == 0x445) {

            CString str;
            this->vis_1200->WriteData(&str);
            if (str[0] == '=') {
                this->vis_map_context->FUN_0041b2a4(str.Mid(1), 3, 0);
            } else if (str[0] == '-') {
                int val = this->vis_1200->FUN_004972d0();
                if (val == -1) {
                    this->vis_map_context->FUN_0041b2a4(str.Mid(1), 1, 0);
                } else {
                    this->vis_map_context->FUN_0041b2a4(str.Mid(this->vis_1200->FUN_00497310()), 2, val);
                }
            } else {
                this->vis_map_context->FUN_0041b2a4(str, 0, 0);
            }
        }
    } else if (obj == this->vis_logownd) {
        this->dialogsMask &= ~0x100;
    } else if (obj == this->vis_famedocument) {
        this->dialogsMask &= ~0x4000;
    } else if (obj == this->vis_famehall) {
        this->dialogsMask &= ~0x1000;
        PostMessage(0x421, 0, 0);
    } else if (obj == this->vis_mainmenu) {
        this->dialogsMask &= ~0x80;
    } else if (obj == this->vis_town || obj == this->vis_towndruid || obj == this->vis_townkaarg)
    {}
    else if (obj == this->vis_tav || obj == this->vis_tavdruid || obj == this->vis_tavkaarg) {
        if (this->sessionMode != 2) {
            this->PlayDefaultMusic();
        }

        this->dialogsMask &= ~4;
        this->vis_map_context->UpdateSelectionState();
    } else if (obj == this->vis_chargen) { //charget 2step
        this->dialogsMask &= ~0x200;

        if (this->vis_chargen->GetCloseCode() == 0x446) {
            this->ShowStartGameSetupForNewSession();
        } else {
            this->m_GameSession.flags |= 4;

            if (this->sessionMode == 2) {
                this->vis_map_context->ConnectAndJoinSession();
                this->m_GameSession.SubmitCharacterSetupAndWaitForSelectedUnit();

                ScenarioSetVar(0x308, (this->m_GameSession.type & 0x40) != 0);
                ScenarioSetVar(0x30d, (this->m_GameSession.type & 0x80) != 0);

                if (g_Server) {
                    g_Server->FUN_00497470(this->m_FameHall.GetDifficulty() + 1);
                }

                PostMessage(0x42e, 0, 0);
            } else {
                this->m_GameSession.ResetCharacter();

                if (this->field_0x3e0.field_10 == 0) {
                    PostMessage(0x451, 0, 0);
                } else {
                    PostMessage(0x489, 0, 0);
                }
            }
        }
    } else if (obj == this->vis_startgame) { // chargen 1step
        this->dialogsMask &= ~0x800;

        if (this->vis_startgame->GetCloseCode() == 0x446) {
            if (!this->vis_map_context->field_0x9d0.IsEmpty()) {
                this->vis_map_context->field_0x9d0.RemoveAll();

                CUnit *uni = this->vis_map_context->GetUnit_3f6c();
                if (uni) {
                    delete uni;
                }

                this->vis_map_context->SetUnit_3f6c(nullptr);
            }

            if (this->sessionMode == 2) {
                PostMessage(0x421, 0, 0);
            } else {
                this->ShowCharacterLoaderDialog();
            }
        } else {
            CString name = this->vis_startgame->GetName();
            strcpy(this->m_GameSession.character_name, name);

            if (!name.IsEmpty()) {
                int pos = name.Find(':');
                if (pos != -1) {
                    name = name.Mid(pos + 1);

                    this->m_GameSession.color = atoi(name);

                    if (this->m_GameSession.color == 0 || this->m_GameSession.color > 16) {
                        this->m_GameSession.color = 0;
                    }

                    name = this->vis_startgame->GetClan().Left(pos);
                }
            }

            if (!name.IsEmpty()) {
                strcat(this->m_GameSession.character_name, "|");
                strcat(this->m_GameSession.character_name, name);
            }

            this->m_FameHall.SetDifficulty(this->vis_startgame->GetSelectedDifficulty());

            this->m_GameSession.type = this->vis_startgame->GetSelectedPortraitPlayerType() << 6; // 0x40 and 0x80 bits

            name = this->vis_startgame->GetName();
            strncpy(this->vis_map_context->GetUnit_3f6c()->str1, name, 12);

            name = this->vis_startgame->GetClan();
            strncpy(this->vis_map_context->GetUnit_3f6c()->str2, name, 12);

            this->m_GameSession.ResetUnitClass();
            this->ShowCharInfoPane();
        }
    } else if (obj == this->vis_charsel) {
        this->dialogsMask &= ~0x400;

        if (this->vis_charsel->GetCloseCode() == 0x446) {
            if (!this->field_0x3e0.field_08.IsEmpty()) {
                PostMessage(WM_CLOSE, 0, 0);
            } else {
                PostMessage(0x421, 0, 0);
            }
        } else if ((this->m_GameSession.flags & 4) == 0) {
            this->ShowStartGameSetupForNewSession();
        } else if (this->field_0x3e0.field_10 == 0) {
            PostMessage(0x451, 0, 0);
        } else {
            PostMessage(0x489, 0, 0);
        }
    } else if (obj == this->vis_globalmap) {
        this->dialogsMask &= ~0x20;
    } else if (obj == this->vis_credits) {
        this->dialogsMask &= ~0x40;

        if (FUN_00497490() == 0) {
            CWnd::PostMessage(0x421, 0, 0);
        } else {
            CWnd::PostMessage(0x429, 0, 0);
        }
    } else if (obj == this->vis_dropgold) {
        this->dialogsMask &= ~0x8;

        this->field_0x438 = timeGetTime();
        this->game_tic_counter = 0;
        this->field_0x43c = 0;
    } else if (obj == this->field_0x12c) {
        this->dialogsMask &= ~0x8;

        if (this->field_0x12c->GetCloseCode() == 0x447) {
            this->vis_map_context->FUN_0041d2da(1);
            PostMessage(0x421, 0, 0);
        } else {
            PostMessage(0x42e, 0, 0);
        }

        delete this->field_0x12c;
        this->field_0x12c = nullptr;
    } else if (obj == this->vis_savegame) { //save game
        this->dialogsMask &= ~0x8;

        if (this->vis_savegame->GetCloseCode() == 0x445) {
            this->AutosaveGame();
        }

        if (this->dialogsMask == 1 && this->serverBootstrapEnabled != 0) {
            this->field_0x438 = timeGetTime();
            this->game_tic_counter = 0;
            this->field_0x43c = 0;
        }

        delete this->vis_savegame;
        this->vis_savegame = nullptr;
    } else if (obj == this->vis_loadgame) { //load game
        this->dialogsMask &= ~8;

        int32_t code = this->vis_loadgame->GetCloseCode();
        if (code == 0x445) {
            PostMessage(0x419, 0, 0);
        } else if (g_Server == nullptr) {
            PostMessage(0x421, 0, 0);
        } else if (this->last_event_id == 0xff) {
            PostMessage(0x41e, 0, 0);
        }

        if (this->dialogsMask == 1 && this->serverBootstrapEnabled != 0) {
            this->field_0x438 = timeGetTime();
            this->game_tic_counter = 0;
            this->field_0x43c = 0;
        }

        delete this->vis_loadgame;
        this->vis_loadgame = nullptr;

        if (code == 0x445) {
            return;
        }
    } else if (obj == this->field_0x34c) {
        this->dialogsMask &= ~8;

        int32_t code = this->field_0x34c->GetCloseCode();

        if (code == 0x44f) {
            this->serverBootstrapEnabled = 0;
            this->EnterSinglePlayerMode();
        } else if (code == 0x44d) {
            this->EnterServerMode();
        } else if (code == 0x44e) {
            this->EnterClientMode();
        } else {
            PostMessage(0x421, 0, 0);
        }
            
        delete this->field_0x34c;
        this->field_0x34c = nullptr;
        return;
    } else if (obj == this->vis_connectiondlg) {
        this->dialogsMask &= ~8;

        if (this->vis_connectiondlg->GetCloseCode() == 0x446) {
            if (this->sessionMode == 3) {
                PostMessage(0x421, 0, 0);
            } else {
                this->ShowCharacterLoaderDialog();
            }
        }

        delete this->vis_connectiondlg;
        this->vis_connectiondlg = nullptr;
        return;
    } else if (obj == this->vis_netmapsel) {
        this->dialogsMask &= ~8;

        if (this->vis_netmapsel->GetCloseCode() == 0x446) {
            this->vis_root->MsgProc(0x446, 0, 0);
            PostMessage(0x45c, 0, 0);
        } else {
            PostMessage(0x457, 0, 0);
        }
        
        delete this->vis_netmapsel;
        this->vis_netmapsel = nullptr;
        return;
    } else if (obj == this->vis_hatbrowser) {
        this->dialogsMask &= ~8;

        if (this->vis_hatbrowser->GetCloseCode() == 0x446) {
            if (this->hat_settings.ishat == 0) {
                this->vis_root->MsgProc(0x446, 0, 0);
                this->ShowCharacterLoaderDialog();
            } else if (g_CLlDriver.IsListen()) {
                this->ShowCharacterLoaderDialog();
            } else {
                PostMessage(0x487, 0, 0);
            }
        } else if (this->hat_settings.ishat == 0) {
            g_NetStru1_local.SetLLDriver(&g_CLlDriver);
            g_CLlDriver.SetHlDriver(&g_NetStru1_local);
            g_CLlDriver.ResetProvider(3);
            PostMessage(0x452, 0, 0);
        } else {
            PostMessage(0x441, 0, 0);
        }
        
        delete this->vis_hatbrowser;
        this->vis_hatbrowser = nullptr;
        return;
    } else if (obj == this->vis_serverscreen) {
        this->dialogsMask &= ~8;
        this->dialogsMask = 0;

        if (this->vis_serverscreen->GetCloseCode() == 0x445) {
            PostMessage(0x421, 0, 0);
        }

        delete this->vis_serverscreen;
        this->vis_serverscreen = nullptr;
    } else if (obj == this->vis_msgbox) {
        this->dialogsMask &= ~8;

        int32_t code = this->vis_msgbox->GetCloseCode();

        this->vis_msgbox = nullptr;

        if (code == 0x445) {
            int j = 0;
            for (int i = 0; i < this->vis_map_context->field_0x9b8.GetSize(); i++) {
                MapPlayerData* mp = this->vis_map_context->field_0x9b8[i];
                if (mp != nullptr && (mp->flags & 1) == 0) {
                    DiplomacyEntry* visdp = (*this->diplomacy_entries)[j];

                    uint16_t dpl = this->vis_map_context->my_main_unit->diplomacy[mp->index] & 0x68;

                    if (visdp->enemy != 0) {
                        dpl |= 1;
                    }

                    if (visdp->ally != 0) {
                        dpl |= 2;
                    }

                    if (visdp->see != 0) {
                        dpl |= 0x10;
                    }

                    if (visdp->mute != 0) {
                        dpl |= 4;
                    }

                    this->vis_map_context->my_main_unit->diplomacy[mp->index] = dpl;

                    this->vis_map_context->my_main_unit->diplomacy[this->vis_map_context->my_main_unit->index] = 0x3a;

                    j++;
                }
            }

            this->vis_map_context->FUN_0041a735();
        }

        delete obj;
    } else if (obj == this->vis_cutscenes) {
        this->dialogsMask &= ~8;

        if (this->vis_cutscenes->GetCloseCode() == 0x445) {
            DAT_00660f88 = ((VisListBox*)this->vis_cutscenes->FindChild(2))->GetSelectedIndex();
            this->PlayVideo(DAT_00660f88);
            PostMessage(0x43b, 0, 0);
        } else {
            PostMessage(0x421, 0, 0);
        }

        delete this->vis_cutscenes;
        this->vis_cutscenes = nullptr; // MUST BE DONE? WAT?
    } else if (obj == this->vis_missionfailed) {
        this->dialogsMask &= ~8;

        if (this->vis_missionfailed->GetCloseCode() == 0x445) {
            PostMessage(0x41e, 0, 0);
        } else {
            PostMessage(0x418, 0, 0);
        }

        delete this->vis_missionfailed;
        this->vis_missionfailed = nullptr; // MUST BE DONE? WAT?
    } else if (obj == this->vis_msgbox_list) {
        this->dialogsMask &= ~0x8000;

        delete this->vis_msgbox_list;
        this->vis_msgbox_list = nullptr; // MUST BE DONE? WAT?

        if ((this->dialogsMask & 1) == 0) {
            this->vis_root->cursor_over_obj_last = this->vis_root->FindChild(1020);
        }
    } else if ((this->dialogsMask & 8) != 0) {
        this->dialogsMask &= ~8;

        if (this->dialogsMask == 1 && this->sessionMode == 2) {
            this->field_0x438 = timeGetTime();
            this->game_tic_counter = 0;
            this->field_0x43c = 0;
        }

        delete obj;
    }

    if (this->dialogsMask == 0x80) {
        g_Cursors[CURSOR_SELECT]->Use();
    }

    this->vis_right_panel->MsgProc(0x408, 0, 0);

    if (this->dialogsMask != 1 || this->vis_map_context->field_0x80 != nullptr) {
        this->vis_root->VMethod9();
    }

    if (this->dialogsMask == 1 && DAT_006658d8.GetSize() != 0) {
        PostMessage(0x433, DAT_006658d8[0], 0);
        DAT_006658d8.RemoveAt(0);
    }
}


void MainWindow::ShowCharacterLoaderDialog() { //48cff7
    g_Cursors[CURSOR_WAIT]->Use();

    this->vis_charinfo->info_mode = 1;
    this->vis_root->AddChild(this->vis_charsel);

    this->vis_charsel->VMethod28();

    this->dialogsMask |= 0x400;
    
    this->vis_root->VMethod9();

    this->field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();
    return;
}


void MainWindow::ShowCharInfoPane() { //491a49
    g_Cursors[CURSOR_WAIT]->Use();

    this->vis_charinfo->info_mode = 1;

    this->vis_right_panel->MsgProc(0x403, (uint32_t)this->vis_map_context, 0);
    
    this->vis_map_context->GetUnit_3f6c()->SetSelected(1); // select

    this->vis_map_context->UpdateSelectionState();

    this->vis_root->AddChild(this->vis_chargen);
    this->vis_chargen->VMethod28();

    this->vis_root->VMethod9();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x200;

    g_Cursors[CURSOR_DEFAULT]->Use();
}




void MainWindow::AutosaveGame() { // 48f905
    if (this->sessionMode == 2) {
        CString local_2a4 = this->last_save_info.filename;
        g_Server->SaveGame(&local_2a4);

        this->field_0x438 = timeGetTime();
        this->game_tic_counter = 0;
        this->field_0x43c = 0;

        CFile local_13c;
        local_13c.Open(this->last_save_info.filename, CFile::modeReadWrite);
        local_13c.Seek(0, CFile::end);
        local_13c.Write(this->last_save_info.title, 0x100);
        
        int inbattle = this->dialogsMask & 1;

        RegFile local_180;
        local_180.SetInt("CurrentState", "InBattle", inbattle);
        local_180.SetString("Character", "Name", this->m_GameSession.character_name);
        local_180.SetInt("GameOptions", "Wimpy", this->vis_map_context->wimpy);
        local_180.SetInt("GameOptions", "ShowHP", this->vis_map_context->show_hp);
        local_180.SetInt("GameOptions", "FlyingHP", this->vis_map_context->flying_hp);
        local_180.SetInt("GameOptions", "Formation", this->vis_map_context->formation);
        local_180.SetInt("GameOptions", "Speed", this->game_speed);
        local_180.SetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);
        local_180.SetInt("View", "X", this->vis_map_context->view_x);
        local_180.SetInt("View", "Y", this->vis_map_context->view_y);

        local_180.SetInt("SpellBook", "IsOpen", this->vis_map_context->IsBookOpen());
        local_180.SetInt("SpellBook", "Pressed", this->vis_spellbook->pressed);

        CArray<uint32_t> local_25c;
        local_180.SetInt32Array("SpellBook", "Shortcuts", local_25c);

        CArray<uint16_t> local_12c;
        CArray<uint16_t> local_248[10];

        if (!this->vis_map_context->field_0x9d0.IsEmpty()) {
            for (POSITION pos = this->vis_map_context->field_0x9d0.GetStartPosition(); pos != nullptr;) {
                uint16_t key;
                CGameObject* obj;

                this->vis_map_context->field_0x9d0.GetNextAssoc(pos, key, obj);

                if (obj->IsSelected() != 0) {
                    local_12c.Add(key);
                }

                for (int i = 0; i < 10; i++) {
                    if (obj->IsInControlGroup(i) != 0) {
                        local_248[i].Add(key);
                    }
                }
            }
        }

        local_180.SetInt16Array("Objects", "Selection", local_12c);

        char buf[256];

        for (int i = 0; i < 10; i++) {
            if (local_248[i].GetSize() != 0) {
                sprintf(buf, "Group%d", i);
                local_180.SetInt16Array("Objects", buf, local_248[i]);
            }
        }

        local_180.SetInt("Inventory", "IsOpen", this->vis_map_context->IsBagOpen());

        if (inbattle) {
            CArray<uint16_t> local_27c;

            local_180.SetInt("Projectiles", "FreeIndex", this->vis_map_context->field_0xa24);

            if (!this->vis_map_context->field_0x9ec.IsEmpty()) {
                for (POSITION pos = this->vis_map_context->field_0x9ec.GetStartPosition(); pos != nullptr;) {
                    uint16_t key;
                    CGameObject* obj;
                    this->vis_map_context->field_0x9ec.GetNextAssoc(pos, key, obj);

                    local_27c.Add(key);

                    sprintf(buf, "Prj%d", key);

                    local_180.SetInt(buf, "x", obj->x_pos);
                    local_180.SetInt(buf, "y", obj->y_pos);
                    local_180.SetInt(buf, "z", obj->z_pos);
                    local_180.SetInt(buf, "picture", obj->typeId);
                    local_180.SetInt(buf, "dir", obj->dir);
                    local_180.SetInt(buf, "phase", obj->phase);
                    local_180.SetInt(buf, "lastaction", obj->last_action);
                    local_180.SetInt(buf, "action", obj->action);
                    local_180.SetInt(buf, "actiondir", obj->action_dir);
                    local_180.SetInt(buf, "actiontarget", obj->action_target);
                    local_180.SetInt(buf, "actionx", obj->action_x);
                    local_180.SetInt(buf, "actiony", obj->action_y);
                    local_180.SetInt(buf, "actionz", obj->action_z);
                    local_180.SetInt(buf, "actionphase", obj->action_phase);
                    local_180.SetInt(buf, "actionsegments", obj->action_segments);
                    local_180.SetInt(buf, "actionspell", obj->action_spell);
                }
            }
            local_180.SetInt16Array("Projectiles", "IDs", local_27c);
        }

        if (inbattle) {
            CArray<uint32_t> local_2a0;

            uint16_t* land = this->vis_map_context->field_0x80->GetLandscape();
            int numcells = this->vis_map_context->field_0x80->GetWidth() * this->vis_map_context->field_0x80->GetHeight();

            uint16_t state = *land & 0x8000;
            local_180.SetInt("Fog", "FirstState", state);

            int idx = 0;
            int num = 1;
            while (true) {
                idx++;
                land++;

                if (idx >= numcells) {
                    local_2a0.Add(num);
                    break;
                }
                
                if (state == (*land & 0x8000)) {
                    num++;
                } else {
                    state = *land & 0x8000;
                    local_2a0.Add(num);
                    num = 1;
                }
            }

            local_180.SetInt32Array("Fog", "Data", local_2a0);
        }

        local_180.WriteToFile(&local_13c);

        this->m_FameHall.SaveDocuments(&local_13c);

        ScenarioSave(&local_13c);

        for (int i = 0; i < 9; i++) {
            this->m_GameSession.shortcuts[i].WriteToFile(&local_13c);
        }

        local_13c.Close();
    } else {
        this->vis_map_context->FUN_0041afcf(this->last_save_info.filename);
    }
}


void MainWindow::EnterSinglePlayerMode() { // 4903d0

    this->sessionMode = 0;

    g_NetStru1_local.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_local);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-waitforever")) {
        g_CLlDriver.SetTimeout(-1);
    }

    if (strstr(cmdline, "-timeout")) {
        g_CLlDriver.SetTimeout(g_CmdTimeout);
    }

    if (this->vis_map_context->ConnectAndJoinSession() == 0) {
        g_CLlDriver.Close();
        PostMessage(0x41d, 0, 0);
    } else {
        if (this->m_GameSession.phased_in == 0) {
            VisMessageBoxWithList* mwnd = new VisMessageBoxWithList(1, 64, 100, 428, 594, TxtFile::AllLines[210], nullptr, 4);
            this->ModalScreen(mwnd);

            if (mwnd->GetCloseCode() != 0x447) {
                PostMessage(0x41d, 0, 0);
                return;
            }
        }

        if (this->inside_building == 0) {
            PostMessage(0x459, 0, 0);
        } else {
            PostMessage(0x457, 0, 0);
        }
    }
}

void MainWindow::EnterServerMode() { //491822
    this->sessionMode = 3;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2) {
        g_CLlDriver.StartServer(g_ServerConfig.max_players, this->m_GameSession.character_name, nullptr);
    }

    this->StartServerSession(0);

    PostMessage(0x459, 0, 0);
}

int MainWindow::StartServerSession(int mode) { //48ca7e
    DAT_006658d8.RemoveAll();

    g_Server = new Server();
    if (g_Server->Start(mode) == 0) {
        this->serverBootstrapEnabled = 1;
        return 0;
    } else {
        this->serverBootstrapEnabled = 0;
        return 1;
    }
}


void MainWindow::EnterClientMode() { // 4918ae

    this->sessionMode = 1;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-timeout")) {
        g_CLlDriver.SetTimeout(g_CmdTimeout);
    }

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2) {
        g_CLlDriver.StartServer(g_ServerConfig.max_players, this->m_GameSession.character_name, nullptr);
    }

    this->StartServerSession(this->sessionMode);

    if (this->vis_map_context->ConnectAndJoinSession() == 0) {
        this->vis_root->MsgProc(0x446, 0, 0);
        PostMessage(0x45c, 0, 0);
    } else {
        PostMessage(0x459, 0, 0);
    }
}


void MainWindow::PlayVideo(int32_t vid_id) { //491f7d
    printf("Video play PlayVideo not implemented yet: %d\n", vid_id);
}


LRESULT MainWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) { // 486c6c

    switch (message) {
    default:
        if (this->vis_root && vis_root->MsgProc(message, wParam, lParam) != 0) {
            return DefWindowProc(message, wParam, lParam);
        }
        break;

    case WM_KEYDOWN:
        break;
    case WM_KEYUP:
        vis_root->MsgProc(WM_KEYUP, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        if (vis_root->MsgProc(WM_MOUSEMOVE, wParam, lParam) != 0) {
            SetCursorPos(g_mousept.GetX(), g_mousept.GetY());
        }

        g_mousept.ResetNext400msg();
        break;

    case WM_LBUTTONDOWN:
        vis_root->MsgProc(message, wParam, lParam);
        g_mousept.ResetNext400msg();
        g_mousept.ForceDisableHint();
        g_LButtonDown = 1;
        break;

    case WM_LBUTTONUP:
        vis_root->MsgProc(message, wParam, lParam);
        g_mousept.ResetNext400msg();
        g_mousept.ForceDisableHint();
        g_LButtonDown = 0;
        break;

    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
        vis_root->MsgProc(message, wParam, lParam);
        g_mousept.ResetNext400msg();
        g_mousept.ForceDisableHint();
        break;

    case 0x400:
        vis_root->MsgProc(0x400, wParam, lParam);
        g_mousept.ForceDisableHint();
        break;

    case 0x416:
        if (this->dialogsMask == 1) {
            this->PopUpScreen(new IngameMenu(1, 100, 60, 440, 400, nullptr, 0, CRect(0, 0, 240, 40)));
        }
        break;

    case 0x418:
        this->last_save_info.title[0] = 0;
        this->last_save_info.filename[0] = 0;
        this->vis_loadgame = new LoadGameWindow(1, 100, 30, 600, 450, 0, &this->last_save_info);
        this->PopUpScreen(this->vis_loadgame);
        break;

    case 0x419:
        if (this->dialogsMask == 1) {
            g_Server->ProcessPlayers();
        }

        this->DestroyServer();

        if (this->dialogsMask == 0 && vis_root->FindChild(1020) != nullptr) {
            this->vis_map_context->FUN_0041d2da(1);

            ScenarioLocation* cur_location = ScenarioGetCurrentLocation();
            if (cur_location != nullptr) {
                int loc_type = cur_location->GetId();
                if (loc_type == 1) {
                    this->vis_town->MsgProc(0x445, 0, 0);
                } else if (loc_type == 2) {
                    this->vis_townkaarg->MsgProc(0x445, 0, 0);
                } else if (loc_type == 3) {
                    this->vis_towndruid->MsgProc(0x445, 0, 0);
                }
            }
        }
        this->BeginLocalCharacterSetup();
        break;

    case 0x41a:
        if (this->dialogsMask == 1 || this->dialogsMask == 0) {
            this->last_save_info.title[0] = 0;
            this->last_save_info.filename[0] = 0;
            this->vis_savegame = new SaveGameWindow(1, 100, 30, 600, 450, 0, &this->last_save_info);
            this->PopUpScreen(this->vis_savegame);
        }
        break;

    case 0x41b:
        if (this->dialogsMask == 1 || this->dialogsMask == 0) {
            this->PopUpScreen(new GameOptionsWindow(1, 20, 0, 620, 480));
        }
        break;

    case 0x41c:
        if (this->dialogsMask == 1) {
            this->PopUpScreen(new EndGameMenu(1, 100, 100, 440, 340, nullptr, 0, CRect(0, 0, 300, 50), this->in_famehall_mission));
        } else if (this->dialogsMask == 0) {
            this->PopUpScreen(new ExitGameMenu(1, 100, 100, 440, 340, CRect(0, 0, 300, 70)));
        }
        break;

    case 0x41d:
        if (this->sessionMode == 1 || this->sessionMode == 3) {
            if (this->sessionMode == 1) {
                this->vis_map_context->FUN_0041d2da(0);
            } else {
                this->vis_map_context->FUN_0041d2da(1);
            }

            if ((this->dialogsMask & 1) != 0) {
                this->UnloadMapAssets();
            }
            
            g_Server->ProcessPlayers();
            PostMessage(0x459, 0, 0);
        } else if (this->sessionMode == 0) {
            this->vis_map_context->FUN_0041d2da(1);

            if ((this->dialogsMask & 1) != 0) {
                this->UnloadMapAssets();
            }

            g_CLlDriver.Close();

            this->field_0x438 = 0x7fffffff;
            this->last_tic_time = 0x7fffffff;

            PostMessage(0x452, 0, 0);
        } else if (this->in_famehall_mission == 0) {
            g_Server->ProcessPlayers();
            PostMessage(0x421, 0, 0);
        } else {
            this->m_FameHall.AddMissionElapsedTime(g_Server->tick / 16);
            this->vis_map_context->CleanupCompletedMissionMapState();

            if ((this->dialogsMask & 1) != 0) {
                this->UnloadMapAssets();
            }
            
            if (g_Server) {
                g_Server->ProcessPlayers();
            }
            
            this->ShowRightPanelInfo();

            this->vis_globalmap->SetTravelOrigin(ScenarioGetCurrentLocation()->rect.TopLeft());

            int32_t loc;
            int32_t local_894 = ScenarioLeaveLocation(&loc);

            CDWordArray local_8ac;
            int32_t var = ScenarioGetVar(0x305);
            if (var != 0) {
                local_8ac.Add(var);
                g_Server->FUN_00501b9e(ScenarioGetVar(0x300), local_8ac);
                this->vis_map_context->ProcessPackets(0);
            }

            if (loc > -1) {
                this->PlayVideo(loc);
            }

            if (FUN_00497490() == 0) {
                if (local_894 != 0) {
                    this->vis_map_context->SendAdjustPlayerGoldAction(local_894);
                }

                if (ScenarioIsTownAvailable(0) == 0) {
                    CList<ScenarioLocation*>* locs = ScenarioGetAvailableLocations();
                    ScenarioEnterLocation(locs->GetHead());
                    PostMessage(0x468, 0, 0);
                } else {
                    this->vis_globalmap->umoirMapMode = 0;
                    this->ShowGlobalMapDialog();
                }
            } else {
                this->m_FameHall.SubmitScore();
                this->vis_map_context->FUN_0041d2da(1);
                PostMessage(MSG_428, 0 ,0);
            }
        }
        break;

    case 0x41e:
        g_Server->ProcessPlayers();
        PostMessage(MSG_421, 0, 0);
        break;

    case 0x41f:
        this->PopUpScreen(new TownMenuListDialogVisualObject(1, 100, 100, 440, 340, CRect(0, 0, 240, 40)));
        break;

    case 0x420:
        this->PopUpScreen(new QuestObjectivesHeaderDialogVisualObject(1, 20, 0, 620, 500));
        break;

    case 0x421:
        if (this->dialogsMask == 0) {
            this->Proc_421();
        }
        break;

    case 0x422:
        if (this->dialogsMask == 1 || (this->dialogsMask == 0 && g_Server != nullptr)) {
            this->PopUpScreen(new SoundPreferencesDialogVisualObject(1, 100, 30, 640, 450, nullptr, &g_SoundSettings));
        } else if (this->dialogsMask == 0) {
            PostMessage(MSG_487, 0, 0);
        }
        break;

    case 0x425:
        ReadFileToString("main\\text\\town.txt", &g_MissionText);
        ScenarioNewGame();
        this->serverLoopCounter = 0;
        this->serverBootstrapEnabled = 1;
        this->InitNewCampaignSession();
        this->m_FameHall.ZeroFactors();
        this->vis_globalmap->RebuildScenarioLocations();
        this->m_FameHall.SetDifficulty(1);

        if (this->sessionMode == 3) {
            PostMessage(MSG_42f, 0, 0);
        } else {
            this->m_GameSession.RefreshCharacterRosterFiles(1);
        }
            this->m_GameSession.InitializeNewCharacterSession(0, nullptr);
            this->ShowStartGameSetupForNewSession();
        break;

    case 0x426:
        ReadFileToString("main\\text\\town.txt", &g_MissionText);
        this->serverLoopCounter = 0;
        this->sessionMode = 0;
        this->ShowCharacterLoaderDialog();
        break;

    case 0x427:
        this->ShowStartupLogoDialog();
        this->vis_logownd->MsgProc(0x43a, 1, 0);
        this->vis_logownd->MsgProc(0x439, 4000, 0);
        vis_root->VMethod9();
        break;

    case 0x428:
        this->ShowCreditsDialog();
        break;

    case 0x429:
        this->ShowFameHallDialog();
        break;

    case 0x42a:
        if (this->dialogsMask == 0 || (this->sessionMode < 2 && this->dialogsMask == 1)) {
            this->ShowShopDialog(wParam);
        }
        break;

    case 0x42b:
        if (this->dialogsMask == 0 || (this->sessionMode < 2 && this->dialogsMask == 1)) {
            this->ShowInnDialog(wParam, lParam);
        }
        break;

    case 0x42d: {
        this->vis_globalmap->umoirMapMode = 0;
        ScenarioLocation* cloc = ScenarioGetCurrentLocation();
        if (cloc->GetKind() == 2) {
            this->vis_globalmap->SetTravelOrigin(cloc->GetRect().TopLeft());

            int32_t loc = -1;
            ScenarioLeaveLocation(&loc);

            if (loc > -1) {
                this->PlayVideo(loc);
            }

            if (cloc->GetId() == 1) {
                this->vis_globalmap->umoirMapMode = 1;
            }
        }
        this->ShowGlobalMapDialog();
    }
        break;

    case 0x42e:
        this->ShowCurrentTownDialog();
        break;

    case 0x42f:
        this->RunSessionBootstrap(0);
        break;

    case 0x430:
        if (FUN_00497490() == 0) {
            this->in_famehall_mission = 1;

            CRect local_8d0(128, 200, 512, 380);
            CRect local_8c0(local_8d0.Width() / 8, local_8d0.Height() - 120, local_8d0.Width() * 7 / 8, local_8d0.Height() - 96);
            this->vis_2action = new Vis2Action(1, local_8d0.left, local_8d0.top, local_8d0.right, local_8d0.bottom, txt_main.GetLine(140), local_8c0);
            this->PopUpScreen(this->vis_2action);
            if (!g_SfxArray[14]->FindPlayingChannel()) {
                g_SfxArray[14]->Play(g_SoundSettings.sfx_pos, 0, 0, 220);
            }
        } else {
            this->in_famehall_mission = 1;
            PostMessage(0x41d, 0, 0);
        }
        break;

    case 0x431:
        if (this->mission_failed_reported == 0 && this->sessionMode == 2) {
            this->mission_failed_reported = 1;
            this->vis_missionfailed = new VisMissionFailed(1, 32, 100, 608, 380, g_MissionFailures[wParam - 2]);
            this->PopUpScreen(this->vis_missionfailed);

            if (!g_SfxArray[16]->FindPlayingChannel()) {
                g_SfxArray[16]->Play(g_SoundSettings.sfx_pos, 0, 0, 220);
            }
        }
        break;

    case 0x432:
        PostMessage(0x436, 0, 0);
        break;

    case 0x433:
        this->last_event_id = wParam;

        if (this->last_event_id == 255) {
            PostMessage(0x431, lParam, 0);
        } else if (this->last_event_id == 254) {
            if (!g_SfxArray[16]->FindPlayingChannel()) {
                g_SfxArray[16]->Play(g_SoundSettings.sfx_pos, 0, 0, 220);
            }
        } else if (this->last_event_id == 253) {
            if (!g_SfxArray[14]->FindPlayingChannel()) {
                g_SfxArray[14]->Play(g_SoundSettings.sfx_pos, 0, 0, 220);
            }
        } else if (this->last_event_id == 250) {
            if (!g_SfxArray[14]->FindPlayingChannel()) {
                g_SfxArray[14]->Play(g_SoundSettings.sfx_pos, 0, 0, 220);
            }
            this->vis_map_context->msglog.Add(txt_patch.GetLine(97), clr_log_sblack, 30000);
        } else if ((this->dialogsMask & 8) == 0) {
            CString local_850;
            local_850.Format("event%d", this->last_event_id);
            ShowRoleKeyDialog(local_850);
        } else {
            DAT_006658d8.Add(this->last_event_id);
        }
        break;

    case 0x434:
        if (this->dialogsMask == 1) {
            this->PopUpScreen(new VisMessageBoxWithList(1, 32, 48, 608, 432, g_HelpStr, nullptr, 0));
        }
        break;

    case 0x435:
        this->LoadData();
        break;

    case 0x436:
        this->PlayVideo(0);
        PostMessage(WM_CLOSE, 0, 0);
        break;

    case 0x438:
        switch (wParam) {
        case 0:
        case 1:
            this->vis_logownd->MsgProc(0x43a, wParam + 1, 0);
            this->vis_logownd->MsgProc(0x439, 4000, 0);
            vis_root->VMethod9();
            break;

        case 2:
            this->vis_logownd->MsgProc(0x43a, wParam + 1, 0);
            this->vis_logownd->MsgProc(0x439, 6000, 0);
            vis_root->VMethod9();
            this->LoadData();
            break;

        case 3:
            this->vis_logownd->MsgProc(0x43a, wParam + 1, 0);
            this->vis_logownd->MsgProc(0x439, 30000, 0);
            vis_root->VMethod9();
            break;

        case 4:
            this->vis_logownd->DoClose(0x445);
            vis_root->RemoveChild(this->vis_logownd);
            this->dialogsMask &= ~0x100;
            PostMessage(0x421, 0, 0);
            break;

        case 5:
            this->vis_logownd->DoClose(0x445);
            vis_root->RemoveChild(this->vis_logownd);
            this->dialogsMask &= ~0x100;
            PostMessage(WM_CLOSE, 0, 0);
            break;

        case 6:
        case 7:
        case 8:
        case 9:
            this->vis_logownd->DoClose(0x445);
            vis_root->RemoveChild(this->vis_logownd);
            this->dialogsMask &= ~0x100;
            PostMessage(0x425, 0, 0);
            break;
        }
        break;

    case 0x43b:
        /*if (DAT_0062f87c == 0)     // if NO CD
            PostMessage(0x421, 0, 0);
        else*/
        {
            CStringArray local_934;
            for (int i = 0; i < 32; i++) {
                if ((g_using_vxd & (1 << i)) != 0) {
                    local_934.Add(txt_cutscene.GetLine(i));
                }
            }
            this->vis_cutscenes = new VisCutScenesDlg(17, 100, 30, 540, 450, local_934);
            this->PopUpScreen(this->vis_cutscenes);
        }
        break;

    case 0x43c:
        if (this->dialogsMask == 1) {
            this->ClearDiplomacyEntries();
            // The original passes the address of the array pointer so the dialog
            // re-reads the current array on every ReadData (see VisDiplomacy::diplomacy).
            this->vis_msgbox = new VisDiplomacy(1, 10, 0, 630, 480, (const CArray<DiplomacyEntry*>*)&this->diplomacy_entries);
            this->PopUpScreen(this->vis_msgbox);
        }
        break;

    case 0x440:
        this->StartHatDedicatedServer();
        break;

    case 0x441:
        if (this->WaitHatConnection() == 0) {
            if (g_CLlDriver.IsListen() == 0) {
                CWnd::PostMessage(0x487, 0, 0);
            } else {
                CWnd::PostMessage(0x489, 0, 0);
            }
        } else {
            if (this->ConnectToServerAddress() == 0) {
                PostMessage(0x487, 0, 0);
            } else {
                this->WaitConnectionCharacterJoin();
            }
        }
        break;

    case 0x442:
        if (wParam == 0) {
            strcpy(this->last_save_info.title, txt_patch.GetLine(55));
        } else {
            strcpy(this->last_save_info.title, txt_patch.GetLine(95));
        }
        sprintf(this->last_save_info.filename, "game%d.sav", 9999 - wParam);
        this->AutosaveGame();
        this->field_0x438 = timeGetTime();
        this->game_tic_counter = 0;
        this->field_0x43c = 0;
        break;

    case 0x443:
        this->dialogsMask |= 8;
        g_Cursors[CURSOR_DEFAULT]->Use();
        break;

    case 0x44c:
        this->Proc_44c((CVisualObject*)wParam);  // FIXME
        break;

    case 0x451:
        if (this->field_0x3e0.field_08.IsEmpty() == 0) {
            this->WaitConnectionCharacterJoin();
        } else if (this->dialogsMask == 0) {
            this->music_player->OnEndTrack();
            if (!IsDplayAvail) {
                this->ModalScreen(new VisMessageBoxWithList(1, 100, 100, 540, 380, txt_main.GetLine(152), nullptr, 0));
                PostMessage(0x421, 0, 0);
            } else {

                this->vis_connectiondlg = new VisConnectionDlg(1, 0, 0, 640, 350, &this->last_protocol);
                this->PopUpScreen(this->vis_connectiondlg);

                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol0")) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(0);
                }
                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol1")) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(1);
                }
                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol2")) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(2);
                }
                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol3")) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(3);
                }
                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol4")) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(4);
                }

                if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol")) {
                    this->vis_connectiondlg->MsgProc(0x445, 0, 0);
                }

                if (g_ServerConfig.protocol > -1 && g_ServerConfig.protocol < 2) {
                    ((VisListBox*)this->vis_connectiondlg->FindChild(2))->SetSelectedIndex(g_ServerConfig.protocol);
                    this->vis_connectiondlg->MsgProc(0x445, 0, 0);
                }
            }
        }
        break;

    case 0x452:
        if (g_CLlDriver.GetProvider() == 2) {
            g_CLlDriver.RecreateDp();
            g_CLlDriver.PrepareForConnect("", nullptr);
        } else {
            if (g_CLlDriver.GetProvider() == 3) {
                g_CLlDriver.RecreateDp();

                CLlAddress* addresses;
                int num;
                g_CLlDriver.EnumAddresses(&addresses, &num);

                g_CLlDriver.PrepareForConnect(this->field_0x3e0.connect_address, nullptr);
            }
        }

        if (this->sessionMode == 3) {
            this->EnterServerMode();
        } else {
            this->field_0x34c = new VisNetDlg(1, 0, 30, 640, 450, &this->net_sessions);
            this->PopUpScreen(this->field_0x34c);
        }
        break;

    case 0x453:
        this->PopUpScreen( new VisNetTcpIp(1, 0, 100, 640, 300, &this->last_ip) );
        if (strstr(afxCurrentWinApp->m_lpCmdLine, "-protocol") != nullptr || g_ServerConfig.protocol > -1) {
            PostMessage(0x445, 0, 0);
        }
        break;

    case 0x454:
        this->PopUpScreen(new VisNetSerialSettings(1, 0, 100, 540, 480, &this->com_settings));
        break;

    case 0x455:
        this->PopUpScreen(new VisNetPhoneBook(1, 0, 100, 650, 400, &this->phone_book));
        break;

    case 0x457:
        if (this->sessionMode != 3) {
            if (this->sessionMode == 0) {
                INT_00660f8c = 0;
                this->enter_ack_received = 0;
                this->vis_map_context->FUN_0041cb21();

                while (this->enter_ack_received == 0) {
                    if (g_NetStru1_local.GetClientsPktNum() == 0) {
                        uint32_t ticks = GetTickCount();
                        if (abs((int32_t)(ticks - g_RemoteTimestamp)) > 1000) {
                            g_NetStru1_local.SendPacket_64(1, 0);
                            g_RemoteTimestamp = ticks;
                        }
                    } else {
                        this->vis_map_context->ProcessPackets(0x64);
                    }

                    MSG msg;
                    if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0) {
                        if (msg.message == WM_QUIT) {
                            return 0;
                        }

                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                    }

                    g_mousept.Update();

                    if (GetAsyncKeyState(VK_ESCAPE) < 0) {
                        INT_00660f8c = 0x100c;
                        PostMessage(0x45c, 0, 0);
                        return 0;
                    }

                    if (INT_00660f8c) {
                        this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0));
                        PostMessage(0x45c, 0, 0);
                        break;
                    }
                }
            }

            if (this->m_GameSession.SubmitCharacterSetupAndWaitForSelectedUnit() == 0) {
                if ((INT_00660f8c & 0xff) < 11) {
                    this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_main.GetLine(192 + (INT_00660f8c & 0xff)), nullptr, 0));
                } else {
                    this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0));
                }
                PostMessage(0x45c, 0, 0);
                break;
            }
        }

        if (this->RunSessionBootstrap(0) == 0) {
            if ((INT_00660f8c & 0xff) < 11) {
                this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_main.GetLine(192 + (INT_00660f8c & 0xff)), nullptr, 0));
            } else {
                this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0));
            }
            PostMessage(0x45c, 0, 0);
        }
        break;
        
    case 0x459: {
        char* token = strstr(afxCurrentWinApp->m_lpCmdLine, "-map\"");
        if (!token) {
            if (g_ServerConfig.map_names.GetSize() == 0) {
                this->vis_netmapsel = new VisNetMapSelection(1, 0, -64, 640, 480, &this->current_map_name);
                this->PopUpScreen(this->vis_netmapsel);
            } else {
                this->current_map_name = g_ServerConfig.map_names[g_ServerConfig.current_map_index];
                PostMessage(0x457, 0, 0);
            }
        } else {
            this->current_map_name = token + 5;
            int nidx = this->current_map_name.Find('"');
            if (nidx != 0) {
                this->current_map_name = this->current_map_name.Left(nidx);
            }

            PostMessage(0x457, 0, 0);
        }
    }
        break;


    case 0x45a: {
        CVisualObject* local_96c = this->vis_map_context->FindChild(16);
        if (!local_96c) {
            vis_root->MsgProc(message, wParam, lParam);
        } else {
            this->vis_map_context->RemoveChild(local_96c);
            delete local_96c;
        }
    }
        break;

    case 0x45b:
        if (g_settings.TipsMode != 0) {
            CString tip;
            MissionGetTips(wParam, &tip);

            CVisualObject* local_960 = this->vis_map_context->FindChild(16);
            if (local_960) {
                this->vis_map_context->RemoveChild(local_960);
                delete local_960;
            }

            this->vis_map_context->AddChild(new VisTipsDialog(16, 10, 20, 370, 188, tip));
        }
        break;

    case 0x45c:
        if (this->field_0x3e0.field_08.IsEmpty() == 0) {
            PostMessage(WM_CLOSE, 0, 0);
        } else if (this->sessionMode == 0 || this->sessionMode == 1) {
            if (this->dialogsMask & 8) {
                vis_root->MsgProc(0x446, 0, 0);
            }

            if (this->dialogsMask & 2) {
                vis_root->MsgProc(0x445, 0, 0);
            }

            if (this->dialogsMask & 4) {
                vis_root->MsgProc(0x445, 0, 0);
            }
            
            this->vis_map_context->FUN_0041d2da(1);

            if (this->dialogsMask & 1) {
                this->UnloadMapAssets();
            }
            
            this->DestroyServer();

            if (g_CLlDriver.GetProvider() == 0) {
                PostMessage(0x454, 0, 0);
            } else if (g_CLlDriver.GetProvider() == 1) {
                PostMessage(0x455, 0, 0);
            } else {
                PostMessage(0x452, 0, 0);
            }
        } else if (this->sessionMode == 3) {
            if (g_CLlDriver.GetProvider() == 3) {
                PostMessage(0x453, 0, 0);
                this->DestroyServer();
            } else {
                PostMessage(0x451, 0, 0);
                this->DestroyServer();
            }
        }
        break;

    case 0x45d:
        if (this->sessionMode == 0) {
            this->vis_map_context->FUN_0041d2da(0);
            if (this->dialogsMask & 1) {
                this->UnloadMapAssets();
            }
            PostMessage(0x459, 0, 0);
        }
        break;

    case 0x463:
        if (this->dialogsMask == 1) {
            this->ShowFameHallDocDlg();
        }
        break;

    case 0x464: {
        VisMessageBoxWithList* msgbox = new VisMessageBoxWithList(1, 100, 100, 540, 480, txt_main.GetLine(151), nullptr, 1);
        this->ModalScreen(msgbox);

        if (msgbox->GetCloseCode() == 0x445) {
            //DAT_00665cf8 = 0x35;
            PostMessage(0x421, 0, 0);
        } else {
            PostMessage(WM_CLOSE, 0, 0);
        }
    }
        break;

    case 0x465: {
        VisMessageBoxWithList* msgbox = new VisMessageBoxWithList(1, 100, 100, 540, 480, txt_main.GetLine(181), nullptr, 1);
        this->ModalScreen(msgbox);

        if (msgbox->GetCloseCode() == 0x445) {
            PostMessage(0x421, 0, 0);
        } else {
            PostMessage(WM_CLOSE, 0, 0);
        }
    }
        break;

    case 0x466: {
        CVisualObject* obj = this->vis_map_context->FindChild(18);
        if (obj) {
            this->vis_map_context->RemoveChild(obj);
            delete obj;
        } else {
            this->vis_map_context->AddChild(new VisQuestStatus(18, 10, 20, 370, 188));
        }
        this->vis_map_context->field_0xe0 = 1;
    }
        break;

    case 0x468:
        if (ScenarioGetCurrentLocation()->GetKind() == 1) {
            CDWordArray local_95c;
            for (int i = 0; i < 20; i++) {
                if (ScenarioGetVar(0x200 + i) != 0 && ScenarioGetVar(0x214 + i) != 0) {
                    local_95c.Add(i + 1);
                }
            }
            if (local_95c.GetSize() != 0) {
                this->SpawnScenarioMissionUnits(ScenarioGetVar(0x300), local_95c);
                this->vis_map_context->ProcessPackets(0);
            }

            this->RunSessionBootstrap(0);
        } else {
            this->vis_map_context->FUN_0041b064(0, 0);
            this->ShowCurrentTownDialog();
        }
        break;

    case 0x472:
        if (g_IsServer == 0) {
            if (this->sessionMode == 3) {
                this->vis_map_context->msglog.Add((const char*)wParam, clrsh_ShockingBlack, 30000);
                free((void*)wParam);
            }
        } else {
            char buf[2048];
            OemToCharA((const char*)wParam, buf);
            this->list_box1.SetCurSel(this->list_box1.AddString(buf));
        }
        break;

    case 0x47f:
        this->EnterClientMode();
        break;

    case 0x482:
        this->EnterClientMode();
        break;

    case 0x486:
        PostMessage(WM_CLOSE, 0, 0);
        break;

    case 0x487:
        this->PopUpScreen(new VisHatServerListDlg(1, 40, 100, 600, 480, &this->hat_settings));
        break;

    case 0x488:
        if (this->RunModalLoop() == 0) {
            PostMessage(0x422, 0, 0);
        } else {
            PostMessage(0x426, 0, 0);
        }
        break;

    case 0x489: {
        this->vis_hatbrowser = new VisHatBrowserDlg(1, 0, -64, 640, 480);
        this->PopUpScreen(this->vis_hatbrowser);

        int32_t res;
        if (this->hat_settings.ishat == 0) {
            res = this->CheckWebVersion(this->hat_settings.hatip);
        } else {
            res = this->LeaveServerMode();
            if (res == 0) {
                g_CLlDriver.Close();
                PostMessage(0x487, 0, 0);
                break;
            }
        }
        PostMessage(0x48a, res, 0);
    }
        break;
    }

    return CWnd::WindowProc(message, wParam, lParam);
}


void MainWindow::PopUpScreen(VisScreen* screen) { //48d26a
    if (screen == this->vis_msgbox_list) {
        this->dialogsMask |= 0x8000;
    } else {
        this->dialogsMask |= 8;
    }

    this->vis_root->AddChild(screen);

    screen->VMethod26();
    screen->VMethod28();

    LockSurface2();

    ShadowRect(g_ScreenSize, 3);

    UnlockSurface2();

    FlushScreen();

    screen->VMethod9();

    this->field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();

    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();

        this->ApplyCursorClip();
    }
}

void MainWindow::ShowGlobalMapDialog() { //48d34b
    g_Cursors[CURSOR_WAIT]->Use();

    this->vis_root->AddChild(this->vis_globalmap);
    this->vis_globalmap->VMethod28();
    this->vis_root->VMethod9();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x10;

    if (g_SoundSettings.music_allowed != 0) {
        CStringArray playlist;
        playlist.Add("music\\map.wav");

        this->music_player->SetPlayList(playlist);
        this->music_player->Play();
    }

    g_Cursors[CURSOR_DEFAULT]->Use();
}

void MainWindow::BeginLocalCharacterSetup() { // 48f79d
    g_mousept.DisableHint();

    strcpy(this->m_GameSession.character_name, "Self");
    this->field_0x400 = 1;
    this->sessionMode = 2;

    this->StartServerSession(this->sessionMode);

    g_Server->LoadSavedGame(this->last_save_info.filename);

    if (!this->vis_map_context->ConnectAndJoinSession()) {
        PostMessage(0x421, 0, 0);
    } else if (this->GetSaveFileInBattle()) {
        this->RunSessionBootstrap(1);
    } else {
        this->vis_map_context->FUN_0041b10f();
        g_Server->PumpNetwork();

        while (g_NetStru1_local.GetClientsPktNum()) {
            this->vis_map_context->ProcessPackets(0x64);
        }

        this->LoadSavegame();
        this->vis_map_context->FUN_0041b064(0, 0);
        PostMessage(0x42e, 0, 0);
    }
    g_mousept.EnableHint();
    this->vis_globalmap->RebuildScenarioLocations();
}

int MainWindow::GetSaveFileInBattle() { // 48de6f
    File2 f;
    f.Open(this->last_save_info.filename, CFile::modeRead);
    f.Seek(4, 0);

    int32_t offset;
    f.Read(&offset, 4);
    f.Seek(offset + 0x100, 0);

    RegFile reg;
    reg.ReadFromFile(&f);
    f.Close();

    return reg.GetInt("CurrentState", "InBattle", 1);
}

int MainWindow::RunSessionBootstrap(int mode) { // 48e502

    this->vis_map_context->field_0x80 = nullptr;
    g_Cursors[CURSOR_WAIT]->Use();

    if (g_SoundSettings.music_allowed != 0) {
        this->music_player->OnEndTrack();
    }

    if (this->sessionMode == 3) {
        CRect r = g_ScreenSize;
        r.bottom -= 72;

        this->vis_map_context->msglog.SetRect(r);

        LockSurface2();
        FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
        UnlockSurface2();

        g_Server->field41_0x1b0 = 1;

        this->vis_serverscreen = new VisServerScreen(1, 0, 0, g_ScreenSize.right, g_ScreenSize.bottom, &this->vis_map_context->msglog);
        this->PopUpScreen(this->vis_serverscreen);
    }

    char buf[128];
    if (this->sessionMode == 2 && mode == 0) {
        sprintf(buf, "%d.alm", ScenarioGetCurrentLocation()->GetId());
    } else {
        strcpy(buf, this->current_map_name);
    }

    if (mode == 0 && this->serverBootstrapEnabled != 0) {
        g_Server->field40_0x1ac = 0;
        g_Server->LoadGame(buf);
    }

    if (this->sessionMode != 3) {
        this->ShowMainRightPanel();
        this->LoadObjectGfx();
        this->LoadStructureGfx();
        this->LoadProjectileGfx();

        this->pending_game_mode = mode;

        this->vis_map_context->FUN_0041cad0(mode);
        this->vis_map_context->FUN_0041aaaa(this->vis_map_context->wimpy);
        this->vis_map_context->FUN_0041abd2(this->vis_map_context->formation);
        this->vis_map_context->FUN_0041ab74();

        if (this->serverBootstrapEnabled != 0) {
            g_Server->ServerTic();
        }

        uint32_t tm = timeGetTime();
        while (true) {
            if (this->vis_map_context->field_0x80) {
                this->pending_game_mode = 0;
                break;
            }

            MSG msg;
            if (PeekMessageA(&msg, nullptr, 0, 0, 1)) {
                if (msg.message == WM_QUIT) {
                    return 0;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            while (g_NetStru1_local.GetClientsPktNum() == 0) {
                if (timeGetTime() - tm > g_CmdTimeout) {
                    INT_00660f8c = 0x1005;
                    return 0;
                }
                g_NetStru1_local.ProcessConnections();
                g_mousept.Update();
            }

            if (this->vis_map_context->ProcessPackets(0x64) == 0) {
                return 0;
            }
        }
    }

    if (this->sessionMode == 2 && mode != 0) {
        File2 fil;
        fil.Open(this->last_save_info.filename, CFile::modeRead);
        fil.Seek(4, 0);
        int32_t offset;
        fil.Read(&offset, 4);
        fil.Seek(offset + 0x100, 0);

        RegFile reg;

        reg.ReadFromFile(&fil);
        this->m_FameHall.LoadDocuments(&fil);
        ScenarioLoad(&fil);

        for (int i = 0; i < 9; i++) {
            this->m_GameSession.shortcuts[i].ReadFromFile(&fil);
        }

        fil.Close();

        int inbattle = reg.GetInt("CurrentState", "InBattle", 1);

        reg.GetSizedString("Character", "Name", "No name", this->m_GameSession.character_name, sizeof(this->m_GameSession.character_name));
        this->m_GameSession.character_name[31] = 0;

        this->vis_map_context->wimpy = reg.GetInt("GameOptions", "Wimpy", this->vis_map_context->wimpy);
        this->vis_map_context->show_hp = reg.GetInt("GameOptions", "ShowHP", this->vis_map_context->show_hp);
        this->vis_map_context->flying_hp = reg.GetInt("GameOptions", "FlyingHP", this->vis_map_context->flying_hp);
        this->vis_map_context->formation = reg.GetInt("GameOptions", "Formation", this->vis_map_context->formation);
        this->game_speed = reg.GetInt("GameOptions", "Speed", this->game_speed);
        g_settings.ShowTimeFlow = reg.GetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);
        this->vis_map_context->view_x = reg.GetInt("View", "X", this->vis_map_context->view_x);
        this->vis_map_context->view_y = reg.GetInt("View", "Y", this->vis_map_context->view_y);

        if (reg.GetInt("SpellBook", "IsOpen", 0) == 0) {
            if (this->vis_map_context->IsBagOpen() != 0) {
                this->vis_map_context->FUN_0041b636();
            }
        } else {
            if (this->vis_map_context->IsBagOpen() == 0) {
                this->vis_map_context->FUN_0041b509();
            }
        }

        this->vis_spellbook->pressed = reg.GetInt("SpellBook", "Pressed", this->vis_spellbook->pressed);

        CArray<uint16_t> sel_ids;
        CArray<uint16_t> grp_ids;

        reg.GetInt16Array("Objects", "Selection", &sel_ids);

        for (int i = 0; i < sel_ids.GetSize(); i++) {
            CGameObject* obj;
            if (this->vis_map_context->field_0x9d0.Lookup(sel_ids[i], obj)) {
                obj->SetSelected(1);
            }
        }

        for (int i = 0; i < 10; i++) {
            sprintf(buf, "Group%d", i);

            grp_ids.RemoveAll();
            reg.GetInt16Array("Objects", buf, &grp_ids);

            for (int j = 0; j < grp_ids.GetSize(); j++) {
                CGameObject* obj;
                if (this->vis_map_context->field_0x9d0.Lookup(grp_ids[i], obj) != 0) {
                    obj->AddControlGroup(i);
                }
            }
        }

        if (reg.GetInt("Inventory", "IsOpen", 0) == 0) {
            if (this->vis_map_context->FUN_0041b495() != 0) {
                this->vis_map_context->FUN_0041b40e();
            }
        } else {
            if (this->vis_map_context->FUN_0041b495() == 0) {
                this->vis_map_context->FUN_0041b381();
            }
        }

        CArray<uint16_t> proj_ids;

        this->vis_map_context->field_0xa24 = reg.GetInt("Projectiles", "FreeIndex", 0);
        reg.GetInt16Array("Projectiles", "IDs", &proj_ids);

        for (int i = 0; i < proj_ids.GetSize(); i++) {
            CProjectile* proj = new CProjectile();

            sprintf(buf, "Prj%d", proj_ids[i]);
            proj->x_pos = reg.GetInt(buf, "x", proj->x_pos);
            proj->y_pos = reg.GetInt(buf, "y", proj->y_pos);
            proj->z_pos = reg.GetInt(buf, "z", proj->z_pos);
            proj->typeId = reg.GetInt(buf, "picture", proj->typeId);
            proj->dir = reg.GetInt(buf, "dir", proj->dir);
            proj->phase = reg.GetInt(buf, "phase", proj->phase);
            proj->last_action = reg.GetInt(buf, "lastaction", proj->last_action);
            proj->action = reg.GetInt(buf, "action", proj->action);
            proj->action_dir = reg.GetInt(buf, "actiondir", proj->action_dir);
            proj->action_target = reg.GetInt(buf, "actiontarget", proj->action_target);
            proj->action_x = reg.GetInt(buf, "actionx", proj->action_x);
            proj->action_y = reg.GetInt(buf, "actiony", proj->action_y);
            proj->action_z = reg.GetInt(buf, "actionz", proj->action_z);
            proj->action_phase = reg.GetInt(buf, "actionphase", proj->action_phase);
            proj->action_segments = reg.GetInt(buf, "actionsegments", proj->action_segments);
            proj->action_spell = reg.GetInt(buf, "actionspell", proj->action_spell);
            proj->pMapObject = this->vis_map_context;
            proj->map_player = this->vis_map_context->my_main_unit;

            proj->unit_id = proj_ids[i];
            this->vis_map_context->field_0x9ec[proj->unit_id] = proj;
            proj->UpdateMapPosition();
        }

        CArray<uint32_t> fog_data;
        if (inbattle != 0) {
            uint16_t* land = this->vis_map_context->field_0x80->GetLandscape();
            uint32_t st = reg.GetInt("Fog", "FirstState", 0);
            reg.GetInt32Array("Fog", "Data", &fog_data);

            for (int i = 0; i < fog_data.GetSize(); i++) {
                for(int j = 0; j < fog_data[i]; j++) {
                    *land |= st;
                    land++;
                }
                st ^= 0x8000;
            }
        }
    }

    this->vis_right_panel->MsgProc(0x408, 0, 0);
    this->vis_root->VMethod9();

    if (this->sessionMode != 3) {
        this->PlayDefaultMusic();
        this->last_event_id = -1;
        this->in_famehall_mission = 0;
        this->mission_failed_reported = 0;
    }

    if (this->sessionMode == 2) {
        this->SetSpeed(this->game_speed);
    } else {
        this->SetSpeed(g_ServerConfig.game_speed);
    }

    if (this->sessionMode == 2) {
        sprintf(buf, "main\\text\\mission%d.txt", ScenarioGetCurrentLocation()->GetId());
        ReadFileToString(buf, &g_MissionText);
        MissionGetBriefing(&g_MissionBriefing);

        g_MissionFailures.RemoveAll();

        CString tmp;
        for (int i = 2; ; i++) {
            MissionGetFailure(i, &tmp);
            if (tmp.IsEmpty()) {
                if (i > 4) {
                    break;
                }

                tmp = txt_main.GetLine(0x118 + i);
            }
            g_MissionFailures.Add(tmp);
        }

        g_MissionSubjs.RemoveAll();
        for (int i = 0; ; i++) {
            MissionGetSubj(i, &tmp);
            if (tmp.IsEmpty()) {
                break;
            }
            g_MissionSubjs.Add(tmp);
        }

        if (mode == 0) {
            PostMessage(0x442, 0, 0);
        }
    } else {
        ReadFileToString("main\\text\\quest.txt", &g_MissionText);
    }

    g_Cursors[CURSOR_DEFAULT]->Use();
    return 1;
}

void MainWindow::ShowMainRightPanel() { // 485969
    this->vis_right_panel->RemoveAllChilds();
    this->vis_right_panel->AddChild(this->vis_minimap);
    this->vis_right_panel->AddChild(this->vis_ordertoolbar);
    this->vis_right_panel->AddChild(this->vis_charinfo);
    this->vis_right_panel->AddChild(this->vis_sidestatus);

    this->vis_root->RemoveAllChilds();
    this->vis_map_context->msglog.Clear();
    this->vis_root->AddChild(this->vis_map_context);
    this->vis_root->AddChild(this->vis_right_panel);

    this->dialogsMask = 1;
}

void MainWindow::LoadObjectGfx() { // 47a5e6
    RegFile reg("graphics\\objects\\objects.reg");
    int count = reg.GetInt("Global", "FileCount", 0);
    int obj_count = reg.GetInt("Global", "ObjectCount", 0);

    for (int i = 0; i < count; i++) {
        char buf[32];
        sprintf(buf, "File%d", i);

        char flname[256];
        reg.GetSizedString("Files", buf, "", flname, sizeof(flname));

        g_GfxFiles.Add(new GfxFile(flname));
    }
    
    char buf[64];
    char parnt[64];
    for (int i = 0; i < obj_count; i++) {
        g_mousept.Update();
        
        sprintf(buf, "Object%d", i);

        GfxObject* obj = new GfxObject();

        obj->id = reg.GetInt(buf, "ID", -1);
        obj->sn = i;

        int parentID = reg.GetInt(buf, "Parent", -1);
        if (parentID != -1) {
            sprintf(parnt, "Object%d", g_GfxObjects[parentID]->sn);
        }

        obj->file = reg.GetInt(buf, "File", -1);

        if (parentID == -1) {
            obj->index = reg.GetInt(buf, "Index", -1);
        } else {
            obj->index = reg.GetInt(buf, "Index", g_GfxObjects[parentID]->index);
        }

        if (parentID == -1) {
            obj->phases = reg.GetInt(buf, "Phases", -1);
        } else {
            obj->phases = reg.GetInt(buf, "Phases", g_GfxObjects[parentID]->phases);
        }

        if (parentID == -1) {
            obj->width = reg.GetInt(buf, "Width", -1);
        } else {
            obj->width = reg.GetInt(buf, "Width", g_GfxObjects[parentID]->width);
        }

        if (parentID == -1) {
            obj->height = reg.GetInt(buf, "Height", -1);
        } else {
            obj->height = reg.GetInt(buf, "Height", g_GfxObjects[parentID]->height);
        }

        if (parentID == -1) {
            obj->center_x = reg.GetInt(buf, "CenterX", -1);
        } else {
            obj->center_x = reg.GetInt(buf, "CenterX", g_GfxObjects[parentID]->center_x);
        }

        if (parentID == -1) {
            obj->center_y = reg.GetInt(buf, "CenterY", -1);
        } else {
            obj->center_y = reg.GetInt(buf, "CenterY", g_GfxObjects[parentID]->center_y);
        }

        if (parentID == -1) {
            obj->fire_object = reg.GetInt(buf, "FireObject", -1);
        } else {
            obj->fire_object = reg.GetInt(buf, "FireObject", g_GfxObjects[parentID]->fire_object);
        }

        if (parentID == -1) {
            obj->dead_object = reg.GetInt(buf, "DeadObject", -1);
        } else {
            obj->dead_object = reg.GetInt(buf, "DeadObject", g_GfxObjects[parentID]->dead_object);
        }

        obj->in_map_editor = reg.GetInt(buf, "InMapEditor", 0);

        memset(obj->desc_text, 0, 32);

        reg.GetSizedString(buf, "DescText", " ", obj->desc_text, 31);

        CArray<uint32_t> animationTime;
        CArray<uint32_t> animationFrame;

        reg.GetInt32Array(buf, "AnimationTime", &animationTime);
        if (parentID != -1) {
            if (animationTime.GetSize() == 0) {
                reg.GetInt32Array(parnt, "AnimationTime", &animationTime);
            }
        }

        reg.GetInt32Array(buf, "AnimationFrame", &animationFrame);
        if (parentID != -1) {
            if (animationFrame.GetSize() == 0) {
                reg.GetInt32Array(parnt, "AnimationFrame", &animationFrame);
            }
        }

        for (int j = 0; j < animationTime.GetSize() && j < animationFrame.GetSize(); j++) {
            for (int frm = 0; frm < animationTime[j]; frm++) {
                obj->frames.Add(animationFrame[j]);
            }
        }

        obj->frame_count = obj->frames.GetSize();
        
        g_GfxObjects.Add(obj);
    }
}

void MainWindow::LoadStructureGfx() { //47efdf
    RegFile reg("graphics\\structures\\structures.reg");

    char buf[64];
    char fil[256];

    int count = reg.GetInt("Global", "Count", 0);
    for (int i = 0; i < count; i++) {
        g_mousept.Update();

        sprintf(buf, "Structure%d", i);

        reg.GetSizedString(buf, "File", "", fil, sizeof(fil));

        StructureInfo* bld = new StructureInfo(fil);

        bld->id = reg.GetInt(buf, "ID", -1);
        bld->tile_width = reg.GetInt(buf, "TileWidth", -1);
        bld->tile_height = reg.GetInt(buf, "TileHeight", -1);
        bld->full_height = reg.GetInt(buf, "FullHeight", -1);
        bld->phases = reg.GetInt(buf, "Phases", -1);
        bld->selection.left = reg.GetInt(buf, "SelectionX1", -1);
        bld->selection.top = reg.GetInt(buf, "SelectionY1", -1);
        bld->selection.right = reg.GetInt(buf, "SelectionX2", -1);
        bld->selection.bottom = reg.GetInt(buf, "SelectionY2", -1);
        bld->shadow_y = reg.GetInt(buf, "ShadowY", 0);
        bld->anim_mask = nullptr;
        bld->frame_count = 0;
        bld->anim_frame_stride = 0;

        reg.GetSizedString(buf, "Picture", "", bld->picture, sizeof(bld->picture));

        bld->indestructible = reg.GetInt(buf, "Indestructible", 0);

        reg.GetSizedString(buf, "DescText", "", bld->desc_text, sizeof(bld->desc_text));

        bld->variable_size = reg.GetInt(buf, "VariableSize", 0);
        bld->usable = reg.GetInt(buf, "Usable", 0);
        bld->light_radius = reg.GetInt(buf, "LightRadius", 0);
        bld->light_pulse = reg.GetInt(buf, "LightPulse", 0);
        bld->flat = reg.GetInt(buf, "Flat", 0);

        if (bld->phases > 1) {
            CArray<uint32_t> anim_time;
            CArray<uint32_t> anim_frame;
            reg.GetInt32Array(buf, "AnimTime", &anim_time);
            reg.GetInt32Array(buf, "AnimFrame", &anim_frame);

            for(int j = 0; j < anim_time.GetSize() && j < anim_frame.GetSize(); j++) {
                for (int frm = 0; frm < anim_time[j]; frm++) {
                    bld->frames.Add(anim_frame[j]);
                }
            }

            bld->frame_count = bld->frames.GetSize();

            bld->anim_mask = new char[1 + bld->tile_width * bld->full_height];
            reg.GetSizedString(buf, "AnimMask", "", bld->anim_mask, bld->tile_width * bld->full_height + 1);

            int sz = strlen(bld->anim_mask);
            for (int j = 0; j < sz; j++) {
                if (bld->anim_mask[j] != '-') {
                    bld->anim_frame_stride++;
                }
            }
        }
        g_StructuresInfo.SetAtGrow(bld->id, bld);
    }
}

void MainWindow::LoadProjectileGfx() { //47e612

    RegFile reg("graphics\\projectiles\\projectiles.reg");
    int count = reg.GetInt("Global", "Count", 0);

    for (int i = 0; i < count; i++) {
        g_mousept.Update();

        char buf[256];
        sprintf(buf, "Projectile%d", i);

        char fil[256];
        reg.GetSizedString(buf, "File", "", fil, sizeof(fil));

        ProjectileInfo* proj = new ProjectileInfo(fil, reg.GetInt(buf, "A16", 0));

        proj->id = reg.GetInt(buf, "ID", -1);
        proj->phases = reg.GetInt(buf, "Phases", -1);
        proj->rotation_phases = reg.GetInt(buf, "RotationPhases", 0x10);
        proj->width = reg.GetInt(buf, "Width", 0x40);
        proj->height = reg.GetInt(buf, "Height", 0x40);
        proj->palette = reg.GetInt(buf, "Palette", 0);
        proj->homing = reg.GetInt(buf, "Homing", 0);
        proj->flip = reg.GetInt(buf, "Flip", 0);
        proj->sfx = reg.GetInt(buf, "SFX", 0);

        g_ProjectileInfos.SetAtGrow(proj->id, proj);
    }

    File2 fl;
    
    fl.Open("graphics\\projectiles\\projectiles.pal", CFile::modeRead);
    fl.Seek(0x36, 0);

    RGBQUAD colors[256];
    fl.Read(colors, sizeof(colors));

    g_pal_projectiles = new CGamePalette();
    g_pal_projectiles->SetPalette(colors, 16, 2, 0);
    fl.Close();

    fl.Open("graphics\\projectiles\\projectile_.pal", CFile::modeRead);
    fl.Seek(0x36, 0);
    fl.Read(colors, sizeof(colors));

    g_pal_projectile_ = new CGamePalette();
    g_pal_projectile_->SetPalette(colors, 16, 2, 0);
    fl.Close();

    for (int i = 0; i < 2; i++) {
        char sbuf[128];
        sprintf(sbuf, "graphics\\projectiles\\smoke%d\\sprites.16a", i);

        g_spr_smoke[i] = new CA16(sbuf);
        g_spr_smoke[i]->ResetPalette(16, 4, 0);
    }
}


void MainWindow::LoadSavegame() { //48df44
    File2 fil;

    fil.Open(this->last_save_info.filename, CFile::modeRead);
    fil.Seek(4, 0);

    int32_t offset;
    fil.Read(&offset, 4);
    fil.Seek(offset + 0x100, 0);

    RegFile reg;

    reg.ReadFromFile(&fil);

    this->m_FameHall.LoadDocuments(&fil);

    ScenarioLoad(&fil);

    for (int i = 0; i < 9; i++) {
        this->m_GameSession.shortcuts[i].ReadFromFile(&fil);
    }

    fil.Close();

    int local_128 = reg.GetInt("CurrentState", "InBattle", 1);

    reg.GetSizedString("Character", "Name", "No name", this->m_GameSession.character_name, sizeof(this->m_GameSession.character_name));
    this->m_GameSession.character_name[31] = 0;

    this->vis_map_context->wimpy = reg.GetInt("GameOptions", "Wimpy", this->vis_map_context->wimpy);
    this->vis_map_context->show_hp = reg.GetInt("GameOptions", "ShowHP", this->vis_map_context->show_hp);
    this->vis_map_context->flying_hp = reg.GetInt("GameOptions", "FlyingHP", this->vis_map_context->flying_hp);
    this->vis_map_context->formation = reg.GetInt("GameOptions", "Formation", this->vis_map_context->formation);

    this->game_speed = reg.GetInt("GameOptions", "Speed", this->game_speed);
    g_settings.ShowTimeFlow = reg.GetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);

    this->vis_map_context->view_x = reg.GetInt("View", "X", this->vis_map_context->view_x);
    this->vis_map_context->view_y = reg.GetInt("View", "Y", this->vis_map_context->view_y);

    reg.GetInt("SpellBook", "IsOpen", 0);

    this->vis_spellbook->pressed = reg.GetInt("SpellBook", "Pressed", this->vis_spellbook->pressed);

    CArray<uint16_t> sel_objs;
    CArray<uint16_t> grp_objs;

    reg.GetInt16Array("Objects", "Selection", &sel_objs);

    for (int i = 0; i < sel_objs.GetSize(); i++) {
        CGameObject* obj = nullptr;
        if (this->vis_map_context->field_0x9d0.Lookup(sel_objs[i], obj) && obj) {
            obj->SetSelected(1);
        }
    }

    for (int i = 0; i < 10; i++) {
        char buf[256];
        sprintf(buf, "Group%d", i);
        
        grp_objs.RemoveAll();
        reg.GetInt16Array("Objects", buf, &grp_objs);

        for(int j = 0; j < grp_objs.GetSize(); j++) {
            CGameObject* obj = nullptr;
            if (this->vis_map_context->field_0x9d0.Lookup(grp_objs[j], obj)) {
                obj->AddControlGroup(i);
            }
        }
    }

    this->vis_map_context->UpdateSelectionState();

    this->last_event_id = -1;
    this->in_famehall_mission = 0;
    this->mission_failed_reported = 0;

    this->SetSpeed(this->game_speed);
}



void MainWindow::UnloadMapAssets() { //48f6f7

    this->vis_map_context->FUN_0041c39c();
    if (g_SoundSettings.music_allowed != 0) {
        this->music_player->OnEndTrack();
    }

    this->FUN_0047ad28();

    for (int i = 0; i < g_UnitGfxFiles.GetSize(); i++) {
        g_UnitGfxFiles[i]->Deinit();
    }
    
    this->FUN_00478130();
    this->UnloadProjectileInfos();
    this->UnloadStructureInfos();

    this->dialogsMask &= ~1;
}

void MainWindow::UnloadProjectileInfos() { //47eab6
    for (int i = 0; i < g_ProjectileInfos.GetSize(); i++) {
        ProjectileInfo* inf = g_ProjectileInfos[i];
        if (inf) {
            delete inf;
        }
    }
    g_ProjectileInfos.RemoveAll();

    if (g_pal_projectiles) {
        delete g_pal_projectiles;
        g_pal_projectiles = nullptr;
    }

    if (g_pal_projectile_) {
        delete g_pal_projectile_;
        g_pal_projectile_ = nullptr;
    }

    for (int i = 0; i < 2; i += 1) {
        if (g_spr_smoke[i]) {
            delete g_spr_smoke[i];
            g_spr_smoke[i] = nullptr;
        }
    }
}

void MainWindow::UnloadStructureInfos() { //47f5e4
    for (int i = 0; i < g_StructuresInfo.GetSize(); i++) {
        StructureInfo* inf = g_StructuresInfo[i];
        if (inf) {
            delete inf;
        }
    }
    g_StructuresInfo.RemoveAll();
}


void MainWindow::FUN_0047ad28() { //47ad28
    for (int i = 0; i < g_GfxFiles.GetSize(); i++) {
        delete g_GfxFiles[i];
    }
    for (int i = 0; i < g_GfxObjects.GetSize(); i++) {
        delete g_GfxObjects[i];
    }
    g_GfxFiles.RemoveAll();
    g_GfxObjects.RemoveAll();
}


void MainWindow::InitNewCampaignSession() { //48dd3a
    this->field_0x400 = 1;
    this->sessionMode = 2;

    if (this->StartServerSession(this->sessionMode) == 0) {
        this->m_GameSession.type = 0;
        if (strstr(afxCurrentWinApp->m_lpCmdLine, "-female") != nullptr) {
            this->m_GameSession.type |= 0x80;
        }
        if (strstr(afxCurrentWinApp->m_lpCmdLine, "-mage") != nullptr) {
            this->m_GameSession.type |= 0x40;
        }
        const char* nm = strstr(afxCurrentWinApp->m_lpCmdLine, "-name");

        if (!nm) {
            strcpy(this->m_GameSession.character_name, "Unnamed");
        } else {
            strncpy(this->m_GameSession.character_name, nm + 5, 32 - 1);
            this->m_GameSession.type |= 0x80;
            char* nm_sp = strstr(this->m_GameSession.character_name, " ");
            if (nm_sp) {
                *nm_sp = 0;
            }
        }
    } else {
        AfxMessageBox("Suxx");
        abort();
    }
}


void MainWindow::ShowStartGameSetupForNewSession() { //491b3e
    g_Cursors[CURSOR_WAIT]->Use();

    this->vis_right_panel->MsgProc(0x403, (uint32_t)this->vis_map_context, 0); //FIXME

    this->vis_root->AddChild(this->vis_startgame);

    if (this->sessionMode != 2) {
        this->m_GameSession.character_name[0] = '\0';
    }

    this->vis_startgame->SetCharName(this->m_GameSession.character_name);

    this->vis_startgame->SetSelectedDifficulty(this->m_FameHall.GetDifficulty());
    this->vis_startgame->SetSelectedPortraitPlayerType(this->m_GameSession.type >> 6);
    this->vis_startgame->VMethod28();
    this->vis_root->VMethod9();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x800;

    g_Cursors[CURSOR_DEFAULT]->Use();

    if (g_SoundSettings.music_allowed) {
        CStringArray lst;
        lst.Add("music\\chrgen.wav");

        CStringArray& playing = this->music_player->GetPlaylist();
        if (this->music_player->GetPlaylistSize() == 0 || this->music_player->GetPlaylist()[0] != lst[0]) {
            this->music_player->SetPlayList(lst);
        }

        this->music_player->Play();
    }
}


void MainWindow::ShowStartupLogoDialog() { // 48dba0
    this->vis_root->AddChild(this->vis_logownd);
    this->vis_logownd->VMethod28();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x100;
}


void MainWindow::ShowCreditsDialog() { // 48da66
    this->vis_root->AddChild(this->vis_credits);
    this->vis_credits->VMethod28();
    this->vis_root->VMethod9();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x40;

    if (g_SoundSettings.music_allowed != 0) {
        CStringArray playlist;
        playlist.Add("music\\credit.wav");

        this->music_player->SetPlayList(playlist);
        this->music_player->Play();
    }
}

void MainWindow::ShowFameHallDialog() { //48d096
    g_Cursors[CURSOR_WAIT]->Use();

    this->vis_root->AddChild(this->vis_famehall);

    this->vis_famehall->VMethod28();

    this->vis_root->VMethod9();

    this->field_0x460 = 0;
    this->dialogsMask |= 0x1000;

    g_Cursors[CURSOR_DEFAULT]->Use();
}


void MainWindow::ShowShopDialog(uint32_t id) { //48cdce
    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
        this->ApplyCursorClip();
    }

    if (this->vis_map_context->IsBookOpen() != 0) {
        this->vis_map_context->FUN_0041b636();
    }

    g_Cursors[CURSOR_WAIT]->Use();

    int locid = 1;
    if (this->sessionMode == 2) {
        locid = ScenarioGetCurrentLocation()->GetId();
    } else {
        switch (id) {
        case 0x22:
        case 0x23:
            locid = 1;
            break;

        case 0x5d:
        case 0x5e:
        case 0x5f:
            locid = 2;
            break;

        case 0x69:
        case 0x6a:
        case 0x6b:
            locid = 3;
            break;
        }
    }

    if (locid == 1) {
        this->vis_root->AddChild(this->vis_shop);
        this->vis_shop->VMethod28();
    } else if (locid == 2) {
        this->vis_root->AddChild(this->vis_shopkaarg);
        this->vis_shopkaarg->VMethod28();
    } else if (locid == 3) {
        this->vis_root->AddChild(this->vis_shopdruid);
        this->vis_shopdruid->VMethod28();
    }

    this->dialogsMask |= 2;
    this->vis_map_context->UpdateSelectionState();
    this->vis_map_context->OnOpenShopDialog();
    this->vis_root->VMethod9();
    this->field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();
}



void MainWindow::ShowInnDialog(uint32_t id, int32_t interact_target) { //48d4da
    if (g_mousept.GetSelectState() != 0) {
        g_mousept.ResetStates();
        this->ApplyCursorClip();
    }

    if (this->vis_map_context->IsBookOpen() != 0) {
        this->vis_map_context->FUN_0041b636();
    }

    g_Cursors[CURSOR_WAIT]->Use();

    int locid = 1;
    if (this->sessionMode == 2) {
        locid = ScenarioGetCurrentLocation()->GetId();
    } else {
        switch (id) {
        case 0x43:
        case 0x44:
        case 0x45:
            locid = 1;
            break;

        case 0x63:
        case 0x64:
        case 0x65:
            locid = 2;
            break;

        case 0x6f:
        case 0x70:
        case 0x71:
            locid = 3;
            break;
        }
    }

    VisTav* inn = nullptr;
    if (locid == 1) {
        inn = this->vis_tav;
    } else if (locid == 2) {
        inn = this->vis_tavkaarg;
    } else if (locid == 3) {
        inn = this->vis_tavdruid;
    }

    this->vis_root->AddChild(inn);

    if (this->sessionMode == 2) {
        int32_t ids[32];
        int32_t num = 0;
        ScenarioEnterInn(ids, &num);

        CDWordArray arr;
        arr.SetSize(num);

        for (int i = 0; i < num; i++) {
            arr[i] = ids[i];
        }

        g_Server->FUN_00501b9e(ScenarioGetVar(0x300), arr);

        this->vis_map_context->ProcessPackets(0);
    } else {
        this->vis_map_context->NetOnOpenInnDialog();
    }

    inn->field_0x138 = interact_target;
    inn->VMethod28();

    this->dialogsMask |= 4;

    this->vis_map_context->UpdateSelectionState();

    this->vis_root->VMethod9();

    this->field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();
}


void MainWindow::RemoteGameIdle() { //48acac
    if (g_NetStru1_local.IsActive() == 0) {
        this->vis_root->MsgProc(0x446, 0, 0);
        PostMessage(0x45c, 0, 0);

        if (g_EnableTrace != 0) {
            CStdioFile f;
            f.Open("error.log", CFile::modeWrite | CFile::modeNoTruncate | CFile::modeCreate);
            f.SeekToEnd();

            CTime tm = CTime::GetCurrentTime();
            f.WriteString(tm.Format("%d.%m.%y %H:%M:%S ") + "Connection lost\n");
            
            f.Close();

            this->vis_map_context->msglog.Add("Connection lost", clrsh_TechBlack, 30000);
        }
    } else {
        g_mousept.Update();

        if (g_NetStru1_local.GetClientsPktNum() == 0) {
            uint32_t tick = GetTickCount();
            if (std::abs((int)(tick - g_RemoteTimestamp)) > 1000) {
                g_NetStru1_local.SendPacket_64(1, 0);
                g_RemoteTimestamp = tick;
            }
        } else {
            while(true) {
                this->game_tic_counter++;
                this->game_tic_counter &= 0xf;

                if (this->game_tic_counter == 0) {
                    g_NetStru1_local.UpdateConnStats();
                }

                this->vis_map_context->ProcessPackets(100);
                this->vis_root->MsgProc(0x401, 0, 0);

                g_mousept.Update();

                if (g_NetStru1_local.GetClientsPktNum() == 0) {
                    break;
                }
            }
            g_RemoteTimestamp = GetTickCount();
        }

        if (g_mousept.GetSelectState() == 0 && this->dialogsMask == 1) {
            if (g_mousept.GetX() <= 0) {
                this->vis_map_context->ScrollMapX(-1);
            }

            if (g_mousept.GetY() <= 0) {
                this->vis_map_context->ScrollMapY(-1);
            }

            if (g_mousept.GetX() >= g_ScreenSize.right - 2) {
                this->vis_map_context->ScrollMapX(1);
            }

            if (g_mousept.GetY() >= g_ScreenSize.bottom - 2) {
                this->vis_map_context->ScrollMapY(1);
            }
        }

        if (this->has_focus != 0) {
            this->vis_root->MsgProc(0x402, 0, 0);
        }

        g_mousept.Update();
    }
}

void MainWindow::SingleGameTimedIdle() { //48a543
    g_mousept.Update();

    uint32_t ticks = timeGetTime();
    while (ticks >= this->last_tic_time + this->game_tic_time * (this->game_tic_counter + 1)) {
        if (this->game_tic_counter == 0) {
            this->last_tic_time = ticks;
        }

        this->game_tic_counter++;
        this->game_tic_counter &= 0xf;

        g_Server->ServerTic();

        this->vis_map_context->ProcessPackets(0x64);
        this->vis_root->MsgProc(0x401, 0, 0);

        g_mousept.Update();

        ticks = timeGetTime();
    }


    if (g_mousept.GetSelectState() == 0 && this->dialogsMask == 1) {
        bool scroll = false;

        if (g_mousept.GetX() <= 0) {
            this->vis_map_context->ScrollMapX(-1);
            scroll = true;
        }


        if (g_mousept.GetY() <= 0) {
            this->vis_map_context->ScrollMapY(-1);
            scroll = true;
        }

        if (g_mousept.GetX() >= g_ScreenSize.right - 2) {
            this->vis_map_context->ScrollMapX(1);
            scroll = true;
        }

        if (g_mousept.GetY() >= g_ScreenSize.bottom - 2) {
            this->vis_map_context->ScrollMapY(1);
            scroll = true;
        }

        if (scroll) {
            this->vis_minimap->MsgProc(0x408, 0, 0);
        }
    }

    if (this->has_focus != 0) {
        this->vis_root->MsgProc(0x402, 0, 0);
    }

    g_mousept.Update();
}

void MainWindow::SingleGameIdle() { //48a3a2
    g_mousept.Update();

    g_Server->ServerTic();

    this->vis_map_context->ProcessPackets(0x64);

    this->vis_root->MsgProc(0x401, 0, 0);

    this->game_tic_counter++;
    g_mousept.Update();


    if (g_mousept.GetSelectState() == 0 && this->dialogsMask == 1) {
        bool scroll = false;

        if (g_mousept.GetX() <= 0) {
            this->vis_map_context->ScrollMapX(-1);
            scroll = true;
        }
            

        if (g_mousept.GetY() <= 0) {
            this->vis_map_context->ScrollMapY(-1);
            scroll = true;
        }

        if (g_mousept.GetX() >= g_ScreenSize.right - 2) {
            this->vis_map_context->ScrollMapX(1);
            scroll = true;
        }

        if (g_mousept.GetY() >= g_ScreenSize.bottom - 2) {
            this->vis_map_context->ScrollMapY(1);
            scroll = true;
        }

        if (scroll) {
            this->vis_minimap->MsgProc(0x408, 0, 0);
        }
    }

    if (this->has_focus != 0) {
        this->vis_root->MsgProc(0x402, 0, 0);
        SaveScreenshot();
    }

    g_mousept.Update();
}

void MainWindow::StartHatDedicatedServer() { //49172d
    //CHECK IN Allods2.exe
    this->sessionMode = 3;
    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);
    g_CLlDriver.ResetProvider(4);

    CLlAddress addr;
    strcpy(addr.address, g_ServerConfig.ip_address);

    if (g_CLlDriver.StartServer(g_ServerConfig.max_players, this->m_GameSession.character_name, &addr) == 0) {
        AfxMessageBox("Error initializing TCP/IP connection", 0, 0);
        exit(1);
    }

    if (this->StartServerSession(0) != 0) {
        AfxMessageBox("Missing or invalid world.res", 0, 0);
        exit(1);
    }

    PostMessage(0x459, 0, 0);
    PasswordManager::manager.OpenFile("passbase.txt");
}

void MainWindow::SpawnScenarioMissionUnits(int32_t id, const CDWordArray& array) { //50260a
    for (int i = 0; i < array.GetSize(); i++) {
        uint16_t uid = array[i];

        int32_t unit_info_id = g_GameDataRes.FindHumanByServerId(uid * 10 + 10000);
        bool not_human = unit_info_id == 0;
        if (not_human) {
            unit_info_id = g_GameDataRes.FindMonsterByServerId(uid * 10 + 10000);
        }

        for (int j = 1; j < 10; j += 1) {
            int32_t id_variant;
            if (not_human) {
                id_variant = g_GameDataRes.FindMonsterByServerId(uid * 10 + 10000 + j);
            } else {
                id_variant = g_GameDataRes.FindHumanByServerId(uid * 10 + 10000 + j);
            }

            if (id_variant > 0) {
                int32_t tmp_id;
                if (not_human) {
                    tmp_id = g_GameDataRes.FindMonsterByServerId(uid * 10 + 9998 + id / 10);
                } else {
                    tmp_id = g_GameDataRes.FindHumanByServerId(uid * 10 + 9998 + id / 10);
                }

                if (tmp_id != 0) {
                    unit_info_id = tmp_id;
                }

break;
            }
        }

        Unit* unit;
        if (not_human) {
            unit = new Unit(g_GameDataRes.monsters[unit_info_id].name);
        } else {
            unit = new Human(g_GameDataRes.humans[unit_info_id].name, 0, nullptr);
        }

        unit->server_id = array[i];
        unit->building_id = g_buildingIdSet.AllocBit() & 0xffff;

        Player* pl = g_PlayersList->GetHead();
        unit->pOwner = pl;

        pl->unit_list->AddTail(unit);

        Group* grp = new Group();

        pl->group_list->groups.AddTail(grp);

        grp->AddUnit(unit);
        g_World->ResetUnitToHold(unit);
        g_World->ProcessAnimateDeadGroup(grp);
        g_NetStru1_main.SendUnitUpdate(unit, unit->pOwner, 0xffffffff, 0xffb, 0, 0);
    }

    g_NetStru1_main.SendTickPacket(nullptr);
    g_NetStru1_main.SendAllData();
}


int32_t MainWindow::RunModalLoop() { //49057b
    MSG msg;

    LockSurface2();
    FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    //Connection to hat in process...
    g_font1->DrawTextWithShadow(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2, txt_patch.GetLine(124), 2, clrsh_ShockingBlack, 1);
    UnlockSurface2();
    FlushScreen();

    this->sessionMode = 0;

    g_NetStru1_local.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_local);
    g_CLlDriver.ResetProvider(4);

    if (strstr(afxCurrentWinApp->m_lpCmdLine, "-timeout")) {
        g_CLlDriver.SetTimeout(g_CmdTimeout);
    }

    CLlAddress local_224;
    strcpy(local_224.address, "0.0.0.0");

    int local_230 = g_CLlDriver.PrepareForConnect(this->hat_settings.hatprogip, &local_224);
    if (local_230) {
        local_230 = g_CLlDriver.Connect(this->m_GameSession.character_name, nullptr);
    }

    if (local_230 == 0) {
        g_CLlDriver.Close();

        this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(19), nullptr, 0));
        PostMessage(WM_CLOSE, 0, 0);

        return 0;
    }

    g_NetStru1_local.ProcessConnections();

    g_NetStru1_local.SendHatCredentials(this->hat_settings.login, this->hat_settings.password, this->hat_settings.deathmatch);
    this->field_0x3e0.hat_id = 0;

    uint32_t start_time = timeGetTime();
    INT_00660f8c = 0;
    while(this->field_0x3e0.hat_id == 0) {
        if (PeekMessageA(&msg, nullptr, 0, 0, 1) != 0) {
            if (msg.message == WM_QUIT) {
                return 0;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (g_NetStru1_local.GetClientsPktNum() == 0) {
            if (g_CmdTimeout < timeGetTime() - start_time) {
                this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(18), nullptr, 0));
                PostMessage(WM_CLOSE, 0, 0);
                return 0;
            }

            g_mousept.Update();
        } else if (this->vis_map_context->ProcessPackets(100) == 0) {
            this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(INT_00660f8c & 0xff), nullptr, 0));
            PostMessage(WM_CLOSE, 0, 0);
            return 0;
        }
    }

    LockSurface2();
    FillRectColorSimple(0, 0, g_ScreenSize.right, g_ScreenSize.bottom, 0);
    g_font1->DrawTextWithShadow(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2, txt_patch.GetLine(125), 2, clrsh_ShockingBlack, 1);
    UnlockSurface2();
    FlushScreen();

    this->m_GameSession.RefreshCharacterRosterFiles(0);

    for (int j = 0; j < this->m_GameSession.hat_char_ids.GetSize(); j++) {
        HatCharId& curid = this->m_GameSession.hat_char_ids[j];

        bool has_file = false;

        for (int k = 0; k < this->m_GameSession.characterRosterHatId.GetSize(); k++) {
            if (curid.id == this->m_GameSession.characterRosterHatId[k].id) {
                has_file = true;
                break;
            }
        }

        if (!has_file) {
            g_NetStru1_local.SendHatCharId(this->m_GameSession.hat_char_ids[j]);
            INT_00660f8c = 0;

            while (true) {
                if (PeekMessageA(&msg, nullptr, 0, 0, 1) != 0) {
                    if (msg.message == WM_QUIT) {
                        return 0;
                    }

                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }

                if (g_NetStru1_local.GetClientsPktNum() != 0) {
                    if (this->vis_map_context->ProcessPackets(100) == 0) {
                        this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(17), nullptr, 0));
                        PostMessage(WM_CLOSE, 0, 0);
                        return 0;
                    }
                    break;
                }

                if (g_CmdTimeout < timeGetTime() - start_time) {
                    this->ModalScreen(new VisMessageBoxWithList(1, 64, 100, 380, 594, txt_patch.GetLine(18), nullptr, 0));
                    PostMessage(WM_CLOSE, 0, 0);
                    return 0;
                }
                g_mousept.Update();
            }
        }

        LockSurface2();
        FillRectColorSimple(0, g_ScreenSize.bottom / 2 + 20, g_ScreenSize.right, g_ScreenSize.bottom / 2 + 40, 0);

        CString local_260;
        local_260.Format("%d / %d", j, this->m_GameSession.hat_char_ids.GetSize());
        g_font1->DrawTextWithShadow(g_ScreenSize.right / 2, g_ScreenSize.bottom / 2 + 20, local_260, 2, clrsh_ShockingBlack, 1);
        UnlockSurface2();
        FlushScreen();
    }
    return 1;
}


int32_t MainWindow::CheckWebVersion(const CString& addr) { //4e5466
    HINTERNET agent = InternetOpenA("RageOfMages2", 0, 0, 0, 0);
    if (!agent) {
        return 0;
    }

    CString tmp;
    for (int i = 0; i < addr.GetLength(); i++) {
        char chr = addr[i];
        if (chr == ' ') {
            tmp += '+';
        } else {
            tmp += chr;
        }
    }

    CString proto = tmp.Left(7);
    proto.MakeLower();

    if (proto != "http://") {
        tmp = "http://" + tmp;
    }

    HINTERNET url = InternetOpenUrlA(agent, tmp, nullptr, 0, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);
    if (!url == 0) {
        InternetCloseHandle(agent);
        return 0;
    }

    static char buf[0x40000];
    DWORD readed = 0;

    char* buf_pos = buf;
    while (true) {
        if (!InternetReadFile(url, buf_pos, 0x40000, &readed)) {
            InternetCloseHandle(agent);
            return 0;
        }

        if (!readed) {
            break;
        }

        buf_pos += readed; // WAT FIXME  OVERFLOW!!
    }

    InternetCloseHandle(agent);

    DAT_00666a00.RemoveAll();
    if (!strstr(buf, "CURRENTCOUNT")) {
        return 0;
    }

    char *cursor = strchr(buf, '|');
    if (!cursor) {
        return 0;
    }

    cursor++;

    int nnum = atoi(cursor);

    cursor = strchr(buf, '\n');
    cursor++;
    cursor = strchr(cursor, '\n');
    cursor++;
    cursor = strchr(cursor, '\n');

    for (int i = 0; i < nnum; i++) {
        while (isprint(*cursor) == 0) {
            cursor++;
        }

        CString srv = '|';
        while(*cursor >= ' ') {
            srv += *cursor;
            cursor++;
        }

        DAT_00666a00.Add(srv);
    }

    return 1;
}

int32_t MainWindow::LeaveServerMode() { //490eb3

    g_NetStru1_main.SendHatLogout();
    INT_00660f8c = 0;

    uint32_t start_time = timeGetTime();
    while (true) {
        if (false) {
            return 1;
        }

        MSG msg;
        if (PeekMessageA(&msg, nullptr, 0, 0, 1) != 0) {
            if (msg.message == WM_QUIT) {
                return 0;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        if (g_NetStru1_local.GetClientsPktNum() != 0) {
            break;
        }

        if (timeGetTime() - start_time > g_CmdTimeout) {
            return 0;
        }

        g_mousept.Update();
    }

    int32_t res = this->vis_map_context->ProcessPackets(100);

    if (res != 0) {
        return res;
    }

    return 0;
}



CGameSession::CGameSession() { //49248a
    //fields init in defination
    strcpy(this->character_name, "Self");
}

CGameSession::~CGameSession() { //4960f0
}

int CGameSession::SubmitCharacterSetupAndWaitForSelectedUnit() {
    //493ffe
    INT_00660f8c = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if ((this->flags & 1) == 0 && wnd->sessionMode != 2) {
        wnd->vis_map_context->FUN_0041cc78(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex]);
    } else {
        wnd->vis_map_context->FUN_0041cbb8();
    }

    if (wnd->sessionMode == 1 || wnd->sessionMode == 2) {
        g_Server->PumpNetwork();
    }

    uint32_t stime = timeGetTime();
    while (wnd->vis_map_context->field_0x3f6c == nullptr) {
        while (g_NetStru1_local.GetClientsPktNum() == 0) {
            MSG msg;

            if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0) {
                if (msg.message == WM_QUIT) {
                    return 0;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }

            if (g_CmdTimeout < timeGetTime() - stime) {
                INT_00660f8c = 0x1008;
                return 0;
            }

            g_mousept.Update();
            g_NetStru1_local.ProcessConnections();
        }

        if (wnd->vis_map_context->ProcessPackets(0x64) == 0) {
            return 0;
        }

    }

    return 1;
}


void CGameSession::ResetCharacter() { //493d8d
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if (wnd->sessionMode == 2) {
        return;
    }

    CUnit* cu = wnd->vis_map_context->GetUnit_3f6c();
    this->money = 1000;
    this->monster_killed = 0;
    this->player_killed = 0;
    this->death_count = 0;
    this->fragCount = 0;

    FileSectionBasicInfo local_40;
    local_40.id1 = this->sessionKeyPart1;
    local_40.id2 = this->sessionKeyPart2;
    local_40.hat_id = this->hat_id;

    strcpy(local_40.nick, this->character_name);

    local_40.character_class = this->type;

    this->face = cu->face;

    local_40.picture = this->face;
    local_40.main_sphere = this->main_sphere;
    local_40.flags = this->flags;
    local_40.color = this->color;

    FileSectionStats local_74;
    local_74.body = cu->stat_body;
    local_74.reaction = cu->stat_reaction;
    local_74.mind = cu->stat_mind;
    local_74.spirit = cu->stat_spirit;
    local_74.money = 1000;
    local_74.monster_kills = 0;
    local_74.player_kills = 0;
    local_74.frags = 0;
    local_74.deaths = 0;
    local_74.spells = cu->spells;
    local_74.active_spell = cu->active_spell;
    local_74.experience[0] = cu->experience_per_sphere[0];
    local_74.experience[1] = cu->experience_per_sphere[1];
    local_74.experience[2] = cu->experience_per_sphere[2];
    local_74.experience[3] = cu->experience_per_sphere[3];
    local_74.experience[4] = cu->experience_per_sphere[4];

    uint8_t buffer[2560];
    uint8_t* bufpos = buffer;

    for (int i = 0; i < 9; i++) {
        this->shortcuts[i].ToBuffer(&bufpos);
    }
    
    WritePlayerFile_4F53EA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], &local_40, &local_74, wnd->vis_map_context->kill_stats.data(), &PacketUnitStateVec::Inst, nullptr, buffer, bufpos - buffer);
}


void CGameSession::ResetUnitClass() {
    this->body = 0;
    this->main_sphere = 0;
    this->RecreateCUnit();
}


void CGameSession::RecreateCUnit() { //4941c0
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* cu = mwnd->vis_map_context->GetUnit_3f6c();
    if (cu) {
        Human* hm = nullptr;
        switch (this->type & 0xc0) {
        default:
        case 0:
            hm = new Human("Start_MF", 1, nullptr);
            break;

        case 0x40:
            hm = new Human("Start_MM", 1, nullptr);
            break;
            
        case 0x80:
            hm = new Human("Start_FF", 1, nullptr);
            break;
            
        case 0xc0:
            hm = new Human("Start_FM", 1, nullptr);
            break;
        }

        if (this->body != 0) {
            hm->body = this->body;
            hm->reaction = this->reaction;
            hm->mind = this->mind;
            hm->spirit = this->spirit;
            hm->RecalcStats();
            hm->face = (byte)this->face;
        }

        if (this->main_sphere == 0) {
            this->main_sphere = hm->main_sphere;
        }

        hm->InitCreationSkills(this->main_sphere, 0x14);
        cu->CopyFromUnit(*hm);

        this->body = cu->stat_body = cu->body;
        this->reaction = cu->stat_reaction = cu->reaction;
        this->mind = cu->stat_mind = cu->mind;
        this->spirit = cu->stat_spirit = cu->spirit;

        this->face = cu->face;

        delete hm;
    }
}


void CGameSession::LoadCharacterRosterEntry(int32_t idx) { //492c66
    this->selectedCharacterRosterFileIndex = idx;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();

    if (unit == nullptr) {
        unit = new CUnit();
        unit->InitState(1, 1, 0, 0, 0, mwnd->vis_map_context->field_0x9b8[0], 0, 0, 0, 1);
        mwnd->vis_map_context->field_0x9d0[1] = unit;
        mwnd->vis_map_context->field_0x3f6c = unit;
        mwnd->vis_map_context->MsgProc(0x405, 0, 0);
    }

    if (idx >= 0 && idx < this->characterRosterFilePaths.GetSize()) {
        CFile f(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], CFile::modeRead);

        FileSectionStats* stats = nullptr;
        FileSectionBasicInfo* basic = nullptr;
        uint8_t* kills = nullptr;
        PacketUnitStateVec* equip = nullptr;
        PacketUnitStateVec* inv = nullptr;
        uint8_t* sec40a = nullptr;
        uint32_t size_40a = 0;
        if (ParsePlayerFile_4F62E6(&f, &basic, &stats, &kills, &equip, &inv, &sec40a, &size_40a) != 0 && basic != nullptr && stats != nullptr) {
            strcpy(this->character_name, basic->nick);

            memset(unit->str1, 0, 12);

            char* delim = strchr(this->character_name, '|');
            if (delim == NULL) {
                memcpy(unit->str1, this->character_name, 11);
                unit->str2[0] = 0;
            } else {
                memcpy(unit->str1, this->character_name, delim - this->character_name);
                memcpy(unit->str2, delim + 1, 12);
            }

            this->type = basic->character_class;
            this->main_sphere = basic->main_sphere;
            this->flags = basic->flags;
            this->color = basic->color;
            this->hat_id = basic->hat_id;

            this->sessionKeyPart1 = basic->id1;
            this->sessionKeyPart2 = basic->id2;

            this->money = stats->money;
            this->monster_killed = stats->monster_kills;
            this->player_killed = stats->player_kills;
            this->death_count = stats->deaths;
            this->fragCount = stats->frags;

            Human* hum;
            switch (this->type & 0xc0) {
            default:
            case 0:
                hum = new Human("Start_MF", 1, nullptr);
                break;
            case 0x40:
                hum = new Human("Start_MM", 1, nullptr);
                break;
            case 0x80:
                hum = new Human("Start_FF", 1, nullptr);
                break;
            case 0xc0:
                hum = new Human("Start_FM", 1, nullptr);
                break;
            }

            this->body = stats->body;
            hum->body = this->body;

            this->reaction = stats->reaction;
            hum->reaction = this->reaction;

            this->mind = stats->mind;
            hum->mind = this->mind;

            this->spirit = stats->spirit;
            hum->spirit = this->spirit;

            this->face = basic->picture;
            hum->face = this->face;

            hum->experience_per_sphere[0] = stats->experience[0];
            hum->experience_per_sphere[1] = stats->experience[1];
            hum->experience_per_sphere[2] = stats->experience[2];
            hum->experience_per_sphere[3] = stats->experience[3];
            hum->experience_per_sphere[4] = stats->experience[4];

            int32_t maxlvl = 0;

            for (int i = 1; i < 6; i++) {
                hum->hit_values.skill_levels[i] = ExperienceTable::GetLevel(hum->experience_per_sphere[i - 1]);

                if (hum->hit_values.skill_levels[i] > maxlvl) {
                    maxlvl = hum->hit_values.skill_levels[i];
                }
            }

            for (int i = 1; i < 6; i++) {
                hum->hit_values2.skill_levels[i] = hum->hit_values.skill_levels[i];
            }

            if (maxlvl >= 96) {
                this->field_0x114 = 4;
            } else if (maxlvl >= 76) {
                this->field_0x114 = 3;
            } else if (maxlvl >= 51) {
                this->field_0x114 = 2;
            } else {
                this->field_0x114 = 1;
            }

            if (maxlvl >= 90) {
                this->field_0x118 = 4;
            } else if (maxlvl >= 51) {
                this->field_0x118 = 3;
            } else if (maxlvl >= 26) {
                this->field_0x118 = 2;
            } else {
                this->field_0x118 = 1;
            }

            hum->experience = 0;

            for (int i = 1; i < 6; i++) {
                hum->experience += hum->experience_per_sphere[i - 1];
            }

            hum->RecalcStats();

            if (equip != nullptr) {
                uint8_t* eq_data = equip->data;

                hum->UnequipAll();

                if (hum->weapon) {
                    hum->inventory->PutItemIntoBagAtDefault(hum->Unequip(hum->weapon));
                }

                if (hum->shield) {
                    hum->inventory->PutItemIntoBagAtDefault(hum->Unequip(hum->shield));
                }

                if (hum->inventory) {
                    delete hum->inventory;
                }

                hum->inventory = new Inventory();
                for (int i = 1; i < 13; i++) {
                    Item* itm = ReadItem(&eq_data);

                    if (itm->item_id != 0) {
                        hum->UseItemOrStash(itm);
                    }
                }
            }

            unit->CopyFromUnit(*hum);

            unit->stat_body = stats->body;
            unit->stat_reaction = stats->reaction;
            unit->stat_mind = stats->mind;
            unit->stat_spirit = stats->spirit;

            unit->map_player = mwnd->vis_map_context->field_0x9b8[0];

            if (sec40a != nullptr) {
                for (int i = 0; i < 9; i++) {
                    this->shortcuts[i].LoadFromBuffer(&sec40a);
                }
            }
        }
    }
}

void CGameSession::InitializeNewCharacterSession(int tp, const char* name) { //493ab6
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    this->hat_id = mwnd->field_0x3e0.hat_id;

    if (name) {
        strcpy(this->character_name, name);
        this->type = tp;
    }

    LARGE_INTEGER k;
    if (QueryPerformanceCounter(&k) == 0) {
        this->sessionKeyPart2 = k.LowPart;
        this->sessionKeyPart1 = timeGetTime();
    }

    this->characterRosterNames.InsertAt(this->characterRosterNames.GetSize() - 1, this->character_name);

    CString fname;
    fname.Format("%u%u.a2c", this->sessionKeyPart1, this->sessionKeyPart2);
    this->characterRosterFilePaths.Add(fname);

    this->selectedCharacterRosterFileIndex = this->characterRosterFilePaths.GetSize() - 1;

    this->flags = 1;

    mwnd->vis_map_context->kill_stats.fill(0);

    if (mwnd->vis_map_context->GetUnit_3f6c() == nullptr) {
        CUnit* cunit = new CUnit();
        cunit->InitState(1, 1, 0, 0, 0, mwnd->vis_map_context->field_0x9b8[0], 0, 0, 0, 1);

        mwnd->vis_map_context->field_0x9d0[1] = cunit;
        mwnd->vis_map_context->field_0x3f6c = cunit;
    }

    this->field_0x118 = 1;
    this->field_0x114 = 1;
    this->main_sphere = 0;

    this->RecreateCUnit();
}

void CGameSession::SetNameAndColor(char* name) { //49381c
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();

    CString str(name);
    int colon = str.Find(':');
    if (colon != -1) {
        str = str.Mid(colon + 1);
        this->color = atoi(str);
        if (this->color < 1 || this->color > 0x10) {
            this->color = 0;
        }
    }

    strcpy(unit->str2, name);
    unit->str2[colon] = '\0';

    this->characterRosterNames[this->selectedCharacterRosterFileIndex] = unit->str1;
    if (unit->str2[0] != '\0') {
        this->characterRosterNames[this->selectedCharacterRosterFileIndex] += '|';
        this->characterRosterNames[this->selectedCharacterRosterFileIndex] += unit->str2;
    }

    strcpy(this->character_name, this->characterRosterNames[0]);

    FileSectionBasicInfo info;
    memcpy(&info, &this->sessionKeyPart1, 8);
    info.hat_id = this->hat_id;
    strcpy(info.nick, this->characterRosterNames[this->selectedCharacterRosterFileIndex]);
    info.character_class = (uint8_t)this->type;
    this->face = unit->face;
    info.picture = (uint8_t)this->face;
    info.main_sphere = (uint8_t)this->main_sphere;
    info.flags = (uint8_t)this->flags;
    info.color = (uint8_t)this->color;

    WritePlayerFile_4F53EA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], &info, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
}

void CGameSession::AutosaveCharacter() { //4946e0
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->sessionMode == 2) {
        return;
    }

    BigStruct2* map = mwnd->vis_map_context;
    CUnit* unit = map->GetUnit_3f6c();

    if (this->flags & 1) {
        this->flags &= ~1;
    }

    FileSectionBasicInfo info;
    memcpy(&info, &this->sessionKeyPart1, 8);
    info.hat_id = this->hat_id;
    strcpy(info.nick, this->character_name);
    info.character_class = (uint8_t)this->type;
    this->face = unit->face;
    info.picture = (uint8_t)this->face;
    info.main_sphere = (uint8_t)this->main_sphere;
    info.flags = (uint8_t)this->flags;
    info.color = (uint8_t)this->color;

    FileSectionStats stats;
    stats.body = unit->stat_body;
    stats.reaction = unit->stat_reaction;
    stats.mind = unit->stat_mind;
    stats.spirit = unit->stat_spirit;
    stats.money = map->my_main_unit->gold;
    stats.monster_kills = this->monster_killed;
    stats.player_kills = this->player_killed;
    stats.frags = this->fragCount;
    stats.deaths = this->death_count;
    stats.spells = unit->spells;
    stats.active_spell = unit->active_spell;
    stats.experience[0] = unit->experience_per_sphere[0];
    stats.experience[1] = unit->experience_per_sphere[1];
    stats.experience[2] = unit->experience_per_sphere[2];
    stats.experience[3] = unit->experience_per_sphere[3];
    stats.experience[4] = unit->experience_per_sphere[4];

    WritePlayerFile_4F53EA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], &info, &stats, nullptr, nullptr, nullptr, nullptr, 0);

    this->last_autosave_tick = GetTickCount();
}

void CGameSession::FUN_004948b2() { //4948b2
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->sessionMode == 2) {
        return;
    }

    BigStruct2* map = mwnd->vis_map_context;
    map->GetUnit_3f6c();

    uint8_t buffer[2560];
    uint8_t* buf = buffer;
    for (int32_t i = 0; i < 9; i++) {
        this->shortcuts[i].ToBuffer(&buf);
    }

    WritePlayerFile_4F53EA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], nullptr, nullptr, nullptr, nullptr, nullptr, buffer, buf - buffer);
}

void CGameSession::DeleteSelectedCharacter() { //493cd8
    DeleteFileA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex]);
    this->characterRosterFilePaths.RemoveAt(this->selectedCharacterRosterFileIndex, 1);
    this->characterRosterNames.RemoveAt(this->selectedCharacterRosterFileIndex, 1);
    this->characterRosterHatId.RemoveAt(this->selectedCharacterRosterFileIndex, 1);
    if (this->selectedCharacterRosterFileIndex == 0) {
        this->selectedCharacterRosterFileIndex++;
    }
    this->LoadCharacterRosterEntry(this->selectedCharacterRosterFileIndex - 1);
}

void CGameSession::SaveKillStats() { //494982
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    if (mwnd->sessionMode == 2) {
        return;
    }

    BigStruct2* map = mwnd->vis_map_context;
    map->GetUnit_3f6c();

    WritePlayerFile_4F53EA(this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex], nullptr, nullptr, map->kill_stats.data(), nullptr, nullptr, nullptr, 0);
}

void CGameSession::SetCharacterStats(int32_t body, int32_t reaction, int32_t mind, int32_t spirit, int32_t main_sphere) { //494148
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();

    this->body = body;
    this->reaction = reaction;
    this->mind = mind;
    this->spirit = spirit;
    if (main_sphere != 0) {
        this->main_sphere = main_sphere;
        unit->unitFlags |= 8;
    }
    this->RecreateCUnit();
}

CString CGameSession::GetSelectedRosterPath() { //420070
    if (this->selectedCharacterRosterFileIndex < this->characterRosterFilePaths.GetSize()) {
        return this->characterRosterFilePaths[this->selectedCharacterRosterFileIndex];
    }
    return CString();
}

void CGameSession::AutosaveTick() { //4946ae
    DWORD tick = GetTickCount();
    if (15000 < tick - this->last_autosave_tick) {
        this->AutosaveCharacter();
    }
}

UserShortcut::UserShortcut() { //4971a0
    this->SetNull();
}

UserShortcut::~UserShortcut() { //41e493
    if (this->mods_size) {
        if (this->mods) {
            free(this->mods);
        }

        this->mods_size = 0;
        this->mods = nullptr;
    }
}

void UserShortcut::SetNull() { //4971c0
    this->mods_size = 0;
    this->item_id = 0;
    this->kind = 0;
    this->mods = nullptr;
}

int UserShortcut::ToBuffer(uint8_t** buf) { //41e5ed
    uint8_t* data = *buf;
    *(uint16_t*)data = this->kind;
    data += 2;
    *(uint16_t*)data = this->item_id;
    data += 2;
    *(uint32_t*)data = this->mods_size;
    data += 4;
    if (this->mods_size) {
        memcpy(data, this->mods, this->mods_size);
        data += this->mods_size;
    }
    *buf = data;
    return 8 + this->mods_size;
}


int32_t UserShortcut::MatchesItem(TokenEntry* entry) { //41e3af
    if (entry == nullptr) {
        return 0;
    }

    if (this->kind != 2 || this->item_id != entry->item_id) {
        return 0;
    }

    if (this->mods_size != entry->mods_size) {
        return 0;
    }

    const uint8_t* entry_mods = (const uint8_t*)entry->mods;
    for (uint32_t i = 0; i < this->mods_size; i++) {
        if (this->mods[i] != entry_mods[i]) {
            return 0;
        }
    }

    return 1;
}

void UserShortcut::SetItem(TokenEntry* entry) { //41e343
    this->kind = 2;
    this->item_id = entry->item_id;
    this->mods_size = entry->mods_size;

    if (this->mods_size != 0) {
        this->mods = (uint8_t*)malloc(this->mods_size);
        memcpy(this->mods, entry->mods, this->mods_size);
    }
}

int32_t UserShortcut::IsSpellShortcut(short spell_id) { //41e456
    if (this->kind == 1 && this->item_id == (uint16_t)spell_id) {
        return 1;
    }
    return 0;
}

void UserShortcut::SetSpell(short spell_id) { //41e323
    this->kind = 1;
    this->item_id = spell_id;
}

void UserShortcut::WriteToFile(CFile* f) { //41e4d3
    f->Write(&this->kind, 2);
    f->Write(&this->item_id, 2);
    f->Write(&this->mods_size, 4);
    if (this->mods_size != 0) {
        f->Write(this->mods, this->mods_size);
    }
}

void UserShortcut::ReadFromFile(CFile* f) { //41e53b
    if (this->mods_size != 0) {
        if (this->mods) {
            free(this->mods);
        }

        this->mods = nullptr;
        this->mods_size = 0;
    }

    f->Read(&this->kind, 2);
    f->Read(&this->item_id, 2);
    f->Read(&this->mods_size, 4);

    if (this->mods_size != 0) {
        this->mods = (uint8_t *)malloc(this->mods_size);
        f->Read(this->mods, this->mods_size);
    }
}


void UserShortcut::LoadFromBuffer(uint8_t** buf) { //41e689
    uint8_t* data = *buf;

    if (this->mods_size != 0) {
        if (this->mods != nullptr) {
            free(this->mods);
        }

        this->mods = nullptr;
        this->mods_size = 0;
    }

    this->kind = *(uint16_t*)data;
    data += 2;
    this->item_id = *(uint16_t*)data;
    data += 2;
    this->mods_size = *(uint32_t*)data;
    data += 4;

    if (this->mods_size != 0) {
        this->mods = (uint8_t*)malloc(this->mods_size);
        memcpy(this->mods, data, this->mods_size);
        data += this->mods_size;
    }

    *buf = data;
}
