#pragma once

// Tangar HAT-protocol support library, ported from my-srvmgr lib/
// (BinaryStream/serialize.hpp, packet.{hpp,cpp}, hat2.cpp send_msg framing,
// socket.{hpp,cpp}). The framing is the rom_hat chunked-XOR wire format:
// payload is split into <=0x8E-byte chunks, each XORed with a per-protocol
// key and prefixed with an 8-byte header; the stream terminates with the
// 64 01 00 00 00 tail.

#include <cstdint>
#include <string>
#include <vector>

#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif

namespace tangar_hat {

// ---------------------------------------------------------------- BinaryStream

class BinaryStream {
public:
    BinaryStream() : position(0) {}

    void Reset() {
        buffer.clear();
        position = 0;
    }

    uint32_t Seek(uint32_t pos) {
        position = pos;
        return position;
    }

    uint32_t GetLength() const { return (uint32_t)buffer.size(); }
    uint32_t GetPosition() const { return position; }
    bool EndOfStream() const { return position >= buffer.size(); }

    uint8_t ReadUInt8() {
        if (position >= buffer.size()) {
            return 0;
        }
        return buffer[position++];
    }

    uint16_t ReadUInt16() {
        uint16_t v = 0;
        for (int i = 0; i < 2; i++) {
            v |= (uint16_t)ReadUInt8() << (8 * i);
        }
        return v;
    }

    uint32_t ReadUInt32() {
        uint32_t v = 0;
        for (int i = 0; i < 4; i++) {
            v |= (uint32_t)ReadUInt8() << (8 * i);
        }
        return v;
    }

    // string with 16-bit size (almost everywhere)
    std::string ReadString() {
        uint16_t size = ReadUInt16();
        std::string out;
        out.reserve(size);
        for (uint16_t i = 0; i < size; i++) {
            out += (char)ReadUInt8();
        }
        return out;
    }

    void WriteUInt8(uint8_t v) {
        buffer.push_back(v);
    }

    void WriteUInt16(uint16_t v) {
        for (int i = 0; i < 2; i++) {
            WriteUInt8((uint8_t)(v >> (8 * i)));
        }
    }

    void WriteUInt32(uint32_t v) {
        for (int i = 0; i < 4; i++) {
            WriteUInt8((uint8_t)(v >> (8 * i)));
        }
    }

    void WriteString(const std::string& what) {
        WriteUInt16((uint16_t)what.size());
        for (char c : what) {
            WriteUInt8((uint8_t)c);
        }
    }

    const std::vector<uint8_t>& GetBuffer() const { return buffer; }

private:
    std::vector<uint8_t> buffer;
    uint32_t position;
};

typedef BinaryStream Packet;

// ------------------------------------------------------------------- framing

constexpr uint32_t SERR_NOTCREATED = 0xFFFFFFFF;
constexpr int SERR_CONNECTION_LOST = 100;
constexpr int SERR_TIMEOUT = 101;
constexpr int SERR_INCOMPLETE_DATA = 102;

void PACKET_XorByKey(uint8_t* data, uint32_t size, uint32_t protover);
int SOCK_SendMsg(SOCKET sock, int ver, const uint8_t* msg, size_t len, int timeout);

SOCKET SOCK_Connect(const std::string& address, uint16_t port, const std::string& localaddress, uint16_t localport);
SOCKET SOCK_Listen(const std::string& address, uint16_t port);
SOCKET SOCK_Accept(SOCKET listener, sockaddr_in& addr);
int SOCK_SendPacket(SOCKET socket, Packet& packet, uint32_t protover, bool extended = false);
int SOCK_ReceivePacket(SOCKET socket, Packet& packet, uint32_t protover);
void SOCK_Destroy(SOCKET socket);
void SOCK_SetBlocking(SOCKET socket, bool blocking);
bool SOCK_WaitEvent(SOCKET socket, uint32_t timeout);

namespace IPFilter {

struct IPAddress {
    std::string Address;
    uint32_t Addr = 0;
    uint32_t Masked = 0;
    uint32_t Mask = 0;

    void FromString(const std::string& str);
};

struct IPFEntry {
    bool Action = false;
    IPAddress Address;
};

struct IPFFile {
    std::vector<IPFEntry> Entries;

    // +1 = explicitly allowed, -1 = explicitly blocked, 0 = not set in this IPF
    int CheckAddress(const std::string& addr) const;
    void ReadIPF(const std::string& str, bool filename = false);
};

} // namespace IPFilter

class PacketReceiver {
public:
    PacketReceiver() : Socket(0) {}

    void Connect(SOCKET sock) { Socket = sock; }
    bool Receive(uint32_t version);
    bool GetPacket(Packet& pack);

private:
    std::vector<Packet> Queue;
    SOCKET Socket;
};

} // namespace tangar_hat
