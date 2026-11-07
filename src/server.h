#ifndef SERVER_H
#define SERVER_H

#include <array>
#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>
#include "map_stuff.h"
#include "token.h"

class Building;
class BuildingsList;
class Group;
class Human;
class Humanoid;
class Inn;
struct Inventory;
struct MapAlm;
struct NetStru2;
class Packet;
class Player;
class Sack;
class SackList;
struct ScriptSettings;
class Shop;
class Spell;
class SpellEffect;
class SpellEffectList;
class TokenPos;
class Unit;
class UnitList;
class VirtualCaster;


extern UnitList* dword_6B37C4;   // Pool of server units for summoning/reuse (0x6B37C4)
extern UnitList* dword_6CDB3C;  // Global server unit list (0x6CDB3C) - used for AddTailAllocId

struct PasswordManager {
    CStringArray content;
    CString fname;
    CTime date;

    void OpenFile(const char* name); //4f6bf9 (client: 4f7d8c)
    void Load(); //4f6c92 (client: 4f7e25)

    static PasswordManager manager; //6a8b98
};
// binary 0x1c: VC6 CTime was a 4-byte time_t; MFC140 CTime is 0x14 and
// 8-aligned, so the struct rounds up (CStringArray + CString are 4 each)
// binary 0x1c: identical except VC6 CTime wrapped a 4-byte time_t and
// MFC140 uses an 8-byte __time64_t
ASSERT_SIZE(PasswordManager, sizeof(CStringArray) + sizeof(CString) + sizeof(CTime));

struct CowardActivation {
    char key[100];
    BOOL enabled;

    int CheckActivationKey(const char* str); // CheckActivationKey (client: 571608)

    CowardActivation(); //5a335c (client: 5714cc)
};
ASSERT_OFFSET(CowardActivation, enabled, 0x64);
ASSERT_SIZE(CowardActivation, 0x68);

struct SrvStru1 {
    BuildingsList* building_list;
    SpellEffectList* effects_list;
    SackList* sack_list;
    UnitList* units_list;
    CList<VirtualCaster*> virtual_casters_list;
    CList<Unit*> some_unit_list;

    static SrvStru1 Instance; //6cf390

public:
    void Deinit(); //4fab1d (client: 482e0c)
    void FreeLists(); // 4faeb4 (client: 4f905a)

    // Make spell effect.
    void MakeAreaSpellEffect(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y, uint32_t spell_id, uint32_t power);  // client: 4f9629
    void MakeTargetSpellEffect(uint8_t a0, uint8_t a1, Unit* unit, uint32_t a3, uint32_t a4);  // client: 4f95ec

    void UpdateMapObjects();  // client: 4f9302
    void ProcessTick(); // 4fb273 (client: 4f9289)

    Unit* AddDummyUnitWithSpell(int32_t xpos, int32_t ypos, int32_t spellID, int16_t SpellDmg); //4fb8cd (client: 4f9466)

    SrvStru1(); //4fa89c (client: 4f91a3)
    ~SrvStru1(); //4fa981 (client: 4f9000)
};
ASSERT_SIZE(SrvStru1, 0x48);

struct SackList {
    CList<Sack*> list;

public:
    void ClearSentToMask(Player* player); // Clear unit vision mask bits for player across this sack list (client: 534ac8)
    void DropInventoryAt(TokenPos* pos, Inventory* inventory, int money, int is_main_player_unit);  // client: 534a18
    int32_t DumpInventoryAt(TokenPos* pos, Inventory* inventory, int money, int force_save);  // client: 534829
    int32_t AddSack(Sack* sack); // Add sack to list and map (client: 534a5f)
    void Serialize(CArchive& ar);  // Serialize (client: 5372d4)

    void UpdateSackVisibility(); //5540ed (client: 534615)

    ~SackList(); //554bc3
};

class SpellEffectList {
public: // VTable at 0060f6dc.
    virtual void FixupPointers(); // 55c213 (client: 537309)

    void ProcessTick(); //5585f2 (client: 535f9b)
    void Serialize(CArchive& ar); // Serialize (client: 5372ed)

    SpellEffectList(); // 55837D (client: 535f0b)
    ~SpellEffectList(); //5583ed
public:
    CList<SpellEffect*> list;
};
ASSERT_SIZE(SpellEffectList, 0x20);

class Srv1 : public CObject { // Aka `A2Srv_1`.
public: // Virtual table at 0060ec18.
    virtual ~Srv1();

public:
    void CreateBuildings(MapAlm* alm); // CreateBuildings (client: 56b41e)
    void SpawnRandomSacks(int count); // Spawn 'count' random sacks on the map; count=0 defaults to max(10, map_width/4). (client: 56eb47)
    int LoadMap(CString mapName); // Load ALM map file (client: 56a69a)
    void CreatePlayers(MapAlm* alm);  // client: 56b2ab
    void CreateUnits(MapAlm* alm);  // client: 56bbf5
    void LoadTriggers(MapAlm* alm, int flag);  // client: 56c741
    void LoadSacks(MapAlm* alm);  // client: 56e06e
    void SpawnAiGroupOutposts(MapAlm* alm);  // client: 56b22a
    void SpawnGroupOutpost(Group* group, MapAlm* alm);  // client: 56ad8b

    Unit* GetScenarioScriptUnit(CMap<int32_t, int32_t, Unit*, Unit*>& umap, uint32_t idx); //59d3c0 (client: 56c270)
    Unit* GetScenarioScriptHero(uint32_t idx); //59d664 (client: 56c514)
    Unit* GetScenarioScriptPCUnit(const char* name); //59d4b8 (client: 56c368)
    Group* GetScenarioScriptGroup(CMap<int32_t, int32_t, Group*, Group*>& gmap, uint32_t idx); //59d808 (client: 56c6b8)
    Player* GetScenarioScriptPlayer(CMap<int32_t, int32_t, Player*, Player*>& gmap, uint32_t idx); //59d77f (client: 56c62f)

    int32_t GetMapCenterX() { return MapStuff_Instance->GetWidth() / 2; } //59fc7c (client: 56eb2c)

public:
    CMapStringToOb gap_0x4;
    uint32_t is_scenario = 0;
    SrvStru1 *srv_stru = nullptr;
    CArray<uint16_t> drop_locations; // Drop locations.
};
ASSERT_OFFSET(Srv1, srv_stru, 0x24);
ASSERT_SIZE(Srv1, 0x3c);


class Player;

struct Server {
public:
    void ServerTic();  // client: 4f967a
    void ProcessPlayers();  // client: 4f4b1f
    void AnnounceHatGameStart(); // 4f465e
    void ScanSavedCharacters(); // 4f9e55
    void SendArenaResults();
    void ResetArenaRound(int end_reason);
    void KickAllPlayers();
    void HatKeepaliveTick();
    int LoadGame(CString map_name); // Load map / saved game (client: 4f44d5)
    int LoadSavedGame(const CString& filename); // Load saved game file (client: 4f2955)
    void SetupArenaFileList(); // Arena mode file list setup

    Human* LoadCharacter(void* data, int32_t size, Player* player); // 4ef4e7 (client: 4f346b): Load character from saved packet.
    Human* CreatePlayerCharacter(Player* player, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t main_skill, uint8_t character_class);  // client: 4fb265
    void SaveCharacter(Humanoid* humanoid);
    void EnsurePlayerGroup(Player* player);
    void UpdateHatAnnounce();  // client: 4f5e75
    void ProcessHatConnections();
    void GetArenaFrags(CString* name, uint32_t* frags);
    void AddArenaFrags(CString* name, int32_t flag); // TODO: that's likely `CString&`
    void HandleArenaPlayer(Player* player); // Arena mode player entry handler
    void SendWorldState(Player* player, int32_t enter_world); // Mission join: send full world state to connecting player (client: 4faab6)

    void OnTeamsReady(); // Called when all team-game players are ready
    void UpdateQuestRune(int team, int picked_up); // Called when all team-game players are ready (sub-step)
    void SpawnRandomSack(); // Spawn random sack on map
    void SaveSack(Sack* sack); // 4F9B9E Save sack to file
    void PrepareSackPickup(class Sack* sack); // Pre-process a sack before pickup
    Human* SummonUnit(CString* unit_name, class Unit* origin, int is_hero); // Summon a unit; returns the new unit (or nullptr on failure) (client: 50157b)
    void ProcessHatPackets(); // 509042
    void UpdateMissionResults(); // 5096E4
    void Tick();  // client: 5010c9
    void MissionResultsTick();  // client: 501511
    void ProcessHatPacket(Packet* pkt); // 5088DF
    void LoadSavedSacks(); // 4F950F
    void UpdateMissionResult(Player* player); // 5090FA
    void CheatCommand(Player* player, CString cheat_string);  // client: 4fcab0

    int ConnectToHat(); //4f0bef - Connect to hat server
    int Start(int mode); //4f06f5 (client: 4f4116)

    void ProcessAvailPackets(); //509010 (client: 50106e)
    void PumpNetwork(); //50907e (client: 5010a0)

    void SetDifficulty(int32_t val) { this->difficulty = val; }; //497470

    void DespawnScenarioUnits(int32_t var300, CDWordArray& arr); //501b9e (client: 4fbe72)
        
    Server(); //4ece8b (client: 4f233b)
    ~Server(); //4f2c26
public:
    int32_t tick16; // This value seems to be advanced every 16 ticks
    int32_t tick;
    CowardActivation coward_activation;
    int32_t send_full_state;
    int32_t fog_of_war; // Is server flag?
    CString base_dir;
    SrvStru1* srv_stru1;
    uint8_t gap_0x80[8];
    HANDLE thread_handle;
    int32_t gap_0x8c;
    CString current_map_name;
    uint32_t has_world_state;
    Srv1 srv1;
    uint32_t map_code;
    uint32_t difficulty; //game type? or dificulty?
    CMapPtrToPtr saved_ptr_map;
    std::array<Spell*, 30> spells;
    ScriptSettings* script_settings;
    uint32_t skip_ai_units;
    uint32_t is_singleplayer;
    uint32_t saved_0x17c;
    uint32_t saved_0x180;
    uint32_t saved_0x184;
    uint32_t saved_0x188;
    uint32_t saved_0x18c;
    uint32_t saved_0x190;
    uint32_t saved_0x194;
    uint32_t saved_0x198;
    uint32_t saved_0x19c;
    uint32_t saved_0x1a0;
    uint32_t use_default_shop;
    uint32_t save_on_server;
    uint32_t map_loaded;
    uint32_t server_screen_shown;
    int32_t last_window_ms;
    int32_t window_ms;
    int32_t total_ms;
    int32_t tic16;
    int32_t max_window_ms;
    CString current_map_title;
    int32_t MapLevel;
    uint32_t gap_0x1d0;
    uint32_t hat_logged_in;
    uint32_t hat_disconnect_tick;
    CStringArray FileList;
    uint32_t hat_upload_tick;
    uint32_t hat_id;
    uint32_t hat_ping_tick;
    uint32_t hat_uploads_pending;
    std::array<int32_t, 2> ctf_score; // Capture-the-flag mode: team scores.
    uint32_t game_started;
    std::array<int32_t, 2> ctf_carrying; // Capture-the-flag mode: 1 if the rune is being carried.
    std::array<int32_t, 2> ctf_dropped;  // Capture-the-flag mode: 1 if the rune is dropped on the ground.
    uint32_t hat_announce_tick;
    uint32_t hat_announce_player_index;
    uint32_t sacks_loaded;
    CStringArray arena_names;
    CArray<uint32_t> arena_scores;
    int32_t map_elapsed_time;
    int32_t map_elapsed_time2;
    static uint16_t somewords[32][32]; //6cfdf0 used in client!

public:
    void AssignPlayerSlot(Player* player, int32_t slot_data);

    // ProcessClientPacket helpers
    Unit* FindPlayerUnit(uint16_t player_ref, uint16_t unit_id);
    Shop* FindShopAt(TokenPos* pos);
    Inn* FindInnAt(TokenPos* pos);   // Inn lookup by TokenPos (client: 4fca5c)
    Player* FindPlayerById(uint16_t player_id);  // client: 4fc8f1
    void SaveGame(CString* str);  // client: 4f25d6
    void SendMapInfo(Player* player);  // client: 4fa9f7
    void ProcessClientPacket(Packet* packet); // 504a96 (client: 4fdea5)

    void SignalGameStart(int32_t start_kind);
    int  JoinPlayer(uint32_t pkt_word0, uint32_t pkt_word1, CString name, CString login,
                    void* Block, int32_t block_size, int32_t team_id);

    void SerializeGame(CArchive& ar); // 4F3188 (client: 4f54f6) - deserialise saved game state from archive

    void Allods2_JoinPlayer(int32_t id, int32_t, CString name, NetStru2* client, uint32_t); //Allods2.exe 4f9a78    Join Player
    void JoinErrorSend(NetStru2* client, int32_t error_code, const CString& str); //4fc4f3 (client: 4f992b)
    Player* Allods2_CreatePlayer(CString name); //500792 (client: 4fb0f3) not used in server, but used in Allods2 (4fb0f3)
};
ASSERT_OFFSET(Server, srv_stru1, 0x7c);
ASSERT_OFFSET(Server, script_settings, 0x170);
ASSERT_SIZE(Server, 0x258);


struct ServerConfig {
    uint32_t repop_delay;
    int32_t protocol; // Legacy protocol setting? Unset when reading the config. Only relevant values are 0 and 1. Default: 2.
    uint32_t game_speed;
    CString  log_file;
    CString  ip_address;
    CString  ip_address2;
    CString  hat_address;
    CString  chr_base;
    CString  server_name;
    uint32_t server_id;
    CStringArray banned_ips;
    CStringArray banned_names;
    CStringArray map_names; // Array of map names. 6D15F0.
    CStringArray report_to_www;
    CDWordArray map_durations; // Array of map durations. 0x7FFFFFFF means no limit. 6D1618.
    int32_t chat_range; // Chat range for proximity chat.
    uint32_t shout_delay;
    uint32_t current_map_index; // Current map index in map array. 6D1634.
    uint32_t save_is_server;
    uint32_t max_players;
//#ifndef A2CLIENT
    uint32_t login_timeout;
    uint32_t reconnect_delay;
    uint32_t gameType;     // 0: co-op, 1: deathmatch, 2: softcore, 3: arena. 6D1648.
    int32_t frag_limit;
    uint32_t flag_score;
    uint32_t map_range_check; // map-level range check enabled flag
    uint32_t shutdown_delay;
    uint32_t always_load_sacks;
    uint32_t arena_time_limit;
    int32_t treasure_probability;
//#endif
    ServerConfig();
};

ASSERT_SIZE(ServerConfig, 0xc8);




int32_t StatLevelPoints(int32_t level); // 5008ca (client: 4fb228)

extern Server* g_Server;   //642c2c
extern int32_t g_IsServer; // 1 for server mode, 0 for client code. 665D00.
extern int32_t g_ShutdownIn; // Server shutdown (in milliseconds?). 0x7FFFFFFF means no shutdown scheduled. 63623c.

#endif
