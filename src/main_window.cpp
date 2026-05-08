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
#include "unit.h"
#include "resource.h"
#include "map_stuff.h"
#include "gfx.h"


int32_t DAT_00660f88 = 0;
CDWordArray DAT_006658d8;

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








MWin_5e8::MWin_5e8()
{
    //4ac2e3
    field_x4 = 0;
    field_x8 = 0;
    field_xc = 10;
}

MWin_5e8::~MWin_5e8()
{
    //4ac352
}

void MWin_5e8::FUN_004ac3af()
{
    //4ac3af
    field_x4 = 0;
    field_x8 = 0;
}

void MWin_5e8::FUN_00497270(uint32_t arg)
{
    //497270
    field_x0 = arg;
}

uint32_t MWin_5e8::FUN_004ac566(int32_t arg)
{
    //4ac566
    for (uint32_t i = 0; i < fame2_arr.GetSize(); i++)
    {
        Fame2& fame = fame2_arr[i];
        if (fame.FUN_004ad880() == arg && fame.FUN_004ad890() != 0)
            return 0;
    }

    Fame2 f;
    f.FUN_004abf0f(arg, 1);
    fame2_arr.Add(f);

    return 1;
}


void MWin_5e8::FUN_004acafa()
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



void MWin_5e8::FUN_004ac945(CFile* file)
{
    //4ac945
    uint32_t num;
    file->Read(&num, 4);
    
    if (num == 0)
        fame1_arr.RemoveAll();
    else
    {
        fame1_arr.SetSize(num);
        for (uint32_t i = 0; i < num; i++)
        {
            uint32_t t;
            file->Read(&t, 4);

            char buf[1024];
            file->Read(buf, t);

            Fame1& fame = fame1_arr[i];

            fame.str = buf;

            file->Read(&fame.field_x4, 4);
            file->Read(&fame.field_x8, 4);
            file->Read(&fame.field_xc, 4);
        }
    }
}

void MWin_5e8::FUN_004ac706(CFile* file)
{ //  4ac706
    int32_t num = fame2_arr.GetSize();
    file->Write(&num, 4);

    for (uint32_t i = 0; i < num; i++)
    {
        Fame2& fm = fame2_arr[i];
        
        //inline 4ac04b
        file->Write(&fm.field_x0, 4);
        file->Write(&fm.field_x4, 4);
    }
    file->Write(&field_x4, 4);
    file->Write(&field_x8, 4);
}


void MWin_5e8::FUN_004ac3ce(const Fame1& fame)
{
    //4ac3ce
    if (fame1_arr.GetSize() == 0)
        fame1_arr.Add(fame);
    else
    {
        int32_t i = 0;
        for (i = 0; i < fame1_arr.GetSize(); i++)
        {
            Fame1& f = fame1_arr[i];
            if (f.field_x4 <= fame.field_x4)
            {
                fame1_arr.InsertAt(i, fame);
                break;
            }
        }
        if (i == fame1_arr.GetSize())
            fame1_arr.Add(fame);
    }

    if (fame1_arr.GetSize() > field_xc)
        fame1_arr.SetSize(field_xc);
}

void MWin_5e8::FUN_00420050()
{
    //420050
    field_x8++;
}







extern "C"
{
    void __fastcall sub_49585B(CWnd* obj);
    void __fastcall sub_48A747();
};

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
                    sub_49585B(this->field_0x7fc);
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

    field_0x5e8.FUN_004ac3af();

    CVisualObject* elm = vis_root->FindChild(0x3fc);
    if (elm)
        elm->MsgProc(0x445, 0, 0);

    FUN_00485a41();

    vis_root->AddChild(field_0xf4);
    field_0xf4->VMethod28();
    vis_root->VMethod9();
    field_0x460 = 0;
    field_0x418 |= 0x80;
    FUN_0048cb3c();

    elm = field_0xd0->FindChild(0x13);
    if (elm)
    {
        field_0xd0->RemoveChild(elm);
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
    field_0xd4->RemoveAllChilds();
    field_0xd4->AddChild(field_0xe0);

    vis_root->RemoveAllChilds();
    vis_root->AddChild(field_0xd0);
    vis_root->AddChild(field_0xd4);

    field_0x418 &= ~1;

    field_0xd4->MsgProc(0x403, (uint32_t)field_0xd0, 0);
}


int32_t SomeMainStructure::GetStringArray1Size()
{
    //438d10
    return string_array1.GetSize();
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

    field_0xd0->field_0xe0 = 1;

    if (obj == field_0xf8 || obj == field_0xfc || obj == field_0x100)
    {
        field_0xd0->FUN_0041a8cc();
        field_0xd0->FUN_00416cf7();
        if (field_0x640 != 2)
            FUN_00494c91();
    }
    else if (obj == field_0x138)
    {
        field_0xd0->RemoveChild(field_0x138);

        if (field_0x138->GetCloseCode() == 0x445)
        {

            CString str;
            field_0x138->WriteData(&str);
            if (str[0] == '=')
                field_0xd0->FUN_0041b2a4(str.Mid(1), 3, 0);
            else if (str[0] == '-')
            {
                int val = field_0x138->FUN_004972d0();
                if (val == -1)
                    field_0xd0->FUN_0041b2a4(str.Mid(1), 1, 0);
                else
                    field_0xd0->FUN_0041b2a4(str.Mid(field_0x138->FUN_00497310()), 2, val);
            }
            else
                field_0xd0->FUN_0041b2a4(str, 0, 0);
        }
    }
    else if (obj == field_0x134)
        field_0x418 &= ~0x100;
    else if (obj == field_0x3d8)
        field_0x418 &= ~0x4000;
    else if (obj == field_0x11c)
    {
        field_0x418 &= ~0x1000;
        PostMessage(0x421, 0, 0);
    }
    else if (obj == field_0xf4)
        field_0x418 &= ~0x80;
    else if (obj == vis_town || obj == field_0x114 || obj == field_0x118)
    {}
    else if (obj == vis_tav || obj == field_0x108 || obj == field_0x10c)
    {
        if (field_0x640 != 2)
            FUN_00494c91();

        field_0x418 &= ~4;
        field_0xd0->FUN_00416cf7();
    }
    else if (obj == field_0x36c)
    { //charget 2step
        field_0x418 &= ~0x200;

        if (field_0x36c->GetCloseCode() == 0x446)
            FUN_00491b3e();
        else
        {
            some_struc.flags |= 4;

            if (field_0x640 == 2)
            {
                field_0xd0->FUN_0040d4e2();
                some_struc.FUN_00493ffe();

                ScenarioSetVar(0x308, (some_struc.field_0x34 & 0x40) != 0);
                ScenarioSetVar(0x30d, (some_struc.field_0x34 & 0x80) != 0);

                if (g_Server)
                    g_Server->FUN_00497470(field_0x5e8.FUN_00497290() + 1);

                PostMessage(0x42e, 0, 0);
            }
            else
            {
                some_struc.FUN_00493d8d();

                if (field_0x3e0.field_10 == 0)
                    PostMessage(0x451, 0, 0);
                else
                    PostMessage(0x489, 0, 0);
            }
        }
    }
    else if (obj == field_0x374)
    { // chargen 1step
        field_0x418 &= ~0x800;

        if (field_0x374->GetCloseCode() == 0x446)
        {
            if (!field_0xd0->field_0x9d0.IsEmpty())
            {
                field_0xd0->field_0x9d0.RemoveAll();

                CUnit *uni = field_0xd0->GetUnit_3f6c();
                if (uni)
                    delete uni;

                field_0xd0->SetUnit_3f6c(nullptr);
            }

            if (field_0x640 == 2)
                PostMessage(0x421, 0, 0);
            else
                FUN_0048cff7();
        }
        else
        {
            CString name = field_0x374->GetName();
            strcpy(some_struc.character_name, name);

            if (!name.IsEmpty())
            {
                int pos = name.Find(':');
                if (pos != -1)
                {
                    name = name.Mid(pos + 1);

                    some_struc.color = atoi(name);

                    if (some_struc.color == 0 || some_struc.color > 16)
                        some_struc.color = 0;

                    name = field_0x374->GetClan().Left(pos);
                }
            }

            if (!name.IsEmpty())
            {
                strcat(some_struc.character_name, "|");
                strcat(some_struc.character_name, name);
            }

            field_0x5e8.FUN_00497270(field_0x374->FUN_004973d0());

            some_struc.field_0x34 = field_0x374->FUN_004973b0() << 6; // 0x40 and 0x80 bits

            name = field_0x374->GetName();
            strncpy(field_0xd0->GetUnit_3f6c()->field_0xec, name, 12);

            name = field_0x374->GetClan();
            strncpy(field_0xd0->GetUnit_3f6c()->field_0xf8, name, 12);

            some_struc.FUN_00494687();
            FUN_00491a49();
        }
    }
    else if (obj == vis_charsel)
    {
        field_0x418 &= ~0x400;

        if (vis_charsel->GetCloseCode() == 0x446)
        {
            if (!field_0x3e0.field_08.IsEmpty())
                PostMessage(WM_CLOSE, 0, 0);
            else
                PostMessage(0x421, 0, 0);
        }
        else if ((some_struc.flags & 4) == 0)
            FUN_00491b3e();
        else if (field_0x3e0.field_10 == 0)
            PostMessage(0x451, 0, 0);
        else
            PostMessage(0x489, 0, 0);
    }
    else if (obj == field_0xf0)
        field_0x418 &= ~0x20;
    else if (obj == field_0x130)
    {
        field_0x418 &= ~0x40;

        if (FUN_00497490() == 0)
            CWnd::PostMessage(0x421, 0, 0);
        else
            CWnd::PostMessage(0x429, 0, 0);
    }
    else if (obj == field_0x120)
    {
        field_0x418 &= ~0x8;

        field_0x438 = timeGetTime();
        game_tic_counter = 0;
        field_0x43c = 0;
    }
    else if (obj == field_0x12c)
    {
        field_0x418 &= ~0x8;

        if (field_0x12c->GetCloseCode() == 0x447)
        {
            field_0xd0->FUN_0041d2da(1);
            PostMessage(0x421, 0, 0);
        }
        else
            PostMessage(0x42e, 0, 0);

        delete field_0x12c;
        field_0x12c = nullptr;
    }
    else if (obj == field_0x140)
    { //save game
        field_0x418 &= ~0x8;

        if (field_0x140->GetCloseCode() == 0x445)
            FUN_0048f905();

        if (field_0x418 == 1 && field_0x63c != 0)
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
        field_0x418 &= ~8;

        int32_t code = field_0x13c->GetCloseCode();
        if (code == 0x445)
            PostMessage(0x419, 0, 0);
        else if (g_Server == nullptr)
            PostMessage(0x421, 0, 0);
        else if (field_0x450 == 0xff)
            PostMessage(0x41e, 0, 0);

        if (field_0x418 == 1 && field_0x63c != 0)
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
        field_0x418 &= ~8;

        int32_t code = field_0x34c->GetCloseCode();

        if (code == 0x44f)
        {
            field_0x63c = 0;
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
        field_0x418 &= ~8;

        if (field_0x350->GetCloseCode() == 0x446)
        {
            if (field_0x640 == 3)
                PostMessage(0x421, 0, 0);
            else
                FUN_0048cff7();
        }

        delete field_0x350;
        field_0x350 = nullptr;
        return;
    }
    else if (obj == field_0x378)
    {
        field_0x418 &= ~8;

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
        field_0x418 &= ~8;

        if (field_0x37c->GetCloseCode() == 0x446)
        {
            if (hat_settings.ishat == 0)
            {
                vis_root->MsgProc(0x446, 0, 0);
                FUN_0048cff7();
            }
            else if (g_CLlDriver.IsListen())
                FUN_0048cff7();
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
        field_0x418 &= ~8;
        field_0x418 = 0;

        if (field_0x3d0->GetCloseCode() == 0x445)
            PostMessage(0x421, 0, 0);

        delete field_0x3d0;
        field_0x3d0 = nullptr;
    }
    else if (obj == field_0x144)
    {
        field_0x418 &= ~8;

        int32_t code = field_0x144->GetCloseCode();

        field_0x144 = nullptr;

        if (code == 0x445)
        {
            int j = 0;
            for (int i = 0; i < field_0xd0->field_0x9b8.GetSize(); i++)
            {
                MapPlayerData* mp = field_0xd0->field_0x9b8[i];
                if (mp != nullptr && (mp->flags & 1) == 0)
                {
                    DiplomacyEntry* visdp = (*field_0x348)[j];

                    uint16_t dpl = field_0xd0->my_main_unit->diplomacy[mp->index] & 0x68;

                    if (visdp->enemy != 0)
                        dpl |= 1;

                    if (visdp->ally != 0)
                        dpl |= 2;

                    if (visdp->see != 0)
                        dpl |= 0x10;

                    if (visdp->mute != 0)
                        dpl |= 4;

                    field_0xd0->my_main_unit->diplomacy[mp->index] = dpl;

                    field_0xd0->my_main_unit->diplomacy[field_0xd0->my_main_unit->index] = 0x3a;

                    j++;
                }
            }

            field_0xd0->FUN_0041a735();
        }

        delete obj;
    }
    else if (obj == field_0x3d4)
    {
        field_0x418 &= ~8;

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
        field_0x418 &= ~8;

        if (field_0x124->GetCloseCode() == 0x445)
            PostMessage(0x41e, 0, 0);
        else
            PostMessage(0x418, 0, 0);

        delete field_0x124;
        field_0x124 = nullptr; // MUST BE DONE? WAT?
    }
    else if (obj == field_0x3dc)
    {
        field_0x418 &= ~0x8000;

        delete field_0x3dc;
        field_0x3dc = nullptr; // MUST BE DONE? WAT?

        if ((field_0x418 & 1) == 0)
            vis_root->cursor_over_obj_last = vis_root->FindChild(1020);
    }
    else if ((field_0x418 & 8) != 0)
    {
        field_0x418 &= ~8;

        if (field_0x418 == 1 && field_0x640 == 2)
        {
            field_0x438 = timeGetTime();
            game_tic_counter = 0;
            field_0x43c = 0;
        }

        delete obj;
    }

    if (field_0x418 == 0x80)
        g_Cursors[CURSOR_SELECT]->Use();

    field_0xd4->MsgProc(0x408, 0, 0);

    if (field_0x418 != 1 || field_0xd0->field_0x80 != nullptr)
        vis_root->VMethod9();

    if (field_0x418 == 1 && DAT_006658d8.GetSize() != 0)
    {
        PostMessage(0x433, DAT_006658d8[0], 0);
        DAT_006658d8.RemoveAt(0);
    }
}


void MainWindow::FUN_0048cff7()
{ //48cff7
    g_Cursors[CURSOR_WAIT]->Use();

    field_0xe0->field_70 = 1;
    vis_root->AddChild(vis_charsel);

    vis_charsel->VMethod28();

    field_0x418 |= 0x400;
    
    vis_root->VMethod9();

    field_0x460 = 0;

    g_Cursors[CURSOR_DEFAULT]->Use();
    return;
}


void MainWindow::FUN_00491a49()
{   //491a49
    g_Cursors[CURSOR_WAIT]->Use();

    field_0xe0->field_70 = 1;

    field_0xd4->MsgProc(0x403, (uint32_t)field_0xd0, 0);
    
    field_0xd0->GetUnit_3f6c()->VMethod1(1); // select

    field_0xd0->FUN_00416cf7();

    vis_root->AddChild(field_0x36c);
    field_0x36c->VMethod28();

    vis_root->VMethod9();

    field_0x460 = 0;
    field_0x418 |= 0x200;

    g_Cursors[CURSOR_DEFAULT]->Use();
}




void MainWindow::FUN_0048f905()
{  // 48f905
    if (field_0x640 == 2)
    {
        CString local_2a4 = field_0x148.buf2;
        g_Server->sub_4ED2DC(&local_2a4);

        field_0x438 = timeGetTime();
        game_tic_counter = 0;
        field_0x43c = 0;

        CFile local_13c;
        local_13c.Open(field_0x148.buf2, CFile::modeReadWrite);
        local_13c.Seek(0, CFile::end);
        local_13c.Write(field_0x148.buf1, 0x100);
        
        int inbattle = field_0x418 & 1;

        RegFile local_180;
        local_180.SetInt("CurrentState", "InBattle", inbattle);
        local_180.SetString("Character", "Name", some_struc.character_name);
        local_180.SetInt("GameOptions", "Wimpy", field_0xd0->wimpy);
        local_180.SetInt("GameOptions", "ShowHP", field_0xd0->show_hp);
        local_180.SetInt("GameOptions", "FlyingHP", field_0xd0->flying_hp);
        local_180.SetInt("GameOptions", "Formation", field_0xd0->formation);
        local_180.SetInt("GameOptions", "Speed", game_speed);
        local_180.SetInt("GameOptions", "ShowTimeFlow", g_settings.ShowTimeFlow);
        local_180.SetInt("View", "X", field_0xd0->view_x);
        local_180.SetInt("View", "Y", field_0xd0->view_y);

        local_180.SetInt("SpellBook", "IsOpen", field_0xd0->IsBookOpen());
        local_180.SetInt("SpellBook", "Pressed", field_0xec->pressed);

        CArray<uint32_t> local_25c;
        local_180.SetInt32Array("SpellBook", "Shortcuts", local_25c);

        CArray<uint16_t> local_12c;
        CArray<uint16_t> local_248[10];

        if (!field_0xd0->field_0x9d0.IsEmpty())
        {
            for (POSITION pos = field_0xd0->field_0x9d0.GetStartPosition(); pos != nullptr;)
            {
                uint16_t key;
                CGameObject* obj;

                field_0xd0->field_0x9d0.GetNextAssoc(pos, key, obj);

                if (obj->FUN_0041f110() != 0)
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

        local_180.SetInt("Inventory", "IsOpen", field_0xd0->IsBagOpen());

        if (inbattle)
        {
            CArray<uint16_t> local_27c;

            local_180.SetInt("Projectiles", "FreeIndex", field_0xd0->field_0xa24);

            if (!field_0xd0->field_0x9ec.IsEmpty())
            {
                for (POSITION pos = field_0xd0->field_0x9ec.GetStartPosition(); pos != nullptr;)
                {
                    uint16_t key;
                    CGameObject* obj;
                    field_0xd0->field_0x9ec.GetNextAssoc(pos, key, obj);

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

            uint16_t* land = field_0xd0->field_0x80->GetLandscape();
            int numcells = field_0xd0->field_0x80->GetWidth() * field_0xd0->field_0x80->GetHeight();

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

        field_0x5e8.FUN_004ac706(&local_13c);

        ScenarioSave(&local_13c);

        for (int i = 0; i < 9; i++)
            some_struc.field_0x8c[i].WriteToFile(&local_13c);

        local_13c.Close();
    }
    else
    {
        field_0xd0->FUN_0041afcf(field_0x148.buf2);
    }
}


void MainWindow::FUN_004903d0()
{ // 4903d0

    field_0x640 = 0;

    g_NetStru1_local.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_local);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-waitforever"))
        g_CLlDriver.SetTimeout(-1);

    if (strstr(cmdline, "-timeout"))
        g_CLlDriver.SetTimeout(g_CmdTimeout);

    if (field_0xd0->FUN_0040d4e2() == 0)
    {
        g_CLlDriver.Close();
        PostMessage(0x41d, 0, 0);
    }
    else
    {
        if (some_struc.field_0x10c == 0)
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
    field_0x640 = 3;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2)
        g_CLlDriver.StartServer(g_ServerConfig.max_players, some_struc.character_name, nullptr);

    FUN_0048ca7e(0);

    PostMessage(0x459, 0, 0);
}

int MainWindow::FUN_0048ca7e(int mode)
{ //48ca7e
    DAT_006658d8.RemoveAll();

    g_Server = new Server();
    if (g_Server->Start(mode) == 0)
    {
        field_0x63c = 1;
        return 0;
    }
    else
    {
        field_0x63c = 0;
        return 1;
    }
}


void MainWindow::FUN_004918ae()
{ // 4918ae

    field_0x640 = 1;

    g_NetStru1_main.SetLLDriver(&g_CLlDriver);
    g_CLlDriver.SetHlDriver(&g_NetStru1_main);

    LPTSTR cmdline = afxCurrentWinApp->m_lpCmdLine;

    if (strstr(cmdline, "-timeout"))
        g_CLlDriver.SetTimeout(g_CmdTimeout);

    if (g_CLlDriver.GetProvider() == 3 || g_CLlDriver.GetProvider() == 2)
        g_CLlDriver.StartServer(g_ServerConfig.max_players, some_struc.character_name, nullptr);

    FUN_0048ca7e(field_0x640);

    if (field_0xd0->FUN_0040d4e2() == 0)
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


LRESULT MainWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
        if (field_0x418 == 1)
            PopUpScreen( new IngameMenu(1, 100, 60, 440, 400, nullptr, 0, CRect(0, 0, 240, 40) ) );
        break;

    }

    return CWnd::WindowProc(message, wParam, lParam);
}


void MainWindow::PopUpScreen(VisScreen* screen)
{ //48d26a
    if (screen == field_0x3dc)
        field_0x418 |= 0x8000;
    else
        field_0x418 |= 8;

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



int SomeMainStructure::FUN_00493ffe()
{
    //493ffe
    INT_00660f8c = 0;

    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if ((flags & 1) == 0 && wnd->field_0x640 != 2)
        wnd->field_0xd0->FUN_0041cc78(string_array2[field_0x164]);
    else
        wnd->field_0xd0->FUN_0041cbb8();

    if (wnd->field_0x640 == 1 || wnd->field_0x640 == 2)
        g_Server->FUN_0050907e();

    uint32_t stime = timeGetTime();
    while (wnd->field_0xd0->field_0x3f6c == nullptr)
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

        if (wnd->field_0xd0->ProcessPackets(0x64) == 0)
            return 0;

    }

    return 1;
}


void SomeMainStructure::FUN_00493d8d()
{ //493d8d
    MainWindow* wnd = (MainWindow*)AfxGetMainWnd();

    if (wnd->field_0x640 == 2)
        return;

    CUnit* cu = wnd->field_0xd0->GetUnit_3f6c();
    money = 1000;
    monster_killed = 0;
    player_killed = 0;
    death_count = 0;
    field_0x44 = 0;

    FileSectionBasicInfo local_40;
    local_40.id1 = field_0x8;
    local_40.id2 = field_0xc;
    local_40.hat_id = field_0x64;

    strcpy(local_40.nick, character_name);

    local_40.character_class = field_0x34;

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
        field_0x8c[i].ToBuffer(&bufpos);
    
    WritePlayerFile_4F53EA(string_array2[field_0x164], &local_40, &local_74, wnd->field_0xd0->kill_stats.data(), &PacketUnitStateVec::Inst, nullptr, buffer, bufpos - buffer);
}


void SomeMainStructure::FUN_00494687()
{
    field_0x4c = 0;
    main_sphere = 0;
    FUN_004941c0();
}


void SomeMainStructure::FUN_004941c0()
{   //4941c0
    MainWindow* mwnd = (MainWindow*)AfxGetMainWnd();
    CUnit* cu = mwnd->field_0xd0->GetUnit_3f6c();
    if (cu)
    {
        Human* hm = nullptr;
        switch (field_0x34 & 0xc0)
        {
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

        if (field_0x4c != 0)
        {
            hm->body = field_0x4c;
            hm->reaction = field_0x50;
            hm->mind = field_0x54;
            hm->spirit = field_0x58;
            hm->VMethod18();
            hm->face = (byte)face;
        }

        if (main_sphere == 0)
            main_sphere = hm->main_sphere;

        hm->sub_533345(main_sphere, 0x14);
        cu->FUN_0046b0d7(*hm);

        field_0x4c = cu->field_0x180[0] = cu->body;
        field_0x50 = cu->field_0x180[1] = cu->reaction;
        field_0x54 = cu->field_0x180[2] = cu->mind;
        field_0x58 = cu->field_0x180[3] = cu->spirit;

        face = cu->face;

        delete hm;
    }
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


