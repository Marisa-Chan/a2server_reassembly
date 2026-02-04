#ifndef SACK_H
#define SACK_H

#include <cstdint>

#include "assert_offset.h"
#include "mfc_templ.h"
#include "token.h"


class Inventory;


class Sack : public Token {
public: // VTable at 0060f640.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~Sack();
    // virtual void Serialize(CArchive& ar);
    // CObject* AssertValid();
    // CObject* Dump(CDumpContext& dc) const;
    // virtual void VMethod1();
    // virtual void VMethod2();

    // Other Token's virtual methods are unchanged.

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
