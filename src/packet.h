#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"


class NetStru2;


__pragma(pack(push, 1))
class Packet
{
public:
    static Packet Inst; //in asm 6b0c00
public:
    Packet(); //in asm 52681f
    Packet(const Packet* src);
public:
    virtual ~Packet();
    virtual Packet* Duplicate();
    virtual void VMethod3(NetStru2*);
    virtual void VMethod4(NetStru2*);
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
class PacketInfo : public Packet
{
public:
    static PacketInfo Inst; //in asm 6d0788
public:
    PacketInfo();
    PacketInfo(const PacketInfo* src);

public:
    virtual ~PacketInfo();
    virtual Packet* Duplicate() override;
    virtual uint32_t GetDataSize() override;
public:
    int32_t field_0xa;
    int32_t field_0xe;
};
__pragma(pack(pop))

ASSERT_SIZE(PacketInfo, 0x12);



__pragma(pack(push, 1))
class PacketJoin : public Packet
{
public:
    static PacketJoin Inst; //in asm 6ce560

public:
    PacketJoin(); // in asm 526c12
    PacketJoin(const PacketJoin* src);
    PacketJoin(CString name); // 526d59

public:
    virtual ~PacketJoin();
    virtual Packet* Duplicate() override;
    virtual void VMethod3(NetStru2*) override;
    virtual void VMethod4(NetStru2*) override;
    virtual uint32_t GetDataSize() override;

public:
    union {
        struct {
            uint8_t player_id;
            uint8_t token_id;
            uint8_t field_0xc;
            uint8_t flags;
        };
        uint32_t __field_0xa;
    };
    
    uint16_t name_len;
    char name[1024];
};
__pragma(pack(pop))

ASSERT_OFFSET(PacketJoin, player_id, 0xa);
ASSERT_SIZE(PacketJoin, 0x410);




__pragma(pack(push, 1))
class PacketTerrain : public Packet
{
public:
    static PacketTerrain Inst; //in asm 6e9db0

public:
    PacketTerrain(); //527346
    PacketTerrain(const PacketTerrain* src); //527365

public:
    virtual ~PacketTerrain();
    virtual Packet* Duplicate() override;
    virtual void VMethod3(NetStru2*) override;
    virtual void VMethod4(NetStru2*) override;
    virtual uint32_t GetDataSize() override;

public:
    
    uint32_t count;
    uint16_t buf[256 * 256];
};
__pragma(pack(pop))

ASSERT_SIZE(PacketTerrain, 0x2000e);


// Unit-state update packet (static instance at _unk_6D1180).
// Built incrementally by sub_519221; the flags_mask records which fields
// are present and the data[] buffer holds their values.
__pragma(pack(push, 1))
class PacketUnitUpdate : public Packet
{
public:
    static PacketUnitUpdate Inst; // _unk_6D1180
public:
    PacketUnitUpdate(); // sub_527987
    PacketUnitUpdate(const PacketUnitUpdate* src); // sub_5279A6 (vtable-only ctor used in Duplicate)
public:
    virtual ~PacketUnitUpdate();
    virtual Packet* Duplicate() override; // sub_57AE70
    virtual void VMethod3(NetStru2*) override; // sub_5279C7
    virtual void VMethod4(NetStru2*) override; // sub_527A6C
    virtual uint32_t GetDataSize() override; // sub_57AEF0
public:
    uint32_t data_offset;  // +0x0A  bytes already written into data[]
    uint32_t field_0xe;    // +0x0E  (purpose unknown)
    uint16_t unit_id;      // +0x12  low 16 bits of the unit's building_id
    uint32_t flags_mask;   // +0x14  bitmask of fields that follow in data[]
    uint8_t  data[0x400];  // +0x18  packed field values

    // Helpers mirroring sub_527B25 / sub_527B60 / sub_527B9D in asm
    void PutByte(uint32_t flag, uint8_t  val) { flags_mask |= flag; data[data_offset++] = val; }
    void PutWord(uint32_t flag, uint16_t val) { flags_mask |= flag; *reinterpret_cast<uint16_t*>(data + data_offset) = val; data_offset += 2; }
    void PutInt(uint32_t flag, uint32_t val){ flags_mask |= flag; *reinterpret_cast<uint32_t*>(data + data_offset) = val; data_offset += 4; }

    uint32_t FUN_00527bd8();
    uint32_t FUN_00527c3b();
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitUpdate, 0x418);


// ---------------------------------------------------------------------------
// PacketUnitRef
// Constructor sub_5268FC, static unk_6E9B40, vtable off_60ED28.
// Single uint16_t payload – typically a unit/object reference.
// No custom Duplicate (inherits base); no custom VMethod3/VMethod4.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketWord : public Packet
{
public:
    static PacketWord Inst; // unk_6E9B40
public:
    PacketWord(); // sub_5268FC
    PacketWord(const PacketWord* src);
public:
    virtual ~PacketWord();
    virtual uint32_t GetDataSize() override; // sub_57A0A0
public:
    uint16_t value; // +0xA  (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketWord, 0xC);


// ---------------------------------------------------------------------------
// PacketItemOperation
// Constructor sub_526924, static unk_6D1670, vtable off_60ED40.
// Variable-length list of uint16_t entries; up to 0xFD (253) entries.
// AppendWord() mirrors sub_52695A.  VMethod3 sends count*2+0xA bytes from &id.
// VMethod4 reads 9 fixed bytes then count*2 variable bytes.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketItemOperation : public Packet
{
public:
    static PacketItemOperation Inst; // unk_6D1670
public:
    PacketItemOperation(); // sub_526924
    PacketItemOperation(const PacketItemOperation* src);
    void AppendWord(uint16_t val); // sub_52695A
public:
    virtual ~PacketItemOperation();
    virtual void VMethod3(NetStru2*) override; // sub_52699A
    virtual void VMethod4(NetStru2*) override; // sub_5269C3
    virtual uint32_t GetDataSize() override;   // sub_57A0E0
public:
    uint16_t field_0xa;       // +0xA
    uint8_t  field_0xc;       // +0xC
    uint8_t  field_0xd;       // +0xD
    uint16_t field_0xe;       // +0xE  (= 0 in ctor)
    uint8_t  field_0x10;      // +0x10 (= 0 in ctor)
    uint8_t  field_0x11;      // +0x11
    uint8_t  count;           // +0x12 (= 0 in ctor, max 0xFD)
    uint16_t entries[253];    // +0x13 variable word array
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketItemOperation, entries, 0x13);
ASSERT_SIZE(PacketItemOperation, 0x20d);


// ---------------------------------------------------------------------------
// PacketCmd
// Constructor sub_526A07, static unk_6B1650, vtable off_60ED58.
// Command packet: 2 type/subtype bytes + 3 word params; field_0x12 init = 1.
// Uses inherited Packet VMethod3/VMethod4 serialisation.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketCmd : public Packet
{
public:
    static PacketCmd Inst; // unk_6B1650
public:
    PacketCmd(); // sub_526A07
    PacketCmd(const PacketCmd* src);
public:
    virtual ~PacketCmd();
    virtual uint32_t GetDataSize() override; // sub_57A130
public:
    uint8_t  field_0xa;   // +0xA
    uint8_t  field_0xb;   // +0xB
    uint8_t  type;        // +0xC (= 0 in ctor)
    uint8_t  subtype;     // +0xD (= 0 in ctor)
    uint16_t field_0xe;   // +0xE  (= 0 in ctor)
    uint16_t field_0x10;  // +0x10 (= 0 in ctor)
    uint16_t field_0x12;  // +0x12 (= 1 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketCmd, 0x14);


// ---------------------------------------------------------------------------
// Packet3Dwords
// Constructor sub_526A4F, static unk_6B1670, vtable off_60ED70.
// Three uint32_t fields followed by one uint8_t; all zeroed in ctor.
// Uses inherited Packet VMethod3/VMethod4 serialisation.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class Packet3Dwords : public Packet
{
public:
    static Packet3Dwords Inst; // unk_6B1670
public:
    Packet3Dwords(); // sub_526A4F
    Packet3Dwords(const Packet3Dwords* src);
public:
    virtual ~Packet3Dwords();
    virtual uint32_t GetDataSize() override; // sub_57A170
public:
    uint32_t field_0xa;  // +0xA (= 0 in ctor)
    uint32_t field_0xe;  // +0xE (= 0 in ctor)
    uint32_t field_0x12; // +0x12 (= 0 in ctor)
    char  field_0x16[32]; // +0x16 (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(Packet3Dwords, 0x36);


// ---------------------------------------------------------------------------
// PacketDword
// Constructor sub_526BB0, static unk_6CDB78, vtable off_60EDB8.
// Single uint32_t value payload; zeroed in ctor.
// Copy ctor: sub_526BD9 (copies 5 bytes: id + dword).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketDword : public Packet
{
public:
    static PacketDword Inst; // unk_6CDB78
public:
    PacketDword(); // sub_526BB0
    PacketDword(const PacketDword* src); // sub_526BD9
public:
    virtual ~PacketDword();
    virtual Packet* Duplicate() override; // sub_57A2D0
    virtual uint32_t GetDataSize() override;
public:
    uint32_t value; // +0xA (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketDword, 0xE);


// ---------------------------------------------------------------------------
// PacketAbility
// Constructor sub_526F68, static unk_70A7D0, vtable off_60EDE8.
// Ability/buff broadcast; id fixed to 0x6B ('k') by ctor.
// Copy ctor: sub_526F8E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketAbility : public Packet
{
public:
    static PacketAbility Inst; // unk_70A7D0
public:
    PacketAbility(); // sub_526F68  sets id = 0x6B
    PacketAbility(const PacketAbility* src); // sub_526F8E
public:
    virtual ~PacketAbility();
    virtual Packet* Duplicate() override; // sub_57A480
    virtual uint32_t GetDataSize() override;
public:
    uint8_t field_0xa; // +0xA
    uint8_t field_0xb; // +0xB
    uint8_t field_0xc; // +0xC
    uint8_t field_0xd; // +0xD
};
__pragma(pack(pop))
ASSERT_SIZE(PacketAbility, 0xE);


// ---------------------------------------------------------------------------
// PacketEight
// Constructor sub_526FC7, static unk_6CE970, vtable off_60EE00.
// Generic 8-byte payload packet; fields not initialised by ctor.
// Copy ctor: sub_526FE6 (copies 9 bytes: id + 8 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketEight : public Packet
{
public:
    static PacketEight Inst; // unk_6CE970
public:
    PacketEight(); // sub_526FC7
    PacketEight(const PacketEight* src); // sub_526FE6
public:
    virtual ~PacketEight();
    virtual Packet* Duplicate() override; // sub_57A540
    virtual uint32_t GetDataSize() override; // sub_57A5C0
public:
    uint8_t data[8]; // +0xA..+0x11
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEight, 0x12);


// ---------------------------------------------------------------------------
// PacketMoveCmd
// Constructor sub_52701F, static unk_6A6A70, vtable off_60EE18.
// Movement / targeting command; id fixed to 0x86 by ctor.
// Bytes at +0xC/+0xD/+0xE zeroed by ctor; 0xA/0xB/0xF/0x10 set by caller.
// Copy ctor: sub_52705A (copies 8 bytes: id + 7 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketMoveCmd : public Packet
{
public:
    static PacketMoveCmd Inst; // unk_6A6A70
public:
    PacketMoveCmd(); // sub_52701F  sets id = 0x86
    PacketMoveCmd(const PacketMoveCmd* src); // sub_52705A
public:
    virtual ~PacketMoveCmd();
    virtual Packet* Duplicate() override; // sub_57A600
    virtual uint32_t GetDataSize() override; // sub_57A680
public:
    uint8_t field_0xa;  // +0xA
    uint8_t field_0xb;  // +0xB
    uint8_t field_0xc;  // +0xC (= 0 in ctor)
    uint8_t field_0xd;  // +0xD (= 0 in ctor)
    uint8_t field_0xe;  // +0xE (= 0 in ctor)
    uint8_t field_0xf;  // +0xF
    uint8_t field_0x10; // +0x10
};
__pragma(pack(pop))
ASSERT_SIZE(PacketMoveCmd, 0x11);


// ---------------------------------------------------------------------------
// PacketEffect
// Constructor sub_527093, static unk_6CDB28, vtable off_60EE30.
// Effect-type notification; effect_type byte zeroed by ctor.
// Copy ctor: sub_5270B9 (copies 4 bytes: id + 3 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketEffect : public Packet
{
public:
    static PacketEffect Inst; // unk_6CDB28
public:
    PacketEffect(); // sub_527093
    PacketEffect(const PacketEffect* src); // sub_5270B9
public:
    virtual ~PacketEffect();
    virtual Packet* Duplicate() override; // sub_57A6C0
    virtual uint32_t GetDataSize() override;
public:
    uint8_t field_0xa;    // +0xA
    uint8_t field_0xb;    // +0xB
    uint8_t effect_type;  // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketEffect, 0xD);


// ---------------------------------------------------------------------------
// PacketAoeZone
// Constructor sub_5270F2, static unk_6B37A8, vtable off_60EE48.
// Area-of-effect zone info; id fixed to 0x87; bytes at +0xD/+0xE zeroed.
// Copy ctor: sub_527126 (copies 19 bytes: id + 18 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketAoeZone : public Packet
{
public:
    static PacketAoeZone Inst; // unk_6B37A8
public:
    PacketAoeZone(); // sub_5270F2  sets id = 0x87
    PacketAoeZone(const PacketAoeZone* src); // sub_527126
public:
    virtual ~PacketAoeZone();
    virtual Packet* Duplicate() override; // sub_57A780
    virtual uint32_t GetDataSize() override; // sub_57A800
public:
    uint8_t field_0xa;    // +0xA
    uint8_t field_0xb;    // +0xB
    uint8_t field_0xc;    // +0xC
    uint8_t field_0xd;    // +0xD (= 0 in ctor)
    uint8_t field_0xe;    // +0xE (= 0 in ctor)
    uint8_t field_0xf[13];// +0xF..+0x1B
};
__pragma(pack(pop))
ASSERT_SIZE(PacketAoeZone, 0x1C);


// ---------------------------------------------------------------------------
// PacketMount
// Constructor sub_52715F, static unk_6B37C8, vtable off_60EE60.
// Unit mount / ride packet; word unit_id zeroed by ctor.
// Copy ctor: sub_527187 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketMount : public Packet
{
public:
    static PacketMount Inst; // unk_6B37C8
public:
    PacketMount(); // sub_52715F
    PacketMount(const PacketMount* src); // sub_527187
public:
    virtual ~PacketMount();
    virtual Packet* Duplicate() override; // sub_57A840
    virtual uint32_t GetDataSize() override;
public:
    uint8_t  field_0xa; // +0xA
    uint8_t  field_0xb; // +0xB
    uint16_t unit_id;   // +0xC (= 0 in ctor)
};
__pragma(pack(pop))
ASSERT_SIZE(PacketMount, 0xE);


// ---------------------------------------------------------------------------
// PacketPlayerInfo
// Constructor sub_5271C0, static unk_6D1888, vtable off_60EE78.
// Large variable-length player-info packet.
// Fixed area: 40-byte preamble at +0xA, int32_t count at +0x32.
// Variable-length data starting at +0x36 with `count` bytes.
// VMethod3 (sub_527220): sends count+45 bytes from &id.
// VMethod4 (sub_527246): reads 44-byte fixed block then optional count bytes.
// Copy ctor: sub_5271DF (memcpy id + preamble + count + var_data).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketPlayerInfo : public Packet
{
public:
    static PacketPlayerInfo Inst; // unk_6D1888
public:
    PacketPlayerInfo(); // sub_5271C0
    PacketPlayerInfo(const PacketPlayerInfo* src); // sub_5271DF
public:
    virtual ~PacketPlayerInfo();
    virtual Packet* Duplicate() override;     // sub_57A900
    virtual void VMethod3(NetStru2*) override; // sub_527220
    virtual void VMethod4(NetStru2*) override; // sub_527246
    virtual uint32_t GetDataSize() override;   // sub_57A980
public:
    uint8_t  preamble[40]; // +0xA..+0x31  (40 bytes of fixed player fields)
    int32_t  count;        // +0x32  length of var_data
    uint8_t  var_data[0x10000];  // +0x36  variable-length data (actual size = count)
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketPlayerInfo, preamble,  0xA);
ASSERT_OFFSET(PacketPlayerInfo, count,     0x32);
ASSERT_OFFSET(PacketPlayerInfo, var_data,  0x36);
ASSERT_SIZE(PacketPlayerInfo, 0x10036);


// ---------------------------------------------------------------------------
// PacketData
// Constructor sub_527283, static unk_6B37D8, vtable off_60EE90.
// Generic length-prefixed binary blob: uint32_t count at +0xA, data at +0xE.
// VMethod3 (sub_5272E3): sends count+5 bytes from &id.
// VMethod4 (sub_527309): reads 4-byte count, then count bytes of data.
// Copy ctor: sub_5272A2 (memcpy count+5 bytes from offset 9).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketData : public Packet
{
public:
    static PacketData Inst; // unk_6B37D8
public:
    PacketData(); // sub_527283
    PacketData(const PacketData* src); // sub_5272A2
public:
    virtual ~PacketData();
    virtual Packet* Duplicate() override;     // sub_57A9D0
    virtual void VMethod3(NetStru2*) override; // sub_5272E3
    virtual void VMethod4(NetStru2*) override; // sub_527309
    virtual uint32_t GetDataSize() override;   // sub_57AA50
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
// Constructor sub_527415, static unk_6C5AF8, vtable off_60EEC0.
// Variable-length TLV-encoded unit state vector; id fixed to 0x76 ('v').
// entry_count: number of property entries; data_size: byte length of data[].
// Each entry: 1–5 bytes (type tag + 1/2/4-byte value), added by helpers.
// Helper sub_527574  : opens a 7-byte compound entry slot.
// Helper sub_5275F8  : appends (uint8_t field, uint8_t value) – 2 bytes.
// Helper sub_5276E0  : appends (uint8_t field, uint32_t value) – 5 bytes.
// VMethod3 (sub_5274CC): if entry_count==0 sends 10 bytes; else also sends data.
// VMethod4 (sub_527528): reads 9 fixed bytes at +0xA, then data_size bytes.
// Copy ctor: sub_527473.
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitStateVec : public Packet
{
public:
    static PacketUnitStateVec Inst; // unk_6C5AF8
public:
    PacketUnitStateVec(); // sub_527415  sets id = 0x76
    PacketUnitStateVec(const PacketUnitStateVec* src); // sub_527473
public:
    virtual ~PacketUnitStateVec();
    virtual Packet* Duplicate() override;     // sub_57AB70
    virtual void VMethod3(NetStru2*) override; // sub_5274CC
    virtual void VMethod4(NetStru2*) override; // sub_527528
    virtual uint32_t GetDataSize() override;   // sub_57ABF0
public:
    uint8_t  field_0xa;    // +0xA
    uint8_t  field_0xb;    // +0xB
    uint8_t  field_0xc;    // +0xC
    uint16_t entry_count;  // +0xD  number of TLV entries (= 0 in ctor)
    uint16_t field_0xf;    // +0xF  (= 0 in ctor)
    uint16_t data_size;    // +0x11 byte length of TLV data (= 0 in ctor)
    uint8_t  data[0x8000]; // +0x13 TLV-encoded property stream
};
__pragma(pack(pop))
ASSERT_OFFSET(PacketUnitStateVec, entry_count, 0xD);
ASSERT_OFFSET(PacketUnitStateVec, data_size,   0x11);
ASSERT_OFFSET(PacketUnitStateVec, data,        0x13);
ASSERT_SIZE(PacketUnitStateVec, 0x8013);


// ---------------------------------------------------------------------------
// PacketSync
// Constructor sub_527810, static unk_6A6A88, vtable off_60EED8.
// Unit visibility-sync / broadcast; id fixed to 0x82 by ctor.
// Copy ctor: sub_527836 (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketSync : public Packet
{
public:
    static PacketSync Inst; // unk_6A6A88
public:
    PacketSync(); // sub_527810  sets id = 0x82
    PacketSync(const PacketSync* src); // sub_527836
public:
    virtual ~PacketSync();
    virtual Packet* Duplicate() override; // sub_57AC40
    virtual uint32_t GetDataSize() override;
public:
    uint8_t field_0xa; // +0xA
    uint8_t field_0xb; // +0xB
    uint8_t field_0xc; // +0xC
    uint8_t field_0xd; // +0xD
};
__pragma(pack(pop))
ASSERT_SIZE(PacketSync, 0xE);


// ---------------------------------------------------------------------------
// PacketUnitProperties
// Constructor sub_52786F, static unk_6D0628, vtable off_60EEF0.
// Variable-size player/unit property flags packet; id forced to 0 by ctor.
// flags (word at +0xC): bitmask of up to 12 included uint16_t fields.
// GetDataSize (sub_5278CE) = 0x1D - (12 - popcount(flags))*2  (5..29 bytes).
// Copy ctor: sub_527895 (copies max 29 bytes: id + 28 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketUnitProperties : public Packet
{
public:
    static PacketUnitProperties Inst; // unk_6D0628
public:
    PacketUnitProperties(); // sub_52786F  sets id = 0
    PacketUnitProperties(const PacketUnitProperties* src); // sub_527895
public:
    virtual ~PacketUnitProperties();
    virtual Packet* Duplicate() override; // sub_57AD00
    virtual uint32_t GetDataSize() override; // sub_5278CE  variable (5..29)
public:
    uint8_t  field_0xa;       // +0xA
    uint8_t  field_0xb;       // +0xB
    uint16_t flags;           // +0xC  bitmask of included fields (bits 0..11)
    uint16_t prop[12];        // +0xE  up to 12 optional property words
};
__pragma(pack(pop))
ASSERT_SIZE(PacketUnitProperties, 0x26);


// ---------------------------------------------------------------------------
// PacketPing
// Constructor sub_52792F, static unk_6CDB88, vtable off_60EF08.
// Network ping / state packet; 4 payload bytes after Packet base.
// Copy ctor: sub_52794E (copies 5 bytes: id + 4 payload bytes).
// ---------------------------------------------------------------------------
__pragma(pack(push, 1))
class PacketPing : public Packet
{
public:
    static PacketPing Inst; // unk_6CDB88
public:
    PacketPing(); // sub_52792F
    PacketPing(const PacketPing* src); // sub_52794E
public:
    virtual ~PacketPing();
    virtual Packet* Duplicate() override; // sub_57ADB0
    virtual uint32_t GetDataSize() override; // sub_57AE30
public:
    uint8_t field_0xa; // +0xA
    uint8_t field_0xb; // +0xB
    uint8_t field_0xc; // +0xC
    uint8_t field_0xd; // +0xD
};
__pragma(pack(pop))
ASSERT_SIZE(PacketPing, 0xE);
