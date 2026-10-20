#pragma once

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


class Player;


class PlayersList {
public:
    CList<Player*> list;

    uint32_t field_0x1c = 0;
    uint32_t next_player_id = 1; // set it here instead of 534b9c constructor

public:
    int CountHumanPlayers(); // I guess it counts current human players?
    int AreTeamsStarted();        // checks if team play is already started
    void ProcessPlayerTimeouts();
    void AddPlayer(Player* player); // AddTail: append player to list
    Player* FindByName(CString name);    // find player by name (CString arg_0 compared against player->name)
    Player* FindById(uint16_t player_id);   // find player by numeric id
    Player* FindByHatCharId(int32_t a, int32_t b);  // find player by HAT character id (hat_char_id1==a && hat_char_id2==b)

    Player* FindByTokenId(uint16_t token_id); // GetByTokenID
    int CountCD(); //536092
    int32_t GetMaxPlayerId();  // returns max player_id in list
    void ResetFrags(); // Reset all players' frags to 0
    void sub_55AFB3(CArchive& ar); // Serialize

    Player* GetHead(); // 5B70F0
    Player* GetNext(Player* player); // 5B7110. Avoid this O(N^2) crap, use normal iteration over `list`.

    ~PlayersList(); //534c02
};
ASSERT_SIZE(PlayersList, 0x24);
