#pragma once

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


class NetClientPeer;


__pragma(pack(push, 1))
class Packet {
public:
    static Packet Inst; // 6b0c00
public:
    Packet(); // 52681f (client: 515651)
    Packet(const Packet* src); // 526856 (client: 515688)
public:
    virtual ~Packet(); // server: none (client: 54a470) - inlined in 5799d0 on server
    virtual Packet* Duplicate(); // 579940 (client: 54a490)
    virtual void WriteTo(NetClientPeer*); // 526887 (client: 5156b9)
    virtual void ReadFrom(NetClientPeer*); // 5268b7 (client: 5156e9)
    virtual uint32_t GetDataSize(); // 5799c0 (client: 54a510) - { return 1; }

public:

    uint8_t order_mode;
    uint16_t from_player_id;
    uint16_t to_player_id;
    uint8_t id;
};
__pragma(pack(pop))

ASSERT_SIZE(Packet, 0xa);


__pragma(pack(push, 1))
class Packet2Dwords : public Packet {
public:
    static Packet2Dwords Inst; // 6d0788
public:
    Packet2Dwords(); // 526b44 (client: 515976)
    Packet2Dwords(const Packet2Dwords* src); // 526b77 (client: 5159a9)

public:
    virtual ~Packet2Dwords(); // 579a80 (client: 54a5d0)
    virtual Packet* Duplicate() override; // 57a210 (client: 54b0c0)
    virtual uint32_t GetDataSize() override; // 57a290 (client: 54b140)
public:
    union {
        int32_t param1;
        struct {
            uint8_t param1_b0;
            uint8_t param1_b1;
            uint8_t param1_b2;
            uint8_t param1_b3;
        };
    };

    union {
        int32_t param2;
        struct {
            uint8_t param2_b0;
            uint8_t param2_b1;
            uint8_t param2_b2;
            uint8_t param2_b3;
        };
    };
};
__pragma(pack(pop))

ASSERT_SIZE(Packet2Dwords, 0x12);



__pragma(pack(push, 1))
class PacketJoin : public Packet {
public:
    static PacketJoin Inst; // 6ce560

public:
    PacketJoin(); // 526c12 (client: 515aac)
    PacketJoin(const PacketJoin* src); // 526c52 (client: 515aec)
    PacketJoin(CString name); // 526d59 (client: 515bf3)

public:
    virtual ~PacketJoin(); // 526e4f (client: 515ce9)
    virtual Packet* Duplicate() override; // 57a390 (client: 54b240)
    virtual void WriteTo(NetClientPeer*) override; // 526ede (client: 515d77)
    virtual void ReadFrom(NetClientPeer*) override; // 526f1d (client: 515db6)
    virtual uint32_t GetDataSize() override; // 57a410 (client: 54b2c0)

public:
    union {
        struct {
            uint8_t player_id;
            uint8_t token_id;
            uint8_t color;
            uint8_t flags;
        };
        uint32_t __as_dword;
    };
    
    uint16_t name_len;
    char name[1024];
};
__pragma(pack(pop))

ASSERT_OFFSET(PacketJoin, player_id, 0xa);
ASSERT_OFFSET(PacketJoin, name, 0x10);
ASSERT_SIZE(PacketJoin, 0x410);




__pragma(pack(push, 1))
class PacketTerrain : public Packet {
public:
    static PacketTerrain Inst; // 6e9db0

public:
    PacketTerrain(); // 527346 (client: 516317)
    PacketTerrain(const PacketTerrain* src); // 527365 (client: 516336)

public:
    virtual ~PacketTerrain(); // 579ac0 (client: 54a610)
    virtual Packet* Duplicate() override; // 57aaa0 (client: 54b930)
    virtual void WriteTo(NetClientPeer*) override; // 5273a7 (client: 516378)
    virtual void ReadFrom(NetClientPeer*) override; // 5273ce (client: 51639f)
    virtual uint32_t GetDataSize() override; // 57ab20 (client: 54b9b0)

public:
    
    uint32_t count;
    uint16_t buf[256 * 256];
};
__pragma(pack(pop))

ASSERT_SIZE(PacketTerrain, 0x2000e);


// Unit-state update packet (static instance at 6D1180).
// Built incrementally by SendUnitUpdate; the flags_mask records which fields
// are present and the data[] buffer holds their values.
__pragma(pack(push, 1))
class PacketUnitUpdate : public Packet {
public:
    static PacketUnitUpdate Inst; // 6D1180
public:
    PacketUnitUpdate(); // 527987 (client: 516957)
    PacketUnitUpdate(const PacketUnitUpdate* src); // 5279a6 (client: 516976) - (vtable-only ctor used in Duplicate)
public:
    virtual ~PacketUnitUpdate(); // 579c00 (client: 54a750)
    virtual Packet* Duplicate() override; // 57ae70 (client: 54bd00)
    virtual void WriteTo(NetClientPeer*) override; // 5279c7 (client: 516997)
    virtual void ReadFrom(NetClientPeer*) override; // 527a6c (client: 516a3c)
    virtual uint32_t GetDataSize() override; // 57aef0 (client: 54bd80)
public:
    uint32_t data_offset;  // +0x0A  bytes already written into data[]
    uint32_t flags_size;    // +0x0E  (purpose unknown)
    uint16_t unit_id;      // +0x12  low 16 bits of the unit's building_id
    uint32_t flags_mask;   // +0x14  bitmask of fields that follow in data[]
    uint8_t  data[0x400];  // +0x18  packed field values

    // 527b25 (client: 516af5)
    void PutByte(uint32_t flag, uint8_t  val) {
        this->flags_mask |= flag;
        this->data[this->data_offset++] = val;
    }

    // 527b60 (client: 516b30)
    void PutWord(uint32_t flag, uint16_t val) {
        this->flags_mask |= flag;
        *reinterpret_cast<uint16_t*>(this->data + this->data_offset) = val;
        this->data_offset += 2;
    }

    // 527b9d (client: 516b6d)
    void PutInt(uint32_t flag, uint32_t val) {
        this->flags_mask |= flag;
        *reinterpret_cast<uint32_t*>(this->data + this->data_offset) = val;
        this->data_offset += 4;
    }

    uint32_t CalcFlagsSize(); // 527bd8 (client: 516ba8)
    uint32_t CalcDataSize(); // 527c3b (client: 516c0b)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitUpdate, 0x418);


// ---------------------------------------------------------------------------
// PacketUnitRef
// Constructor 5268FC, static 6E9B40, vtable 60ED28.
// Single uint16_t payload – typically a unit/object reference.
// No custom Duplicate (inherits base); no custom WriteTo/ReadFrom.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketWord : public Packet {
public:
    static PacketWord Inst; // 6E9B40
public:
    PacketWord(); // 5268fc (client: 51572e)
    PacketWord(const PacketWord* src); // C++ copy ctor (no retail twin)
public:
    virtual ~PacketWord(); // 579a00 (client: 54a550)
    virtual uint32_t GetDataSize() override; // 57a0a0 (client: 54af50)
public:
    uint16_t value; // +0xA  (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketWord, 0xC);


// ---------------------------------------------------------------------------
// PacketGroupOrder
// Constructor 526924, static 6D1670, vtable 60ED40.
// Variable-length list of uint16_t entries; up to 0xFD (253) entries.
// AppendWord() mirrors 52695A.  WriteTo sends count*2+0xA bytes from &id.
// ReadFrom reads 9 fixed bytes then count*2 variable bytes.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketGroupOrder : public Packet {
public:
    static PacketGroupOrder Inst; // 6D1670
public:
    PacketGroupOrder(); // 526924 (client: 515756)
    PacketGroupOrder(const PacketGroupOrder* src); // C++ copy ctor (no retail twin)
    void AppendWord(uint16_t val); // 52695a (client: 51578c)
public:
    virtual ~PacketGroupOrder(); // 579a20 (client: 54a570)
    virtual void WriteTo(NetClientPeer*) override; // 52699a (client: 5157cc)
    virtual void ReadFrom(NetClientPeer*) override; // 5269c3 (client: 5157f5)
    virtual uint32_t GetDataSize() override; // 57a0e0 (client: 54af90)
public:
    uint16_t x;       // +0xA
    uint8_t  y;       // +0xC
    uint8_t  y_high;       // +0xD
    uint16_t param;       // +0xE  (= 0 in ctor)
    uint8_t  slot;      // +0x10 (= 0 in ctor)
    uint8_t  slot_high;      // +0x11
    uint8_t  count;           // +0x12 (= 0 in ctor, max 0xFD)
    uint16_t entries[253];    // +0x13 variable word array
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketGroupOrder, entries, 0x13);
ASSERT_SIZE(PacketGroupOrder, 0x20d);


// ---------------------------------------------------------------------------
// PacketItemTransfer
// Constructor 526A07, static 6B1650, vtable 60ED58.
// Command packet: 2 type/subtype bytes + 3 word params; field_0x12 init = 1.
// Uses inherited Packet WriteTo/ReadFrom serialisation.
// ---------------------------------------------------------------------------
// PacketItemTransfer::type - where the item comes from.
// Values >= TRANSFER_SRC_SHOP are shop interactions; SHELF_* name the four
// multi-shop display shelves (shelf index = value - TRANSFER_SRC_SHELF_1).
enum TransferSource : uint8_t {
    TRANSFER_SRC_NONE    = 0, // no operation (ctor default)
    TRANSFER_SRC_UNEQUIP = 1, // unequip from body slot (param1 = body slot - 1)
    TRANSFER_SRC_BAG     = 2, // take from bag (param1 = bag position, param3 = item id)
    TRANSFER_SRC_SACK    = 3, // pick up ground sack
    TRANSFER_SRC_SHOP    = 4, // buy from the shop (shop table / desk)
    TRANSFER_SRC_SHELF_1 = 5, // shop shelf 1
    TRANSFER_SRC_SHELF_2 = 6, // shop shelf 2
    TRANSFER_SRC_SHELF_3 = 7, // shop shelf 3
    TRANSFER_SRC_SHELF_4 = 8, // shop shelf 4
};

// PacketItemTransfer::subtype - where the item goes.
// Same numeric layout as TransferSource but independent semantics
// (e.g. 4 = sell on the destination side, buy on the source side).
enum TransferDest : uint8_t {
    TRANSFER_DST_NONE    = 0, // no operation (ctor default)
    TRANSFER_DST_EQUIP   = 1, // equip the item
    TRANSFER_DST_BAG     = 2, // put into bag slot (param2 = bag position)
    TRANSFER_DST_GROUND  = 3, // drop on ground (creates a sack)
    TRANSFER_DST_SHOP    = 4, // sell to the shop
    TRANSFER_DST_SHELF_1 = 5, // stow on shop shelf 1 (server picks the shelf by item kind)
    TRANSFER_DST_SHELF_2 = 6, // stow on shop shelf 2
    TRANSFER_DST_SHELF_3 = 7, // stow on shop shelf 3
    TRANSFER_DST_SHELF_4 = 8, // stow on shop shelf 4
};
__pragma(pack(push, 1))
class PacketItemTransfer : public Packet {
public:
    static PacketItemTransfer Inst; // 6B1650
public:
    PacketItemTransfer(); // 526a07 (client: 515839)
    PacketItemTransfer(const PacketItemTransfer* src); // C++ copy ctor (no retail twin)
public:
    virtual ~PacketItemTransfer(); // 579a40 (client: 54a590)
    virtual uint32_t GetDataSize() override; // 57a130 (client: 54afe0)
public:
    uint16_t unit_id;     // +0xA
    TransferSource type;  // +0xC (= TRANSFER_SRC_NONE in ctor)
    TransferDest   subtype; // +0xD (= TRANSFER_DST_NONE in ctor)
    uint16_t param1;   // +0xE  (= 0 in ctor)
    uint16_t param2;  // +0x10 (= 0 in ctor)
    uint16_t param3;  // +0x12 (= 1 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketItemTransfer, 0x14);


// ---------------------------------------------------------------------------
// PacketEnterGame
// Constructor 526A4F, static 6B1670, vtable 60ED70.
// Three uint32_t fields followed by one uint8_t; all zeroed in ctor.
// Uses inherited Packet WriteTo/ReadFrom serialisation.
// ---------------------------------------------------------------------------
struct EnterGameParams {
    union {
        uint32_t raw;
        struct {
            uint32_t cd_flag    : 8; // [7:0]   g_IsCdPresent
            uint32_t color_slot : 8; // [15:8]  1..16 (or 0 = auto)
            uint32_t min_level  : 8; // [23:16] GetSkillTier1() / min_server_level
            uint32_t max_level  : 8; // [31:24] GetSkillTier2() / max_server_level
        };
    };

    constexpr EnterGameParams(uint32_t v = 0) : raw(v) {}
    constexpr operator uint32_t() const { return this->raw; }
};
ASSERT_SIZE(EnterGameParams, 4);

__pragma(pack(push, 1))
class PacketEnterGame : public Packet {
public:
    static PacketEnterGame Inst; // 6B1670
public:
    PacketEnterGame(); // 526a4f (client: 515881)
    PacketEnterGame(const PacketEnterGame* src); // C++ copy ctor (no retail twin)
public:
    virtual ~PacketEnterGame(); // 579a60 (client: 54a5b0)
    virtual uint32_t GetDataSize() override; // 57a170 (client: 54b020)
public:
    uint32_t value1;  // +0xA (= 0 in ctor)
    uint32_t value2;  // +0xE (= 0 in ctor)
    EnterGameParams value3; // +0x12 (= 0 in ctor)
    char  text[32]; // +0x16 (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEnterGame, 0x36);


// ---------------------------------------------------------------------------
// PacketDword
// Constructor 526BB0, static 6CDB78, vtable 60EDB8.
// Single uint32_t value payload; zeroed in ctor.
// Copy ctor: 526BD9 (copies 5 bytes: id + dword).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketDword : public Packet {
public:
    static PacketDword Inst; // 6CDB78
public:
    PacketDword(); // 526bb0 (client: 515a16)
    PacketDword(const PacketDword* src); // 526bd9 (client: 515a3f)
public:
    virtual ~PacketDword(); // 579aa0 (client: 54a5f0)
    virtual Packet* Duplicate() override; // 57a2d0 (client: 54b180)
    virtual uint32_t GetDataSize() override; // 57a350 (client: 54b200)
public:
    uint32_t value; // +0xA (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketDword, 0xE);


// ---------------------------------------------------------------------------
// PacketUnitAction
// Constructor 526F68, static 70A7D0, vtable 60EDE8.
// Ability/buff broadcast; id fixed to 0x6B ('k') by ctor.
// Copy ctor: 526F8E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitAction : public Packet {
public:
    static PacketUnitAction Inst; // 70A7D0
public:
    PacketUnitAction(); // 526f68 (client: 515e01) - sets id = 0x6B
    PacketUnitAction(const PacketUnitAction* src); // 526f8e (client: 515e27)
public:
    virtual ~PacketUnitAction(); // 579b00 (client: 54a650)
    virtual Packet* Duplicate() override; // 57a480 (client: 54b310)
    virtual uint32_t GetDataSize() override; // 57a500 (client: 54b390)
public:
    uint16_t unit_id; // +0xA
    uint8_t angle; // +0xC
    uint8_t segments; // +0xD
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitAction, 0xE);


// ---------------------------------------------------------------------------
// PacketSack
// Constructor 526FC7, static 6CE970, vtable 60EE00.
// Generic 8-byte payload packet; fields not initialised by ctor.
// Copy ctor: 526FE6 (copies 9 bytes: id + 8 payload bytes).
// ---------------------------------------------------------------------------
struct SackVisualType {
    union {
        uint8_t raw;
        struct {
            uint8_t gold_tier  : 6; // [5:0] (uint8_t)log10(sack->_exp)
            uint8_t has_rune_f : 1; // [6]   0x40 ("Quest RuneF")
            uint8_t has_rune_a : 1; // [7]   0x80 ("Quest RuneA")
        };
    };

    constexpr SackVisualType(uint8_t v = 0) : raw(v) {}
    constexpr operator uint8_t() const { return this->raw; }
};
ASSERT_SIZE(SackVisualType, 1);

__pragma(pack(push, 1))
class PacketSack : public Packet {
public:
    static PacketSack Inst; // 6CE970
public:
    PacketSack(); // 526fc7 (client: 515e94)
    PacketSack(const PacketSack* src); // 526fe6 (client: 515eb3)
public:
    virtual ~PacketSack(); // 579b20 (client: 54a670)
    virtual Packet* Duplicate() override; // 57a540 (client: 54b3d0)
    virtual uint32_t GetDataSize() override; // 57a5c0 (client: 54b450)
public:
    uint16_t unit_id; // 0xa
    uint8_t gap_c;
    SackVisualType type_id; // 0xd
    uint16_t xpos; // 0xe
    uint16_t ypos; // 0x10
};
__pragma(pack(pop))
ASSERT_SIZE(PacketSack, 0x12);


// ---------------------------------------------------------------------------
// PacketMoveBroadcast
// Constructor 52701F, static 6A6A70, vtable 60EE18.
// Movement / targeting command; id fixed to 0x86 by ctor.
// Bytes at +0xC/+0xD/+0xE zeroed by ctor; 0xA/0xB/0xF/0x10 set by caller.
// Copy ctor: 52705A (copies 8 bytes: id + 7 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketMoveBroadcast : public Packet {
public:
    static PacketMoveBroadcast Inst; // 6A6A70
public:
    PacketMoveBroadcast(); // 52701f (client: 515f20) - sets id = 0x86
    PacketMoveBroadcast(const PacketMoveBroadcast* src); // 52705a (client: 515f5b)
public:
    virtual ~PacketMoveBroadcast(); // 579b40 (client: 54a690)
    virtual Packet* Duplicate() override; // 57a600 (client: 54b490)
    virtual uint32_t GetDataSize() override; // 57a680 (client: 54b510)
public:
    uint16_t unit_id;  // +0xA
    uint8_t type_id;  // +0xC (= 0 in ctor)
    uint8_t x;  // +0xD (= 0 in ctor)
    uint8_t y;  // +0xE (= 0 in ctor)
    uint16_t segments;  // +0xF
};
__pragma(pack(pop))
ASSERT_SIZE(PacketMoveBroadcast, 0x11);


// ---------------------------------------------------------------------------
// PacketEffect
// Constructor 527093, static 6CDB28, vtable 60EE30.
// Effect-type notification; effect_type byte zeroed by ctor.
// Copy ctor: 5270B9 (copies 4 bytes: id + 3 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketEffect : public Packet {
public:
    static PacketEffect Inst; // 6CDB28
public:
    PacketEffect(); // 527093 (client: 515fc8)
    PacketEffect(const PacketEffect* src); // 5270b9 (client: 515fee)
public:
    virtual ~PacketEffect(); // 579b60 (client: 54a6b0)
    virtual Packet* Duplicate() override; // 57a6c0 (client: 54b550)
    virtual uint32_t GetDataSize() override; // 57a740 (client: 54b5d0)
public:
    uint16_t unit_id;    // +0xA
    uint8_t effect_type;  // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEffect, 0xD);


// ---------------------------------------------------------------------------
// PacketAoeZone
// Constructor 5270F2, static 6B37A8, vtable 60EE48.
// Area-of-effect zone info; id fixed to 0x87; bytes at +0xD/+0xE zeroed.
// Copy ctor: 527126 (copies 19 bytes: id + 18 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketAoeZone : public Packet {
public:
    static PacketAoeZone Inst; // 6B37A8
public:
    PacketAoeZone(); // 5270f2 (client: 51605b) - sets id = 0x87
    PacketAoeZone(const PacketAoeZone* src); // 527126 (client: 51608f)
public:
    virtual ~PacketAoeZone(); // 579b80 (client: 54a6d0)
    virtual Packet* Duplicate() override; // 57a780 (client: 54b610)
    virtual uint32_t GetDataSize() override; // 57a800 (client: 54b690)
public:
    uint8_t type_id;    // +0xA
    uint8_t x;    // +0xB; `x` coordinate of top-left corner of AoE zone
    uint8_t y;    // +0xC; `y` coordinate of top-left corner of AoE zone
    uint8_t width;    // +0xD (= 0 in ctor); width of AoE zone in cells
    uint8_t height;    // +0xE (= 0 in ctor); height of AoE zone in cells
    uint8_t set_flag;    // +0xF
    uint8_t data[12] ;    // +0x10..+0x1B
};
__pragma(pack(pop))
ASSERT_SIZE(PacketAoeZone, 0x1C);


// ---------------------------------------------------------------------------
// PacketRangedAttack
// Constructor 52715F, static 6B37C8, vtable 60EE60.
// Unit mount / ride packet; word unit_id zeroed by ctor.
// Copy ctor: 527187 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketRangedAttack : public Packet {
public:
    static PacketRangedAttack Inst; // 6B37C8
public:
    PacketRangedAttack(); // 52715f (client: 5160fc)
    PacketRangedAttack(const PacketRangedAttack* src); // 527187 (client: 516124)
public:
    virtual ~PacketRangedAttack(); // 579ba0 (client: 54a6f0)
    virtual Packet* Duplicate() override; // 57a840 (client: 54b6d0)
    virtual uint32_t GetDataSize() override; // 57a8c0 (client: 54b750)
public:
    uint16_t  unit_id; // +0xA
    uint16_t target_id;   // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketRangedAttack, 0xE);


// ---------------------------------------------------------------------------
// PacketFileChunk
// Constructor 5271C0, static 6D1888, vtable 60EE78.
// Large variable-length player-info packet.
// Fixed area: 40-byte preamble at +0xA, int32_t count at +0x32.
// Variable-length data starting at +0x36 with `count` bytes.
// WriteTo (527220): sends count+45 bytes from &id.
// ReadFrom (527246): reads 44-byte fixed block then optional count bytes.
// Copy ctor: 5271DF (memcpy id + preamble + count + var_data).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketFileChunk : public Packet {
public:
    static PacketFileChunk Inst; // 6D1888
public:
    PacketFileChunk(); // 5271c0 (client: 516191)
    PacketFileChunk(const PacketFileChunk* src); // 5271df (client: 5161b0)
public:
    virtual ~PacketFileChunk(); // 579c40 (client: 54a790)
    virtual Packet* Duplicate() override; // 57a900 (client: 54b790)
    virtual void WriteTo(NetClientPeer*) override; // 527220 (client: 5161f1)
    virtual void ReadFrom(NetClientPeer*) override; // 527246 (client: 516217)
    virtual uint32_t GetDataSize() override; // 57a980 (client: 54b810)
public:
    uint8_t  preamble[0x20]; // +0xA..+0x2b  (32 bytes of fixed player fields)
    uint32_t offset;         // +0x2a
    uint32_t total_length;   // +0x2e
    int32_t  count;        // +0x32  length of var_data
    uint8_t  var_data[0x10000];  // +0x36  variable-length data (actual size = count)
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketFileChunk, preamble,  0xA);
ASSERT_OFFSET(PacketFileChunk, offset,    0x2A);
ASSERT_OFFSET(PacketFileChunk, count,     0x32);
ASSERT_OFFSET(PacketFileChunk, var_data,  0x36);
ASSERT_SIZE(PacketFileChunk, 0x10036);


// ---------------------------------------------------------------------------
// PacketData
// Constructor 527283, static 6B37D8, vtable 60EE90.
// Generic length-prefixed binary blob: uint32_t count at +0xA, data at +0xE.
// WriteTo (5272E3): sends count+5 bytes from &id.
// ReadFrom (527309): reads 4-byte count, then count bytes of data.
// Copy ctor: 5272A2 (memcpy count+5 bytes from offset 9).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketData : public Packet {
public:
    static PacketData Inst; // 6B37D8
public:
    PacketData(); // 527283 (client: 516254)
    PacketData(const PacketData* src); // 5272a2 (client: 516273)
public:
    virtual ~PacketData(); // 579ae0 (client: 54a630)
    virtual Packet* Duplicate() override; // 57a9d0 (client: 54b860)
    virtual void WriteTo(NetClientPeer*) override; // 5272e3 (client: 5162b4)
    virtual void ReadFrom(NetClientPeer*) override; // 527309 (client: 5162da)
    virtual uint32_t GetDataSize() override; // 57aa50 (client: 54b8e0)
public:
    uint32_t count;    // +0xA  byte length of data[]
    uint8_t  data[0x10000];  // +0xE  variable-length payload (actual size = count)
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketData, count, 0xA);
ASSERT_OFFSET(PacketData, data,  0xE);
ASSERT_SIZE(PacketData, 0x1000e);

// PacketUnitStateVec::mode for ITEM_LIST packets. The client dispatches on
// (mode & 0x7f) in the ITEM_LIST handler; bit 0x80 is a partial-update flag.
enum ItemListMode : uint8_t {
    ITEM_LIST_EQUIPMENT    = 1, // equipment slots (start_index = slot-present flags)
    ITEM_LIST_INVENTORY    = 2, // bag inventory (mode | 0x80 = partial update at start_index)
    ITEM_LIST_SHOP_TABLE   = 4, // shop table: items the customer picked up (TokenEntry parse flag 1)
    ITEM_LIST_SHELF_1      = 5, // shop shelf 1
    ITEM_LIST_SHELF_2      = 6, // shop shelf 2
    ITEM_LIST_SHELF_3      = 7, // shop shelf 3
    ITEM_LIST_SHELF_4      = 8, // shop shelf 4
    ITEM_LIST_INN_REWARDS  = 9, // inn quest-reward inventory (client fills VisTav::rewards)
    ITEM_LIST_PARTIAL_UPDATE = 0x80, // mode bit: inventory update replaces only [start_index, start_index+entry_count)
};
// Convenience bit operators.
inline ItemListMode operator|(ItemListMode a, ItemListMode b) { return (ItemListMode)((uint8_t)a | (uint8_t)b); }
inline ItemListMode& operator|=(ItemListMode& a, ItemListMode b) { a = a | b; return a; }


// ---------------------------------------------------------------------------
// PacketUnitStateVec
// Constructor 527415, static 6C5AF8, vtable 60EEC0.
// Variable-length TLV-encoded unit state vector; id fixed to 0x76 ('v').
// entry_count: number of property entries; data_size: byte length of data[].
// Each entry: 1–5 bytes (type tag + 1/2/4-byte value), added by helpers.
// Helper 527574  : opens a 7-byte compound entry slot.
// Helper 5275F8  : appends (uint8_t field, uint8_t value) – 2 bytes.
// Helper 5276E0  : appends (uint8_t field, uint32_t value) – 5 bytes.
// WriteTo (5274CC): if entry_count==0 sends 10 bytes; else also sends data.
// ReadFrom (527528): reads 9 fixed bytes at +0xA, then data_size bytes.
// Copy ctor: 527473.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitStateVec : public Packet {
public:
    static PacketUnitStateVec Inst; // 6C5AF8
public:
    PacketUnitStateVec(); // 527415 (client: 5163e6) - sets id = 0x76
    PacketUnitStateVec(const PacketUnitStateVec* src); // 527473 (client: 516443)
public:
    virtual ~PacketUnitStateVec(); // 527456 (client: 516427)
    virtual Packet* Duplicate() override; // 57ab70 (client: 54ba00)
    virtual void WriteTo(NetClientPeer*) override; // 5274cc (client: 51649c)
    virtual void ReadFrom(NetClientPeer*) override; // 527528 (client: 5164f8)
    virtual uint32_t GetDataSize() override; // 57abf0 (client: 54ba80)
    void AppendByteByte(uint8_t a, uint8_t b, uint8_t* slot); // 5275f8 (client: 5165c8)
    void AppendByteInt(uint8_t a, int32_t v, uint8_t* slot); // 5276e0 (client: 5166b0)
public:
    uint16_t building_id;    // +0xA
    ItemListMode mode; // +0xC
    uint16_t entry_count;  // +0xD  number of TLV entries (= 0 in ctor)
    uint16_t start_index;    // +0xF  (= 0 in ctor)
    uint16_t data_size;    // +0x11 byte length of TLV data (= 0 in ctor)
    uint8_t  data[0x8000]; // +0x13 TLV-encoded property stream
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketUnitStateVec, entry_count, 0xD);
ASSERT_OFFSET(PacketUnitStateVec, data_size,   0x11);
ASSERT_OFFSET(PacketUnitStateVec, data,        0x13);
ASSERT_SIZE(PacketUnitStateVec, 0x8013);


// ---------------------------------------------------------------------------
// PacketBuildingHp
// Constructor 527810, static 6A6A88, vtable 60EED8.
// Unit visibility-sync / broadcast; id fixed to 0x82 by ctor.
// Copy ctor: 527836 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketBuildingHp : public Packet {
public:
    static PacketBuildingHp Inst; // 6A6A88
public:
    PacketBuildingHp(); // 527810 (client: 5167e0) - sets id = 0x82
    PacketBuildingHp(const PacketBuildingHp* src); // 527836 (client: 516806)
public:
    virtual ~PacketBuildingHp(); // 579bc0 (client: 54a710)
    virtual Packet* Duplicate() override; // 57ac40 (client: 54bad0)
    virtual uint32_t GetDataSize() override; // 57acc0 (client: 54bb50)
public:
    uint16_t building_id; // +0xA
    int16_t hp; // +0xC
};
__pragma(pack(pop))
ASSERT_SIZE(PacketBuildingHp, 0xE);


// ---------------------------------------------------------------------------
// PacketUnitEquipment
// Constructor 52786F, static 6D0628, vtable 60EEF0.
// Variable-size player/unit property flags packet; id forced to 0 by ctor.
// flags (word at +0xC): bitmask of up to 12 included uint16_t fields.
// GetDataSize (5278CE) = 0x1D - (12 - popcount(flags))*2  (5..29 bytes).
// Copy ctor: 527895 (copies max 29 bytes: id + 28 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitEquipment : public Packet {
public:
    static PacketUnitEquipment Inst; // 6D0628
public:
    PacketUnitEquipment(); // 52786f (client: 51683f) - sets id = 0
    PacketUnitEquipment(const PacketUnitEquipment* src); // 527895 (client: 516865)
public:
    virtual ~PacketUnitEquipment(); // 579c20 (client: 54a770)
    virtual Packet* Duplicate() override; // 57ad00 (client: 54bb90)
    virtual void ReadFrom(NetClientPeer* net) override;
    virtual uint32_t GetDataSize() override; // 5278ce (client: 51689e) - variable (5.29)
public:
    uint16_t unit_id;       // +0xA
    uint16_t flags;           // +0xC  bitmask of included fields (bits 0..11)
    uint16_t prop[12];        // +0xE  up to 12 optional property words
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitEquipment, 0x26);


// ---------------------------------------------------------------------------
// PacketUnitHp
// Constructor 52792F, static 6CDB88, vtable 60EF08.
// Network ping / state packet; 4 payload bytes after Packet base.
// Copy ctor: 52794E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitHp : public Packet {
public:
    static PacketUnitHp Inst; // 6CDB88
public:
    PacketUnitHp(); // 52792f (client: 5168ff)
    PacketUnitHp(const PacketUnitHp* src); // 52794e (client: 51691e)
public:
    virtual ~PacketUnitHp(); // 579be0 (client: 54a730)
    virtual Packet* Duplicate() override; // 57adb0 (client: 54bc40)
    virtual uint32_t GetDataSize() override; // 57ae30 (client: 54bcc0)
public:
    uint16_t unit_id; // +0xA
    int16_t hp; // +0xC
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitHp, 0xE);
