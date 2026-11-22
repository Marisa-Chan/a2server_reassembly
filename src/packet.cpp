#include "packet.h"

#include "net.h"



Packet Packet::Inst;

// 52681f (client: 515651)
Packet::Packet() {
    this->order_mode = 0;
    this->from_player_id = 0;
    this->to_player_id = 0;
    this->id = 0;
}

// 526856 (client: 515688)
Packet::Packet(const Packet* src) {
    this->id = src->id;
}

// server: none (client: 54a470) - inlined in 5799d0 on server
Packet::~Packet() {
    //inlined in 5799da and many other places
}

// 579940 (client: 54a490)
Packet* Packet::Duplicate() {
    return new Packet(this);
}

// 526887 (client: 5156b9)
void Packet::WriteTo(NetStru2* net) {
    uint32_t sz = this->GetDataSize();
    net->WriteData(&this->id, sz);
}

// 5268b7 (client: 5156e9)
void Packet::ReadFrom(NetStru2* net) {
    uint32_t sz = this->GetDataSize();
    if (sz != 1) {
        net->ReadData(this + 1, sz - 1);
    }
}

// 5799c0 (client: 54a510) - { return 1; }
uint32_t Packet::GetDataSize() {
    return 1;
}



PacketJoin PacketJoin::Inst;

// 526c12 (client: 515aac)
PacketJoin::PacketJoin() {
    this->id = CHAT;

    this->player_id = 0;
    this->token_id = 0;
    this->color = 0;
    this->flags = 0;
    this->name_len = 0;
    this->name[0] = 0;

}

// 526c52 (client: 515aec)
PacketJoin::PacketJoin(const PacketJoin *src) {

    this->id = src->id;
    this->from_player_id = src->from_player_id;

#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    if (src->id == HAT_AUTH) {
        memcpy(&this->player_id, &src->player_id, 0x46);
        uint16_t auth_len = *reinterpret_cast<const uint16_t*>(src->name + 0x3E);
        memcpy(this->name + 0x40, src->name + 0x40, auth_len + 1);
        return;
    }
#endif

    this->player_id = src->player_id;
    this->token_id = src->token_id;
    this->color = src->color;
    this->flags = src->flags;
    this->name_len = src->name_len;
    memcpy(this->name, src->name, this->name_len + 1);
}

// 526d59 (client: 515bf3)
PacketJoin::PacketJoin(CString _name) {
    this->id = CHAT;

    this->player_id = 0;
    this->token_id = 0;
    this->color = 0;
    this->flags = 0;

#ifdef A2SERVER_PATCH
    strncpy(this->name, _name, sizeof(this->name) - 1);
    this->name[sizeof(this->name) - 1] = '\0';
    this->name_len = static_cast<uint16_t>(strlen(this->name));
#else
    strcpy(this->name, _name);
    this->name_len = _name.GetLength();
#endif
}

// 526e4f (client: 515ce9)
PacketJoin::~PacketJoin() {
}

// 57a390 (client: 54b240)
Packet* PacketJoin::Duplicate() {
    return new PacketJoin(this);
}

// 526ede (client: 515d77)
void PacketJoin::WriteTo(NetStru2* net) {
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    if (this->id == HAT_AUTH) {
        uint16_t auth_len = static_cast<uint16_t>(strlen(this->name + 0x40));
        *reinterpret_cast<uint16_t*>(this->name + 0x3E) = auth_len;
        net->WriteData(&this->id, static_cast<uint32_t>(auth_len) + 0x48);
        return;
    }
#endif
    this->name_len = strlen(this->name);
    net->WriteData(&this->id, this->name_len + 8);
}

// 526f1d (client: 515db6)
void PacketJoin::ReadFrom(NetStru2* net) {
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    if (this->id == HAT_AUTH) {
        net->ReadData(&this->player_id, 0x46);
        uint16_t auth_len = *reinterpret_cast<uint16_t*>(this->name + 0x3E);
        if (auth_len >= sizeof(this->name) - 0x40) {
            auth_len = static_cast<uint16_t>(sizeof(this->name) - 0x41);
            *reinterpret_cast<uint16_t*>(this->name + 0x3E) = auth_len;
        }
        net->ReadData(this->name + 0x40, static_cast<uint32_t>(auth_len) + 1);
        this->name[0x40 + auth_len] = 0;
        return;
    }
#endif
    net->ReadData(&this->player_id, 6);
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    if (this->name_len >= sizeof(this->name)) {
        this->name_len = static_cast<uint16_t>(sizeof(this->name) - 1);
    }
#endif
    net->ReadData(this->name, this->name_len + 1);
    this->name[this->name_len] = 0;
}

// 57a410 (client: 54b2c0)
uint32_t PacketJoin::GetDataSize() {
#if defined(A2CLIENT_PATCH) || defined(A2SERVER_PATCH)
    if (this->id == HAT_AUTH) {
        uint16_t auth_len = *reinterpret_cast<uint16_t*>(this->name + 0x3E);
        return static_cast<uint32_t>(auth_len) + 0x48;
    }
#endif
    return this->name_len + 8;
}



Packet2Dwords Packet2Dwords::Inst;

// 526b44 (client: 515976)
Packet2Dwords::Packet2Dwords() {
    this->param1 = 0;
    this->param2 = 0;
}

// 526b77 (client: 5159a9)
Packet2Dwords::Packet2Dwords(const Packet2Dwords* src) {
    this->id = src->id;
    this->param1 = src->param1;
    this->param2 = src->param2;
}

// 579a80 (client: 54a5d0)
Packet2Dwords::~Packet2Dwords() {
}

// 57a210 (client: 54b0c0)
Packet* Packet2Dwords::Duplicate() {
    return new Packet2Dwords(this);
}

// 57a290 (client: 54b140)
uint32_t Packet2Dwords::GetDataSize() {
    return 9;
}


PacketTerrain PacketTerrain::Inst;

// 527346 (client: 516317)
PacketTerrain::PacketTerrain() {
}

// 527365 (client: 516336)
PacketTerrain::PacketTerrain(const PacketTerrain* src) {
    memcpy(&this->id, &src->id, this->count * 2 + 5);    
}
// 579ac0 (client: 54a610)
PacketTerrain::~PacketTerrain() {
}

// 57aaa0 (client: 54b930)
Packet* PacketTerrain::Duplicate() {
    return new PacketTerrain(this);
}
    
// 5273a7 (client: 516378)
void PacketTerrain::WriteTo(NetStru2* net) {
    net->WriteData(&this->id, this->count * 2 + 5);
}

// 5273ce (client: 51639f)
void PacketTerrain::ReadFrom(NetStru2* net) {
    net->ReadData(&this->count, 4);
#ifdef A2SERVER_PATCH
    if (this->count > (sizeof(this->buf) / sizeof(this->buf[0]))) {
        this->count = static_cast<uint32_t>(sizeof(this->buf) / sizeof(this->buf[0]));
    }
#endif
    if (this->count) {
        net->ReadData(this->buf, this->count * 2);
    }
}

// 57ab20 (client: 54b9b0)
uint32_t PacketTerrain::GetDataSize() {
    return this->count * 2 + 5;
}

// ============================================================
// PacketUnitUpdate
// ============================================================
PacketUnitUpdate PacketUnitUpdate::Inst;

// 527987 (client: 516957)
PacketUnitUpdate::PacketUnitUpdate() {
}

// 5279a6 (client: 516976) - (vtable-only ctor used in Duplicate)
PacketUnitUpdate::PacketUnitUpdate(const PacketUnitUpdate* src)
    :Packet(this) {
    // 5279A6 – vtable-only ctor used in Duplicate; does NOT copy fields
}

// 579c00 (client: 54a750)
PacketUnitUpdate::~PacketUnitUpdate() {
}

// 57ae70 (client: 54bd00)
Packet* PacketUnitUpdate::Duplicate() {
    return new PacketUnitUpdate(this);
}

// 5279c7 (client: 516997)
void PacketUnitUpdate::WriteTo(NetStru2* net) {

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

// 527a6c (client: 516a3c)
void PacketUnitUpdate::ReadFrom(NetStru2* net) {
    switch (this->id) {
    case UNIT_CHANGE_4:
        this->flags_size = 4;
        break;
    case UNIT_CHANGE_1:
        this->flags_size = 1;
        break;
    case UNIT_CHANGE_2:
        this->flags_size = 2;
        break;
    case UNIT_CHANGE_3:
        this->flags_size = 3;
        break;
    }
    this->flags_mask = 0;
    net->ReadData(&this->unit_id, this->flags_size + 2);
    net->ReadData(this->data, this->CalcDataSize());
}

// 57aef0 (client: 54bd80)
uint32_t PacketUnitUpdate::GetDataSize() {
    // 5F0
    return this->flags_size + 3 + this->data_offset;
}

// 527bd8 (client: 516ba8)
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

// 527c3b (client: 516c0b)
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

// 5268fc (client: 51572e)
PacketWord::PacketWord() {
    // 5FC
    this->value = 0;
}

PacketWord::PacketWord(const PacketWord* src) {
    this->id      = src->id;
    this->value = src->value;
}

// 579a00 (client: 54a550)
PacketWord::~PacketWord() {
}

// 57a0a0 (client: 54af50)
uint32_t PacketWord::GetDataSize() {
    return 3;
}

// ============================================================
// PacketWordList
// ============================================================
PacketGroupOrder PacketGroupOrder::Inst;

// 526924 (client: 515756)
PacketGroupOrder::PacketGroupOrder() {
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

// 52695a (client: 51578c)
void PacketGroupOrder::AppendWord(uint16_t val) {
    if (this->count < 0xFD) {
        this->entries[this->count++] = val;
    }
}

// 5275f8 (client: 5165c8)
void PacketUnitStateVec::AppendByteByte(uint8_t a, uint8_t b, uint8_t* slot) {
    this->data[this->data_size] = a;
    this->data_size++;
    this->data[this->data_size] = b;
    this->data_size++;
    slot[5] += 1;
    slot[6] += 2;
}

// 5276e0 (client: 5166b0)
void PacketUnitStateVec::AppendByteInt(uint8_t a, int32_t v, uint8_t* slot) {
    this->data[this->data_size] = a;
    this->data_size++;
    *reinterpret_cast<int32_t*>(&this->data[this->data_size]) = v;
    this->data_size += 4;
    slot[5] += 1;
    slot[6] += 5;
}

// 579a20 (client: 54a570)
PacketGroupOrder::~PacketGroupOrder() {
}

// 52699a (client: 5157cc)
void PacketGroupOrder::WriteTo(NetStru2* net) {
    // 59A
    net->WriteData(&this->id, this->count * 2 + 0xA);
}

// 5269c3 (client: 5157f5)
void PacketGroupOrder::ReadFrom(NetStru2* net) {
    // 5C3
    net->ReadData(&this->x, 9);
#ifdef A2SERVER_PATCH
    if (this->count > 0xFD) {
        this->count = 0xFD;
    }
#endif
    if (this->count) {
        net->ReadData(this->entries, this->count * 2);
    }
}

// 57a0e0 (client: 54af90)
uint32_t PacketGroupOrder::GetDataSize() {
    return this->count * 2 + 10;
}

// ============================================================
// PacketItemTransfer
// ============================================================
PacketItemTransfer PacketItemTransfer::Inst;

// 526a07 (client: 515839)
PacketItemTransfer::PacketItemTransfer() {
    this->type       = TRANSFER_SRC_NONE;
    this->subtype    = TRANSFER_DST_NONE;
    this->param1  = 0;
    this->param2 = 0;
    this->param3 = 1;
}

PacketItemTransfer::PacketItemTransfer(const PacketItemTransfer* src) {
    memcpy(&this->id, &src->id, 0xB); // id(1)+0xA+0xB+type+subtype+0xE(2)+0x10(2)+0x12(2) = 11
}

// 579a40 (client: 54a590)
PacketItemTransfer::~PacketItemTransfer() {
}

// 57a130 (client: 54afe0)
uint32_t PacketItemTransfer::GetDataSize() {
    // 57A130  id(1) + 10 derived bytes = 0xB
    return 0xB;
}

// ============================================================
// PacketEnterGame
// ============================================================
PacketEnterGame PacketEnterGame::Inst;

// 526a4f (client: 515881)
PacketEnterGame::PacketEnterGame() {
    this->value1  = 0;
    this->value2  = 0;
    this->value3 = 0;
    this->text[0] = 0;
}

PacketEnterGame::PacketEnterGame(const PacketEnterGame* src) {
    memcpy(&this->id, &src->id, 0x2D); // id(1)+4+4+4+32 = 45
}

// 579a60 (client: 54a5b0)
PacketEnterGame::~PacketEnterGame() {
}

// 57a170 (client: 54b020)
uint32_t PacketEnterGame::GetDataSize() {
    // 57A170  id(1) + 44 derived bytes = 0x2D
    return 0x2D;
}

// ============================================================
// PacketDword
// ============================================================
PacketDword PacketDword::Inst;

// 526bb0 (client: 515a16)
PacketDword::PacketDword() {
    this->value = 0;
}

// 526bd9 (client: 515a3f)
PacketDword::PacketDword(const PacketDword* src) {
    this->id    = src->id;
    this->value = src->value;
}

// 579aa0 (client: 54a5f0)
PacketDword::~PacketDword() {
}

// 57a2d0 (client: 54b180)
Packet* PacketDword::Duplicate() {
    return new PacketDword(this);
}

// 57a350 (client: 54b200)
uint32_t PacketDword::GetDataSize() {
    return 5; // id(1) + value(4)
}

// ============================================================
// PacketUnitAction
// ============================================================
PacketUnitAction PacketUnitAction::Inst;

// 526f68 (client: 515e01) - sets id = 0x6B
PacketUnitAction::PacketUnitAction() {
    // 526F68  id = 0x6B
    this->id = UNIT_MOVE;
}

// 526f8e (client: 515e27)
PacketUnitAction::PacketUnitAction(const PacketUnitAction* src) {
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

// 579b00 (client: 54a650)
PacketUnitAction::~PacketUnitAction() {
}

// 57a480 (client: 54b310)
Packet* PacketUnitAction::Duplicate() {
    // 580
    return new PacketUnitAction(this);
}

// 57a500 (client: 54b390)
uint32_t PacketUnitAction::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketSack
// ============================================================
PacketSack PacketSack::Inst;

// 526fc7 (client: 515e94)
PacketSack::PacketSack() {
    // 5C7
}

// 526fe6 (client: 515eb3)
PacketSack::PacketSack(const PacketSack* src) {
    memcpy(&this->id, &src->id, 9); // id + 8 bytes
}

// 579b20 (client: 54a670)
PacketSack::~PacketSack() {
}

// 57a540 (client: 54b3d0)
Packet* PacketSack::Duplicate() {
    return new PacketSack(this);
}

// 57a5c0 (client: 54b450)
uint32_t PacketSack::GetDataSize() {
    return 9; // id(1) + 8 bytes
}

// ============================================================
// PacketMoveBroadcast
// ============================================================
PacketMoveBroadcast PacketMoveBroadcast::Inst;

// 52701f (client: 515f20) - sets id = 0x86
PacketMoveBroadcast::PacketMoveBroadcast() {
    // 52701F  id = 0x86
    this->id         = EFFECT_MOVE;
    this->type_id  = 0;
    this->x  = 0;
    this->y  = 0;
}

// 52705a (client: 515f5b)
PacketMoveBroadcast::PacketMoveBroadcast(const PacketMoveBroadcast* src) {
    memcpy(&this->id, &src->id, 8); // id + 7 bytes
}

// 579b40 (client: 54a690)
PacketMoveBroadcast::~PacketMoveBroadcast() {
}

// 57a600 (client: 54b490)
Packet* PacketMoveBroadcast::Duplicate() {
    // 500
    return new PacketMoveBroadcast(this);
}

// 57a680 (client: 54b510)
uint32_t PacketMoveBroadcast::GetDataSize() {
    // 580
    return 8; // id(1) + 7 bytes
}

// ============================================================
// PacketEffect
// ============================================================
PacketEffect PacketEffect::Inst;

// 527093 (client: 515fc8)
PacketEffect::PacketEffect() {
    // 593
    this->effect_type = 0;
}

// 5270b9 (client: 515fee)
PacketEffect::PacketEffect(const PacketEffect* src) {
    memcpy(&this->id, &src->id, 4); // id + 3 bytes
}

// 579b60 (client: 54a6b0)
PacketEffect::~PacketEffect() {
}

// 57a6c0 (client: 54b550)
Packet* PacketEffect::Duplicate() {
    return new PacketEffect(this);
}

// 57a740 (client: 54b5d0)
uint32_t PacketEffect::GetDataSize() {
    return 4; // id(1) + 3 bytes
}

// ============================================================
// PacketAoeZone
// ============================================================
PacketAoeZone PacketAoeZone::Inst;

// 5270f2 (client: 51605b) - sets id = 0x87
PacketAoeZone::PacketAoeZone() {
    // 5270F2  id = 0x87
    this->id         = AOE_ZONE;
    this->width  = 0;
    this->height  = 0;
}

// 527126 (client: 51608f)
PacketAoeZone::PacketAoeZone(const PacketAoeZone* src) {
    memcpy(&this->id, &src->id, 19); // id + 18 bytes
}

// 579b80 (client: 54a6d0)
PacketAoeZone::~PacketAoeZone() {
    // 510
}

// 57a780 (client: 54b610)
Packet* PacketAoeZone::Duplicate() {
    // 580
    return new PacketAoeZone(this);
}

// 57a800 (client: 54b690)
uint32_t PacketAoeZone::GetDataSize() {
    return 19; // id(1) + 18 bytes
}

// ============================================================
// PacketRangedAttack
// ============================================================
PacketRangedAttack PacketRangedAttack::Inst;

// 52715f (client: 5160fc)
PacketRangedAttack::PacketRangedAttack() {
    this->target_id = 0;
}

// 527187 (client: 516124)
PacketRangedAttack::PacketRangedAttack(const PacketRangedAttack* src) {
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

// 579ba0 (client: 54a6f0)
PacketRangedAttack::~PacketRangedAttack() {
}

// 57a840 (client: 54b6d0)
Packet* PacketRangedAttack::Duplicate() {
    return new PacketRangedAttack(this);
}

// 57a8c0 (client: 54b750)
uint32_t PacketRangedAttack::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketFileChunk
// ============================================================
PacketFileChunk PacketFileChunk::Inst;

// 5271c0 (client: 516191)
PacketFileChunk::PacketFileChunk() {
    this->count = 0;
}

// 5271df (client: 5161b0)
PacketFileChunk::PacketFileChunk(const PacketFileChunk* src) {
    memcpy(&this->id, &src->id, src->count + 0x2D);
}

// 579c40 (client: 54a790)
PacketFileChunk::~PacketFileChunk() {
}

// 57a900 (client: 54b790)
Packet* PacketFileChunk::Duplicate() {
    return new PacketFileChunk(this);
}

// 527220 (client: 5161f1)
void PacketFileChunk::WriteTo(NetStru2* net) {
    net->WriteData(&this->id, this->count + 0x2D);
}

// 527246 (client: 516217)
void PacketFileChunk::ReadFrom(NetStru2* net) {
    // 527246  reads 0x2C bytes at +0xA, then optional count bytes at +0x36
    net->ReadData(&this->preamble[0], 0x2C);
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
    if (this->count < 0) {
        this->count = 0;
    } else if (this->count > static_cast<int32_t>(sizeof(this->var_data))) {
        this->count = static_cast<int32_t>(sizeof(this->var_data));
    }
#endif
    if (this->count > 0) {
        net->ReadData(this->var_data, this->count);
    }
}

// 57a980 (client: 54b810)
uint32_t PacketFileChunk::GetDataSize() {
    return this->count + 0x2D;
}

// ============================================================
// PacketData
// ============================================================
PacketData PacketData::Inst;

// 527283 (client: 516254)
PacketData::PacketData() {
    this->count = 0;
}

// 5272a2 (client: 516273)
PacketData::PacketData(const PacketData* src) {
    memcpy(&this->id, &src->id, src->count + 5);
}

// 579ae0 (client: 54a630)
PacketData::~PacketData() {
}

// 57a9d0 (client: 54b860)
Packet* PacketData::Duplicate() {
    return new PacketData(this);
}

// 5272e3 (client: 5162b4)
void PacketData::WriteTo(NetStru2* net) {
    net->WriteData(&this->id, this->count + 5);
}

// 527309 (client: 5162da)
void PacketData::ReadFrom(NetStru2* net) {
    net->ReadData(&this->count, 4);
#ifdef A2SERVER_PATCH
    if (this->count > sizeof(this->data)) {
        this->count = static_cast<uint32_t>(sizeof(this->data));
    }
#endif
    if (this->count > 0) {
        net->ReadData(this->data, this->count);
    }
}

// 57aa50 (client: 54b8e0)
uint32_t PacketData::GetDataSize() {
    return this->count + 5;
}

// ============================================================
// PacketUnitStateVec
// ============================================================
PacketUnitStateVec PacketUnitStateVec::Inst;

// 527415 (client: 5163e6) - sets id = 0x76
PacketUnitStateVec::PacketUnitStateVec() {
    // 527415  id = 0x76
    this->id          = ITEM_LIST;
    this->entry_count = 0;
    this->start_index   = 0;
    this->data_size   = 0;
}

// 527473 (client: 516443)
PacketUnitStateVec::PacketUnitStateVec(const PacketUnitStateVec* src) {
    memcpy(&this->id, &src->id, 10);
    memcpy(this->data, src->data, src->data_size);
}

// 527456 (client: 516427)
PacketUnitStateVec::~PacketUnitStateVec() {
}

// 57ab70 (client: 54ba00)
Packet* PacketUnitStateVec::Duplicate() {
    return new PacketUnitStateVec(this);
}

// 5274cc (client: 51649c)
void PacketUnitStateVec::WriteTo(NetStru2* net) {
    net->WriteData(&this->id, 0xA);
    if (this->entry_count > 0) {
        net->WriteData(this->data, this->data_size);
    //printf("PacketUnitStateVec: %x %d %x\n", id, entry_count, data_size);
    }
}

// 527528 (client: 5164f8)
void PacketUnitStateVec::ReadFrom(NetStru2* net) {
    net->ReadData(&this->building_id, 9);
#ifdef A2SERVER_PATCH
    if (this->data_size > sizeof(this->data)) {
        this->data_size = static_cast<uint16_t>(sizeof(this->data));
    }
#endif
    if (this->entry_count > 0) {
        net->ReadData(this->data, this->data_size);
    }
}

// 57abf0 (client: 54ba80)
uint32_t PacketUnitStateVec::GetDataSize() {
    return 0xA + (this->entry_count ? this->data_size : 0);
}

// ============================================================
// PacketBuildingHp
// ============================================================
PacketBuildingHp PacketBuildingHp::Inst;

// 527810 (client: 5167e0) - sets id = 0x82
PacketBuildingHp::PacketBuildingHp() {
    // 527810  id = 0x82
    this->id = BUILDING_HP;
}

// 527836 (client: 516806)
PacketBuildingHp::PacketBuildingHp(const PacketBuildingHp* src) {
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

// 579bc0 (client: 54a710)
PacketBuildingHp::~PacketBuildingHp() {
}

// 57ac40 (client: 54bad0)
Packet* PacketBuildingHp::Duplicate() {
    return new PacketBuildingHp(this);
}

// 57acc0 (client: 54bb50)
uint32_t PacketBuildingHp::GetDataSize() {
    return 5; // id(1) + 4 bytes
}

// ============================================================
// PacketUnitEquipment
// ============================================================
PacketUnitEquipment PacketUnitEquipment::Inst;

// 52786f (client: 51683f) - sets id = 0
PacketUnitEquipment::PacketUnitEquipment() {
    // 52786F  id = 0
    this->id    = 0;
    this->flags = 0;
}

// 527895 (client: 516865)
PacketUnitEquipment::PacketUnitEquipment(const PacketUnitEquipment* src) {
    memcpy(&this->id, &src->id, 0x1D); // id + 28 bytes (max)
}

// 579c20 (client: 54a770)
PacketUnitEquipment::~PacketUnitEquipment() {
}

// 57ad00 (client: 54bb90)
Packet* PacketUnitEquipment::Duplicate() {
    return new PacketUnitEquipment(this);
}

void PacketUnitEquipment::ReadFrom(NetStru2* net) {
    net->ReadData(&this->unit_id, 4);
    uint32_t sz = this->GetDataSize();
    if (sz > 5) {
        net->ReadData(this->prop, sz - 5);
    }
}

// 5278ce (client: 51689e) - variable (5.29)
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

// 52792f (client: 5168ff)
PacketUnitHp::PacketUnitHp() {
}

// 52794e (client: 51691e)
PacketUnitHp::PacketUnitHp(const PacketUnitHp* src) {
    memcpy(&this->id, &src->id, 5); // id + 4 bytes
}

// 579be0 (client: 54a730)
PacketUnitHp::~PacketUnitHp() {
}

// 57adb0 (client: 54bc40)
Packet* PacketUnitHp::Duplicate() {
    return new PacketUnitHp(this);
}

// 57ae30 (client: 54bcc0)
uint32_t PacketUnitHp::GetDataSize() {
    return 5; // id(1) + 4 bytes
}
