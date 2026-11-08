#pragma once

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>


class NetStru2;


__pragma(pack(push, 1))
class Packet {
public:
    static Packet Inst; //in asm 6b0c00
public:
    Packet(); //in asm 52681f (client: 515651)
    Packet(const Packet* src);  // client: 515688
public:
    virtual ~Packet();
    virtual Packet* Duplicate();  // client: 54bc40
    virtual void WriteTo(NetStru2*);  // client: 5156b9
    virtual void ReadFrom(NetStru2*);  // client: 5156e9
    virtual uint32_t GetDataSize();// { return 1; } (client: 54a510)

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
    static Packet2Dwords Inst; //in asm 6d0788
public:
    Packet2Dwords();  // client: 515881
    Packet2Dwords(const Packet2Dwords* src);  // client: 516806

public:
    virtual ~Packet2Dwords();  // client: 4019b0
    virtual Packet* Duplicate() override;  // client: 54a490
    virtual uint32_t GetDataSize() override;  // client: 54b020
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
    static PacketJoin Inst; //in asm 6ce560

public:
    PacketJoin(); // in asm 526c12 (client: 515aac)
    PacketJoin(const PacketJoin* src);  // client: 515aec
    PacketJoin(CString name); // 526d59 (client: 515bf3)

public:
    virtual ~PacketJoin();  // client: 515ce9
    virtual Packet* Duplicate() override;  // client: 54b240
    virtual void WriteTo(NetStru2*) override;  // client: 515d77
    virtual void ReadFrom(NetStru2*) override;  // client: 515db6
    virtual uint32_t GetDataSize() override;  // client: 54b2c0

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
    static PacketTerrain Inst; //in asm 6e9db0

public:
    PacketTerrain(); //527346 (client: 516317)
    PacketTerrain(const PacketTerrain* src); //527365 (client: 516336)

public:
    virtual ~PacketTerrain();  // client: 54a610
    virtual Packet* Duplicate() override;  // client: 54b930
    virtual void WriteTo(NetStru2*) override;  // client: 516378
    virtual void ReadFrom(NetStru2*) override;  // client: 51639f
    virtual uint32_t GetDataSize() override;  // client: 54b9b0

public:
    
    uint32_t count;
    uint16_t buf[256 * 256];
};
__pragma(pack(pop))

ASSERT_SIZE(PacketTerrain, 0x2000e);


// Unit-state update packet (static instance at _unk_6D1180).
// Built incrementally by SendUnitUpdate; the flags_mask records which fields
// are present and the data[] buffer holds their values.
__pragma(pack(push, 1))
class PacketUnitUpdate : public Packet {
public:
    static PacketUnitUpdate Inst; // _unk_6D1180
public:
    PacketUnitUpdate(); // 527987 (client: 516957)
    PacketUnitUpdate(const PacketUnitUpdate* src); // 5279A6 (vtable-only ctor used in Duplicate) (client: 516976)
public:
    virtual ~PacketUnitUpdate();  // client: 54bda0
    virtual Packet* Duplicate() override; // 57AE70 (client: 54bd00)
    virtual void WriteTo(NetStru2*) override; // 5279C7 (client: 516997)
    virtual void ReadFrom(NetStru2*) override; // 527A6C (client: 516a3c)
    virtual uint32_t GetDataSize() override; // 57AEF0 (client: 54bd80)
public:
    uint32_t data_offset;  // +0x0A  bytes already written into data[]
    uint32_t flags_size;    // +0x0E  (purpose unknown)
    uint16_t unit_id;      // +0x12  low 16 bits of the unit's building_id
    uint32_t flags_mask;   // +0x14  bitmask of fields that follow in data[]
    uint8_t  data[0x400];  // +0x18  packed field values

    // Helpers mirroring 527B25 / 527B60 / 527B9D in asm
    void PutByte(uint32_t flag, uint8_t  val) { this->flags_mask |= flag; this->data[this->data_offset++] = val; }
    void PutWord(uint32_t flag, uint16_t val) { this->flags_mask |= flag; *reinterpret_cast<uint16_t*>(this->data + this->data_offset) = val; this->data_offset += 2; }
    void PutInt(uint32_t flag, uint32_t val){ this->flags_mask |= flag; *reinterpret_cast<uint32_t*>(this->data + this->data_offset) = val; this->data_offset += 4; }

    uint32_t CalcFlagsSize();
    uint32_t CalcDataSize();
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitUpdate, 0x418);


// ---------------------------------------------------------------------------
// PacketUnitRef
// Constructor 5268FC, static unk_6E9B40, vtable off_60ED28.
// Single uint16_t payload – typically a unit/object reference.
// No custom Duplicate (inherits base); no custom VMethod3/VMethod4.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketWord : public Packet {
public:
    static PacketWord Inst; // unk_6E9B40
public:
    PacketWord(); // 5268FC (client: 515651)
    PacketWord(const PacketWord* src);  // client: 515651
public:
    virtual ~PacketWord();  // client: 54a520
    virtual uint32_t GetDataSize() override; // 57A0A0 (client: 54a510)
public:
    uint16_t value; // +0xA  (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketWord, 0xC);


// ---------------------------------------------------------------------------
// PacketGroupOrder
// Constructor 526924, static unk_6D1670, vtable off_60ED40.
// Variable-length list of uint16_t entries; up to 0xFD (253) entries.
// AppendWord() mirrors 52695A.  VMethod3 sends count*2+0xA bytes from &id.
// VMethod4 reads 9 fixed bytes then count*2 variable bytes.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketGroupOrder : public Packet {
public:
    static PacketGroupOrder Inst; // unk_6D1670
public:
    PacketGroupOrder(); // 526924 (client: 515756)
    PacketGroupOrder(const PacketGroupOrder* src);  // client: 515756
    void AppendWord(uint16_t val); // 52695A (client: 51578c)
public:
    virtual ~PacketGroupOrder();  // client: 54afb0
    virtual void WriteTo(NetStru2*) override; // 52699A (client: 5157cc)
    virtual void ReadFrom(NetStru2*) override; // 5269C3 (client: 5157f5)
    virtual uint32_t GetDataSize() override;   // 57A0E0 (client: 54af90)
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
// Constructor 526A07, static unk_6B1650, vtable off_60ED58.
// Command packet: 2 type/subtype bytes + 3 word params; field_0x12 init = 1.
// Uses inherited Packet VMethod3/VMethod4 serialisation.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketItemTransfer : public Packet {
public:
    static PacketItemTransfer Inst; // unk_6B1650
public:
    PacketItemTransfer(); // 526A07 (client: 51572e)
    PacketItemTransfer(const PacketItemTransfer* src);  // client copy ctor: no twin found (client: 51572e)
public:
    virtual ~PacketItemTransfer();  // client: 54af60
    virtual uint32_t GetDataSize() override; // 57A130 (client: 54af50)
public:
    uint16_t unit_id;     // +0xA
    uint8_t  type;        // +0xC (= 0 in ctor)
    uint8_t  subtype;     // +0xD (= 0 in ctor)
    uint16_t param1;   // +0xE  (= 0 in ctor)
    uint16_t param2;  // +0x10 (= 0 in ctor)
    uint16_t param3;  // +0x12 (= 1 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketItemTransfer, 0x14);


// ---------------------------------------------------------------------------
// PacketEnterGame
// Constructor 526A4F, static unk_6B1670, vtable off_60ED70.
// Three uint32_t fields followed by one uint8_t; all zeroed in ctor.
// Uses inherited Packet VMethod3/VMethod4 serialisation.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketEnterGame : public Packet {
public:
    static PacketEnterGame Inst; // unk_6B1670
public:
    PacketEnterGame(); // 526A4F (client: 515839)
    PacketEnterGame(const PacketEnterGame* src);  // client copy ctor: no twin found (client: 515839)
public:
    virtual ~PacketEnterGame();  // client: 54aff0
    virtual uint32_t GetDataSize() override; // 57A170 (client: 54afe0)
public:
    uint32_t value1;  // +0xA (= 0 in ctor)
    uint32_t value2;  // +0xE (= 0 in ctor)
    uint32_t value3; // +0x12 (= 0 in ctor)
    char  text[32]; // +0x16 (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEnterGame, 0x36);


// ---------------------------------------------------------------------------
// PacketDword
// Constructor 526BB0, static unk_6CDB78, vtable off_60EDB8.
// Single uint32_t value payload; zeroed in ctor.
// Copy ctor: 526BD9 (copies 5 bytes: id + dword).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketDword : public Packet {
public:
    static PacketDword Inst; // unk_6CDB78
public:
    PacketDword(); // 526BB0 (client: 515976)
    PacketDword(const PacketDword* src); // 526BD9 (client: 516806)
public:
    virtual ~PacketDword();  // client: 54b150
    virtual Packet* Duplicate() override; // 57A2D0 (client: 54b0c0)
    virtual uint32_t GetDataSize() override;  // client: 54b140
public:
    uint32_t value; // +0xA (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketDword, 0xE);


// ---------------------------------------------------------------------------
// PacketUnitAction
// Constructor 526F68, static unk_70A7D0, vtable off_60EDE8.
// Ability/buff broadcast; id fixed to 0x6B ('k') by ctor.
// Copy ctor: 526F8E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitAction : public Packet {
public:
    static PacketUnitAction Inst; // unk_70A7D0
public:
    PacketUnitAction(); // 526F68  sets id = 0x6B (client: 515a16)
    PacketUnitAction(const PacketUnitAction* src); // 526F8E (client: 515a3f)
public:
    virtual ~PacketUnitAction();  // client: 54b210
    virtual Packet* Duplicate() override; // 57A480 (client: 54b180)
    virtual uint32_t GetDataSize() override;
public:
    uint16_t unit_id; // +0xA
    uint8_t angle; // +0xC
    uint8_t segments; // +0xD
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitAction, 0xE);


// ---------------------------------------------------------------------------
// PacketSack
// Constructor 526FC7, static unk_6CE970, vtable off_60EE00.
// Generic 8-byte payload packet; fields not initialised by ctor.
// Copy ctor: 526FE6 (copies 9 bytes: id + 8 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketSack : public Packet {
public:
    static PacketSack Inst; // unk_6CE970
public:
    PacketSack(); // 526FC7 (client: 515e01)
    PacketSack(const PacketSack* src); // 526FE6 (client: 515e27)
public:
    virtual ~PacketSack();  // client: 54b3a0
    virtual Packet* Duplicate() override; // 57A540 (client: 54b310)
    virtual uint32_t GetDataSize() override; // 57A5C0 (client: 54b390)
public:
    uint16_t unit_id; // 0xa
    uint8_t gap_c;
    uint8_t type_id; // 0xd
    int16_t xpos; // 0xe
    int16_t ypos; // 0x10
};
__pragma(pack(pop))
ASSERT_SIZE(PacketSack, 0x12);


// ---------------------------------------------------------------------------
// PacketMoveBroadcast
// Constructor 52701F, static unk_6A6A70, vtable off_60EE18.
// Movement / targeting command; id fixed to 0x86 by ctor.
// Bytes at +0xC/+0xD/+0xE zeroed by ctor; 0xA/0xB/0xF/0x10 set by caller.
// Copy ctor: 52705A (copies 8 bytes: id + 7 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketMoveBroadcast : public Packet {
public:
    static PacketMoveBroadcast Inst; // unk_6A6A70
public:
    PacketMoveBroadcast(); // 52701F  sets id = 0x86 (client: 515e94)
    PacketMoveBroadcast(const PacketMoveBroadcast* src); // 52705A (client: 515eb3)
public:
    virtual ~PacketMoveBroadcast();  // client: 54b460
    virtual Packet* Duplicate() override; // 57A600 (client: 54b3d0)
    virtual uint32_t GetDataSize() override; // 57A680 (client: 54b450)
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
// Constructor 527093, static unk_6CDB28, vtable off_60EE30.
// Effect-type notification; effect_type byte zeroed by ctor.
// Copy ctor: 5270B9 (copies 4 bytes: id + 3 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketEffect : public Packet {
public:
    static PacketEffect Inst; // unk_6CDB28
public:
    PacketEffect(); // 527093 (client: 515f20)
    PacketEffect(const PacketEffect* src); // 5270B9 (client: 515f5b)
public:
    virtual ~PacketEffect();  // client: 54b520
    virtual Packet* Duplicate() override; // 57A6C0 (client: 54b490)
    virtual uint32_t GetDataSize() override;
public:
    uint16_t unit_id;    // +0xA
    uint8_t effect_type;  // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEffect, 0xD);


// ---------------------------------------------------------------------------
// PacketAoeZone
// Constructor 5270F2, static unk_6B37A8, vtable off_60EE48.
// Area-of-effect zone info; id fixed to 0x87; bytes at +0xD/+0xE zeroed.
// Copy ctor: 527126 (copies 19 bytes: id + 18 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketAoeZone : public Packet {
public:
    static PacketAoeZone Inst; // unk_6B37A8
public:
    PacketAoeZone(); // 5270F2  sets id = 0x87 (client: 51605b)
    PacketAoeZone(const PacketAoeZone* src); // 527126 (client: 515fee)
public:
    virtual ~PacketAoeZone();  // client: 54b5e0
    virtual Packet* Duplicate() override; // 57A780 (client: 54b550)
    virtual uint32_t GetDataSize() override; // 57A800 (client: 54b5d0)
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
// Constructor 52715F, static unk_6B37C8, vtable off_60EE60.
// Unit mount / ride packet; word unit_id zeroed by ctor.
// Copy ctor: 527187 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketRangedAttack : public Packet {
public:
    static PacketRangedAttack Inst; // unk_6B37C8
public:
    PacketRangedAttack(); // 52715F (client: 51605b)
    PacketRangedAttack(const PacketRangedAttack* src); // 527187 (client: 51608f)
public:
    virtual ~PacketRangedAttack();  // client: 54b6a0
    virtual Packet* Duplicate() override; // 57A840 (client: 54b610)
    virtual uint32_t GetDataSize() override;
public:
    uint16_t  unit_id; // +0xA
    uint16_t target_id;   // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketRangedAttack, 0xE);


// ---------------------------------------------------------------------------
// PacketFileChunk
// Constructor 5271C0, static unk_6D1888, vtable off_60EE78.
// Large variable-length player-info packet.
// Fixed area: 40-byte preamble at +0xA, int32_t count at +0x32.
// Variable-length data starting at +0x36 with `count` bytes.
// VMethod3 (527220): sends count+45 bytes from &id.
// VMethod4 (527246): reads 44-byte fixed block then optional count bytes.
// Copy ctor: 5271DF (memcpy id + preamble + count + var_data).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketFileChunk : public Packet {
public:
    static PacketFileChunk Inst; // unk_6D1888
public:
    PacketFileChunk(); // 5271C0 (client: 516191)
    PacketFileChunk(const PacketFileChunk* src); // 5271DF (client: 5161b0)
public:
    virtual ~PacketFileChunk();  // client: 54b830
    virtual Packet* Duplicate() override;     // 57A900 (client: 54b790)
    virtual void WriteTo(NetStru2*) override; // 527220 (client: 5161f1)
    virtual void ReadFrom(NetStru2*) override; // 527246 (client: 516217)
    virtual uint32_t GetDataSize() override;   // 57A980 (client: 54b810)
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
// Constructor 527283, static unk_6B37D8, vtable off_60EE90.
// Generic length-prefixed binary blob: uint32_t count at +0xA, data at +0xE.
// VMethod3 (5272E3): sends count+5 bytes from &id.
// VMethod4 (527309): reads 4-byte count, then count bytes of data.
// Copy ctor: 5272A2 (memcpy count+5 bytes from offset 9).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketData : public Packet {
public:
    static PacketData Inst; // unk_6B37D8
public:
    PacketData(); // 527283 (client: 516254)
    PacketData(const PacketData* src); // 5272A2 (client: 516273)
public:
    virtual ~PacketData();  // client: 54b900
    virtual Packet* Duplicate() override;     // 57A9D0 (client: 54b860)
    virtual void WriteTo(NetStru2*) override; // 5272E3 (client: 5162b4)
    virtual void ReadFrom(NetStru2*) override; // 527309 (client: 5162da)
    virtual uint32_t GetDataSize() override;   // 57AA50 (client: 54b8e0)
public:
    uint32_t count;    // +0xA  byte length of data[]
    uint8_t  data[0x10000];  // +0xE  variable-length payload (actual size = count)
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketData, count, 0xA);
ASSERT_OFFSET(PacketData, data,  0xE);
ASSERT_SIZE(PacketData, 0x1000e);


// ---------------------------------------------------------------------------
// PacketUnitStateVec
// Constructor 527415, static unk_6C5AF8, vtable off_60EEC0.
// Variable-length TLV-encoded unit state vector; id fixed to 0x76 ('v').
// entry_count: number of property entries; data_size: byte length of data[].
// Each entry: 1–5 bytes (type tag + 1/2/4-byte value), added by helpers.
// Helper 527574  : opens a 7-byte compound entry slot.
// Helper 5275F8  : appends (uint8_t field, uint8_t value) – 2 bytes.
// Helper 5276E0  : appends (uint8_t field, uint32_t value) – 5 bytes.
// VMethod3 (5274CC): if entry_count==0 sends 10 bytes; else also sends data.
// VMethod4 (527528): reads 9 fixed bytes at +0xA, then data_size bytes.
// Copy ctor: 527473.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitStateVec : public Packet {
public:
    static PacketUnitStateVec Inst; // unk_6C5AF8
public:
    PacketUnitStateVec(); // 527415  sets id = 0x76 (client: 5163e6)
    PacketUnitStateVec(const PacketUnitStateVec* src); // 527473 (client: 516443)
public:
    virtual ~PacketUnitStateVec();  // client: 54baa0
    virtual Packet* Duplicate() override;     // 57AB70 (client: 54ba00)
    virtual void WriteTo(NetStru2*) override; // 5274CC (client: 51649c)
    virtual void ReadFrom(NetStru2*) override; // 527528 (client: 5164f8)
    virtual uint32_t GetDataSize() override;   // 57ABF0 (client: 54ba80)
    void AppendByteByte(uint8_t a, uint8_t b, uint8_t* slot); // 5275F8 (client: 5165c8)
    void AppendByteInt(uint8_t a, int32_t v, uint8_t* slot);  // 5276E0 (client: 5165c8)
public:
    uint16_t building_id;    // +0xA
    uint8_t  mode;    // +0xC
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
// Constructor 527810, static unk_6A6A88, vtable off_60EED8.
// Unit visibility-sync / broadcast; id fixed to 0x82 by ctor.
// Copy ctor: 527836 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketBuildingHp : public Packet {
public:
    static PacketBuildingHp Inst; // unk_6A6A88
public:
    PacketBuildingHp(); // 527810  sets id = 0x82 (client: 5160fc)
    PacketBuildingHp(const PacketBuildingHp* src); // 527836 (client: 516806)
public:
    virtual ~PacketBuildingHp();  // client: 54b760
    virtual Packet* Duplicate() override; // 57AC40 (client: 54b6d0)
    virtual uint32_t GetDataSize() override;
public:
    uint16_t building_id; // +0xA
    int16_t hp; // +0xC
};
__pragma(pack(pop))
ASSERT_SIZE(PacketBuildingHp, 0xE);


// ---------------------------------------------------------------------------
// PacketUnitEquipment
// Constructor 52786F, static unk_6D0628, vtable off_60EEF0.
// Variable-size player/unit property flags packet; id forced to 0 by ctor.
// flags (word at +0xC): bitmask of up to 12 included uint16_t fields.
// GetDataSize (5278CE) = 0x1D - (12 - popcount(flags))*2  (5..29 bytes).
// Copy ctor: 527895 (copies max 29 bytes: id + 28 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitEquipment : public Packet {
public:
    static PacketUnitEquipment Inst; // unk_6D0628
public:
    PacketUnitEquipment(); // 52786F  sets id = 0 (client: 51683f)
    PacketUnitEquipment(const PacketUnitEquipment* src); // 527895 (client: 516865)
public:
    virtual ~PacketUnitEquipment();  // client: 54bc10
    virtual Packet* Duplicate() override; // 57AD00 (client: 54bb90)
    virtual uint32_t GetDataSize() override; // 5278CE  variable (5..29) (client: 51689e)
public:
    uint16_t unit_id;       // +0xA
    uint16_t flags;           // +0xC  bitmask of included fields (bits 0..11)
    uint16_t prop[12];        // +0xE  up to 12 optional property words
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitEquipment, 0x26);


// ---------------------------------------------------------------------------
// PacketUnitHp
// Constructor 52792F, static unk_6CDB88, vtable off_60EF08.
// Network ping / state packet; 4 payload bytes after Packet base.
// Copy ctor: 52794E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitHp : public Packet {
public:
    static PacketUnitHp Inst; // unk_6CDB88
public:
    PacketUnitHp(); // 52792F (client: 5167e0)
    PacketUnitHp(const PacketUnitHp* src); // 52794E (client: 516806)
public:
    virtual ~PacketUnitHp();  // client: 54bb60
    virtual Packet* Duplicate() override; // 57ADB0 (client: 54bad0)
    virtual uint32_t GetDataSize() override; // 57AE30 (client: 54bb50)
public:
    uint16_t unit_id; // +0xA
    int16_t hp; // +0xC
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitHp, 0xE);
