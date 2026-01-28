#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>

#include "asm_mfc.h"
#include "mfc_templ.h"


// TODO: move to its own file and populate.
struct Group {};

// TODO: move to its own file and populate.
struct UnitList {};

struct Settings {
    uint8_t some_fields[0x1f];
    uint8_t formation;
    uint8_t autobuff_mask;
    uint8_t more_fields[3];
};
static_assert(offsetof(Settings, autobuff_mask) == 0x20, "Settings::autobuff_mask offset mismatch");
static_assert(sizeof(Settings) == 0x24, "Settings size mismatch");

class Unit;
class Human;

class Player : public CObject {
public:
    // virtual CRuntimeClass* GetRuntimeClass() const;
    // virtual ~Player();
    // virtual void Serialize(CArchive& ar);
    // CObject AssertValid
    // CObject Dump

    static AFX_DATA CRuntimeClass classPlayer;

public:
    uint16_t player_id;
    uint8_t gap_0x6[2];
    uint32_t token_id;
    uint8_t gap_0xc[4];
    uint32_t hat_player_id;
    uint32_t flags;
    const char* name;
    const char* str;
    uint8_t gap_0x20[4];
    UnitList* unit_list;
    CList<Group*>* group_list;
    uint32_t is_ai; // 1 for AI players, 0 for human players.
    uint16_t vision_sharing_id; // This player's ID in a vision-sharing bitmask.
    uint16_t vision_sharing_mask; // This player's vision-sharing bitmask.
    Settings* settings;
    Human* main_unit;
    int32_t money;
    uint8_t gap_0x40[4];
    uint8_t kill_stats[2560]; // Indexed by monster type ID.
    uint8_t field_0xa44;
    uint8_t field_0xa45;
    uint8_t gap_0xa46[2];
    uint32_t monster_kills;
    uint32_t player_kills;
    uint32_t field_0xa50;
    uint32_t deaths;
    uint32_t frags;
    uint32_t field_0xa5c;
    uint32_t field_0xa60;
    uint32_t field_0xa64;
    uint32_t field_0xa68;
    uint32_t field_0xa6c;
    uint32_t field_0xa70;
    uint16_t building_entered_from_yx;
    uint8_t field_0xa76;
    uint8_t field_0xa77;
    const char* str2;
    uint32_t field_0xa7c;
    uint32_t field_0xa80;
    uint32_t field_0xa84;
    uint32_t min_server_level;
    uint32_t max_server_level;
    uint32_t field_0xa90;
    uint32_t field_0xa94;
    uint8_t field_0xa98;
    uint8_t gap_0xa99[7];
};

static_assert(offsetof(Player, flags) == 0x14, "Player::flags offset mismatch");
static_assert(offsetof(Player, kill_stats) == 0x44, "Player::kill_stats offset mismatch");
static_assert(offsetof(Player, min_server_level) == 0xa88, "Player::min_server_level offset mismatch");
static_assert(sizeof(Player) == 0xaa0, "Player size mismatch");


#endif
