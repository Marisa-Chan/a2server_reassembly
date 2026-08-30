#ifndef PLAYER_FILE_H
#define PLAYER_FILE_H


#include <array>
#include <cstdint>

#include "assert_offset.h"


class CFile;
class PacketUnitStateVec;


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

void __cdecl DecryptEncryptedData(uint8_t* data, int32_t size, uint16_t key); //4f535e
uint32_t __cdecl CalcChecksum(uint8_t* data, int32_t size); //4f5308

int32_t __cdecl ParsePlayerFile_4F62E6(
    CFile* file,
    FileSectionBasicInfo** basic_info,
    FileSectionStats** stats,
    uint8_t** kill_stats,
    PacketUnitStateVec** equipment,
    PacketUnitStateVec** inventory,
    uint8_t** section_40a,
    uint32_t* size_40a
);

int32_t __cdecl WritePlayerFile_4F53EA(
    const char* filename,
    FileSectionBasicInfo* basic_info,
    FileSectionStats* stats,
    uint8_t* kill_stats,
    PacketUnitStateVec* equip_pkt,
    PacketUnitStateVec* inv_pkt,
    uint8_t* section_40a,
    uint32_t size_40a
);


#endif
