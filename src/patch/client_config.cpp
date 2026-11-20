// Ported from a2mgr config.cpp (ParseConfig2x / ParseConfig1x, trimmed to
// the knobs consumed by ported client features).

#include "client_config.h"

#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>

#include "../game_app.h"
#include "../ingame.h"
#include "../main_window.h"
#include "../vis_playing_area.h"
#include "config_new.h"
#include "hotkeys.h"
#include "tangar_log.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace tangar_cfg {

bool r_fps = false;
bool r_time = false;
bool r_stopwatch = false;
bool show_stopwatch = false;
uint32_t show_stopwatch_frozen = 0;
int64_t stopwatch_start = 0;
uint32_t g_scrollintv = 25;
bool r_grid = false;
uint32_t r_grid_color = 0x3800;
uint32_t r_width = 640;
uint32_t r_height = 480;
uint32_t z_resolution = 0;
bool z_lang_override = false;
bool z_russian = true;
bool stats_based_level_selection = true; // a2mgr default: on
bool z_screendir = true;
bool z_logsdir = true;
bool g_altchat = false;
int32_t z_softcore = 0; // 0=off, 1=force softcore, 2=force sandbox (no config parse in a2mgr)

namespace {

std::string TrimStr(const std::string& s) {
    size_t a = 0;
    size_t b = s.size();
    while (a < b && isspace((unsigned char)s[a])) {
        a++;
    }
    while (b > a && isspace((unsigned char)s[b - 1])) {
        b--;
    }
    return s.substr(a, b - a);
}

std::vector<std::string> SplitSpaces(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (isspace((unsigned char)c)) {
            if (!cur.empty()) {
                out.push_back(cur);
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) {
        out.push_back(cur);
    }
    return out;
}

bool StrToBool(const std::string& s) {
    std::string t;
    for (char c : s) {
        t += (char)tolower((unsigned char)c);
    }
    return t == "yes" || t == "true" || t == "1" || t == "on";
}

bool CheckInt(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!isdigit((unsigned char)c)) {
            return false;
        }
    }
    return true;
}

uint32_t StrToUInt(const std::string& s) {
    uint32_t v = 0;
    for (char c : s) {
        if (!isdigit((unsigned char)c)) {
            break;
        }
        v = v * 10 + (uint32_t)(c - '0');
    }
    return v;
}

void StripComment(std::string& wh) {
    size_t cw = wh.find_first_of("#;");
    if (cw != std::string::npos) {
        wh.erase(cw);
    }
    if (wh.empty()) {
        return;
    }
    cw = wh.find("//");
    if (cw != std::string::npos) {
        wh.erase(cw);
    }
}

std::vector<std::string> SplitChar(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    out.push_back(cur);
    return out;
}

void Parse2x(std::ifstream& f) {
    uint32_t r_widtho = r_width;
    uint32_t r_heighto = r_height;
    bool invalidwh = false;
    z_resolution = 0;

    std::string wh;
    while (std::getline(f, wh)) {
        wh = TrimStr(wh);
        if (wh.empty()) {
            continue;
        }
        StripComment(wh);
        if (wh.empty()) {
            continue;
        }

        std::vector<std::string> cmd = SplitSpaces(wh);
        if (cmd.empty() || cmd.size() > 2) {
            continue;
        }

        if (cmd[0] == "g_scrollspeed" && cmd.size() == 2 && CheckInt(cmd[1])) {
            uint32_t speed = StrToUInt(cmd[1]);
            if (speed > 100) {
                speed = 100;
            }
            g_scrollintv = 10 * (100 - speed);
        } else if (cmd[0] == "r_width" && cmd.size() == 2) {
            if (CheckInt(cmd[1])) {
                r_width = StrToUInt(cmd[1]);
                if (!invalidwh) {
                    z_resolution++;
                }
            } else {
                invalidwh = true;
            }
        } else if (cmd[0] == "r_height" && cmd.size() == 2) {
            if (CheckInt(cmd[1])) {
                r_height = StrToUInt(cmd[1]);
                if (!invalidwh) {
                    z_resolution++;
                }
            } else {
                invalidwh = true;
            }
        } else if (cmd[0] == "r_fps") {
            r_fps = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "r_time") {
            r_time = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "r_stopwatch") {
            r_stopwatch = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "z_screendir") {
            z_screendir = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "z_logsdir") {
            z_logsdir = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "g_altchat") {
            g_altchat = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "r_grid") {
            r_grid = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd[0] == "r_grid_color" && cmd.size() == 2) {
            std::vector<std::string> parmc = SplitChar(cmd[1], ':');
            if (parmc.size() == 3 && CheckInt(parmc[0]) && CheckInt(parmc[1]) && CheckInt(parmc[2])) {
                uint32_t c_r = (uint32_t)((float)StrToUInt(parmc[0]) / 255.0f * 31.0f);
                uint32_t c_g = (uint32_t)((float)StrToUInt(parmc[1]) / 255.0f * 63.0f);
                uint32_t c_b = (uint32_t)((float)StrToUInt(parmc[2]) / 255.0f * 31.0f);
                uint32_t val = 0;
                val |= (c_r & 0x1F);
                val <<= 5;
                val |= (c_g & 0x3F);
                val <<= 6;
                val |= (c_b & 0x1F);
                r_grid_color = val;
            } else if (cmd[1].size() > 2 && cmd[1][0] == '0' && (cmd[1][1] == 'x' || cmd[1][1] == 'X')) {
                r_grid_color = (uint32_t)strtoul(cmd[1].c_str() + 2, nullptr, 16);
            } else if (CheckInt(cmd[1])) {
                r_grid_color = StrToUInt(cmd[1]);
            }
        } else if (cmd[0] == "z_language" && cmd.size() == 2) {
            std::string lang;
            for (char c : cmd[1]) {
                lang += (char)tolower((unsigned char)c);
            }
            if (lang == "rus" || lang == "russian") {
                z_russian = true;
                z_lang_override = true;
            } else if (lang == "eng" || lang == "english") {
                z_russian = false;
                z_lang_override = true;
            } else {
                z_lang_override = false;
            }
        } else if (cmd[0] == "stats_based_level_selection") {
            stats_based_level_selection = (cmd.size() == 2) ? StrToBool(cmd[1]) : true;
        } else if (cmd.size() == 2 && cmd[0].compare(0, 6, "alias:") == 0) {
            std::string name = cmd[0].substr(6);
            std::string action = cmd[1];
            for (char& c : name) {
                c = (char)tolower((unsigned char)c);
            }
            for (char& c : action) {
                c = (char)tolower((unsigned char)c);
            }
            tangar_hotkeys::ParseAlias(name, action);
        }
    }

    if (z_resolution != 2) {
        invalidwh = true;
    }
    if (invalidwh) {
        r_width = r_widtho;
        r_height = r_heighto;
        z_resolution = 0;
    }
}

void Parse1x(std::ifstream& f) {
    std::string wh;
    while (std::getline(f, wh)) {
        wh = TrimStr(wh);
        size_t cw = wh.find_first_of('=');
        if (cw == std::string::npos) {
            continue;
        }

        std::string prm = TrimStr(wh.substr(0, cw));
        std::string val = TrimStr(wh.substr(cw + 1));

        if (prm == "scroll_inverval") {
            g_scrollintv = StrToUInt(val);
        } else if (prm == "show_fps") {
            r_fps = StrToBool(val);
        } else if (prm == "show_watches") {
            r_time = StrToBool(val);
        } else if (prm == "alt_chat") {
            g_altchat = StrToBool(val);
        }
    }
}

} // namespace

namespace {

std::vector<std::string> SplitCommandLine(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool in_quotes = false;
    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes && (c == ' ' || c == '\t')) {
            if (!cur.empty()) {
                out.push_back(cur);
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) {
        out.push_back(cur);
    }
    return out;
}

} // namespace

void GetResolution(int32_t* w, int32_t* h) {
    *w = 640;
    *h = 480;

    if (z_resolution == 0) {
        DEVMODEA mode = {};
        mode.dmSize = sizeof(mode);
        EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &mode);
        if (mode.dmPelsWidth >= 1024 && mode.dmPelsHeight >= 768) {
            *w = 1024;
            *h = 768;
        } else if (mode.dmPelsWidth >= 800 && mode.dmPelsHeight >= 600) {
            *w = 800;
            *h = 600;
        }
    } else {
        *w = (int32_t)r_width;
        *h = (int32_t)r_height;
    }

    std::vector<std::string> parms = SplitCommandLine(GetCommandLineA());
    for (size_t i = 0; i < parms.size(); ++i) {
        if (parms[i] == "-640") {
            *w = 640;
            *h = 480;
        } else if (parms[i] == "-800") {
            *w = 800;
            *h = 600;
        } else if (parms[i] == "-1024") {
            *w = 1024;
            *h = 768;
        } else if (parms[i].compare(0, 6, "--res:") == 0) {
            int w1 = 0;
            int h1 = 0;
            if (sscanf_s(parms[i].c_str(), "--res:%dx%d", &w1, &h1) == 2 && w1 > 0 && h1 > 0) {
                *w = w1;
                *h = h1;
            }
        }
    }
}

const char* HatAddress() {
    static std::string hat;
    if (!hat.empty()) {
        return hat.c_str();
    }

    std::vector<std::string> args = SplitCommandLine(GetCommandLineA());
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == "-hat") {
            hat = args[i + 1];
        }
    }

    if (hat.empty()) {
        hat = "hat.rom2.ru";
    }
    if (hat.size() > 255) {
        hat.resize(255);
    }
    return hat.c_str();
}

void InitClientLogFile() {
    if (z_logsdir && GetFileAttributesA("logs") == INVALID_FILE_ATTRIBUTES) {
        CreateDirectoryA("logs", nullptr);
    }

    std::string wherelog = "allods2";
    if (z_logsdir) {
        time_t t = time(nullptr);
        struct tm tmv = {};
        if (localtime_s(&tmv, &t) == 0) {
            char buf[128] = {};
            _snprintf(buf, sizeof(buf) - 1, "logs\\allods2_%02u-%02u-%04u",
                      static_cast<unsigned>(tmv.tm_mday),
                      static_cast<unsigned>(tmv.tm_mon + 1),
                      static_cast<unsigned>(tmv.tm_year + 1900));
            wherelog = buf;
        } else {
            wherelog = "logs\\allods2";
        }
    }

    DWORD self_pid = GetCurrentProcessId();
    char self_path[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, self_path, MAX_PATH);
    const char* self_base = strrchr(self_path, '\\');
    self_base = (self_base != nullptr) ? (self_base + 1) : self_path;

    uint32_t other_instances = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe = {};
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe)) {
            do {
                if (pe.th32ProcessID != self_pid && _stricmp(pe.szExeFile, self_base) == 0) {
                    other_instances++;
                }
            } while (Process32Next(snap, &pe));
        }
        CloseHandle(snap);
    }

    if (other_instances != 0) {
        char pid_buf[32] = {};
        _snprintf(pid_buf, sizeof(pid_buf) - 1, ".%lu", static_cast<unsigned long>(self_pid));
        wherelog += pid_buf;
    }
    wherelog += ".log";

    bool pre_exists = (GetFileAttributesA(wherelog.c_str()) != INVALID_FILE_ATTRIBUTES);
    Config::LogFile = wherelog;
    if (pre_exists) {
        FILE* fp = fopen(Config::LogFile.c_str(), "a");
        if (fp != nullptr) {
            fputc('\n', fp);
            fclose(fp);
        }
    }
    log_format("Rage of Mages II (v1.07)\n");
}

void ParseConfig() {
    std::ifstream f;
    bool oldfmt = false;
    f.open("allods2.cfg", std::ios::in);
    if (!f.is_open()) {
        f.open("allods2.conf", std::ios::in);
        if (!f.is_open()) {
            return;
        }
        oldfmt = true;
    }

    if (oldfmt) {
        Parse1x(f);
    } else {
        Parse2x(f);
    }

    f.close();
}

void WriteChat(const char* msg) {
    if (msg == nullptr) {
        return;
    }
    MainWindow* main_wnd = (MainWindow*)AfxGetMainWnd();
    if (main_wnd != nullptr && main_wnd->vis_map_context != nullptr) {
        main_wnd->vis_map_context->msglog.Add(msg, clr_log_sblack, 30000);
    } else {
        LogMessage(CString(msg));
    }
}

} // namespace tangar_cfg
