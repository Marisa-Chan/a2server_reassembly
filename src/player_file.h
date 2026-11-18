#ifndef PLAYER_FILE_H
#define PLAYER_FILE_H


#include <array>
#include <cstdint>

#include "assert_offset.h"


class CFile;
class PacketUnitStateVec;


namespace player_file {
enum : uint32_t {
    A2C_MAGIC       = 0x04507989,
    SEC_BASIC_INFO  = 0xAAAAAAAA,
    SEC_KILL_STATS  = 0x55555555,
    SEC_UNKNOWN_40A = 0x40A40A40,
    SEC_EQUIPMENT   = 0xDE0DE0DE,
    SEC_STATS       = 0x41392521,
    SEC_INVENTORY   = 0x3A5A3A5A,
};
} // namespace player_file

struct CharacterClassCode {
    enum ClassIndex : uint8_t {
        CLASS_WARRIOR_MALE   = 0, // 0x00
        CLASS_MAGE_MALE      = 1, // 0x40
        CLASS_WARRIOR_FEMALE = 2, // 0x80 (Amazon)
        CLASS_MAGE_FEMALE    = 3, // 0xC0 (Sorceress / Witch)
    };

    union {
        uint8_t raw;
        struct {
            uint8_t portrait_index : 6; // [5:0] 0..63
            uint8_t is_mage        : 1; // [6]   0x40
            uint8_t is_female      : 1; // [7]   0x80
        };
        struct {
            uint8_t portrait_low   : 6; // [5:0]
            uint8_t class_index    : 2; // [7:6] ClassIndex
        };
    };

    constexpr CharacterClassCode(uint8_t v = 0) : raw(v) {}
    constexpr operator uint8_t() const { return this->raw; }
};
ASSERT_SIZE(CharacterClassCode, 1);

struct FileSectionBasicInfo {
    uint32_t id1;
    uint32_t id2;
    uint32_t hat_id;
    char nick[32];
    CharacterClassCode character_class;
    uint8_t picture;
    uint8_t main_sphere;
    uint8_t flags;
    uint8_t color;
    uint8_t gap_0x31;
    uint8_t gap_0x32;
    uint8_t gap_0x33;
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

// On-disk section header preceding each encrypted player-file section.
struct PlayerFileSectionHeader {
    uint32_t tag;
    uint32_t size;
    uint16_t reserved;
    uint16_t key;
    uint32_t checksum;
};


// See other types:
//  Server side: `ParsePlayerFile` (4f62e6),
//  Hat side: `CCharacter::SaveToStream`.

void __cdecl DecryptEncryptedData(uint8_t* data, int32_t size, uint16_t key); //4f535e (client: 4f68dd)
uint32_t __cdecl CalcChecksum(uint8_t* data, int32_t size); //4f5308 (client: 4f6887)

// 4f62e6
int32_t __cdecl ParsePlayerFile(
    CFile* file,
    FileSectionBasicInfo** basic_info,
    FileSectionStats** stats,
    uint8_t** kill_stats,
    PacketUnitStateVec** equipment,
    PacketUnitStateVec** inventory,
    uint8_t** section_40a,
    uint32_t* size_40a
);

// 4f53ea
int32_t __cdecl WritePlayerFile(
    const char* filename,
    FileSectionBasicInfo* basic_info,
    FileSectionStats* stats,
    uint8_t* kill_stats,
    PacketUnitStateVec* equip_pkt,
    PacketUnitStateVec* inv_pkt,
    uint8_t* section_40a,
    uint32_t size_40a
);

// Stats section from the most recent ParsePlayerFile call (the static buffer the
// retail client parses into, 0x6995a8 twin). Read by the Tangar client patch.
// TODO: rewrite in a better way.
FileSectionStats* GetParsedPlayerStats();

// Basic-info section from the most recent ParsePlayerFile call (client 0x69c058 twin).
FileSectionBasicInfo* GetParsedPlayerBasicInfo();


#endif
