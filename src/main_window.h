#pragma once

#include "asm_mfc.h"
#include "assert_offset.h"
#include "visual.h"
#include "mouse.h"

extern int32_t g_LButtonDown; //660f4c
extern int32_t DAT_00660f88; //660f88


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
    void FUN_004ac706(CFile* file);
    void FUN_004ac3ce(const Fame1& fame);

    void FUN_00420050();

    int32_t FUN_00497290() { return field_x0; }; //497290
};

ASSERT_SIZE(MWin_5e8, 0x38);


struct UserShortcut
{
    uint16_t kind;
    uint16_t item_id;
    uint32_t mods_size;
    uint8_t* mods;

    UserShortcut(); //4971a0
    ~UserShortcut(); //41e493

    void SetNull(); //4971c0
    int ToBuffer(uint8_t** buf); //41e5ed

    void WriteToFile(CFile* f); //41e4d3
};


struct HatCharId
{
    int32_t id1;
    int32_t id2;
};

class SomeMainStructure : public CObject
{
public:
    int32_t GetStringArray1Size(); //438d10
    void FUN_00493cd8(); //493cd8
    void FUN_00492c66(int32_t idx);  //492c66
    void FUN_004946e0(); //4946e0
    void FUN_004946ae(); //4946ae

    CString FUN_00420070(); //420070
    void FUN_00494982(); //494982

    int32_t FUN_00420110() { return field_0x118; }; //420110
    int32_t FUN_004200f0() { return field_0x114; }; //4200f0

    int FUN_00493ffe(); //493ffe
    void FUN_00493d8d(); //493d8d
    void FUN_00494687(); //494687
    void FUN_004941c0(); //4941c0
public:
    int32_t field_0x4;
    int32_t field_0x8;
    int32_t field_0xc;
    char character_name[32];
    int32_t flags;
    int32_t field_0x34;
    int32_t money;
    int32_t monster_killed;
    int32_t player_killed;
    int32_t field_0x44;
    int32_t death_count;
    int32_t field_0x4c;
    int32_t field_0x50;
    int32_t field_0x54;
    int32_t field_0x58;
    int32_t main_sphere;
    int32_t face;
    int32_t field_0x64;
    int32_t field_0x68;
    int32_t field_0x6c;
    int32_t field_0x70;
    int32_t field_0x74;
    int32_t field_0x78;
    int32_t field_0x7c;
    int32_t field_0x80;
    int32_t field_0x84;
    int32_t color;
    UserShortcut field_0x8c[9];
    CStringArray string_array1;
    int32_t field_0x10c;
    int32_t field_0x110;
    int32_t field_0x114;
    int32_t field_0x118;
    CArray<HatCharId> field_0x11c;
    CArray<HatCharId> field_0x130;
    int32_t field_0x144;
    int32_t field_0x148;
    int32_t field_0x14c;
    CStringArray string_array2;
    int32_t field_0x164;
    CWordArray word_array;
};
ASSERT_SIZE(SomeMainStructure, 0x17c);

struct HatSettings
{
    CString hatip;
    CString hatprogip;
    int32_t ishat;
    CString login;
    CString password;
    int32_t deathmatch;
    int32_t store;
};
ASSERT_SIZE(HatSettings, 0x1c);

struct Main3e0
{
    CString field_00;
    CString field_04;
    CString field_08;
    int32_t field_0c;
    int32_t field_10;
    CString field_14;
};
ASSERT_SIZE(Main3e0, 0x18);

struct DiplomacyEntry
{
    char* name = nullptr;
    int32_t enemy = 0;
    int32_t ally = 0;
    int32_t see = 0;
    int32_t mute = 0;

    DiplomacyEntry() = default;
    DiplomacyEntry(const char* _name, int e, int a, int s, int m);
    ~DiplomacyEntry();

};
ASSERT_SIZE(DiplomacyEntry, 0x14);


struct CLlNetSession;
struct AvailNetSession
{
    CString character_name;
    int32_t field_0x4;
    CLlNetSession* sessions;
    int32_t num_sessions;
    int32_t selected_index;
};
ASSERT_SIZE(AvailNetSession, 0x14);




class MainWindow : public CFrameWnd
{
public: // VTable at 0060c1a8.
    virtual const AFX_MSGMAP* GetMessageMap() const override; //483d54
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override; // 486c6c

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

    void ResetItemCursor(); //48cd44

    void FUN_00494a9e(); //494a9e

    void FUN_00494c91(); //494c91

    void FUN_00491b3e(); //491b3e

    void Proc_44c(CVisualObject* obj); //48b061

    void FUN_0048cff7(); //48cff7
    void FUN_00491a49(); //491a49
    void FUN_0048f905(); //48f905
    void FUN_004903d0(); //4903d0
    void FUN_00491822(); //491822
    int FUN_0048ca7e(int mode); //48ca7e
    void FUN_004918ae(); //4918ae

    void FUN_00491f7d(int32_t vid_id); //491f7d

    void PopUpScreen(VisScreen* screen); //48d26a   popup screen
    void UpdateCursorClip() { ClipCursor(&clip_cursor_rect); } //48cc87

public:
    int32_t field_0xbc;
    int32_t field_0xc0;
    void (*field_0xc4)();
    MusicPlayer* music_player;
    CVisualObject* vis_root;
    BigStruct2* field_0xd0; // BigStruct2*
    CVisualObject* field_0xd4;
    CVisualObject* field_0xd8;
    CVisualObject* field_0xdc;
    VisCharInfo* field_0xe0;
    CVisualObject* field_0xe4;
    VisInvType1* field_0xe8;
    VisSpellBook* field_0xec;
    CVisualObject* field_0xf0;
    VisScreen* field_0xf4;
    CVisualObject* field_0xf8;
    CVisualObject* field_0xfc;
    CVisualObject* field_0x100;
    VisTav* vis_tav;
    CVisualObject* field_0x108;
    CVisualObject* field_0x10c;
    VisTown* vis_town;
    CVisualObject* field_0x114;
    CVisualObject* field_0x118;
    CVisualObject* field_0x11c;
    CVisualObject* field_0x120;
    VisScreen* field_0x124;
    int32_t field_0x128;
    VisScreen* field_0x12c;
    CVisualObject* field_0x130;
    CVisualObject* field_0x134;
    Vis1200* field_0x138;
    LoadGameWindow* field_0x13c;
    VisScreen* field_0x140;
    VisMessageBox* field_0x144;
    MWin_Unk1 field_0x148;
    CArray<DiplomacyEntry*>* field_0x348;
    VisScreen* field_0x34c;
    VisScreen* field_0x350;
    AvailNetSession net_sessions;
    int32_t field_0x368;
    VisScreen* field_0x36c;
    VisCharSelect* vis_charsel;
    VisCharGen* field_0x374;
    VisScreen* field_0x378; // BigStruct1*
    VisScreen* field_0x37c;
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
    HatSettings hat_settings;
    VisScreen* field_0x3d0;
    VisScreen* field_0x3d4;
    CVisualObject* field_0x3d8;
    VisScreen* field_0x3dc;
    Main3e0 field_0x3e0;
    int32_t field_0x3f8;
    int32_t field_0x3fc;
    int32_t field_0x400;
    int32_t field_0x404;

    GO_d0* field_0x408;
    int32_t field_0x40c;
    int32_t field_0x410;
    CCursor* item_cursor;

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
    SomeMainStructure some_struc;
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
