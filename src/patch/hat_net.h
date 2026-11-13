#pragma once

// Tangar HAT control-connection layer, ported from my-srvmgr/protolayer_hat.
// {h,cpp}. The server maintains one outbound connection to the HAT master
// (address from server.cfg [network]/HatAddress): it authenticates, pushes
// periodic server-info updates, and receives broadcasts / mute orders /
// screenshot notifications.

#include <cstdint>
#include <string>

#include "hat_packet.h"

namespace tangar_hat {

struct ServerInfo {
    uint32_t PlayerCount;
    uint32_t MapLevel;
    std::string MapName;
    uint32_t GameMode;
    uint32_t MapSize;
};

// Read HatAddress/HatAddress2 from the parsed server config into the
// connection targets (Net_HatPrepare in the manager).
void NetHatPrepare();

// Connection lifecycle.
bool NetHatInit();
bool NetHatProcess();
void NetHatShutdown();

// One pump call per server tick (Net_RegularProc): process + reconnect.
void NetRegularProc();

// Outbound commands.
bool NetCmdHatAuth();
bool NetCmdShutdown();
bool NetCmdUpdateInfo();
bool NetCmdBroadcast(const std::string& what);
bool NetCmdUnlockLogin(const std::string& login);

// Lobby-info update hook equivalent (imp_UpdateInfo): snapshot then send.
void NetHatUpdateInfo(uint8_t player_count, const char* name, uint8_t level, uint8_t type, uint8_t size);

// Game-socket extended-packet framing hooks (my-srvmgr send0 / recv0):
// FlushEnqueuedPackets drains PlayerInfo::EnqueuedPackets as 0x80100000 frames
// before normal game sends; RecvServerFramed demultiplexes incoming 0x80100000
// manager frames from normal 8-byte-headed game packets (returns -2 when an
// extended packet was consumed and no trailing packet is immediately queued).
void FlushEnqueuedPackets(SOCKET s);
int RecvServerFramed(SOCKET s, char* buf, int len, int flags);

} // namespace tangar_hat
