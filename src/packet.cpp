#include "packet.h"

#include "net.h"



Packet Packet::Inst;

Packet::Packet() {
    //52681f
    this->order_mode = 0;
    this->from_player_id = 0;
    this->to_player_id = 0;
    this->id = 0;
}

Packet::Packet(const Packet* src) {
    //526856
    this->id = src->id;
}

Packet::~Packet() {
    //inlined in 5799da and many other places
}

Packet* Packet::Duplicate() {
    //579940
    return new Packet(this);
}

void Packet::WriteTo(NetStru2* net) {
    //526887
    uint32_t sz = this->GetDataSize();
    net->WriteData(&this->id, sz);
}

void Packet::ReadFrom(NetStru2* net) {
    //5268b7
    uint32_t sz = this->GetDataSize();
    if (sz != 1) {
        net->ReadData(this + 1, sz - 1);
    }
}

uint32_t Packet::GetDataSize() {
    //5799c0
    return 1;
}



PacketJoin PacketJoin::Inst;

PacketJoin::PacketJoin() {
    //526c12
    this->id = CHAT;

    this->player_id = 0;
    this->token_id = 0;
    this->color = 0;
    this->flags = 0;
    this->name_len = 0;
    this->name[0] = 0;

}

PacketJoin::PacketJoin(const PacketJoin *src) {
    //526c52

    this->id = src->id;
    this->from_player_id = src->from_player_id;

    this->player_id = src->player_id;
    this->token_id = src->token_id;
    this->color = src->color;
    this->flags = src->flags;
    this->name_len = src->name_len;
    memcpy(this->name, src->name, this->name_len + 1);
}

PacketJoin::PacketJoin(CString _name) {
    //526d59
    this->id = CHAT;

    this->player_id = 0;
    this->token_id = 0;
    this->color = 0;
    this->flags = 0;

    strcpy(this->name, _name);
    this->name_len = _name.GetLength();
}

PacketJoin::~PacketJoin() {
    //526e4f
}

Packet* PacketJoin::Duplicate() {
    //57a390
    return new PacketJoin(this);
}

void PacketJoin::WriteTo(NetStru2* net) {
    //526ede
    this->name_len = strlen(this->name);
    net->WriteData(&this->id, this->name_len + 8);
}

void PacketJoin::ReadFrom(NetStru2* net) {
    //526f1d
    net->ReadData(&this->player_id, 6);
    net->ReadData(this->name, this->name_len + 1);
    this->name[this->name_len] = 0;
}

uint32_t PacketJoin::GetDataSize() {
    //57a410
    return this->name_len + 8;
}



Packet2Dwords Packet2Dwords::Inst;

Packet2Dwords::Packet2Dwords() {
    //526b44
    this->param1 = 0;
    this->param2 = 0;
}

Packet2Dwords::Packet2Dwords(const Packet2Dwords* src) {
    //526b77
    this->id = src->id;
    this->param1 = src->param1;
    this->param2 = src->param2;
}

Packet2Dwords::~Packet2Dwords() {
    //579a80
}

Packet* Packet2Dwords::Duplicate() {
    //57a210
    return new Packet2Dwords(this);
}

uint32_t Packet2Dwords::GetDataSize() {
    //57a290
    return 9;
}


PacketTerrain PacketTerrain::Inst;

PacketTerrain::PacketTerrain() {
    //527346
}

PacketTerrain::PacketTerrain(const PacketTerrain* src) {
    //527365
    memcpy(&this->id, &src->id, this->count * 2 + 5);    
}
PacketTerrain::~PacketTerrain() {
    //579ac0
}

Packet* PacketTerrain::Duplicate() {
    //57aaa0
    return new PacketTerrain(this);
}
    
void PacketTerrain::WriteTo(NetStru2* net) {
    //5273a7
    net->WriteData(&this->id, this->count * 2 + 5);
}

void PacketTerrain::ReadFrom(NetStru2* net) {
    //5273ce
    net->ReadData(&this->count, 4);
    if (this->count) {
        net->ReadData(this->buf, this->count * 2);
    }
}

uint32_t PacketTerrain::GetDataSize() {
    //57ab20
    return this->count * 2 + 5;
}

// ============================================================
// PacketUnitUpdate
// ============================================================
PacketUnitUpdate PacketUnitUpdate::Inst;

PacketUnitUpdate::PacketUnitUpdate() {
    // 527987
}

PacketUnitUpdate::PacketUnitUpdate(const PacketUnitUpdate* src)
    :Packet(this) {
    // 5279A6 – vtable-only ctor used in Duplicate; does NOT copy fields
}

PacketUnitUpdate::~PacketUnitUpdate() {
    // 57AF10
}

Packet* PacketUnitUpdate::Duplicate() {
    // 57AE70
    return new PacketUnitUpdate(this);
}

void PacketUnitUpdate::WriteTo(NetStru2* net) {
    // 5279C7

    switch (this->CalcFlagsSize()) {
    case 1:
        this->id = UNIT_CHANGE_1;
        break;
    case 2:
        this->id = UNIT_CHANGE_2;
        break;
    case 3:
        this->id = UNIT_CHANGE_3;
        break;
    case 4:
        this->id = UNIT_CHANGE_4;
        break;
    }

    net->WriteData(&this->id, 1);
    net->WriteData(&this->unit_id, this->flags_size + 2);
    net->WriteData(this->data, this->data_offset);
}

void PacketUnitUpdate::ReadFrom(NetStru2* net) {
    // 527A6C
    switch (this->id) {
    case 0x6c:
        this->flags_size = 4;
        break;
    case 0x6e:
        this->flags_size = 1;
        break;
    case 0x6f:
        this->flags_size = 2;
        break;
    case 0x70:
        this->flags_size = 3;
        break;
    }
    this->flags_mask = 0;
    net->ReadData(&this->unit_id, this->flags_size + 2);
    net->ReadData(this->data, this->CalcDataSize());
}

uint32_t PacketUnitUpdate::GetDataSize() {
    // 5F0
    return this->flags_size + 3 + this->data_offset;
}

uint32_t PacketUnitUpdate::CalcFlagsSize() {
    if (this->flags_mask < 0x100) {
        this->flags_size = 1;
    } else if (this->flags_mask < 0x10000) {
        this->flags_size = 2;
    } else if (this->flags_mask < 0x1000000) {
        this->flags_size = 3;
    } else {
        this->flags_size = 4;
    }

    return this->flags_size;
}

uint32_t PacketUnitUpdate::CalcDataSize() {
    static uint32_t offsets[32]
    {   2, 2, 5, 4, 3, 2, 1, 2, 4, 4, 4, 4, 4, 2, 2, 1,
        2, 2, 2, 1, 5, 0, 0, 0, 5, 0x10, 0, 0, 0, 8, 0, 0x18
    };

    this->data_offset = 0;

    uint32_t sh = 1;
    for (int i = 0; i < 32; i++) {
        if ((this->flags_mask & sh) != 0) {
            this->data_offset += offsets[i];
        }
        sh <<= 1;
    }

    return this->data_offset;
}

// ============================================================
// PacketUnitRef
// ============================================================
PacketWord PacketWord::Inst;

PacketWord::PacketWord() {
    // 5FC
    this->value = 0;
}

PacketWord::PacketWord(const PacketWord* src) {
    this->id      = src->id;
    this->value = src->value;
}

PacketWord::~PacketWord() {
    // 57A0B0
}

uint32_t PacketWord::GetDataSize() {
    // 57A0A0
    return 3;
}

// ============================================================
// PacketWordList
// ============================================================
PacketGroupOrder PacketGroupOrder::Inst;

PacketGroupOrder::PacketGroupOrder() {
    // 526924
    this->param  = 0;
    this->slot = 0;
    this->count      = 0;
}

PacketGroupOrder::PacketGroupOrder(const PacketGroupOrder* src) {
    this->id          = src->id;
    this->x   = src->x;
    this->y   = src->y;
    this->y_high   = src->y_high;
    this->param   = src->param;
    this->slot  = src->slot;
    this->slot_high  = src->slot_high;
    this->count       = src->count;
    memcpy(this->entries, src->entries, src->count * 2);
}

void PacketGroupOrder::AppendWord(uint16_t val) {
    // 52695A
    if (this->count < 0xFD) {
        this->entries[this->count++] = val;
    }
}

// 5275F8
void PacketUnitStateVec::AppendByteByte(uint8_t a, uint8_t b, uint8_t* slot) {
    this->data[this->data_size] = a;
    this->data_size++;
    this->data[this->data_size] = b;
    this->data_size++;
    slot[5] += 1;
    slot[6] += 2;
}

// 5276E0
void PacketUnitStateVec::AppendByteInt(uint8_t a, int32_t v, uint8_t* slot) {
    this->data[this->data_size] = a;
    this->data_size++;
    *reinterpret_cast<int32_t*>(&this->data[this->data_size]) = v;
    this->data_size += 4;
    slot[5] += 1;
    slot[6] += 5;
}

PacketGroupOrder::~PacketGroupOrder() {
    // 57A100
}

void PacketGroupOrder::WriteTo(NetStru2* net) {
    // 59A
    net->WriteData(&this->id, this->count * 2 + 0xA);
}

void PacketGroupOrder::ReadFrom(NetStru2* net) {
    // 5C3
    net->ReadData(&this->x, 9);
    if (this->count) {
        net->ReadData(this->entries, this->count * 2);
    }
}

uint32_t PacketGroupOrder::GetDataSize() {
    // 57A0E0
    return this->count * 2 + 10;
}

// ============================================================
// PacketItemTransfer
// ============================================================
PacketItemTransfer PacketItemTransfer::Inst;

PacketItemTransfer::PacketItemTransfer() {
    // 526A07
    this->type       = TRANSFER_SRC_NONE;
    this->subtype    = TRANSFER_DST_NONE;
    this->param1  = 0;
    this->param2 = 0;
    this->param3 = 1;
}

PacketItemTransfer::PacketItemTransfer(const PacketItemTransfer* src) {
    memcpy(&this->id, &src->id, 0xB); // id(1)+0xA+0xB+type+subtype+0xE(2)+0x10(2)+0x12(2) = 11
}

PacketItemTransfer::~PacketItemTransfer() {
    // 57A140
}

uint32_t PacketItemTransfer::GetDataSize() {
    // 57A130  id(1) + 10 derived bytes = 0xB
    return 0xB;
}

// ============================================================
// PacketEnterGame
// ============================================================
PacketEnterGame PacketEnterGame::Inst;

PacketEnterGame::PacketEnterGame() {
    // 526A4F
    this->value1  = 0;
    this->value2  = 0;
    this->value3 = 0;
    this->text[0] = 0;
}

PacketEnterGame::PacketEnterGame(const PacketEnterGame* src) {
    memcpy(&this->id, &src->id, 0xE); // id(1)+4+4+4+1 = 14
}

PacketEnterGame::~PacketEnterGame() {
    // 57A180
}

uint32_t PacketEnterGame::GetDataSize() {
    // 57A170  id(1) + 13 derived bytes = 0xE
    return 0x2D;
}

// ============================================================
// PacketDword
// ============================================================
PacketDword PacketDword::Inst;

PacketDword::PacketDword() {
    // 526BB0
    this->value = 0;
}

PacketDword::PacketDword(const PacketDword* src) {
    // 526BD9
    this->id    = src->id;
    this->value = src->value;
}

PacketDword::~PacketDword() {
    // 57A360
}

Packet* PacketDword::Duplicate() {
    // 57A2D0
    return new PacketDword(this);
}

uint32_t PacketDword::GetDataSize() {
    // 57A350
    return 5; // id(1) + value(4)
}

// ============================================================
// PacketUnitAction
// ============================================================
PacketUnitAction PacketUnitAction::Inst;

PacketUnitAction::PacketUnitAction() {
    // 526F68  id = 0x6B
    this->id = UNIT_MOVE;
}

PacketUnitAction::PacketUnitAction(const PacketUnitAction* src) {
    // 526F8E
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

PacketUnitAction::~PacketUnitAction() {
    // 57A510
}

Packet* PacketUnitAction::Duplicate() {
    // 580
    return new PacketUnitAction(this);
}

uint32_t PacketUnitAction::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketSack
// ============================================================
PacketSack PacketSack::Inst;

PacketSack::PacketSack() {
    // 5C7
}

PacketSack::PacketSack(const PacketSack* src) {
    // 526FE6
    memcpy(&this->id, &src->id, 9); // id + 8 bytes
}

PacketSack::~PacketSack() {
    // 57A5D0
}

Packet* PacketSack::Duplicate() {
    // 57A540
    return new PacketSack(this);
}

uint32_t PacketSack::GetDataSize() {
    // 57A5C0
    return 9; // id(1) + 8 bytes
}

// ============================================================
// PacketMoveBroadcast
// ============================================================
PacketMoveBroadcast PacketMoveBroadcast::Inst;

PacketMoveBroadcast::PacketMoveBroadcast() {
    // 52701F  id = 0x86
    this->id         = 0x86;
    this->type_id  = 0;
    this->x  = 0;
    this->y  = 0;
}

PacketMoveBroadcast::PacketMoveBroadcast(const PacketMoveBroadcast* src) {
    // 52705A
    memcpy(&this->id, &src->id, 8); // id + 7 bytes
}

PacketMoveBroadcast::~PacketMoveBroadcast() {
    // 57A690
}

Packet* PacketMoveBroadcast::Duplicate() {
    // 500
    return new PacketMoveBroadcast(this);
}

uint32_t PacketMoveBroadcast::GetDataSize() {
    // 580
    return 8; // id(1) + 7 bytes
}

// ============================================================
// PacketEffect
// ============================================================
PacketEffect PacketEffect::Inst;

PacketEffect::PacketEffect() {
    // 593
    this->effect_type = 0;
}

PacketEffect::PacketEffect(const PacketEffect* src) {
    // 5270B9
    memcpy(&this->id, &src->id, 4); // id + 3 bytes
}

PacketEffect::~PacketEffect() {
    // 57A750
}

Packet* PacketEffect::Duplicate() {
    // 57A6C0
    return new PacketEffect(this);
}

uint32_t PacketEffect::GetDataSize() {
    return 4; // id(1) + 3 bytes
}

// ============================================================
// PacketAoeZone
// ============================================================
PacketAoeZone PacketAoeZone::Inst;

PacketAoeZone::PacketAoeZone() {
    // 5270F2  id = 0x87
    this->id         = 0x87;
    this->width  = 0;
    this->height  = 0;
}

PacketAoeZone::PacketAoeZone(const PacketAoeZone* src) {
    // 527126
    memcpy(&this->id, &src->id, 19); // id + 18 bytes
}

PacketAoeZone::~PacketAoeZone() {
    // 510
}

Packet* PacketAoeZone::Duplicate() {
    // 580
    return new PacketAoeZone(this);
}

uint32_t PacketAoeZone::GetDataSize() {
    // 57A800
    return 19; // id(1) + 18 bytes
}

// ============================================================
// PacketRangedAttack
// ============================================================
PacketRangedAttack PacketRangedAttack::Inst;

PacketRangedAttack::PacketRangedAttack() {
    // 52715F
    this->target_id = 0;
}

PacketRangedAttack::PacketRangedAttack(const PacketRangedAttack* src) {
    // 527187
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

PacketRangedAttack::~PacketRangedAttack() {
    // 57A8D0
}

Packet* PacketRangedAttack::Duplicate() {
    // 57A840
    return new PacketRangedAttack(this);
}

uint32_t PacketRangedAttack::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketFileChunk
// ============================================================
PacketFileChunk PacketFileChunk::Inst;

PacketFileChunk::PacketFileChunk() {
    // 5271C0
    this->count = 0;
}

PacketFileChunk::PacketFileChunk(const PacketFileChunk* src) {
    // 5271DF
    memcpy(&this->id, &src->id, src->count + 0x2D);
}

PacketFileChunk::~PacketFileChunk() {
    // 57A9A0
}

Packet* PacketFileChunk::Duplicate() {
    // 57A900
    return new PacketFileChunk(this);
}

void PacketFileChunk::WriteTo(NetStru2* net) {
    // 527220
    net->WriteData(&this->id, this->count + 0x2D);
}

void PacketFileChunk::ReadFrom(NetStru2* net) {
    // 527246  reads 0x2C bytes at +0xA, then optional count bytes at +0x36
    net->ReadData(&this->preamble[0], 0x2C);
    if (this->count > 0) {
        net->ReadData(this->var_data, this->count);
    }
}

uint32_t PacketFileChunk::GetDataSize() {
    // 57A980
    return this->count + 0x2D;
}

// ============================================================
// PacketData
// ============================================================
PacketData PacketData::Inst;

PacketData::PacketData() {
    // 527283
    this->count = 0;
}

PacketData::PacketData(const PacketData* src) {
    // 5272A2
    memcpy(&this->id, &src->id, src->count + 5);
}

PacketData::~PacketData() {
    // 57AA70
}

Packet* PacketData::Duplicate() {
    // 57A9D0
    return new PacketData(this);
}

void PacketData::WriteTo(NetStru2* net) {
    // 5272E3
    net->WriteData(&this->id, this->count + 5);
}

void PacketData::ReadFrom(NetStru2* net) {
    // 527309
    net->ReadData(&this->count, 4);
    if (this->count > 0) {
        net->ReadData(this->data, this->count);
    }
}

uint32_t PacketData::GetDataSize() {
    // 57AA50
    return this->count + 5;
}

// ============================================================
// PacketUnitStateVec
// ============================================================
PacketUnitStateVec PacketUnitStateVec::Inst;

PacketUnitStateVec::PacketUnitStateVec() {
    // 527415  id = 0x76
    this->id          = 0x76;
    this->entry_count = 0;
    this->start_index   = 0;
    this->data_size   = 0;
}

PacketUnitStateVec::PacketUnitStateVec(const PacketUnitStateVec* src) {
    // 527473
    memcpy(&this->id, &src->id, 10);
    memcpy(this->data, src->data, src->data_size);
}

PacketUnitStateVec::~PacketUnitStateVec() {
    // 57AC10
}

Packet* PacketUnitStateVec::Duplicate() {
    // 57AB70
    return new PacketUnitStateVec(this);
}

void PacketUnitStateVec::WriteTo(NetStru2* net) {
    // 5274CC
    net->WriteData(&this->id, 0xA);
    if (this->entry_count > 0) {
        net->WriteData(this->data, this->data_size);
    //printf("PacketUnitStateVec: %x %d %x\n", id, entry_count, data_size);
    }
}

void PacketUnitStateVec::ReadFrom(NetStru2* net) {
    // 527528
    net->ReadData(&this->building_id, 9);
    if (this->entry_count > 0) {
        net->ReadData(this->data, this->data_size);
    }
}

uint32_t PacketUnitStateVec::GetDataSize() {
    // 57ABF0
    return 0xA + (this->entry_count ? this->data_size : 0);
}

// ============================================================
// PacketBuildingHp
// ============================================================
PacketBuildingHp PacketBuildingHp::Inst;

PacketBuildingHp::PacketBuildingHp() {
    // 527810  id = 0x82
    this->id = BUILDING_HP;
}

PacketBuildingHp::PacketBuildingHp(const PacketBuildingHp* src) {
    // 527836
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

PacketBuildingHp::~PacketBuildingHp() {
    // 57ACD0
}

Packet* PacketBuildingHp::Duplicate() {
    // 57AC40
    return new PacketBuildingHp(this);
}

uint32_t PacketBuildingHp::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketUnitEquipment
// ============================================================
PacketUnitEquipment PacketUnitEquipment::Inst;

PacketUnitEquipment::PacketUnitEquipment() {
    // 52786F  id = 0
    this->id    = 0;
    this->flags = 0;
}

PacketUnitEquipment::PacketUnitEquipment(const PacketUnitEquipment* src) {
    // 527895
    memcpy(&this->id, &src->id, 0x1D); // id + 28 bytes (max)
}

PacketUnitEquipment::~PacketUnitEquipment() {
    // 57AD80
}

Packet* PacketUnitEquipment::Duplicate() {
    // 57AD00
    return new PacketUnitEquipment(this);
}

uint32_t PacketUnitEquipment::GetDataSize() {
    // 5278CE  variable: 0x1D - (12 - popcount(flags)) * 2
    int popcount = 0;
    for (int i = 0; i < 12; i++) {
        if (this->flags & (1 << i)) {
            popcount++;
        }
    }
    return 0x1D - (12 - popcount) * 2;
}

// ============================================================
// PacketUnitHp
// ============================================================
PacketUnitHp PacketUnitHp::Inst;

PacketUnitHp::PacketUnitHp() {
    // 52792F
}

PacketUnitHp::PacketUnitHp(const PacketUnitHp* src) {
    // 52794E
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

PacketUnitHp::~PacketUnitHp() {
    // 57AE40
}

Packet* PacketUnitHp::Duplicate() {
    // 57ADB0
    return new PacketUnitHp(this);
}

uint32_t PacketUnitHp::GetDataSize() {
    // 57ae30
    return 5; // id(1) + 4 bytes
}
