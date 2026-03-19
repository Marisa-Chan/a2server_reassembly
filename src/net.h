#pragma once

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"

#pragma warning(push)
#pragma warning(disable: 4005) // dplay.h redefines INTERFACE without a guard
#include "dplay.h"
#pragma warning(pop)


class AreaEffect;
struct CLlDriver;
class Effect;
class NetStru1;
struct NetStru2;
struct NetStru3;
class Packet;
class Player;
class Token;
class Unit;
class QuestMap;


struct PackerTail {
    PackerTail* field_0x0;
    PackerTail* field_0x4;
    int32_t field_0x8;
    int32_t field_0xc;
    int32_t field_0x10;
    int32_t field_0x14;
};
ASSERT_SIZE(PackerTail, 0x18);

struct PackerDat {
    std::array<int32_t, 256> bitMap;
    std::array<int32_t, 256> bitLength;
    std::array<int32_t, 256> freq;
    PackerTail* tail;
};
ASSERT_OFFSET(PackerDat, freq, 0x800);
ASSERT_SIZE(PackerDat, 0xc04);

struct ConnStatInfo {
    uint32_t field1;
    uint32_t cur_bs;
    uint32_t total_bytes;
    uint32_t max_bs;
    uint32_t time;
};
ASSERT_SIZE(ConnStatInfo, 0x14);

class NetStru1 {
public: // VTable at 0060ecc0.
    virtual void OnClientConnect(NetStru2* client);
    virtual void OnClientDisconnect(NetStru2* client);

public:
    CLlDriver* driver;
    NetStru1* field_0x8;
    CList<int32_t> list;
    CList<NetStru2*> clients;
    CList<NetStru3*> free_net3;
    CriticalSection critical_section;
    CriticalSection critical_section2;
    PackerDat packer_dat1;
    PackerDat packer_dat2;
    uint32_t field_0x1898;
    uint32_t field_0x189c;
    uint32_t field_0x18a0;
    uint32_t field_0x18a4;
    uint32_t field_0x18a8;
    uint32_t field_0x18ac;
    uint32_t field_0x18b0;
    NetStru2* fields_0x18b4;
    CList<NetStru2*> list_0x18b8;
    CMap<int32_t, int32_t, ConnStatInfo, ConnStatInfo&> client_stat;
    uint8_t buf1[1024];
    uint8_t buf2[1024];

    static NetStru1 Inst;

public:
    // Transfers unit ownership to another player.
    // `this` is unused, but in ASM this function accepts a `NetStru1*` in ECX, so it's here.
    void FUN_004fb4ca(Unit* unit, Player* new_owner);

public:
    void FUN_005186cd(Packet* pkt);
    void FUN_0051ce86(uint32_t msg_type, uint32_t player_id, Player* recpt);
    void FUN_0051ceac(uint32_t id, Player* recpt);
    void FUN_0051c748(Player* recpt);
    void sub_51C61E(Token* token); // Remove unit/sack from network tracking
    void FUN_0051d49b(Player* recpt);
    void FUN_005188db();

    void FUN_0051cd89(const CString& name, Player* player);

    NetStru2* FUN_00518544(uint16_t player_id);
    void FUN_005170b6(NetStru2* arg1);

    void FUN_0051800f();
    void FUN_0051d6b4(uint16_t arg);
    void FUN_0051cefb(uint8_t param_1, int32_t param_2, int32_t param_3, Player* param_4);
    
    void sub_519221(Unit* unit, Player* player, uint32_t mask, int32_t param4, int32_t param5, int32_t param6);

    // sub_51A0EF / sub_51A6D5 / sub_51BDA4 are still in asm but called from sub_519221
    void sub_51A0EF(Unit* unit, Player* player, int32_t flags);
    void sub_51A6D5(Unit* unit, Player* player, int32_t param5, int32_t param6);
    void sub_51BDA4(Effect* effect, Unit* unit, int32_t arg);

    void sub_51D837(int32_t param1, Player* player); // Send server state / player list to player
    void sub_51C8B1(Player* player);  // Send all existing players (PacketJoin) to new player
    void sub_51CB21(Player* player);  // Send terrain/diplomacy visibility state to player
    void sub_51C0F7(Player* player);  // Send units from pending unit list to player
    void sub_51CA5D(Player* player);  // Send server state to player
    void sub_51D1A8(uint16_t player_id, Player* player);  // Send kill stats (pass 0,nullptr for all)
    void sub_51CF5C(Unit* unit, int flag, Player* player); // Send unit visibility packet (retn 0Ch)
    void sub_51BE8F(AreaEffect* obj, int flag); // Send area-effect packet
    void sub_51D4F6(QuestMap* quest_map, Player* player, int flag); // Send SrvStru1 state list from packet

    void sub_51C7CC(int32_t latency, Player* player);  // Send latency update to player
    void sub_51CD2A(Player* player, int32_t event_id, int32_t arg3); // Send in-game event trigger

    NetStru2* sub_5185D5(uint16_t player_id);
    void sub_51C46E(Player* player);
    void sub_51C601(Unit* unit, int unused);        // Send unit-level-up packet
    void sub_51C822(NetStru2* ns2);    // Send spectator/alliance state

    // sub_51E7FC – decode an incoming network message into the appropriate Packet singleton,
    // populate it via VMethod4, and stamp it with the sender id.
    Packet* sub_51E7FC(uint8_t cmd, NetStru2* ns2);
    void sub_51AC77(CObject* token, Player* player, int8_t flag); // Broadcast token state to players
};
ASSERT_OFFSET(NetStru1, packer_dat1, 0x90);
ASSERT_OFFSET(NetStru1, field_0x1898, 0x1898);
ASSERT_SIZE(NetStru1, 0x20f0);

__pragma(pack(push, 1))
struct NetStru3 {
    uint32_t timestamp;
    uint32_t timestamp2;
    uint16_t pos;
    uint8_t field_0xa;
    uint8_t field_0xb;
    uint8_t buf_id;
    uint16_t size;
    uint8_t field_0xf;
    uint8_t buf[144];
    uint32_t datasz;
    uint32_t field_0xa4;
};
__pragma(pack(pop))
ASSERT_OFFSET(NetStru3, datasz, 0xa0);
ASSERT_SIZE(NetStru3, 0xa8);

__pragma(pack(push, 1))
struct NetStru2 {
    NetStru1* net_stru1;
    CLlDriver* driver;
    uint8_t buf[256];
    uint16_t player_id;
    uint8_t gap_0x10a[2];
    uint32_t uid;
    NetStru3 stru3[2];
    int32_t stru3_id;
    CList<NetStru3*> unpacked_buffers;
    CriticalSection critical_section;
    uint32_t field_0x298;
    uint32_t field_0x29c;
    uint32_t field_0x2a0;
    uint32_t field_0x2a4;
    uint32_t field_0x2a8;
    uint32_t field_0x2ac;
    CString str;
    uint32_t field_0x2b4;
    uint32_t field_0x2b8;
    uint32_t field_0x2bc;


public:
    int FUN_00515ef3(void* buf, uint32_t size);
    int FUN_00515f9c(void* buf, uint32_t size);

    // sub_5167A5 – walks the free_net3 linked list and decrements the
    // ref-count byte (field_0xF) of the first live entry it finds.
    void sub_5167A5();
};
__pragma(pack(pop))

ASSERT_OFFSET(NetStru2, player_id, 0x108);
ASSERT_OFFSET(NetStru2, critical_section, 0x280);
ASSERT_SIZE(NetStru2, 0x2c0);

struct NetSockLatency
{
    uint32_t lat_times[128];
    uint32_t num;
    uint32_t calc_latency;
};

struct A2NetSock {
    uint32_t is_in_use;
    uint32_t uid;
    HANDLE socket;
    DPID player_dpid;
    NetStru2* manager;
    CList<NetStru3*> list_0x14;
    CList<NetStru3*> list_0x30;
    uint32_t field_0x4c;
    uint32_t field_0x50;
    NetSockLatency latency_check;
    uint32_t latency;
    int32_t field_0x260;
    int32_t field_0x264;
    HANDLE wait_obj;
    NetStru3* current_buffer;
    uint32_t copy_num;
};
ASSERT_OFFSET(A2NetSock, list_0x30, 0x30);
ASSERT_OFFSET(A2NetSock, wait_obj, 0x268);
ASSERT_SIZE(A2NetSock, 0x274);

struct SocketNm {
    DPID dpid;
    A2NetSock *sock;
};

struct ComSettings 
{
    int32_t index;
    uint32_t speed;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t flow_control;
};

struct CLlName 
{
    char name[256];
};

struct CLlAddress : CLlName
{
    char address[256];
    union {
        struct {
            uint32_t field_0x200;
            uint32_t field_0x204;
            uint32_t field_0x208;
            uint32_t field_0x20c;
            uint32_t field_0x210;
        };
        GUID guid;
        ComSettings com;
    };
};
ASSERT_SIZE(CLlAddress, 0x214);

struct CLlNetSession : CLlName
{
    GUID guid;
};
ASSERT_SIZE(CLlNetSession, 0x110);

struct CLlConn : CLlName
{
    int32_t typ;
};
ASSERT_SIZE(CLlConn, 0x104);

struct CLlDriver {
    NetStru1* net_stru1;
    A2NetSock listen_socket;
    A2NetSock ping_socket;
    int32_t is_server;
    int32_t provider;
    int32_t guaranteed;
    int32_t latency;
    char address_str[128];
    A2NetSock* connection_sockets;
    SocketNm* connections_info;
    CLlAddress* enum_addresses;
    uint32_t enum_addresses_num;
    CLlNetSession* enum_sessions;
    uint32_t enum_sessions_num;
    CLlName* enum_players;
    uint32_t enum_players_num;
    CLlConn* enum_conns;
    uint32_t enum_conns_num;
    uint32_t max_connections;
    uint32_t num_connections;
    uint32_t next_uid;
    uint32_t unused;
    GUID application_guid;
    union
    {
        IDirectPlay4A* dplay4;
        IDirectPlay3A* dplay3;
    };
    HANDLE ev_create_player;
    HANDLE ev_close;
    HANDLE ev_new_session;
    uint32_t session_lost;
    CLlAddress cur_address;
    CriticalSection critical_section;
    uint32_t timeout;
    uint32_t keepalive;
    uint32_t dplay_is_4;
    char comp_name[256];
    CLlAddress* server_start_addr;

public:
    void sub_5229CD(int32_t conn_uid, int32_t latency_ms); // Set connection latency limit
    int32_t sub_5229FD(int32_t conn_uid);  // Get current measured latency (ms)
    int32_t sub_522A51(int32_t conn_uid);  // Get current packet-loss figure
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);
