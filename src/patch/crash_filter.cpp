// Ported from my-srvmgr/crash_filter.cpp and srvmgr.cpp (DllMain crash save),
// extended with timestamped MiniDumpWriteDump (.dmp) output via dbghelp.

#include "crash_filter.h"

#include "config_new.h"
#include "tangar_log.h"

#include "../game_app.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

#include <windows.h>
#include <dbghelp.h>

#include <cstdint>
#include <cstdio>

namespace tangar_crash {

namespace {

volatile LONG g_in_crash_handler = 0;

void WriteMinidump(EXCEPTION_POINTERS* info) {
    SYSTEMTIME tm = {};
    GetLocalTime(&tm);

    char dmp_path[MAX_PATH] = {};
    _snprintf(dmp_path, sizeof(dmp_path) - 1,
              "crash_%04u%02u%02u_%02u%02u%02u.dmp",
              static_cast<unsigned>(tm.wYear),
              static_cast<unsigned>(tm.wMonth),
              static_cast<unsigned>(tm.wDay),
              static_cast<unsigned>(tm.wHour),
              static_cast<unsigned>(tm.wMinute),
              static_cast<unsigned>(tm.wSecond));

    HANDLE hFile = CreateFileA(dmp_path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        log_format("[crash_filter] Failed to create minidump file %s (err=%lu)\n",
                   dmp_path, GetLastError());
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mdei = {};
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = info;
    mdei.ClientPointers = FALSE;

    BOOL ok = MiniDumpWriteDump(GetCurrentProcess(),
                                GetCurrentProcessId(),
                                hFile,
                                MiniDumpNormal,
                                info != nullptr ? &mdei : nullptr,
                                nullptr,
                                nullptr);
    CloseHandle(hFile);

    if (ok) {
        log_format("[crash_filter] Wrote minidump: %s\n", dmp_path);
    } else {
        log_format("[crash_filter] MiniDumpWriteDump failed for %s (err=%lu)\n",
                   dmp_path, GetLastError());
    }
}

void PrintStackTrace(uintptr_t ebp) {
    char trace_buf[2048] = {};
    int pos = _snprintf(trace_buf, sizeof(trace_buf) - 1,
                        "0x%08Xh <= ", static_cast<unsigned int>(ebp));

    if (ebp != 0 && (ebp & 3u) == 0 && !IsBadReadPtr(reinterpret_cast<const void*>(ebp), sizeof(uintptr_t))) {
        uintptr_t stebp = *reinterpret_cast<const uintptr_t*>(ebp);
        for (int depth = 0; depth < 64; ++depth) {
            if (stebp == 0 || (stebp & 3u) != 0 ||
                IsBadReadPtr(reinterpret_cast<const void*>(stebp), 2 * sizeof(uintptr_t))) {
                break;
            }
            uintptr_t ret_addr = *reinterpret_cast<const uintptr_t*>(stebp + sizeof(uintptr_t));
            if (pos > 0 && static_cast<size_t>(pos) < sizeof(trace_buf) - 32) {
                int n = _snprintf(trace_buf + pos, sizeof(trace_buf) - 1 - static_cast<size_t>(pos),
                                  "%08Xh <= ", static_cast<unsigned int>(ret_addr));
                if (n > 0) {
                    pos += n;
                }
            }
            uintptr_t next_ebp = *reinterpret_cast<const uintptr_t*>(stebp);
            if (next_ebp <= stebp) {
                break;
            }
            stebp = next_ebp;
        }
    }

    log_format("BEGIN STACK TRACE: %sEND STACK TRACE\n", trace_buf);
}

void Traceback(const CONTEXT* orig_ctx) {
    if (orig_ctx == nullptr) {
        return;
    }

    CONTEXT ctx = *orig_ctx;
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    STACKFRAME64 frame = {};
    frame.AddrPC.Offset = ctx.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Esp;
    frame.AddrStack.Mode = AddrModeFlat;

    if (!SymInitialize(hProcess, nullptr, TRUE)) {
        log_format("[crash_filter] Failed to initialize symbol table\n");
    }

    log_format("=== Stack Trace Start ===\n");

    for (int i = 0; i < 64; ++i) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &frame, &ctx,
                         nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
            break;
        }

        DWORD64 addr = frame.AddrPC.Offset;
        if (i > 0 && addr == 0) {
            break;
        }

        BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(hProcess, addr, &displacement, symbol)) {
            log_format("  [0x%08llx] %s + 0x%llx\n",
                       static_cast<unsigned long long>(addr),
                       symbol->Name,
                       static_cast<unsigned long long>(displacement));
        } else {
            log_format("  [0x%08llx] (no symbol)\n",
                       static_cast<unsigned long long>(addr));
        }
    }

    SymCleanup(hProcess);
    log_format("=== Stack Trace End ===\n");
}

__declspec(noinline) void EmergencySaveAllCharacters() {
    if (g_Server == nullptr || g_PlayersList == nullptr) {
        return;
    }

    for (POSITION pos = g_PlayersList->list.GetHeadPosition(); pos != nullptr;) {
        Player* player = g_PlayersList->list.GetNext(pos);
        if (player == nullptr || player->is_ai != 0 || player->main_unit == nullptr) {
            continue;
        }
        g_Server->SaveCharacter(player->main_unit);
    }
}

void EmergencySaveWithSEH() {
    if (Config::MapLoaded) {
        log_format("Trying to save characters...\n");
        __try {
            EmergencySaveAllCharacters();
            log_format("upd_all(): Characters saved.\n");
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            log_format("upd_all() failed.\n");
        }
    } else {
        log_format("Exception was caught, but there is no map. Not saving characters.\n");
    }
}

LONG WINAPI UnhandledCrashFilter(EXCEPTION_POINTERS* info) {
    if (Config::ExitingCleanly) {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    if (InterlockedCompareExchange(&g_in_crash_handler, 1, 0) != 0) {
        TerminateProcess(GetCurrentProcess(), 1);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    __try {
        WriteMinidump(info);

        if (info != nullptr && info->ContextRecord != nullptr && info->ExceptionRecord != nullptr) {
            log_format("EXCEPTION DUMP:\neax=%08Xh,ebx=%08Xh,ecx=%08Xh,edx=%08Xh,\n"
                       "esp=%08Xh,ebp=%08Xh,esi=%08Xh,edi=%08Xh;\neip=%08Xh;\n"
                       "addr=%08Xh,code=%08Xh,flags=%08Xh\n",
                       static_cast<unsigned int>(info->ContextRecord->Eax),
                       static_cast<unsigned int>(info->ContextRecord->Ebx),
                       static_cast<unsigned int>(info->ContextRecord->Ecx),
                       static_cast<unsigned int>(info->ContextRecord->Edx),
                       static_cast<unsigned int>(info->ContextRecord->Esp),
                       static_cast<unsigned int>(info->ContextRecord->Ebp),
                       static_cast<unsigned int>(info->ContextRecord->Esi),
                       static_cast<unsigned int>(info->ContextRecord->Edi),
                       static_cast<unsigned int>(info->ContextRecord->Eip),
                       static_cast<unsigned int>(reinterpret_cast<uintptr_t>(info->ExceptionRecord->ExceptionAddress)),
                       static_cast<unsigned int>(info->ExceptionRecord->ExceptionCode),
                       static_cast<unsigned int>(info->ExceptionRecord->ExceptionFlags));

            PrintStackTrace(static_cast<uintptr_t>(info->ContextRecord->Ebp));
            Traceback(info->ContextRecord);
        }

        EmergencySaveWithSEH();
        ExitProcess(1);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        TerminateProcess(GetCurrentProcess(), 1);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace

void SetExceptionFilter() {
    SetUnhandledExceptionFilter(&UnhandledCrashFilter);
}

} // namespace tangar_crash
