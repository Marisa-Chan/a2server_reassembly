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


extern UnitList* g_UnitPool;   // Pool of server units for summoning/reuse (0x6B37C4)
extern UnitList* g_ServerUnits;  // Global server unit list (0x6CDB3C) - used for AddTailAllocId

struct PasswordManager {
    CStringArray content;
    CString fname;
    CTime date;

    void OpenFile(const char* name); // 4f6bf9 (client: 4f7d8c)
    void Load(); // 4f6c92 (client: 4f7e25)

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

    int CheckActivationKey(const char* str); // 5a3498 (client: 571608) - CheckActivationKey

    CowardActivation(); // 5a335c (client: 5714cc)
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
    void Deinit(); // 4fab1d (client: none)
    void FreeLists(); // 4faeb4 (client: 4f905a)

    // Make spell effect.
    void MakeAreaSpellEffect(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y, uint32_t spell_id, uint32_t power); // 4fbb24 (client: 4f9629)
    void MakeTargetSpellEffect(uint8_t a0, uint8_t a1, Unit* unit, uint32_t a3, uint32_t a4); // 4fbae3 (client: 4f95ec)

    void UpdateMapObjects(); // 4fb4aa (client: 4f9302)
    void TransferUnitOwnership(Unit* unit, Player* new_owner); // 4fb4ca (client: 4f9322)
    void ProcessTick(); // 4fb273 (client: 4f9289)

    Unit* AddDummyUnitWithSpell(int32_t xpos, int32_t ypos, int32_t spellID, int16_t SpellDmg); // 4fb8cd (client: 4f9466)

    SrvStru1(); // 4fa89c (client: 4f91a3)
    ~SrvStru1(); // 4fa981 (client: 4f9000)
};
ASSERT_SIZE(SrvStru1, 0x48);

struct SackList {
    CList<Sack*> list;

public:
    void ClearSentToMask(Player* player); // 554b03 (client: 534ac8) - Clear unit vision mask bits for player across this sack list
    void DropInventoryAt(TokenPos* pos, Inventory* inventory, int money, int is_main_player_unit); // 554927 (client: 534a18)
    int32_t DumpInventoryAt(TokenPos* pos, Inventory* inventory, int money, int force_save); // 554460 (client: 534829)
    int32_t AddSack(Sack* sack); // 55496e (client: 534a5f) - Add sack to list and map
    void Serialize(CArchive& ar); // 55bc58 (client: 5372d4) - Serialize

    void UpdateSackVisibility(); // 5540ed (client: 534615)

    ~SackList(); // 554bc3 (client: 534b1d)
};

class SpellEffectList {
public: // VTable at 0060f6dc.
    virtual void FixupPointers(); // 55c213 (client: 537309)

    void ProcessTick(); // 5585f2 (client: 535f9b)
    void Serialize(CArchive& ar); // 55bf31 (client: 5372ed) - Serialize

    SpellEffectList(); // 55837d (client: 535f0b)
    ~SpellEffectList(); // 5583ed (client: 535f2d)
public:
    CList<SpellEffect*> list;
};
ASSERT_SIZE(SpellEffectList, 0x20);

class Srv1 : public CObject { // Aka `A2Srv_1`.
public: // Virtual table at 0060ec18.
    virtual ~Srv1(); // 59b6f0 (client: 56a5a0)

public:
    void CreateBuildings(MapAlm* alm); // 59c56e (client: 56b41e) - CreateBuildings
    void SpawnRandomSacks(int count); // 59fc97 (client: 56eb47) - Spawn 'count' random sacks on the map; count=0 defaults to max(10, map_width/4).
    int LoadMap(CString mapName); // 59b7ea (client: 56a69a) - Load ALM map file
    void CreatePlayers(MapAlm* alm); // 59c3fb (client: 56b2ab)
    void CreateUnits(MapAlm* alm); // 59cd45 (client: 56bbf5)
    void LoadTriggers(MapAlm* alm, int flag); // 59d891 (client: 56c741)
    void LoadSacks(MapAlm* alm); // 59f1be (client: 56e06e)
    void SpawnAiGroupOutposts(MapAlm* alm); // 59c37a (client: 56b22a)
    void SpawnGroupOutpost(Group* group, MapAlm* alm); // 59bedb (client: 56ad8b)

    Unit* GetScenarioScriptUnit(CMap<int32_t, int32_t, Unit*, Unit*>& umap, uint32_t idx); // 59d3c0 (client: 56c270)
    Unit* GetScenarioScriptHero(uint32_t idx); // 59d664 (client: 56c514)
    Unit* GetScenarioScriptPCUnit(const char* name); // 59d4b8 (client: 56c368)
    Group* GetScenarioScriptGroup(CMap<int32_t, int32_t, Group*, Group*>& gmap, uint32_t idx); // 59d808 (client: 56c6b8)
    Player* GetScenarioScriptPlayer(CMap<int32_t, int32_t, Player*, Player*>& gmap, uint32_t idx); // 59d77f (client: 56c62f)

    int32_t GetMapCenterX() { return MapStuff_Instance->GetWidth() / 2; } // 59fc7c (client: 56eb2c)

public:
    CMapStringToOb gap_0x4;
    uint32_t is_scenario = 0;
    SrvStru1 *srv_stru = nullptr;
    CArray<uint16_t> drop_locations; // Drop locations.
};
ASSERT_OFFSET(Srv1, srv_stru, 0x24);
ASSERT_SIZE(Srv1, 0x3c);


class Player;

enum Difficulty : uint32_t {
    DIFFICULTY_EASY   = 1, // client halves monster hp
    DIFFICULTY_NORMAL = 2, // default; no scaling
    DIFFICULTY_HARD   = 3, // client adds +50 attack/defense, hp * 1.5
};

struct Server {
public:
    void ServerTic(); // 4fbb79 (client: 4f967a)
    void ProcessPlayers(); // 4f1e2a (client: 4f4b1f)
    void AnnounceHatGameStart(); // 4f465e (client: none)
    void ScanSavedCharacters(); // 4f9e55 (client: none)
    void SendArenaResults(); // 4fa5c4 (client: none)
    void ResetArenaRound(int end_reason); // 4fa78e (client: none)
    void KickAllPlayers(); // 4f8831 (client: none)
    void HatKeepaliveTick(); // 4f0ecf (client: none)
    int LoadGame(CString map_name); // 4f1471 (client: 4f44d5) - Load map / saved game
    int LoadSavedGame(const CString& filename); // 4edb83 (client: 4f2955) - Load saved game file
    void SetupArenaFileList(); // 4fa01f (client: none) - Arena mode file list setup

    Human* LoadCharacter(void* data, int32_t size, Player* player); // 4ef4e7 (client: 4f346b) - Load character from saved packet.
    Human* CreatePlayerCharacter(Player* player, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t main_skill, uint8_t character_class); // 500907 (client: 4fb265)
    void SaveCharacter(Humanoid* humanoid); // 4ee028 (client: 4f2c57)
    void EnsurePlayerGroup(Player* player); // 5013d4 (client: 4fb99d)
    void UpdateHatAnnounce(); // 4f4570 (client: 4f5e75)
    void ProcessHatConnections(); // 4f0d58 (client: none)
    void GetArenaFrags(CString* name, uint32_t* frags); // 4fa4bb (client: none)
    void AddArenaFrags(CString* name, int32_t flag); // 4fa348 (client: none)
    void HandleArenaPlayer(Player* player); // 4fa551 (client: none) - Arena mode player entry handler
    void SendWorldState(Player* player, int32_t enter_world); // 4ff937 (client: 4faab6) - Mission join: send full world state to connecting player

    void OnTeamsReady(); // 4f8f86 (client: none) - Called when all team-game players are ready
    void UpdateQuestRune(int team, int picked_up); // 4f8fbf (client: none) - Called when all team-game players are ready (sub-step)
    void SpawnRandomSack(); // 4f8b74 (client: none) - Spawn random sack on map
    void SaveSack(Sack* sack); // 4f9b9e (client: none) - Save sack to file
    void PrepareSackPickup(class Sack* sack); // 4f9ad3 (client: none) - Pre-process a sack before pickup
    Human* SummonUnit(CString* unit_name, class Unit* origin, int is_hero); // 509879 (client: 50157b) - Summon a unit; returns the new unit (or nullptr on failure)
    void ProcessHatPackets(); // 509042 (client: none)
    void UpdateMissionResults(); // 5096e4 (client: 5014cc)
    void Tick(); // 5090a7 (client: 5010c9)
    void MissionResultsTick(); // 50979a (client: 501511)
    void ProcessHatPacket(Packet* pkt); // 5088df (client: none)
    void LoadSavedSacks(); // 4f950f (client: none)
    void UpdateMissionResult(Player* player); // 5090fa (client: 501114)
    void CheatCommand(Player* player, CString cheat_string); // 502d0b (client: 4fcab0)

    int ConnectToHat(); // 4f0bef (client: none) - Connect to hat server
    int Start(int mode); // 4f06f5 (client: 4f4116)

    void ProcessAvailPackets(); // 509010 (client: 50106e)
    void PumpNetwork(); // 50907e (client: 5010a0)

    void SetDifficulty(int32_t val) { this->difficulty = (Difficulty)val; }; // 497470 (client: 49aa90)

    void DespawnScenarioUnits(int32_t var300, CDWordArray& arr); // 501b9e (client: 4fbe72)
        
    Server(); // 4ece8b (client: 4f233b)
    ~Server(); // 4f2c26 (client: 4f5180)
public:
    int32_t tick16; // This value seems to be advanced every 16 ticks
    int32_t tick;
    CowardActivation coward_activation;
    int32_t send_full_state;
    int32_t multiplayer; // Is server flag?
    CString base_dir;
    SrvStru1* srv_stru1;
    uint8_t gap_0x80[8];
    HANDLE thread_handle;
    int32_t gap_0x8c;
    CString current_map_name;
    uint32_t has_world_state;
    Srv1 srv1;
    uint32_t map_code;
    Difficulty difficulty; // 1..3 (saved as raw int in games)
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
    void AssignPlayerSlot(Player* player, int32_t slot_data); // 4ff439 (client: none)

    // ProcessClientPacket helpers
    Unit* FindPlayerUnit(uint16_t player_ref, uint16_t unit_id); // 502ad1 (client: 4fc761)
    Shop* FindShopAt(TokenPos* pos); // 502c50 (client: 4fc9f5)
    #ifdef A2SERVER_PATCH
    Shop* FindShopFor(Unit* unit);
    #endif
    Inn* FindInnAt(TokenPos* pos); // 502cb7 (client: 4fca5c) - Inn lookup by TokenPos
    #ifdef A2SERVER_PATCH
    Inn* FindInnFor(Unit* unit);
    #endif
    Player* FindPlayerById(uint16_t player_id); // 502b4a (client: 4fc8f1)
    void SaveGame(CString* str); // 4ed2dc (client: 4f25d6)
    void SendMapInfo(Player* player); // 4ff878 (client: 4fa9f7)
    void ProcessClientPacket(Packet* packet); // 504a96 (client: 4fdea5)

    void SignalGameStart(int32_t start_kind); // 4f94c0 (client: none)
    int  JoinPlayer(uint32_t pkt_word0, uint32_t pkt_word1, CString name, CString login,
                    void* Block, int32_t block_size, int32_t team_id); // 4fc644 (client: none)

    void SerializeGame(CArchive& ar); // 4f3188 (client: 4f54f6) - deserialise saved game state from archive

    void Allods2_JoinPlayer(int32_t id, int32_t, CString name, NetStru2* client, uint32_t); // server: none (client: 4f9a78) - Allods2.exe Join Player
    void JoinErrorSend(NetStru2* client, int32_t error_code, const CString& str); // 4fc4f3 (client: 4f992b)
    Player* Allods2_CreatePlayer(CString name); // 500792 (client: 4fb0f3) - not used in server, but used in Allods2
};
ASSERT_OFFSET(Server, srv_stru1, 0x7c);
ASSERT_OFFSET(Server, script_settings, 0x170);
ASSERT_SIZE(Server, 0x258);


enum GameType : uint32_t {
    GAME_TYPE_COOPERATIVE = 0, // no player-vs-player damage
    GAME_TYPE_DEATHMATCH  = 1, // everyone is an enemy
    GAME_TYPE_TEAMPLAY    = 2, // teams (parser value name: teamplay, internally: softcore)
    GAME_TYPE_ARENA       = 3, // arena with time/score limits
};

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
//#ifndef A2CLIENT // TODO: client doesn't have these fields.
    uint32_t login_timeout;
    uint32_t reconnect_delay;
    GameType gameType;     // 0: co-op, 1: deathmatch, 2: softcore, 3: arena. 6D1648.
    int32_t frag_limit;
    uint32_t flag_score;
    uint32_t map_range_check; // map-level range check enabled flag
    uint32_t shutdown_delay;
    uint32_t always_load_sacks;
    uint32_t arena_time_limit;
    int32_t treasure_probability;
//#endif
    ServerConfig(); // 4f6fc9 (client: 4f8107)
};

ASSERT_SIZE(ServerConfig, 0xc8);




int32_t StatLevelPoints(int32_t level); // 5008ca (client: 4fb228)

extern Server* g_Server;   //642c2c
extern int32_t g_IsServer; // 1 for server mode, 0 for client code. 665D00.
extern int32_t g_ShutdownIn; // Server shutdown (in milliseconds?). 0x7FFFFFFF means no shutdown scheduled. 63623c.

#endif
