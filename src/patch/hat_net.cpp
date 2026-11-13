// Ported from my-srvmgr/protolayer_hat.cpp. Map/player field mapping:
// map width/height = MapStuff_Instance->map_width/map_height (+0x50000/+0x50004),
// map time = g_Server->map_elapsed_time2 (+0x254)/1000, player id1 = hat_char_id1,
// id2 = flags, login = Player::login, IP = NetStru2::buf (the join-time address).

#include "hat_net.h"

#include <ctime>
#include <mutex>
#include <windows.h>

#include "config_new.h"
#include "flags.h"
#include "player_info.h"
#include "screenshots.h"
#include "tangar_log.h"
#include "tangar_utils.h"

#include "../game_app.h"
#include "../map_stuff.h"
#include "../net.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

namespace tangar_hat {

namespace {

std::string HatAddr;
uint16_t HatPort = 0;
std::string ControlAddr;
uint16_t ControlPort = 0;
SOCKET Socket = 0;
bool Connected = false;
uint32_t LastReconnect = 0;
PacketReceiver Receiver;
bool HaveInfo = false;
ServerInfo Info;
bool ShuttingDown = false;
uint32_t LastUpdate = 0;

} // namespace

// ------------------------------------------------------------- outbound cmds

bool NetCmdHatAuth() {
    Packet pack;
    pack.WriteUInt8(0x10);
    pack.WriteUInt32(Config::ServerCaps);

    return SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) == 0;
}

bool NetCmdShutdown() {
    ShuttingDown = true;

    Packet pack;
    pack.WriteUInt8(0x11);

    return SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) == 0;
}

bool NetCmdUpdateInfo() {
    if (Config::ServerCaps & SVC_DETAILED_INFO) {
        Packet pack;
        pack.WriteUInt8(0x12);
        // base information
        if (Info.PlayerCount == 0xFF && Info.MapLevel == 0xFF &&
            Info.GameMode == 0xFF && Info.MapSize == 0xFF) { // shutdown
            return NetCmdShutdown();
        }

        pack.WriteUInt32(Info.GameMode);
        pack.WriteString(Info.MapName);
        pack.WriteUInt8((uint8_t)Info.MapLevel);

        uint32_t p_mapwidth = MapStuff_Instance != nullptr ? (uint32_t)MapStuff_Instance->map_width : 0;
        uint32_t p_mapheight = MapStuff_Instance != nullptr ? (uint32_t)MapStuff_Instance->map_height : 0;
        uint32_t p_maptime = g_Server != nullptr ? (uint32_t)g_Server->map_elapsed_time2 / 1000 : 0;
        pack.WriteUInt32(p_mapwidth - 16);  // Map Width
        pack.WriteUInt32(p_mapheight - 16); // Map Height
        pack.WriteUInt32(p_maptime);        // Map Time
        pack.WriteUInt32(Config::ServerFlags);

        // non-AI player roster
        std::vector<Player*> tmp_players;
        for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
            Player* player = g_PlayersList->list.GetNext(pos);
            if (player != nullptr && player->is_ai == 0) {
                tmp_players.push_back(player);
            }
        }

        pack.WriteUInt32((uint32_t)tmp_players.size());
        for (Player* player : tmp_players) {
            const char* player_nickname = (const char*)player->name;
            const char* player_login = (const char*)player->login;
            uint32_t player_id1 = player->hat_char_id1;
            uint32_t player_id2 = player->flags;
            pack.WriteString(player_nickname);
            pack.WriteString(player_login != nullptr ? player_login : "");
            pack.WriteUInt32(player_id1);
            pack.WriteUInt32(player_id2);
            NetStru2* vd = g_NetStru1_main.GetClientByPlayerID(player->player_id);
            bool player_connected = (vd != nullptr);
            pack.WriteUInt8(player_connected ? 1 : 0);
            if (player->is_ai != 0 || !player_connected) { // AI or disconnected
                pack.WriteString("");
            } else {
                pack.WriteString((const char*)vd->buf);
            }
        }

        // send the logins to return
        std::vector<std::string> logins;
        std::string mask = Config::ChrBase;
        mask += "*";
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(mask.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            bool found = true;
            while (found) {
                std::string login = std::string(fd.cFileName);
                found = (FindNextFileA(h, &fd) != 0);

                if (login.find(".") != std::string::npos) {
                    continue;
                }

                logins.push_back(login);
            }
        }

        FindClose(h);

        pack.WriteUInt32((uint32_t)logins.size());
        for (const std::string& login : logins) {
            pack.WriteString(login);
        }

        return SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) == 0;
    } else {
        Packet pack;
        pack.WriteUInt8(0xD2);
        pack.WriteUInt8((uint8_t)Info.PlayerCount);
        pack.WriteUInt8((uint8_t)Info.MapLevel);
        pack.WriteUInt8((uint8_t)Info.GameMode);
        pack.WriteUInt8((uint8_t)Info.MapSize);
        pack.WriteString(Info.MapName);

        return SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) == 0;
    }
}

bool NetCmdBroadcast(const std::string& what) {
    Packet pack;
    pack.WriteUInt8(0x13);
    pack.WriteString(what);

    if (SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) != 0) {
        Connected = false;
        return false;
    }
    return true;
}

bool NetCmdUnlockLogin(const std::string& login) {
    Packet pack;
    pack.WriteUInt8(0x14);
    pack.WriteString(login);

    return SOCK_SendPacket(Socket, pack, Config::ProtocolVersion) == 0;
}

// --------------------------------------------------------- connection control

void NetHatPrepare() {
    // HatAddress2 ([network]/ipaddress2): the control connection binds a
    // local port of (its port + 1000) and connects to HatAddress.
    std::string dadr((const char*)g_ServerConfig.ip_address2);
    std::vector<std::string> ipd = Explode(dadr, ":");
    if (ipd.size() > 0) {
        ControlAddr = ipd[0];
    }
    if (ipd.size() == 2) {
        ControlPort = (uint16_t)(StrToUInt(ipd[1]) + 1000);
    }

    std::string dadd((const char*)g_ServerConfig.hat_address);
    ipd = Explode(dadd, ":");
    if (ipd.size() > 0) {
        HatAddr = ipd[0];
    }
    if (ipd.size() == 2) {
        HatPort = (uint16_t)StrToUInt(ipd[1]);
    }

    ShuttingDown = false;
    LastReconnect = 0;
}

bool NetHatInit() {
    if (ControlPort == 0 && ControlAddr.empty()) {
        return false;
    }
    if (ShuttingDown) {
        return false;
    }

    Socket = SOCK_Connect(HatAddr, HatPort, ControlAddr, ControlPort);
    if (Socket == SERR_NOTCREATED) {
        NetHatShutdown();
        return false;
    }

    Connected = true;
    Receiver.Connect(Socket);
    LastUpdate = GetTickCount();

    if (!NetCmdHatAuth()) {
        NetHatShutdown();
        Printf("Warning: control connection to hat failed (disconnected).");
        return false;
    }

    Printf("Control connection to hat established.");

    return true;
}

bool NetHatProcess() {
    if (ShuttingDown) {
        return false;
    }
    if (!Connected) {
        return false;
    }
    if (!Receiver.Receive(Config::ProtocolVersion)) {
        return false;
    }

    if (GetTickCount() - LastUpdate > 15000 && HaveInfo) {
        if (!NetCmdUpdateInfo()) {
            Connected = false;
        }
        LastUpdate = GetTickCount();
    }

    Packet pack;
    while (Receiver.GetPacket(pack)) {
        uint8_t packet_id = pack.ReadUInt8();

        switch (packet_id) {
            case 0x63: { // broadcast from hat
                std::string message = pack.ReadString();
                g_NetStru1_main.AnnouncePlayerName(message.c_str(), nullptr);
                break;
            }
            case 0x64: { // screenshot information
                std::string login = pack.ReadString();
                pack.ReadUInt8(); // ????
                uint32_t uid = pack.ReadUInt32();
                bool done = pack.ReadUInt8() != 0;
                std::string url = pack.ReadString();
                ClientScreenshot* cs = ClientScreenshot_FindByUID(uid);
                if (cs == nullptr) {
                    if (done) {
                        if (!url.empty()) {
                            Printf("screenshot: %s: Comeback transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                        } else {
                            Printf("screenshot: %s: Comeback transmission failed.", login.c_str());
                        }
                    } else {
                        Printf("screenshot: %s: Comeback transmission initiated.", login.c_str());
                    }
                } else if (cs->SourcePlayer != nullptr) {
                    CString msg;
                    if (done) {
                        if (!url.empty()) {
                            msg.Format("screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                        } else {
                            msg.Format("screenshot: %s: Transmission failed.", login.c_str());
                        }
                    } else {
                        msg.Format("screenshot: %s: Transmission initiated.", login.c_str());
                    }
                    g_NetStru1_main.AnnouncePlayerName(msg, cs->SourcePlayer);
                } else {
                    if (done) {
                        if (!url.empty()) {
                            Printf("screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                        } else {
                            Printf("screenshot: %s: Transmission failed.", login.c_str());
                        }
                    } else {
                        Printf("screenshot: %s: Transmission initiated.", login.c_str());
                    }
                }
                if (done) {
                    ClientScreenshot_Drop(uid);
                }
                break;
            }
            case 0x65: { // mute player packet
                std::string login = pack.ReadString();
                time_t unmutedate = pack.ReadUInt32();
                struct tm parsedTime;
                localtime_s(&parsedTime, &unmutedate);
                Player* player = nullptr;
                for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
                    Player* p = g_PlayersList->list.GetNext(pos);
                    if (p != nullptr && p->is_ai == 0 && p->login.Compare(login.c_str()) == 0) {
                        player = p;
                        break;
                    }
                }
                PlayerInfo* pi = player != nullptr ? PI_Get(player) : nullptr;
                if (pi != nullptr) {
                    pi->UnmuteDate = (uint32_t)unmutedate;
                    Printf("Player %s (login %s) should be muted until %02d.%02d.%04d %02d:%02d:%02d.",
                           (const char*)player->name, login.c_str(), parsedTime.tm_mday, parsedTime.tm_mon + 1,
                           parsedTime.tm_year + 1900, parsedTime.tm_hour, parsedTime.tm_min, parsedTime.tm_sec);
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}

void NetHatShutdown() {
    if (Connected && Socket == 0) {
        return;
    }
    if (Socket != 0) {
        SOCK_Destroy(Socket);
    }
    Socket = 0;
    Connected = false;
}

void NetRegularProc() {
    if (ControlPort != 0 && !ControlAddr.empty() && !ShuttingDown) {
        if (!NetHatProcess() && (GetTickCount() - LastReconnect > 5000)) {
            NetHatShutdown();
            NetHatInit(); // reconnect
            LastReconnect = GetTickCount();
        }
    }
}

void NetHatUpdateInfo(uint8_t player_count, const char* name, uint8_t level, uint8_t type, uint8_t size) {
    Info.PlayerCount = player_count;
    Info.MapLevel = level;
    Info.MapName = name;
    Info.GameMode = type;
    Info.MapSize = size;
    HaveInfo = true;

    if (!NetCmdUpdateInfo()) {
        Connected = false;
    }
}

namespace {

bool RecvExactServer(SOCKET s, uint8_t* dst, int32_t total) {
    int32_t got = 0;
    while (got < total) {
        int r = recv(s, reinterpret_cast<char*>(dst + got), total - got, 0);
        if (r > 0) {
            got += r;
            continue;
        }
        if (r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
            if (!SOCK_WaitEvent(s, 1000)) {
                return false;
            }
            continue;
        }
        return false;
    }
    return true;
}

// Ported from my-srvmgr/srvmgr_new.cpp:209 (Sv_ProcessClientPacket).
bool ProcessClientExtendedPacket(int16_t player_id, Packet& pack) {
    (void)pack;
    if (player_id == -1) {
        return true; // hat
    }
    return true;
}

} // namespace

// Ported from my-srvmgr/srvmgr.cpp:2475 (send0).
void FlushEnqueuedPackets(SOCKET s) {
    if (g_CLlDriver.connection_sockets == nullptr || g_PlayersList == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < g_CLlDriver.max_connections; i++) {
        A2NetSock* conn = g_CLlDriver.connection_sockets + i;
        if (conn->is_in_use == 0 || conn->socket != s || conn->manager == nullptr) {
            continue;
        }

        uint16_t player_id = conn->manager->player_id;
        if (player_id == 0) {
            break;
        }

        Player* player = g_PlayersList->FindById(player_id);
        PlayerInfo* pi = PI_Get(player);
        if (pi != nullptr && !pi->EnqueuedPackets.empty()) {
            for (Packet& cmd : pi->EnqueuedPackets) {
                SOCK_SendPacket(s, cmd, 0, true);
            }
            pi->EnqueuedPackets.clear();
        }
        break;
    }
}

// Ported from my-srvmgr/srvmgr.cpp:2539 (recv0).
int RecvServerFramed(SOCKET s, char* buf, int len, int flags) {
    SOCKET hat_socket = g_HatLLDriver.listen_socket.socket;

    int16_t p_id = -1;
    if (g_CLlDriver.connection_sockets != nullptr) {
        for (uint32_t i = 0; i < g_CLlDriver.max_connections; i++) {
            A2NetSock* conn = g_CLlDriver.connection_sockets + i;
            if (conn->socket == s) {
                if (conn->manager == nullptr) {
                    Printf("Warning: received packet from player with NULL network structure!\n");
                    memset(buf, 0, static_cast<size_t>(len));
                    return -1;
                }
                p_id = static_cast<int16_t>(conn->manager->player_id);
                break;
            }
        }
    }

    if (p_id == -1 && s != hat_socket) {
        Printf("Warning: received packet from unknown player!\n");
        memset(buf, 0, static_cast<size_t>(len));
        return -1;
    }

    int r = recv(s, buf, 4, 0);
    if (r != 4) {
        return -1;
    }
    if (*reinterpret_cast<int32_t*>(buf) == 0) {
        return 0;
    }

    if (!RecvExactServer(s, reinterpret_cast<uint8_t*>(buf + 4), 4)) {
        return -1;
    }

    int32_t pkt_size = *reinterpret_cast<int32_t*>(buf);
    uint32_t pkt_flags = *reinterpret_cast<uint32_t*>(buf + 4);

    if ((pkt_flags & 0x80100000u) == 0x80100000u) {
        if (pkt_size < 0 || pkt_size > 0x100000) {
            return -1;
        }

        std::vector<uint8_t> rd(static_cast<size_t>(pkt_size));
        if (pkt_size > 0 && !RecvExactServer(s, rd.data(), pkt_size)) {
            return -1;
        }

        Packet pack;
        pack.Reset();
        for (int32_t i = 0; i < pkt_size; i++) {
            pack.WriteUInt8(rd[static_cast<size_t>(i)]);
        }
        pack.Seek(0);

        if (!ProcessClientExtendedPacket(p_id, pack)) {
            return -1;
        }

        memset(buf, 0, static_cast<size_t>(len));
        if (!SOCK_WaitEvent(s, 0)) {
            return -2;
        }
        return RecvServerFramed(s, buf, len, flags);
    }

    if (pkt_size < 0 || pkt_size + 8 > len) {
        return -1;
    }

    if (pkt_size > 0 && !RecvExactServer(s, reinterpret_cast<uint8_t*>(buf + 8), pkt_size)) {
        return -1;
    }
    return pkt_size + 8;
}

namespace {

#ifdef SRV_LOG
struct ClientLogNode {
    SOCKET s;
    std::string ip;
    std::string name;
    ClientLogNode* n;
    ClientLogNode* p;
};

ClientLogNode* g_ClientLogHead = nullptr;
std::mutex g_ClientLogMutex;
#endif

bool IsPeerIpBanned(const char* ip_str) {
    if (ip_str == nullptr || ip_str[0] == '\0') {
        return false;
    }

    for (int i = 0; i < g_ServerConfig.banned_ips.GetSize(); i++) {
        if (_stricmp(g_ServerConfig.banned_ips[i], ip_str) == 0) {
            return true;
        }
    }

    IPFilter::IPFFile ipf;
    ipf.ReadIPF("ban.ip", true);
    if (!ipf.Entries.empty() && ipf.CheckAddress(ip_str) == -1) {
        return true;
    }

    return false;
}

} // namespace

// Ported from my-srvmgr/srvmgr.cpp:2426 (accept0 @ 5BB6AE).
SOCKET AcceptServerSocket(SOCKET s, sockaddr* addr, int* addrlen) {
    SOCKET r = accept(s, addr, addrlen);
    if (r == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    char ip_buf[32] = {};
    if (addr != nullptr && addr->sa_family == AF_INET) {
        const sockaddr_in* sin = reinterpret_cast<const sockaddr_in*>(addr);
        std::snprintf(ip_buf, sizeof(ip_buf), "%u.%u.%u.%u",
                      sin->sin_addr.S_un.S_un_b.s_b1,
                      sin->sin_addr.S_un.S_un_b.s_b2,
                      sin->sin_addr.S_un.S_un_b.s_b3,
                      sin->sin_addr.S_un.S_un_b.s_b4);
        if (IsPeerIpBanned(ip_buf)) {
            Printf("Connection from banned IP %s rejected", ip_buf);
            closesocket(r);
            WSASetLastError(WSAECONNRESET);
            return INVALID_SOCKET;
        }
    }

#ifdef SRV_LOG
    {
        std::lock_guard<std::mutex> lock(g_ClientLogMutex);
        ClientLogNode* c = g_ClientLogHead;
        g_ClientLogHead = new ClientLogNode{r, ip_buf, std::string(), c, nullptr};
        if (c != nullptr) {
            c->p = g_ClientLogHead;
        }
    }
#endif

    return r;
}

// Ported from my-srvmgr/srvmgr.cpp:2517 (closesocket0 @ 5BB6A8).
int CloseServerSocket(SOCKET s) {
#ifdef SRV_LOG
    {
        std::lock_guard<std::mutex> lock(g_ClientLogMutex);
        ClientLogNode* c = g_ClientLogHead;
        while (c != nullptr && c->s != s) {
            c = c->n;
        }
        if (c != nullptr) {
            ClientLogNode* t = c->n;
            if (c->p != nullptr) {
                c->p->n = t;
                if (t != nullptr) {
                    t->p = c->p;
                }
            } else {
                g_ClientLogHead = t;
                if (t != nullptr) {
                    t->p = nullptr;
                }
            }
            delete c;
        }
    }
#endif
    return closesocket(s);
}

} // namespace tangar_hat
