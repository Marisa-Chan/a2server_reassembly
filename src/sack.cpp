#include "sack.h"
#include "inventory.h"

uint32_t BldIdSet_AllocBit();

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
    this->needs_save = 0;
    this->building_id = BldIdSet_AllocBit() & 0xFFFF;
    if (!this->position->IsAtCellCenter()) {
        this->position->ClampToMap(this->position->GetX(), this->position->GetY());
    }
    this->seen_by_mask = 0;
    this->seen_by_mask_prev = 0;
    this->resend_mask = 0xFFFF;
}

// 55401E
void Sack::CalcContentsValue() {
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
    this->inventory->Serialize(ar);
}

// 57dd30
void Sack::RegenTick() {
}

// 57dd40
void Sack::ProcessEffects() {
}
