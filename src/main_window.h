#pragma once

#include "asm_mfc.h"
#include "assert_offset.h"



class CListBox2 : public CListBox
{
public:
    CListBox2();
    ~CListBox2();

    virtual const AFX_MSGMAP* GetMessageMap() const override;

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

    static void __fastcall OnKeyDown(CListBox2*, void* edx, UINT, UINT, UINT);
    static void __fastcall OnLButtonDblClk(CListBox2*, void* edx, UINT, CPoint);

    void _OnKeyDown(UINT, UINT, UINT);
    void _OnLButtonDblClk(UINT, CPoint);
};


class CEdit2 : public CEdit
{
public:
    CEdit2();
    ~CEdit2();

    virtual const AFX_MSGMAP* GetMessageMap() const override;

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

    static void __fastcall OnKeyDown(CEdit2*, void* edx, UINT, UINT, UINT);
    void _OnKeyDown(UINT, UINT, UINT);
};


class MainWindow : public CFrameWnd
{
public: // VTable at 0060c1a8.
    // virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override; // sub_486c6c

public:
    void sub_48A756(); // Game loop tick processing function, called from GameApp::OnIdle when server is active.
    void sub_484259();
    void sub_41EA70(UINT msg, WPARAM wparam, LPARAM lparam);

    bool SetSpeed(int speed);

public:
    int32_t field_0xbc;
    int32_t field_0xc0;
    int32_t field_0xc4;
    int32_t field_0xc8;
    int32_t field_0xcc;
    void* field_0xd0; // BigStruct2*
    int32_t field_0xd4;
    int32_t field_0xd8;
    int32_t field_0xdc;
    int32_t field_0xe0;
    int32_t field_0xe4;
    int32_t field_0xe8;
    int32_t field_0xec;
    int32_t field_0xf0;
    int32_t field_0xf4;
    int32_t field_0xf8;
    int32_t field_0xfc;
    int32_t field_0x100;
    int32_t field_0x104;
    int32_t field_0x108;
    int32_t field_0x10c;
    int32_t field_0x110;
    int32_t field_0x114;
    int32_t field_0x118;
    int32_t field_0x11c;
    int32_t field_0x120;
    int32_t field_0x124;
    int32_t field_0x128;
    int32_t field_0x12c;
    int32_t field_0x130;
    int32_t field_0x134;
    int32_t field_0x138;
    int32_t field_0x13c;
    int32_t field_0x140;
    int32_t field_0x144;
    char field_0x148[256];
    char field_0x248[256];
    int32_t field_0x348;
    int32_t field_0x34c;
    int32_t field_0x350;
    CString field_0x354;
    int32_t field_0x358;
    int32_t field_0x35c;
    int32_t field_0x360;
    int32_t field_0x364;
    int32_t field_0x368;
    int32_t field_0x36c;
    int32_t field_0x370;
    int32_t field_0x374;
    void* field_0x378; // BigStruct1*
    int32_t field_0x37c;
    CString field_0x380;
    int32_t field_0x384;
    int32_t field_0x388;
    int32_t field_0x38c;
    int32_t field_0x390;
    int32_t field_0x394;
    int32_t field_0x398;
    int32_t field_0x39c;
    int32_t field_0x3a0;
    int32_t field_0x3a4;
    int32_t field_0x3a8;
    int32_t field_0x3ac;
    CString field_0x3b0;
    CString field_0x3b4;
    int32_t field_0x3b8;
    int32_t field_0x3bc;
    int32_t field_0x3c0;
    int32_t field_0x3c4;
    int32_t field_0x3c8;
    int32_t field_0x3cc;
    int32_t field_0x3d0;
    int32_t field_0x3d4;
    int32_t field_0x3d8;
    int32_t field_0x3dc;
    CString field_0x3e0;
    int32_t field_0x3e4;
    CString field_0x3e8;
    int32_t field_0x3ec;
    int32_t field_0x3f0;
    CString field_0x3f4;
    int32_t field_0x3f8;
    int32_t field_0x3fc;
    int32_t field_0x400;
    int32_t field_0x404;
    int32_t field_0x408;
    int32_t field_0x40c;
    int32_t field_0x410;
    int32_t field_0x414;
    int32_t field_0x418;
    int32_t field_0x41c;
    int32_t game_tic_counter;
    int32_t field_0x424;
    int32_t last_tic_time;
    int32_t game_tic_time;
    int32_t game_speed;
    int32_t field_0x434;
    int32_t field_0x438;
    int32_t field_0x43c;
    int32_t field_0x440;
    int32_t field_0x444;
    int32_t field_0x448;
    int32_t field_0x44c;
    int32_t field_0x450;
    int32_t field_0x454;
    int32_t field_0x458;
    int32_t field_0x45c;
    int32_t field_0x460;
    int32_t fields_0x464[111]; // I got tired of listing all the fields :)
    CString current_map_name;
    int32_t field_0x624;
    HCURSOR field_0x628;
    HCURSOR field_0x62c;
    HCURSOR field_0x630;
    HCURSOR field_0x634;
    HCURSOR field_0x638;
    int32_t field_0x63c;
    int32_t field_0x640;
    int32_t field_0x644;
    int32_t field_0x648;
    int32_t field_0x64c;
    int32_t field_0x650;
    CStatusBar status_bar;
    CListBox list_box1;
    CListBox2 list_box2;
    CStatic static1;
    CStatic static2;
    CEdit2 edit;
    CWnd* field_0x7fc;
    int32_t field_0x800;
    int32_t field_0x804;
};
ASSERT_OFFSET(MainWindow, field_0xc4, 0xc4);
ASSERT_OFFSET(MainWindow, field_0x148, 0x148);
ASSERT_OFFSET(MainWindow, current_map_name, 0x620);
ASSERT_OFFSET(MainWindow, list_box1, 0x6d0);
ASSERT_SIZE(MainWindow, 0x808);
