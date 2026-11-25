#include "sack.h"
#include "inventory.h"

uint32_t BldIdSet_AllocBit();

IMPLEMENT_SERIAL(Sack, Token, 1); // 637318

// 553d5d (client: 534311)
Sack::Sack() {
    this->inventory = new Inventory();
    this->Init();
}

// 553df1 (client: 5343a5)
Sack::Sack(const TokenPos* pos) : Token(pos) {
    this->inventory = new Inventory();
    this->Init();
}

// 553e8b (client: 53443f)
Sack::Sack(const TokenPos* pos, Inventory* inv) : Token(pos) {
    this->inventory = inv;
    this->Init();
}

// 553eee (client: 5344a2)
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

// 55401e (client: 5345b7)
void Sack::CalcContentsValue() {
    this->worth = this->money;
    for (POSITION pos = this->inventory->items.GetHeadPosition(); pos != nullptr;) {
        Item* item = this->inventory->items.GetNext(pos);
        this->worth += item->worth;
    }
}

// 553f6a (client: 53451e)
Sack::~Sack() {
    BldIdSet_Unset(this->building_id);
    delete this->inventory;
    this->inventory = nullptr;
}

// 55c889 (client: 537600)
void Sack::Serialize(CArchive& ar) {
    Token::Serialize(ar);
    if (ar.IsStoring()) {
        ar << this->money;
    } else {
        ar >> this->money;
    }
    this->inventory->Serialize(ar);
}

// 57dd30 (client: 54ee70)
void Sack::RegenTick() {
}

// 57dd40 (client: 54ee80)
void Sack::ProcessEffects() {
}
