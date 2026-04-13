#pragma once

#include "asm_mfc.h"
#include "assert_offset.h"
#include "visual.h"


extern int32_t g_LButtonDown; //660f4c


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


struct MWin_Unk1
{
    char buf1[256];
    char buf2[256];

    MWin_Unk1()
    {
        buf1[0] = 0;
        buf2[0] = 0;
    }
};


struct Fame1
{
    CString str;
    uint32_t field_x4 = 0;
    uint32_t field_x8 = 0;
    uint32_t field_xc = 0;
};

ASSERT_SIZE(Fame1, 0x10);

struct Fame2
{
    int32_t field_x0;
    uint32_t field_x4;
    CObject* field_x8;  // CObject?
    CString str;
    CStringArray str_arr;
    CRect rect;
    uint32_t field_x34;
    uint32_t field_x38;

    Fame2();
    void operator=(const Fame2& b);

    void Clear();
    void FUN_004abf0f(int32_t arg1, uint32_t arg2);
    int32_t FUN_004ad880();
    uint32_t FUN_004ad890();

};

ASSERT_SIZE(Fame2, 0x3C);

struct MWin_5e8
{
    uint32_t field_x0;
    uint32_t field_x4;
    uint32_t field_x8;
    uint32_t field_xc;
    CArray<Fame1> fame1_arr;
    CArray<Fame2> fame2_arr;

    MWin_5e8();
    ~MWin_5e8();

    void FUN_004ac3af();
    void FUN_00497270(uint32_t arg);
    uint32_t FUN_004ac566(int32_t arg);
    void FUN_004acafa();
    void FUN_004ac945(CFile* file);
    void FUN_004ac3ce(const Fame1& fame);
};

ASSERT_SIZE(MWin_5e8, 0x38);


struct SomeMainSubStruc1
{
    uint16_t field_0x0;
    uint16_t field_0x2;
    uint32_t field_0x4;
    uint32_t field_0x8;
};


class SomeMainStructure : public CObject
{
public:
    int32_t GetStringArray1Size(); //438d10
    void FUN_00493cd8(); //493cd8
    void FUN_00492c66(int32_t idx);  //492c66

public:
    int32_t field_0x4;
    int32_t field_0x8;
    int32_t field_0xc;
    char character_name[32];
    int32_t field_0x30;
    int32_t field_0x34;
    int32_t field_0x38;
    int32_t field_0x3c;
    int32_t field_0x40;
    int32_t field_0x44;
    int32_t field_0x48;
    int32_t field_0x4c;
    int32_t field_0x50;
    int32_t field_0x54;
    int32_t field_0x58;
    int32_t field_0x5c;
    int32_t field_0x60;
    int32_t field_0x64;
    int32_t field_0x68;
    int32_t field_0x6c;
    int32_t field_0x70;
    int32_t field_0x74;
    int32_t field_0x78;
    int32_t field_0x7c;
    int32_t field_0x80;
    int32_t field_0x84;
    int32_t field_0x88;
    SomeMainSubStruc1 field_0x8c[9];
    CStringArray string_array1;
    int32_t field_0x10c;
    int32_t field_0x110;
    int32_t field_0x114;
    int32_t field_0x118;
    CArray<void*> field_0x11c;
    CArray<void*> field_0x130;
    int32_t field_0x144;
    int32_t field_0x148;
    int32_t field_0x14c;
    CStringArray string_array2;
    int32_t field_0x164;
    CWordArray word_array;
};
ASSERT_SIZE(SomeMainStructure, 0x17c);

class MainWindow : public CFrameWnd
{
public: // VTable at 0060c1a8.
    // virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override; // sub_486c6c

    MainWindow(); //in asm 4837e1
    virtual ~MainWindow(); //in asm 4961b0

    void CreateUI(); //in asm 484ab0
    void LoadData(); //in asm 483d64

public:
    void sub_48A756(); // Game loop tick processing function, called from GameApp::OnIdle when server is active.
    void sub_484259();
    void sub_41EA70(UINT msg, WPARAM wparam, LPARAM lparam);

    int SetSpeed(int speed);

    BOOL UpdateClipCursor();

    void Proc_421(); //48d49b

    void FUN_0048cb3c(); //48cb3c

    void FUN_00485a41(); //485a41

    void ModalScreen(VisScreen* scr);

public:
    int32_t field_0xbc;
    int32_t field_0xc0;
    void (*field_0xc4)();
    int32_t field_0xc8;
    CVisualObject* vis_root;
    CVisualObject* field_0xd0; // BigStruct2*
    CVisualObject* field_0xd4;
    CVisualObject* field_0xd8;
    CVisualObject* field_0xdc;
    CVisualObject* field_0xe0;
    CVisualObject* field_0xe4;
    CVisualObject* field_0xe8;
    CVisualObject* field_0xec;
    CVisualObject* field_0xf0;
    VisScreen* field_0xf4;
    CVisualObject* field_0xf8;
    CVisualObject* field_0xfc;
    CVisualObject* field_0x100;
    CVisualObject* field_0x104;
    CVisualObject* field_0x108;
    CVisualObject* field_0x10c;
    int32_t field_0x110;
    CVisualObject* field_0x114;
    CVisualObject* field_0x118;
    CVisualObject* field_0x11c;
    CVisualObject* field_0x120;
    int32_t field_0x124;
    int32_t field_0x128;
    int32_t field_0x12c;
    CVisualObject* field_0x130;
    CVisualObject* field_0x134;
    CVisualObject* field_0x138;
    int32_t field_0x13c;
    int32_t field_0x140;
    int32_t field_0x144;
    MWin_Unk1 field_0x148;
    int32_t field_0x348;
    int32_t field_0x34c;
    int32_t field_0x350;
    CString field_0x354;
    int32_t field_0x358;
    int32_t field_0x35c;
    int32_t field_0x360;
    int32_t field_0x364;
    int32_t field_0x368;
    CVisualObject* field_0x36c;
    CVisualObject* field_0x370;
    CVisualObject* field_0x374;
    CVisualObject* field_0x378; // BigStruct1*
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
    CVisualObject* field_0x3d0;
    int32_t field_0x3d4;
    CVisualObject* field_0x3d8;
    VisScreen* field_0x3dc;
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
    int32_t field_0x464;
    SomeMainStructure field_0x468;
    int32_t field_0x5e4;
    MWin_5e8 field_0x5e8;
    CString current_map_name;
    int32_t field_0x624;
    HCURSOR cursor_sizewe;
    HCURSOR cursor_sizens;
    HCURSOR cursor_sizenwse;
    HCURSOR cursor_sizenesw;
    HCURSOR cursor_arrow;
    int32_t field_0x63c;
    int32_t field_0x640;
    RECT clip_cursor_rect; //0x644
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
