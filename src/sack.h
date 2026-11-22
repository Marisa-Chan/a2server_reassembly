#ifndef SACK_H
#define SACK_H

#include <cstdint>

#include "assert_offset.h"
#include <afxtempl.h>
#include "token.h"


struct Inventory;


// 60f640
class Sack : public Token {
public:
    DECLARE_SERIAL(Sack); // Runtime class definition at 637318.

public: // VTable at 60f640.
    virtual ~Sack(); // 553f6a (client: 53451e)
    virtual void Serialize(CArchive& ar) override; // 55c889 (client: 537600)
    virtual void RegenTick() override; // 57dd30 (client: 54ee70)
    virtual void ProcessEffects() override; // 57dd40 (client: 54ee80)

public:
    Sack(); // 553d5d (client: 534311)
    Sack(const TokenPos* pos); // 553df1 (client: 5343a5)
    Sack(const TokenPos* pos, Inventory* inv); // 553e8b (client: 53443f)

    void Init(); // 553eee (client: 5344a2)
    void CalcContentsValue(); // 55401e (client: 5345b7)

public:
    int32_t money;
    Inventory* inventory;
    uint32_t needs_save;
    uint16_t seen_by_mask;
    uint16_t seen_by_mask_prev;
    uint16_t resend_mask;
    uint8_t gap_0x4e[2];
};
ASSERT_OFFSET(Sack, inventory, 0x40);
ASSERT_SIZE(Sack, 0x50);


#endif
