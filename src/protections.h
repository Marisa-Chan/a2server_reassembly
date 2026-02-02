#ifndef PROTECTIONS_H
#define PROTECTIONS_H

#include <cstdint>
#include <array>

#include "asm_mfc.h"
#include "assert_offset.h"

struct Protections {
    uint16_t defense;
    uint16_t absorption;
    std::array<uint16_t, 6> magic_protections;
    std::array<uint8_t, 6> weapon_protections;

    Protections();
    Protections(const Protections &);
    Protections(Protections &&);

    Protections& operator=(const Protections &);
    Protections& operator+=(const Protections &);
    Protections& operator-=(const Protections &);

    void Clear();

    void Serialize(CArchive& ar);
};
ASSERT_SIZE(Protections, 0x16);

#endif
