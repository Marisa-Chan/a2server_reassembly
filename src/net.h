#pragma once

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class AreaEffect;
struct CLlDriver;
class Effect;
class NetStru1;
struct NetStru2;
struct NetStru3;
class Packet;
class Player;
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
    std::array<int32_t, 256> field_0x0;
    std::array<int32_t, 256> field_0x400;
    std::array<int32_t, 256> field_0x800;
    PackerTail* tail;
};
ASSERT_OFFSET(PackerDat, field_0x800, 0x800);
ASSERT_SIZE(PackerDat, 0xc04);

class NetStru1 {
public: // VTable at 0060ecc0.
    uint32_t vtable;
    // virtual void VMethod1(void* param_1) = 0; // Don't know the parameter type.
    // virtual void VMethod2(void* param_1) = 0;

public:
    CLlDriver* driver;
    int32_t field_0x8;
    CList<int32_t> list;
    CList<NetStru2*> list_stru2;
    CList<NetStru3*> list_stru3;
    CriticalSection critical_section;
    CriticalSection critical_section2;
    PackerDat packer_dat1;
    PackerDat packer_dat2;
    int32_t field_0x1898;
    int32_t field_0x189c;
    int32_t field_0x18a0;
    int32_t field_0x18a4;
    int32_t field_0x18a8;
    int32_t field_0x18ac;
    void* field_0x18b0;
    NetStru2* fields_0x18b4;
    CList<NetStru2*> list_0x18b8;
    CMap<int32_t, int32_t, int32_t, int32_t> map_0x18d4;

public:
    // Transfers unit ownership to another player.
    // `this` is unused, but in ASM this function accepts a `NetStru1*` in ECX, so it's here.
    void FUN_004fb4ca(Unit* unit, Player* new_owner);

public:
    void FUN_005186cd(Packet* pkt);
    void FUN_0051ce86(uint32_t msg_type, uint32_t player_id, Player* recpt);
    void FUN_0051ceac(uint32_t id, Player* recpt);
    void FUN_0051c748(Player* recpt);
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
    void sub_51C601(Unit* unit);        // Send unit-level-up packet
    void sub_51C822(NetStru2* ns2);    // Send spectator/alliance state

    // sub_51E7FC – decode an incoming network message into the appropriate Packet singleton,
    // populate it via VMethod4, and stamp it with the sender id.
    Packet* sub_51E7FC(uint8_t cmd, NetStru2* ns2);
};
ASSERT_OFFSET(NetStru1, packer_dat1, 0x90);
ASSERT_OFFSET(NetStru1, field_0x1898, 0x1898);
ASSERT_SIZE(NetStru1, 0x18f0);

struct NetStru3 {
    int32_t field_0x0;
    int32_t field_0x4;
    int32_t field_0x8;
    int32_t field_0xc;
    uint8_t gap_0x10[144];
    int32_t field_0xa0;
    int32_t field_0xa4;
};
ASSERT_OFFSET(NetStru3, field_0xa0, 0xa0);
ASSERT_SIZE(NetStru3, 0xa8);

struct NetStru2 {
    NetStru1* net_stru1;
    int32_t field_0x4;
    uint8_t gap_0x8[256];
    uint16_t player_id;
    uint8_t gap_0x10a[2];
    int32_t uid;
    NetStru3 stru3;
    NetStru3 stru3_2;
    int32_t field_0x260;
    CList<NetStru3*> list_stru3;
    CriticalSection critical_section;
    int32_t field_0x298;
    int32_t field_0x29c;
    int32_t field_0x2a0;
    int32_t field_0x2a4;
    int32_t field_0x2a8;
    int32_t field_0x2ac;
    CString str;
    int32_t field_0x2b4;
    int32_t field_0x2b8;
    int32_t field_0x2bc;


public:
    int FUN_00515ef3(void* buf, uint32_t size);
    int FUN_00515f9c(void* buf, uint32_t size);

    // sub_5167A5 – walks the list_stru3 linked list and decrements the
    // ref-count byte (field_0xF) of the first live entry it finds.
    void sub_5167A5();
};
ASSERT_OFFSET(NetStru2, player_id, 0x108);
ASSERT_OFFSET(NetStru2, critical_section, 0x280);
ASSERT_SIZE(NetStru2, 0x2c0);

struct A2NetSock {
    int32_t is_in_use;
    int32_t maybe_uid;
    HANDLE socket;
    int32_t field_0xc;
    NetStru2* net_stru2;
    CList<void*> list_0x14;
    CList<void*> list_0x30;
    uint8_t gap_0x4c[528]; // There some fields here, to be determined later.
    int32_t field_0x25c;
    int32_t field_0x260;
    int32_t field_0x264;
    HANDLE wait_obj;
    NetStru3* stru3;
    int32_t field_0x270;
};
ASSERT_OFFSET(A2NetSock, list_0x30, 0x30);
ASSERT_OFFSET(A2NetSock, wait_obj, 0x268);
ASSERT_SIZE(A2NetSock, 0x274);

struct SocketNm {
    uint8_t field_0x0;
    uint8_t field_0x4;
};

struct SockStartNm {
    char field_0x0[256];
    char conn_string[256];
};

struct CLlDriver {
    NetStru1* net_stru1;
    A2NetSock listen_socket;
    A2NetSock socket;
    int32_t field_0x4ec;
    int32_t use_provider;
    int32_t field_0x4f4;
    int32_t field_0x4f8;
    uint8_t gap_0x4fc[128];
    A2NetSock* connection_sockets;
    SocketNm* socket_nms;
    int32_t field_0x584[8];
    int32_t num_connections;
    int32_t field_0x5a8;
    int32_t maybe_next_uid;
    int32_t field_0x5b0;
    int32_t some_consts[4];
    void* dplay;
    int32_t field_0x5c8;
    HANDLE handle;
    int32_t field_0x5d0;
    int32_t field_0x5d4;
    uint8_t gap_0x5d8[532];
    CriticalSection critical_section;
    int32_t field_0x804;
    int32_t field_0x808;
    int32_t dplay_is_4;
    char comp_name[256];
    SockStartNm* add_strings;

public:
    void sub_5229CD(int32_t conn_uid, int32_t latency_ms); // Set connection latency limit
    int32_t sub_5229FD(int32_t conn_uid);  // Get current measured latency (ms)
    int32_t sub_522A51(int32_t conn_uid);  // Get current packet-loss figure
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);
