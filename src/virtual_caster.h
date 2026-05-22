#pragma once

#include <cstdint>

#include "assert_offset.h"
#include "token.h"


class VirtualCaster : public Token {
public:
    DECLARE_SERIAL(VirtualCaster); // Runtime class definition at 6363d0.

public: // VTable at 0060ef88.
    virtual ~VirtualCaster(); // 528993
    virtual void Serialize(CArchive& ar) override; // 55D009

public:
    VirtualCaster(); // 528900

public:
    uint8_t byte_0x3c;
    uint8_t gap_0x3d[3];
    uint8_t* properties; // Initialized with a 6-byte array.
};
ASSERT_OFFSET(VirtualCaster, properties, 0x40);
ASSERT_SIZE(VirtualCaster, 0x44);
