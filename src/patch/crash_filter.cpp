// Ported from srvmgr/crash_filter.cpp and srvmgr.cpp (DllMain crash save),
// extended with timestamped MiniDumpWriteDump (.dmp) output via dbghelp.

#include "crash_filter.h"

#include "config_new.h"
#include "netlog.h"
#include "tangar_log.h"

#include "../game_app.h"
#include "../player.h"
#include "../players_list.h"
#include "../server.h"
#include "../unit.h"

#include <windows.h>
#include <dbghelp.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace tangar_crash {

namespace {

volatile LONG g_in_crash_handler = 0;

// Dump directory prefix (<exe dir>\..\logs\) and server number for the file name.
// Computed once in SetExceptionFilter(). If the exe path can't be parsed,
// the prefix stays empty and dumps go to the current directory.
char g_dump_prefix[MAX_PATH] = {};
char g_server_tag[16] = "0";

void InitDumpLocation() {
    char exe_path[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH - 1);

    char* slash = strrchr(exe_path, '\\');
    if (slash == nullptr) {
        return;
    }
    *slash = '\0';                          // exe_path = ...\Server2

    slash = strrchr(exe_path, '\\');        // slash before "Server2"
    if (slash == nullptr) {
        return;
    }

    // Server number = trailing digits of the folder name ("Server2" -> "2")
    const char* folder = slash + 1;
    const char* end = folder + strlen(folder);
    const char* p = end;
    while (p > folder && p[-1] >= '0' && p[-1] <= '9') {
        --p;
    }
    if (p != end && (end - p) < static_cast<ptrdiff_t>(sizeof(g_server_tag))) {
        strcpy(g_server_tag, p);
    }

    *slash = '\0';                          // exe_path = <parent of exe dir>
    _snprintf(g_dump_prefix, sizeof(g_dump_prefix) - 2, "%s\\logs", exe_path);
    CreateDirectoryA(g_dump_prefix, nullptr);   // fine if it already exists
    strcat(g_dump_prefix, "\\");
}

void WriteMinidump(EXCEPTION_POINTERS* info) {
    SYSTEMTIME tm = {};
    GetLocalTime(&tm);

    char dmp_path[MAX_PATH] = {};
    _snprintf(dmp_path, sizeof(dmp_path) - 1,
              "%ssrvmgr%s_crash_%04u%02u%02u_%02u%02u%02u.dmp",
              g_dump_prefix, g_server_tag,
              static_cast<unsigned>(tm.wYear),
              static_cast<unsigned>(tm.wMonth),
              static_cast<unsigned>(tm.wDay),
              static_cast<unsigned>(tm.wHour),
              static_cast<unsigned>(tm.wMinute),
              static_cast<unsigned>(tm.wSecond));

    HANDLE hFile = CreateFileA(dmp_path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                               CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS) {
        for (unsigned seq = 1; seq < 100 && hFile == INVALID_HANDLE_VALUE; ++seq) {
            _snprintf(dmp_path, sizeof(dmp_path) - 1,
                      "%ssrvmgr%s_crash_%04u%02u%02u_%02u%02u%02u_%02u.dmp",
                      g_dump_prefix, g_server_tag,
                      static_cast<unsigned>(tm.wYear),
                      static_cast<unsigned>(tm.wMonth),
                      static_cast<unsigned>(tm.wDay),
                      static_cast<unsigned>(tm.wHour),
                      static_cast<unsigned>(tm.wMinute),
                      static_cast<unsigned>(tm.wSecond),
                      seq);
            hFile = CreateFileA(dmp_path, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS) {
                break;
            }
        }
    }
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
            netlog::logf("CRASH", "  [0x%08llx] %s + 0x%llx",
                         static_cast<unsigned long long>(addr), symbol->Name,
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
            netlog::logf("CRASH", "code=%08X eip=%08X addr=%08X (dump written, see .dmp)",
                         static_cast<unsigned int>(info->ExceptionRecord->ExceptionCode),
                         static_cast<unsigned int>(info->ContextRecord->Eip),
                         static_cast<unsigned int>(reinterpret_cast<uintptr_t>(info->ExceptionRecord->ExceptionAddress)));

            PrintStackTrace(static_cast<uintptr_t>(info->ContextRecord->Ebp));
            Traceback(info->ContextRecord);
        }

        if (g_IsServer != 0) {
            EmergencySaveWithSEH();
        }
        ExitProcess(1);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        TerminateProcess(GetCurrentProcess(), 1);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace

namespace {

// First-chance hook: log the stack of C++ exceptions (0xE06D7363) at throw
// time, before any handler unwinds the stack. This is what makes throw sites
// of MFC "improper argument" boxes visible. Rate-limited and guarded so it
// can never deadlock on dbghelp.
volatile LONG g_in_first_chance = 0;

LONG WINAPI FirstChanceHandler(EXCEPTION_POINTERS* info) {
    if (info == nullptr || info->ExceptionRecord == nullptr) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (info->ExceptionRecord->ExceptionCode != 0xE06D7363) { // C++ EH
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (InterlockedCompareExchange(&g_in_first_chance, 1, 0) != 0) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    __try {
        log_format("[crash_filter] C++ exception at throw time:\n");
        netlog::logf("CRASH", "C++ exception at throw time:");
        PrintStackTrace(static_cast<uintptr_t>(info->ContextRecord->Ebp));
        Traceback(info->ContextRecord);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    InterlockedExchange(&g_in_first_chance, 0);
    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace

void WriteDiagnostic(const char* reason) {
    CONTEXT ctx = {};
    RtlCaptureContext(&ctx);

    log_format("[crash_filter] Diagnostic (%s):\n", reason != nullptr ? reason : "");
    netlog::logf("CRASH", "diagnostic: %s", reason != nullptr ? reason : "");

    WriteMinidump(nullptr);
    Traceback(&ctx);
}

void SetExceptionFilter() {
    InitDumpLocation();
    SetUnhandledExceptionFilter(&UnhandledCrashFilter);
    AddVectoredExceptionHandler(1, &FirstChanceHandler);
}

} // namespace tangar_crash