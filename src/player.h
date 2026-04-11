#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>

#include "asm_mfc.h"
#include "assert_offset.h"
#include "mfc_templ.h"


class UnitList;
struct GroupList;


struct Settings {
    uint8_t some_fields[0x1f];
    uint8_t formation;
    uint8_t autobuff_mask;
    uint8_t more_fields[3];

public:
    Settings();
    void Serialize(CArchive& ar);
};
ASSERT_OFFSET(Settings, autobuff_mask, 0x20);
ASSERT_SIZE(Settings, 0x24);

class Unit;
class Human;

class Player : public CObject {
public: // VTable at 0060f140.

    DECLARE_SERIAL(Player);

    virtual ~Player();
    virtual void Serialize(CArchive& ar);

public:
    uint16_t player_id;
    uint8_t gap_0x6[2];
    uint32_t token_id;
    uint8_t gap_0xc[4];
    uint32_t hat_player_id;
    uint32_t flags;
    CString name;
    CString str;
    int32_t field_0x20;
    UnitList* unit_list;
    GroupList* group_list;
    uint32_t is_ai; // 1 for AI players, 0 for human players.
    uint16_t vision_sharing_id; // This player's ID in a vision-sharing bitmask.
    uint16_t vision_sharing_mask; // This player's vision-sharing bitmask.
    Settings* settings;
    Human* main_unit;
    int32_t money;
    uint8_t field_0x40;
    uint8_t field_0x41;
    uint8_t field_0x42;
    uint8_t field_0x43;
    uint8_t kill_stats[2560]; // Indexed by monster type ID.
    uint8_t field_0xa44;
    uint8_t field_0xa45;
    uint8_t gap_0xa46[2];
    uint32_t monster_kills;
    uint32_t player_kills;
    int32_t field_0xa50;
    uint32_t deaths;
    uint32_t frags;
    uint32_t field_0xa5c;
    uint32_t field_0xa60;
    uint32_t field_0xa64;
    int32_t field_0xa68;
    uint32_t field_0xa6c;
    uint32_t field_0xa70; // Team play: 0 for blue team, 1 for red team.
    uint16_t building_entered_from_yx;
    uint8_t field_0xa76;
    uint8_t field_0xa77;
    CString login;
    uint32_t field_0xa7c;
    uint32_t field_0xa80;
    uint32_t field_0xa84;
    int32_t min_server_level;
    int32_t max_server_level;
    uint32_t field_0xa90;
    uint32_t field_0xa94;
    uint8_t field_0xa98;
    uint8_t gap_0xa99[7];

public:
    Player();

public:
    void sub_534AC1(int32_t amount, uint32_t notify_flag);
    void sub_5346AC(); // Kill all of this player's units on the map
    void sub_534B17(); // Initialize inactivity timer (sets this+0A90h/0A94h from GetTickCount)
    void sub_534B59(); // Reset inactivity timer and notify NetStru1

    void FUN_00534778(); //534778

    void sub_5347A1(Unit* unit); // Remove a unit from this player's unit list (PlayerRemoveUnit)
};

ASSERT_OFFSET(Player, flags, 0x14);
ASSERT_OFFSET(Player, kill_stats, 0x44);
ASSERT_OFFSET(Player, min_server_level, 0xa88);
ASSERT_SIZE(Player, 0xaa0);


#endif
