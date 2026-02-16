#pragma once

#include <array>
#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


struct CLlDriver;
class NetStru1;
struct NetStru2;
struct NetStru3;
class Packet;
class Player;


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
    void FUN_005186cd(Packet* pkt);
    void FUN_0051ce86(uint32_t msg_type, uint32_t player_id, Player* recpt);
    void FUN_0051ceac(uint32_t id, Player* recpt);
    void FUN_0051c748(Packet* recpt);
    void FUN_0051d49b(Player* recpt);
    void FUN_005188db();

    void FUN_0051cd89(const CString& name, Player* player);

    NetStru2* FUN_00518544(uint16_t player_id);
    void FUN_005170b6(NetStru2* arg1);

    void FUN_0051800f();
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
    LPSTR str;
    int32_t field_0x2b4;
    int32_t field_0x2b8;
    int32_t field_0x2bc;
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
};
ASSERT_OFFSET(CLlDriver, connection_sockets, 0x57c);
ASSERT_OFFSET(CLlDriver, critical_section, 0x7ec);
ASSERT_SIZE(CLlDriver, 0x914);




__pragma(pack(push, 1))
class Packet
{
public:
    static Packet Inst; //in asm 6b0c00
public:
    Packet(); //in asm 52681f
public:
    virtual ~Packet();
    virtual Packet* Duplicate();
    virtual void VMethod3(NetStru1*);
    virtual void VMethod4(NetStru1*);
    virtual uint32_t GetDataSize();// { return 1; }

public:

    uint8_t field_0x4;
    uint16_t field_0x5;
    uint16_t to_player_id;
    uint8_t id;
};
__pragma(pack(pop))

ASSERT_SIZE(Packet, 0xa);


__pragma(pack(push, 1))
class PacketJoin : public Packet
{
public:
    static PacketJoin Inst; //in asm 6ce560
public:
    PacketJoin(); //in asm 526c12
public:
    virtual ~PacketJoin();
    virtual Packet* Duplicate() override;
    virtual void VMethod3(NetStru1*) override;
    virtual void VMethod4(NetStru1*) override;
    virtual uint32_t GetDataSize() override;

public:
    uint8_t player_id;
    uint8_t token_id;
    uint8_t field_0xc;
    uint8_t flags;
    uint16_t name_len;
    char name[1024];
};
__pragma(pack(pop))

ASSERT_SIZE(PacketJoin, 0x410);
