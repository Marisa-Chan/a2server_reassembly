#ifndef TOKEN_H
#define TOKEN_H

#include <inttypes.h>

#include <afx.h>
#include <afxtempl.h>

#include "assert_offset.h"
#include "bitset.h"


extern BitSet<0x6000> g_buildingIdSet;

void BldIdSet_Unset(uint32_t index);

class Effect;
class Player;
struct TokenPos;

__pragma(pack(push, 1))

struct MapStuff;
extern MapStuff *MapStuff_Instance;

struct PosYX {
	union {
		uint16_t val;
		struct {
			uint8_t x;
			uint8_t y;
		};
	};

	inline void Set(uint8_t _x, uint8_t _y) {
		this->x = _x;
		this->y = _y;
	}

	inline void Set(uint16_t _val) {
		this->val = _val;
	}

	inline PosYX() { this->val = 0; }
	inline PosYX(uint16_t _val) { this->val = _val; }
	inline PosYX(uint8_t _x, uint8_t _y) {
		this->x = _x;
		this->y = _y;
	}

	inline bool operator==(const PosYX b) const { return this->val == b.val; }
	inline bool operator!=(const PosYX b) const { return this->val != b.val; }

	inline void operator=(uint16_t _val) { this->val = _val; }
	inline operator uint16_t() { return this->val; }
};

struct TokenPos {
public:
	uint8_t x;
	uint8_t y;
	PosYX YX;
	uint8_t x_subcell; // When a unit is moving, this holds the subcell position in 1/256ths of a cell. Middle of the cell is 128.
	uint8_t y_subcell;
	uint8_t gap_x6;
	uint8_t gap_x7;
	MapStuff* pInstance;

public:
	TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst);  // client: 5599a9

	TokenPos();  // client: 559945
	TokenPos(uint16_t _YX, MapStuff* _inst);  // client: 559a12
	TokenPos(const TokenPos&);  // client: 559e9e
	TokenPos(TokenPos&&);  // client: 559e9e

	TokenPos& operator=(const TokenPos&);

	void Set(uint8_t _x, uint8_t _y, MapStuff* _inst); //58a4b1 (client: 559961)

	void FixupInstance(); // in asm (client: 563671)
	int ClampToMap(uint8_t x, uint8_t y); //in asm (client: 559c98)

	void Serialize(CArchive& ar);  // client: 51b0f8

	uint8_t GetX() const;  // client: 559fa0
	uint8_t GetY() const;  // client: 559fb0
	PosYX GetYX() const;
	uint16_t CompatGetYX() const; //only for compatible with asm (client: 559f8e)
	uint16_t GetXx() const;  // client: 559f4b
	uint16_t GetYy() const;  // client: 559f6c

	uint8_t Distance(const TokenPos* b) const;  // client: 559b6f
	int IsSameYX(const TokenPos* b) const;  // client: 559a63

	void SetCoords(uint8_t _x, uint8_t _y);  // client: 559be7
	void SetCoords2(uint16_t Xx, uint16_t Yy);  // client: 559c31

	int32_t IsAtCellCenter(); // Check if we'in the middle of a cell (both subcell values are 0x80).
};

ASSERT_OFFSET(TokenPos, x_subcell, 0x4);
ASSERT_SIZE(TokenPos, 0xC);



class Token : public CObject {
public:
	DECLARE_SERIAL(Token); // Runtime class definition at 6363b8.

public:
	virtual ~Token();  // client: 5172c7
	virtual void Serialize(CArchive& ar); //55ab39 (client: 536caf)
	// CObject AssertValid
	// CObject Dump
	virtual void RegenTick();  // client: 54bfc0
	virtual void ProcessEffects();  // client: 51746a
	virtual int32_t GetTokenSize(); // TokenSize (client: 54bfd0)
	virtual int32_t GetMovementType(); // MovementType (client: 54bfe0)
	virtual void FixupPointers();  // client: 536e27
	virtual int32_t IsItem(); // IsItem (client: 54bff0)
	virtual int32_t IsUnit(); // IsUnit (client: 54c000)
	virtual int32_t IsHumanoid(); // IsHumanoid (client: 54c010)
	virtual int32_t IsBuilding(); // IsBuilding (client: 54c020)

public:
	Token();  // client: 517363
	Token(const Token&);  // client: 5171ed
//	Token(Token&&);
	Token(const TokenPos* tpos);  // client: 517084
	Token(const TokenPos* tpos, Player *pl);  // client: 51713a

public:
	int32_t building_id;
	int32_t TokenID;
	int16_t itemDataID;
	uint16_t typeId;
	TokenPos* position = nullptr;
	Player* pOwner = nullptr;
	uint16_t sent_to_mask;
	int8_t gap_x1a;
	int8_t gap_x1b;
	int32_t _exp;
	CList<Effect*> _effects;
};
__pragma(pack(pop))

ASSERT_OFFSET(Token, pOwner, 0x14);
ASSERT_SIZE(Token, 0x3C);

#endif //TOKEN_H
