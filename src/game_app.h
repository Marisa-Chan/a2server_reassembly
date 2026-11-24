#ifndef GAME_APP_H
#define GAME_APP_H

#include <winsock2.h>
#include <afx.h>
#include <afxtempl.h>
#include <afxwin.h>

#include "net.h"
#include "table.h"
#include "txtfile.h"
#include "player.h"
#include "server.h"
#include "shop_assortment.h"

class Item;
class PlayersList;
struct Server;

#pragma pack(push, 1)
struct ItemNamePktEntry {
    uint16_t id;        // 0x00 - item type id (high byte = category, low byte = index)
    uint8_t gap_2;    // 0x02
    uint8_t gap_3;    // 0x03
    uint8_t flags;      // 0x04 - template flags (OR'd into runtime item flags)
    uint8_t mods_count;    // 0x05
    uint8_t data_len;   // 0x06 - length of variable data following this header
    uint8_t data[];     // 0x07 - variable-length data (data_len bytes)
    // For magic items (id & 0xF00 == 0xE00): *(int32_t*)&data[1] is patched with EquipData::shape
    // Total record size: 7 + data_len
};
#pragma pack(pop)
class NetStru1;



struct GameSettings {
    int32_t* pGameSpeed = nullptr;
    int32_t* pFormationMode = nullptr;
    int32_t* pWimpyMode = nullptr;
    int32_t* pShowAllHitPoints = nullptr;
    int32_t Smoothing = 1;
    int32_t* pShowFlyingHP = nullptr;
    int32_t ShowTimeFlow = 1;
    int32_t TipsMode = 1;
    int32_t Acknowledgement = 1;
    int32_t AutoCasting = AutoCastSettings::TIER_MEDIUM | AutoCastSettings::TARGET_OWN;
    int32_t* pShadows = nullptr;
    int32_t* pLightning = nullptr;
    int32_t* pAnimation = nullptr;
    int32_t ClanNames = 1;
    int32_t* pMessageColors = nullptr;

    void Init(); // 442386 (client: 444958)
    void Load(HKEY& rkey); // 442420 (client: 4449f2)
    void Save(HKEY& rkey); // 4421be (client: 444790)
};
ASSERT_SIZE(GameSettings, 0x3c);

extern GameSettings g_settings; //660ec0

extern CMapStringToString g_TunesMap; //660d80


// Base address 0x65fbbc; consumed by CProjectile::Draw (46e291) case 7 -> RenderDistort, freed by FreeDistortMap.
class DistortMap {
public:
    int16_t** offsets = nullptr; // 0x00 - grid of {dx, dy} pairs
    int32_t outer = 0;           // 0x04
    int32_t inner = 0;           // 0x08
    int32_t radius = 0;          // 0x0c

public:
    DistortMap(int32_t outer, int32_t inner); // 4a9f90 (client: 4adfc0)
    ~DistortMap(); // 4aa04f (client: 4ae07f)
    void BuildScaleTable(); // 4aa0af (client: 4ae0df)
    void RenderDistort(int32_t x, int32_t y); // 4aa29b (client: 4ae2cb)
};
ASSERT_SIZE(DistortMap, 0x10);

extern DistortMap* g_DistortMap; // 65fbbc

// Squared-distance lookup table for the visible radius (indexes 0..20 are unused).
extern uint16_t g_DistTable[41][41]; // 660000

extern uint32_t g_CpuFeatureFlags; // 660f84 - bit 1 = MMX available. We're hardcoding it to 1, because where would you get a non-MMX CPU nowadays?


class GameApp : public CWinApp {
public:
    static GameApp theApp;
public:

    virtual BOOL InitInstance() override; // 4823bd (client: 48626d)
#ifdef A2CLIENT_PATCH
    virtual int Run() override;
#endif
    virtual BOOL OnIdle(LONG lCount) override; // 4835b0 (client: 4872dc)
    virtual int ExitInstance() override; // 4833f1 (client: 48711d)
#ifdef A2SERVER_PATCH
    virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt) override;
#endif
};

void ReadFileToString(const char* fname, CString* str); // 4758cd (client: 47961d)

int32_t GetRandS16(int32_t range); // 4a4720 (client: 4a8090)


void __cdecl PostConsoleLine(const char* source); // 43a857 (client: 43cc47)

void LogMessage(CString message); // 43aa23 (client: 43ce08)
void LogWarning(const CString& msg); // 59b753 (client: 56a603)

int32_t ParseConfig(const char* fname); // 4f7188 (client: 4f8233)

void ParseTunes(); // 476b21 (client: 47a86d)
void LoadFonts(); // 460c37 (client: 464849)
void __cdecl SetMessageColors(uint32_t); // 476de7 (client: 47ab33)

int IsLateCampaignMission(); // 497490 (client: 49a8b0)

void SrandInit(); // 5421e9 (client: 5296c9)

int32_t __cdecl AppGetWorkingDir(int maxchar, char* buf); // 4761dd (client: 479f29)
int __cdecl AppHasAnySaveFile(); // 440d96 (client: 4431fd)
void __cdecl AppFindSavesList(CArray<WIN32_FIND_DATAA>* list, int p); // 43eaa0 (client: 440f04)


extern int32_t g_kbShiftState; //660f44
extern int32_t g_kbControlState; //660f40
extern CRect vis_scr_rect; //65fb88
extern int32_t g_kbMenuState; //660f48


extern GameDataRes g_GameDataRes; //6d0668
extern PlayersList* g_PlayersList; //6cdb24

int32_t __cdecl Random0N(int32_t n); // 542216 (client: 5296f6) - returns [0, n]
int32_t __cdecl Random1N(int32_t n); // 54223f (client: 52971f) - returns [1, n]

// RollRandomItem: Create a random item (level = item tier, max_gold = max price range).
Item* __cdecl RollRandomItem(int32_t level, int32_t max_gold); // 5499a6 (client: 52dd92)

double __cdecl Rand01(); // 5421fd (client: 5296dd) - returns rand()/RAND_MAX in [0,1)
Item* __cdecl RollItemInPriceRange(CString category, int32_t min_price, int32_t max_price); // 549372 (client: 52d7df) - create item of given category in price range
// pick a random (shape, material, item_data_id) combo from an equip table, skipping empty shape/material slots
void __cdecl RollItemDataId(CArray<WorldEquip>* table, int32_t* shape_id, int32_t* material_id, int32_t* item_data_id, int32_t min_id); // 5498b8 (client: 52dcaf)

void __stdcall ShowRoleKeyDialog(const char* str); // 41d723 (client: 41efa5)



struct ScenarioState {
    int32_t vars[512];
    int32_t locationEntrySelectionFlags[20];
    int32_t locationEntryStates[20];
    int32_t locationEntryLocationIds[20];
    int32_t reserved23cTo2ef[180];
    int32_t currentLocationTransientFlags[16];
    int32_t scenarioChapter;
    int32_t startMissionAllowed;
    int32_t specialDialogStateLowConfidence;
    int32_t innSceneUnlockedLowConfidence;
    int32_t followupLocationUnlockFlagLowConfidence;
    int32_t locationTransitionStateLowConfidence;
    int32_t laterSceneStateLowConfidence;
    int32_t rebuildAvailableLocationsPending;
    int32_t isMage;
    int32_t unk;
    int32_t pairedObjectiveBCompletedLowConfidence;
    int32_t endingVariantFlagLowConfidence;
    int32_t mission31UnlockedLowConfidence;
    int32_t isFemale;
    int32_t reserved30eTo37f[114];
    int32_t completedLocationFlags[128];
};
ASSERT_SIZE(ScenarioState, 0x1000);

struct ScenarioLocation;

int32_t __stdcall ScenarioGetVar(int32_t); // fnptr: 665a04 (client: 631474), scenario.dll: 10001cfd
void __stdcall ScenarioSetVar(int32_t, int32_t); // fnptr: 665a00 (client: 631470), scenario.dll: 10001d0e
void __stdcall ScenarioTalkTo(uint32_t); // fnptr: 665bd4 (client: 631644), scenario.dll: 100032c0
void __stdcall ScenarioEnterLocation(ScenarioLocation* loc); // fnptr: 6659fc (client: 63146c), scenario.dll: 10001d22
int32_t __stdcall ScenarioLeaveLocation(int32_t*); // fnptr: 6658c8 (client: 631338), scenario.dll: 10002806
int32_t __stdcall ScenarioEnterShop(); // fnptr: 665b0c (client: 63157c), scenario.dll: 1000285f
void __stdcall ScenarioLeaveShop(); // fnptr: 665b08 (client: 631578), scenario.dll: 10002866
void __stdcall ScenarioEnterInn(int32_t*, int32_t*); // fnptr: 665be8 (client: 631658), scenario.dll: 1000286b
void __stdcall ScenarioLeaveInn(); // fnptr: 6658f0 (client: 631360), scenario.dll: 100032bb
void __stdcall ScenarioNewGame(); // fnptr: 6658cc (client: 63133c), scenario.dll: 100033be
void __stdcall ScenarioSave(CFile*); // fnptr: 6658d0 (client: 631340), scenario.dll: 10003585
void __stdcall ScenarioLoad(CFile*); // fnptr: 665bd8 (client: 631648), scenario.dll: 10003686
CList<ScenarioLocation*>* __stdcall ScenarioGetAvailableLocations(); // fnptr: 6658c4 (client: 631334), scenario.dll: 100037de
AssortGenParams* __stdcall ScenarioGetShopAssortment(); // fnptr: 665bdc (client: 63164c), scenario.dll: 100037e8
int32_t __stdcall ScenarioIsTownAvailable(int32_t); // fnptr: 665cf4 (client: 631764), scenario.dll: 10003800
int32_t __stdcall ScenarioIsMissionAvailable(int32_t); // fnptr: 6659f8 (client: 631468), scenario.dll: 10003871
ScenarioLocation* __stdcall ScenarioGetCurrentLocation(); // fnptr: 6658f4 (client: 631364), scenario.dll: 100038c8
CList<ScenarioLocation*>* __stdcall ScenarioGetAllLocations(); // fnptr: 665cf0 (client: 631760), scenario.dll: 100038d2





// in C++
extern uint32_t g_CmdLatency; //665d08
extern uint32_t g_CmdTimeout; //630e24
extern uint32_t g_CmdSafeVideo; //660f7c

extern char g_CdSpeed[256]; //6658f8
extern char g_resolution[256]; //665bf0
extern char g_InitialWorkingDir[1024]; //6654c0
extern char g_WorkingDir[1024]; //65fbc8
extern HWND g_MainWndHWND; //6658ec

extern uint32_t g_IsCdPresent; //660f94
extern uint32_t g_EnableTrace; //660f78
extern uint32_t g_MapRevealed; //660f74


extern uint32_t g_VideoOptionFlags; //659d18

extern CString g_HelpStr; //660e68

extern TxtFile txt_main; //660e98
extern TxtFile txt_heropicture; //65fb98
extern TxtFile txt_stats; //65fba8
extern TxtFile txt_spells; //660d58
extern TxtFile txt_spell; //660E48
extern TxtFile txt_dialogs; //65FFF0
extern TxtFile txt_unitname; //660E58
extern TxtFile txt_building; //660DD8
extern TxtFile txt_itemname; //660D48
extern TxtFile txt_npcnames; //65FFD0
extern TxtFile txt_cutscene; //660DC8
extern TxtFile txt_cutpaths; //660E18
extern TxtFile txt_tunes; //660F30
extern TxtFile txt_patch; //65FFE0
extern TxtFile g_GlobalMapLocationHints; //660e88

extern ServerConfig g_ServerConfig; //6D15A0
extern int32_t g_EnableVideo; //62f87c  client:5fb96c

#endif
