// Tangar HAT-protocol support library implementation, ported from
// my-srvmgr lib/hat2.cpp (send_msg framing), lib/packet.cpp (XOR keys) and
// lib/socket.cpp (connection + packet IO + PacketReceiver).

#include "hat_packet.h"

#include <cstdlib>
#include <cstring>

namespace tangar_hat {

// ------------------------------------------------------------------ XOR keys

namespace {

const uint8_t key_08[0x50] = {0x02E,0x0C7,0x0C4,0x08D,0x0FF,0x0E5,0x05D,0x00B,0x0D6,0x0FF,0x07F,0x0FF,0x0D7,0x034,0x0D2,0x002,0x0E2,0x06D,0x09E,0x048,0x07B,0x0C6,0x06A,0x0F1,0x097,0x073,0x056,0x077,0x0FA,0x09D,0x080,0x000,0x011,0x004,0x029,0x008,0x0A6,0x08B,0x02A,0x047,0x064,0x000,0x001,0x038,0x084,0x0A0,0x040,0x069,0x001,0x0F9,0x0FA,0x0BE,0x0EA,0x0FF,0x097,0x07B,0x0A7,0x026,0x0ED,0x0F7,0x06B,0x07B,0x03B,0x04F,0x044,0x074,0x0A3,0x009,0x079,0x040,0x038,0x03A,0x020,0x05D,0x0A3,0x040,0x0C3,0x0E8,0x07F,0x03B,},
key_10[0x50] = {0x0DF,0x019,0x061,0x043,0x0AD,0x0C4,0x04C,0x01C,0x038,0x08D,0x01D,0x0BE,0x070,0x081,0x0EB,0x063,0x01A,0x052,0x0F4,0x0EC,0x084,0x0E1,0x04E,0x0EF,0x048,0x05D,0x069,0x011,0x083,0x06E,0x0E6,0x0C6,0x0F8,0x02F,0x053,0x0D0,0x0FF,0x0EA,0x0D1,0x016,0x037,0x0D4,0x0C0,0x0BE,0x056,0x051,0x0E6,0x08D,0x0A0,0x068,0x0B3,0x004,0x0CC,0x073,0x09D,0x0C0,0x063,0x03B,0x0AC,0x009,0x0F6,0x00F,0x0BC,0x0A1,0x096,0x074,0x001,0x035,0x07E,0x08C,0x0DB,0x00D,0x00A,0x0DF,0x052,0x008,0x0C1,0x02B,0x03D,0x0B5,},
key_11[0x50] = {0x0D1,0x016,0x037,0x0D4,0x0C0,0x0BE,0x056,0x051,0x0E6,0x08D,0x0A0,0x068,0x0B3,0x004,0x0CC,0x073,0x09D,0x0C0,0x0AC,0x009,0x0F6,0x00F,0x0BC,0x0A1,0x096,0x075,0x001,0x035,0x07E,0x08C,0x0DB,0x00D,0x02B,0x03D,0x0B5,0x00A,0x0DF,0x052,0x008,0x063,0x03B,0x08D,0x01D,0x0BE,0x070,0x0EB,0x063,0x0EF,0x048,0x05D,0x0AD,0x0C4,0x0C1,0x01A,0x052,0x0F4,0x0EC,0x084,0x0E1,0x04E,0x04C,0x01C,0x038,0x069,0x011,0x0DF,0x019,0x061,0x043,0x081,0x083,0x06E,0x0E6,0x0C6,0x0F8,0x02F,0x053,0x0D0,0x0FF,0x0EA,},
key_15[0x50] = {0x0A0,0x061,0x091,0x07A,0x09A,0x03F,0x08C,0x0F1,0x0A9,0x02F,0x09F,0x05E,0x0DE,0x0AD,0x0CF,0x04E,0x080,0x000,0x081,0x0A9,0x001,0x0F0,0x0FF,0x08E,0x041,0x048,0x0A6,0x02A,0x0EF,0x0DF,0x019,0x061,0x043,0x0AD,0x0C4,0x04C,0x01C,0x038,0x08D,0x01D,0x0BE,0x070,0x0D6,0x0FF,0x07F,0x0FF,0x0D7,0x034,0x0D2,0x002,0x0E2,0x06D,0x09E,0x048,0x07B,0x0C6,0x0E1,0x04E,0x0EF,0x048,0x05D,0x069,0x011,0x083,0x06E,0x0E6,0x0C6,0x0F8,0x02F,0x053,0x0D0,0x0FF,0x0EA,0x0D1,0x016,0x037,0x0DE,0x0AD,0x0BE,0x0EF,},
key_20[0x50] = {0x0C9,0x086,0x093,0x0FC,0x06A,0x044,0x018,0x0AA,0x0AA,0x012,0x09A,0x060,0x0CF,0x010,0x041,0x0E8,0x0BA,0x0BA,0x01A,0x088,0x010,0x0AF,0x0DE,0x0AD,0x0BE,0x0EF,0x084,0x091,0x05A,0x015,0x020,0x01A,0x055,0x005,0x0E7,0x087,0x001,0x011,0x0E5,0x0D1,0x006,0x0E6,0x068,0x071,0x0CC,0x0CC,0x018,0x080,0x000,0x011,0x03F,0x018,0x0CA,0x016,0x0C9,0x026,0x0AD,0x051,0x05A,0x080,0x0CD,0x0EF,0x034,0x088,0x014,0x081,0x0BE,0x0FE,0x0EE,0x0ED,0x0C0,0x0FF,0x0EE,0x0E8,0x079,0x0A5,0x0E7,0x050,0x041,0x0CA,};
;

const uint8_t* PACKET_GetKey(uint32_t ver) {
    switch (ver) {
        case 7:
        case 8:
        case 9:
            return key_08;
        case 10:
            return key_10;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            return key_11;
        case 20:
            return key_20;
        default:
            return nullptr;
    }
}

} // namespace

void PACKET_XorByKey(uint8_t* data, uint32_t size, uint32_t protover) {
    const uint8_t* key = PACKET_GetKey(protover);
    if (key == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < size; i++) {
        data[i] ^= key[(i % 0x8E) % 0x50];
    }
}

int SOCK_SendMsg(SOCKET sock, int ver, const uint8_t* msg, size_t len, int timeout) {
    static const uint8_t end[] = {0x64, 0x01, 0x00, 0x00, 0x00};
    int ret_code = 0;
    uint32_t idx = 0;
    uint8_t pack[0x96];
    fd_set fd;
    timeval tv;

    uint8_t* STp = (uint8_t*)malloc(len + 5);
    memcpy(STp, msg, len);
    memcpy(STp + len, end, 5);

    memset(pack, 0, 8);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    len += 5;

    while (idx < len) {
        if (len - idx > 0x8E) {
            pack[0] = 0x8E;
            pack[7] = 0;
        } else {
            pack[0] = (uint8_t)(len - idx);
            pack[7] = 1;
        }
        memcpy(pack + 8, STp + idx, pack[0]);
        PACKET_XorByKey(pack + 8, pack[0], ver);

        int nBytesSent = 0;
        while (nBytesSent < pack[0] + 8) {
            FD_ZERO(&fd);
            FD_SET(sock, &fd);
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            if (select((int)sock + 1, NULL, &fd, NULL, &tv) == 0) {
                ret_code = -2;
                goto ex; // timeout
            }
            int err_code = send(sock, (const char*)pack, pack[0] + 8, 0);
            if (err_code == SOCKET_ERROR || err_code == 0) {
                ret_code = -1;
                goto ex; // send error
            }
            nBytesSent += err_code;
        }
        idx += pack[0];
    }

ex:
    if (STp != nullptr) {
        free(STp);
    }
    return ret_code;
}

SOCKET SOCK_Connect(const std::string& address, uint16_t port, const std::string& localaddress, uint16_t localport) {
    sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(localaddress.c_str());
    local.sin_port = htons(localport);
    SOCKET out = socket(AF_INET, SOCK_STREAM, 0);
    if (out == INVALID_SOCKET) {
        return SERR_NOTCREATED;
    }
    if (bind(out, (sockaddr*)&local, sizeof(local)) != 0) {
        closesocket(out);
        return SERR_NOTCREATED;
    }

    sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(address.c_str());
    if (remote.sin_addr.s_addr == INADDR_NONE) {
        hostent* host = gethostbyname(address.c_str());
        if (host == nullptr) {
            closesocket(out);
            return SERR_NOTCREATED;
        }
        remote.sin_addr.s_addr = *((unsigned long*)host->h_addr);
    }
    remote.sin_port = htons(port);
    if (connect(out, (sockaddr*)&remote, sizeof(remote)) != 0) {
        closesocket(out);
        return SERR_NOTCREATED;
    }

    return out;
}

int SOCK_SendPacket(SOCKET socket, Packet& packet, uint32_t protover, bool extended) {
    if (!extended) {
        if (packet.GetLength() == 0) {
            return 0;
        }
        const std::vector<uint8_t>& tbuf = packet.GetBuffer();
        uint32_t size = packet.GetLength();
        uint8_t* data = new uint8_t[size];
        for (uint32_t i = 0; i < size; i++) {
            data[i] = tbuf[i];
        }

        int ret_code = SOCK_SendMsg(socket, (int)protover, data, size, 1);
        delete[] data;
        if (ret_code == -2) {
            return SERR_TIMEOUT;
        }
        if (ret_code == -1) {
            return SERR_CONNECTION_LOST;
        }
        return 0;
    } else {
        uint32_t size = packet.GetLength();
        const std::vector<uint8_t>& tbuf = packet.GetBuffer();
        uint8_t* data = new uint8_t[size + 8];
        *(uint32_t*)(data) = size;
        *(uint32_t*)(data + 4) = 0x80100000;
        for (uint32_t i = 0; i < size; i++) {
            data[i + 8] = tbuf[i];
        }

        int ret_code = send(socket, (const char*)data, size + 8, 0);
        delete[] data;
        if (ret_code == -1) {
            return SERR_CONNECTION_LOST;
        }
        return 0;
    }
}

int SOCK_ReceivePacket(SOCKET socket, Packet& packet, uint32_t protover) {
    packet.Reset();
    uint32_t origin = 0xFFFFFFFF;

    while (!(origin & 0xFFFF0000) || origin == 0xFFFFFFFF) {
        uint32_t _siz;
        uint32_t _org;
        if (!SOCK_WaitEvent(socket, 0) && origin == 0xFFFFFFFF) {
            return SERR_TIMEOUT;
        } else if (origin != 0xFFFFFFFF) {
            return SERR_INCOMPLETE_DATA;
        }
        int headersize = recv(socket, (char*)&_siz, 4, 0); // receive packet length
        if (headersize != 4) {
            return SERR_CONNECTION_LOST;
        }
        if (!SOCK_WaitEvent(socket, 0)) {
            return SERR_CONNECTION_LOST;
        }
        headersize = recv(socket, (char*)&_org, 4, 0); // receive packet origin
        if (headersize != 4) {
            return SERR_CONNECTION_LOST;
        }
        if (_siz > 0xFF || _siz == 0) {
            return SERR_CONNECTION_LOST;
        }
        uint8_t* data = new uint8_t[_siz];
        memset(data, 0, _siz);
        if (!SOCK_WaitEvent(socket, 0)) {
            delete[] data;
            return SERR_CONNECTION_LOST;
        }
        int packetsize = recv(socket, (char*)data, _siz, 0);
        if (packetsize != (int)_siz) {
            packet.Reset();
            delete[] data;
            return SERR_CONNECTION_LOST;
        }
        origin = _org;
        PACKET_XorByKey(data, _siz, protover);
        for (uint32_t i = 0; i < _siz; i++) {
            packet.WriteUInt8(data[i]);
        }
        delete[] data;
    }
    packet.Seek(0);
    return 0;
}

void SOCK_Destroy(SOCKET socket) {
    closesocket(socket);
}

bool SOCK_WaitEvent(SOCKET socket, uint32_t timeout) {
    fd_set fd;
    timeval tv;
    FD_ZERO(&fd);
    FD_SET(socket, &fd);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    return select(0, &fd, NULL, NULL, &tv) > 0;
}

bool PacketReceiver::Receive(uint32_t version) {
    if (Socket == 0) {
        return false;
    }

    Packet pack;
    pack.Reset();

    if (Queue.size() == 0) {
        Queue.push_back(pack);
    }

    int zz = SOCK_ReceivePacket(Socket, pack, version);

    if (zz == SERR_TIMEOUT) {
        return true;
    } else if (zz == SERR_CONNECTION_LOST) {
        return false;
    }

    Queue.back() = pack;

    if (zz == 0) {
        // create "next" packet
        Packet pack2;
        pack2.Reset();
        Queue.push_back(pack2);
    }

    return true;
}

bool PacketReceiver::GetPacket(Packet& pack) {
    if (Socket == 0) {
        return false;
    }
    if (Queue.size() <= 1) {
        return false;
    }

    pack = Queue.front();
    Queue.erase(Queue.begin());

    return true;
}

} // namespace tangar_hat
