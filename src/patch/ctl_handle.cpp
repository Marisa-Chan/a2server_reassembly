// Ported from my-srvmgr/ctl_handle.cpp and srvmgr.cpp (DllMain ThreadProc).

#include "ctl_handle.h"

#include "config_new.h"
#include "kick_char.h"
#include "tangar_log.h"

#include "../net.h"
#include "../player.h"
#include "../server.h"

#include <windows.h>

#include <deque>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace tangar_ctl {

namespace {

enum class ActionKind {
    Say,
    Cheat,
};

struct ControlAction {
    ActionKind kind;
    std::string caster;
    std::string text;
};

HANDLE g_thread_handle = nullptr;
HANDLE g_stop_event = nullptr;
std::mutex g_queue_mutex;
std::deque<ControlAction> g_pending_actions;
bool g_full_rights_override = false;

void ConvertLineFromWin1251(std::string& s) {
    if (!s.empty() && s.back() == '\r') {
        s.pop_back();
    }
    if (!s.empty()) {
        CharToOemBuffA(s.c_str(), &s[0], static_cast<DWORD>(s.size()));
    }
}

std::vector<std::string> ReadLines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        return lines;
    }
    std::string line;
    while (std::getline(f, line)) {
        lines.push_back(line);
    }
    return lines;
}

DWORD WINAPI ControlThreadProc(LPVOID /*lpParam*/) {
    if (WaitForSingleObject(g_stop_event, 5000) == WAIT_OBJECT_0) {
        return 0;
    }

    while (WaitForSingleObject(g_stop_event, 0) != WAIT_OBJECT_0) {
        if (!Config::ControlDirectory.empty()) {
            std::string mask = Config::ControlDirectory + "\\*";
            WIN32_FIND_DATAA fd = {};
            HANDLE h = FindFirstFileA(mask.c_str(), &fd);
            if (h != INVALID_HANDLE_VALUE) {
                bool found = true;
                while (found) {
                    if (fd.cFileName[0] != '.' &&
                        (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                        std::string full_path = Config::ControlDirectory + "\\" + std::string(fd.cFileName);
                        std::string prefix = fd.cFileName;
                        size_t dot_pos = prefix.find('.');
                        if (dot_pos != std::string::npos) {
                            prefix.erase(dot_pos);
                        }

                        bool identified = true;
                        if (prefix == "say") {
                            std::vector<std::string> ll = ReadLines(full_path);
                            if (!ll.empty()) {
                                ConvertLineFromWin1251(ll[0]);
                                ControlAction act;
                                act.kind = ActionKind::Say;
                                act.text = ll[0];
                                std::lock_guard<std::mutex> lock(g_queue_mutex);
                                g_pending_actions.push_back(std::move(act));
                            }
                        } else if (prefix == "cheat") {
                            std::vector<std::string> ll = ReadLines(full_path);
                            if (!ll.empty()) {
                                ConvertLineFromWin1251(ll[0]);
                                ControlAction act;
                                act.kind = ActionKind::Cheat;
                                if (ll.size() == 1) {
                                    act.text = ll[0];
                                } else {
                                    ConvertLineFromWin1251(ll[1]);
                                    act.caster = ll[0];
                                    act.text = ll[1];
                                }
                                std::lock_guard<std::mutex> lock(g_queue_mutex);
                                g_pending_actions.push_back(std::move(act));
                            }
                        } else {
                            identified = false;
                        }

                        if (identified) {
                            DeleteFileA(full_path.c_str());
                        }
                    }
                    found = (FindNextFileA(h, &fd) != 0);
                }
                FindClose(h);
            }
        }

        if (WaitForSingleObject(g_stop_event, 100) == WAIT_OBJECT_0) {
            break;
        }
    }

    return 0;
}

} // namespace

void StartControlThread() {
    if (g_thread_handle != nullptr) {
        return;
    }

    g_stop_event = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (g_stop_event == nullptr) {
        return;
    }

    DWORD thread_id = 0;
    g_thread_handle = CreateThread(nullptr, 0, &ControlThreadProc, nullptr, 0, &thread_id);
    if (g_thread_handle == nullptr) {
        CloseHandle(g_stop_event);
        g_stop_event = nullptr;
    }
}

void StopControlThread() {
    if (g_stop_event != nullptr) {
        SetEvent(g_stop_event);
    }
    if (g_thread_handle != nullptr) {
        WaitForSingleObject(g_thread_handle, INFINITE);
        CloseHandle(g_thread_handle);
        g_thread_handle = nullptr;
    }
    if (g_stop_event != nullptr) {
        CloseHandle(g_stop_event);
        g_stop_event = nullptr;
    }
}

void PollControlActions() {
    if (g_Server == nullptr) {
        return;
    }

    std::deque<ControlAction> batch;
    {
        std::lock_guard<std::mutex> lock(g_queue_mutex);
        if (g_pending_actions.empty()) {
            return;
        }
        batch.swap(g_pending_actions);
    }

    for (const ControlAction& act : batch) {
        if (act.kind == ActionKind::Say) {
            Printf("[ctl] %s", act.text.c_str());
            g_NetStru1_main.AnnouncePlayerName(act.text.c_str(), nullptr);
        } else if (act.kind == ActionKind::Cheat) {
            if (!act.caster.empty()) {
                Player* pp = FindPlayerByNickname(act.caster.c_str());
                if (pp != nullptr) {
                    Printf("[ctl] %s: %s", act.caster.c_str(), act.text.c_str());
                    g_full_rights_override = true;
                    g_Server->CheatCommand(pp, CString(act.text.c_str()));
                    g_full_rights_override = false;
                }
            } else {
                Printf("[ctl] <server>: %s", act.text.c_str());
                g_Server->CheatCommand(nullptr, CString(act.text.c_str()));
            }
        }
    }
}

bool IsFullRightsOverride() {
    return g_full_rights_override;
}

} // namespace tangar_ctl
