#include "main_window.h"
#include <cstdio>
#include "asm_mfc.h"
#include "game_app.h"
#include "net.h"
#include "players_list.h"
#include "server.h"
#include <stdio.h>


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







extern "C"
{
    void __fastcall sub_4271E6(void* obj);
    void __fastcall sub_49585B(CWnd* obj);
    void __fastcall sub_48A747();

    // Global variables
    extern int32_t dword_6D1660;
    extern uint8_t byte_642C68[16];
};

// Game loop tick processing function
// Called from GameApp::OnIdle when server is active
void MainWindow::sub_48A756()
{
    try {
        sub_4271E6(byte_642C68);

        // Calculate next tick time
        int32_t next_tick_time = this->field_0x428 + (this->field_0x420 + 1) * this->field_0x42c;
        DWORD current_time = GetTickCount();

        if (next_tick_time <= (int32_t)current_time) {
            // Initialize timestamp on first tick
            if (this->field_0x420 == 0) {
                this->field_0x428 = GetTickCount();
            }
            
            // Increment and wrap tick counter (0-15)
            this->field_0x420 = (this->field_0x420 + 1) & 0x0F;
            
            // Process server
            g_Server->sub_4FBB79();
            
            sub_4271E6(byte_642C68);
            
            // Server mode processing (g_IsServer != 0 means server mode)
            if (g_IsServer != 0) {
                if (this->field_0x420 == 1) {
                    // Process network packets?
                    this->sub_484259();
                    
                    // Server shutdown notifications (message 0x0D).
                    if (g_ShutdownIn != 0x7FFFFFFF) {
                        if (g_ShutdownIn < 6000000) {
                            if (g_ShutdownIn > 1000) { // > 1 second
                                int32_t tick_interval = this->field_0x42c * 16;
                                if ((g_ShutdownIn % 60000) < tick_interval) {
                                    if (g_GameType == 0) {
                                        g_NetStru1_main.FUN_0051ce86(0x0D, g_ShutdownIn / 1000, nullptr);
                                    }
                                }
                            }
                            
                            if (g_ShutdownIn < 60000) {
                                int32_t tick_interval = this->field_0x42c * 16;
                                if ((g_ShutdownIn % 15000) < tick_interval) {
                                    if (g_GameType == 0) {
                                        g_NetStru1_main.FUN_0051ce86(0x0D, g_ShutdownIn / 1000, nullptr);
                                    }
                                }
                            }
                        }
                        
                        int32_t tick_interval = this->field_0x42c * 16;
                        g_ShutdownIn -= tick_interval;
                        
                        if (g_ShutdownIn <= 0) {
                            g_ShutdownIn = 0;
                            g_Server->sub_4F8831();
                            this->field_0x624 = 1;
                        }
                    }
                }
                
                if (this->field_0x800 != 0) {
                    sub_49585B(this->field_0x7fc);
                }
                
                // Check if should shut down.
                if (this->field_0x624 != 0) {
                    if (g_Server->FileList.GetSize() == 0) {
                        this->sub_41EA70(WM_CLOSE, 0, 0);
                    }
                }
                
                if (g_Server->field59_0x208 == 0) {
                    if (g_PlayersList->sub_53618F() == 0) {
                        if (g_Server->FileList.GetSize() == 0) {
                            g_Server->sub_4F1E2A();

                            // Get vtable and call virtual function at offset 0x48 (18-th virtual function).
                            void** vtable = *(void***)this->field_0x3d0;
                            void (__fastcall *vfunc)(void*, int, UINT, int, int) = 
                                (void (__fastcall *)(void*, int, UINT, int, int))vtable[0x48 / 4];

                            vfunc((void*)this->field_0x3d0, 0, 0x445, 0, 0);
                            return;
                        }
                    }
                }
                
                g_Server->map_elapsed_time += this->field_0x42c * 16;
                g_Server->map_elapsed_time2 += this->field_0x42c * 16;

                int32_t map_duration = g_MapDurations.GetAt(g_CurrentMapIndex);
                
                // Send "map will change soon" notifications (message 0x0C).
                if (map_duration != 0x7FFFFFFF) {
                    int32_t target_time = map_duration * 60000;
                    int32_t time_diff = target_time - g_Server->map_elapsed_time;
                    
                    if (time_diff < 659999 && time_diff > 1000) {
                        int32_t tick_interval = this->field_0x42c * 16;
                        
                        if ((time_diff % 60000) < tick_interval) {
                            if (g_GameType == 0 || g_GameType == 3) {
                                g_NetStru1_main.FUN_0051ce86(0x0C, time_diff / 1000, nullptr);
                            }
                        }
                        
                        if (time_diff < 60000) {
                            if ((time_diff % 15000) < tick_interval) {
                                if (g_GameType == 0 || g_GameType == 3) {
                                    g_NetStru1_main.FUN_0051ce86(0x0C, time_diff / 1000, nullptr);
                                }
                            }
                        }
                    }
                    
                    // Check if map should change now.
                    if (time_diff <= (this->field_0x42c * 16)) {
                        if (g_Server->field59_0x208 == 0) {
                            g_CurrentMapIndex++;
                            
                            int32_t size = g_MapDurations.GetSize();
                            if (g_CurrentMapIndex >= size) {
                                g_CurrentMapIndex = 0;
                            }
                            
                            if (g_GameType == 3) {
                                g_Server->sub_4FA5C4();
                            }
                            
                            g_Server->sub_4F94C0(0);

                            return;
                        }
                    }
                }
                
                // Some kind of timeout?
                int32_t timeout_threshold = dword_6D1660 * 60000;
                if (timeout_threshold <= g_Server->map_elapsed_time2) {
                    if (g_GameType == 3) {
                        if (g_Server->field59_0x208 == 0) {
                            g_Server->sub_4FA78E(2);
                        }
                    }
                }
            }
            
            // Client mode processing (g_IsServer == 0 means client mode).
            if (g_IsServer == 0) {
                if (this->field_0xbc != 0) {
                    if (this->field_0x3d0 != 0) {
                        if (this->field_0x420 == 1) {
                            // Call virtual function at offset 0x34 (13-th virtual function).
                            void** vtable = *(void***)this->field_0x3d0;
                            void (__fastcall *vfunc)(void*, int) = 
                                (void (__fastcall *)(void*, int))vtable[0x34 / 4];
                            vfunc((void*)this->field_0x3d0, 0);
                        }
                    }
                }
            }
            
            // Calculate sleep time for timing control
            int32_t desired_time = this->field_0x428 + (this->field_0x420 + 1) * this->field_0x42c;
            int32_t sleep_time = desired_time - GetTickCount();
            
            if (sleep_time > 1) {
                Sleep(sleep_time - 1);
            }
        }
        
        sub_4271E6(byte_642C68);
    } catch (...) {
        sub_48A747(); // exception handler that calls _exit(-1)
    }
}

