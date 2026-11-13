#pragma once

// Tangar client-side v20 HAT authorization & extended-packet wire framing,
// ported from my-a2mgr/network_protocol.cpp (crcmain,
// NETPROTO_createAuthorizationPacket, recv_) and my-a2mgr/a2mgr.cpp (SHA1
// anticheat fingerprint setup).

#include <cstdint>
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#include <afx.h>

#include "hat_packet.h"

class PacketJoin;

namespace tangar_client {

constexpr uint32_t PROTO_VER = 20;

// Lazily compute SHA-1 digests of executable, resources, and first volume UUID
// used by the v20 HAT authorization fingerprint (my-a2mgr/a2mgr.cpp:107-174).
void EnsureAnticheatHashes();

// Populate PacketJoin as a v20 HAT authorization packet (id = 0) and apply
// the crcmain gamemode -> z_softcore side effect (client: 50e48b).
void BuildAuthorizationPacket(PacketJoin* pkt, const CString& login, const CString& password, uint32_t flags);

// Handle extended server->client manager packets (pkt_flags == 0x80100000).
bool ProcessServerPacket(tangar_hat::Packet& pack);

// Framed TCP receive hook for CLlDriver::RecvThreadTcp (client: 5122d1,
// my-a2mgr recv_). Demultiplexes 0x80100000 manager packets and normal
// 8-byte-headed game packets.
int RecvClientFramed(SOCKET s, char* buf, int len, int flags);

} // namespace tangar_client
