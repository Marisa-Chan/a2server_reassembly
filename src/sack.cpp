#include "sack.h"
#include "inventory.h"

extern "C" uint32_t BldIdSet_AllocBit();

IMPLEMENT_SERIAL(Sack, Token, 1); // 637318

// 553D5D
Sack::Sack() {
    this->inventory = new Inventory();
    this->Init();
}

// 553DF1
Sack::Sack(const TokenPos* pos) : Token(pos) {
    this->inventory = new Inventory();
    this->Init();
}

// 553E8B
Sack::Sack(const TokenPos* pos, Inventory* inv) : Token(pos) {
    this->inventory = inv;
    this->Init();
}

// 553eee
void Sack::Init() {
    this->money = 0;
    this->field_0x44 = 0;
    this->building_id = BldIdSet_AllocBit() & 0xFFFF;
    if (!this->position->sub_58bec3()) {
        this->position->FUN_0058a7e8(this->position->GetX(), this->position->GetY());
    }
    this->field_0x48 = 0;
    this->field_0x4a = 0;
    this->field_0x4c = 0xFFFF;
}

// 55401E
void Sack::sub_55401E() {
    this->_exp = this->money;
    for (POSITION pos = this->inventory->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = this->inventory->items.GetNext(pos);
        this->_exp += item->_exp;
    }
}

// 553F6A
Sack::~Sack() {
    BldIdSet_Unset(this->building_id);
    delete this->inventory;
    this->inventory = nullptr;
}

// 55C889
void Sack::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->money;
    } else {
        ar >> this->money;
    }
    this->inventory->sub_55CCE4(ar);
}

// 57dd30
void Sack::VMethod1() {
}

// 57dd40
void Sack::VMethod2() {
}
