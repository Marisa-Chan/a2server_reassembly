#pragma once
#include <winsock2.h>
#include <array>
#include <cstdint>

#include <afx.h>
#include "assert_offset.h"
#include <afxtempl.h>

#pragma warning(push)
#pragma warning(disable: 4005) // dplay.h redefines INTERFACE without a guard
#include "dplay.h"
#pragma warning(pop)


class AreaEffect;
class CMultiShopInstance;
class CMultiShopShelf;
class PacketAoeZone;
class PacketMoveBroadcast;
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


constexpr const bool IsDplayAvail = true;

#pragma pack(push, 1)
union HatCharId {
    uint64_t id;
    struct {
        int32_t id1;
        int32_t id2;
    };
};
#pragma pack(pop)

struct PackerTail {
    PackerTail* child0;
    PackerTail* child1;
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

    void DeleteTail(PackerTail* node); //526621 (client: 515453)
    void Clear(); //5265f7 (client: 515429)

    PackerDat(); //5260fa (client: 514efa)
    ~PackerDat(); //526115 (client: 514f15)

    void ResetFreq(); //526128
    void AddFreq(const uint8_t* data, int32_t sz); //52614b (client: 514f4b)
    void SaveFreq(CFile* file); //5261aa (client: 514faa)
    void LoadFreq(CFile* file); //5261d0 (client: 514fd0)
    void BuildTree(); //526247 (client: 515047)
    void BuildCodes(); //52651f (client: 515351)
    void BuildCodesWalk(PackerTail* node, uint32_t code, int32_t depth); //52656b (client: 51539d)
    static void Insert(PackerTail* slot, uint8_t ch, int32_t weight); //5263d3 (client: 5151ea)
    static int CompareFreq(const void* a, const void* b); //526218 (client: 515018)
    uint32_t PackBits(const uint8_t* src, int32_t sz, uint32_t* out); //52667e (client: 5154b0)
    uint32_t UnpackBits(const uint8_t* in, int32_t sz, uint8_t* out, int32_t outsz); //526749 (client: 51557b)
};

uint32_t PackBitsToBytes(uint32_t bits); //526665 (client: 515497)
ASSERT_OFFSET(PackerDat, freq, 0x800);
ASSERT_SIZE(PackerDat, 0xc04);

struct ConnStatInfo {
    uint32_t field1;
    uint32_t cur_bs;
    uint32_t total_bytes;
    uint32_t max_bs;
    uint32_t time;

    ConnStatInfo() {
        memset(this, 0, sizeof(ConnStatInfo));
    }
};
ASSERT_SIZE(ConnStatInfo, 0x14);

class NetStru1 {
public: // VTable at 0060ecc0.
    virtual void OnClientConnect(NetStru2* client); //51fc72 (client: 50fe58)
    virtual void OnClientDisconnect(NetStru2* client); //51f561 (client: 50fc92)

public:
    CLlDriver* lldriver; //low-level net driver
    NetStru1* linked_hl;
    CList<NetStru2*> disconnect_list;
    CList<NetStru2*> new_connects;
    CList<NetStru3*> free_net3;
    CRITICAL_SECTION critical_section;
    CRITICAL_SECTION critical_section2;
    PackerDat packer_dat1; //huffman tables for Pack/Unpack channel 1
    PackerDat packer_dat2; //huffman tables for Pack/Unpack channel 2
    uint32_t field_0x1898; //packer1 trained-bytes statistic
    uint32_t field_0x189c; //packer1 packed-bytes statistic
    uint32_t field_0x18a0; //packer2 trained-bytes statistic
    uint32_t field_0x18a4; //packer2 packed-bytes statistic
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
    // Huffman-packs `sz` bytes from `src` into `out` using packer table `mode` (1/2).
    // Returns the packed BIT count (0 when mode is invalid). compression_mode == 1
    // only trains the tables (AddFreq) and copies the input verbatim.
    uint32_t Pack(char mode, const uint8_t* src, uint32_t sz, uint32_t* out); //51832c (client: 509f10)
    // Inverse of Pack: decodes `sz` bits into `out` (max `outsz` bytes). Returns the
    // byte count written. compression_mode == 1 trains from sz>>3 bytes + copies.
    uint32_t Unpack(char mode, const uint8_t* in, int32_t sz, uint8_t* out, int32_t outsz); //51846f (client: 50a053)

public:
    // Transfers unit ownership to another player.
    // `this` is unused. TODO: looks like this should be in `SrvStru1` instead.
    void TransferUnitOwnership(Unit* unit, Player* new_owner);  // client: 4f9322

    NetStru1(int stats); //51684b (client: 509343)
    ~NetStru1(); //516b44 (client: 5094ca)

public:
    void SendTeamNotify(uint32_t msg_type, uint32_t player_id, Player* recpt);  // client: 50df07
    void SendBarePacket(uint8_t id, Player* recpt);  // client: 50df2d
    void SendTickPacket(Player* recpt); //51c748 (client: 50d82f)
    void UntrackToken(Token* token); // Remove unit/sack from network tracking (client: 50d7b2)
    void AnnouncePlayer(Player* recpt); // 51d49b (client: 50e2cc)

    void AnnouncePlayerName(const CString& name, Player* player); // 51cd89 (client: 50de98)

    NetStru2* GetClientByPlayerID(uint16_t player_id);//518544 (client: 50a128)
    NetStru2* GetClientByLowUid(uint32_t low_uid); //5185d5 (client: 50a1ab)

    void AddStat1(NetStru2* client, int32_t value); //51eff8 (client: 50f79c)
    void AddPacketStatistic(Packet* pkt); //51f0bb (client: 50f7dd)
    void SendAllData(); // 5188db (client: 50a3b1) and 518927 variant without flag check because in original it's used once and not optimal, because iterate all and do not do anything

    void QueuePacketSend(Packet* pkt); //5186cd (client: 50a218)
    void SendPacket_64(uint32_t val, uint16_t player_id); //51d88d (client: 50e446)
    void SetLLDriver(CLlDriver* drv); //517ff9 (client: 509bd1)
    void SetLinkedHLDriver(NetStru1* hl); //51703e (client: 50963b)

    uint32_t GetClientsPktNum(); //518a6b (client: 50a4de)
    int IsActive(); //518ac9 (client: 50a53b)

    void SendTeamState(uint16_t to_player_id);
    void SendInfoPacket(uint8_t id, int32_t value1, int32_t value2, Player* target);  // client: 50df7c
    
    void SendUnitActionPacket(Unit* unit, uint8_t angle, uint8_t speed_dir, uint8_t mode);  // client: 50cd3c
    void SendRangedAttackPacket(Unit* unit);  // client: 50cdcd
    void SendUnitTurn(Unit* unit, Unit* target); // Notify clients about unit turning to face its target (client: 50d730)
    void SendUnitUpdate(Unit* unit, Player* player, uint32_t mask, int32_t props_flags, int32_t inv_start_index, int32_t inv_end_index);  // client: 50ac91

    void SendHumanoidProps(Unit* unit, Player* player, int32_t flags);  // client: 50b9df
    void SendMonsterInventory(Unit* unit, Player* player, int32_t start_index, int32_t end_index);  // client: 50beae
    void BroadcastEffect(Effect* effect, Unit* unit, uint8_t packet_id);  // client: 50d132

    void SendGameInfo(int32_t game_speed, Player* player); // Send server state / player list to player
    void SendExistingPlayers(Player* player);  // Send all existing players (PacketJoin) to new player (client: 50d96e)
    void SendVisibilityState(Player* player);  // Send terrain/diplomacy visibility state to player (client: 50dabc)
    void SendPendingUnits(Player* player);  // Send units from pending unit list to player (client: 50d485)
    void SendServerState(Player* player);  // Send server state to player (client: 50da69)
    void SendKillStats(uint16_t player_id, Player* player);  // Send kill stats (pass 0,nullptr for all) (client: 50e1b7)
    void BroadcastUnitState(Unit* unit); // Broadcast unit to all players. 51b870 (client: 50cc7f)
    void SendUnitVisibility(Unit* unit, int flag, Player* player); // Send unit visibility packet (retn 0Ch) (client: 50dfdd)
    void SendAreaEffect(AreaEffect* obj, int flag); // Send area-effect packet (client: 50d21d)
    void BroadcastEffectMove(Effect* effect, uint8_t type); // Broadcast move packet for an effect token (client: 50d19c)
    void SendSrvStru1State(QuestMap* quest_map, Player* player, int flag); // Send SrvStru1 state list from packet (client: 50e326)
    void SendShopTable(Token* token, Inventory* inventory, Player* player, int32_t mode);  // client: 50c006
    void SendShopShelf(Shop* shop, CMultiShopShelf* shelf, Player* player, uint8_t slot); // 51AA26 (client: 50c10b) - send single shop shelf contents
    void SendShopShelves(CMultiShopInstance* inst, Player* player, int32_t shelf_id); // 51AB99 (client: 50c25c) - send shop shelf contents
    void UpdateConnStats();  // client: 50f6ef

    void SendLatency(int32_t latency, Player* player);  // Send latency update to player (client: 50d8ab)
    void SendGameEvent(Player* player, int32_t event_id, int32_t value2); // Send in-game event trigger (client: 50de39)
    void SendArenaScore(CString& name, uint32_t score, char type, Player* player); // 51CDFB - send arena score

    void ResendPlayerUnits(Player* player); // 51C46E (client: 50d66d)
    void SendLevelUp(Unit* unit, int unused);        // Send unit-level-up packet (client: 50d795)
    void SendSpectatorState(NetStru2* ns2);    // Send spectator/alliance state (client: 50d901)

    // DecodePacket – decode an incoming network message into the appropriate Packet singleton,
    // populate it via VMethod4, and stamp it with the sender id.
    Packet* DecodePacket(uint8_t cmd, NetStru2* ns2); //51e7fc (client: 50f04e)
    Packet* ReceivePacket(NetStru2* cli); //518a23 (client: 50a498) //receive packet from client
    Packet* ReceiveAnyPacket(); //518980 (client: 50a3fc) iterate over active connections and try receive packet

    void BroadcastTokenState(Token* token, Player* player, uint8_t flag); // Broadcast token state to players (client: 50c2ea)
    void SendSpellCastAtUnit(Unit* caster, Spell* spell, Unit* target, int16_t delay); // Send targeted-unit spell packet (client: 50ce4e)
    void SendSpellCastAtPos(Unit* caster, Spell* spell, TokenPos* pos, int16_t delay); // Send point/area spell packet (client: 50cf32)
    void SendSpellCastMulti(Unit* caster, Spell* spell, CList<Unit*>* units, int32_t flag); // 51BC6B (client: 50d009): Send multi-target spell packet
    void BroadcastMovePacket(PacketMoveBroadcast* pkt, TokenPos* pos);  // Broadcast move packet to all players in visibility (client: 50c867)
    void BroadcastAoeZone(PacketAoeZone* pkt, AreaEffect* area_effect);  // Broadcast AoE-zone packet to all players in visibility. 51b638 (client: 50cab9)
    void SendUnitAction(Packet* pkt, Unit* unit); // 51B0F0 - send unit action packet to relevant players (in asm) (client: 50c65a)


    void AddTailNet2(NetStru2*& net2); //517e91 (client: 509b9b)
    void AddTailFreeNet3(NetStru3* net3); //517b00 (client: 509a73)

    NetStru2* AllocClientBufManager(uint32_t uid); //517c99 (client: 509ad7)

    NetStru3* GetFreeNet3(); //5177f5 (client: 50994f)

    void ProcessNewConnects(); //51755c (client: 509889)
    void ProcessDisconnectList(); //51725b (client: 509799)
    void ProcessConnections(); // 51800f (client: 509be7)
    void DisconnectClient(NetStru2* cli); //5170b6 (client: 5096a2)

    void SendHatJoinOk(int32_t a, int32_t b); // 51E5FB - Send HAT join-success reply (pkt 0xD8)
    void SendHatJoinFail(int32_t a, int32_t b, int32_t error_code); // 51E63F - Send HAT join-failure reply (pkt 0xD9)
    void SendHatPlayerCheck(int32_t a, int32_t b, uint8_t status, int16_t player_id); // 51E794 - Send HAT player-check reply (pkt 0xE1)

    void SendHatLogin(CString name); //51e205 (client: 50ec0e) - Send hat login packet
    void SendHatLobbyInfo(int32_t player_count, CString source, int32_t map_level, int32_t game_type, int32_t map_range); //51e289 - Send hat lobby info packet
    void SendHatCharData(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id);  // client: 50eb05
    void SendHatMessage(int32_t hat_char_id1, int32_t hat_char_id2, CString name, void* data, uint32_t size, uint16_t player_id);
    void SendHatKeepalive(uint16_t player_id);
    void SendHatRawString(const char* str, int32_t len); // 51E40C - Send raw string data to HAT (client: 50ede9)

    void SendHatCredentials(const CString& login, const CString& password, uint32_t flags); //51d8d6 (client: 50e48b)
    void SendHatCharId(HatCharId charid); //51d9d0 (client: 50e58b) client 50e58b
    void SendHatLogout(); //51d9a0 (client: 50e55b)

    void WriteStatsLog(CString fname);//51f124 (client: 50f846)

    void SendHatConnectServer(HatCharId key, uint8_t body, uint8_t reaction, uint8_t mind, uint8_t spirit, uint8_t sphere, uint8_t type_face, uint8_t color, const CString& name, const CString& srv); //51da14   (client:50e5cf)

    void SendServerConnectHatTicket(HatCharId ticket, uint16_t player_id); //51dce5   (client:50e8c0)
    void FUN_0050eede(); // client-only helper (client: 50eede)
    void FUN_0050e70d(HatCharId id); // client-only helper (client: 50e70d)

    NetStru2* GetFirstClient();
};
ASSERT_OFFSET(NetStru1, packer_dat1, 0x90);
ASSERT_OFFSET(NetStru1, field_0x1898, 0x1898);
ASSERT_SIZE(NetStru1, 0x20f0);

__pragma(pack(push, 1))
struct NetStru3 {
    uint32_t timestamp;
    uint32_t timestamp2;
    union {
        struct {
            union {
                struct {
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

    void Clear(); //5156f6 (client: 508736)
    NetStru3(); //5155c0 (client: 50863e)
    ~NetStru3(); //515694 (client: 5086f3)

    void CopyData(const NetStru3* src); //515625 (client: 508684)

    int WriteToBuffer(void* data, uint32_t sz); //51573e (client: 50877e)
    int ReadFromBuffer(void* out, uint32_t sz); //51579e (client: 5087de)
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
    uint32_t is_claimed;
    uint32_t field_0x2ac;
    CString str;
    int32_t field_0x2b4;
    uint32_t field_0x2b8;
    uint32_t field_0x2bc;


public:
    int WriteData(void* buf, uint32_t size);  // client: 508c6a
    int ReadData(void* buf, uint32_t size);  // client: 508d13

    // ReleaseNetBuf – walks the free_net3 linked list and decrements the
    // ref-count byte (field_0xF) of the first live entry it finds.
    void ReleaseNetBuf();  // 5167a5 (client: 5092a4)

    void SetDrivers(NetStru1* HlDriver, CLlDriver* LlDriver); //51682d (client: 509325)
    int ReceiveData(NetStru3* buffer); //5161bf (client: 508e04)

    int SendData(); //516408 (client: 508f33)

    NetStru2(); //515831 (client: 508871)
    ~NetStru2(); //515c27 (client: 508acc)
    void ReturnBuffers(); //515d9d (client: 508b9a)    return buffers to High-level driver

    uint32_t GetBuffersPktNum(); //51670a (client: 509208)  Get received_buffers summary of pkt_num

    uint32_t GetUid() { return this->uid; }; //41f500 (client: 420700)
};
__pragma(pack(pop))

ASSERT_OFFSET(NetStru2, player_id, 0x108);
ASSERT_OFFSET(NetStru2, critical_section, 0x280);
ASSERT_SIZE(NetStru2, 0x2c0);

struct NetSockLatency {
    uint32_t lat_times[128];
    uint32_t num;
    uint32_t calc_latency;

    void AddLatency(uint32_t lat_time); //51fdf7 (client: 50ff67)
    static inline uint32_t ShiftToByte(uint32_t val, int shift) { return (val >> shift) & 0xFF; };
    uint32_t GetLatency() const { return this->calc_latency * 2; }; //5204fa (client: 510014)
};

struct A2NetSock {
    uint32_t is_in_use;
    uint32_t uid;
    SOCKET socket;
    uint32_t player_dpid;
    NetStru2* manager;
    CList<NetStru3*> list_0x14;
    CList<NetStru3*> list_0x30;
    uint32_t next_out_pktid;
    uint32_t next_in_pktid;
    NetSockLatency latency_check;
    uint32_t latency;
    int32_t stat_pkts_queued;
    int32_t stat_pkts_sent;
    HANDLE wait_obj;
    NetStru3* current_buffer;
    int32_t copy_num;

    A2NetSock(); //520510 (client: 51002a)
    ~A2NetSock(); //520684 (client: 510121)
};
ASSERT_OFFSET(A2NetSock, list_0x30, 0x30);
ASSERT_OFFSET(A2NetSock, wait_obj, 0x268);
ASSERT_SIZE(A2NetSock, 0x274);

struct SocketNm {
    uint32_t dpid;
    A2NetSock *sock;

    SocketNm() {
        this->dpid = 0;
        this->sock = nullptr;
    };

    ~SocketNm() {
        this->dpid = 0;
        this->sock = nullptr;
    };
};

struct ComSettings { //DPCOMPORTADDRESS
    int32_t index;
    uint32_t speed;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t flow_control;

    void Reset() { //43cff2 (client: 43f402)
        this->index = 1;
        this->speed = 14400;
        this->stop_bits = 0;
        this->parity = 0;
        this->flow_control = 4;
    }

    void Load(HKEY& rkey) { //43d02e (client: 43f43e)
        this->Reset();
        DWORD sz = 0x14;
        RegQueryValueExA(rkey, "comportsettings", nullptr, nullptr, (LPBYTE)this, &sz);
    }
};
ASSERT_SIZE(ComSettings, 0x14);

struct CLlName {
    char name[256];
};

struct CLlAddress : CLlName {
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

struct CLlNetSession : CLlName {
    GUID guid;
};
ASSERT_SIZE(CLlNetSession, 0x110);

struct CLlConn : CLlName {
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
    union {
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
    A2NetSock* GetClientBySocketId(uint32_t uid); //5224c4 (client: 511c00)

    CLlDriver(GUID _appid, NetStru1 *net1); //5208ec (client: 510241)
    ~CLlDriver(); //520ad3 (client: 510429)

    void SetHlDriver(NetStru1* drv); //496e50 (client: 49a920)

    void Close(); //5225d0 (client: 511d0c)
    void CloseTcp(); //523b01 (client: 5131ad)
    void CloseDp(); //525a09 (client: 514b64)
    void CloseTcpSocket(A2NetSock* sock); //523a16 (client: 5130cf)
    void CloseDpSock(A2NetSock* sock, int idx = -1); //525bcd (client: 514d27)

    CLlAddress* AllocAddress(); //520d8c (client: 510646)
    void FreeEnumAddresses(); //520c80 (client: 51053a)

    int EnumAddresses(CLlAddress** addrs, int* num); //52182f (client: 5110c6)
    void EnumAddressesTcp(); //5214c5 (client: 510d5c)
    void EnumAddressesDp(); //521200 (client: 510a97)
    static BOOL __stdcall cbEnumAddress(REFGUID guidDataType, DWORD dwDataSize, LPCVOID lpData, LPVOID lpContext); //5211b2 (client: 510a50)

    CLlNetSession* AllocSession(); //520e2e (client: 5106e8)
    void FreeEnumSessions(); //520cc3 (client: 51057d)

    typedef int(*CBUPDATECALLBACK)();
    int EnumSessions(CLlNetSession**, int* num, CBUPDATECALLBACK cbUpdate = nullptr, uint32_t timeout = 0); //52210d (client: 51184d)
    static BOOL __stdcall cbEnumSessions(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext); //522020 (client: 51176c)


    CLlConn* AllocEnumConns(); //520ed0 (client: 51078a)
    void FreeEnumConns(); //520d06 (client: 5105c0)
    void EnumConnections(CLlConn** conns, int* num); //5210ea (client: 510988)
    static BOOL __stdcall cbEnumConnections(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); //52100e (client: 5108c8)


    CLlName* AllocEnumPlayers(); //520f72 (client: 51082c)
    void FreeEnumPlayers(); //520d49 (client: 510603)
    void EnumPlayers(CLlNetSession* ses, CLlName** names, int* num); //52238e (client: 511ace)
    static BOOL __stdcall cbEnumPlayes(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext); //52233f (client: 511a7f)


    void Free(); //5219af (client: 511246)
    void FreeTcp(); //523611 (client: 420b70)
    void FreeDp(); //523d5a (client: 513406)

    int ResetProvider(int provider); //52194f (client: 5111e6)

    int CreateTcp(); //52359b (client: 512c58)
    int CreateDp(); //523ccf (client: 51337b)

    int IsListen() { return this->listen_socket.is_in_use == 1; }; //496ed0 (client: 49a810)

    int IsInUse(A2NetSock* sock); //52293a (client: 512078)
    int IsUidInUse(uint32_t uid); //522966 (client: 5120a2) - GetClientBySocketId + IsInUse
    void SetEventNewSession(); //522318 (client: 511a58)

    int MakeSockAddr(const char* address, sockaddr_in* out, int port, int resolve); //52343e (client: 512afd)
    int StartServerTcp(); //523621 (client: 512cdc)

    static void __cdecl SetSockOptions(SOCKET s, int linger); //522af4 (client: 512230)

    static void __cdecl AcceptThreadTcp(void* context); //522e8f (client: 51258b)
    int RecvThreadTcp(A2NetSock* sock); //522b95 (client: 5122d1)

    int PrepareAddressDp(CLlAddress* addr); //523d96 (client: 513442)
    int StartServerDp(); //5241ea (client: 513896)
    static void __cdecl RecvThreadDp(void* context); //523b7a (client: 513226)
    void HandleMessageDp(uint32_t from, uint32_t to, void* data, uint32_t datasz); //524abd (client: 514141)

    A2NetSock* FindDpSock(uint32_t id, int* out_idx); //524986 (client: 51400a)

    int StartServer(int maxconn, const char* name, CLlAddress* addr); //521a09 (client: 51129c)

    int PrepareForConnect(const char* name, CLlAddress* addr); //521d2a (client: 51151c)


    int ConnectDp(const char* name, CLlNetSession* session); //52465f (client: 513d0b)
    int ConnectTcp(); //523794 (client: 512e4f)

    int Connect(const char* name, CLlNetSession* session); //5223fb (client: 511b3b)

    int SendDataTcp(A2NetSock* sock, NetStru3* buf); //52394a (client: 513005)
    int SendDataDp(A2NetSock* sock, NetStru3* buffer); //5255c6 (client: 514795)
    int SendData(uint32_t uid, NetStru3* buffer); //5226c4 (client: 511dfc)

    void SendAllPendingDp(A2NetSock* sock); //525b86 (client: 514ce1)
    void DisconnectClient(uint32_t uid); //522526 (client: 511c62)
    void CleanupInvalidTcpClient(uint32_t uid); //52262a (client: 511d62)

    void CleanupAllInvalid(); //522809 (client: 511f4d)

    int RestartModemServerDp(); //52460e (client: 513cba)
    void RecreateDp(); //524644 (client: 513cf0)

    void SetLatency(uint32_t conn_uid, int32_t latency_ms); // 5229cd (client: 512109) Set connection latency limit
    int32_t GetLatency(uint32_t conn_uid);  //5229FD  Get current measured latency (ms) (client: 512139)
    int32_t GetPacketLoss(uint32_t conn_uid);  //522A51 Get current packet-loss figure (client: 51218d)

    void SetTimeout(uint32_t t) { this->timeout = t; } //496f20 (client: 49ab40)
    int GetProvider() { return this->provider; } //451610 (client: 453580)
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);





extern NetStru1 g_NetStru1_main;  //6c3a08
extern NetStru1 g_NetStru1_local; //6b16b0

extern CLlDriver g_CLlDriver; //6d07a0
extern CLlDriver g_HatLLDriver; //6cdbc0

extern uint8_t g_PackStage[0x800]; //6e9900 (client: 6b5338) - NetStru1::Pack staging buffer



void SendWWWReport(const CString& surl, const CString& data, int val3); //client 4ea58f
