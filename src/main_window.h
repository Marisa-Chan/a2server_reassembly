#pragma once

#include "asm_mfc.h"
#include "assert_offset.h"
#include "visual.h"
#include "mouse.h"
#include "net.h"

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


struct SaveFileInfo
{
    char title[256];
    char filename[256];

    SaveFileInfo()
    {
        title[0] = 0;
        filename[0] = 0;
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

struct CFameHall
{
    uint32_t difficulty;
    uint32_t m_nTotalTime;
    uint32_t m_nDifficultyWeight;
    uint32_t m_nMaxEntries;
    CArray<Fame1> m_Entries;
    CArray<Fame2> m_Documents;

    CFameHall();
    ~CFameHall();

    void ZeroFactors();
    void SetDifficulty(uint32_t arg);
    uint32_t FUN_004ac566(int32_t arg);
    void FUN_004acafa();
    void FUN_004ac945(CFile* file);
    void FUN_004ac706(CFile* file);
    void FUN_004ac7a2(CFile* file);
    void FUN_004ac3ce(const Fame1& fame);

    void FUN_00420050();

    void SubmitScore(); //4ac498

    int32_t GetDifficulty() { return difficulty; }; //497290
    void AddMissionElapsedTime(int32_t t) { m_nTotalTime += t; } //497250
};

ASSERT_SIZE(CFameHall, 0x38);


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
    void ReadFromFile(CFile* f); //41e53b

    void LoadFromBuffer(uint8_t** buf); //41e689
};


union HatCharId
{
    uint64_t id;
    struct {
        int32_t id1;
        int32_t id2;
    };
};

//60c2a8
class CGameSession : public CObject
{
public:
    CGameSession(); //49248a
    ~CGameSession(); //4960f0

    int32_t GetStringArray1Size(); //438d10
    void FUN_00493cd8(); //493cd8
    void LoadCharacterRosterEntry(int32_t idx);  //492c66
    void FUN_004946e0(); //4946e0
    void FUN_004946ae(); //4946ae

    CString FUN_00420070(); //420070
    void FUN_00494982(); //494982

    int32_t FUN_00420110() { return field_0x118; }; //420110
    int32_t FUN_004200f0() { return field_0x114; }; //4200f0

    int SubmitCharacterSetupAndWaitForSelectedUnit(); //493ffe
    void FUN_00493d8d(); //493d8d
    void FUN_00494687(); //494687
    void RecreateCUnit(); //4941c0


    void RefreshCharacterRosterFiles(int val); //49265a
    void InitializeNewCharacterSession(int tp, const char* name); //493ab6
public:
    int32_t field_0x4 = 0;
    int32_t sessionKeyPart1 = 0;
    int32_t sessionKeyPart2 = 0;
    char character_name[32];
    int32_t flags = 0;
    int32_t type = 0;
    int32_t money = 0;
    int32_t monster_killed = 0;
    int32_t player_killed = 0;
    int32_t fragCount = 0;
    int32_t death_count = 0;
    int32_t body = 0;
    int32_t reaction = 0;
    int32_t mind = 0;
    int32_t spirit = 0;
    int32_t main_sphere = 0;
    int32_t face = 0;
    int32_t field_0x64 = 0;
    int32_t field_0x68 = 0;
    int32_t field_0x6c = 0;
    int32_t field_0x70 = 0;
    int32_t field_0x74 = 0;
    int32_t field_0x78 = 0;
    int32_t field_0x7c = 0;
    int32_t field_0x80 = 0;
    int32_t field_0x84 = 0;
    int32_t color = 0;
    UserShortcut shortcuts[9];
    CStringArray characterRosterNames;
    int32_t field_0x10c = 0;
    int32_t field_0x110 = 0;
    int32_t field_0x114 = 1;
    int32_t field_0x118 = 1;
    CArray<HatCharId> characterRosterHatId;
    CArray<HatCharId> field_0x130;
    int32_t field_0x144 = 0;
    int32_t field_0x148 = 0;
    int32_t field_0x14c = 0;
    CStringArray characterRosterFilePaths;
    int32_t selectedCharacterRosterFileIndex = -1;
    CWordArray word_array;
};
ASSERT_SIZE(CGameSession, 0x17c);

struct PhoneBook
{
    CString entry;
    CString dial;
    CStringArray phones;
};
ASSERT_SIZE(PhoneBook, 0x1c);

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
public:
    enum
    {
        MSG_421 = 0x421,
        MSG_428 = 0x428,
        MSG_42f = 0x42f,
        MSG_487 = 0x487,
    };
public: // VTable at 0060c1a8.
    virtual const AFX_MSGMAP* GetMessageMap() const override; //483d54
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override; // 486c6c
    LRESULT NewWindowProc(UINT message, WPARAM wParam, LPARAM lParam); // 486c6c

    MainWindow(); // 4837e1
    virtual ~MainWindow(); // 4961b0

    void CreateUI(); //484ab0
    void LoadData(); //483d64
    void LoadSettings(); //in asm 441a03

    void RemoteGameIdle(); //48acac
    void SingleGameTimedIdle(); //48a543
    void SingleGameIdle(); //48a3a2

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

    void ShowStartGameSetupForNewSession(); //491b3e

    void Proc_44c(CVisualObject* obj); //48b061

    void ShowCharacterLoaderDialog(); //48cff7
    void FUN_00491a49(); //491a49
    void FUN_0048f905(); //48f905
    void FUN_004903d0(); //4903d0
    void FUN_00491822(); //491822
    int FUN_0048ca7e(int mode); //48ca7e
    void FUN_004918ae(); //4918ae

    void FUN_00491f7d(int32_t vid_id); //491f7d

    void FUN_0048f79d(); //48f79d
    int GetSaveFileInBattle(); //48de6f
    int FUN_0048e502(int mode); //48e502
    void FUN_00485969(); //485969
    void FUN_0047a5e6(); //47a5e6
    void FUN_0047efdf(); //47efdf
    void FUN_0047e612(); //47e612
    void FUN_0048df44(); //48df44

    void FUN_0048f6f7(); //48f6f7
    void FUN_0047ad28(); //47ad28

    void FUN_00478130(); //478130 in asm

    void FUN_0047eab6(); //47eab6
    void FUN_0047f5e4(); //47f5e4

    void ShowGlobalMapDialog(); //48d34b

    void InitNewCampaignSession(); //48dd3a

    void ShowStartupLogoDialog(); // 48dba0

    void PopUpScreen(VisScreen* screen); //48d26a   popup screen
    void UpdateCursorClip() { ClipCursor(&clip_cursor_rect); } //48cc87
    void SetMusicProc(void (*func)()) { music_update_proc = func; } //45cc80

    void SetControlPositions(); //484127

public:
    int32_t field_0xbc;
    int32_t field_0xc0;
    void (*music_update_proc)();
    MusicPlayer* music_player;
    CVisualObject* vis_root;
    BigStruct2* vis_map_context; // 0xd0
    VisRightPanel* vis_right_panel; //0xd4
    VisMiniMap* vis_minimap;
    VisOrderToolbar* vis_ordertoolbar;
    VisCharInfo* vis_charinfo;
    VisSideStatus* vis_sidestatus;
    VisInvType1* vis_invtype1;
    VisSpellBook* vis_spellbook;
    VisGlobalMap* vis_globalmap;
    VisMainMenu* vis_mainmenu;
    VisShop* vis_shop;
    VisShopDruid* vis_shopdruid;
    VisShopKaarg* vis_shopkaarg;
    VisTav* vis_tav;
    VisTavDruid* vis_tavdruid;
    VisTavKaarg* vis_tavkaarg;
    VisTown* vis_town;
    VisTownDruid* vis_towndruid;
    VisTownKaarg* vis_townkaarg;
    VisFameHall* vis_famehall;
    VisDropGold* vis_dropgold;
    VisScreen* field_0x124;
    int32_t field_0x128;
    VisScreen* field_0x12c;
    VisCredits* vis_credits;
    VisLogoWnd* vis_logownd;
    Vis1200* vis_1200;
    LoadGameWindow* field_0x13c;
    SaveGameWindow* field_0x140;
    VisMessageBox* field_0x144;
    SaveFileInfo field_0x148;
    CArray<DiplomacyEntry*>* field_0x348;
    VisScreen* field_0x34c;
    VisScreen* field_0x350;
    AvailNetSession net_sessions;
    int32_t field_0x368;
    VisCharGen* vis_chargen;
    VisCharSelect* vis_charsel;
    VisStartGame* vis_startgame; //0x374
    VisScreen* field_0x378; // BigStruct1*
    VisScreen* field_0x37c;
    PhoneBook phone_book; //380
    ComSettings com_settings; //39c
    CString last_ip; //3b0
    HatSettings hat_settings; //3b4
    VisScreen* field_0x3d0;
    VisScreen* field_0x3d4;
    VisFameDocument* vis_famedocument;
    VisScreen* field_0x3dc;
    Main3e0 field_0x3e0;
    int32_t field_0x3f8;
    int32_t field_0x3fc;
    int32_t field_0x400;
    int32_t field_0x404;

    TokenEntry* field_0x408;
    int32_t field_0x40c;
    int32_t field_0x410;
    CCursor* item_cursor;

    int32_t dialogsMask; //dialogsMask
    int32_t serverLoopCounter;
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
    CGameSession m_GameSession;
    int32_t field_0x5e4;
    CFameHall m_FameHall; //5e8
    CString current_map_name;
    int32_t field_0x624;
    HCURSOR cursor_sizewe;
    HCURSOR cursor_sizens;
    HCURSOR cursor_sizenwse;
    HCURSOR cursor_sizenesw;
    HCURSOR cursor_arrow;
    int32_t serverBootstrapEnabled; //0x63c
    int32_t sessionMode; //0x640
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
ASSERT_OFFSET(MainWindow, music_update_proc, 0xc4);
ASSERT_OFFSET(MainWindow, field_0x148, 0x148);
ASSERT_OFFSET(MainWindow, current_map_name, 0x620);
ASSERT_OFFSET(MainWindow, list_box1, 0x6d0);
ASSERT_SIZE(MainWindow, 0x808);
