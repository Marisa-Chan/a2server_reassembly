#pragma once

#include <cstdint>

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
};
ASSERT_SIZE(PlayersList, 0x24);
