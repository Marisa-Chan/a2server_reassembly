#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <afx.h>
#include <afxwin.h>
#include <windows.h>
#include <corecrt_memory.h>
#include <inttypes.h>

#include <dbghelp.h> // must be included after windows.h

int       a2_argc;
const char** a2_argv;
uint8_t a2_mbctype[257];
void* a2_pioinfo[64];
FILE* a2_stdout;
FILE* a2_stderr;

// Retail binaries are /SUBSYSTEM:WINDOWS: no terminal window next to the
// game/server window. Terminal output still works in three cases:
//   - launched from a console (cmd/PowerShell): we attach the parent console
//   - stdout/stderr redirected (> file, pipe): handles stay valid, untouched
//   - "-console" on the command line: force a console window (debugging)
static void RedirectStdioToConsole() {
    // CONERR$ is not a device name; console stderr shares CONOUT$.
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
}

static bool HasConsoleFlag() {
    const char* cli = GetCommandLineA();
    if (cli == nullptr) {
        return false;
    }
    const char* p = cli;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t' || *p == '"') {
            p++;
        }
        const char* tok = p;
        while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '"') {
            p++;
        }
        size_t len = static_cast<size_t>(p - tok);
        if ((len == 8 && strncmp(tok, "-console", 8) == 0) ||
            (len == 9 && strncmp(tok, "--console", 9) == 0)) {
            return true;
        }
    }
    return false;
}

static void InitGuiConsole() {
    if (HasConsoleFlag()) {
        if (GetConsoleWindow() == nullptr && !AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }
        if (GetConsoleWindow() != nullptr) {
            RedirectStdioToConsole();
        }
        return;
    }

    // Launched from a console without redirection: keep printf flowing to
    // the launching terminal. Explorer/double-click launches fail here
    // silently and stay console-free.
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == nullptr || out == INVALID_HANDLE_VALUE) {
        if (AttachConsole(ATTACH_PARENT_PROCESS)) {
            RedirectStdioToConsole();
        }
    }
}

// With the WINDOWS subsystem there is usually no console at crash time;
// pop one so the printf trace and the "press any key" wait still work.
static void EnsureCrashConsole() {
    if (GetConsoleWindow() != nullptr) {
        return;
    }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
    RedirectStdioToConsole();
}

void Traceback(EXCEPTION_POINTERS *info) {
    CONTEXT* ctx = info->ContextRecord;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    STACKFRAME64 frame = {};
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrStack.Mode = AddrModeFlat;

    DWORD machineType = IMAGE_FILE_MACHINE_I386;

    if (!SymInitialize(hProcess, NULL, TRUE)) {
        printf("[crash_filter] Failed to initialize symbol table\n");
    }

    char buffer[1024];
    
    printf("EXCEPTION DUMP:\neax=%08Xh,ebx=%08Xh,ecx=%08Xh,edx=%08Xh,\nesp=%08Xh,ebp=%08Xh,esi=%08Xh,edi=%08Xh;\neip=%08Xh;\naddr=%ph,code=%08Xh,flags=%08Xh\n",
            info->ContextRecord->Eax,
            info->ContextRecord->Ebx,
            info->ContextRecord->Ecx,
            info->ContextRecord->Edx,
            info->ContextRecord->Esp,
            info->ContextRecord->Ebp,
            info->ContextRecord->Esi,
            info->ContextRecord->Edi,
            info->ContextRecord->Eip,
            info->ExceptionRecord->ExceptionAddress,
            info->ExceptionRecord->ExceptionCode,
            info->ExceptionRecord->ExceptionFlags);

    printf("=== Stack Trace Start ===\n");

    for (int i = 0; i < 64; ++i) {
        if (!StackWalk64(machineType, hProcess, hThread, &frame, ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
            break;
        }

        DWORD64 addr = frame.AddrPC.Offset;
        if (i && addr == 0) {
            break;
        }

        BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;

        if (SymFromAddr(hProcess, addr, &displacement, symbol)) {
            printf("  [0x%08llx] %s + 0x%llx\n", addr, symbol->Name, displacement);
        } else {
            printf("  [0x%08llx] (no symbol)\n", addr);
        }
    }

    SymCleanup(hProcess);
    printf("=== Stack Trace End ===\n");
}

DWORD ExceptionHandler(EXCEPTION_POINTERS *info) {
    __try {
        EnsureCrashConsole();
        Traceback(info);

        printf("Press any key to exit...\n");
        fflush(stdout);
        _getch();

        fflush(stdout);
        fflush(stderr);
        ExitProcess(1);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) { /* empty */ }

    return EXCEPTION_EXECUTE_HANDLER;
}

void SetExceptionFilter() {
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&ExceptionHandler);
}

int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    memset(a2_pioinfo, 0, sizeof(a2_pioinfo));
    memset(a2_mbctype, 0, sizeof(a2_mbctype));
    a2_stdout = stdout;
    a2_stderr = stderr;
    a2_argc = __argc;
    a2_argv = (const char**)__argv;

    InitGuiConsole();
    SetExceptionFilter();

    return AfxWinMain(hInst, hInstPrev, cmdline, cmdshow);
}
