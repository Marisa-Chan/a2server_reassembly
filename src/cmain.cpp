#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <afx.h>
#include <afxwin.h>
#include <windows.h>
#include <corecrt_memory.h>
#include <inttypes.h>

#include <dbghelp.h> // must be included after windows.h
/*
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return AfxWinMain(hInst, hInstPrev, cmdline, cmdshow);
}*/

int       a2_argc;
const char** a2_argv;
uint8_t a2_mbctype[257];
void* a2_pioinfo[64];
FILE* a2_stdout;
FILE* a2_stderr;

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
        Traceback(info);

		printf("Press any key to exit...\n");
		_getch();

        ExitProcess(1);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) { /* empty */ }

    return EXCEPTION_EXECUTE_HANDLER;
}

void SetExceptionFilter() {
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&ExceptionHandler);
}

int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

int main(int argc, const char* argv[]) {
	memset(a2_pioinfo, 0, sizeof(a2_pioinfo));
	memset(a2_mbctype, 0, sizeof(a2_mbctype));
	a2_stdout = stdout;
	a2_stderr = stderr;
	a2_argc = argc;
	a2_argv = argv;

    SetExceptionFilter();

	HINSTANCE hInstance = GetModuleHandle(NULL);

	LPSTR lpCmdLine = GetCommandLineA();

	STARTUPINFO si = { 0 };

	GetStartupInfo(&si);

	int nCmdShow = si.wShowWindow;

	int exitCode = AfxWinMain(hInstance, NULL, lpCmdLine, si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
	
	return exitCode;
}
