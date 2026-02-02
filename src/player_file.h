#ifndef PLAYER_FILE_H
#define PLAYER_FILE_H


#include <array>
#include <cstdint>

#include "assert_offset.h"


struct FileSectionBasicInfo {
    uint32_t id1;
    uint32_t id2;
    uint32_t hat_id;
    char nick[32];
    uint8_t character_class;
    uint8_t picture;
    uint8_t main_sphere;
    uint8_t flags;
    uint8_t color;
    uint8_t field_0x31;
    uint8_t field_0x32;
    uint8_t field_0x33;
};
ASSERT_OFFSET(FileSectionBasicInfo, main_sphere, 0x2e);
ASSERT_SIZE(FileSectionBasicInfo, 0x34);

struct FileSectionStats {
    uint32_t monster_kills;
    uint32_t player_kills;
    uint32_t frags;
    uint32_t deaths;
    uint32_t money;
    uint8_t body;
    uint8_t reaction;
    uint8_t mind;
    uint8_t spirit;
    uint32_t spells;
    uint32_t active_spell;
    std::array<uint32_t, 5> experience;
};
ASSERT_OFFSET(FileSectionStats, mind, 0x16);
ASSERT_SIZE(FileSectionStats, 0x34);

// See other types:
//  Server side: `ParsePlayerFile_004f62e6`,
//  Hat side: `CCharacter::SaveToStream`.


#endif
