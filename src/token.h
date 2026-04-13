#ifndef TOKEN_H
#define TOKEN_H

#include <inttypes.h>

#include "asm_mfc.h"
#include "mfc_templ.h"

#include "bitset.h"


extern BitSet<0x6000> g_buildingIdSet;


class Effect;
class Player;
struct TokenPos;

__pragma(pack(push, 1))

struct MapStuff;
extern MapStuff *MapStuff_Instance;

struct TokenPos
{
public:
	uint8_t x;
	uint8_t y;
	uint16_t YX;
	uint8_t x_subcell; // When a unit is moving, this holds the subcell position in 1/256ths of a cell. Middle of the cell is 128.
	uint8_t y_subcell;
	uint8_t field_x6;
	uint8_t field_x7;
	MapStuff* pInstance;

public:
	TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst);

	TokenPos();
	TokenPos(uint16_t _YX, MapStuff* _inst);
	TokenPos(const TokenPos&);
	TokenPos(TokenPos&&);

	TokenPos& operator=(const TokenPos&);

	void FUN_00594200(); // in asm
	int FUN_0058a7e8(uint8_t x, uint8_t y); //in asm

	void Serialize(CArchive& ar);

	uint8_t GetX() const;
	uint8_t GetY() const;
	uint16_t GetYX() const;
	uint16_t GetXx() const;
	uint16_t GetYy() const;

	uint8_t Distance(const TokenPos* b) const;
	int IsSameYX(const TokenPos* b) const;

	void SetCoords(uint8_t _x, uint8_t _y);
	void SetCoords2(uint16_t Xx, uint16_t Yy);

	int32_t sub_58bec3(); // Check if we'in the middle of a cell (both subcell values are 0x80).
};

ASSERT_OFFSET(TokenPos, x_subcell, 0x4);
ASSERT_SIZE(TokenPos, 0xC);



class Token : public CObject
{
public:
	virtual CRuntimeClass* GetRuntimeClass() const;

	virtual ~Token();
	virtual void Serialize(CArchive& ar);
	// CObject AssertValid
	// CObject Dump
	virtual void VMethod1();
	virtual void VMethod2();
	virtual int32_t VMethod3();
	virtual int32_t VMethod4();
	virtual void VMethod5();
	virtual int32_t VMethod6();
	virtual int32_t VMethod7();
	virtual int32_t VMethod8();
	virtual int32_t VMethod9();

public:
	Token();
	Token(const Token&);
//	Token(Token&&);
	Token(const TokenPos* tpos);
	Token(const TokenPos* tpos, Player *pl);

	static CObject* PASCAL CreateObject();

public:
	int32_t building_id;
	int32_t TokenID;
	int16_t itemDataID;
	uint16_t typeId;
	TokenPos* position = nullptr;
	Player* pOwner = nullptr;
	int16_t field_x18;
	int8_t field_x1a;
	int8_t field_x1b;
	int32_t _exp;
	CList<Effect*> _effects;

public:

	// Not const because AFX_CLASSINIT will write into it
	static AFX_DATA CRuntimeClass classToken;
};
__pragma(pack(pop))

ASSERT_OFFSET(Token, pOwner, 0x14);
ASSERT_SIZE(Token, 0x3C);

#endif //TOKEN_H
