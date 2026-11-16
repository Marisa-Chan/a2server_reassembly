// Centralized network debug logging - see netlog.h.

#include "netlog.h"

#include <windows.h>

#include <cstdio>
#include <cstring>
#include <mutex>

#include "../util.h" // XorData

namespace netlog {

namespace {

std::mutex g_mutex;
FILE* g_file = nullptr;
bool g_tried = false;

// Upper hex dump cap per log call; larger payloads are truncated.
const size_t kHexCap = 400;

void open_file() {
    if (g_tried) {
        return;
    }
    g_tried = true;

    // Logging is opt-in: only write when the command line carries
    // --net-log=DIRECTORY (the value ends at whitespace or a quote).
    const char* cli = GetCommandLineA();
    const char* flag = cli != nullptr ? strstr(cli, "--net-log=") : nullptr;
    if (flag == nullptr) {
        return;
    }
    flag += strlen("--net-log=");

    char dir[MAX_PATH] = {};
    size_t n = 0;
    while (*flag != '\0' && *flag != ' ' && *flag != '\t' && *flag != '"') {
        if (n < sizeof(dir) - 1) {
            dir[n++] = *flag;
        }
        flag++;
    }
    if (n == 0) {
        return;
    }

    char path[MAX_PATH];
#ifdef A2CLIENT
    _snprintf(path, sizeof(path) - 1, "%s\\client.log", dir);
#else
    _snprintf(path, sizeof(path) - 1, "%s\\server.log", dir);
#endif

    char dirbuf[MAX_PATH];
    strncpy(dirbuf, path, sizeof(dirbuf) - 1);
    char* slash = strrchr(dirbuf, '\\');
    if (slash != nullptr) {
        *slash = '\0';
        CreateDirectoryA(dirbuf, nullptr); // ok if it already exists
    }

    g_file = fopen(path, "a");

    SYSTEMTIME tm = {};
    GetLocalTime(&tm);
    char exe[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe, MAX_PATH - 1);
    fprintf(g_file, "=== session start %04u-%02u-%02u %02u:%02u:%02u pid=%lu exe=%s ===\n",
            tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond,
            GetCurrentProcessId(), exe);
    fflush(g_file);
}

void write_line(const char* dir, const char* fmt, va_list args) {
    std::lock_guard<std::mutex> lock(g_mutex);
    open_file();
    if (g_file == nullptr) {
        return;
    }

    SYSTEMTIME tm = {};
    GetLocalTime(&tm);
    fprintf(g_file, "[%02u:%02u:%02u.%03u] %-8s ", tm.wHour, tm.wMinute, tm.wSecond,
            tm.wMilliseconds, dir);
    vfprintf(g_file, fmt, args);
    fputc('\n', g_file);
    fflush(g_file);
}

void dump_hex(FILE* f, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        fprintf(f, "             %04x  ", (unsigned)i);
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                fprintf(f, "%02x ", data[i + j]);
            } else {
                fputs("   ", f);
            }
        }
        fputs(" |", f);
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = data[i + j];
            fputc(c >= 0x20 && c < 0x7f ? c : '.', f);
        }
        fputs("|\n", f);
    }
    if (len > kHexCap) {
        fprintf(f, "             ... (%u more bytes)\n", (unsigned)(len - kHexCap));
    }
}

// ------------------------------------------------- game packet stream parsing
// Size table derived from the retail Packet::GetDataSize() overrides (same
// tables the a2probe_scratch python probes use).

const uint8_t kUnitUpdOffs[32] = {2, 2, 5, 4, 3, 2, 1, 2, 4, 4, 4, 4, 4, 2, 2, 1,
                                  2, 2, 2, 1, 5, 0, 0, 0, 5, 16, 0, 0, 0, 8, 0, 24};

uint32_t unitupd_datalen(uint32_t flags) {
    uint32_t n = 0;
    uint32_t sh = 1;
    for (int i = 0; i < 32; i++) {
        if (flags & sh) {
            n += kUnitUpdOffs[i];
        }
        sh <<= 1;
    }
    return n;
}

// Returns the total length of the packet starting at data[0], or 0 if unknown.
uint32_t packet_len(const uint8_t* d, size_t avail) {
    if (avail < 1) {
        return 0;
    }
    uint8_t id = d[0];
    auto u8 = [&](size_t o) -> uint32_t { return o < avail ? d[o] : 0; };
    auto u16 = [&](size_t o) -> uint32_t { return o + 1 < avail ? d[o] | (d[o + 1] << 8) : 0; };
    auto u32 = [&](size_t o) -> uint32_t {
        return o + 3 < avail ? d[o] | (d[o + 1] << 8) | (d[o + 2] << 16) | ((uint32_t)d[o + 3] << 24) : 0;
    };

    switch (id) {
        case 0x03: case 0x05: case 0x09: case 0x0a: case 0xaf: case 0xb7: case 0xb8:
        case 0x4a: case 0x4b: case 0x4c: case 0x5a: case 0xc8: case 0xd6: case 0xdb:
        case 0xdc:
            return 1;
        case 0x04: case 0x0b: case 0x39: case 0x3b: case 0x5c: case 0xca: case 0xcc:
        case 0xd0: case 0xd8: case 0xe1: case 0x23: case 0x3a: case 0x3d: case 0x3e:
        case 0x3f: case 0x40: case 0x46: case 0x48: case 0x83: case 0x84: case 0x92:
        case 0x94: case 0xaa: case 0xc3: case 0x65: case 0x67: case 0x97: case 0xab:
        case 0xac: case 0xad: case 0xb4: case 0xb5: case 0xb6:
            return 9;
        case 0x0e: case 0x10: case 0xb3: case 0xc1: case 0xdf: case 0x64:
            return 5;
        case 0x6b: case 0x6d: case 0x71: case 0x72: case 0x73: case 0x82:
            return 5;
        case 0x88: case 0x89:
            return 4;
        case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x38: case 0x69:
        case 0x6a: case 0x74:
            return 3;
        case 0x06: case 0x07: case 0x08: case 0x0f: case 0x49: case 0x4e: case 0x5b:
        case 0x91: case 0x93: case 0x96: case 0xae: case 0xbf: case 0xc9: case 0xd1:
        case 0xd2: case 0xd3:
            return 8 + u16(5);
        case 0x12: case 0x13: case 0x14: case 0x16: case 0x17: case 0x18: case 0x19:
        case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f: case 0x21:
        case 0x24: case 0x25: case 0x26: case 0x4d:
            return 10 + 2 * u8(9);
        case 0x45: case 0x8a: case 0x8c: case 0x9b: case 0xb9: case 0xbb: case 0xbc:
            return 5 + 2 * u32(1);
        case 0x6c: case 0x6e: case 0x6f: case 0x70: {
            uint32_t fs = id == 0x6c ? 4 : (id == 0x6e ? 1 : (id == 0x6f ? 2 : 3));
            uint32_t flags = 0;
            for (uint32_t i = 0; i < fs; i++) {
                flags |= (uint32_t)d[3 + i] << (8 * i);
            }
            return 3 + fs + unitupd_datalen(flags);
        }
        case 0xba: case 0xbe: case 0xc2: case 0xcb: case 0xcd: case 0xce: case 0xcf:
        case 0xd4: case 0xd9: case 0xda: case 0xdd: case 0xde: case 0xe0:
            return 5 + u32(1);
        case 0x22:
            return 11;
        case 0x7a:
            return 9;
        case 0x86: case 0x8b:
            return 8;
        case 0x87:
            return 19;
        case 0x9c: {
            uint32_t flags = u16(3);
            uint32_t bits = 0;
            for (uint32_t i = 0; i < 16; i++) {
                if (flags & (1u << i)) {
                    bits++;
                }
            }
            return 5 + 2 * bits;
        }
        case 0xc0:
            return 45 + u32(41);
        case 0x76:
            return 10 + u16(8);
        default:
            return 0;
    }
}

struct PktName {
    uint8_t id;
    const char* name;
};

const PktName kNames[] = {
    {0x00, "AUTH"},        {0x02, "ENTER_GAME"},  {0x03, "LEAVE"},
    {0x36, "PING36"},      {0x64, "TICK"},        {0x6c, "UNITUPD"},
    {0x71, "MOVE71"},      {0x76, "NAME"},        {0x7a, "CAST"},
    {0x83, "OPEN_SHOP"},   {0x84, "OPEN_INN"},    {0x86, "EFFECT_MOVE"},
    {0x87, "AOE_ZONE"},    {0x91, "CHAT"},        {0x93, "PLAYER_ANNOUNCED"},
    {0x96, "PLAYER_INFO"}, {0x97, "PLAYER_LEFT"}, {0x9b, "TERRAIN_DATA"},
    {0x9c, "ENEMY_EQUIP"}, {0xaa, "MAP_REVEAL"},  {0xab, "VIEW_POS"},
    {0xae, "ANNOUNCEMENT"},{0xaf, "SRV_SHUTDOWN"},{0xb4, "MISSION_RESULT"},
    {0xb5, "MISSION_DONE"},{0xb6, "GAME_EVENT"},  {0xb9, "DIPLOMACY"},
    {0xbf, "MAP_INFO"},    {0xc0, "FILE_CHUNK"},  {0xc1, "LATENCY"},
    {0xc2, "TEAM_STATE"},  {0xc3, "GAME_START"},  {0xc8, "HAT_LOGOUT"},
    {0xc9, "HAT_CREDS"},   {0xca, "HAT_PICK_CHAR"},{0xcb, "HAT_CONNECT"},
    {0xcc, "HAT_CHAR_REQ"},{0xcd, "SERVER_LIST"}, {0xce, "HAT_CHAR_LIST"},
    {0xcf, "HAT_CHAR_DATA"},{0xd0, "HAT_TICKET"}, {0xd1, "HAT_LOGIN"},
    {0xd2, "HAT_LOBBY_INFO"},{0xd3, "HAT_CHAR_RETURN"},{0xd5, "HAT_LOGIN_OK"},
    {0xd8, "HAT_JOIN_OK"}, {0xd9, "HAT_JOIN_FAIL"},{0xda, "HAT_SESSION_SETUP"},
    {0xdb, "HAT_SESSION_FAIL"},{0xdd, "HAT_PLAYER_JOIN"},{0xdf, "HAT_ERROR"},
    {0xe0, "HAT_MESSAGE"}, {0xe1, "HAT_PLAYER_CHECK"},
};

} // namespace

const char* pkt_name(uint8_t id) {
    for (const PktName& n : kNames) {
        if (n.id == id) {
            return n.name;
        }
    }
    return "";
}

void logf(const char* dir, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_line(dir, fmt, args);
    va_end(args);
}

void hex(const char* dir, const void* data, size_t len, const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(g_mutex);
    open_file();
    if (g_file == nullptr) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    SYSTEMTIME tm = {};
    GetLocalTime(&tm);
    fprintf(g_file, "[%02u:%02u:%02u.%03u] %-8s ", tm.wHour, tm.wMinute, tm.wSecond,
            tm.wMilliseconds, dir);
    vfprintf(g_file, fmt, args);
    va_end(args);
    fprintf(g_file, " (%u bytes)\n", (unsigned)len);
    dump_hex(g_file, static_cast<const uint8_t*>(data), len < kHexCap ? len : kHexCap);
    fflush(g_file);
}

void frame(const char* dir, long peer_port, const void* framedata, size_t len, const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(g_mutex);
    open_file();
    if (g_file == nullptr) {
        return;
    }

    const uint8_t* frame_bytes = static_cast<const uint8_t*>(framedata);
    uint16_t payload_len = len >= 2 ? (uint16_t)(frame_bytes[0] | (frame_bytes[1] << 8)) : 0;
    uint8_t pkt_num = len >= 8 ? frame_bytes[7] : 0;

    va_list args;
    va_start(args, fmt);
    SYSTEMTIME tm = {};
    GetLocalTime(&tm);
    fprintf(g_file, "[%02u:%02u:%02u.%03u] %-8s ", tm.wHour, tm.wMinute, tm.wSecond,
            tm.wMilliseconds, dir);
    vfprintf(g_file, fmt, args);
    va_end(args);
    fprintf(g_file, " port=%ld frame=%u%c\n", peer_port, (unsigned)payload_len,
            pkt_num ? '*' : ' ');

    if (payload_len > 0 && 8 + (size_t)payload_len <= len) {
        // De-XOR a copy of the payload and walk the packet id stream.
        uint8_t plain[1024];
        size_t plen = payload_len < sizeof(plain) ? payload_len : sizeof(plain);
        memcpy(plain, frame_bytes + 8, plen);
        XorData(plain, (uint32_t)plen);

        fprintf(g_file, "             ids:");
        size_t off = 0;
        while (off < plen) {
            uint32_t l = packet_len(plain + off, plen - off);
            if (l == 0) {
                fprintf(g_file, " 0x%02x?", plain[off]);
                break;
            }
            const char* name = pkt_name(plain[off]);
            if (name[0] != '\0') {
                fprintf(g_file, " %s", name);
            } else {
                fprintf(g_file, " 0x%02x", plain[off]);
            }
            off += l;
        }
        fputc('\n', g_file);
        dump_hex(g_file, plain, plen < kHexCap ? plen : kHexCap);
    }
    fflush(g_file);
}

void conn(const char* what, uint64_t sock, const char* peer_ip, int peer_port) {
    if (peer_ip != nullptr) {
        logf("EVT", "%s sock=%llx peer=%s:%d", what, (unsigned long long)sock, peer_ip, peer_port);
    } else {
        logf("EVT", "%s sock=%llx", what, (unsigned long long)sock);
    }
}

} // namespace netlog
