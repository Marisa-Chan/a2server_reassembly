#include "main_window.h"
#include <cstdio>
#include "asm_mfc.h"
#include "game_app.h"
#include "net.h"
#include "players_list.h"
#include "server.h"
#include <stdio.h>
#include <cstdio>

#include "asm_mfc.h"
#include "game_app.h"
#include "net.h"
#include "players_list.h"
#include "server.h"
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

uint32_t g_RemoteTimestamp = 0;
int32_t DAT_00660f88 = 0;
uint32_t g_using_vxd = 1; // 62fa9c
CDWordArray DAT_006658d8;

CRect vis_scr_rect; //65fb88



//495033
CListBox2::CListBox2() = default;

//495052
CListBox2::~CListBox2() = default;

const AFX_MSGMAP* CListBox2::GetMessageMap() const
{
    //49506e
    return &CListBox2::messageMap;
}

//60bfa8
AFX_DATADEF const AFX_MSGMAP CListBox2::messageMap =
{ &CListBox::messageMap, &CListBox2::_messageEntries[0] };

const AFX_MSGMAP_ENTRY CListBox2::_messageEntries[] =
{
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDBLCLK()
    {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
};

void __fastcall CListBox2::OnKeyDown(CListBox2* _this, void* edx, UINT arg1, UINT arg2, UINT arg3)
{ _this->_OnKeyDown(arg1, arg2, arg3); }

void __fastcall CListBox2::OnLButtonDblClk(CListBox2* _this, void* edx, UINT arg1, CPoint arg2)
{ _this->_OnLButtonDblClk(arg1, arg2); }

void CListBox2::_OnKeyDown(UINT, UINT, UINT)
{
	Default();
}

void CListBox2::_OnLButtonDblClk(UINT, CPoint)
{
	Default();
}












CEdit2::CEdit2() = default;
CEdit2::~CEdit2() = default;

const AFX_MSGMAP* CEdit2::GetMessageMap() const
{
	//494efb
	return &CEdit2::messageMap;
}

//60bf70
AFX_DATADEF const AFX_MSGMAP CEdit2::messageMap =
{ &CEdit::messageMap, &CEdit2::_messageEntries[0] };

const AFX_MSGMAP_ENTRY CEdit2::_messageEntries[] =
{
	ON_WM_KEYDOWN()
	{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0}
};

void __fastcall CEdit2::OnKeyDown(CEdit2* _this, void* edx, UINT arg1, UINT arg2, UINT arg3)
{ _this->_OnKeyDown(arg1, arg2, arg3); }

void CEdit2::_OnKeyDown(UINT nChar, UINT nRepcnt, UINT nFlags)
{
	Default();

	if (nChar == VK_RETURN)
	{
		CString txt;
		CWnd::GetWindowText(txt);

		char buf[1024];
		CharToOemA(txt, buf);

		g_NetStru1_main.FUN_0051cd89(buf, nullptr);

		LogMessage(buf);

		SetSel(0xffff0000, 0);
	}
}






Fame2::Fame2()
{
    //4abdab
    field_x0 = -1;
    field_x4 = 1;
    field_x8 = nullptr;
    str.Empty();
    field_x34 = 1;
    field_x38 = 0;
}

void Fame2::Clear()
{
    //4ac27f
    if (field_x8)
        delete field_x8;

    field_x8 = nullptr;
    str.Empty();
    str_arr.RemoveAll();
}

void Fame2::FUN_004abf0f(int32_t arg1, uint32_t arg2)
{
    //4abf0f
    Clear();
    field_x0 = arg1;
    field_x4 = arg2;
}


int32_t Fame2::FUN_004ad880()
{
    //4ad880
    return field_x0;
}

uint32_t Fame2::FUN_004ad890()
{
    //4ad890
    return field_x4;
}

void Fame2::operator=(const Fame2& b)
{
    //4abd30
    field_x0 = b.field_x0;
    field_x4 = b.field_x4;
    field_x8 = b.field_x8;
    str = b.str;
    rect = b.rect;
    field_x34 = b.field_x34;
    field_x38 = b.field_x38;
}








CFameHall::CFameHall()
{
    //4ac2e3
    m_nTotalTime = 0;
    m_nDifficultyWeight = 0;
    m_nMaxEntries = 10;
}

CFameHall::~CFameHall()
{
    //4ac352
}

void CFameHall::ZeroFactors()
{
    //4ac3af
    m_nTotalTime = 0;
    m_nDifficultyWeight = 0;
}

void CFameHall::SetDifficulty(uint32_t arg)
{
    //497270
    difficulty = arg;
}

uint32_t CFameHall::FUN_004ac566(int32_t arg)
{
    //4ac566
    for (uint32_t i = 0; i < m_Documents.GetSize(); i++)
    {
        Fame2& fame = m_Documents[i];
        if (fame.FUN_004ad880() == arg && fame.FUN_004ad890() != 0)
            return 0;
    }

    Fame2 f;
    f.FUN_004abf0f(arg, 1);
    m_Documents.Add(f);

    return 1;
}


void CFameHall::FUN_004acafa()
{
    //4acafa
    Fame1 fame;
    int32_t tval = 70000;
    for (int i = 0; i < 9; i++)
    {
        fame.field_x4 = tval + GetRandS16(5000);
        fame.str = TxtFile::AllLines[i + 0x107];
        FUN_004ac3ce(fame);
        tval -= 7000;
    }

    fame.field_x4 = 0;
    fame.str = TxtFile::AllLines[0x110];
    FUN_004ac3ce(fame);
}



void CFameHall::FUN_004ac945(CFile* file)
{
    //4ac945
    uint32_t num;
    file->Read(&num, 4);
    
    if (num == 0)
        m_Entries.RemoveAll();
    else
    {
        m_Entries.SetSize(num);
        for (uint32_t i = 0; i < num; i++)
        {
            uint32_t t;
            file->Read(&t, 4);

            char buf[1024];
            file->Read(buf, t);

            Fame1& fame = m_Entries[i];

            fame.str = buf;

            file->Read(&fame.field_x4, 4);
            file->Read(&fame.field_x8, 4);
            file->Read(&fame.field_xc, 4);
        }
    }
}

void CFameHall::FUN_004ac706(CFile* file)
{ //  4ac706
    int32_t num = m_Documents.GetSize();
    file->Write(&num, 4);

    for (uint32_t i = 0; i < num; i++)
    {
        Fame2& fm = m_Documents[i];
        
        //inline 4ac04b
        file->Write(&fm.field_x0, 4);
        file->Write(&fm.field_x4, 4);
    }
    file->Write(&m_nTotalTime, 4);
    file->Write(&m_nDifficultyWeight, 4);
}

void CFameHall::FUN_004ac7a2(CFile* file)
{ // 4ac7a2
    int32_t cnt;
    file->Read(&cnt, 4);
    m_Documents.SetSize(cnt);

    for (int i = 0; i < cnt; i++)
    {
        Fame2& fame = m_Documents[i];
        file->Read(&fame.field_x0, 4);
        file->Read(&fame.field_x4, 4);
    }

    file->Read(&m_nTotalTime, 4);
    file->Read(&m_nDifficultyWeight, 4);
}


void CFameHall::FUN_004ac3ce(const Fame1& fame)
{
    //4ac3ce
    if (m_Entries.GetSize() == 0)
        m_Entries.Add(fame);
    else
    {
        int32_t i = 0;
        for (i = 0; i < m_Entries.GetSize(); i++)
        {
            Fame1& f = m_Entries[i];
            if (f.field_x4 <= fame.field_x4)
            {
                m_Entries.InsertAt(i, fame);
                break;
            }
        }
        if (i == m_Entries.GetSize())
            m_Entries.Add(fame);
    }

    if (m_Entries.GetSize() > m_nMaxEntries)
        m_Entries.SetSize(m_nMaxEntries);
}

void CFameHall::FUN_00420050()
{
    //420050
    m_nDifficultyWeight++;
}

void CFameHall::SubmitScore()
{ //4ac498
    Fame1 fm;
    MainWindow *mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();
    if (unit)
    {
        fm.str = unit->str1;
        if (m_nTotalTime == 0)
            fm.field_x4 = unit->exp_summary / 500000.0 * m_nDifficultyWeight;
        else
            fm.field_x4 = unit->exp_summary / (m_nTotalTime * 10.0) * m_nDifficultyWeight;

        FUN_004ac3ce(fm);
    }
}


// 4951bd
ServerControlDialog::ServerControlDialog(CWnd* parent_window)
    : CDialog(0x6b, parent_window)
{
}

ServerControlDialog::~ServerControlDialog() {
}

void sub_4954EA(); // 4954ea — game.cpp

extern const AFX_MSGMAP off_610878; //610878 — CDialog message map, data remains in Main.asm

//60bff8
AFX_DATADEF const AFX_MSGMAP ServerControlDialog::messageMap =
{ &off_610878, &ServerControlDialog::_messageEntries[0] };

void __fastcall ServerControlDialog::MsgThunk495439(ServerControlDialog* _this, void* edx)
{ _this->sub_495439(); }

void __fastcall ServerControlDialog::MsgThunk49544C(ServerControlDialog* _this, void* edx)
{ _this->sub_49544C(); }

void __fastcall ServerControlDialog::MsgThunk49549B(ServerControlDialog* _this, void* edx)
{ _this->sub_49549B(); }

void __fastcall ServerControlDialog::MsgThunk4956D5(ServerControlDialog* _this, void* edx)
{ _this->sub_4956D5(); }

void __fastcall ServerControlDialog::MsgThunk495774(ServerControlDialog* _this, void* edx)
{ _this->sub_495774(); }

void __fastcall ServerControlDialog::MsgThunk4957AD(ServerControlDialog* _this, void* edx)
{ _this->sub_4957AD(); }

void __fastcall ServerControlDialog::MsgThunk4957E2(ServerControlDialog* _this, void* edx)
{ _this->sub_4957E2(); }

void __fastcall ServerControlDialog::MsgThunk49583E(ServerControlDialog* _this, void* edx)
{ _this->sub_49583E(); }

const AFX_MSGMAP_ENTRY ServerControlDialog::_messageEntries[] =
{
    { WM_DESTROY, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&MsgThunk495439 },
    ON_COMMAND(0x3f5, MsgThunk49544C)
    ON_COMMAND(0x3f6, MsgThunk49549B)
    ON_COMMAND(0x3f4, sub_4954EA)
    { WM_CLOSE, 0, 0, 0, AfxSig_vv, (AFX_PMSG)&MsgThunk4956D5 },
    ON_COMMAND(0x3ff, MsgThunk495774)
    ON_COMMAND(0x3f7, MsgThunk4957AD)
    ON_COMMAND(0x3fb, MsgThunk4957E2)
    ON_COMMAND(0x3fe, MsgThunk49583E)
    {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
};

// 4952fa
void ServerControlDialog::DoDataExchange(CDataExchange* pDX)
{
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
const AFX_MSGMAP* ServerControlDialog::GetMessageMap() const
{
    return &ServerControlDialog::messageMap;
}

void ServerControlDialog::sub_495439()
{
    //495439
    CWnd::OnDestroy();
}

void ServerControlDialog::sub_49544C()
{
    //49544C
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->SetSpeed(main_wnd->game_speed - 1)) {
        g_ServerConfig.game_speed = main_wnd->game_speed;
        g_NetStru1_main.sub_51D837(g_ServerConfig.game_speed, nullptr);
    }
}

void ServerControlDialog::sub_49549B()
{
    //49549B
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd->SetSpeed(main_wnd->game_speed + 1)) {
        g_ServerConfig.game_speed = main_wnd->game_speed;
        g_NetStru1_main.sub_51D837(g_ServerConfig.game_speed, nullptr);
    }
}

// 49585B
void ServerControlDialog::sub_49585B()
{
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
    this->button_188.EnableWindow(main_wnd->field_0x624 == 0);
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



MainWindow::MainWindow()
{ //4837e1
    field_0x348 = new CArray<DiplomacyEntry*>; //avoid struct with only one this field

    vis_root = nullptr;

    CPoint resolution;

    if (g_IsServer == 0)
    {
        const char* cmdline = afxCurrentWinApp->m_lpCmdLine;

        if (strstr(cmdline, "-800"))
        {
            resolution.x = 800;
            resolution.y = 600;
        }
        else if (strstr(cmdline, "-1024"))
        {
            resolution.x = 1024;
            resolution.y = 768;
        }
        else if (strstr(cmdline, "-640"))
        {
            resolution.x = 640;
            resolution.y = 480;
        }
        else if(strstr(g_resolution, "-800"))
        {
            resolution.x = 800;
            resolution.y = 600;
        }
        else if (strstr(g_resolution, "-1024"))
        {
            resolution.x = 1024;
            resolution.y = 768;
        }
        else
        {
            resolution.x = 640;
            resolution.y = 480;
        }

    }
    else
    {
        resolution.x = 640;
        resolution.y = 480;
    }

    g_ScreenSize.right = resolution.x;
    g_ScreenSize.bottom = resolution.y;

    vis_scr_rect.left = (resolution.x - 640) / 2;
    vis_scr_rect.right = resolution.x - (resolution.x - 640) / 2;
    vis_scr_rect.top = (resolution.y - 480) / 2;
    vis_scr_rect.bottom = resolution.y - (resolution.y - 480) / 2;

    if (g_IsServer == 0)
    {
        const char* cls = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, NULL, NULL, NULL);
        CFrameWnd::Create(cls, "Allods", WS_POPUP, CRect(0, 0, resolution.x, resolution.y), NULL, 0, 0, NULL);
    }
    else
    {
        CFrameWnd::LoadFrame(0x65, WS_TILEDWINDOW, NULL, NULL);
        SetWindowPos(0, 0, 0, resolution.x, resolution.y, SWP_NOREPOSITION | SWP_NOMOVE);
        SetControlPositions();
        ModifyStyle(0, 0, 0);
    }

    field_0x7fc = 0;
    field_0x800 = 0;
    music_player = nullptr;
    field_0x408 = nullptr;
    field_0x40c = -1;
    item_cursor = nullptr;
    music_update_proc = nullptr;
    dialogsMask = 0;
    field_0x44c = 0;
    game_speed = 4;
    field_0x438 = 0x7fffffff;
    last_tic_time = 0x7fffffff;
    field_0x448 = 0;
    serverBootstrapEnabled = 0;
    field_0x404 = 0;
    field_0x144 = nullptr;
    field_0x454 = 0;
    field_0x34c = nullptr;
    field_0x3dc = nullptr;
    field_0x378 = nullptr;
    field_0x37c = nullptr;
    vis_map_context = nullptr;

    field_0x3e0.field_08.Empty();
    field_0x3e0.field_00.Empty();
    field_0x3e0.field_0c = 0;
    field_0x3e0.field_10 = 0;

    field_0x624 = 0;

    cursor_sizewe = LoadCursorA(0, IDC_SIZEWE);
    cursor_sizens = LoadCursorA(0, IDC_SIZENS);
    cursor_sizenwse = LoadCursorA(0, IDC_SIZENWSE);
    cursor_sizenesw = LoadCursorA(0, IDC_SIZENESW);
    cursor_arrow = LoadCursorA(0, IDC_ARROW);
}


MainWindow::~MainWindow()
{ // 4961b0
    delete field_0x348;
}


void MainWindow::CreateUI()
{ //484ab0
    vis_root = new CVisualObject(0, 0, 0, g_ScreenSize.right, g_ScreenSize.bottom, nullptr);
    vis_map_context = new BigStruct2(0, 0, g_ScreenSize.right - 160, g_ScreenSize.bottom);
    vis_right_panel = new VisRightPanel(4, g_ScreenSize.right - 160, 0, g_ScreenSize.right, g_ScreenSize.bottom);
    vis_minimap = new VisMiniMap(5, 0, 0, 160, 158);
    vis_ordertoolbar = new VisOrderToolbar(6, 0, 158, 160, 238);
    vis_charinfo = new VisCharInfo(7, 0, 238, 160, 480);
    vis_sidestatus = new VisSideStatus(8, 0, 480, 160, g_ScreenSize.bottom);
    vis_invtype1 = new VisInvType1(2, 0, vis_map_context->GetRect().bottom - 90, g_ScreenSize.right - 160, vis_map_context->GetRect().bottom);

    vis_spellbook = new VisSpellBook(3, 0, vis_map_context->GetRect().bottom - 90, g_ScreenSize.right - 160, vis_map_context->GetRect().bottom);
    vis_spellbook->MsgProc(0x411, 0, 0);

    vis_shop = new VisShop(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_shopdruid = new VisShopDruid(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_shopkaarg = new VisShopKaarg(1000, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_globalmap = new VisGlobalMap(1050, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_mainmenu = new VisMainMenu(1090, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_tav = new VisTav(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_tavdruid = new VisTavDruid(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_tavkaarg = new VisTavKaarg(1100, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_town = new VisTown(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_towndruid = new VisTownDruid(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_townkaarg = new VisTownKaarg(1020, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_credits = new VisCredits(1110, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_logownd = new VisLogoWnd(1200, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_chargen = new VisCharGen(1110, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_charsel = new VisCharSelect(1120, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_startgame = new VisStartGame(1126, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_famehall = new VisFameHall(1200, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_famedocument = new VisFameDocument(1210, vis_scr_rect.left, vis_scr_rect.top, vis_scr_rect.right, vis_scr_rect.bottom);
    vis_1200 = new Vis1200(1200, 0, 0, g_ScreenSize.right - 160, 30);

    field_0xc0 = 0;

    vis_dropgold = new VisDropGold(1000000000, 100, g_ScreenSize.bottom - 200);

    vis_shop->VMethod26();
    vis_shopdruid->VMethod26();
    vis_shopkaarg->VMethod26();
    vis_tav->VMethod26();
    vis_tavdruid->VMethod26();
    vis_tavkaarg->VMethod26();
}

void MainWindow::LoadData()
{ //483d64

    g_ter_tiles.fill(nullptr);
    LoadCursors();
    LoadGraphics();
    LoadVfxData();
    LoadGameSounds();
    g_Cursors[CURSOR_DEFAULT]->Use();
    g_mousept.Paint();

    if (InitSound(AfxGetMainWnd()->m_hWnd, 16) != 0)
        g_SoundSettings.field_0x20 = 0;

    music_player = new MusicPlayer(g_isLowMemory == 0 ? 0xac000 : 0x56000);
    music_player->SetVolume(g_SoundSettings.mus_pos);
}



void GameSettings::Init()
{ // 442386
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    pGameSpeed = &mwnd->game_speed;
    pFormationMode = &mwnd->vis_map_context->formation;
    pWimpyMode = &mwnd->vis_map_context->wimpy;
    pShowAllHitPoints = &mwnd->vis_map_context->show_hp;
    pShowFlyingHP = &mwnd->vis_map_context->flying_hp;
    pShadows = &g_Shadows;
    pLightning = &g_Lightning;
    pAnimation = &g_Animation;
    pMessageColors = &g_MessageColors;
}

void GameSettings::Load(HKEY& rkey)
{ //442420
    DWORD sz = 4;
    RegQueryValueExA(rkey, "GameSpeed", nullptr, nullptr, (LPBYTE)pGameSpeed, &sz);
    RegQueryValueExA(rkey, "FormationMode", nullptr, nullptr, (LPBYTE)pFormationMode, &sz);
    RegQueryValueExA(rkey, "WimpyMode", nullptr, nullptr, (LPBYTE)pWimpyMode, &sz);
    RegQueryValueExA(rkey, "ShowAllHitPoints", nullptr, nullptr, (LPBYTE)pShowAllHitPoints, &sz);
    RegQueryValueExA(rkey, "Smoothing", nullptr, nullptr, (LPBYTE)&g_settings.Smoothing, &sz);
    RegQueryValueExA(rkey, "ShowFlyingHP", nullptr, nullptr, (LPBYTE)pShowFlyingHP, &sz);
    RegQueryValueExA(rkey, "ShowTimeFlow", nullptr, nullptr, (LPBYTE)&g_settings.ShowTimeFlow, &sz);
    RegQueryValueExA(rkey, "TipsMode", nullptr, nullptr, (LPBYTE)&g_settings.TipsMode, &sz);
    RegQueryValueExA(rkey, "AutoCasting", nullptr, nullptr, (LPBYTE)&AutoCasting, &sz);
    RegQueryValueExA(rkey, "Acknowledgement", nullptr, nullptr, (LPBYTE)&Acknowledgement, &sz);
    RegQueryValueExA(rkey, "Shadows", nullptr, nullptr, (LPBYTE)pShadows, &sz);
    RegQueryValueExA(rkey, "Lighting", nullptr, nullptr, (LPBYTE)pLightning, &sz);
    RegQueryValueExA(rkey, "Animation", nullptr, nullptr, (LPBYTE)pAnimation, &sz);
    RegQueryValueExA(rkey, "ClanNames", nullptr, nullptr, (LPBYTE)&ClanNames, &sz);
    RegQueryValueExA(rkey, "MessageColors", nullptr, nullptr, (LPBYTE)pMessageColors, &sz);
}

void PhoneBook::Load(HKEY& rkey)
{ //43ce7d
    DWORD sz = 4;
    int32_t num = 0;
    RegQueryValueExA(rkey, "phonebooksize", nullptr, nullptr, (LPBYTE)&num, &sz);

    for (int32_t i = 0; i < num; i++)
    {
        char name[64];
        sprintf(name, "phone%d", i);

        char data[64];
        sz = sizeof(data);
        RegQueryValueExA(rkey, name, nullptr, nullptr, (LPBYTE)data, &sz);
        phones.Add(data);
    }
}

void HatSettings::Load(HKEY& rkey)
{ //43d249
    char buf[256];

    buf[0] = '\0';
    DWORD sz = 255;
    RegQueryValueExA(rkey, "hatip", nullptr, nullptr, (LPBYTE)buf, &sz);

    hatip = buf;
    if (hatip.IsEmpty())
        hatip = txt_patch.GetLine(118);

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "hatprogip", nullptr, nullptr, (LPBYTE)buf, &sz);

    hatprogip = buf;
    if (hatprogip.IsEmpty())
        hatprogip = txt_patch.GetLine(119);

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "login", nullptr, nullptr, (LPBYTE)buf, &sz);

    login = XorRegString(buf);

    buf[0] = '\0';
    sz = 255;
    RegQueryValueExA(rkey, "password", nullptr, nullptr, (LPBYTE)buf, &sz);

    password = XorRegString(buf);

    sz = 4;
    ishat = 0;
    RegQueryValueExA(rkey, "ishat", nullptr, nullptr, (LPBYTE)&ishat, &sz);

    deathmatch = 0;
    RegQueryValueExA(rkey, "deathmatch", nullptr, nullptr, (LPBYTE)&deathmatch, &sz);

    store = 1;
    RegQueryValueExA(rkey, "store", nullptr, nullptr, (LPBYTE)&store, &sz);
}


void MainWindow::LoadSettings()
{ //441a03
    g_settings.Init();
    last_protocol = 0;
    com_settings.Reset();

    HKEY regkey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\1C\\Allods 2", 0, KEY_READ, &regkey) != 0)
        return;

    g_SoundSettings.Load(regkey);
    g_settings.Load(regkey);

    DWORD sz = 4;
    last_protocol = 0;
    RegQueryValueExA(regkey, "lastprotocol", nullptr, nullptr, (LPBYTE)&last_protocol, &sz); //inline 43d091

    phone_book.Load(regkey);

    sz = 0x14;
    RegQueryValueExA(regkey, "comportsetting", nullptr, nullptr, (LPBYTE)&com_settings, &sz); //inline 43d02e

    char buf[256];
    sz = 255;
    buf[0] = 0;
    RegQueryValueExA(regkey, "lastip", nullptr, nullptr, (LPBYTE)buf, &sz); //inline 43d0f5
    last_ip = buf;

    hat_settings.Load(regkey);

    sz = 4;
    RegQueryValueExA(regkey, "Using VxD", nullptr, nullptr, (LPBYTE)&g_using_vxd, &sz);
    if (g_using_vxd == 0)
        g_using_vxd = 1;

    RegCloseKey(regkey);
}


void MainWindow::SetControlPositions()
{ //484127
    CRect r;
    GetClientRect(&r);
    static2.SetWindowPos(NULL, 0, 0, r.right, 40, SWP_NOREPOSITION);
    list_box2.SetWindowPos(NULL, 0, 40, 200, 240, SWP_NOREPOSITION);

    CRect rl2;
    list_box2.GetClientRect(&rl2);
    rl2.bottom += 6;

    static1.SetWindowPos(NULL, 200, 40, r.right - 200, rl2.bottom, SWP_NOREPOSITION);
    list_box1.SetWindowPos(NULL, 0, rl2.bottom + 40, r.right, r.bottom - rl2.bottom - 80, SWP_NOREPOSITION);

    CRect rl1;
    list_box1.GetClientRect(&rl1);
    rl1.bottom += 6;

    edit.SetWindowPos(NULL, 0, rl2.bottom + 40 + rl1.bottom, r.right, r.bottom - rl2.bottom - rl1.bottom - 60, SWP_NOREPOSITION);
}


// Game loop tick processing function
// Called from GameApp::OnIdle when server is active
void MainWindow::sub_48A756()
{
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
                this->sub_484259();
                    
                // Server shutdown notifications (message 0x0D).
                if (g_ShutdownIn != 0x7FFFFFFF) {
                    if (g_ShutdownIn < 6000000 && g_ShutdownIn > 1000) { // > 1 second
                        int32_t tick_interval = this->game_tic_time * 16;
                        if ((g_ShutdownIn % 60000) < tick_interval) {
                            if (g_ServerConfig.gameType == 0) {
                                g_NetStru1_main.FUN_0051ce86(0x0D, g_ShutdownIn / 1000, nullptr);
                            }
                        }

                        if (g_ShutdownIn < 60000) {
                            int32_t tick_interval = this->game_tic_time * 16;
                            if ((g_ShutdownIn % 15000) < tick_interval) {
                                if (g_ServerConfig.gameType == 0) {
                                    g_NetStru1_main.FUN_0051ce86(0x0D, g_ShutdownIn / 1000, nullptr);
                                }
                            }
                        }
                    }
                        
                    int32_t tick_interval = this->game_tic_time * 16;
                    g_ShutdownIn -= tick_interval;
                        
                    if (g_ShutdownIn < 0) {
                        g_ShutdownIn = 0;
                        g_Server->sub_4F8831();
                        this->field_0x624 = 1;
                    }
                }
                
                if (this->field_0x800 != 0) {
                    this->field_0x7fc->sub_49585B();
                }
                
                // Check if should shut down.
                if (this->field_0x624 != 0) {
                    if (g_Server->FileList.GetSize() == 0) {
                        PostMessage(WM_CLOSE, 0, 0);
                    }
                }
                
                if (g_Server->field59_0x208 != 0) {
                    if (g_PlayersList->CountHumanPlayers() == 0) {
                        if (g_Server->FileList.GetSize() == 0) {
                            g_Server->sub_4F1E2A();

                            field_0x3d0->MsgProc(0x445, 0, 0);
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
                                g_NetStru1_main.FUN_0051ce86(0x0C, time_diff / 1000, nullptr);
                            }
                        }
                        
                        if (time_diff < 60000) {
                            if ((time_diff % 15000) < tick_interval) {
                                if (g_ServerConfig.gameType == 0 || g_ServerConfig.gameType == 3) {
                                    g_NetStru1_main.FUN_0051ce86(0x0C, time_diff / 1000, nullptr);
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
                                g_Server->sub_4FA5C4();
                            }
                            
                            g_Server->FUN_004f94c0(0);

                            return;
                        }
                    }
                }
                
                // Some kind of timeout?
                int32_t timeout_threshold = g_ServerConfig.arena_time_limit * 60000;
                if (timeout_threshold <= g_Server->map_elapsed_time2) {
                    if (g_ServerConfig.gameType == 3) {
                        if (g_Server->field59_0x208 == 0) {
                            g_Server->sub_4FA78E(2);
                        }
                    }
                }
            }
            
            // Client mode processing (g_IsServer == 0 means client mode).
            if (g_IsServer == 0) {
                if (this->field_0xbc != 0) {
                    if (field_0x3d0 != nullptr) {
                        if (this->game_tic_counter == 1) {
                            field_0x3d0->VMethod9();
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

void MainWindow::sub_484259()
{ //484259
    if (!g_Server)
        return;

    int32_t sredne = 0;
    if (g_Server->tic16)
        sredne = g_Server->field44_0x1bc / g_Server->tic16 / 10;

    char buf[2560];

    if (g_Server->srv_stru1->sack_list)
    {
        CString str;
        str.Format(txt_patch.GetLine(66), g_PlayersList->CountHumanPlayers(),
                                            dword_6CDB3C->unit_list.GetCount(),
                                            g_Server->srv_stru1->building_list->GetCount(),
                                            g_Server->srv_stru1->units_list->unit_list.GetCount(),
                                            g_Server->srv_stru1->sack_list->list.GetCount(),
                                            g_Server->field42_0x1b4 / 10,
                                            sredne,
                                            current_map_name,
                                            txt_main.GetLine(108 + game_speed));
        OemToCharA(str, buf);
        SetDlgItemText(0x7de0, buf);
    }

    CString t;
    int sel = list_box2.GetCurSel();
    if (sel >= 0)
        list_box2.GetText(sel, t);

    list_box2.ResetContent();

    for (POSITION it = g_PlayersList->list.GetHeadPosition(); it; )
    {
        Player* pl = g_PlayersList->list.GetNext(it);
        if (pl->is_ai == 0)
        {
            CString s = pl->name;
            if (pl->field_0x42 == 0)
                s = "(-)" + s;
            OemToCharA(s, buf);
            list_box2.AddString(buf);
        }
    }

    if (!t.IsEmpty())
        list_box2.SelectString(0, t);

    sel = list_box2.GetCurSel();

    Player* selpl = nullptr;
    
    if (sel >= 0)
    {
        int i = 0;
        for (POSITION it = g_PlayersList->list.GetHeadPosition(); it; )
        {
            Player* pl = g_PlayersList->list.GetNext(it);
            if (pl->is_ai == 0)
            {
                if (i == sel)
                {
                    selpl = pl;
                    break;
                }
                i++;
            }
        }
    }

    if (!selpl)
        SetDlgItemText(0x7ddf, "");
    else
    {
        CString local_a54;
        CString local_a44;
        CString stat1;
        CString stat2;

        NetStru2* con = g_NetStru1_main.GetClientByPlayerID(selpl->player_id);
        if (con)
        {
            uint32_t conid = con->GetUid();
            if (g_CLlDriver.GetProvider() == 4)
            {
                con = g_NetStru1_main.GetClientByLowUid(conid & 0x3fff);
                local_a54 = con->str;
                local_a44 = con->buf;
            }
            ConnStatInfo* stat;
            if (g_NetStru1_main.client_stat.Lookup(conid, stat))
            {
                stat1.Format("%d:%02d:%02d", stat->time / 3600, (stat->time % 3600) / 60, stat->time % 60);

                int bs = 0;
                if (stat->time)
                    bs = stat->total_bytes / stat->time;

                stat2.Format("%d/%d/%d", stat->cur_bs, bs, stat->max_bs);
            }
        }

        int32_t xpos = 0;
        int32_t ypos = 0;
        if (selpl->main_unit)
        {
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


int MainWindow::SetSpeed(int speed)
{
    //48dc02
    int32_t old_speed = game_speed;

    if (speed < 0)
        speed = 0;
  
    if (speed > 8)
        speed = 8;

    game_speed = speed;

    int ticks_per_second = 16;
    switch (game_speed)
    {
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

    game_tic_time = 1000 / ticks_per_second;
    game_tic_counter = 0;
    field_0x43c = 0;
    field_0x438 = timeGetTime();
    last_tic_time = timeGetTime(); //GetTickCount??
    field_0x440 = 0;
    return old_speed != game_speed;
}



BOOL MainWindow::UpdateClipCursor()
{
    //48cbc8
    return GetClipCursor(&clip_cursor_rect);
}



#ifdef A2CLIENT
void MainWindow::Proc_421()
{
#pragma message("REVERSE IT, it's not FULL");

    //48d49b Allods2.exe
    field_0x3e0.field_10 = 0;
    field_0x3e0.field_14.Empty();
    field_0x3e0.field_0c = 0;

    m_FameHall.ZeroFactors();

    CVisualObject* elm = vis_root->FindChild(0x3fc);
    if (elm)
        elm->MsgProc(0x445, 0, 0);

    FUN_00485a41();

    vis_root->AddChild(vis_mainmenu);
    vis_mainmenu->VMethod28();
    vis_root->VMethod9();
    field_0x460 = 0;
    dialogsMask |= 0x80;
    FUN_0048cb3c();

    elm = vis_map_context->FindChild(0x13);
    if (elm)
    {
        vis_map_context->RemoveChild(elm);
        delete elm;
    }

}
#else
void MainWindow::Proc_421()
{
    //48d49b

    field_0x3e0.field_10 = 0;
    field_0x3e0.field_14.Empty();
    FUN_0048cb3c();
    PostMessage(0x440, 0, 0);
}
#endif


void MainWindow::FUN_00485a41()
{
    //485a41
    vis_right_panel->RemoveAllChilds();
    vis_right_panel->AddChild(vis_charinfo);

    vis_root->RemoveAllChilds();
    vis_root->AddChild(vis_map_context);
    vis_root->AddChild(vis_right_panel);

    dialogsMask &= ~1;

    vis_right_panel->MsgProc(0x403, (uint32_t)vis_map_context, 0);
}


int32_t CGameSession::GetStringArray1Size()
{
    //438d10
    return characterRosterNames.GetSize();
}


void MainWindow::ResetItemCursor()
{
    //48cd44
    if (!field_0x408)
        return;

    if (item_cursor)
        delete item_cursor;

    item_cursor = nullptr;
    field_0x408 = nullptr;
    field_0x40c = -1;
    field_0x410 = -1;
}


//445aa9
DiplomacyEntry::~DiplomacyEntry()
{
    if (name)
        delete name;
}

void MainWindow::Proc_44c(CVisualObject* obj)
{
    field_0x460 = 1;

    vis_root->RemoveChild(obj);

    vis_map_context->field_0xe0 = 1;

    if (obj == vis_shop || obj == vis_shopdruid || obj == vis_shopkaarg)
    {
        vis_map_context->FUN_0041a8cc();
        vis_map_context->UpdateSelectionState();
        if (sessionMode != 2)
            FUN_00494c91();
    }
    else if (obj == vis_1200)
    {
        vis_map_context->RemoveChild(vis_1200);

        if (vis_1200->GetCloseCode() == 0x445)
        {

            CString str;
            vis_1200->WriteData(&str);
            if (str[0] == '=')
                vis_map_context->FUN_0041b2a4(str.Mid(1), 3, 0);
            else if (str[0] == '-')
            {
                int val = vis_1200->FUN_004972d0();
                if (val == -1)
                    vis_map_context->FUN_0041b2a4(str.Mid(1), 1, 0);
                else
                    vis_map_context->FUN_0041b2a4(str.Mid(vis_1200->FUN_00497310()), 2, val);
            }
            else
                vis_map_context->FUN_0041b2a4(str, 0, 0);
        }
    }
    else if (obj == vis_logownd)
        dialogsMask &= ~0x100;
    else if (obj == vis_famedocument)
        dialogsMask &= ~0x4000;
    else if (obj == vis_famehall)
    {
        dialogsMask &= ~0x1000;
        PostMessage(0x421, 0, 0);
    }
    else if (obj == vis_mainmenu)
        dialogsMask &= ~0x80;
    else if (obj == vis_town || obj == vis_towndruid || obj == vis_townkaarg)
    {}
    else if (obj == vis_tav || obj == vis_tavdruid || obj == vis_tavkaarg)
    {
        if (sessionMode != 2)
            FUN_00494c91();

        dialogsMask &= ~4;
        vis_map_context->UpdateSelectionState();
    }
    else if (obj == vis_chargen)
    { //charget 2step
        dialogsMask &= ~0x200;

        if (vis_chargen->GetCloseCode() == 0x446)
            ShowStartGameSetupForNewSession();
        else
        {
            m_GameSession.flags |= 4;

            if (sessionMode == 2)
            {
                vis_map_context->ConnectAndJoinSession();
                m_GameSession.SubmitCharacterSetupAndWaitForSelectedUnit();

                ScenarioSetVar(0x308, (m_GameSession.type & 0x40) != 0);
                ScenarioSetVar(0x30d, (m_GameSession.type & 0x80) != 0);

                if (g_Server)
                    g_Server->FUN_00497470(m_FameHall.GetDifficulty() + 1);

                PostMessage(0x42e, 0, 0);
            }
            else
            {
                m_GameSession.FUN_00493d8d();

                if (field_0x3e0.field_10 == 0)
                    PostMessage(0x451, 0, 0);
                else
                    PostMessage(0x489, 0, 0);
            }
        }
    }
    else if (obj == vis_startgame)
    { // chargen 1step
        dialogsMask &= ~0x800;

        if (vis_startgame->GetCloseCode() == 0x446)
        {
            if (!vis_map_context->field_0x9d0.IsEmpty())
            {
                vis_map_context->field_0x9d0.RemoveAll();

                CUnit *uni = vis_map_context->GetUnit_3f6c();
                if (uni)
                    delete uni;

                vis_map_context->SetUnit_3f6c(nullptr);
            }

            if (sessionMode == 2)
                PostMessage(0x421, 0, 0);
            else
                ShowCharacterLoaderDialog();
        }
        else
        {
            CString name = vis_startgame->GetName();
            strcpy(m_GameSession.character_name, name);

            if (!name.IsEmpty())
            {
                int pos = name.Find(':');
                if (pos != -1)
                {
                    name = name.Mid(pos + 1);

                    m_GameSession.color = atoi(name);

                    if (m_GameSession.color == 0 || m_GameSession.color > 16)
                        m_GameSession.color = 0;

                    name = vis_startgame->GetClan().Left(pos);
                }
            }

            if (!name.IsEmpty())
            {
                strcat(m_GameSession.character_name, "|");
                strcat(m_GameSession.character_name, name);
            }

            m_FameHall.SetDifficulty(vis_startgame->GetSelectedDifficulty());

            m_GameSession.type = vis_startgame->GetSelectedPortraitPlayerType() << 6; // 0x40 and 0x80 bits

            name = vis_startgame->GetName();
            strncpy(vis_map_context->GetUnit_3f6c()->str1, name, 12);

            name = vis_startgame->GetClan();
            strncpy(vis_map_context->GetUnit_3f6c()->str2, name, 12);

            m_GameSession.FUN_00494687();
            FUN_00491a49();
        }
    }
    else if (obj == vis_charsel)
    {
        dialogsMask &= ~0x400;

        if (vis_charsel->GetCloseCode() == 0x446)
        {
            if (!field_0x3e0.field_08.IsEmpty())
                PostMessage(WM_CLOSE, 0, 0);
            else
                PostMessage(0x421, 0, 0);
        }
        else if ((m_GameSession.flags & 4) == 0)
            ShowStartGameSetupForNewSession();
        else if (field_0x3e0.field_10 == 0)
            PostMessage(0x451, 0, 0);
        else
            PostMessage(0x489, 0, 0);
    }
    else if (obj == vis_globalmap)
        dialogsMask &= ~0x20;
    else if (obj == vis_credits)
    {
        dialogsMask &= ~0x40;

        if (FUN_00497490() == 0)
            CWnd::PostMessage(0x421, 0, 0);
        else
            CWnd::PostMessage(0x429, 0, 0);
    }
    else if (obj == vis_dropgold)
    {
        dialogsMask &= ~0x8;

        field_0x438 = timeGetTime();
        game_tic_counter = 0;
        field_0x43c = 0;
    }
    else if (obj == field_0x12c)
    {
        dialogsMask &= ~0x8;

        if (field_0x12c->GetCloseCode() == 0x447)
        {
            vis_map_context->FUN_0041d2da(1);
            PostMessage(0x421, 0, 0);
        }
        else
            PostMessage(0x42e, 0, 0);

        delete field_0x12c;
        field_0x12c = nullptr;
    }
    else if (obj == field_0x140)
    { //save game
        dialogsMask &= ~0x8;

        if (field_0x140->GetCloseCode() == 0x445)
            FUN_0048f905();

        if (dialogsMask == 1 && serverBootstrapEnabled != 0)
        {
            field_0x438 = timeGetTime();
            game_tic_counter = 0;
            field_0x43c = 0;
        }

        delete field_0x140;
        field_0x140 = nullptr;
    }
    else if (obj == field_0x13c)
    { //load game
        dialogsMask &= ~8;

        int32_t code = field_0x13c->GetCloseCode();
        if (code == 0x445)
            PostMessage(0x419, 0, 0);
        else if (g_Server == nullptr)
            PostMessage(0x421, 0, 0);
        else if (field_0x450 == 0xff)
            PostMessage(0x41e, 0, 0);

        if (dialogsMask == 1 && serverBootstrapEnabled != 0)
        {
            field_0x438 = timeGetTime();
            game_tic_counter = 0;
            field_0x43c = 0;
        }

        delete field_0x13c;
        field_0x13c = nullptr;

        if (code == 0x445)
            return;
    }
    else if (obj == field_0x34c)
    {
        dialogsMask &= ~8;

        int32_t code = field_0x34c->GetCloseCode();

        if (code == 0x44f)
        {
            serverBootstrapEnabled = 0;
            FUN_004903d0();
        }
        else if (code == 0x44d) 
            FUN_00491822();
        else if (code == 0x44e)
            FUN_004918ae();
        else
            PostMessage(0x421, 0, 0);
            
        delete field_0x34c;
        field_0x34c = nullptr;
        return;
    }
    else if (obj == field_0x350)
    {
        dialogsMask &= ~8;

        if (field_0x350->GetCloseCode() == 0x446)
        {
            if (sessionMode == 3)
                PostMessage(0x421, 0, 0);
            else
                ShowCharacterLoaderDialog();
        }

        delete field_0x350;
        field_0x350 = nullptr;
        return;
    }
    else if (obj == field_0x378)
    {
        dialogsMask &= ~8;

        if (field_0x378->GetCloseCode() == 0x446)
        {
            vis_root->MsgProc(0x446, 0, 0);
            PostMessage(0x45c, 0, 0);
        }
        else
            PostMessage(0x457, 0, 0);
        
        delete field_0x378;
        field_0x378 = nullptr;
        return;
    }
    else if (obj == field_0x37c)
    {
        dialogsMask &= ~8;

        if (field_0x37c->GetCloseCode() == 0x446)
        {
            if (hat_settings.ishat == 0)
            {
                vis_root->MsgProc(0x446, 0, 0);
                ShowCharacterLoaderDialog();
            }
            else if (g_CLlDriver.IsListen())
                ShowCharacterLoaderDialog();
            else
                PostMessage(0x487, 0, 0);
        }
        else if (hat_settings.ishat == 0)
        {
            g_NetStru1_local.SetLLDriver(&g_CLlDriver);
            g_CLlDriver.SetHlDriver(&g_NetStru1_local);
            g_CLlDriver.ResetProvider(3);
            PostMessage(0x452, 0, 0);
        }
        else
            PostMessage(0x441, 0, 0);
        
        delete field_0x37c;
        field_0x37c = nullptr;
        return;
    }
    else if (obj == field_0x3d0)
    {
        dialogsMask &= ~8;
        dialogsMask = 0;

        if (field_0x3d0->GetCloseCode() == 0x445)
            PostMessage(0x421, 0, 0);

        delete field_0x3d0;
        field_0x3d0 = nullptr;
    }
    else if (obj == field_0x144)
    {
        dialogsMask &= ~8;

        int32_t code = field_0x144->GetCloseCode();

        field_0x144 = nullptr;

        if (code == 0x445)
        {
            int j = 0;
            for (int i = 0; i < vis_map_context->field_0x9b8.GetSize(); i++)
            {
                MapPlayerData* mp = vis_map_context->field_0x9b8[i];
                if (mp != nullptr && (mp->flags & 1) == 0)
                {
                    DiplomacyEntry* visdp = (*field_0x348)[j];

                    uint16_t dpl = vis_map_context->my_main_unit->diplomacy[mp->index] & 0x68;

                    if (visdp->enemy != 0)
                        dpl |= 1;

                    if (visdp->ally != 0)
                        dpl |= 2;

                    if (visdp->see != 0)
                        dpl |= 0x10;

                    if (visdp->mute != 0)
                        dpl |= 4;

                    vis_map_context->my_main_unit->diplomacy[mp->index] = dpl;

                    vis_map_context->my_main_unit->diplomacy[vis_map_context->my_main_unit->index] = 0x3a;

                    j++;
                }
            }

            vis_map_context->FUN_0041a735();
        }

        delete obj;
    }
    else if (obj == field_0x3d4)
    {
        dialogsMask &= ~8;

        if (field_0x3d4->GetCloseCode() == 0x445)
        {
            DAT_00660f88 = ((VisListBox*)field_0x3d4->FindChild(2))->GetSelectedIndex();
            FUN_00491f7d(DAT_00660f88);
            PostMessage(0x43b, 0, 0);
        }
        else
            PostMessage(0x421, 0, 0);

        delete field_0x3d4;
        field_0x3d4 = nullptr; // MUST BE DONE? WAT?
    }
    else if (obj == field_0x124)
    {
        dialogsMask &= ~8;

        if (field_0x124->GetCloseCode() == 0x445)
            PostMessage(0x41e, 0, 0);
        else
            PostMessage(0x418, 0, 0);

        delete field_0x124;
        field_0x124 = nullptr; // MUST BE DONE? WAT?
    }
    else if (obj == field_0x3dc)
    {
        dialogsMask &= ~0x8000;

        delete field_0x3dc;
        field_0x3dc = nullptr; // MUST BE DONE? WAT?

        if ((dialogsMask & 1) == 0)
            vis_root->cursor_over_obj_last = vis_root->FindChild(1020);
    }
    else if ((dialogsMask & 8) != 0)
    {
        dialogsMask &= ~8;

        if (dialogsMask == 1 && sessionMode == 2)
        {
            field_0x438 = timeGetTime();
            game_tic_counter = 0;
            field_0x43c = 0;
        }

        delete obj;
    }

    if (dialogsMask == 0x80)
        g_Cursors[CURSOR_SELECT]->Use();

    vis_right_panel->MsgProc(0x408, 0, 0);

    if (dialogsMask != 1 || vis_map_context->field_0x80 != nullptr)
        vis_root->VMethod9();

    if (dialogsMask == 1 && DAT_006658d8.GetSize() != 0)
    {
        PostMessage(0x433, DAT_006658d8[0], 0);
        DAT_006658d8.RemoveAt(0);
    }
}


void MainWindow::ShowCharacterLoaderDialog()
{ //48cff7
    g_Cursors[CURSOR_WAIT]->Use();

    vis_charinfo->info_mode = 1;
    vis_root->AddChild(vis_charsel);

    vis_charsel->VMethod28();

    dialogsMask |= 0x400;
    
    vis_root->VMethod9();

    field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();
    return;
}


void MainWindow::FUN_00491a49()
{   //491a49
    g_Cursors[CURSOR_WAIT]->Use();

    vis_charinfo->info_mode = 1;

    vis_right_panel->MsgProc(0x403, (uint32_t)vis_map_context, 0);
    
    vis_map_context->GetUnit_3f6c()->VMethod1(1); // select

    vis_map_context->UpdateSelectionState();

    vis_root->AddChild(vis_chargen);
    vis_chargen->VMethod28();

    vis_root->VMethod9();

    field_0x460 = 0;
    dialogsMask |= 0x200;

    g_Cursors[CURSOR_DEFAULT]->Use();
}




void MainWindow::FUN_0048f905()
{  // 48f905
    if (sessionMode == 2)
    {
        CString local_2a4 = field_0x148.filename;
        g_Server->sub_4ED2DC(&local_2a4);

        field_0x438 = timeGetTime();
        game_tic_counter = 0;
        field_0x43c = 0;

        CFile local_13c;
        local_13c.Open(field_0x148.filename, CFile::modeReadWrite);
        local_13c.Seek(0, CFile::end);
        local_13c.Write(field_0x148.title, 0x100);
        
        int inbattle = dialogsMask & 1;

        RegFile local_180;
        local_180.SetInt("CurrentState", "InBattle", inbattle);
        local_180.SetString("Character", "Name", m_GameSession.character_name);
        local_180.SetInt("GameOptions", "Wimpy", vis_map_context->wimpy);
        local_180.SetInt("GameOptions", "ShowHP", vis_map_context->show_hp);
        local_180.SetInt("GameOptions", "FlyingHP", vis_map_context->flying_hp);
        local_180.SetInt("GameOptions", "Formation", vis_map_context->formation);
        local_180.SetInt("GameOptions", "Speed", game_speed);
        local_180.SetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);
        local_180.SetInt("View", "X", vis_map_context->view_x);
        local_180.SetInt("View", "Y", vis_map_context->view_y);

        local_180.SetInt("SpellBook", "IsOpen", vis_map_context->IsBookOpen());
        local_180.SetInt("SpellBook", "Pressed", vis_spellbook->pressed);

        CArray<uint32_t> local_25c;
        local_180.SetInt32Array("SpellBook", "Shortcuts", local_25c);

        CArray<uint16_t> local_12c;
        CArray<uint16_t> local_248[10];

        if (!vis_map_context->field_0x9d0.IsEmpty())
        {
            for (POSITION pos = vis_map_context->field_0x9d0.GetStartPosition(); pos != nullptr;)
            {
                uint16_t key;
                CGameObject* obj;

                vis_map_context->field_0x9d0.GetNextAssoc(pos, key, obj);

                if (obj->IsSelected() != 0)
                    local_12c.Add(key);

                for (int i = 0; i < 10; i++)
                {
                    if (obj->FUN_0041f1c0(i) != 0)
                        local_248[i].Add(key);
                }
            }
        }

        local_180.SetInt16Array("Objects", "Selection", local_12c);

        char buf[256];

        for (int i = 0; i < 10; i++)
        {
            if (local_248[i].GetSize() != 0)
            {
                sprintf(buf, "Group%d", i);
                local_180.SetInt16Array("Objects", buf, local_248[i]);
            }
        }

        local_180.SetInt("Inventory", "IsOpen", vis_map_context->IsBagOpen());

        if (inbattle)
        {
            CArray<uint16_t> local_27c;

            local_180.SetInt("Projectiles", "FreeIndex", vis_map_context->field_0xa24);

            if (!vis_map_context->field_0x9ec.IsEmpty())
            {
                for (POSITION pos = vis_map_context->field_0x9ec.GetStartPosition(); pos != nullptr;)
                {
                    uint16_t key;
                    CGameObject* obj;
                    vis_map_context->field_0x9ec.GetNextAssoc(pos, key, obj);

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

        if (inbattle)
        {
            CArray<uint32_t> local_2a0;

            uint16_t* land = vis_map_context->field_0x80->GetLandscape();
            int numcells = vis_map_context->field_0x80->GetWidth() * vis_map_context->field_0x80->GetHeight();

            uint16_t state = *land & 0x8000;
            local_180.SetInt("Fog", "FirstState", state);

            int idx = 0;
            int num = 1;
            while (true)
            {
                idx++;
                land++;

                if (idx >= numcells)
                {
                    local_2a0.Add(num);
                    break;
                }
                
                if (state == (*land & 0x8000))
                    num++;
                else
                {
                    state = *land & 0x8000;
                    local_2a0.Add(num);
                    num = 1;
                }
            }

            local_180.SetInt32Array("Fog", "Data", local_2a0);
        }

        local_180.WriteToFile(&local_13c);

        m_FameHall.FUN_004ac706(&local_13c);

        ScenarioSave(&local_13c);

        for (int i = 0; i < 9; i++)
            m_GameSession.shortcuts[i].WriteToFile(&local_13c);

        local_13c.Close();
    }
    else
    {
        vis_map_context->FUN_0041afcf(field_0x148.filename);
    }
}


void MainWindow::FUN_004903d0()
{ // 4903d0

    sessionMode = 0;

    g_NetStru1_local.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_local);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-waitforever"))
        g_CLlDriver.SetTimeout(-1);

    if (strstr(cmdline, "-timeout"))
        g_CLlDriver.SetTimeout(g_CmdTimeout);

    if (vis_map_context->ConnectAndJoinSession() == 0)
    {
        g_CLlDriver.Close();
        PostMessage(0x41d, 0, 0);
    }
    else
    {
        if (m_GameSession.field_0x10c == 0)
        {
            VisMessageBoxWithList* mwnd = new VisMessageBoxWithList(1, 64, 100, 428, 594, TxtFile::AllLines[210], nullptr, 4);
            ModalScreen(mwnd);

            if (mwnd->GetCloseCode() != 0x447)
            {
                PostMessage(0x41d, 0, 0);
                return;
            }
        }

        if (field_0x454 == 0)
            PostMessage(0x459, 0, 0);
        else
            PostMessage(0x457, 0, 0);
    }
}

void MainWindow::FUN_00491822()
{  //491822
    sessionMode = 3;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2)
        g_CLlDriver.StartServer(g_ServerConfig.max_players, m_GameSession.character_name, nullptr);

    FUN_0048ca7e(0);

    PostMessage(0x459, 0, 0);
}

int MainWindow::FUN_0048ca7e(int mode)
{ //48ca7e
    DAT_006658d8.RemoveAll();

    g_Server = new Server();
    if (g_Server->Start(mode) == 0)
    {
        serverBootstrapEnabled = 1;
        return 0;
    }
    else
    {
        serverBootstrapEnabled = 0;
        return 1;
    }
}


void MainWindow::FUN_004918ae()
{ // 4918ae

    sessionMode = 1;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-timeout"))
        g_CLlDriver.SetTimeout(g_CmdTimeout);

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2)
        g_CLlDriver.StartServer(g_ServerConfig.max_players, m_GameSession.character_name, nullptr);

    FUN_0048ca7e(sessionMode);

    if (vis_map_context->ConnectAndJoinSession() == 0)
    {
        vis_root->MsgProc(0x446, 0, 0);
        PostMessage(0x45c, 0, 0);
    }
    else
        PostMessage(0x459, 0, 0);
}


void MainWindow::FUN_00491f7d(int32_t vid_id)
{ //491f7d
    printf("Video play FUN_00491f7d not implemented yet: %d\n", vid_id);
}


LRESULT MainWindow::NewWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{  // 486c6c

    switch (message)
    {
    default:
        if (vis_root && vis_root->MsgProc(message, wParam, lParam) != 0)
            return DefWindowProc(message, wParam, lParam);
        break;

    case WM_KEYDOWN:
        break;
    case WM_KEYUP:
        vis_root->MsgProc(WM_KEYUP, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        if (vis_root->MsgProc(WM_MOUSEMOVE, wParam, lParam) != 0)
            SetCursorPos(g_mousept.GetX(), g_mousept.GetY());

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
        if (dialogsMask == 1)
            PopUpScreen(new IngameMenu(1, 100, 60, 440, 400, nullptr, 0, CRect(0, 0, 240, 40)));
        break;

    case 0x418:
        field_0x148.title[0] = 0;
        field_0x148.filename[0] = 0;
        field_0x13c = new LoadGameWindow(1, 100, 30, 600, 450, 0, &field_0x148);
        PopUpScreen(field_0x13c);
        break;

    case 0x419:
        if (dialogsMask == 1)
            g_Server->sub_4F1E2A();

        FUN_0048cb3c();

        if (dialogsMask == 0 && vis_root->FindChild(1020) != nullptr)
        {
            vis_map_context->FUN_0041d2da(1);

            ScenarioLocation* cur_location = ScenarioGetCurrentLocation();
            if (cur_location != nullptr)
            {
                int loc_type = cur_location->GetId();
                if (loc_type == 1)
                    vis_town->MsgProc(0x445, 0, 0);
                else if (loc_type == 2)
                    vis_townkaarg->MsgProc(0x445, 0, 0);
                else if (loc_type == 3)
                    vis_towndruid->MsgProc(0x445, 0, 0);
            }
        }
        FUN_0048f79d();
        break;

    case 0x41a:
        if (dialogsMask == 1 || dialogsMask == 0)
        {
            field_0x148.title[0] = 0;
            field_0x148.filename[0] = 0;
            field_0x140 = new SaveGameWindow(1, 100, 30, 600, 450, 0, &field_0x148);
            PopUpScreen(field_0x140);
        }
        break;

    case 0x41b:
        if (dialogsMask == 1 || dialogsMask == 0)
            PopUpScreen(new GameOptionsWindow(1, 20, 0, 620, 480));
        break;

    case 0x41c:
        if (dialogsMask == 1)
            PopUpScreen(new EndGameMenu(1, 100, 100, 440, 340, nullptr, 0, CRect(0, 0, 300, 50), field_0x3f8));
        else if (dialogsMask == 0)
            PopUpScreen(new ExitGameMenu(1, 100, 100, 440, 340, CRect(0, 0, 300, 70)));
        break;

    case 0x41d:
        if (sessionMode == 1 || sessionMode == 3)
        {
            if (sessionMode == 1)
                vis_map_context->FUN_0041d2da(0);
            else
                vis_map_context->FUN_0041d2da(1);

            if ((dialogsMask & 1) != 0)
                FUN_0048f6f7();
            
            g_Server->sub_4F1E2A();
            PostMessage(0x459, 0, 0);
        }
        else if (sessionMode == 0)
        {
            vis_map_context->FUN_0041d2da(1);

            if ((dialogsMask & 1) != 0)
                FUN_0048f6f7();

            g_CLlDriver.Close();

            field_0x438 = 0x7fffffff;
            last_tic_time = 0x7fffffff;

            PostMessage(0x452, 0, 0);
        }
        else if (field_0x3f8 == 0)
        {
            g_Server->sub_4F1E2A();
            PostMessage(0x421, 0, 0);
        }
        else
        {
            m_FameHall.AddMissionElapsedTime(g_Server->tick / 16);
            vis_map_context->CleanupCompletedMissionMapState();

            if ((dialogsMask & 1) != 0)
                FUN_0048f6f7();
            
            if (g_Server)
                g_Server->sub_4F1E2A();
            
            FUN_00485a41();

            vis_globalmap->SetTravelOrigin(ScenarioGetCurrentLocation()->rect.TopLeft());

            int32_t loc;
            int32_t local_894 = ScenarioLeaveLocation(&loc);

            CDWordArray local_8ac;
            int32_t var = ScenarioGetVar(0x305);
            if (var != 0)
            {
                local_8ac.Add(var);
                g_Server->FUN_00501b9e(ScenarioGetVar(0x300), local_8ac);
                vis_map_context->ProcessPackets(0);
            }

            if (loc > -1)
                FUN_00491f7d(loc);

            if (FUN_00497490() == 0)
            {
                if (local_894 != 0)
                    vis_map_context->SendAdjustPlayerGoldAction(local_894);

                if (ScenarioIsTownAvailable(0) == 0)
                {
                    CList<ScenarioLocation*>* locs = ScenarioGetAvailableLocations();
                    ScenarioEnterLocation(locs->GetHead());
                    PostMessage(0x468, 0, 0);
                }
                else
                {
                    vis_globalmap->umoirMapMode = 0;
                    ShowGlobalMapDialog();
                }
            }
            else
            {
                m_FameHall.SubmitScore();
                vis_map_context->FUN_0041d2da(1);
                PostMessage(MSG_428, 0 ,0);
            }
        }
        break;

    case 0x41e:
        g_Server->sub_4F1E2A();
        PostMessage(MSG_421, 0, 0);
        break;

    case 0x41f:
        PopUpScreen(new TownMenuListDialogVisualObject(1, 100, 100, 440, 340, CRect(0, 0, 240, 40)));
        break;

    case 0x420:
        PopUpScreen(new QuestObjectivesHeaderDialogVisualObject(1, 20, 0, 620, 500));
        break;

    case 0x421:
        if (dialogsMask == 0)
            Proc_421();
        break;

    case 0x422:
        if (dialogsMask == 1 || (dialogsMask == 0 && g_Server != nullptr))
        {
            PopUpScreen(new SoundPreferencesDialogVisualObject(1, 100, 30, 640, 450, nullptr, &g_SoundSettings));
        }
        else if (dialogsMask == 0)
            PostMessage(MSG_487, 0, 0);
        break;

    case 0x425:
        ReadFileToString("main\\text\\town.txt", &g_MissionText);
        ScenarioNewGame();
        serverLoopCounter = 0;
        serverBootstrapEnabled = 1;
        InitNewCampaignSession();
        m_FameHall.ZeroFactors();
        vis_globalmap->RebuildScenarioLocations();
        m_FameHall.SetDifficulty(1);

        if (sessionMode == 3)
            PostMessage(MSG_42f, 0, 0);
        else
            m_GameSession.RefreshCharacterRosterFiles(1);
            m_GameSession.InitializeNewCharacterSession(0, nullptr);
            ShowStartGameSetupForNewSession();
        break;

    case 0x426:
        ReadFileToString("main\\text\\town.txt", &g_MissionText);
        serverLoopCounter = 0;
        sessionMode = 0;
        ShowCharacterLoaderDialog();
        break;
    }

    return CWnd::WindowProc(message, wParam, lParam);
}


void MainWindow::PopUpScreen(VisScreen* screen)
{ //48d26a
    if (screen == field_0x3dc)
        dialogsMask |= 0x8000;
    else
        dialogsMask |= 8;

    vis_root->AddChild(screen);

    screen->VMethod26();
    screen->VMethod28();

    LockSurface2();

    ShadowRect(g_ScreenSize, 3);

    UnlockSurface2();

    FlushScreen();

    screen->VMethod9();

    field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();

    if (g_mousept.GetSelectState() != 0)
    {
        g_mousept.ResetStates();

        UpdateCursorClip();
    }
}

void MainWindow::ShowGlobalMapDialog()
{ //48d34b
    g_Cursors[CURSOR_WAIT]->Use();

    vis_root->AddChild(vis_globalmap);
    vis_globalmap->VMethod28();
    vis_root->VMethod9();

    field_0x460 = 0;
    dialogsMask |= 0x10;

    if (g_SoundSettings.field_0x20 != 0)
    {
        CStringArray playlist;
        playlist.Add("music\\map.wav");

        music_player->SetPlayList(playlist);
        music_player->Play();
    }

    g_Cursors[CURSOR_DEFAULT]->Use();
}

void MainWindow::FUN_0048f79d()
{ // 48f79d
    g_mousept.DisableHint();

    strcpy(m_GameSession.character_name, "Self");
    field_0x400 = 1;
    sessionMode = 2;

    FUN_0048ca7e(sessionMode);

    g_Server->sub_4EDB83(field_0x148.filename);

    if (!vis_map_context->ConnectAndJoinSession())
        PostMessage(0x421, 0, 0);
    else if (GetSaveFileInBattle())
        FUN_0048e502(1);
    else
    {
        vis_map_context->FUN_0041b10f();
        g_Server->FUN_0050907e();

        while (g_NetStru1_local.GetClientsPktNum())
        {
            vis_map_context->ProcessPackets(0x64);
        }

        FUN_0048df44();
        vis_map_context->FUN_0041b064(0, 0);
        PostMessage(0x42e, 0, 0);
    }
    g_mousept.EnableHint();
    vis_globalmap->RebuildScenarioLocations();
}

int MainWindow::GetSaveFileInBattle()
{ // 48de6f
    File2 f;
    f.Open(field_0x148.filename, CFile::modeRead);
    f.Seek(4, 0);

    int32_t offset;
    f.Read(&offset, 4);
    f.Seek(offset + 0x100, 0);

    RegFile reg;
    reg.ReadFromFile(&f);
    f.Close();

    return reg.GetInt("CurrentState", "InBattle", 1);
}

int MainWindow::FUN_0048e502(int mode)
{ // 48e502

    vis_map_context->field_0x80 = nullptr;
    g_Cursors[CURSOR_WAIT]->Use();

    if (g_SoundSettings.field_0x20 != 0)
        music_player->OnEndTrack();

    if (sessionMode == 3)
    {
        CRect r = g_ScreenSize;
        r.bottom -= 72;

        vis_map_context->msglog.SetRect(r);

        LockSurface2();
        FillRectColorSimple(g_ScreenSize.left, g_ScreenSize.top, g_ScreenSize.right, g_ScreenSize.bottom, 0);
        UnlockSurface2();

        g_Server->field41_0x1b0 = 1;

        field_0x3d0 = new VisServerScreen(1, 0, 0, g_ScreenSize.right, g_ScreenSize.bottom, &vis_map_context->msglog);
        PopUpScreen(field_0x3d0);
    }

    char buf[128];
    if (sessionMode == 2 && mode == 0)
        sprintf(buf, "%d.alm", ScenarioGetCurrentLocation()->GetId());
    else
        strcpy(buf, current_map_name);

    if (mode == 0 && serverBootstrapEnabled != 0)
    {
        g_Server->field40_0x1ac = 0;
        g_Server->sub_4F1471(buf);
    }

    if (sessionMode != 3)
    {
        FUN_00485969();
        FUN_0047a5e6();
        FUN_0047efdf();
        FUN_0047e612();

        field_0x404 = mode;

        vis_map_context->FUN_0041cad0(mode);
        vis_map_context->FUN_0041aaaa(vis_map_context->wimpy);
        vis_map_context->FUN_0041abd2(vis_map_context->formation);
        vis_map_context->FUN_0041ab74();

        if (serverBootstrapEnabled != 0)
            g_Server->ServerTic();

        uint32_t tm = timeGetTime();
        while (true)
        {
            if (vis_map_context->field_0x80)
            {
                field_0x404 = 0;
                break;
            }

            MSG msg;
            if (PeekMessageA(&msg, nullptr, 0, 0, 1))
            {
                if (msg.message == WM_QUIT)
                    return 0;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            while (g_NetStru1_local.GetClientsPktNum() == 0)
            {
                if (timeGetTime() - tm > g_CmdTimeout)
                {
                    INT_00660f8c = 0x1005;
                    return 0;
                }
                g_NetStru1_local.ProcessConnections();
                g_mousept.Update();
            }

            if (vis_map_context->ProcessPackets(0x64) == 0)
                return 0;
        }
    }

    if (sessionMode == 2 && mode != 0)
    {
        File2 fil;
        fil.Open(field_0x148.filename, CFile::modeRead);
        fil.Seek(4, 0);
        int32_t offset;
        fil.Read(&offset, 4);
        fil.Seek(offset + 0x100, 0);

        RegFile reg;

        reg.ReadFromFile(&fil);
        m_FameHall.FUN_004ac7a2(&fil);
        ScenarioLoad(&fil);

        for (int i = 0; i < 9; i++)
            m_GameSession.shortcuts[i].ReadFromFile(&fil);

        fil.Close();

        int inbattle = reg.GetInt("CurrentState", "InBattle", 1);

        reg.GetSizedString("Character", "Name", "No name", m_GameSession.character_name, sizeof(m_GameSession.character_name));
        m_GameSession.character_name[31] = 0;

        vis_map_context->wimpy = reg.GetInt("GameOptions", "Wimpy", vis_map_context->wimpy);
        vis_map_context->show_hp = reg.GetInt("GameOptions", "ShowHP", vis_map_context->show_hp);
        vis_map_context->flying_hp = reg.GetInt("GameOptions", "FlyingHP", vis_map_context->flying_hp);
        vis_map_context->formation = reg.GetInt("GameOptions", "Formation", vis_map_context->formation);
        game_speed = reg.GetInt("GameOptions", "Speed", game_speed);
        g_settings.ShowTimeFlow = reg.GetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);
        vis_map_context->view_x = reg.GetInt("View", "X", vis_map_context->view_x);
        vis_map_context->view_y = reg.GetInt("View", "Y", vis_map_context->view_y);

        if (reg.GetInt("SpellBook", "IsOpen", 0) == 0)
        {
            if (vis_map_context->IsBagOpen() != 0)
                vis_map_context->FUN_0041b636();
        }
        else
        {
            if (vis_map_context->IsBagOpen() == 0)
                vis_map_context->FUN_0041b509();
        }

        vis_spellbook->pressed = reg.GetInt("SpellBook", "Pressed", vis_spellbook->pressed);

        CArray<uint16_t> sel_ids;
        CArray<uint16_t> grp_ids;

        reg.GetInt16Array("Objects", "Selection", &sel_ids);

        for (int i = 0; i < sel_ids.GetSize(); i++)
        {
            CGameObject* obj;
            if (vis_map_context->field_0x9d0.Lookup(sel_ids[i], obj))
                obj->VMethod1(1);
        }

        for (int i = 0; i < 10; i++)
        {
            sprintf(buf, "Group%d", i);

            grp_ids.RemoveAll();
            reg.GetInt16Array("Objects", buf, &grp_ids);

            for (int j = 0; j < grp_ids.GetSize(); j++)
            {
                CGameObject* obj;
                if (vis_map_context->field_0x9d0.Lookup(grp_ids[i], obj) != 0)
                    obj->FUN_0041f180(i);
            }
        }

        if (reg.GetInt("Inventory", "IsOpen", 0) == 0)
        {
            if (vis_map_context->FUN_0041b495() != 0)
                vis_map_context->FUN_0041b40e();
        }
        else
        {
            if (vis_map_context->FUN_0041b495() == 0)
                vis_map_context->FUN_0041b381();
        }

        CArray<uint16_t> proj_ids;

        vis_map_context->field_0xa24 = reg.GetInt("Projectiles", "FreeIndex", 0);
        reg.GetInt16Array("Projectiles", "IDs", &proj_ids);

        for (int i = 0; i < proj_ids.GetSize(); i++)
        {
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
            proj->pMapObject = vis_map_context;
            proj->map_player = vis_map_context->my_main_unit;

            proj->unit_id = proj_ids[i];
            vis_map_context->field_0x9ec[proj->unit_id] = proj;
            proj->FUN_0046190d();
        }

        CArray<uint32_t> fog_data;
        if (inbattle != 0)
        {
            uint16_t* land = vis_map_context->field_0x80->GetLandscape();
            uint32_t st = reg.GetInt("Fog", "FirstState", 0);
            reg.GetInt32Array("Fog", "Data", &fog_data);

            for (int i = 0; i < fog_data.GetSize(); i++)
            {
                for(int j = 0; j < fog_data[i]; j++)
                {
                    *land |= st;
                    land++;
                }
                st ^= 0x8000;
            }
        }
    }

    vis_right_panel->MsgProc(0x408, 0, 0);
    vis_root->VMethod9();

    if (sessionMode != 3)
    {
        FUN_00494c91();
        field_0x450 = -1;
        field_0x3f8 = 0;
        field_0x3fc = 0;
    }

    if (sessionMode == 2)
        SetSpeed(game_speed);
    else
        SetSpeed(g_ServerConfig.game_speed);

    if (sessionMode == 2)
    {
        sprintf(buf, "main\\text\\mission%d.txt", ScenarioGetCurrentLocation()->GetId());
        ReadFileToString(buf, &g_MissionText);
        MissionGetBriefing(&g_MissionBriefing);

        g_MissionFailures.RemoveAll();

        CString tmp;
        for (int i = 2; ; i++)
        {
            MissionGetFailure(i, &tmp);
            if (tmp.IsEmpty())
            {
                if (i > 4)
                    break;

                tmp = txt_main.GetLine(0x118 + i);
            }
            g_MissionFailures.Add(tmp);
        }

        g_MissionSubjs.RemoveAll();
        for (int i = 0; ; i++)
        {
            MissionGetSubj(i, &tmp);
            if (tmp.IsEmpty())
                break;
            g_MissionSubjs.Add(tmp);
        }

        if (mode == 0)
            PostMessage(0x442, 0, 0);
    }
    else
    {
        ReadFileToString("main\\text\\quest.txt", &g_MissionText);
    }

    g_Cursors[CURSOR_DEFAULT]->Use();
    return 1;
}

void MainWindow::FUN_00485969()
{ // 485969
    vis_right_panel->RemoveAllChilds();
    vis_right_panel->AddChild(vis_minimap);
    vis_right_panel->AddChild(vis_ordertoolbar);
    vis_right_panel->AddChild(vis_charinfo);
    vis_right_panel->AddChild(vis_sidestatus);

    vis_root->RemoveAllChilds();
    vis_map_context->msglog.Clear();
    vis_root->AddChild(vis_map_context);
    vis_root->AddChild(vis_right_panel);

    dialogsMask = 1;
}

void MainWindow::FUN_0047a5e6()
{  // 47a5e6
    RegFile reg("graphics\\objects\\objects.reg");
    int count = reg.GetInt("Global", "FileCount", 0);
    int obj_count = reg.GetInt("Global", "ObjectCount", 0);

    for (int i = 0; i < count; i++)
    {
        char buf[32];
        sprintf(buf, "File%d", i);

        char flname[256];
        reg.GetSizedString("Files", buf, "", flname, sizeof(flname));

        g_GfxFiles.Add(new GfxFile(flname));
    }
    
    char buf[64];
    char parnt[64];
    for (int i = 0; i < obj_count; i++)
    {
        g_mousept.Update();
        
        sprintf(buf, "Object%d", i);

        GfxObject* obj = new GfxObject();

        obj->id = reg.GetInt(buf, "ID", -1);
        obj->sn = i;

        int parentID = reg.GetInt(buf, "Parent", -1);
        if (parentID != -1)
            sprintf(parnt, "Object%d", g_GfxObjects[parentID]->sn);

        obj->file = reg.GetInt(buf, "File", -1);

        if (parentID == -1)
            obj->index = reg.GetInt(buf, "Index", -1);
        else
            obj->index = reg.GetInt(buf, "Index", g_GfxObjects[parentID]->index);

        if (parentID == -1)
            obj->phases = reg.GetInt(buf, "Phases", -1);
        else
            obj->phases = reg.GetInt(buf, "Phases", g_GfxObjects[parentID]->phases);

        if (parentID == -1)
            obj->width = reg.GetInt(buf, "Width", -1);
        else
            obj->width = reg.GetInt(buf, "Width", g_GfxObjects[parentID]->width);

        if (parentID == -1)
            obj->height = reg.GetInt(buf, "Height", -1);
        else
            obj->height = reg.GetInt(buf, "Height", g_GfxObjects[parentID]->height);

        if (parentID == -1)
            obj->center_x = reg.GetInt(buf, "CenterX", -1);
        else
            obj->center_x = reg.GetInt(buf, "CenterX", g_GfxObjects[parentID]->center_x);

        if (parentID == -1)
            obj->center_y = reg.GetInt(buf, "CenterY", -1);
        else
            obj->center_y = reg.GetInt(buf, "CenterY", g_GfxObjects[parentID]->center_y);

        if (parentID == -1)
            obj->fire_object = reg.GetInt(buf, "FireObject", -1);
        else
            obj->fire_object = reg.GetInt(buf, "FireObject", g_GfxObjects[parentID]->fire_object);

        if (parentID == -1)
            obj->dead_object = reg.GetInt(buf, "DeadObject", -1);
        else
            obj->dead_object = reg.GetInt(buf, "DeadObject", g_GfxObjects[parentID]->dead_object);

        obj->in_map_editor = reg.GetInt(buf, "InMapEditor", 0);

        memset(obj->desc_text, 0, 32);

        reg.GetSizedString(buf, "DescText", " ", obj->desc_text, 31);

        CArray<uint32_t> animationTime;
        CArray<uint32_t> animationFrame;

        reg.GetInt32Array(buf, "AnimationTime", &animationTime);
        if (parentID != -1)
        {
            if (animationTime.GetSize() == 0)
                reg.GetInt32Array(parnt, "AnimationTime", &animationTime);
        }

        reg.GetInt32Array(buf, "AnimationFrame", &animationFrame);
        if (parentID != -1)
        {
            if (animationFrame.GetSize() == 0)
                reg.GetInt32Array(parnt, "AnimationFrame", &animationFrame);
        }

        for (int j = 0; j < animationTime.GetSize() && j < animationFrame.GetSize(); j++)
        {
            for (int frm = 0; frm < animationTime[j]; frm++)
                obj->frames.Add(animationFrame[j]);
        }

        obj->frame_count = obj->frames.GetSize();
        
        g_GfxObjects.Add(obj);
    }
}

void MainWindow::FUN_0047efdf()
{ //47efdf
    RegFile reg("graphics\\structures\\structures.reg");

    char buf[64];
    char fil[256];

    int count = reg.GetInt("Global", "Count", 0);
    for (int i = 0; i < count; i++)
    {
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
        bld->field_0x38 = 0;

        reg.GetSizedString(buf, "Picture", "", bld->picture, sizeof(bld->picture));

        bld->indestructible = reg.GetInt(buf, "Indestructible", 0);

        reg.GetSizedString(buf, "DescText", "", bld->desc_text, sizeof(bld->desc_text));

        bld->variable_size = reg.GetInt(buf, "VariableSize", 0);
        bld->usable = reg.GetInt(buf, "Usable", 0);
        bld->light_radius = reg.GetInt(buf, "LightRadius", 0);
        bld->light_pulse = reg.GetInt(buf, "LightPulse", 0);
        bld->flat = reg.GetInt(buf, "Flat", 0);

        if (bld->phases > 1)
        {
            CArray<uint32_t> anim_time;
            CArray<uint32_t> anim_frame;
            reg.GetInt32Array(buf, "AnimTime", &anim_time);
            reg.GetInt32Array(buf, "AnimFrame", &anim_frame);

            for(int j = 0; j < anim_time.GetSize() && j < anim_frame.GetSize(); j++)
            {
                for (int frm = 0; frm < anim_time[j]; frm++)
                    bld->frames.Add(anim_frame[j]);
            }

            bld->frame_count = bld->frames.GetSize();

            bld->anim_mask = new char[1 + bld->tile_width * bld->full_height];
            reg.GetSizedString(buf, "AnimMask", "", bld->anim_mask, bld->tile_width * bld->full_height + 1);

            int sz = strlen(bld->anim_mask);
            for (int j = 0; j < sz; j++)
            {
                if (bld->anim_mask[j] != '-')
                    bld->field_0x38++;
            }
        }
        g_StructuresInfo.SetAtGrow(bld->id, bld);
    }
}

void MainWindow::FUN_0047e612()
{ //47e612

    RegFile reg("graphics\\projectiles\\projectiles.reg");
    int count = reg.GetInt("Global", "Count", 0);

    for (int i = 0; i < count; i++)
    {
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

    for (int i = 0; i < 2; i++)
    {
        char sbuf[128];
        sprintf(sbuf, "graphics\\projectiles\\smoke%d\\sprites.16a", i);

        g_spr_smoke[i] = new CA16(sbuf);
        g_spr_smoke[i]->ResetPalette(16, 4, 0);
    }
}


void MainWindow::FUN_0048df44()
{ //48df44
    File2 fil;

    fil.Open(field_0x148.filename, CFile::modeRead);
    fil.Seek(4, 0);

    int32_t offset;
    fil.Read(&offset, 4);
    fil.Seek(offset + 0x100, 0);

    RegFile reg;

    reg.ReadFromFile(&fil);

    m_FameHall.FUN_004ac7a2(&fil);

    ScenarioLoad(&fil);

    for (int i = 0; i < 9; i++)
        m_GameSession.shortcuts[i].ReadFromFile(&fil);

    fil.Close();

    int local_128 = reg.GetInt("CurrentState", "InBattle", 1);

    reg.GetSizedString("Character", "Name", "No name", m_GameSession.character_name, sizeof(m_GameSession.character_name));
    m_GameSession.character_name[31] = 0;

    vis_map_context->wimpy = reg.GetInt("GameOptions", "Wimpy", vis_map_context->wimpy);
    vis_map_context->show_hp = reg.GetInt("GameOptions", "ShowHP", vis_map_context->show_hp);
    vis_map_context->flying_hp = reg.GetInt("GameOptions", "FlyingHP", vis_map_context->flying_hp);
    vis_map_context->formation = reg.GetInt("GameOptions", "Formation", vis_map_context->formation);

    game_speed = reg.GetInt("GameOptions", "Speed", game_speed);
    g_settings.ShowTimeFlow = reg.GetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);

    vis_map_context->view_x = reg.GetInt("View", "X", vis_map_context->view_x);
    vis_map_context->view_y = reg.GetInt("View", "Y", vis_map_context->view_y);

    reg.GetInt("SpellBook", "IsOpen", 0);

    vis_spellbook->pressed = reg.GetInt("SpellBook", "Pressed", vis_spellbook->pressed);

    CArray<uint16_t> sel_objs;
    CArray<uint16_t> grp_objs;

    reg.GetInt16Array("Objects", "Selection", &sel_objs);

    for (int i = 0; i < sel_objs.GetSize(); i++)
    {
        CGameObject* obj = nullptr;
        if (vis_map_context->field_0x9d0.Lookup(sel_objs[i], obj) && obj)
            obj->VMethod1(1);
    }

    for (int i = 0; i < 10; i++)
    {
        char buf[256];
        sprintf(buf, "Group%d", i);
        
        grp_objs.RemoveAll();
        reg.GetInt16Array("Objects", buf, &grp_objs);

        for(int j = 0; j < grp_objs.GetSize(); j++)
        {
            CGameObject* obj = nullptr;
            if (vis_map_context->field_0x9d0.Lookup(grp_objs[j], obj))
                obj->FUN_0041f180(i);
        }
    }

    vis_map_context->UpdateSelectionState();

    field_0x450 = -1;
    field_0x3f8 = 0;
    field_0x3fc = 0;

    SetSpeed(game_speed);
}



void MainWindow::FUN_0048f6f7()
{ //48f6f7

    vis_map_context->FUN_0041c39c();
    if (g_SoundSettings.field_0x20 != 0)
        music_player->OnEndTrack();

    FUN_0047ad28();

    for (int i = 0; i < g_UnitGfxFiles.GetSize(); i++)
        g_UnitGfxFiles[i]->Deinit();
    
    FUN_00478130();
    FUN_0047eab6();
    FUN_0047f5e4();

    dialogsMask &= ~1;
}

void MainWindow::FUN_0047eab6()
{ //47eab6
    for (int i = 0; i < g_ProjectileInfos.GetSize(); i++)
    {
        ProjectileInfo* inf = g_ProjectileInfos[i];
        if (inf)
            delete inf;
    }
    g_ProjectileInfos.RemoveAll();

    if (g_pal_projectiles)
    {
        delete g_pal_projectiles;
        g_pal_projectiles = nullptr;
    }

    if (g_pal_projectile_)
    {
        delete g_pal_projectile_;
        g_pal_projectile_ = nullptr;
    }

    for (int i = 0; i < 2; i += 1)
    {
        if (g_spr_smoke[i])
        {
            delete g_spr_smoke[i];
            g_spr_smoke[i] = nullptr;
        }
    }
}

void MainWindow::FUN_0047f5e4()
{ //47f5e4
    for (int i = 0; i < g_StructuresInfo.GetSize(); i++)
    {
        StructureInfo* inf = g_StructuresInfo[i];
        if (inf)
            delete inf;
    }
    g_StructuresInfo.RemoveAll();
}


void MainWindow::FUN_0047ad28()
{ //47ad28
    for (int i = 0; i < g_GfxFiles.GetSize(); i++)
        delete g_GfxFiles[i];
    for (int i = 0; i < g_GfxObjects.GetSize(); i++)
        delete g_GfxObjects[i];
    g_GfxFiles.RemoveAll();
    g_GfxObjects.RemoveAll();
}


void MainWindow::InitNewCampaignSession()
{ //48dd3a
    field_0x400 = 1;
    sessionMode = 2;

    if (FUN_0048ca7e(sessionMode) == 0)
    {
        m_GameSession.type = 0;
        if (strstr(afxCurrentWinApp->m_lpCmdLine, "-female") != nullptr)
            m_GameSession.type |= 0x80;
        if (strstr(afxCurrentWinApp->m_lpCmdLine, "-mage") != nullptr)
            m_GameSession.type |= 0x40;
        const char* nm = strstr(afxCurrentWinApp->m_lpCmdLine, "-name");

        if (!nm)
            strcpy(m_GameSession.character_name, "Unnamed");
        else
        {
            strncpy(m_GameSession.character_name, nm + 5, 32 - 1);
            m_GameSession.type |= 0x80;
            char* nm_sp = strstr(m_GameSession.character_name, " ");
            if (nm_sp)
                *nm_sp = 0;
        }
    }
    else
    {
        AfxMessageBox("Suxx");
        abort();
    }
}


void MainWindow::ShowStartGameSetupForNewSession()
{ //491b3e
    g_Cursors[CURSOR_WAIT]->Use();

    vis_right_panel->MsgProc(0x403, (uint32_t)vis_map_context, 0); //FIXME

    vis_root->AddChild(vis_startgame);

    if (sessionMode != 2)
        m_GameSession.character_name[0] = '\0';

    vis_startgame->SetCharName(m_GameSession.character_name);

    vis_startgame->SetSelectedDifficulty(m_FameHall.GetDifficulty());
    vis_startgame->SetSelectedPortraitPlayerType(m_GameSession.type >> 6);
    vis_startgame->VMethod28();
    vis_root->VMethod9();

    field_0x460 = 0;
    dialogsMask |= 0x800;

    g_Cursors[CURSOR_DEFAULT]->Use();

    if (g_SoundSettings.field_0x20)
    {
        CStringArray lst;
        lst.Add("music\\chrgen.wav");

        CStringArray& playing = music_player->GetPlaylist();
        if (music_player->GetPlaylistSize() == 0 || music_player->GetPlaylist()[0] != lst[0])
            music_player->SetPlayList(lst);

        music_player->Play();
    }
}


void MainWindow::ShowStartupLogoDialog()
{ // 48dba0
    vis_root->AddChild(vis_logownd);
    vis_logownd->VMethod28();

    field_0x460 = 0;
    dialogsMask |= 0x100;
}


void MainWindow::RemoteGameIdle()
{ //48acac
    if (g_NetStru1_local.IsActive() == 0)
    {
        vis_root->MsgProc(0x446, 0, 0);
        PostMessage(0x45c, 0, 0);

        if (g_EnableTrace != 0)
        {
            CStdioFile f;
            f.Open("error.log", CFile::modeWrite | CFile::modeNoTruncate | CFile::modeCreate);
            f.SeekToEnd();

            CTime tm = CTime::GetCurrentTime();
            f.WriteString(tm.Format("%d.%m.%y %H:%M:%S ") + "Connection lost\n");
            
            f.Close();

            vis_map_context->msglog.Add("Connection lost", clrsh_TechBlack, 30000);
        }
    }
    else
    {
        g_mousept.Update();

        if (g_NetStru1_local.GetClientsPktNum() == 0)
        {
            uint32_t tick = GetTickCount();
            if (std::abs((int)(tick - g_RemoteTimestamp)) > 1000)
            {
                g_NetStru1_local.SendPacket_64(1, 0);
                g_RemoteTimestamp = tick;
            }
        }
        else
        {
            while(true)
            {
                game_tic_counter++;
                game_tic_counter &= 0xf;

                if (game_tic_counter == 0)
                    g_NetStru1_local.sub_51EEB7();

                vis_map_context->ProcessPackets(100);
                vis_root->MsgProc(0x401, 0, 0);

                g_mousept.Update();

                if (g_NetStru1_local.GetClientsPktNum() == 0)
                    break;
            }
            g_RemoteTimestamp = GetTickCount();
        }

        if (g_mousept.GetSelectState() == 0 && dialogsMask == 1)
        {
            if (g_mousept.GetX() <= 0)
                vis_map_context->ScrollMapX(-1);

            if (g_mousept.GetY() <= 0)
                vis_map_context->ScrollMapY(-1);

            if (g_mousept.GetX() >= g_ScreenSize.right - 2)
                vis_map_context->ScrollMapX(1);

            if (g_mousept.GetY() >= g_ScreenSize.bottom - 2)
                vis_map_context->ScrollMapY(1);
        }

        if (field_0xbc != 0)
            vis_root->MsgProc(0x402, 0, 0);

        g_mousept.Update();
    }
}

void MainWindow::SingleGameTimedIdle()
{ //48a543
    g_mousept.Update();

    uint32_t ticks = timeGetTime();
    while (ticks >= last_tic_time + game_tic_time * (game_tic_counter + 1))
    {
        if (game_tic_counter == 0)
            last_tic_time = ticks;

        game_tic_counter++;
        game_tic_counter &= 0xf;

        g_Server->ServerTic();

        vis_map_context->ProcessPackets(0x64);
        vis_root->MsgProc(0x401, 0, 0);

        g_mousept.Update();

        ticks = timeGetTime();
    }


    if (g_mousept.GetSelectState() == 0 && dialogsMask == 1)
    {
        bool scroll = false;

        if (g_mousept.GetX() <= 0)
        {
            vis_map_context->ScrollMapX(-1);
            scroll = true;
        }


        if (g_mousept.GetY() <= 0)
        {
            vis_map_context->ScrollMapY(-1);
            scroll = true;
        }

        if (g_mousept.GetX() >= g_ScreenSize.right - 2)
        {
            vis_map_context->ScrollMapX(1);
            scroll = true;
        }

        if (g_mousept.GetY() >= g_ScreenSize.bottom - 2)
        {
            vis_map_context->ScrollMapY(1);
            scroll = true;
        }

        if (scroll)
            vis_minimap->MsgProc(0x408, 0, 0);
    }

    if (field_0xbc != 0)
        vis_root->MsgProc(0x402, 0, 0);

    g_mousept.Update();
}

void MainWindow::SingleGameIdle()
{ //48a3a2
    g_mousept.Update();

    g_Server->ServerTic();

    vis_map_context->ProcessPackets(0x64);

    vis_root->MsgProc(0x401, 0, 0);

    game_tic_counter++;
    g_mousept.Update();


    if (g_mousept.GetSelectState() == 0 && dialogsMask == 1)
    {
        bool scroll = false;

        if (g_mousept.GetX() <= 0)
        {
            vis_map_context->ScrollMapX(-1);
            scroll = true;
        }
            

        if (g_mousept.GetY() <= 0)
        {
            vis_map_context->ScrollMapY(-1);
            scroll = true;
        }

        if (g_mousept.GetX() >= g_ScreenSize.right - 2)
        {
            vis_map_context->ScrollMapX(1);
            scroll = true;
        }

        if (g_mousept.GetY() >= g_ScreenSize.bottom - 2)
        {
            vis_map_context->ScrollMapY(1);
            scroll = true;
        }

        if (scroll)
            vis_minimap->MsgProc(0x408, 0, 0);
    }

    if (field_0xbc != 0)
    {
        vis_root->MsgProc(0x402, 0, 0);
        SaveScreenshot();
    }

    g_mousept.Update();
}



CGameSession::CGameSession()
{ //49248a
    //fields init in defination
    strcpy(character_name, "Self");
}

CGameSession::~CGameSession()
{ //4960f0
}

int CGameSession::SubmitCharacterSetupAndWaitForSelectedUnit()
{
    //493ffe
    INT_00660f8c = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if ((flags & 1) == 0 && wnd->sessionMode != 2)
        wnd->vis_map_context->FUN_0041cc78(characterRosterFilePaths[selectedCharacterRosterFileIndex]);
    else
        wnd->vis_map_context->FUN_0041cbb8();

    if (wnd->sessionMode == 1 || wnd->sessionMode == 2)
        g_Server->FUN_0050907e();

    uint32_t stime = timeGetTime();
    while (wnd->vis_map_context->field_0x3f6c == nullptr)
    {
        while (g_NetStru1_local.GetClientsPktNum() == 0)
        {
            MSG msg;

            if (PeekMessageA(&msg, NULL, 0, 0, 1) != 0)
            {
                if (msg.message == WM_QUIT) {
                    return 0;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }

            if (g_CmdTimeout < timeGetTime() - stime)
            {
                INT_00660f8c = 0x1008;
                return 0;
            }

            g_mousept.Update();
            g_NetStru1_local.ProcessConnections();
        }

        if (wnd->vis_map_context->ProcessPackets(0x64) == 0)
            return 0;

    }

    return 1;
}


void CGameSession::FUN_00493d8d()
{ //493d8d
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if (wnd->sessionMode == 2)
        return;

    CUnit* cu = wnd->vis_map_context->GetUnit_3f6c();
    money = 1000;
    monster_killed = 0;
    player_killed = 0;
    death_count = 0;
    fragCount = 0;

    FileSectionBasicInfo local_40;
    local_40.id1 = sessionKeyPart1;
    local_40.id2 = sessionKeyPart2;
    local_40.hat_id = field_0x64;

    strcpy(local_40.nick, character_name);

    local_40.character_class = type;

    face = cu->face;

    local_40.picture = face;
    local_40.main_sphere = main_sphere;
    local_40.flags = flags;
    local_40.color = color;

    FileSectionStats local_74;
    local_74.body = cu->field_0x180[0];
    local_74.reaction = cu->field_0x180[1];
    local_74.mind = cu->field_0x180[2];
    local_74.spirit = cu->field_0x180[3];
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

    for (int i = 0; i < 9; i++)
        shortcuts[i].ToBuffer(&bufpos);
    
    WritePlayerFile_4F53EA(characterRosterFilePaths[selectedCharacterRosterFileIndex], &local_40, &local_74, wnd->vis_map_context->kill_stats.data(), &PacketUnitStateVec::Inst, nullptr, buffer, bufpos - buffer);
}


void CGameSession::FUN_00494687()
{
    body = 0;
    main_sphere = 0;
    RecreateCUnit();
}


void CGameSession::RecreateCUnit()
{   //4941c0
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* cu = mwnd->vis_map_context->GetUnit_3f6c();
    if (cu)
    {
        Human* hm = nullptr;
        switch (type & 0xc0)
        {
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

        if (body != 0)
        {
            hm->body = body;
            hm->reaction = reaction;
            hm->mind = mind;
            hm->spirit = spirit;
            hm->VMethod18();
            hm->face = (byte)face;
        }

        if (main_sphere == 0)
            main_sphere = hm->main_sphere;

        hm->sub_533345(main_sphere, 0x14);
        cu->CopyFromUnit(*hm);

        body = cu->field_0x180[0] = cu->body;
        reaction = cu->field_0x180[1] = cu->reaction;
        mind = cu->field_0x180[2] = cu->mind;
        spirit = cu->field_0x180[3] = cu->spirit;

        face = cu->face;

        delete hm;
    }
}


void CGameSession::LoadCharacterRosterEntry(int32_t idx)
{ //492c66
    selectedCharacterRosterFileIndex = idx;

    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* unit = mwnd->vis_map_context->GetUnit_3f6c();

    if (unit == nullptr)
    {
        unit = new CUnit();
        unit->VMethod17(1, 1, 0, 0, 0, mwnd->vis_map_context->field_0x9b8[0], 0, 0, 0, 1);
        mwnd->vis_map_context->field_0x9d0[1] = unit;
        mwnd->vis_map_context->field_0x3f6c = unit;
        mwnd->vis_map_context->MsgProc(0x405, 0, 0);
    }

    if (idx >= 0 && idx < characterRosterFilePaths.GetSize())
    {
        CFile f(characterRosterFilePaths[selectedCharacterRosterFileIndex], CFile::modeRead);

        FileSectionStats* stats = nullptr;
        FileSectionBasicInfo* basic = nullptr;
        uint8_t* kills = nullptr;
        PacketUnitStateVec* equip = nullptr;
        PacketUnitStateVec* inv = nullptr;
        uint8_t* sec40a = nullptr;
        uint32_t size_40a = 0;
        if (ParsePlayerFile_4F62E6(&f, &basic, &stats, &kills, &equip, &inv, &sec40a, &size_40a) != 0 && basic != nullptr && stats != nullptr)
        {
            strcpy(character_name, basic->nick);

            memset(unit->str1, 0, 12);

            char* delim = strchr(character_name, '|');
            if (delim == NULL)
            {
                memcpy(unit->str1, character_name, 11);
                unit->str2[0] = 0;
            }
            else {
                memcpy(unit->str1, character_name, delim - character_name);
                memcpy(unit->str2, delim + 1, 12);
            }

            type = basic->character_class;
            main_sphere = basic->main_sphere;
            flags = basic->flags;
            color = basic->color;
            field_0x64 = basic->hat_id;

            sessionKeyPart1 = basic->id1;
            sessionKeyPart2 = basic->id2;

            money = stats->money;
            monster_killed = stats->monster_kills;
            player_killed = stats->player_kills;
            death_count = stats->deaths;
            fragCount = stats->frags;

            Human* hum;
            switch (type & 0xc0)
            {
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

            body = stats->body;
            hum->body = body;

            reaction = stats->reaction;
            hum->reaction = reaction;

            mind = stats->mind;
            hum->mind = mind;

            spirit = stats->spirit;
            hum->spirit = spirit;

            face = basic->picture;
            hum->face = face;

            hum->experience_per_sphere[0] = stats->experience[0];
            hum->experience_per_sphere[1] = stats->experience[1];
            hum->experience_per_sphere[2] = stats->experience[2];
            hum->experience_per_sphere[3] = stats->experience[3];
            hum->experience_per_sphere[4] = stats->experience[4];

            int32_t maxlvl = 0;

            for (int i = 1; i < 6; i++)
            {
                hum->hit_values.skill_levels[i] = ExperienceTable::GetLevel(hum->experience_per_sphere[i - 1]);

                if (hum->hit_values.skill_levels[i] > maxlvl)
                    maxlvl = hum->hit_values.skill_levels[i];
            }

            for (int i = 1; i < 6; i++)
                hum->hit_values2.skill_levels[i] = hum->hit_values.skill_levels[i];

            if (maxlvl >= 96)
                field_0x114 = 4;
            else if (maxlvl >= 76)
                field_0x114 = 3;
            else if (maxlvl >= 51)
                field_0x114 = 2;
            else
                field_0x114 = 1;

            if (maxlvl >= 90)
                field_0x118 = 4;
            else if (maxlvl >= 51)
                field_0x118 = 3;
            else if (maxlvl >= 26)
                field_0x118 = 2;
            else
                field_0x118 = 1;

            hum->experience = 0;

            for (int i = 1; i < 6; i++)
                hum->experience += hum->experience_per_sphere[i - 1];

            hum->VMethod18();

            if (equip != nullptr)
            {
                uint8_t* eq_data = equip->data;

                hum->VMethod15();

                if (hum->weapon)
                    hum->inventory->PutItemIntoBagAtDefault(hum->Unequip(hum->weapon));

                if (hum->shield)
                    hum->inventory->PutItemIntoBagAtDefault(hum->Unequip(hum->shield));

                if (hum->inventory)
                    delete hum->inventory;

                hum->inventory = new Inventory();
                for (int i = 1; i < 13; i++)
                {
                    Item* itm = sub_4F499B(&eq_data);

                    if (itm->item_id != 0)
                        hum->VMethod13(itm);
                }
            }

            unit->CopyFromUnit(*hum);

            unit->field_0x180[0] = stats->body;
            unit->field_0x180[1] = stats->reaction;
            unit->field_0x180[2] = stats->mind;
            unit->field_0x180[3] = stats->spirit;

            unit->map_player = mwnd->vis_map_context->field_0x9b8[0];

            if (sec40a != nullptr)
            {
                for (int i = 0; i < 9; i++)
                    shortcuts[i].LoadFromBuffer(&sec40a);
            }
        }
    }
}

void CGameSession::InitializeNewCharacterSession(int tp, const char* name)
{ //493ab6
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();

    field_0x64 = mwnd->field_0x3e0.field_0c;

    if (name)
    {
        strcpy(character_name, name);
        type = tp;
    }

    LARGE_INTEGER k;
    if (QueryPerformanceCounter(&k) == 0)
    {
        sessionKeyPart2 = k.LowPart;
        sessionKeyPart1 = timeGetTime();
    }

    characterRosterNames.InsertAt(characterRosterNames.GetSize() - 1, character_name);

    CString fname;
    fname.Format("%u%u.a2c", sessionKeyPart1, sessionKeyPart2);
    characterRosterFilePaths.Add(fname);

    selectedCharacterRosterFileIndex = characterRosterFilePaths.GetSize() - 1;

    flags = 1;

    mwnd->vis_map_context->kill_stats.fill(0);

    if (mwnd->vis_map_context->GetUnit_3f6c() == nullptr)
    {
        CUnit* cunit = new CUnit();
        cunit->VMethod17(1, 1, 0, 0, 0, mwnd->vis_map_context->field_0x9b8[0], 0, 0, 0, 1);

        mwnd->vis_map_context->field_0x9d0[1] = cunit;
        mwnd->vis_map_context->field_0x3f6c = cunit;
    }

    field_0x118 = 1;
    field_0x114 = 1;
    main_sphere = 0;

    RecreateCUnit();
}

UserShortcut::UserShortcut()
{ //4971a0
    SetNull();
}

UserShortcut::~UserShortcut()
{ //41e493
    if (mods_size)
    {
        if (mods)
            free(mods);

        mods_size = 0;
        mods = nullptr;
    }
}

void UserShortcut::SetNull()
{ //4971c0
    mods_size = 0;
    item_id = 0;
    kind = 0;
    mods = nullptr;
}

int UserShortcut::ToBuffer(uint8_t** buf)
{ //41e5ed
    uint8_t* data = *buf;
    *(uint16_t*)data = kind;
    data += 2;
    *(uint16_t*)data = item_id;
    data += 2;
    *(uint32_t*)data = mods_size;
    data += 4;
    if (mods_size)
    {
        memcpy(data, mods, mods_size);
        data += mods_size;
    }
    *buf = data;
    return 8 + mods_size;
}


void UserShortcut::WriteToFile(CFile* f)
{ //41e4d3
    f->Write(&kind, 2);
    f->Write(&item_id, 2);
    f->Write(&mods_size, 4);
    if (mods_size != 0)
        f->Write(mods, mods_size);
}

void UserShortcut::ReadFromFile(CFile* f)
{ //41e53b
    if (mods_size != 0)
    {
        if (mods) 
            free(mods);

        mods = nullptr;
        mods_size = 0;
    }

    f->Read(&kind, 2);
    f->Read(&item_id, 2);
    f->Read(&mods_size, 4);

    if (mods_size != 0)
    {
        mods = (uint8_t *)malloc(mods_size);
        f->Read(mods, mods_size);
    }
}


void UserShortcut::LoadFromBuffer(uint8_t** buf)
{ //41e689
    uint8_t* data = *buf;

    if (mods_size != 0)
    {
        if (mods != nullptr)
            free(mods);

        mods = nullptr;
        mods_size = 0;
    }

    kind = *(uint16_t*)data;
    data += 2;
    item_id = *(uint16_t*)data;
    data += 2;
    mods_size = *(uint32_t*)data;
    data += 4;

    if (mods_size != 0)
    {
        mods = (uint8_t*)malloc(mods_size);
        memcpy(mods, data, mods_size);
        data += mods_size;
    }

    *buf = data;
}
