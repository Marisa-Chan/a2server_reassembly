// Ported from my-srvmgr/screenshots.cpp.

#include "screenshots.h"

#include <ctime>
#include <vector>

#include "config_new.h"

namespace {

std::vector<ClientScreenshot> g_screenshots;
uint32_t g_screenshot_uid = 0;

} // namespace

uint32_t ClientScreenshot_Enqueue(Player* gm, Player* target) {
    ClientScreenshot cs;
    cs.SourcePlayer = gm;
    cs.TargetPlayer = target;
    cs.RequestedAt = static_cast<uint32_t>(time(nullptr));
    cs.UID = (static_cast<uint32_t>(Config::IPAddressP) << 16) | ((++g_screenshot_uid) & 0xFFFFu);
    g_screenshots.push_back(cs);
    return cs.UID;
}

ClientScreenshot* ClientScreenshot_FindByUID(uint32_t uid) {
    for (ClientScreenshot& cs : g_screenshots) {
        if (cs.UID == uid) {
            return &cs;
        }
    }
    return nullptr;
}

void ClientScreenshot_Drop(uint32_t uid) {
    for (auto it = g_screenshots.begin(); it != g_screenshots.end();) {
        if (it->UID == uid) {
            it = g_screenshots.erase(it);
        } else {
            ++it;
        }
    }
}

void ClientScreenshot_DropPlayer(Player* player) {
    for (ClientScreenshot& cs : g_screenshots) {
        if (cs.SourcePlayer == player) {
            cs.SourcePlayer = nullptr;
        }
        if (cs.TargetPlayer == player) {
            cs.TargetPlayer = nullptr;
        }
    }
}
