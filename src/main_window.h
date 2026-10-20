#pragma once
#include <winsock2.h>
#include <afx.h>
#include <afxwin.h>
#include <afxext.h>
#include "assert_offset.h"
#include "visual.h"
#include "mouse.h"
#include "net.h"

extern int32_t g_LButtonDown; //660f4c
extern int32_t DAT_00660f88; //660f88
extern int32_t INT_00660f8c; //660f8c

void SaveSettings(); //441b1e


class CListBox2 : public CListBox {
public:
    CListBox2();
    ~CListBox2();

    virtual const AFX_MSGMAP* GetMessageMap() const override;

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

    void OnKeyDown(UINT, UINT, UINT);
    void OnLButtonDblClk(UINT, CPoint);
};


class CEdit2 : public CEdit {
public:
    CEdit2();
    ~CEdit2();

    virtual const AFX_MSGMAP* GetMessageMap() const override;

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

    void OnKeyDown(UINT, UINT, UINT);
};


struct SaveFileInfo {
    char title[256];
    char filename[256];

    SaveFileInfo() {
        this->title[0] = 0;
        this->filename[0] = 0;
    }
};


struct Fame1 {
    CString str;
    uint32_t score = 0;
    uint32_t field_x8 = 0;
    uint32_t field_xc = 0;
};

ASSERT_SIZE(Fame1, 0x10);

struct Fame2 {
    int32_t doc_id;
    uint32_t is_text;
    CBmp64* picture;
    CString str;
    CStringArray str_arr;
    CRect rect;
    uint32_t field_x34;
    uint32_t top_line;

    Fame2();
    void operator=(const Fame2& b);

    void Clear();
    void SetDocument(int32_t arg1, uint32_t arg2);
    int32_t GetDocId();
    uint32_t GetIsText();
    bool ScrollDown(); // 4abf35 in asm
    int32_t ScrollUp(); // 4abf72 in asm
    void Draw(int32_t x, int32_t y); // 4abfb1 in asm
    void LoadDocument(); // 4ac0af in asm

};

ASSERT_SIZE(Fame2, 0x3C);

struct CFameHall {
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
    uint32_t AddTextDocument(int32_t arg);
    void SeedDefaultEntries();
    void LoadEntries(CFile* file);
    void SaveEntries(CFile* file); //4ac839 - save m_Entries to file
    void SaveDocuments(CFile* file);
    void LoadDocuments(CFile* file);
    void AddEntry(const Fame1& fame);

    void AddDifficultyPoint();

    void SubmitScore(); //4ac498

    int32_t GetDifficulty() { return this->difficulty; }; //497290
    void AddMissionElapsedTime(int32_t t) { this->m_nTotalTime += t; } //497250
};

ASSERT_SIZE(CFameHall, 0x38);


struct UserShortcut {
    uint16_t kind;
    uint16_t item_id;
    uint32_t mods_size;
    uint8_t* mods;

    UserShortcut(); //4971a0
    ~UserShortcut(); //41e493

    void SetNull(); //4971c0
    int ToBuffer(uint8_t** buf); //41e5ed

    void SetSpell(short spell_id); //41e323 in asm
    int32_t IsSpellShortcut(short spell_id); //41e456 in asm
    int32_t MatchesItem(TokenEntry* entry); //41e3af in asm
    void SetItem(TokenEntry* entry); //41e343 in asm

    void WriteToFile(CFile* f); //41e4d3
    void ReadFromFile(CFile* f); //41e53b

    void LoadFromBuffer(uint8_t** buf); //41e689
};



//60c2a8
class CGameSession : public CObject {
public:
    CGameSession(); //49248a
    ~CGameSession(); //4960f0

    int32_t GetStringArray1Size(); //438d10
    void DeleteSelectedCharacter(); //493cd8
    void LoadCharacterRosterEntry(int32_t idx);  //492c66
    void AutosaveCharacter(); //4946e0
    void AutosaveTick(); //4946ae

    CString GetSelectedRosterPath(); //420070
    void SaveKillStats(); //494982

    int32_t FUN_00420110() { return this->field_0x118; }; //420110
    int32_t FUN_004200f0() { return this->field_0x114; }; //4200f0

    int SubmitCharacterSetupAndWaitForSelectedUnit(); //493ffe
    void ResetCharacter(); //493d8d
    void ResetUnitClass(); //494687
    void RecreateCUnit(); //4941c0
    void SetCharacterStats(int32_t body, int32_t reaction, int32_t mind, int32_t spirit, int32_t main_sphere); //494148 in asm


    void RefreshCharacterRosterFiles(int val); //49265a
    void InitializeNewCharacterSession(int tp, const char* name); //493ab6
    void SetNameAndColor(char* name); //49381c in asm
    void FUN_004948b2(); //4948b2 in asm
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
    int32_t hat_id = 0;
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
    int32_t phased_in = 0;
    int32_t last_autosave_tick = 0;
    int32_t field_0x114 = 1;
    int32_t field_0x118 = 1;
    CArray<HatCharId> characterRosterHatId;
    CArray<HatCharId> hat_char_ids;
    int32_t field_0x144 = 0;
    int32_t field_0x148 = 0;
    int32_t field_0x14c = 0;
    CStringArray characterRosterFilePaths;
    int32_t selectedCharacterRosterFileIndex = -1;
    CWordArray word_array;
};
ASSERT_SIZE(CGameSession, 0x17c);

struct PhoneBook {
    CString entry;
    CString dial;
    CStringArray phones;

    void Load(HKEY& rkey); //43ce7d
    void Save(HKEY& rkey); //43cf29
};
ASSERT_SIZE(PhoneBook, 0x1c);

struct HatSettings {
    CString hatip;
    CString hatprogip;
    int32_t ishat;
    CString login;
    CString password;
    int32_t deathmatch;
    int32_t store;

    void Load(HKEY& rkey); //43d249
    void Save(HKEY& rkey); //43d51b
};
ASSERT_SIZE(HatSettings, 0x1c);

struct Main3e0 {
    CString field_00;
    CString field_04;
    CString field_08;
    int32_t hat_id;
    int32_t field_10;
    CString connect_address;
};
ASSERT_SIZE(Main3e0, 0x18);

struct DiplomacyEntry {
    char* name = nullptr;
    int32_t enemy = 0;
    int32_t ally = 0;
    int32_t see = 0;
    int32_t mute = 0;

    DiplomacyEntry() = default;
    DiplomacyEntry(const char* _name, int e, int a, int s, int m); //445a46 in asm
    ~DiplomacyEntry();

};
ASSERT_SIZE(DiplomacyEntry, 0x14);


struct CLlNetSession;
struct AvailNetSession {
    CString character_name;
    int32_t field_0x4;
    CLlNetSession* sessions;
    int32_t num_sessions;
    int32_t selected_index;
};
ASSERT_SIZE(AvailNetSession, 0x14);




// Server control dialog, resource 0x6b.
class ServerControlDialog : public CDialog {
public:
    CEdit edit_5c;       // 0x5c
    CStatic static_98;   // 0x98
    CButton button_d4;   // 0xd4
    CButton button_110;  // 0x110
    CButton button_14c;  // 0x14c
    CButton button_188;  // 0x188
    CButton button_1c4;  // 0x1c4
    CButton button_200;  // 0x200
    CButton button_23c;  // 0x23c
    CString str_278;     // 0x278
    CString str_27c;     // 0x27c

public:
    ServerControlDialog(CWnd* parent_window); // 4951bd

    // VTable at 60C458.
    virtual ~ServerControlDialog() override; // 496900
    virtual const AFX_MSGMAP* GetMessageMap() const override; // 495429
protected:
    virtual void DoDataExchange(CDataExchange* pDX) override; // 4952fa

public:
    void UpdateControlStates(); // 49585B — update control states.

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

    // Message handlers
    void OnDestroy(); // 495439 — ON_WM_DESTROY
    void OnCommand3f5(); // 49544C — ON_COMMAND 0x3f5
    void OnCommand3f6(); // 49549B — ON_COMMAND 0x3f6
    void OnCommandUnknown(); // 4954EA
    void OnClose(); // 4956D5 — ON_WM_CLOSE
    void OnCommand3ff(); // 495774 — ON_COMMAND 0x3ff
    void OnCommand3f7(); // 4957AD — ON_COMMAND 0x3f7
    void OnCommand3fb(); // 4957E2 — ON_COMMAND 0x3fb
    void OnCommand3fe(); // 49583E — ON_COMMAND 0x3fe
};

ASSERT_SIZE(ServerControlDialog, 0x280);


class MainWindow : public CFrameWnd {
public:
    enum {
        MSG_421 = 0x421,
        MSG_428 = 0x428,
        MSG_42f = 0x42f,
        MSG_487 = 0x487,
    };

    static const AFX_MSGMAP_ENTRY _messageEntries[];
    static AFX_DATA const AFX_MSGMAP messageMap;

public: // VTable at 0060c1a8.
    virtual const AFX_MSGMAP* GetMessageMap() const override; //483d54
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override; // 486c6c

    MainWindow(); // 4837e1
    virtual ~MainWindow(); // 4961b0

    void CreateUI(); //484ab0
    void LoadData(); //483d64
    void LoadSettings(); //441a03

    void RemoteGameIdle(); //48acac
    void SingleGameTimedIdle(); //48a543
    void SingleGameIdle(); //48a3a2

    void OnDestroy(); //483ea8
    int OnCreate(LPCREATESTRUCT lpCreateStruct); //484022
    void OnPaint(); //484a1a
    BOOL OnSetCursor(CWnd*, UINT, UINT); //486391
    void OnSetFocus(CWnd*); //4863c6
    void OnKeyDown(UINT, UINT, UINT); //4863f6
    void OnSysKeyDown(UINT, UINT, UINT); //486a2f
    void OnKeyUp(UINT, UINT, UINT); //486bb0
    void OnSysKeyUp(UINT, UINT, UINT); //486c25
    void OnChar(UINT, UINT, UINT); //48a35f
    void OnKillFocus(CWnd*); //48b01d
    void OnClose(); //4949f9
    void OnSize(UINT, int, int); //494e8e
    void BeginShutdown(); //495010
    void ShowServerControlDialog(); //4950c0
    void MarkHatSessionEnded(); //495743
    void OnSizing(UINT, LPRECT); //495ab2
    void OnNcMouseMove(UINT, CPoint); //495b9c
    void OnNcLButtonDown(UINT, CPoint); //495c6e
    
public:
    void IdleLoop(); // Game loop tick processing function, called from GameApp::OnIdle when server is active.
    void ServerTickPump();
    void sub_41EA70(UINT msg, WPARAM wparam, LPARAM lparam);

    int SetSpeed(int speed);

    BOOL UpdateClipCursor();

    void Proc_421(); //48d49b

    void DestroyServer(); //48cb3c

    void UpdateMapRect(); //48cbe2

    void ApplyCursorClip(); //48cc87

    void OpenConsoleInput(); //4919c7
    void ProcessPlayersTick(); //4919b1

    void ShowRightPanelInfo(); //485a41
    void ClearVisualTree(); //485aed in asm

    void ModalScreen(VisScreen* scr); //494a31

    void ResetItemCursor(); //48cd44

    void SetItemCursor(TokenEntry* entry, int32_t idx, const char* str, int32_t tab); //48cca1 in asm

    void ClearDiplomacyEntries(); //494a9e

    void PlayDefaultMusic(); //494c91

    void ShowStartGameSetupForNewSession(); //491b3e

    void Proc_44c(CVisualObject* obj); //48b061

    void ShowCharacterLoaderDialog(); //48cff7
    void ShowCharInfoPane(); //491a49
    void AutosaveGame(); //48f905
    void EnterSinglePlayerMode(); //4903d0
    void FUN_00491822(); //491822
    int FUN_0048ca7e(int mode); //48ca7e
    void FUN_004918ae(); //4918ae

    void FUN_00491f7d(int32_t vid_id); //491f7d

    void FUN_0048f79d(); //48f79d
    int GetSaveFileInBattle(); //48de6f
    int RunSessionBootstrap(int mode); //48e502
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
    void ShowCreditsDialog(); //48da66
    void ShowFameHallDialog(); //48d096
    void ShowShopDialog(uint32_t id); //48cdce
    void ShowInnDialog(uint32_t id, int32_t interact_target); //48d4da

    void PopUpScreen(VisScreen* screen); //48d26a   popup screen
    void SetMusicProc(void (*func)()) { this->music_update_proc = func; } //45cc80

    void SetControlPositions(); //484127

    void StartHatDedicatedServer(); //49172d

    void ShowCurrentTownDialog(); //48d815 in asm
    int32_t WaitHatConnection(); //490f6c in asm
    int32_t ConnectToServerAddress(); //491142 in asm
    void WaitConnectionCharacterJoin(); //49152f in asm
    void ShowFameHallDocDlg(); //48d125 in asm

    void SpawnScenarioMissionUnits(int32_t id, const CDWordArray& array); //50260a
    int32_t FUN_0049057b(); //49057b
    int32_t FUN_004e5466(const CString& str); //4e5466
    int32_t FUN_00490eb3(); //490eb3

public:
    int32_t has_focus;
    int32_t console_active;
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
    VisScreen* vis_missionfailed;
    Vis2Action* vis_2action;
    VisScreen* field_0x12c;
    VisCredits* vis_credits;
    VisLogoWnd* vis_logownd;
    Vis1200* vis_1200;
    LoadGameWindow* vis_loadgame;
    SaveGameWindow* vis_savegame;
    VisMessageBox* vis_msgbox;
    SaveFileInfo last_save_info;
    CArray<DiplomacyEntry*>* diplomacy_entries;
    VisScreen* field_0x34c;
    VisConnectionDlg* vis_connectiondlg;
    AvailNetSession net_sessions;
    int32_t last_protocol;
    VisCharGen* vis_chargen;
    VisCharSelect* vis_charsel;
    VisStartGame* vis_startgame; //0x374
    VisNetMapSelection* vis_netmapsel; // BigStruct1*
    VisHatBrowserDlg* vis_hatbrowser;
    PhoneBook phone_book; //380
    ComSettings com_settings; //39c
    CString last_ip; //3b0
    HatSettings hat_settings; //3b4
    VisScreen* vis_serverscreen;
    VisScreen* vis_cutscenes;
    VisFameDocument* vis_famedocument;
    VisScreen* vis_msgbox_list;
    Main3e0 field_0x3e0;
    int32_t in_famehall_mission;
    int32_t mission_failed_reported;
    int32_t field_0x400;
    int32_t pending_game_mode;

    TokenEntry* drag_entry;
    int32_t drag_src_slot;
    int32_t drag_src_tab;
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
    int32_t unlimited_speed;
    int32_t field_0x44c;
    int32_t last_event_id;
    int32_t inside_building;
    int32_t enter_ack_received;
    int32_t input_locked;
    int32_t field_0x460;
    int32_t field_0x464;
    CGameSession m_GameSession;
    int32_t field_0x5e4;
    CFameHall m_FameHall; //5e8
    CString current_map_name;
    int32_t shutdown_pending;
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
    ServerControlDialog* server_ctrl_dlg;
    int32_t server_ctrl_shown;
    int32_t field_0x804;
};
ASSERT_OFFSET(MainWindow, music_update_proc, 0xc4);
ASSERT_OFFSET(MainWindow, field_0x148, 0x148);
ASSERT_OFFSET(MainWindow, current_map_name, 0x620);
ASSERT_OFFSET(MainWindow, list_box1, 0x6d0);
ASSERT_SIZE(MainWindow, 0x808);
