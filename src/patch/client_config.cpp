// Ported from my-a2mgr config.cpp (ParseConfig2x / ParseConfig1x, trimmed to
// the knobs consumed by ported client features).

#include "client_config.h"

#include <windows.h>

#include "hotkeys.h"

#include <cctype>
#include <cstdint>
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
uint32_t g_scrollintv = 35;
bool r_grid = false;
uint32_t r_grid_color = 255;
bool z_lang_override = false;
bool z_russian = false;
bool stats_based_level_selection = true; // a2mgr default: on
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

void Parse2x(std::ifstream& f) {
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
            g_scrollintv = StrToUInt(cmd[1]);
        } else if (cmd[0] == "r_fps" && cmd.size() == 2) {
            r_fps = StrToBool(cmd[1]);
        } else if (cmd[0] == "r_time" && cmd.size() == 2) {
            r_time = StrToBool(cmd[1]);
        } else if (cmd[0] == "r_stopwatch" && cmd.size() == 2) {
            r_stopwatch = StrToBool(cmd[1]);
        } else if (cmd[0] == "r_grid" && cmd.size() == 2) {
            r_grid = StrToBool(cmd[1]);
        } else if (cmd[0] == "r_grid_color" && cmd.size() == 2) {
            // accepts a decimal color or 0xRRGGBB hex
            if (cmd[1].size() > 2 && cmd[1][0] == '0' && (cmd[1][1] == 'x' || cmd[1][1] == 'X')) {
                r_grid_color = (uint32_t)strtoul(cmd[1].c_str() + 2, nullptr, 16);
            } else if (CheckInt(cmd[1])) {
                r_grid_color = StrToUInt(cmd[1]);
            }
        } else if (cmd[0] == "z_language" && cmd.size() == 2) {
            std::string lang;
            for (char c : cmd[1]) {
                lang += (char)tolower((unsigned char)c);
            }
            z_lang_override = true;
            z_russian = (lang == "russian" || lang == "ru");
        } else if (cmd[0] == "stats_based_level_selection" && cmd.size() == 2) {
            stats_based_level_selection = StrToBool(cmd[1]);
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

} // namespace tangar_cfg
