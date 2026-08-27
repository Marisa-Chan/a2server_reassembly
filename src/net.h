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
class CMultiShopInstance;
class CMultiShopShelf;
class PacketAoeZone;
class PacketMoveCmd;
struct CLlDriver;
class Effect;
class NetStru1;
struct NetStru2;
struct NetStru3;
class Packet;
class Player;
class Shop;
class Spell;
class Token;
struct TokenPos;
class Unit;
class QuestMap;
struct Inventory;


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

    void DeleteTail(PackerTail* node); //526621
    void Clear(); //5265f7

    PackerDat(); //5260fa
    ~PackerDat(); //526115
};
ASSERT_OFFSET(PackerDat, freq, 0x800);
ASSERT_SIZE(PackerDat, 0xc04);

struct ConnStatInfo {
    uint32_t field1;
    uint32_t cur_bs;
    uint32_t total_bytes;
    uint32_t max_bs;
    uint32_t time;

    ConnStatInfo()
    {
        memset(this, 0, sizeof(ConnStatInfo));
    }
};
ASSERT_SIZE(ConnStatInfo, 0x14);

class NetStru1 {
public: // VTable at 0060ecc0.
    virtual void OnClientConnect(NetStru2* client); //51fc72
    virtual void OnClientDisconnect(NetStru2* client); //51f561

public:
    CLlDriver* lldriver; //low-level net driver
    NetStru1* linked_hl;
    CList<NetStru2*> disconnect_list;
    CList<NetStru2*> new_connects;
    CList<NetStru3*> free_net3;
    CRITICAL_SECTION critical_section;
    CRITICAL_SECTION critical_section2;
    PackerDat packer_dat1; //unused
    PackerDat packer_dat2; //unused
    uint32_t field_0x1898; //unused packer1 statistic
    uint32_t field_0x189c; //unused packer1 statistic
    uint32_t field_0x18a0; //unused packer2 statistic
    uint32_t field_0x18a4; //unused packer2 statistic
    uint32_t compression_type;
    uint32_t compression_mode; // 0 - disable, 1 - make statistic, 2 - compression
    uint32_t make_statistics; // calculate statistic
    NetStru2* local_client;
    CList<NetStru2*> active_connects;
    CMap<int32_t, int32_t, ConnStatInfo*, ConnStatInfo*> client_stat;
    uint32_t stat_pkt_num[256];
    uint32_t stat_pkt_size[256];

    static NetStru1 HatConnector;

public:
    // Transfers unit ownership to another player.
    // `this` is unused. TODO: looks like this should be in `SrvStru1` instead.
    void FUN_004fb4ca(Unit* unit, Player* new_owner);

    NetStru1(int stats); //51684b
    ~NetStru1(); //516b44

public:
    void FUN_0051ce86(uint32_t msg_type, uint32_t player_id, Player* recpt);
    void FUN_0051ceac(uint8_t id, Player* recpt);
    void FUN_0051c748(Player* recpt);
    void sub_51C61E(Token* token); // Remove unit/sack from network tracking
    void FUN_0051d49b(Player* recpt);

    void FUN_0051cd89(const CString& name, Player* player);

    NetStru2* GetClientByPlayerID(uint16_t player_id);//518544
    NetStru2* GetClientByLowUid(uint32_t low_uid); //5185d5

    void AddStat1(NetStru2* client, int32_t value); //51eff8
    void AddPacketStatistic(Packet* pkt); //51f0bb
    void SendAllData(); // 5188db and 518927 variant without flag check because in original it's used once and not optimal, because iterate all and do not do anything

    void QueuePacketSend(Packet* pkt); //5186cd
    void SendPacket_64(uint32_t val, uint16_t player_id); //51d88d
    void SetLLDriver(CLlDriver* drv); //517ff9
    void SetLinkedHLDriver(NetStru1* hl); //51703e

    uint32_t GetClientsPktNum(); //518a6b
    int IsActive(); //518ac9

    void FUN_0051d6b4(uint16_t arg);
    void FUN_0051cefb(uint8_t param_1, int32_t param_2, int32_t param_3, Player* param_4);
    
    void sub_51B99E(Unit* unit, uint8_t angle, uint8_t speed_dir, uint8_t mode);
    void sub_51C59C(Unit* unit, Unit* target); // Notify clients about unit turning to face its target
    void sub_519221(Unit* unit, Player* player, uint32_t mask, int32_t param4, int32_t param5, int32_t param6);

    void sub_51A0EF(Unit* unit, Player* player, int32_t flags);
    void sub_51A6D5(Unit* unit, Player* player, int32_t param5, int32_t param6);
    void sub_51BDA4(Effect* effect, Unit* unit, uint8_t arg);

    void sub_51D837(int32_t param1, Player* player); // Send server state / player list to player
    void sub_51C8B1(Player* player);  // Send all existing players (PacketJoin) to new player
    void sub_51CB21(Player* player);  // Send terrain/diplomacy visibility state to player
    void sub_51C0F7(Player* player);  // Send units from pending unit list to player
    void sub_51CA5D(Player* player);  // Send server state to player
    void sub_51D1A8(uint16_t player_id, Player* player);  // Send kill stats (pass 0,nullptr for all)
    void sub_51B870(Unit* unit); // Broadcast unit to all players
    void sub_51CF5C(Unit* unit, int flag, Player* player); // Send unit visibility packet (retn 0Ch)
    void sub_51BE8F(AreaEffect* obj, int flag); // Send area-effect packet
    void sub_51BE0E(Effect* effect, uint8_t type); // Broadcast move-cmd packet for an effect token
    void sub_51D4F6(QuestMap* quest_map, Player* player, int flag); // Send SrvStru1 state list from packet
    void sub_51A8B2(Token* token, Inventory* inventory, Player* player, int32_t param);
    void sub_51AA26(Shop* shop, CMultiShopShelf* shelf, Player* player, uint8_t slot); // 51AA26 - send single shop shelf contents
    void sub_51AB99(CMultiShopInstance* inst, Player* player, int32_t param); // 51AB99 - send shop shelf contents
    void sub_51EEB7();

    void sub_51C7CC(int32_t latency, Player* player);  // Send latency update to player
    void sub_51CD2A(Player* player, int32_t event_id, int32_t arg3); // Send in-game event trigger
    void sub_51CDFB(CString& name, uint32_t score, char type, Player* player); // 51CDFB - send arena score

    void sub_51C46E(Player* player);
    void sub_51C601(Unit* unit, int unused);        // Send unit-level-up packet
    void sub_51C822(NetStru2* ns2);    // Send spectator/alliance state

    // DecodePacket – decode an incoming network message into the appropriate Packet singleton,
    // populate it via VMethod4, and stamp it with the sender id.
    Packet* DecodePacket(uint8_t cmd, NetStru2* ns2); //51e7fc
    Packet* ReceivePacket(NetStru2* cli); //518a23 //receive packet from client
    Packet* ReceiveAnyPacket(); //518980 iterate over active connections and try receive packet

    void sub_51AC77(Token* token, Player* player, uint8_t flag); // Broadcast token state to players
    void sub_51BAB0(Unit* caster, Spell* spell, Unit* target, int16_t delay); // Send targeted-unit spell packet
    void sub_51BB94(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay); // Send point/area spell packet
    void sub_51BC6B(Unit* caster, Spell* spell, CList<Unit*>* units, int32_t flag); // 51BC6B: Send multi-target spell packet
    void sub_51B370(PacketMoveCmd* pkt, TokenPos* pos);  // Broadcast move-cmd packet to all players in visibility
    void sub_51B638(PacketAoeZone* pkt, AreaEffect* area_effect);  // Broadcast AoE-zone packet to all players in visibility
    void sub_51B0F0(Packet* pkt, Unit* unit); // 51B0F0 - send unit action packet to relevant players (in asm)


    void AddTailNet2(NetStru2*& net2); //517e91
    void AddTailFreeNet3(NetStru3* net3); //517b00

    NetStru2* AllocClientBufManager(uint32_t uid); //517c99

    NetStru3* GetFreeNet3(); //5177f5

    void ProcessNewConnects(); //51755c
    void ProcessDisconnectList(); //51725b
    void ProcessConnections(); // 51800f
    void DisconnectClient(NetStru2* cli); //5170b6

    void sub_51E5FB(int32_t a, int32_t b); // 51E5FB - Send HAT join-success reply (pkt 0xD8)
    void sub_51E63F(int32_t a, int32_t b, int32_t error_code); // 51E63F - Send HAT join-failure reply (pkt 0xD9)
    void sub_51E794(int32_t a, int32_t b, uint8_t status, int16_t player_id); // 51E794 - Send HAT player-check reply (pkt 0xE1)

    void sub_51E205(CString name); //51e205 - Send hat login packet
    void sub_51E289(int32_t arg0, CString source, int32_t arg8, int32_t argC, int32_t arg10); //51e289 - Send hat lobby info packet
    void sub_51DFA7(int32_t param1, int32_t param2, CString name, void* data, uint32_t size, uint16_t player_id);
    void sub_51E0B7(int32_t param1, int32_t param2, CString name, void* data, uint32_t size, uint16_t player_id);
    void sub_51E1C7(uint16_t player_id);
    void sub_51E40C(const char* str, int32_t len); // 51E40C - Send raw string data to HAT

    void WriteStatsLog(CString fname);//51f124

    NetStru2* FUN_0041f520();
};
ASSERT_OFFSET(NetStru1, packer_dat1, 0x90);
ASSERT_OFFSET(NetStru1, field_0x1898, 0x1898);
ASSERT_SIZE(NetStru1, 0x20f0);

__pragma(pack(push, 1))
struct NetStru3 {
    uint32_t timestamp;
    uint32_t timestamp2;
    union
    {
        struct
        {
            union
            {
                struct
                {
                    uint16_t pos;
                    uint16_t __padding_0xa;
                };
                uint32_t pktid;
            };
            
            uint8_t cmode; //compression mode
            uint16_t csize;
            uint8_t pkt_num; //number of packets in buffer?
        };
        uint8_t full_data[8];
    };
    uint8_t buf[144];
    int32_t datasz;
    int32_t readpos;

    void Clear(); //5156f6
    NetStru3(); //5155c0
    ~NetStru3(); //515694

    void CopyData(const NetStru3* src); //515625

    int WriteToBuffer(void* data, uint32_t sz); //51573e
    int ReadFromBuffer(void* out, uint32_t sz); //51579e
};
__pragma(pack(pop))
ASSERT_OFFSET(NetStru3, datasz, 0xa0);
ASSERT_SIZE(NetStru3, 0xa8);

__pragma(pack(push, 1))
struct NetStru2 {
    NetStru1* hldriver;
    CLlDriver* lldriver;
    uint8_t buf[256];
    uint16_t player_id;
    uint8_t gap_0x10a[2];
    uint32_t uid;
    NetStru3 out_buffers[2]; // buffers with send data
    int32_t out_buff_id; // currenct send buffer
    CList<NetStru3*> received_buffers; //buffers with received data
    CRITICAL_SECTION critical_section;
    uint32_t compression_type; // 0 - disabled, 1,2 - compression types
    uint32_t is_local_player;
    uint32_t field_0x2a0;
    uint32_t field_0x2a4;
    uint32_t field_0x2a8;
    uint32_t field_0x2ac;
    CString str;
    int32_t field_0x2b4;
    uint32_t field_0x2b8;
    uint32_t field_0x2bc;


public:
    int WriteData(void* buf, uint32_t size);
    int ReadData(void* buf, uint32_t size);

    // sub_5167A5 – walks the free_net3 linked list and decrements the
    // ref-count byte (field_0xF) of the first live entry it finds.
    void sub_5167A5();

    void SetDrivers(NetStru1* HlDriver, CLlDriver* LlDriver); //51682d
    int ReceiveData(NetStru3* buffer); //5161bf

    int SendData(); //516408

    NetStru2(); //515831
    ~NetStru2(); //515c27
    void ReturnBuffers(); //515d9d    return buffers to High-level driver

    uint32_t GetBuffersPktNum(); //51670a  Get received_buffers summary of pkt_num

    uint32_t GetUid() { return uid; }; //41f500
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

    void AddLatency(uint32_t lat_time); //51fdf7
    static inline uint32_t ShiftToByte(uint32_t val, int shift) { return (val >> shift) & 0xFF; };
    uint32_t GetLatency() const { return calc_latency * 2; }; //5204fa
};

struct A2NetSock {
    uint32_t is_in_use;
    uint32_t uid;
    SOCKET socket;
    uint32_t player_dpid;
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
    int32_t copy_num;

    A2NetSock(); //520510
    ~A2NetSock(); //520684
};
ASSERT_OFFSET(A2NetSock, list_0x30, 0x30);
ASSERT_OFFSET(A2NetSock, wait_obj, 0x268);
ASSERT_SIZE(A2NetSock, 0x274);

struct SocketNm {
    uint32_t dpid;
    A2NetSock *sock;

    SocketNm()
    {
        dpid = 0;
        sock = nullptr;
    };

    ~SocketNm()
    {
        dpid = 0;
        sock = nullptr;
    };
};

struct ComSettings  //DPCOMPORTADDRESS
{
    int32_t index;
    uint32_t speed;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t flow_control;
};
ASSERT_SIZE(ComSettings, 0x14);

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
    NetStru1* hl_driver;
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
    CRITICAL_SECTION critical_section;
    uint32_t timeout;
    uint32_t keepalive;
    uint32_t dplay_is_4;
    char comp_name[256];
    CLlAddress* server_start_addr;

public:
    A2NetSock* GetClientBySocketId(uint32_t uid); //5224c4

    CLlDriver(GUID _appid, NetStru1 *net1); //5208ec
    ~CLlDriver(); //520ad3

    void SetHlDriver(NetStru1* drv); //496e50

    void Close(); //5225d0
    void CloseTcp(); //523b01
    void CloseDp(); //525a09
    void CloseTcpSocket(A2NetSock* sock); //523a16
    void CloseDpSock(A2NetSock* sock, int idx = -1); //525bcd

    CLlAddress* AllocAddress(); //520d8c
    void FreeEnumAddresses(); //520c80

    int EnumAddresses(CLlAddress** addrs, int* num); //52182f
    void EnumAddressesTcp(); //5214c5
    void EnumAddressesDp(); //5214c5
    static BOOL __stdcall cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext); //5211b2

    CLlNetSession* AllocSession(); //520e2e
    void FreeEnumSessions(); //520cc3

    typedef int(*CBUPDATECALLBACK)();
    int EnumSessions(CLlNetSession**, int* num, CBUPDATECALLBACK cbUpdate = nullptr, uint32_t timeout = 0); //52210d
    static BOOL __stdcall cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext); //522020


    CLlConn* AllocEnumConns(); //520ed0
    void FreeEnumConns(); //520d06
    void EnumConnections(CLlConn** conns, int* num); //5210ea
    static BOOL __stdcall cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); //52100e


    CLlName* AllocEnumPlayers(); //520f72
    void FreeEnumPlayers(); //520d49
    void EnumPlayers(CLlNetSession* ses, CLlName** names, int* num); //52238e
    static BOOL __stdcall cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); //52233f


    void Free(); //5219af
    void FreeTcp(); //523611
    void FreeDp(); //523d5a

    int ResetProvider(int provider); //52194f

    int CreateTcp(); //52359b
    int CreateDp(); //523ccf

    int IsListen() { return listen_socket.is_in_use == 1; }; //496ed0

    int IsInUse(A2NetSock* sock); //52293a
    void SetEventNewSession(); //522318

    int MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve); //52343e
    int StartServerTcp(); //523621

    static void __cdecl SetSockOptions(SOCKET s, int linger); //522af4

    static void __cdecl AcceptThreadTcp(void* context); //522e8f
    int RecvThreadTcp(A2NetSock* sock); //522b95

    int PrepareAddressDp(CLlAddress* addr); //523d96
    int StartServerDp(); //5241ea
    static void __cdecl RecvThreadDp(void* context); //523b7a
    void HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz); //524abd

    A2NetSock* FindDpSock(uint32_t id, int* out_idx); //524986

    int StartServer(int maxconn, const char* name, CLlAddress* addr); //521a09

    int PrepareForConnect(const char* name, CLlAddress* addr); //521d2a


    int ConnectDp(const char* name, CLlNetSession* session); //52465f
    int ConnectTcp(); //523794

    int Connect(const char* name, CLlNetSession* session); //5223fb

    int SendDataTcp(A2NetSock* sock, NetStru3* buf); //52394a
    int SendDataDp(A2NetSock* sock, NetStru3* buffer); //5255c6
    int SendData(uint32_t uid, NetStru3* buffer); //5226c4

    void SendAllPendingDp(A2NetSock* sock); //525b86
    void DisconnectClient(uint32_t uid); //522526
    void CleanupInvalidTcpClient(uint32_t uid); //52262a

    void CleanupAllInvalid(); //522809

    int RestartModemServerDp(); //52460e
    void RecreateDp(); //524644

    void SetLatency(uint32_t conn_uid, int32_t latency_ms); // 5229cd Set connection latency limit
    int32_t GetLatency(uint32_t conn_uid);  //5229FD  Get current measured latency (ms)
    int32_t GetPacketLoss(uint32_t conn_uid);  //522A51 Get current packet-loss figure

    void SetTimeout(uint32_t t) { timeout = t; } //496f20
    int GetProvider() { return provider; } //451610
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);





extern NetStru1 g_NetStru1_main;  //6c3a08
extern NetStru1 g_NetStru1_local; //6b16b0

extern CLlDriver g_CLlDriver; //6d07a0
extern CLlDriver g_HatLLDriver; //6cdbc0
