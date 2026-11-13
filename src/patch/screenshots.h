#pragma once

// Tangar remote screenshot request tracker, ported from
// my-srvmgr/screenshots.{h,cpp}. Pairs each GM `#screenshot <nick>` request
// with a unique UID so HAT 0x64 completion callbacks can notify SourcePlayer.

#include <cstdint>

class Player;

struct ClientScreenshot {
    Player* SourcePlayer;
    Player* TargetPlayer;
    uint32_t RequestedAt;
    uint32_t UID;
};

uint32_t ClientScreenshot_Enqueue(Player* gm, Player* target);
ClientScreenshot* ClientScreenshot_FindByUID(uint32_t uid);
void ClientScreenshot_Drop(uint32_t uid);
void ClientScreenshot_DropPlayer(Player* player);
