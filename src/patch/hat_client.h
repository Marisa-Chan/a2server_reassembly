#pragma once

// Tangar client-side v20 HAT authorization & extended-packet wire framing,
// ported from a2mgr/network_protocol.cpp (crcmain,
// NETPROTO_createAuthorizationPacket, recv_) and a2mgr/a2mgr.cpp (SHA1
// fingerprint setup for self-update).

#include <cstdint>
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#include <afx.h>

#include "hat_packet.h"

class MainWindow;
class PacketJoin;

namespace tangar_client {

constexpr uint32_t PROTO_VER = 20;

// Check remote /allods/version_id & /allods/version on api.allods2.eu, download
// any updated resource files into data/, and self-restart if files changed.
// Passes through cleanly when the update endpoint is unused or unreachable
// (ported from a2mgr/network_protocol_ext.cpp NETPROTO_updater, client: 4949cd).
bool RunAutoUpdater(MainWindow* wnd);

// Lazily compute SHA-1 digests of executable, resources, and first volume UUID
// used by the v20 HAT authorization fingerprint (a2mgr/a2mgr.cpp:107-174).
void EnsureFileHashes();

// Populate PacketJoin as a v20 HAT authorization packet (id = 0) and apply
// the crcmain gamemode -> z_softcore side effect (client: 50e48b).
void BuildAuthorizationPacket(PacketJoin* pkt, const CString& login, const CString& password, uint32_t flags);

// Handle extended server->client manager packets (pkt_flags == 0x80100000).
bool ProcessServerPacket(tangar_hat::Packet& pack);

// Deferred screen-surface capture + background PNG upload hook for
// UnlockSurface1 (imp_MakeScreenshotNet, client: 457ff4).
void ProcLastScreenshot();

// Framed TCP receive hook for CLlDriver::RecvThreadTcp (client: 5122d1,
// a2mgr recv_). Demultiplexes 0x80100000 manager packets and normal
// 8-byte-headed game packets.
int RecvClientFramed(SOCKET s, char* buf, int len, int flags);

} // namespace tangar_client
