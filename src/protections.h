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

    Protections();  // client: 51f203
    Protections(const Protections &);  // client: 51f234
    Protections(Protections &&);

    Protections& operator=(const Protections &);
    Protections& operator+=(const Protections &);
    Protections& operator-=(const Protections &);

    void Clear();  // client: 51f219

    void Serialize(CArchive& ar);  // client: 51f42a
};
ASSERT_SIZE(Protections, 0x16);

#endif
