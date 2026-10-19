#include "token.h"
#include "server.h"
#include "game_app.h"

//6b16a8
MapStuff* MapStuff_Instance = nullptr;

//70a7e0
BitSet<0x6000> g_buildingIdSet;

void BldIdSet_Set(uint32_t index) { g_buildingIdSet.Set(index); }
void BldIdSet_Unset(uint32_t index) { g_buildingIdSet.Unset(index); }
void BldIdSet_Clear() { g_buildingIdSet.Clear(); }
uint32_t BldIdSet_AllocBit() { return g_buildingIdSet.AllocBit(); }


void TokenPos::Set(uint8_t _x, uint8_t _y, MapStuff* _inst) { //58a4b1
	this->x = _x;
	this->y = _y;
	this->YX.Set(_x, _y);
	this->x_subcell = 128;
	this->y_subcell = 128;
	this->pInstance = _inst;
}

TokenPos::TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst) {
	//58a4f9
	this->Set(_x, _y, _inst);
}

TokenPos::TokenPos() {
	//58a495
	this->Set(0, 0, nullptr);
}

TokenPos::TokenPos(uint16_t _YX, MapStuff* _inst) {
	//58a562
	//58a583
	this->Set(_YX & 8, _YX >> 8, _inst);
}

//58a9ee
TokenPos::TokenPos(const TokenPos&) = default;
TokenPos::TokenPos(TokenPos&&) = default;

//58aa46
TokenPos& TokenPos::operator=(const TokenPos&) = default;

void TokenPos::Serialize(CArchive& ar) {
	//58ab11
	if (ar.IsStoring()) {
		//ar.Write(this, sizeof(TokenPos));
		ar.Write(this, 0xc);
	} else {
		//ar.Read(this, sizeof(TokenPos));
		ar.Read(this, 0xc);
	}
}

uint8_t TokenPos::GetX() const {
	//58aaf0
	return this->x;
}

uint8_t TokenPos::GetY() const {
	//58ab00
	return this->y;
}

PosYX TokenPos::GetYX() const {
	return this->YX;
}

uint16_t TokenPos::CompatGetYX() const {
	//58aade
	return this->YX.val;
}

uint16_t TokenPos::GetXx() const {
	//58aa9b
	return (this->x << 8) | this->x_subcell;
}

uint16_t TokenPos::GetYy() const {
	//58aabc
	return (this->y << 8) | this->y_subcell;
}

uint8_t TokenPos::Distance(const TokenPos* b) const {
	//58a6bf
	uint8_t xLen = abs((int32_t)this->x - (int32_t)b->x);
	uint8_t yLen = abs((int32_t)this->y - (int32_t)b->y);
	if (xLen <= yLen) {
		return yLen;
	}
	return xLen;
}

int TokenPos::IsSameYX(const TokenPos* b) const {
	//58a5b3
	return this->YX == b->YX;
}

void TokenPos::SetCoords(uint8_t _x, uint8_t _y) {
	//58a737
	this->x = _x;
	this->y = _y;
	this->YX.Set(_x, _y);
	this->x_subcell = 128;
	this->y_subcell = 128;
}

void TokenPos::SetCoords2(uint16_t Xx, uint16_t Yy) {
	//58a781
	this->x = Xx >> 8;
	this->y = Yy >> 8;
	this->YX.Set(this->x, this->y);
	this->x_subcell = Xx & 0xff;
	this->y_subcell = Yy & 0xff;
}

void TokenPos::FUN_00594200() { // 594200
	if (!this->pInstance) {
		return;
	}

	MapStuff* stuf = nullptr;
	if (g_Server->field23_0xdc.Lookup(this->pInstance, *(void**)&stuf)) {
		this->pInstance = stuf;
	}
}

int TokenPos::FUN_0058a7e8(uint8_t _x, uint8_t _y) { //58a7e8
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

int32_t TokenPos::sub_58bec3() { // Check if we'in the middle of a cell (both subcell values are 0x80). //58bec3
	if (this->x_subcell == 128 && this->y_subcell == 128) {
		return 1;
	}
	return 0;
}




Token::Token() {
	// 5280ad
	this->pOwner = nullptr;
	this->position = new TokenPos(0, MapStuff_Instance);
	this->sent_to_mask = 0;
	this->_exp = 0;
	this->building_id = 0;
}

Token::Token(const Token& b) {
	// 5285c6
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

Token::Token(const TokenPos* tpos) {
	// 5281af
	this->pOwner = nullptr;
	this->position = new TokenPos(*tpos);
	this->sent_to_mask = 0;
	this->_exp = 0;
	this->building_id = 0;
}

Token::Token(const TokenPos* tpos, Player* pl) {
	// 5282af
	this->pOwner = pl;
	this->position = new TokenPos(*tpos);
	this->sent_to_mask = 0;
	this->_exp = 0;
	this->building_id = 0;
}

// 6363b8
IMPLEMENT_SERIAL(Token, CObject, 1);

Token::~Token() {
	// 5283d6
	if (this->position) {
		delete this->position;
	}
}

void Token::RegenTick() {
	// 57b290
}

void Token::ProcessEffects() {
	// 52871a
}

int32_t Token::GetTokenSize() {
	// 57b2a0
	return 1;
}

int32_t Token::GetMovementType() {
	// 57b2b0
	return 0;
}

void Token::VMethod5() {
	// 55af48
	this->position->FUN_00594200();
}

int32_t Token::VMethod6() {
	// 57b2c0
	return 0;
}

int32_t Token::VMethod7() {
	// 57b2d0
	return 0;
}

int32_t Token::VMethod8() {
	// 57b2e0
	return 0;
}

int32_t Token::VMethod9() {
	// 57b2f0
	return 0;
}


void Token::Serialize(CArchive& ar) { //55ab39
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
		ar.Read(&this->itemDataID, 2);
		ar.Read(&this->typeId, 2);
		ar.Read(&this->TokenID, 4);
		ar.Read(&this->sent_to_mask, 2);
		this->sent_to_mask = 0; //WTF?
		ar.Read(&this->_exp, 4);
		void* ptr = nullptr;
		ar.Read(&ptr, 4); //32
		g_Server->field23_0xdc[ptr] = this;

		ar.Read(&ptr, 4);//32

		g_Server->field23_0xdc.Lookup(ptr, ptr);
		this->pOwner = (Player*)ptr;
	}
}
