#ifndef SACK_H
#define SACK_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"
#include "token.h"


struct Inventory;


class Sack : public Token {
public:
    DECLARE_SERIAL(Sack); // Runtime class definition at 637318.

public: // VTable at 60f640.
    virtual ~Sack(); // 553f6a
    virtual void Serialize(CArchive& ar) override; // 55c889
    virtual void VMethod1() override; // 553f7c
    virtual void VMethod2() override; // 553f8b

public:
    Sack();
    Sack(const TokenPos* pos);
    Sack(const TokenPos* pos, Inventory* inv);

    void Init(); // 553eee
    void sub_55401E(); // 55401E
    void sub_4F9B9E(); // 4F9B9E

public:
    int32_t money;
    Inventory* inventory;
    uint32_t field_0x44;
    uint16_t field_0x48;
    uint16_t field_0x4a;
    uint16_t field_0x4c;
    uint8_t gap_0x4e[2];
};
ASSERT_OFFSET(Sack, inventory, 0x40);
ASSERT_SIZE(Sack, 0x50);


#endif
