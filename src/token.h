#ifndef TOKEN_H
#define TOKEN_H

#include "asm_mfc.h"
#include "mfc_templ.h"


#include <inttypes.h>



class Effect;
class Player;
class TokenPos;

__pragma(pack(push, 1))

struct astruct_5;
extern astruct_5 *astruct_5_Instance;

struct TokenPos
{
	TokenPos(uint8_t _x, uint8_t _y, astruct_5* _inst);

	TokenPos();
	TokenPos(uint16_t _YX, astruct_5* _inst);
	TokenPos(const TokenPos&);
	TokenPos(TokenPos&&);

	TokenPos& operator=(const TokenPos&);

	uint8_t x;
	uint8_t y;
	uint16_t YX;
	uint8_t field_x4;
	uint8_t field_x5;
	uint8_t field_x6;
	uint8_t field_x7;
	astruct_5* pInstance;

	void FUN_00594200(); // in asm
};



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
	int16_t typeId;
	TokenPos* position = nullptr;
	Player* pOwner = nullptr;
	int16_t field_x18;
	int8_t field_x1a;
	int8_t field_x1b;
	int32_t exp;
	CList<Effect*> _effects;

public:

	// Not const because AFX_CLASSINIT will write into it
	static AFX_DATA CRuntimeClass classToken;
};
__pragma(pack(pop))

#endif //TOKEN_H