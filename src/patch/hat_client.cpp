// Tangar client-side v20 HAT authorization & extended-packet wire framing,
// ported from my-a2mgr/network_protocol.cpp, my-a2mgr/CRC_32.cpp,
// my-a2mgr/lib/sha1.cpp, and my-a2mgr/a2mgr.cpp.

#include "hat_client.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>

#include "client_config.h"
#include "happyhttp/happyhttp.h"
#include "lodepng/lodepng.h"
#include "../gfx.h"
#include "../main_window.h"
#include "../packet.h"

namespace tangar_client {

namespace {

// ------------------------------------------------------------------- SHA-1
// FIPS-180-1 SHA-1 (ported from my-a2mgr/lib/sha1.cpp)

struct Sha1Context {
    uint32_t total[2];
    uint32_t state[5];
    uint8_t buffer[64];
};

uint32_t ReadBe32(const uint8_t* b, size_t i) {
    return (static_cast<uint32_t>(b[i]) << 24)
         | (static_cast<uint32_t>(b[i + 1]) << 16)
         | (static_cast<uint32_t>(b[i + 2]) << 8)
         | (static_cast<uint32_t>(b[i + 3]));
}

void WriteBe32(uint32_t n, uint8_t* b, size_t i) {
    b[i]     = static_cast<uint8_t>(n >> 24);
    b[i + 1] = static_cast<uint8_t>(n >> 16);
    b[i + 2] = static_cast<uint8_t>(n >> 8);
    b[i + 3] = static_cast<uint8_t>(n);
}

uint32_t Rol32(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

void Sha1Starts(Sha1Context* ctx) {
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xEFCDAB89u;
    ctx->state[2] = 0x98BADCFEu;
    ctx->state[3] = 0x10325476u;
    ctx->state[4] = 0xC3D2E1F0u;
}

void Sha1Process(Sha1Context* ctx, const uint8_t data[64]) {
    uint32_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ReadBe32(data, static_cast<size_t>(i) * 4);
    }
    for (int i = 16; i < 80; i++) {
        w[i] = Rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];

    for (int i = 0; i < 80; i++) {
        uint32_t f = 0;
        uint32_t k = 0;
        if (i < 20) {
            f = d ^ (b & (c ^ d));
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (d & (b | c));
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }
        uint32_t temp = Rol32(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = Rol32(b, 30);
        b = a;
        a = temp;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

void Sha1Update(Sha1Context* ctx, const uint8_t* input, size_t ilen) {
    if (ilen == 0) {
        return;
    }

    uint32_t left = ctx->total[0] & 0x3Fu;
    size_t fill = 64 - left;

    ctx->total[0] += static_cast<uint32_t>(ilen);
    if (ctx->total[0] < static_cast<uint32_t>(ilen)) {
        ctx->total[1]++;
    }

    if (left != 0 && ilen >= fill) {
        memcpy(ctx->buffer + left, input, fill);
        Sha1Process(ctx, ctx->buffer);
        input += fill;
        ilen -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        Sha1Process(ctx, input);
        input += 64;
        ilen -= 64;
    }

    if (ilen > 0) {
        memcpy(ctx->buffer + left, input, ilen);
    }
}

void Sha1Finish(Sha1Context* ctx, uint8_t output[20]) {
    static const uint8_t padding[64] = { 0x80 };
    uint32_t high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    uint32_t low = (ctx->total[0] << 3);

    uint8_t msglen[8];
    WriteBe32(high, msglen, 0);
    WriteBe32(low, msglen, 4);

    uint32_t last = ctx->total[0] & 0x3Fu;
    uint32_t padn = (last < 56) ? (56 - last) : (120 - last);

    Sha1Update(ctx, padding, padn);
    Sha1Update(ctx, msglen, 8);

    for (int i = 0; i < 5; i++) {
        WriteBe32(ctx->state[i], output, static_cast<size_t>(i) * 4);
    }
}

void ComputeSha1(const uint8_t* input, size_t ilen, uint8_t output[20]) {
    Sha1Context ctx;
    Sha1Starts(&ctx);
    Sha1Update(&ctx, input, ilen);
    Sha1Finish(&ctx, output);
    memset(&ctx, 0, sizeof(ctx));
}

bool ComputeSha1File(const char* path, uint8_t output[20]) {
    FILE* f = nullptr;
    if (fopen_s(&f, path, "rb") != 0 || f == nullptr) {
        memset(output, 0, 20);
        return false;
    }

    Sha1Context ctx;
    Sha1Starts(&ctx);

    uint8_t buf[4096];
    size_t n = 0;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        Sha1Update(&ctx, buf, n);
    }

    bool ok = (ferror(f) == 0);
    fclose(f);

    if (!ok) {
        memset(output, 0, 20);
        return false;
    }

    Sha1Finish(&ctx, output);
    memset(&ctx, 0, sizeof(ctx));
    return true;
}

// ------------------------------------------------------------------ CRC-32
// IEEE 802.3 CRC-32 (ported from my-a2mgr/CRC_32.cpp)

uint32_t CalcCrc32(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return 0;
    }

    static uint32_t table[256];
    static bool table_ready = false;
    if (!table_ready) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int j = 0; j < 8; j++) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            table[i] = c;
        }
        table_ready = true;
    }

    uint32_t crc = 0xFFFFFFFFu;
    const uint8_t* p = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 8) ^ table[(crc & 0xFFu) ^ p[i]];
    }
    return crc ^ 0xFFFFFFFFu;
}

// --------------------------------------------------------- Anticheat state

bool g_hashes_initialized = false;
uint8_t g_sha1_exe[20] = {};
uint8_t g_sha1_dll[20] = {};
uint8_t g_sha1_patch_res[20] = {};
uint8_t g_sha1_world_res[20] = {};
uint8_t g_sha1_uuid[20] = {};

// Ported from my-a2mgr/network_protocol.cpp:21 (crcmain).
// `data` points to `reinterpret_cast<uint8_t*>(pkt) + 0x0A`, so `data + 0x40`
// aliases `pkt + 0x4A` (the login-len / gamemode / PROTO_VER dword).
void FillCrcMain(uint8_t* data) {
    constexpr uint32_t net_key = 0x6CDB248Du;
    constexpr uint32_t session_id = 0u;

    uint32_t crc32_exe = CalcCrc32(g_sha1_exe, 20);
    uint32_t crc32_dll = CalcCrc32(g_sha1_dll, 20);
    uint32_t crc32_patch = CalcCrc32(g_sha1_patch_res, 20);
    uint32_t crc32_world = CalcCrc32(g_sha1_world_res, 20);

    uint32_t lrc_exe = crc32_exe ^ net_key;
    uint32_t lrc_dll = (crc32_dll ^ net_key) + crc32_exe;
    uint32_t lrc_patch = crc32_patch ^ crc32_dll;
    uint32_t lrc_world = (crc32_world ^ net_key) - crc32_patch;

    memset(data, 0, 0x18);
    *reinterpret_cast<uint32_t*>(data + 0x00) = lrc_exe;
    *reinterpret_cast<uint32_t*>(data + 0x04) = lrc_dll;
    *reinterpret_cast<uint32_t*>(data + 0x08) = lrc_patch;
    *reinterpret_cast<uint32_t*>(data + 0x0C) = lrc_world;
    *reinterpret_cast<uint32_t*>(data + 0x10) = net_key;
    *reinterpret_cast<uint32_t*>(data + 0x14) = session_id ^ 0xDEADBEEFu;
    memcpy(data + 0x18, g_sha1_uuid, 20);

    // UUID CRC32 is computed BEFORE XORing the 5 UUID dwords with net_key.
    *reinterpret_cast<uint32_t*>(data + 0x2C) = CalcCrc32(data + 0x18, 20) ^ net_key;
    for (int i = 0; i < 5; i++) {
        *reinterpret_cast<uint32_t*>(data + 0x18 + i * 4) ^= net_key;
    }

    uint32_t uoth = *reinterpret_cast<uint32_t*>(data + 0x40);
    uint32_t gamemode = (uoth & 0x00FF0000u) >> 16;

    if (gamemode == 4) { // GAMEMODE_SANDBOX
        MainWindow* wnd = static_cast<MainWindow*>(AfxGetMainWnd());
        if (wnd != nullptr) {
            wnd->hat_settings.deathmatch = 0;
        }
        tangar_cfg::z_softcore = 2;
    } else if (gamemode == 2) { // GAMEMODE_SOFTCORE
        MainWindow* wnd = static_cast<MainWindow*>(AfxGetMainWnd());
        if (wnd != nullptr) {
            wnd->hat_settings.deathmatch = 0;
        }
        tangar_cfg::z_softcore = 1;
    } else {
        tangar_cfg::z_softcore = 0;
    }
}

bool RecvExact(SOCKET s, uint8_t* dst, int32_t total) {
    int32_t got = 0;
    while (got < total) {
        int r = recv(s, reinterpret_cast<char*>(dst + got), total - got, 0);
        if (r > 0) {
            got += r;
            continue;
        }
        if (r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
            if (!tangar_hat::SOCK_WaitEvent(s, 1000)) {
                return false;
            }
            continue;
        }
        return false;
    }
    return true;
}

} // namespace

void EnsureAnticheatHashes() {
    if (g_hashes_initialized) {
        return;
    }
    g_hashes_initialized = true;

    char exe_path[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH - 1);

    ComputeSha1File(exe_path, g_sha1_exe);
    ComputeSha1File("a2mgr.dll", g_sha1_dll);
    ComputeSha1File("patch.res", g_sha1_patch_res);
    ComputeSha1File("world.res", g_sha1_world_res);

    char tmp_uid[64] = {};
    HANDLE vol_handle = FindFirstVolumeA(tmp_uid, sizeof(tmp_uid));
    if (vol_handle != INVALID_HANDLE_VALUE && vol_handle != nullptr) {
        ComputeSha1(reinterpret_cast<const uint8_t*>(tmp_uid), strlen(tmp_uid), g_sha1_uuid);
        FindVolumeClose(vol_handle);
    } else {
        memset(g_sha1_uuid, 0, sizeof(g_sha1_uuid));
    }
}

// client: 50e48b (NETPROTO_createAuthorizationPacket)
void BuildAuthorizationPacket(PacketJoin* pkt, const CString& login, const CString& password, uint32_t flags) {
    EnsureAnticheatHashes();

    uint8_t* raw = reinterpret_cast<uint8_t*>(pkt);
    memset(raw + 0x0A, 0, 0x46);

    CString namepwd = login + password;
    strncpy(reinterpret_cast<char*>(raw + 0x50), static_cast<const char*>(namepwd), sizeof(pkt->name) - 0x41);
    raw[0x50 + sizeof(pkt->name) - 0x41] = '\0';

    pkt->id = 0;
    pkt->to_player_id = 0;

    uint32_t packed_meta = (static_cast<uint32_t>(login.GetLength()) & 0xFFFFu)
                         | ((flags & 0xFFu) << 16)
                         | (PROTO_VER << 24);
    *reinterpret_cast<uint32_t*>(raw + 0x4A) = packed_meta;

    FillCrcMain(raw + 0x0A);
}

namespace {

struct ScreenshotInfo {
    std::string login;
    uint32_t uid;
    int32_t width;
    int32_t height;
    uint16_t* data;
    bool flip;
};

std::atomic<ScreenshotInfo*> g_last_screenshot{nullptr};

void HttpReplyDataCb(const happyhttp::Response* r, void* userdata, const unsigned char* data, int n) {
    (void)r;
    std::string* out = static_cast<std::string*>(userdata);
    if (out != nullptr && n > 0) {
        out->append(reinterpret_cast<const char*>(data), static_cast<size_t>(n));
    }
}

std::string ExtractHatHost() {
    std::string host = tangar_cfg::HatAddress();
    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        host.erase(colon);
    }
    if (host.empty()) {
        host = "hat.rom2.ru";
    }
    return host;
}

void SendScreenshotStatus(SOCKET s, const ScreenshotInfo* si, uint8_t done, const std::string& reply) {
    if (s == tangar_hat::SERR_NOTCREATED) {
        return;
    }
    tangar_hat::Packet p;
    p.WriteUInt32(0x5C0EE250u);
    p.WriteString(si->login);
    p.WriteUInt32(si->uid);
    p.WriteUInt8(done);
    if (done != 0) {
        p.WriteString(reply);
    }
    tangar_hat::SOCK_SendPacket(s, p, 24);
}

// Ported from my-a2mgr/network_protocol_ext.cpp:707 (Cl_ScreenshotThread).
DWORD WINAPI ScreenshotUploadThread(LPVOID param) {
    ScreenshotInfo* si = static_cast<ScreenshotInfo*>(param);
    SOCKET s = tangar_hat::SOCK_Connect(ExtractHatHost(), 8000, "0.0.0.0", 0);

    SendScreenshotStatus(s, si, 0, "");

    size_t pixel_count = static_cast<size_t>(si->width) * static_cast<size_t>(si->height);
    uint32_t* ndata = new uint32_t[pixel_count];
    const uint16_t* orp = si->data;

    for (int32_t row = 0; row < si->height; row++) {
        int32_t dst_y = si->flip ? (si->height - 1 - row) : row;
        uint32_t* rowp = ndata + static_cast<size_t>(dst_y) * static_cast<size_t>(si->width);
        for (int32_t x = 0; x < si->width; x++) {
            uint16_t a = *orp++;
            uint32_t r = (a & 0xF800u) >> 11;
            uint32_t g = (a & 0x07E0u) >> 5;
            uint32_t b = (a & 0x001Fu);
            r = r * 255u / 31u;
            g = g * 255u / 63u;
            b = b * 255u / 31u;
            *rowp++ = 0xFF000000u | (b << 16) | (g << 8) | r;
        }
    }

    delete[] si->data;
    si->data = nullptr;

    std::string post_reply;
    unsigned char* png_buf = nullptr;
    size_t png_size = 0;
    unsigned int encode_err = lodepng_encode32(
        &png_buf, &png_size,
        reinterpret_cast<const unsigned char*>(ndata),
        static_cast<unsigned int>(si->width),
        static_cast<unsigned int>(si->height));
    delete[] ndata;

    if (encode_err != 0) {
        SendScreenshotStatus(s, si, 1, post_reply);
        if (s != tangar_hat::SERR_NOTCREATED) {
            tangar_hat::SOCK_Destroy(s);
        }
        if (png_buf != nullptr) {
            free(png_buf);
        }
        delete si;
        return 0;
    }

    static const char kBoundaryPre[] =
        "------WebKitFormBoundaryePkpFF7tjBAqx29L\r\n"
        "Content-Disposition: form-data; name=\"p\"; filename=\"ps.png\"\r\n"
        "Content-Type: image/png\r\n\r\n";
    static const char kBoundaryPost[] =
        "\r\n------WebKitFormBoundaryePkpFF7tjBAqx29L--\r\n";

    size_t pre_len = sizeof(kBoundaryPre) - 1;
    size_t post_len = sizeof(kBoundaryPost) - 1;
    int post_size = static_cast<int>(pre_len + png_size + post_len);

    char path_buf[512];
    snprintf(path_buf, sizeof(path_buf), "/posts.php?login=%s&uid=%u", si->login.c_str(), si->uid);

    happyhttp::Connection hh_conn("api.allods2.eu", 8080);
    hh_conn.setcallbacks(nullptr, HttpReplyDataCb, nullptr, &post_reply);

    try {
        hh_conn.putrequest("POST", path_buf);
        hh_conn.putheader("Connection", "close");
        hh_conn.putheader("Content-Length", post_size);
        hh_conn.putheader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundaryePkpFF7tjBAqx29L");
        hh_conn.putheader("Accept", "text/plain");
        hh_conn.endheaders();

        hh_conn.send(reinterpret_cast<const unsigned char*>(kBoundaryPre), static_cast<int>(pre_len));
        hh_conn.send(png_buf, static_cast<int>(png_size));
        hh_conn.send(reinterpret_cast<const unsigned char*>(kBoundaryPost), static_cast<int>(post_len));

        while (hh_conn.outstanding()) {
            hh_conn.pump();
        }
    } catch (const happyhttp::Wobbly&) {
        post_reply.clear();
    }

    if (png_buf != nullptr) {
        free(png_buf);
    }

    SendScreenshotStatus(s, si, 1, post_reply);
    if (s != tangar_hat::SERR_NOTCREATED) {
        tangar_hat::SOCK_Destroy(s);
    }
    delete si;
    return 0;
}

} // namespace

// Ported from my-a2mgr/network_protocol_ext.cpp:853 (ProcLastSI / imp_MakeScreenshotNet @ 457ff4).
void ProcLastScreenshot() {
    if (g_selDrawBitmap.lpSurface == nullptr) {
        return;
    }

    ScreenshotInfo* si = g_last_screenshot.exchange(nullptr);
    if (si == nullptr) {
        return;
    }

    uint8_t* dst = reinterpret_cast<uint8_t*>(si->data);
    const uint8_t* src_base = static_cast<const uint8_t*>(g_selDrawBitmap.lpSurface);
    int32_t src_pitch = g_selDrawBitmap.lPitch;
    size_t row_bytes = static_cast<size_t>(si->width) * 2;

    for (int32_t y = 0; y < si->height; y++) {
        memcpy(dst + static_cast<size_t>(y) * row_bytes, src_base + y * src_pitch, row_bytes);
    }

    HANDLE thread = CreateThread(nullptr, 0, ScreenshotUploadThread, si, 0, nullptr);
    if (thread != nullptr) {
        CloseHandle(thread);
    } else {
        delete[] si->data;
        delete si;
    }
}

// Ported from my-a2mgr/network_protocol_ext.cpp:825 (Cl_ProcessServerPacket).
bool ProcessServerPacket(tangar_hat::Packet& pack) {
    uint8_t type = pack.ReadUInt8();
    if (type == 0x01) { // screenshot
        std::string login = pack.ReadString();
        uint32_t uid = pack.ReadUInt32();

        int32_t width = g_ScreenSize.right - g_ScreenSize.left;
        int32_t height = g_ScreenSize.bottom - g_ScreenSize.top;
        if (width <= 0 || height <= 0) {
            return true;
        }

        ScreenshotInfo* si = new ScreenshotInfo();
        si->login = login;
        si->uid = uid;
        si->width = width;
        si->height = height;
        si->data = new uint16_t[static_cast<size_t>(width) * static_cast<size_t>(height)];
        si->flip = false;

        ScreenshotInfo* prev = g_last_screenshot.exchange(si);
        if (prev != nullptr) {
            delete[] prev->data;
            delete prev;
        }

        CWnd* main_wnd = AfxGetMainWnd();
        if (main_wnd != nullptr && main_wnd->GetSafeHwnd() != nullptr && IsIconic(main_wnd->GetSafeHwnd())) {
            ProcLastScreenshot();
        }
        return true;
    }
    return false;
}

// Ported from my-a2mgr/network_protocol.cpp:225 (recv_).
int RecvClientFramed(SOCKET s, char* buf, int len, int flags) {
    (void)flags;

    static SOCKET recv_last_socket = INVALID_SOCKET;
    static bool recv_last_incomplete = false;
    static int32_t recv_last_remaining = -1;

    if (s != recv_last_socket) {
        recv_last_socket = s;
        recv_last_incomplete = false;
        recv_last_remaining = -1;
    }

    if (recv_last_incomplete) {
        int32_t to_read = recv_last_remaining < len ? recv_last_remaining : len;
        int r_ = recv(s, buf, to_read, 0);
        if (r_ > 0) {
            if (r_ == recv_last_remaining) {
                recv_last_incomplete = false;
                recv_last_remaining = -1;
            } else {
                recv_last_remaining -= r_;
            }
        }
        return r_;
    }

    int r = recv(s, buf, 4, 0);
    if (r != 4) {
        return -1;
    }
    if (*reinterpret_cast<int32_t*>(buf) == 0) {
        return 0;
    }

    if (!RecvExact(s, reinterpret_cast<uint8_t*>(buf + 4), 4)) {
        return -1;
    }

    int32_t pkt_size = *reinterpret_cast<int32_t*>(buf);
    uint32_t pkt_flags = *reinterpret_cast<uint32_t*>(buf + 4);

    if ((pkt_flags & 0x80100000u) == 0x80100000u) {
        if (pkt_size < 0 || pkt_size > 0x100000) {
            return -1;
        }

        std::vector<uint8_t> rd(static_cast<size_t>(pkt_size));
        if (pkt_size > 0 && !RecvExact(s, rd.data(), pkt_size)) {
            return -1;
        }

        tangar_hat::Packet pack;
        pack.Reset();
        for (int32_t i = 0; i < pkt_size; i++) {
            pack.WriteUInt8(rd[static_cast<size_t>(i)]);
        }
        pack.Seek(0);

        if (!ProcessServerPacket(pack)) {
            return -1;
        }

        // Synthesize an encrypted 5-byte SERVER_TICK (0x64) frame so
        // CLlDriver::RecvThreadTcp sees a valid non-empty game packet.
        *reinterpret_cast<uint32_t*>(buf) = 5;
        *reinterpret_cast<uint32_t*>(buf + 4) = 0x01000000u;
        *reinterpret_cast<uint8_t*>(buf + 8) = 0x64;
        *reinterpret_cast<uint32_t*>(buf + 9) = 1;
        tangar_hat::PACKET_XorByKey(reinterpret_cast<uint8_t*>(buf + 8), 5, PROTO_VER);
        return 8 + 5;
    }

    if (pkt_size < 0 || pkt_size + 8 > len) {
        return -1;
    }

    r = recv(s, buf + 8, pkt_size, 0);
    if (r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
        recv_last_incomplete = true;
        recv_last_remaining = pkt_size;
        return 8;
    }
    if (r <= 0) {
        return -1;
    }
    if (r != pkt_size) {
        recv_last_incomplete = true;
        recv_last_remaining = pkt_size - r;
    }
    return r + 8;
}

} // namespace tangar_client
