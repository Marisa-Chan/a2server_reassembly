#ifndef PROTECTIONS_H
#define PROTECTIONS_H

#include <cstdint>
#include <array>

#include <afx.h>
#include "assert_offset.h"

struct Protections {
    int16_t defense;
    int16_t absorption;
    std::array<uint16_t, 6> magic_protections;
    std::array<uint8_t, 6> weapon_protections;

    Protections(); // 5339f8 (client: 51f203)
    Protections(const Protections &); // 533a29 (client: 51f234)
    Protections(Protections &&);

    Protections& operator=(const Protections &); // 533af0 (client: 51f2fb)
    Protections& operator+=(const Protections &); // 533a48 (client: 51f253)
    Protections& operator-=(const Protections &); // 533b77 (client: 51f382)

    void Clear(); // 533a0e (client: 51f219)

    void Serialize(CArchive& ar); // 533c1f (client: 51f42a)
};
ASSERT_SIZE(Protections, 0x16);

#endif
