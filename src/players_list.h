#pragma once

#include <cstdint>
#include "asm_mfc.h"

#include "assert_offset.h"
#include "mfc_templ.h"


class Player;


class PlayersList : public CList<Player*> {
public: // VTable at 0060f158.
    // virtual ~PlayersList();
    // virtual void Serialize(CArchive& ar);

public:
    uint32_t field_0x1c;
    uint32_t next_player_id;

public:
    void FUN_00534ddd();
    int CountHumanPlayers(); // I guess it counts current human players?
    int sub_53636E();        // checks if team play is already started
    void sub_5357C6(Player* player); // AddTail: append player to list
    Player* sub_535D39(CString name);    // find player by name (CString arg_0 compared against player->name)
};
ASSERT_SIZE(PlayersList, 0x24);
