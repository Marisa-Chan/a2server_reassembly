#pragma once

#include <cstdint>
#include <afx.h>

#include "assert_offset.h"
#include <afxtempl.h>


class Player;


class PlayersList {
public:
    CList<Player*> list;

    uint32_t gap_0x1c = 0;
    uint32_t next_player_id = 1; // set it here instead of 534b9c constructor

public:
    int CountHumanPlayers(); // 53618f (client: 5205cf) - I guess it counts current human players?
    int AreTeamsStarted(); // 53636e (client: none) - checks if team play is already started
    void ProcessPlayerTimeouts(); // 534ddd (client: 51ff32)
    void AddPlayer(Player* player); // 5357c6 (client: 520219) - AddTail: append player to list
    Player* FindByName(CString name); // 535d39 (client: 52048d) - find player by name (name compared against player->name)
    Player* FindById(uint16_t player_id); // 535b50 (client: 520402) - find player by numeric id
    Player* FindByHatCharId(int32_t a, int32_t b); // 535e94 (client: none) - find player by HAT character id (hat_char_id1==a && hat_char_id2==b)

    Player* FindByTokenId(uint16_t token_id); // 535c46 (client: 520449) - GetByTokenID
    int CountCD(); // 536092 (client: 52057e)
    int32_t GetMaxPlayerId(); // 535f97 (client: 52052f) - returns max player_id in list
    void ResetFrags(); // 536286 (client: none) - Reset all players' frags to 0
    void Serialize(CArchive& ar); // 55afb3 (client: 536e8d) - Serialize

    Player* GetHead(); // 5b70f0 (client: 585150)
    Player* GetNext(Player* player); // 5b7110 (client: 547970) - Avoid this O(N^2) crap, use normal iteration over `list`.

    ~PlayersList(); // 534c02 (client: 51feea)
};
ASSERT_SIZE(PlayersList, 0x24);
