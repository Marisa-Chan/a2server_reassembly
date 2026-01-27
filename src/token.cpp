#include "token.h"

//6b16a8
astruct_5* astruct_5_Instance = nullptr;

//70a7e0
BitSet<0x6000> g_buildingIdSet;

extern "C"
{
	void BldIdSet_Set(uint32_t index) { g_buildingIdSet.Set(index); }
	void BldIdSet_Unset(uint32_t index) { g_buildingIdSet.Unset(index); }
	void BldIdSet_Clear() { g_buildingIdSet.Clear(); }
	uint32_t BldIdSet_AllocBit() { return g_buildingIdSet.AllocBit(); }
}




TokenPos::TokenPos(uint8_t _x, uint8_t _y, astruct_5* _inst)
{
	//58a4b1 ?
	//58a4f9
	x = _x;
	y = _y;
	YX = ((uint16_t)_y << 8) | _x;
	x_subcell = 128;
	y_subcell = 128;
	pInstance = _inst;
}

TokenPos::TokenPos()
{ 
	//58a495
	TokenPos(0, 0, nullptr); 
}

TokenPos::TokenPos(uint16_t _YX, astruct_5* _inst) 
{
	//58a562
	//58a583
	TokenPos(_YX & 8, _YX >> 8, _inst); 
}

//58a9ee
TokenPos::TokenPos(const TokenPos&) = default;
TokenPos::TokenPos(TokenPos&&) = default;

//58aa46
TokenPos& TokenPos::operator=(const TokenPos&) = default;

void TokenPos::Serialize(CArchive& ar)
{
	//58ab11
	if (ar.IsStoring())
	{
		//ar.Write(this, sizeof(TokenPos));
		ar.Write(this, 0xc);
	}
	else
	{
		//ar.Read(this, sizeof(TokenPos));
		ar.Read(this, 0xc);
	}
}

uint8_t TokenPos::GetX() const
{
	//58aaf0
	return x;
}

uint8_t TokenPos::GetY() const
{
	//58ab00
	return y;
}

uint16_t TokenPos::GetYX() const
{
	//58aade
	return YX;
}

uint16_t TokenPos::GetXx() const
{
	//58aa9b
	return (x << 8) | x_subcell;
}

uint16_t TokenPos::GetYy() const
{
	//58aabc
	return (y << 8) | y_subcell;
}

uint8_t TokenPos::Distance(const TokenPos* b) const
{
	//58a6bf
	uint8_t xLen = abs((int32_t)x - (int32_t)b->x);
	uint8_t yLen = abs((int32_t)y - (int32_t)b->y);
	if (xLen <= yLen)
		return yLen;
	return xLen;
}

bool TokenPos::IsSameYX(const TokenPos* b) const
{
	//58a5b3
	return YX == b->YX;
}

void TokenPos::SetCoords(uint8_t _x, uint8_t _y)
{
	//58a737
	x = _x;
	y = _y;
	YX = (_y << 8) | _x;
	x_subcell = 128;
	y_subcell = 128;
}

void TokenPos::SetCoords2(uint16_t Xx, uint16_t Yy)
{
	//58a781
	x = Xx >> 8;
	y = Yy >> 8;
	YX = (y << 8) | x;
	x_subcell = Xx & 0xff;
	y_subcell = Yy & 0xff;
}





Token::Token()
{
	// 5280ad
	pOwner = nullptr;
	position = new TokenPos(0, astruct_5_Instance);
	field_x18 = 0;
	exp = 0;
	building_id = 0;
}

Token::Token(const Token& b)
{
	// 5285c6
	itemDataID = b.itemDataID;
	typeId = b.typeId;
	pOwner = b.pOwner;
	TokenID = b.TokenID;
	field_x18 = b.field_x18;
	exp = b.exp;
	//building_id = b.building_id;  // No copy in vanilla?
	if (b.position == nullptr)
		position = nullptr;
	else
		position = new TokenPos(*b.position);
}

//Token::Token(Token&&) = default;

Token::Token(const TokenPos* tpos)
{
	// 5281af
	pOwner = nullptr;
	position = new TokenPos(*tpos);
	field_x18 = 0;
	exp = 0;
	building_id = 0;
}

Token::Token(const TokenPos* tpos, Player* pl)
{
	// 5282af
	pOwner = pl;
	position = new TokenPos(*tpos);
	field_x18 = 0;
	exp = 0;
	building_id = 0;
}

// 6363b8
AFX_DATA CRuntimeClass Token::classToken =
{ "Token", sizeof(Token), 1, &Token::CreateObject, &CObject::classCObject, NULL };

CObject* PASCAL Token::CreateObject()
{ 
	// 527ff6
	return new Token;
}

CRuntimeClass* Token::GetRuntimeClass() const
{
	// 528063
	return (CRuntimeClass*) &classToken;
}

Token::~Token()
{
	// 5283d6
	if (position)
		delete position;
}

//void Token::Serialize(CArchive& ar)
//{
//
//}

void Token::VMethod1()
{
	// 57b290
}

void Token::VMethod2()
{
	// 52871a
}

int32_t Token::VMethod3()
{
	// 57b2a0
	return 1;
}

int32_t Token::VMethod4()
{
	// 57b2b0
	return 0;
}

void Token::VMethod5()
{
	// 55af48
	position->FUN_00594200();
}

int32_t Token::VMethod6()
{
	// 57b2c0
	return 0;
}

int32_t Token::VMethod7()
{
	// 57b2d0
	return 0;
}

int32_t Token::VMethod8()
{
	// 57b2e0
	return 0;
}

int32_t Token::VMethod9()
{
	// 57b2f0
	return 0;
}



static const AFX_CLASSINIT _init_Token((CRuntimeClass*)&Token::classToken);
