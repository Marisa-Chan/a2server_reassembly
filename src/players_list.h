#pragma once

#include <cstdint>
#include "asm_mfc.h"

#include "assert_offset.h"
#include "mfc_templ.h"


class Player;


class PlayersList
{
public:
    CList<Player*> list;

    uint32_t field_0x1c = 0;
    uint32_t next_player_id = 1; // set it here instead of 534b9c constructor

public:
    int CountHumanPlayers(); // I guess it counts current human players?
    int sub_53636E();        // checks if team play is already started
    void sub_534DDD();
    void sub_5357C6(Player* player); // AddTail: append player to list
    Player* sub_535D39(CString name);    // find player by name (CString arg_0 compared against player->name)
    Player* sub_535B50(uint16_t player_id);   // find player by numeric id
    Player* sub_535E94(int32_t a, int32_t b);  // find player where field_0xa80==a && field_0xa84==b (spectator claim)

    Player* sub_535C46(uint16_t token_id); // GetByTokenID
    int CountCD(); //536092
    int32_t sub_535F97();  // returns max player_id in list
    void sub_536286(); // Reset all players' frags to 0

    ~PlayersList(); //534c02
};
ASSERT_SIZE(PlayersList, 0x24);
