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
    field_0x3f0 = 0;
    field_0x3f4.Empty();
    field_0x3ec = 0;

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
    field_0x3f0 = 0;
    field_0x3f4.Empty();
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
