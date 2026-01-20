#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <corecrt_memory.h>
#include <inttypes.h>

/*int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{

	return 0;
}*/

typedef void(__cdecl* a2_PVFV)(void);

extern "C"
{
	int	WINAPI a2WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd );

	a2_PVFV a2_LegacyInitArray[];
	int       a2_argc;
	const char** a2_argv;
	uint8_t a2_mbctype[257];
	void* a2_pioinfo[64];
	FILE* a2_stdout;
	FILE* a2_stderr;

	int a2LegacyInit()
	{
		printf("Start a2LegacyInit\n");
		a2_PVFV *func = a2_LegacyInitArray;
		while (*func != nullptr)
		{
			(*func)();
			func++;
		}
		printf("end a2LegacyInit\n");
		return 0;
	}
}


int main(int argc, const char* argv[])
{
	memset(a2_pioinfo, 0, sizeof(a2_pioinfo));
	memset(a2_mbctype, 0, sizeof(a2_mbctype));
	a2_stdout = stdout;
	a2_stderr = stderr;
	a2_argc = argc;
	a2_argv = argv;

	printf("A2 Reassembly main func call A2 WinMain\n");

	HINSTANCE hInstance = GetModuleHandle(NULL);

	LPSTR lpCmdLine = GetCommandLineA();

	STARTUPINFO si = { 0 };

	GetStartupInfo(&si);

	int nCmdShow = si.wShowWindow;

	int exitCode = a2WinMain(hInstance, NULL, lpCmdLine, si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
	
	return exitCode;
}