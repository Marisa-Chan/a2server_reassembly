#include "packet.h"

#include "net.h"



Packet Packet::Inst;

Packet::Packet()
{
    //52681f
    field_0x4 = 0;
    field_0x5 = 0;
    to_player_id = 0;
    id = 0;
}

Packet::Packet(const Packet* src)
{
    //526856
    id = src->id;
}

Packet::~Packet()
{
    //inlined in 5799da and many other places
}

Packet* Packet::Duplicate()
{
    //579940
    return new Packet(this);
}

void Packet::VMethod3(NetStru2* net)
{
    //526887
    uint32_t sz = GetDataSize();
    net->WriteData(&id, sz);
}

void Packet::VMethod4(NetStru2* net)
{
    //5268b7
    uint32_t sz = GetDataSize();
    if (sz != 1)
        net->ReadData(this + 1, sz - 1);
}

uint32_t Packet::GetDataSize()
{
    //5799c0
    return 1;
}



PacketJoin PacketJoin::Inst;

PacketJoin::PacketJoin()
{
    //526c12
    id = 0x91;

    player_id = 0;
    token_id = 0;
    field_0xc = 0;
    flags = 0;
    name_len = 0;
    name[0] = 0;

}

PacketJoin::PacketJoin(const PacketJoin *src)
{
    //526c52

    id = src->id;
    field_0x5 = src->field_0x5;

    player_id = src->player_id;
    token_id = src->token_id;
    field_0xc = src->field_0xc;
    flags = src->flags;
    name_len = src->name_len;
    memcpy(name, src->name, name_len + 1);
}

PacketJoin::PacketJoin(CString _name)
{
    //526d59
    id = 0x91;

    player_id = 0;
    token_id = 0;
    field_0xc = 0;
    flags = 0;

    strcpy(name, _name);
    name_len = _name.GetLength();
}

PacketJoin::~PacketJoin()
{
    //526e4f
}

Packet* PacketJoin::Duplicate()
{
    //57a390
    return new PacketJoin(this);
}

void PacketJoin::VMethod3(NetStru2* net)
{
    //526ede
    name_len = strlen(name);
    net->WriteData(&id, name_len + 8);
}

void PacketJoin::VMethod4(NetStru2* net)
{
    //526f1d
    net->ReadData(&player_id, 6);
    net->ReadData(name, name_len + 1);
    name[name_len] = 0;
}

uint32_t PacketJoin::GetDataSize()
{
    //57a410
    return name_len + 8;
}



PacketInfo PacketInfo::Inst;

PacketInfo::PacketInfo()
{
    //526b44
    field_0xa = 0;
    field_0xe = 0;
}

PacketInfo::PacketInfo(const PacketInfo* src)
{
    //526b77
    id = src->id;
    field_0xa = src->field_0xa;
    field_0xe = src->field_0xe;
}

PacketInfo::~PacketInfo()
{
    //579a80
}

Packet* PacketInfo::Duplicate()
{
    //57a210
    return new PacketInfo(this);
}

uint32_t PacketInfo::GetDataSize()
{
    //57a290
    return 9;
}


PacketTerrain PacketTerrain::Inst;

PacketTerrain::PacketTerrain()
{
    //527346
}

PacketTerrain::PacketTerrain(const PacketTerrain* src)
{
    //527365
    memcpy(&id, &src->id, count * 2 + 5);    
}
PacketTerrain::~PacketTerrain()
{
    //579ac0
}

Packet* PacketTerrain::Duplicate()
{
    //57aaa0
    return new PacketTerrain(this);
}
    
void PacketTerrain::VMethod3(NetStru2* net)
{
    //5273a7
    net->WriteData(&id, count * 2 + 5);
}

void PacketTerrain::VMethod4(NetStru2* net)
{
    //5273ce
    net->ReadData(&count, 4);
    if (count)
        net->ReadData(buf, count * 2);
}

uint32_t PacketTerrain::GetDataSize()
{
    //57ab20
    return count * 2 + 5;
}

// ============================================================
// PacketUnitUpdate
// ============================================================
PacketUnitUpdate PacketUnitUpdate::Inst;

PacketUnitUpdate::PacketUnitUpdate()
{
    //sub_527987
}

PacketUnitUpdate::PacketUnitUpdate(const PacketUnitUpdate* src)
    :Packet(this)
{
    //sub_5279A6 – vtable-only ctor used in Duplicate; does NOT copy fields
}

PacketUnitUpdate::~PacketUnitUpdate()
{
    //sub_57AF10
}

Packet* PacketUnitUpdate::Duplicate()
{
    //sub_57AE70
    return new PacketUnitUpdate(this);
}

void PacketUnitUpdate::VMethod3(NetStru2* net)
{
    //sub_5279C7

    switch (FUN_00527bd8())
    {
    case 1:
        id = 0x6e;
        break;
    case 2:
        id = 0x6f;
        break;
    case 3:
        id = 0x70;
        break;
    case 4:
        id = 0x6c;
        break;
    }

    net->WriteData(&id, 1);
    net->WriteData(&unit_id, field_0xe + 2);
    net->WriteData(data, data_offset);
}

void PacketUnitUpdate::VMethod4(NetStru2* net)
{
    //sub_527A6C
    switch (id)
    {
    case 0x6c:
        field_0xe = 4;
        break;
    case 0x6e:
        field_0xe = 1;
        break;
    case 0x6f:
        field_0xe = 2;
        break;
    case 0x70:
        field_0xe = 3;
        break;
    }
    flags_mask = 0;
    net->ReadData(&unit_id, field_0xe + 2);
    net->ReadData(data, FUN_00527c3b());
}

uint32_t PacketUnitUpdate::GetDataSize()
{
    //sub_57AEF0
    return field_0xe + 3 + data_offset;
}

uint32_t PacketUnitUpdate::FUN_00527bd8()
{
    if (flags_mask < 0x100)
        field_0xe = 1;
    else if (flags_mask < 0x10000)
        field_0xe = 2;
    else if (flags_mask < 0x1000000)
        field_0xe = 3;
    else
        field_0xe = 4;

    return field_0xe;
}

uint32_t PacketUnitUpdate::FUN_00527c3b()
{
    static uint32_t offsets[32]
    {   2, 2, 5, 4, 3, 2, 1, 2, 4, 4, 4, 4, 4, 2, 2, 1,
        2, 2, 2, 1, 5, 0, 0, 0, 5, 0x10, 0, 0, 0, 8, 0, 0x18
    };

    data_offset = 0;

    uint32_t sh = 1;
    for (int i = 0; i < 32; i++)
    {
        if ((flags_mask & sh) != 0)
            data_offset += offsets[i];
        sh <<= 1;
    }

    return data_offset;
}

// ============================================================
// PacketUnitRef
// ============================================================
PacketWord PacketWord::Inst;

PacketWord::PacketWord()
{
    //sub_5268FC
    value = 0;
}

PacketWord::PacketWord(const PacketWord* src)
{
    id      = src->id;
    value = src->value;
}

PacketWord::~PacketWord()
{
    //sub_57A0B0
}

uint32_t PacketWord::GetDataSize()
{
    //sub_57A0A0
    return 3;
}

// ============================================================
// PacketWordList
// ============================================================
PacketItemOperation PacketItemOperation::Inst;

PacketItemOperation::PacketItemOperation()
{
    //sub_526924
    field_0xe  = 0;
    field_0x10 = 0;
    count      = 0;
}

PacketItemOperation::PacketItemOperation(const PacketItemOperation* src)
{
    id          = src->id;
    unit_id   = src->unit_id;
    field_0xc   = src->field_0xc;
    field_0xd   = src->field_0xd;
    field_0xe   = src->field_0xe;
    field_0x10  = src->field_0x10;
    field_0x11  = src->field_0x11;
    count       = src->count;
    memcpy(entries, src->entries, src->count * 2);
}

void PacketItemOperation::AppendWord(uint16_t val)
{
    //sub_52695A
    if (count < 0xFD)
        entries[count++] = val;
}

PacketItemOperation::~PacketItemOperation()
{
    //sub_57A100
}

void PacketItemOperation::VMethod3(NetStru2* net)
{
    //sub_52699A
    net->WriteData(&id, count * 2 + 0xA);
}

void PacketItemOperation::VMethod4(NetStru2* net)
{
    //sub_5269C3
    net->ReadData(&unit_id, 9);
    if (count)
        net->ReadData(entries, count * 2);
}

uint32_t PacketItemOperation::GetDataSize()
{
    //sub_57A0E0
    return count * 2 + 10;
}

// ============================================================
// PacketCmd
// ============================================================
PacketCmd PacketCmd::Inst;

PacketCmd::PacketCmd()
{
    //sub_526A07
    type       = 0;
    subtype    = 0;
    field_0xe  = 0;
    field_0x10 = 0;
    field_0x12 = 1;
}

PacketCmd::PacketCmd(const PacketCmd* src)
{
    memcpy(&id, &src->id, 0xB); // id(1)+0xA+0xB+type+subtype+0xE(2)+0x10(2)+0x12(2) = 11
}

PacketCmd::~PacketCmd()
{
    //sub_57A140
}

uint32_t PacketCmd::GetDataSize()
{
    //sub_57A130  id(1) + 10 derived bytes = 0xB
    return 0xB;
}

// ============================================================
// Packet3Dwords
// ============================================================
Packet3Dwords Packet3Dwords::Inst;

Packet3Dwords::Packet3Dwords()
{
    //sub_526A4F
    field_0xa  = 0;
    field_0xe  = 0;
    field_0x12 = 0;
    field_0x16[0] = 0;
}

Packet3Dwords::Packet3Dwords(const Packet3Dwords* src)
{
    memcpy(&id, &src->id, 0xE); // id(1)+4+4+4+1 = 14
}

Packet3Dwords::~Packet3Dwords()
{
    //sub_57A180
}

uint32_t Packet3Dwords::GetDataSize()
{
    //sub_57A170  id(1) + 13 derived bytes = 0xE
    return 0x2D;
}

// ============================================================
// PacketDword
// ============================================================
PacketDword PacketDword::Inst;

PacketDword::PacketDword()
{
    //sub_526BB0
    value = 0;
}

PacketDword::PacketDword(const PacketDword* src)
{
    //sub_526BD9
    id    = src->id;
    value = src->value;
}

PacketDword::~PacketDword()
{
    //sub_57A360
}

Packet* PacketDword::Duplicate()
{
    //sub_57A2D0
    return new PacketDword(this);
}

uint32_t PacketDword::GetDataSize()
{
    //sub_57A350
    return 5; // id(1) + value(4)
}

// ============================================================
// PacketAbility
// ============================================================
PacketAbility PacketAbility::Inst;

PacketAbility::PacketAbility()
{
    //sub_526F68  id = 0x6B
    id = 0x6B;
}

PacketAbility::PacketAbility(const PacketAbility* src)
{
    //sub_526F8E
    memcpy(&id, &src->id, 5); // id + 4 bytes
}

PacketAbility::~PacketAbility()
{
    //sub_57A510
}

Packet* PacketAbility::Duplicate()
{
    //sub_57A480
    return new PacketAbility(this);
}

uint32_t PacketAbility::GetDataSize()
{
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketEight
// ============================================================
PacketEight PacketEight::Inst;

PacketEight::PacketEight()
{
    //sub_526FC7
}

PacketEight::PacketEight(const PacketEight* src)
{
    //sub_526FE6
    memcpy(&id, &src->id, 9); // id + 8 bytes
}

PacketEight::~PacketEight()
{
    //sub_57A5D0
}

Packet* PacketEight::Duplicate()
{
    //sub_57A540
    return new PacketEight(this);
}

uint32_t PacketEight::GetDataSize()
{
    //sub_57A5C0
    return 9; // id(1) + 8 bytes
}

// ============================================================
// PacketMoveCmd
// ============================================================
PacketMoveCmd PacketMoveCmd::Inst;

PacketMoveCmd::PacketMoveCmd()
{
    //sub_52701F  id = 0x86
    id         = 0x86;
    field_0xc  = 0;
    field_0xd  = 0;
    field_0xe  = 0;
}

PacketMoveCmd::PacketMoveCmd(const PacketMoveCmd* src)
{
    //sub_52705A
    memcpy(&id, &src->id, 8); // id + 7 bytes
}

PacketMoveCmd::~PacketMoveCmd()
{
    //sub_57A690
}

Packet* PacketMoveCmd::Duplicate()
{
    //sub_57A600
    return new PacketMoveCmd(this);
}

uint32_t PacketMoveCmd::GetDataSize()
{
    //sub_57A680
    return 8; // id(1) + 7 bytes
}

// ============================================================
// PacketEffect
// ============================================================
PacketEffect PacketEffect::Inst;

PacketEffect::PacketEffect()
{
    //sub_527093
    effect_type = 0;
}

PacketEffect::PacketEffect(const PacketEffect* src)
{
    //sub_5270B9
    memcpy(&id, &src->id, 4); // id + 3 bytes
}

PacketEffect::~PacketEffect()
{
    //sub_57A750
}

Packet* PacketEffect::Duplicate()
{
    //sub_57A6C0
    return new PacketEffect(this);
}

uint32_t PacketEffect::GetDataSize()
{
    return 4; // id(1) + 3 bytes
}

// ============================================================
// PacketAoeZone
// ============================================================
PacketAoeZone PacketAoeZone::Inst;

PacketAoeZone::PacketAoeZone()
{
    //sub_5270F2  id = 0x87
    id         = 0x87;
    field_0xd  = 0;
    field_0xe  = 0;
}

PacketAoeZone::PacketAoeZone(const PacketAoeZone* src)
{
    //sub_527126
    memcpy(&id, &src->id, 19); // id + 18 bytes
}

PacketAoeZone::~PacketAoeZone()
{
    //sub_57A810
}

Packet* PacketAoeZone::Duplicate()
{
    //sub_57A780
    return new PacketAoeZone(this);
}

uint32_t PacketAoeZone::GetDataSize()
{
    //sub_57A800
    return 19; // id(1) + 18 bytes
}

// ============================================================
// PacketMount
// ============================================================
PacketMount PacketMount::Inst;

PacketMount::PacketMount()
{
    //sub_52715F
    unit_id = 0;
}

PacketMount::PacketMount(const PacketMount* src)
{
    //sub_527187
    memcpy(&id, &src->id, 5); // id + 4 bytes
}

PacketMount::~PacketMount()
{
    //sub_57A8D0
}

Packet* PacketMount::Duplicate()
{
    //sub_57A840
    return new PacketMount(this);
}

uint32_t PacketMount::GetDataSize()
{
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketPlayerInfo
// ============================================================
PacketPlayerInfo PacketPlayerInfo::Inst;

PacketPlayerInfo::PacketPlayerInfo()
{
    //sub_5271C0
    count = 0;
}

PacketPlayerInfo::PacketPlayerInfo(const PacketPlayerInfo* src)
{
    //sub_5271DF
    memcpy(&id, &src->id, src->count + 0x2D);
}

PacketPlayerInfo::~PacketPlayerInfo()
{
    //sub_57A9A0
}

Packet* PacketPlayerInfo::Duplicate()
{
    //sub_57A900
    return new PacketPlayerInfo(this);
}

void PacketPlayerInfo::VMethod3(NetStru2* net)
{
    //sub_527220
    net->WriteData(&id, count + 0x2D);
}

void PacketPlayerInfo::VMethod4(NetStru2* net)
{
    //sub_527246  reads 0x2C bytes at +0xA, then optional count bytes at +0x36
    net->ReadData(&preamble[0], 0x2C);
    if (count > 0)
        net->ReadData(var_data, count);
}

uint32_t PacketPlayerInfo::GetDataSize()
{
    //sub_57A980
    return count + 0x2D;
}

// ============================================================
// PacketData
// ============================================================
PacketData PacketData::Inst;

PacketData::PacketData()
{
    //sub_527283
    count = 0;
}

PacketData::PacketData(const PacketData* src)
{
    //sub_5272A2
    memcpy(&id, &src->id, src->count + 5);
}

PacketData::~PacketData()
{
    //sub_57AA70
}

Packet* PacketData::Duplicate()
{
    //sub_57A9D0
    return new PacketData(this);
}

void PacketData::VMethod3(NetStru2* net)
{
    //sub_5272E3
    net->WriteData(&id, count + 5);
}

void PacketData::VMethod4(NetStru2* net)
{
    //sub_527309
    net->ReadData(&count, 4);
    if (count > 0)
        net->ReadData(data, count);
}

uint32_t PacketData::GetDataSize()
{
    //sub_57AA50
    return count + 5;
}

// ============================================================
// PacketUnitStateVec
// ============================================================
PacketUnitStateVec PacketUnitStateVec::Inst;

PacketUnitStateVec::PacketUnitStateVec()
{
    //sub_527415  id = 0x76
    id          = 0x76;
    entry_count = 0;
    field_0xf   = 0;
    data_size   = 0;
}

PacketUnitStateVec::PacketUnitStateVec(const PacketUnitStateVec* src)
{
    //sub_527473
    memcpy(&id, &src->id, 10);
    memcpy(data, src->data, src->data_size);
}

PacketUnitStateVec::~PacketUnitStateVec()
{
    //sub_57AC10
}

Packet* PacketUnitStateVec::Duplicate()
{
    //sub_57AB70
    return new PacketUnitStateVec(this);
}

void PacketUnitStateVec::VMethod3(NetStru2* net)
{
    //sub_5274CC
    net->WriteData(&id, 0xA);
    if (entry_count > 0)
        net->WriteData(data, data_size);
    //printf("PacketUnitStateVec: %x %d %x\n", id, entry_count, data_size);
}

void PacketUnitStateVec::VMethod4(NetStru2* net)
{
    //sub_527528
    net->ReadData(&field_0xa, 9);
    if (entry_count > 0)
        net->ReadData(data, data_size);
}

uint32_t PacketUnitStateVec::GetDataSize()
{
    //sub_57ABF0
    return 0xA + (entry_count ? data_size : 0);
}

// ============================================================
// PacketSync
// ============================================================
PacketSync PacketSync::Inst;

PacketSync::PacketSync()
{
    //sub_527810  id = 0x82
    id = 0x82;
}

PacketSync::PacketSync(const PacketSync* src)
{
    //sub_527836
    memcpy(&id, &src->id, 5); // id + 4 bytes
}

PacketSync::~PacketSync()
{
    //sub_57ACD0
}

Packet* PacketSync::Duplicate()
{
    //sub_57AC40
    return new PacketSync(this);
}

uint32_t PacketSync::GetDataSize()
{
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketUnitProperties
// ============================================================
PacketUnitProperties PacketUnitProperties::Inst;

PacketUnitProperties::PacketUnitProperties()
{
    //sub_52786F  id = 0
    id    = 0;
    flags = 0;
}

PacketUnitProperties::PacketUnitProperties(const PacketUnitProperties* src)
{
    //sub_527895
    memcpy(&id, &src->id, 0x1D); // id + 28 bytes (max)
}

PacketUnitProperties::~PacketUnitProperties()
{
    //sub_57AD80
}

Packet* PacketUnitProperties::Duplicate()
{
    //sub_57AD00
    return new PacketUnitProperties(this);
}

uint32_t PacketUnitProperties::GetDataSize()
{
    //sub_5278CE  variable: 0x1D - (12 - popcount(flags)) * 2
    int popcount = 0;
    for (int i = 0; i < 12; i++)
        if (flags & (1 << i)) popcount++;
    return 0x1D - (12 - popcount) * 2;
}

// ============================================================
// PacketPing
// ============================================================
PacketPing PacketPing::Inst;

PacketPing::PacketPing()
{
    //sub_52792F
}

PacketPing::PacketPing(const PacketPing* src)
{
    //sub_52794E
    memcpy(&id, &src->id, 5); // id + 4 bytes
}

PacketPing::~PacketPing()
{
    //sub_57AE40
}

Packet* PacketPing::Duplicate()
{
    //sub_57ADB0
    return new PacketPing(this);
}

uint32_t PacketPing::GetDataSize()
{
    // sub_57ae30
    return 5; // id(1) + 4 bytes
}
