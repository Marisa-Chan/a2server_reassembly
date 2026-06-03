#ifndef SERVER_H
#define SERVER_H

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"
#include "map_stuff.h"
#include "token.h"

class Building;
class BuildingsList;
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

struct CowardActivation {
    char key[100];
    BOOL enabled;

    int sub_5A3498(const char* str); // sub_5A3498

    CowardActivation(); //5a335c
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
    void Deinit(); //4fab1d

    // Make spell effect.
    void sub_4FBB24(uint8_t from_x, uint8_t from_y, uint8_t to_x, uint8_t to_y, uint32_t spell_id, uint32_t power);
    void sub_4FBAE3(uint8_t a0, uint8_t a1, Unit* unit, uint32_t a3, uint32_t a4);

    void sub_4FB4AA();

    Unit* AddDummyUnitWithSpell(int32_t xpos, int32_t ypos, int32_t spellID, int16_t SpellDmg); //4fb8cd

    SrvStru1(); //4fa89c
    ~SrvStru1(); //4fa981
};
ASSERT_SIZE(SrvStru1, 0x48);

struct SackList {
    CList<Sack*> list;

public:
    void sub_554B03(Player* player); // Clear unit vision mask bits for player across this sack list
    void sub_554927(TokenPos* pos, Inventory* inventory, int money, int is_main_player_unit);
    int32_t sub_554460(TokenPos* pos, Inventory* inventory, int money, int param_5);
    int32_t sub_55496E(Sack* sack); // Add sack to list and map

    void FUN_005540ed(); //5540ed

    ~SackList(); //554bc3
};

class SpellEffectList {
public: // VTable at 0060f6dc.
    virtual void VMethod1(); // 55c213

    SpellEffectList(); // 55837D
    ~SpellEffectList(); //5583ed
public:
    CList<SpellEffect*> list;
};
ASSERT_SIZE(SpellEffectList, 0x20);

class Srv1 : public CObject { // Aka `A2Srv_1`.
public: // Virtual table at 0060ec18.
    virtual ~Srv1();

public:
    void sub_59C56E(MapAlm* alm); // CreateBuildings
    void sub_59FC97(int count); // Spawn 'count' random sacks on the map; count=0 defaults to max(10, map_width/4).
    int sub_59B7EA(CString mapName); // Load ALM map file
    void sub_59C3FB(MapAlm* alm);
    void sub_59CD45(MapAlm* alm);
    void sub_59D891(MapAlm* alm, int flag);
    void sub_59F1BE(MapAlm* alm);
    void sub_59C37A(MapAlm* alm);

    int32_t GetMapCenterX() { return MapStuff_Instance->GetWidth() / 2; } //59fc7c

public:
    CMapStringToOb field1_0x4;
    uint32_t field2_0x20 = 0;
    SrvStru1 *srv_stru = nullptr;
    CArray<uint16_t> field4_0x28;
};
ASSERT_OFFSET(Srv1, srv_stru, 0x24);
ASSERT_SIZE(Srv1, 0x3c);


class Player;

Human* _stdcall sub_4EF4E7(void* block, int arg14, Player* player); // Load character.

struct Server {
public:
    void ServerTic();
    void sub_4F1E2A();
    void sub_4FA5C4();
    void sub_4FA78E(int arg);
    void sub_4F8831();
    void sub_4F0ECF();
    int sub_4F1471(CString param_1); // Load map / saved game
    int sub_4EDB83(const CString& filename); // Load saved game file
    void sub_4FA01F(); // Arena mode file list setup

    // Sub-functions called by sub_4FC644
    Human* sub_500907(Player* player, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t main_skill, uint8_t character_class);
    void sub_4EE028(Humanoid* humanoid);
    void sub_5013D4(Player* player);
    void sub_4F4570();
    void sub_4F0D58();
    void sub_4FA4BB(CString* name, uint32_t* frags);
    void sub_4FA348(CString* name, int flag);
    void sub_4FA551(Player* player); // Arena mode player entry handler
    void sub_4FF937(Player* player, int arg4); // Mission join: send full world state to connecting player

    void sub_4F8F86(); // Called when all team-game players are ready
    void sub_4F8FBF(int arg1, int arg2); // Called when all team-game players are ready (sub-step)
    void sub_4F8B74(); // Spawn random sack on map
    void sub_4F9B9E(Sack* sack); // Save sack to file
    void sub_4F9AD3(class Sack* sack); // Pre-process a sack before pickup
    Human* sub_509879(CString* unit_name, class Unit* origin, int is_hero); // Summon a unit; returns the new unit (or nullptr on failure)
    void sub_5090A7();
    void sub_50979A();
    void sub_5346AC_player(class Player* target); // Kill all of a player's units (unused alias; see Player::sub_5346AC)
    void CheatCommand(Player* player, CString param_2);

    int sub_4F0BEF(); //4f0bef - Connect to hat server
    int Start(int mode); //4f06f5

    void ProcessAvailPackets(); //509010
    void FUN_0050907e(); //50907e

    void FUN_00497470(int32_t val) { field22_0xd8 = val; }; //497470

    void FUN_00501b9e(int32_t var300, CDWordArray& arr); //501b9e
        
    Server(); //4ece8b
    ~Server(); //4f2c26
public:
    int32_t tick16; // This value seems to be advanced every 16 ticks
    int32_t tick;
    CowardActivation coward_activation;
    int32_t field3_0x70;
    int32_t field4_0x74; // Fog of war flag?
    CString field5_0x78;
    SrvStru1* srv_stru1;
    uint8_t gap_0x80[8];
    HANDLE field15_0x88;
    int32_t field16_0x8c;
    CString current_map_name;
    uint32_t field18_0x94;
    Srv1 field19_0x98;
    uint32_t field21_0xd4;
    uint32_t field22_0xd8; //game type? or dificulty?
    CMapPtrToPtr field23_0xdc;
    std::array<Spell*, 30> spells;
    ScriptSettings* script_settings;
    uint32_t field26_0x174;
    uint32_t field27_0x178;
    uint32_t field28_0x17c;
    uint32_t field29_0x180;
    uint32_t field30_0x184;
    uint32_t field31_0x188;
    uint32_t field32_0x18c;
    uint32_t field33_0x190;
    uint32_t field34_0x194;
    uint32_t field35_0x198;
    uint32_t field36_0x19c;
    uint32_t field37_0x1a0;
    uint32_t field38_0x1a4;
    uint32_t field39_0x1a8;
    uint32_t field40_0x1ac;
    uint32_t field41_0x1b0;
    int32_t field42_0x1b4;
    int32_t field43_0x1b8;
    int32_t field44_0x1bc;
    int32_t tic16;
    int32_t field46_0x1c4;
    CString current_map_title;
    int32_t MapLevel;
    uint32_t field49_0x1d0;
    uint32_t field50_0x1d4;
    uint32_t field51_0x1d8;
    CStringArray FileList;
    uint32_t field53_0x1f0;
    uint32_t field54_0x1f4;
    uint32_t field55_0x1f8;
    uint32_t field56_0x1fc;
    std::array<int32_t, 2> ctf_score; // Capture-the-flag mode: team scores.
    uint32_t field59_0x208;
    std::array<int32_t, 2> ctf_carrying; // Capture-the-flag mode: 1 if the rune is being carried.
    std::array<int32_t, 2> ctf_dropped;  // Capture-the-flag mode: 1 if the rune is dropped on the ground.
    uint32_t field64_0x21c;
    uint32_t field65_0x220;
    uint32_t field66_0x224;
    CStringArray field67_0x228;
    CArray<uint32_t> field68_0x23c;
    int32_t map_elapsed_time;
    int32_t map_elapsed_time2;
    static uint16_t somewords[32][32]; //6cfdf0 used in client!

public:
    void FUN_004ff439(Player* player, int32_t arg4);

    // sub_504a96 helpers
    Unit* sub_502AD1(uint16_t player_ref, uint16_t unit_id);
    Shop* sub_502C50(TokenPos* pos);
    Inn* sub_502CB7(TokenPos* pos);   // Inn lookup by TokenPos
    Player* sub_502B4A(uint16_t player_id);
    void sub_4ED2DC(CString* str);
    void sub_4FF878(Player* player);
    void sub_504a96(Packet* packet);

    void FUN_004f94c0(int32_t arg);
    int  sub_4FC644(uint32_t pkt_word0, uint32_t pkt_word1, CString name, CString login,
                    void* Block, int32_t block_size, int32_t team_id);


    void Allods2_JoinPlayer(int32_t id, int32_t, CString name, NetStru2* client, uint32_t); //Allods2.exe 4f9a78    Join Player
    void JoinErrorSend(NetStru2* client, int32_t arg, const CString& str); //4fc4f3
    Player* Allods2_CreatePlayer(CString name); //500792 not used in server, but used in Allods2 (4fb0f3)
};
ASSERT_OFFSET(Server, srv_stru1, 0x7c);
ASSERT_OFFSET(Server, script_settings, 0x170);
ASSERT_SIZE(Server, 0x258);


struct ServerConfig
{
    uint32_t repop_delay;
    uint32_t protocol; // Legacy protocol setting? Unset when reading the config. Only relevant values are 0 and 1. Default: 2.
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

    ServerConfig();
};

ASSERT_SIZE(ServerConfig, 0xc8);





extern int32_t g_IsServer; // 1 for server mode, 0 for client code. 665D00.
extern int32_t g_ShutdownIn; // Server shutdown (in milliseconds?). 0x7FFFFFFF means no shutdown scheduled. 63623c.

#endif
