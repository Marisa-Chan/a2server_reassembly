#ifndef GAME_APP_H
#define GAME_APP_H

#include "asm_mfc.h"
#include "mfc_templ.h"
#include "net.h"
#include "table.h"
#include "txtfile.h"
#include "server.h"
#include "shop_assortment.h"

class Item;
class PlayersList;
struct Server;

#pragma pack(push, 1)
struct ItemNamePktEntry {
	uint16_t id;        // 0x00 - item type id (high byte = category, low byte = index)
	uint8_t field_2;    // 0x02
	uint8_t field_3;    // 0x03
	uint8_t flags;      // 0x04 - template flags (OR'd into runtime item flags)
	uint8_t field_5;    // 0x05
	uint8_t data_len;   // 0x06 - length of variable data following this header
	uint8_t data[];     // 0x07 - variable-length data (data_len bytes)
	// For magic items (id & 0xF00 == 0xE00): *(int32_t*)&data[1] is patched with EquipData::shape
	// Total record size: 7 + data_len
};
#pragma pack(pop)
class NetStru1;



struct GameSettings
{
	int32_t* pGameSpeed = nullptr;
	int32_t* pFormationMode = nullptr;
	int32_t* pWimpyMode = nullptr;
	int32_t* pShowAllHitPoints = nullptr;
	int32_t Smoothing = 1;
	int32_t* pShowFlyingHP = nullptr;
	int32_t ShowTimeFlow = 1;
	int32_t TipsMode = 1;
	int32_t Acknowledgement = 1;
	int32_t AutoCasting = 25;
	int32_t* pShadows = nullptr;
	int32_t* pLightning = nullptr;
	int32_t* pAnimation = nullptr;
	int32_t ClanNames = 1;
	int32_t* pMessageColors = nullptr;

	void Init(); // 442386
	void Load(HKEY& rkey); //442420
};
ASSERT_SIZE(GameSettings, 0x3c);

extern GameSettings g_settings; //660ec0

extern CMapStringToString g_TunesMap; //660d80


// Base address 0x65fbbc; consumed by sub_46E291 case 7 -> sub_4AA29B, freed by sub_47619A.
class DistortMap
{
public:
	int16_t** offsets = nullptr; // 0x00 - grid of {dx, dy} pairs
	int32_t outer = 0;           // 0x04
	int32_t inner = 0;           // 0x08
	int32_t radius = 0;          // 0x0c

public:
	DistortMap(int32_t outer, int32_t inner); // 4a9f90
	void sub_4AA0AF(); // 4aa0af
};
ASSERT_SIZE(DistortMap, 0x10);

extern DistortMap* g_DistortMap; // 65fbbc

// Squared-distance lookup table for the visible radius (indexes 0..20 are unused).
extern uint16_t g_DistTable[41][41]; // 660000

extern uint32_t g_CpuFeatureFlags; // 660f84 - bit 1 = MMX available. We're hardcoding it to 1, because where would you get a non-MMX CPU nowadays?


class GameApp : public CWinApp
{
public:
	static GameApp theApp;
public:

	virtual BOOL InitInstance() override;
	virtual BOOL OnIdle(LONG lCount) override;
	virtual int ExitInstance() override;
};

void ReadFileToString(const char* fname, CString* str);

int32_t GetRandS16(int32_t range);


void __cdecl sub_43A857(const char* source); // 43A857

/* in asm */
void LogMessage(CString message); //43AA23
void LogWarning(const CString& msg); //59b753

int32_t ParseConfig(const char* fname); //4f7188

void ParseTunes(); //476b21
void LoadFonts();  //460c37
void LoadSettings(); //441a03
void __cdecl SetMessageColors(uint32_t); //476de7

int FUN_00497490(); //497490

void SrandInit(); //5421e9

int32_t __cdecl AppGetWorkingDir(int maxchar, char* buf); //4761dd
int __cdecl AppHasAnySaveFile(); //440d96
void __cdecl AppFindSavesList(CArray<WIN32_FIND_DATAA>* list, int p); //43eaa0


extern int32_t g_kbShiftState; //660f44
extern int32_t g_kbControlState; //660f40
extern int32_t g_kbMenuState; //660f48


extern GameDataRes g_GameDataRes; //6d0668
extern PlayersList* g_PlayersList; //6cdb24
extern Server* g_Server;   //642c2c

int32_t __cdecl Random0N(int32_t n); // returns [0, n]
int32_t __cdecl Random1N(int32_t n); // returns [1, n]

// sub_5499A6: Create a random item (arg1 = level, arg2 = max_gold range).
Item* __cdecl sub_5499A6(int32_t level, int32_t max_gold);

double __cdecl sub_5421FD();                                        // 5421FD - returns rand()/RAND_MAX in [0,1)
Item* __cdecl sub_549372(CString category, int32_t min_price, int32_t max_price); // 549372 - create item of given category in price range
// 5498B8 - pick a random (shape, material, item_data_id) combo from an equip table, skipping empty shape/material slots
void __cdecl sub_5498B8(CArray<WorldEquip>* table, int32_t* shape_id, int32_t* material_id, int32_t* item_data_id, int32_t min_id);


struct ScenarioLocation
{
	int32_t kind;
	int32_t id;
	CRect rect;

	int GetId() { return id; } //475150
	CRect& GetRect() { return rect; } //475170
};

extern "C" int32_t(__stdcall* ScenarioGetVar)(int32_t); //665a04
extern "C" void(__stdcall* ScenarioSetVar)(int32_t, int32_t); //665a00
extern "C" void(__stdcall* ScenarioTalkTo)(uint32_t); //665bd4
extern "C" void(__stdcall* ScenarioEnterLocation)(ScenarioLocation* loc); //6659fc
extern "C" int32_t(__stdcall* ScenarioLeaveLocation)(int32_t*); //6658c8
extern "C" void(__stdcall* ScenarioEnterShop)(); //665b0c
extern "C" void(__stdcall* ScenarioLeaveShop)(); //665b08
extern "C" void(__stdcall* ScenarioEnterInn)(int32_t*, int32_t*); //665be8
extern "C" void(__stdcall* ScenarioLeaveInn)(); //6658f0
extern "C" void(__stdcall* ScenarioNewGame)(); //6658cc
extern "C" void (__stdcall* ScenarioSave)(CFile*); //6658d0
extern "C" void (__stdcall* ScenarioLoad)(CFile*); //665bd8
extern "C" CList<ScenarioLocation*>* (__stdcall* ScenarioGetAvailableLocations)(); //6658c4
extern "C" AssortGenParams* (__stdcall* ScenarioGetShopAssortment)(); //665bdc
extern "C" int32_t (__stdcall* ScenarioIsTownAvailable)(int32_t); //665cf4
extern "C" int32_t (__stdcall* ScenarioIsMissionAvailable)(int32_t); //6659f8
extern "C" ScenarioLocation* (__stdcall* ScenarioGetCurrentLocation)(); //6658f4
extern "C" CList<ScenarioLocation*>* (__stdcall* ScenarioGetAllLocations)(); //665cf0





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
extern uint32_t INT_00660f74; //660f74


extern uint32_t g_DWORD_00659d18; //659d18

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
extern TxtFile TxtFile_00660e88; //660e88

extern ServerConfig g_ServerConfig; //6D15A0

#endif
