#include "token.h"
#include "server.h"
#include "game_app.h"

//6b16a8
MapStuff* MapStuff_Instance = nullptr;

//70a7e0
BitSet<0x6000> g_buildingIdSet;

// 509cd8 (client: 501873)
void BldIdSet_Set(uint32_t index) { g_buildingIdSet.Set(index); }
// 509cfb (client: 501896)
void BldIdSet_Unset(uint32_t index) { g_buildingIdSet.Unset(index); }
// 509d4c (client: 5018e7)
void BldIdSet_Clear() { g_buildingIdSet.Clear(); }
// 509d9f (client: 50193a)
uint32_t BldIdSet_AllocBit() { return g_buildingIdSet.AllocBit(); }


// 58a4b1 (client: 559961)
void TokenPos::Set(uint8_t _x, uint8_t _y, MapStuff* _inst) {
    this->x = _x;
    this->y = _y;
    this->YX.Set(_x, _y);
    this->x_subcell = 128;
    this->y_subcell = 128;
    this->pInstance = _inst;
}

// 58a4f9 (client: 5599a9)
TokenPos::TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst) {
    this->Set(_x, _y, _inst);
}

// 58a495 (client: 559945)
TokenPos::TokenPos() {
    this->Set(0, 0, nullptr);
}

// 58a562 (client: 559a12)
TokenPos::TokenPos(uint16_t _YX, MapStuff* _inst) {
    this->Set(_YX & 8, _YX >> 8, _inst);
}

// 58a9ee (client: 559e9e)
TokenPos::TokenPos(const TokenPos&) = default;
TokenPos::TokenPos(TokenPos&&) = default;

// 58aa46 (client: 559ef6)
TokenPos& TokenPos::operator=(const TokenPos&) = default;

// 58ab11 (client: 559fc1)
void TokenPos::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        //ar.Write(this, sizeof(TokenPos));
        ar.Write(this, 0xc);
    } else {
        //ar.Read(this, sizeof(TokenPos));
        ar.Read(this, 0xc);
    }
}

// 58aaf0 (client: 559fa0)
uint8_t TokenPos::GetX() const {
    #ifdef A2SERVER_PATCH
    if (this == nullptr) {
        return 0;
    }
    #endif
    return this->x;
}

// 58ab00 (client: 559fb0)
uint8_t TokenPos::GetY() const {
    return this->y;
}

// split from 58aade (client: 559f8e)
PosYX TokenPos::GetYX() const {
    return this->YX;
}

// 58aade (client: 559f8e) - ABI compatibility wrapper
uint16_t TokenPos::CompatGetYX() const {
    return this->YX.val;
}

// 58aa9b (client: 559f4b)
uint16_t TokenPos::GetXx() const {
    return (this->x << 8) | this->x_subcell;
}

// 58aabc (client: 559f6c)
uint16_t TokenPos::GetYy() const {
    return (this->y << 8) | this->y_subcell;
}

// 58a6bf (client: 559b6f)
uint8_t TokenPos::Distance(const TokenPos* b) const {
    uint8_t xLen = abs((int32_t)this->x - (int32_t)b->x);
    uint8_t yLen = abs((int32_t)this->y - (int32_t)b->y);
    if (xLen <= yLen) {
        return yLen;
    }
    return xLen;
}

// 58a5b3 (client: 559a63)
int TokenPos::IsSameYX(const TokenPos* b) const {
    return this->YX == b->YX;
}

// 58a737 (client: 559be7)
void TokenPos::SetCoords(uint8_t _x, uint8_t _y) {
    this->x = _x;
    this->y = _y;
    this->YX.Set(_x, _y);
    this->x_subcell = 128;
    this->y_subcell = 128;
}

// 58a781 (client: 559c31)
void TokenPos::SetCoords2(uint16_t Xx, uint16_t Yy) {
    this->x = Xx >> 8;
    this->y = Yy >> 8;
    this->YX.Set(this->x, this->y);
    this->x_subcell = Xx & 0xff;
    this->y_subcell = Yy & 0xff;
}

// 594200 (client: 563671)
void TokenPos::FixupInstance() {
    if (!this->pInstance) {
        return;
    }

    MapStuff* stuf = nullptr;
    if (g_Server->saved_ptr_map.Lookup(this->pInstance, *(void**)&stuf)) {
        this->pInstance = stuf;
    }
}

// 58a7e8 (client: 559c98)
int TokenPos::ClampToMap(uint8_t _x, uint8_t _y) {
    int ok = 1;
    if (_x < 8) {
        _x = 8;
        ok = 0;
    }
    if (_y < 8) {
        _y = 8;
        ok = 0;
    }
    if (_x > this->pInstance->GetWidth() - 9) {
        _x = this->pInstance->GetWidth() - 9;
        ok = 0;
    }
    if (_y > this->pInstance->GetHeight() - 9) {
        _y = this->pInstance->GetHeight() - 9;
        ok = 0;
    }
    this->SetCoords(_x, _y);
    return ok;
}

// 58bec3 (client: 55b373) - Check if we'in the middle of a cell (both subcell values are 0x80).
int32_t TokenPos::IsAtCellCenter() {
    if (this->x_subcell == 128 && this->y_subcell == 128) {
        return 1;
    }
    return 0;
}




// 5280ad (client: 517084)
Token::Token() {
    this->pOwner = nullptr;
    this->position = new TokenPos(0, MapStuff_Instance);
    this->sent_to_mask = 0;
    this->_exp = 0;
    this->building_id = 0;
}

// 5285c6 (client: 517363)
Token::Token(const Token& b) {
    this->itemDataID = b.itemDataID;
    this->typeId = b.typeId;
    this->pOwner = b.pOwner;
    this->TokenID = b.TokenID;
    this->sent_to_mask = b.sent_to_mask;
    this->_exp = b._exp;
    //building_id = b.building_id;  // No copy in vanilla?
    if (b.position == nullptr) {
        this->position = nullptr;
    } else {
        this->position = new TokenPos(*b.position);
    }
}

//Token::Token(Token&&) = default;

// 5281af (client: 51713a)
Token::Token(const TokenPos* tpos) {
    this->pOwner = nullptr;
    this->position = new TokenPos(*tpos);
    this->sent_to_mask = 0;
    this->_exp = 0;
    this->building_id = 0;
}

// 5282af (client: 5171ed)
Token::Token(const TokenPos* tpos, Player* pl) {
    this->pOwner = pl;
    this->position = new TokenPos(*tpos);
    this->sent_to_mask = 0;
    this->_exp = 0;
    this->building_id = 0;
}

// 6363b8
IMPLEMENT_SERIAL(Token, CObject, 1);

// 5283d6 (client: 5172c7)
Token::~Token() {
    if (this->position) {
        delete this->position;
    }
}

// 57b290 (client: 54bfc0)
void Token::RegenTick() {
}

// 52871a (client: 51746a)
void Token::ProcessEffects() {
}

// 57b2a0 (client: 54bfd0) - TokenSize
int32_t Token::GetTokenSize() {
    return 1;
}

// 57b2b0 (client: 54bfe0) - MovementType
int32_t Token::GetMovementType() {
    return 0;
}

// 55af48 (client: 536e27)
void Token::FixupPointers() {
    this->position->FixupInstance();
}

// 57b2c0 (client: 54bff0) - IsItem
int32_t Token::IsItem() {
    return 0;
}

// 57b2d0 (client: 54c000) - IsUnit
int32_t Token::IsUnit() {
    return 0;
}

// 57b2e0 (client: 54c010) - IsHumanoid
int32_t Token::IsHumanoid() {
    return 0;
}

// 57b2f0 (client: 54c020) - IsBuilding
int32_t Token::IsBuilding() {
    return 0;
}


// 55ab39 (client: 536caf)
void Token::Serialize(CArchive& ar) {
    this->position->Serialize(ar);
    if (ar.IsStoring()) {
        ar.Write(&this->building_id, 4);
        ar.Write(&this->itemDataID, 2);
        ar.Write(&this->typeId, 2);
        ar.Write(&this->TokenID, 4);
        ar.Write(&this->sent_to_mask, 2);
        ar.Write(&this->_exp, 4);
        void* ptr = this;
        ar.Write(&ptr, 4); //32
        ar.Write(&this->pOwner, 4); //32
    } else {
        ar.Read(&this->building_id, 4);
        if (this->building_id != 0) {
            BldIdSet_Set((uint16_t)this->building_id);
        }
        ar.Read(&this->itemDataID, 2);
        ar.Read(&this->typeId, 2);
        ar.Read(&this->TokenID, 4);
        ar.Read(&this->sent_to_mask, 2);
        this->sent_to_mask = 0; //WTF?
        ar.Read(&this->_exp, 4);
        void* ptr = nullptr;
        ar.Read(&ptr, 4); //32
        g_Server->saved_ptr_map[ptr] = this;

        ar.Read(&ptr, 4);//32

        void* remapped_owner = nullptr;
        if (g_Server->saved_ptr_map.Lookup(ptr, remapped_owner)) {
            this->pOwner = (Player*)remapped_owner;
        } else {
            this->pOwner = nullptr;
        }
    }
}
