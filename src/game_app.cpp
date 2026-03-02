#include "game_app.h"
#include "server.h"
#include "main_window.h"
#include "resource.h"
#include "file.h"
#include "table.h"

GameApp GameApp::theApp;

ServerConfig g_ServerConfig;

char g_CdSpeed[256];
char g_resolution[256];
char g_InitialWorkingDir[1024];
char g_WorkingDir[1024];

uint32_t g_CmdLatency;
uint32_t g_CmdTimeout = 15000;
uint32_t g_CmdSafeVideo = 0;

HWND g_MainWndHWND;
uint32_t g_IsCdPresent;
uint32_t g_EnableMusic;
uint32_t g_EnableTrace;

uint32_t g_DWORD_00659d18;

uint32_t g_MessageColors;

CString g_HelpStr;

TxtFile txt_main;
TxtFile txt_heropicture;
TxtFile txt_stats;
TxtFile txt_spells;
TxtFile txt_spell;
TxtFile txt_dialogs;
TxtFile txt_unitname;
TxtFile txt_building;
TxtFile txt_itemname;
TxtFile txt_npcnames;
TxtFile txt_cutscene;
TxtFile txt_cutpaths;
TxtFile txt_tunes;
TxtFile txt_patch;


HANDLE g_AutoRunEvent;
HMODULE g_scenario_dll;


void ReadFileToString(const char* fname, CString* str)
{
	//4758cd

	File2 f;
	f.Open(fname, 0);

	uint32_t len = f.GetLength();
	char* buf = (char *)malloc(len + 1);

	f.Read(buf, len);
	f.Close();

	buf[len] = 0;
	CharToOemA(buf, buf);

	*str = buf;

	free(buf);
}

int32_t GetRandS16(int32_t range)
{
	//4a4720
	return ((rand() * range) / 32767) % range;
}




BOOL GameApp::InitInstance()
{
	//4823bd
#ifdef A2CLIENT
	g_IsServer = 0;
#else
	g_IsServer = 1;
#endif

	const char* stmp = strstr(m_lpCmdLine, "-latency");
	if (stmp)
	{
		g_CmdLatency = atoi(stmp + 8);
		if (g_CmdLatency && (g_CmdLatency < 50 || g_CmdLatency > 10000))
			g_CmdLatency = 0;
	}

	stmp = strstr(m_lpCmdLine, "-timeout");
	if (stmp)
	{
		g_CmdTimeout = atoi(stmp + 8);
		if (g_CmdTimeout == 0 || g_CmdTimeout > 120)
			g_CmdTimeout = 15;

		g_CmdTimeout *= 1000;
	}

	HKEY regkey;
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\1C\\Allods 2", &regkey) != ERROR_SUCCESS)
	{
		char buf[1024];
		sprintf(buf, "%s\\%s", "SOFTWARE", "1C");
		RegCreateKeyA(HKEY_LOCAL_MACHINE, buf, &regkey);
		strcat(buf, "\\Allods2");
		RegCreateKeyA(HKEY_LOCAL_MACHINE, buf, &regkey);
	}
	RegCloseKey(regkey);

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\1C\\Allods 2", 0, KEY_READ, &regkey);
	
	DWORD bufsz = 255;
	if (RegQueryValueExA(regkey, "CDSPEED", NULL, NULL, (uint8_t*)g_CdSpeed, &bufsz) != ERROR_SUCCESS)
		strcpy(g_CdSpeed, "-4x");

	bufsz = 255;
	if (RegQueryValueExA(regkey, "RESOLUTION", NULL, NULL, (uint8_t*)g_resolution, &bufsz) != ERROR_SUCCESS)
		strcpy(g_resolution, "-640");

	GetCurrentDirectoryA(sizeof(g_InitialWorkingDir), g_InitialWorkingDir);

	char pathbuf[256];
	bufsz = 255;
	if (RegQueryValueExA(regkey, "INSTALLDIR", NULL, NULL, (uint8_t*)pathbuf, &bufsz) == ERROR_SUCCESS)
		SetCurrentDirectoryA(pathbuf);

	MainWindow* mwnd = new MainWindow();

	m_pMainWnd = mwnd;

	mwnd->ShowWindow(m_nCmdShow);

	g_MainWndHWND = mwnd->m_hWnd;

	if (g_IsServer == 0)
		SetCursor(NULL);
	else
		SetCursor(mwnd->cursor_arrow);

	GetCurrentDirectoryA(sizeof(g_WorkingDir), g_WorkingDir);

	char path[260];
	GetTempPathA(256, path);

	path[strlen(path) - 1] = 0;

	Resources::StaticAddPath(path);

	g_IsCdPresent = 1;

	bufsz = 255;
	if (RegQueryValueExA(regkey, "CD", NULL, NULL, (uint8_t*)path, &bufsz) == ERROR_SUCCESS)
	{
		strcat(path, "\\");

		DWORD a, b, c;
		GetVolumeInformationA(path, pathbuf, 255, &a, &b, &c, NULL, 0);

		if (strcmp(pathbuf, "ROM2") != 0)
			g_IsCdPresent = 0;
		
		strcat(path, "Allods");

		Resources::StaticAddPath(path);
	}
	else
	{
		g_IsCdPresent = 0;
	}

	RegCloseKey(regkey);

	CTime tm = CTime::GetCurrentTime();

	Resources::StaticAddResFile("graphics.res");
	Resources::StaticAddResFile("main.res");
	Resources::StaticAddResFile("patch.res");
	Resources::StaticAddResFile("world_srv.res");
	Resources::StaticAddResFile("video.res");

	if (strstr(m_lpCmdLine, "-nomusic"))
		g_EnableMusic = 0;

	if (strstr(m_lpCmdLine, "-trace"))
		g_EnableTrace = 1;

	g_CLlDriver.field_0x808 = 0;

	Resources::StaticAddResFile("sfx.res");
	Resources::StaticAddResFile("movies.res");
	Resources::StaticAddResFile("scenario.res");
	Resources::StaticAddResFile("speech.res");

	Resources::FUN_004e632d("update.lst");

	mwnd->UpdateClipCursor();

	FUN_00475e7a();

	mwnd->field_0x5e8.FUN_004ac3af();
	mwnd->field_0x5e8.FUN_00497270(1);
	mwnd->field_0x5e8.FUN_004ac566(1);

	g_DWORD_00659d18 = 0;

	if (strstr(m_lpCmdLine, "-safevideo"))
		g_CmdSafeVideo = 1;

	stmp = strstr(m_lpCmdLine, "-cfg\"");
	if (stmp)
	{
		const char* qend = strchr(stmp + 5, '\"');
		if (qend)
		{
			CString confname(stmp + 5, qend - (stmp + 5));
			int32_t ierr = ParseConfig( confname );
			if (ierr != 0)
			{
				CString msg;
				if (ierr < 1)
					msg = "Error loading " + confname + ".";
				else
					msg.Format("Error in %s. Line %d.", confname, ierr);

				AfxMessageBox(msg);
			}
		}
	}
	else
	{
		int32_t ierr = ParseConfig("server.cfg");
		if (ierr > 0)
		{
			CString msg;
			msg.Format("Error in server.cfg. Line %d.", ierr);

			AfxMessageBox(msg);
		}
	}

#ifndef A2CLIENT
	if (g_ServerConfig.chr_base.IsEmpty())
	{
		AfxMessageBox("Missing \'ChrBase\' parameter in config file.");
		exit(1);
	}

	CString ffdir = g_ServerConfig.chr_base.Left(g_ServerConfig.chr_base.GetLength() - 1);

	CFileFind fnd;
	if (fnd.FindFile(ffdir))
	{
		fnd.FindNextFileA();

		if (!fnd.IsDirectory())
		{
			CString msg = "Invalid ChrBase directory \"" + ffdir + "\"";
			AfxMessageBox(msg);
			exit(1);
		}
	}
	else
	{
		if (!CreateDirectoryA(ffdir, NULL))
		{
			CString msg = "ChrBase directory \"" + ffdir + "\" does not exist and can't be created";
			AfxMessageBox(msg);
			exit(1);
		}
	}
	fnd.Close();
#endif

	if (!InitVideo())
	{
		AfxMessageBox("Unable to initialize video");
		exit(1);
	}

	if (strstr(m_lpCmdLine, "-window") == NULL && !SetVideoMode())
	{
		AfxMessageBox("Unable to set video mode");
		exit(1);
	}

	ReadFileToString("main\\text\\help.txt", &g_HelpStr);

	txt_main.LoadChunkFile("main\\text\\main.txt");
	txt_heropicture.LoadChunkFile("main\\text\\heropicture.txt");
	txt_stats.LoadChunkFile("main\\text\\stats.txt");
	txt_spells.LoadChunkFile("main\\text\\spells.txt");
	txt_spell.LoadChunkFile("main\\text\\spell.txt");
	txt_dialogs.LoadChunkFile("main\\text\\dialogs.txt");
	txt_unitname.LoadChunkFile("main\\text\\unitname.txt");
	txt_building.LoadChunkFile("main\\text\\building.txt");
	txt_itemname.LoadChunkFile("main\\text\\itemname.txt");
	txt_npcnames.LoadChunkFile("main\\text\\npcnames.txt");
	txt_cutscene.LoadChunkFile("main\\text\\cutscene.txt");
	txt_cutpaths.LoadChunkFile("main\\text\\cutpaths.txt");
	txt_tunes.LoadChunkFile("main\\text\\tunes.txt");
	txt_patch.LoadChunkFile("patch\\patch.txt");

	ParseTunes();

	CFile ffame;
	if (!ffame.Open("famehall.dat", 0))
	{
		mwnd->field_0x5e8.FUN_004acafa();
	}
	else
	{
		if (!ffame.GetLength())
			mwnd->field_0x5e8.FUN_004acafa();
		else
			mwnd->field_0x5e8.FUN_004ac945(&ffame);

		ffame.Close();
	}

	char wintext[256];
	OemToCharA(TxtFile::AllLines[0x96], wintext);
	mwnd->SetWindowText(wintext);

	g_GameDataRes.ParseWorldIn("World\\Data\\");

	LoadItemNames();
	LoadFonts();
	mwnd->CreateUI();
	mwnd->field_0x460 = 1;
	mwnd->LoadData();
	mwnd->PostMessageA(0x421, 0, 0);

	g_AutoRunEvent = CreateEventA(NULL, TRUE, TRUE, "Allods 2 AutoRun");
	g_scenario_dll = LoadLibraryA("scenario.dll");

	if (g_scenario_dll == NULL)
	{
		AfxMessageBox("Can\'t find scenario.dll");
		return FALSE;
	}

	ScenarioGetVar = GetProcAddress(g_scenario_dll, (LPCSTR)1);
	ScenarioSetVar = GetProcAddress(g_scenario_dll, (LPCSTR)2);
	ScenarioTalkTo = GetProcAddress(g_scenario_dll, (LPCSTR)3);
	ScenarioEnterLocation = GetProcAddress(g_scenario_dll, (LPCSTR)5);
	ScenarioLeaveLocation = GetProcAddress(g_scenario_dll, (LPCSTR)6);
	ScenarioEnterShop = GetProcAddress(g_scenario_dll, (LPCSTR)7);
	ScenarioLeaveShop = GetProcAddress(g_scenario_dll, (LPCSTR)8);
	ScenarioEnterInn = GetProcAddress(g_scenario_dll, (LPCSTR)9);
	ScenarioLeaveInn = GetProcAddress(g_scenario_dll, (LPCSTR)10);
	ScenarioNewGame = GetProcAddress(g_scenario_dll, (LPCSTR)11);
	ScenarioSave = GetProcAddress(g_scenario_dll, (LPCSTR)12);
	ScenarioLoad = GetProcAddress(g_scenario_dll, (LPCSTR)13);
	ScenarioGetAvailableLocations = GetProcAddress(g_scenario_dll, (LPCSTR)14);
	ScenarioGetShopAssortment = GetProcAddress(g_scenario_dll, (LPCSTR)15);
	ScenarioIsTownAvailable = GetProcAddress(g_scenario_dll, (LPCSTR)16);
	ScenarioIsMissionAvailable = GetProcAddress(g_scenario_dll, (LPCSTR)17);
	ScenarioGetCurrentLocation = GetProcAddress(g_scenario_dll, (LPCSTR)18);
	ScenarioGetAllLocations = GetProcAddress(g_scenario_dll, (LPCSTR)19);

	mwnd->field_0x45c = 0;

	PrecomputeExperienceLevelsTable();
	SetMessageColors(g_MessageColors);

	return TRUE;
}