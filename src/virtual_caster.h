#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"


class VirtualCaster : public Token {
public: // VTable at 0060ef88.
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~VirtualCaster();
    // virtual void Serialize(CArchive& ar);

public:
    uint8_t byte_0x3c;
    uint8_t gap_0x3d[3];
    uint8_t* properties; // Initialized with a 6-byte array.
    uint8_t gap_0x44[20];
};
ASSERT_OFFSET(VirtualCaster, properties, 0x40);
ASSERT_SIZE(VirtualCaster, 0x58);
