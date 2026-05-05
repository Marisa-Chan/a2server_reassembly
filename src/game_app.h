#ifndef GAME_APP_H
#define GAME_APP_H

#include "asm_mfc.h"
#include "net.h"
#include "table.h"
#include "txtfile.h"
#include "server.h"

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
};
ASSERT_SIZE(GameSettings, 0x3c);

extern GameSettings g_settings; //660ec0




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


extern "C" void __cdecl sub_43A857(const char* source); // 43A857

/* in asm */
void LogMessage(CString message); //43AA23

void FUN_00475e7a(); 

int32_t ParseConfig(const char* fname); //4f7188

int32_t InitVideo(); //453fcc
int32_t SetVideoMode(); //4546c6
void ParseTunes(); //476b21
void LoadFonts();  //460c37
void LoadSettings(); //441a03
void PrecomputeExperienceLevelsTable(); //53069e
void SetMessageColors(uint32_t); //476de7

int FUN_00497490(); //497490

void SrandInit(); //5421e9



extern int32_t g_kbShiftState; //660f44
extern int32_t g_kbControlState; //660f40
extern int32_t g_kbMenuState; //660f48


extern GameDataRes g_GameDataRes; //6d0668
extern PlayersList* g_PlayersList; //6cdb24
extern Server* g_Server;   //642c2c

extern UnitList* dword_6B37C4;   // Pool of server units for summoning/reuse (0x6B37C4)
extern "C" UnitList* dword_6CDB3C;  // Global server unit list (0x6CDB3C) - used for AddTailAllocId

extern "C" int32_t __cdecl Random0N(int32_t n); // returns [0, n]
extern "C" int32_t __cdecl Random1N(int32_t n); // returns [1, n]

// sub_5499A6: Create a random item (arg1 = level, arg2 = max_gold range).
extern "C" Item* __cdecl sub_5499A6(int level, int max_gold);

extern "C" int32_t(__cdecl* ScenarioGetVar)(int32_t); //665a04
extern "C" void(__cdecl* ScenarioSetVar)(int32_t, int32_t); //665a00
extern "C" FARPROC ScenarioTalkTo; //665bd4
extern "C" FARPROC ScenarioEnterLocation; //6659fc
extern "C" FARPROC ScenarioLeaveLocation; //6658c8
extern "C" FARPROC ScenarioEnterShop; //665b0c
extern "C" FARPROC ScenarioLeaveShop; //665b08
extern "C" FARPROC ScenarioEnterInn; //665be8
extern "C" FARPROC ScenarioLeaveInn; //6658f0
extern "C" FARPROC ScenarioNewGame; //6658cc
extern "C" void (__cdecl* ScenarioSave)(CFile*); //6658d0
extern "C" FARPROC ScenarioLoad; //665bd8
extern "C" FARPROC ScenarioGetAvailableLocations; //6658c4
extern "C" FARPROC ScenarioGetShopAssortment; //665bdc
extern "C" FARPROC ScenarioIsTownAvailable; //665cf4
extern "C" FARPROC ScenarioIsMissionAvailable; //6659f8
extern "C" FARPROC ScenarioGetCurrentLocation; //6658f4
extern "C" FARPROC ScenarioGetAllLocations; //665cf0





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
extern uint32_t g_EnableMusic; //660e10
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

extern ServerConfig g_ServerConfig; //6D15A0

#endif
