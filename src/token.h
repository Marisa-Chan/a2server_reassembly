#ifndef TOKEN_H
#define TOKEN_H

#include <inttypes.h>

#include <afx.h>
#include <afxtempl.h>

#include "assert_offset.h"
#include "bitset.h"


extern BitSet<0x6000> g_buildingIdSet;

void BldIdSet_Unset(uint32_t index); // 509cfb (client: 501896)

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
    TokenPos(uint8_t _x, uint8_t _y, MapStuff* _inst); // 58a4f9 (client: 5599a9)

    TokenPos(); // 58a495 (client: 559945)
    TokenPos(uint16_t _YX, MapStuff* _inst); // 58a562 (client: 559a12)
    TokenPos(const TokenPos&); // 58a9ee (client: 559e9e)
    TokenPos(TokenPos&&);  // client: 559e9e

    TokenPos& operator=(const TokenPos&); // 58aa46 (client: 559ef6)

    void Set(uint8_t _x, uint8_t _y, MapStuff* _inst); // 58a4b1 (client: 559961)

    void FixupInstance(); // 594200 (client: 563671)
    int ClampToMap(uint8_t x, uint8_t y); // 58a7e8 (client: 559c98)

    void Serialize(CArchive& ar); // 58ab11 (client: 559fc1)

    uint8_t GetX() const; // 58aaf0 (client: 559fa0)
    uint8_t GetY() const; // 58ab00 (client: 559fb0)
    PosYX GetYX() const; // split from 58aade (client: 559f8e)
    uint16_t CompatGetYX() const; // 58aade (client: 559f8e) - ABI compatibility wrapper
    uint16_t GetXx() const; // 58aa9b (client: 559f4b)
    uint16_t GetYy() const; // 58aabc (client: 559f6c)

    uint8_t Distance(const TokenPos* b) const; // 58a6bf (client: 559b6f)
    int IsSameYX(const TokenPos* b) const; // 58a5b3 (client: 559a63)

    void SetCoords(uint8_t _x, uint8_t _y); // 58a737 (client: 559be7)
    void SetCoords2(uint16_t Xx, uint16_t Yy); // 58a781 (client: 559c31)

    int32_t IsAtCellCenter(); // 58bec3 (client: 55b373) - Check if we'in the middle of a cell (both subcell values are 0x80).
};

ASSERT_OFFSET(TokenPos, x_subcell, 0x4);
ASSERT_SIZE(TokenPos, 0xC);



class Token : public CObject {
public:
    DECLARE_SERIAL(Token); // Runtime class definition at 6363b8.

public:
    virtual ~Token(); // 5283d6 (client: 5172c7)
    virtual void Serialize(CArchive& ar); // 55ab39 (client: 536caf)
    // CObject AssertValid
    // CObject Dump
    virtual void RegenTick(); // 57b290 (client: 54bfc0)
    virtual void ProcessEffects(); // 52871a (client: 51746a)
    virtual int32_t GetTokenSize(); // 57b2a0 (client: 54bfd0) - TokenSize
    virtual int32_t GetMovementType(); // 57b2b0 (client: 54bfe0) - MovementType
    virtual void FixupPointers(); // 55af48 (client: 536e27)
    virtual int32_t IsItem(); // 57b2c0 (client: 54bff0) - IsItem
    virtual int32_t IsUnit(); // 57b2d0 (client: 54c000) - IsUnit
    virtual int32_t IsHumanoid(); // 57b2e0 (client: 54c010) - IsHumanoid
    virtual int32_t IsBuilding(); // 57b2f0 (client: 54c020) - IsBuilding

public:
    Token(); // 5280ad (client: 517084)
    Token(const Token&); // 5285c6 (client: 517363)
    // Token(Token&&);
    Token(const TokenPos* tpos); // 5281af (client: 51713a)
    Token(const TokenPos* tpos, Player *pl); // 5282af (client: 5171ed)

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
