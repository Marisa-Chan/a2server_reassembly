#include "game_app.h"
#include "server.h"
#include "main_window.h"
#include "resource.h"
#include "file.h"
#include "table.h"
#include "item.h"

GameApp GameApp::theApp;

ServerConfig g_ServerConfig;

char g_CdSpeed[256]; //6658f8
char g_resolution[256]; //665bf0
char g_InitialWorkingDir[1024]; //6654c0
char g_WorkingDir[1024]; //65fbc8

GameSettings g_settings; //660ec0

uint32_t g_CmdLatency;
uint32_t g_CmdTimeout = 15000;
uint32_t g_CmdSafeVideo = 0;

HWND g_MainWndHWND;
uint32_t g_IsCdPresent;
uint32_t g_EnableMusic;
uint32_t g_EnableTrace;
uint32_t INT_00660f74; //660f74

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


// 43A857
void __cdecl sub_43A857(const char* source) {
	MainWindow* mainWnd = (MainWindow*)AfxGetMainWnd();
	if (!mainWnd || mainWnd->field_0x640 != 3) {
		return;
	}

	// Allocate a copy of source and post it to the main window (receiver frees).
	size_t len = strlen(source) + 1;
	char* buf = (char*)malloc(len);
	strcpy(buf, source);
	PostMessageA(g_MainWndHWND, 0x472, (WPARAM)buf, 0);

	// Append to log file if configured.
	if (g_ServerConfig.log_file.IsEmpty()) {
		return;
	}

	CStdioFile file;
	if (!file.Open(g_ServerConfig.log_file, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate)) {
		return;
	}
	file.SeekToEnd();

	CTime now = CTime::GetCurrentTime();
	CString timestamp = now.Format("%d.%m.%y %H:%M:%S ");
	file.WriteString(timestamp + source + "\n");
}

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

int32_t ParseConfig(const char* fname) // 4f7188
{
	CStdioFile file;
	if (!file.Open(fname, 0, nullptr)) {
		return -1;
	}

	CString section_name;
	int32_t line_num = 1;

	while (file.GetPosition() < file.GetLength()) {
		CString original_line;
		file.ReadString(original_line);

		CString lowered(original_line);
		lowered.MakeLower();

		int semicolon_pos = lowered.Find(';');
		int eq_pos = lowered.Find('=');

		if (semicolon_pos >= 0 && eq_pos >= semicolon_pos) {
			eq_pos = -1;
		}

		if (semicolon_pos != -1) {
			lowered = lowered.Left(semicolon_pos);
		}


		if (lowered.IsEmpty())
		{
			line_num++;
			continue;
		}

		if (lowered[0] == '[') {
			// Section header
			lowered = lowered.Mid(1);
			lowered.TrimLeft();
			int close_bracket = lowered.Find(']');
			if (close_bracket == -1) {
				return line_num;
			}
			lowered = lowered.Left(close_bracket);
			lowered.TrimRight();
			section_name = lowered;
		} else if (section_name == "maps") {
			// Maps section
			lowered.TrimLeft();
			lowered.TrimRight();
			if (lowered.GetLength() == 0) {
				continue; // Empty maps lines don't increment line_num (matches original ASM)
			}
			if (eq_pos == -1) {
				g_ServerConfig.map_names.SetAtGrow(g_ServerConfig.map_names.GetSize(), lowered);
				g_ServerConfig.map_durations.SetAtGrow(g_ServerConfig.map_durations.GetSize(), 0x7FFFFFFF);
			} else {
				CString temp(lowered);
				lowered = lowered.Left(eq_pos);
				lowered.TrimRight();
				temp = temp.Mid(eq_pos + 1);
				temp.TrimLeft();
				double d = atof(temp);
				g_ServerConfig.map_names.SetAtGrow(g_ServerConfig.map_names.GetSize(), lowered);
				g_ServerConfig.map_durations.SetAtGrow(g_ServerConfig.map_durations.GetSize(), d * 60.0);
			}
		} else if (eq_pos != -1 && section_name != "bannedplayers") {
			// Key=value line (not in bannedplayers section)
			CString value_lowered = lowered.Mid(eq_pos + 1);
			value_lowered.TrimLeft();
			value_lowered.TrimRight();

			CString value_original = original_line.Mid(eq_pos + 1);
			value_original.TrimLeft();
			value_original.TrimRight();

			int int_value = atoi(value_lowered);

			if (lowered.Find("repopdelay") == 0 && section_name == "settings") {
				if (int_value < 20) { int_value = 20; }
				if (int_value > 500) { int_value = 500; }
				g_ServerConfig.repop_delay = int_value;
			} else if (lowered.Find("logintimeout") == 0 && section_name == "settings") {
				if (int_value < 10) { int_value = 10; }
				if (int_value > 300) { int_value = 300; }
				g_ServerConfig.login_timeout = int_value;
			} else if (lowered.Find("reconnectdelay") == 0 && section_name == "settings") {
				if (int_value < 1) { int_value = 1; }
				g_ServerConfig.reconnect_delay = int_value;
			} else if (lowered.Find("protocol") == 0 && section_name == "settings") {
				// Protocol setting is read but ignored.
			} else if (lowered.Find("gametype") == 0 && section_name == "settings") {
				if (value_lowered == "cooperative") {
					g_ServerConfig.gameType = 0;
				} else if (value_lowered == "deathmatch") {
					g_ServerConfig.gameType = 1;
				} else if (value_lowered == "teamplay") {
					g_ServerConfig.gameType = 2;
				} else if (value_lowered == "arena") {
					g_ServerConfig.gameType = 3;
				} else {
					return line_num;
				}
			} else if (lowered.Find("gamespeed") == 0 && section_name == "settings") {
				if (int_value < 0 || int_value > 8) { int_value = 4; }
				g_ServerConfig.game_speed = int_value;
			} else if (lowered.Find("logfile") == 0 && section_name == "settings") {
				g_ServerConfig.log_file = value_lowered;
			} else if (lowered.Find("chrbase") == 0 && section_name == "settings") {
				g_ServerConfig.chr_base = value_lowered;
				int len = g_ServerConfig.chr_base.GetLength();
				if (len > 0 && ((LPCTSTR)g_ServerConfig.chr_base)[len - 1] != '\\') {
					g_ServerConfig.chr_base += '\\';
				}
			} else if (lowered.Find("ipaddress2") == 0 && section_name == "settings") {
				g_ServerConfig.ip_address2 = value_lowered;
			} else if (lowered.Find("ipaddress") == 0 && section_name == "settings") {
				g_ServerConfig.ip_address = value_lowered;
			} else if (lowered.Find("hataddress") == 0 && section_name == "settings") {
				g_ServerConfig.hat_address = value_lowered;
			} else if (lowered.Find("description") == 0 && section_name == "settings") {
				g_ServerConfig.server_name.Empty();
				for (int i = 0; i < value_original.GetLength(); i++) {
					char c = value_original[i];
					if (c == '&' || c == '%' || c == '|' || c == '?' || (unsigned char)c >= 0x80) {
						g_ServerConfig.server_name += '*';
					} else {
						g_ServerConfig.server_name += c;
					}
				}
			} else if (lowered.Find("serverid") == 0 && section_name == "settings") {
				g_ServerConfig.server_id = int_value;
			} else if (lowered.Find("sayrange") == 0 && section_name == "settings") {
				if (int_value < 1 || int_value > 255) { int_value = 255; }
				g_ServerConfig.chat_range = int_value;
			} else if (lowered.Find("shoutdelay") == 0 && section_name == "settings") {
				if (int_value < 0) { int_value = 0; }
				g_ServerConfig.shout_delay = int_value;
			} else if (lowered.Find("shutdowndelay") == 0 && section_name == "settings") {
				if (int_value < 1 || int_value > 60) { int_value = 5; }
				g_ServerConfig.shutdown_delay = int_value;
			} else if (lowered.Find("maxplayers") == 0 && section_name == "settings") {
				if (int_value < 1 || int_value > 16) { int_value = 16; }
				g_ServerConfig.max_players = int_value;
			} else if (lowered.Find("fraglimit") == 0 && section_name == "settings") {
				if (int_value < 1) { int_value = 0x7FFFFFFF; }
				g_ServerConfig.frag_limit = int_value;
			} else if (lowered.Find("arenatimelimit") == 0 && section_name == "settings") {
				if (int_value < 1) { int_value = 0x7FFFFFFF; }
				g_ServerConfig.arena_time_limit = int_value;
			} else if (lowered.Find("flagscore") == 0 && section_name == "settings") {
				if (int_value < 1) { int_value = 1; }
				g_ServerConfig.flag_score = int_value;
			} else if (lowered.Find("save") == 0 && section_name == "settings") {
				if (value_lowered == "client") {
					g_ServerConfig.save_is_server = 0;
				} else if (value_lowered == "server") {
					g_ServerConfig.save_is_server = 1;
				} else {
					return line_num;
				}
			} else if (lowered.Find("scaledmaps") == 0 && section_name == "settings") {
				if (value_lowered == "on") {
					g_ServerConfig.map_range_check = 1;
				} else if (value_lowered == "off") {
					g_ServerConfig.map_range_check = 0;
				} else {
					return line_num;
				}
			} else if (lowered.Find("alwaysloadsacks") == 0 && section_name == "settings") {
				if (value_lowered == "on") {
					g_ServerConfig.always_load_sacks = 1;
				} else if (value_lowered == "off") {
					g_ServerConfig.always_load_sacks = 0;
				} else {
					return line_num;
				}
			} else if (lowered.Find("treasureprobability") == 0 && section_name == "settings") {
				if (int_value < 1) { int_value = 0; }
				if (int_value > 100) { int_value = 100; }
				g_ServerConfig.treasure_probability = int_value;
			} else {
				return line_num;
			}
		} else {
			// No '=' or section is "bannedplayers"
			if (section_name == "bannedips") {
				lowered.TrimLeft();
				lowered.TrimRight();
				g_ServerConfig.banned_ips.SetAtGrow(g_ServerConfig.banned_ips.GetSize(), lowered);
			} else if (section_name == "bannedplayers") {
				original_line.TrimLeft();
				original_line.TrimRight();
				g_ServerConfig.banned_names.SetAtGrow(g_ServerConfig.banned_names.GetSize(), original_line);
			} else if (section_name == "reporttowww") {
				lowered.TrimLeft();
				lowered.TrimRight();
				g_ServerConfig.report_to_www.SetAtGrow(g_ServerConfig.report_to_www.GetSize(), lowered);
			} else {
				lowered.TrimRight();
				if (lowered.GetLength() != 0) {
					return line_num;
				}
			}
		}

		line_num++;
	}

	return 0;
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

#ifdef A2CLIENT
	g_IsCdPresent = 1; // always has CD for client, patch
#endif

	RegCloseKey(regkey);

	CTime tm = CTime::GetCurrentTime();

	Resources::StaticAddResFile("graphics.res");
	Resources::StaticAddResFile("main.res");
	Resources::StaticAddResFile("patch.res");
#ifdef A2CLIENT
	Resources::StaticAddResFile("world.res");
#else
	Resources::StaticAddResFile("world_srv.res");
#endif
	Resources::StaticAddResFile("video.res");

	if (strstr(m_lpCmdLine, "-nomusic"))
		g_EnableMusic = 0;

	if (strstr(m_lpCmdLine, "-trace"))
		g_EnableTrace = 1;

	g_CLlDriver.keepalive = 0;

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
	mwnd->LoadSettings();
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

	ScenarioGetVar = (int32_t(__stdcall*)(int32_t))GetProcAddress(g_scenario_dll, (LPCSTR)1);
	ScenarioSetVar = (void(__stdcall*)(int32_t, int32_t))GetProcAddress(g_scenario_dll, (LPCSTR)2);
	ScenarioTalkTo = GetProcAddress(g_scenario_dll, (LPCSTR)3);
	ScenarioEnterLocation = GetProcAddress(g_scenario_dll, (LPCSTR)5);
	ScenarioLeaveLocation = (int32_t(__stdcall *)(int32_t*))GetProcAddress(g_scenario_dll, (LPCSTR)6);
	ScenarioEnterShop = GetProcAddress(g_scenario_dll, (LPCSTR)7);
	ScenarioLeaveShop = GetProcAddress(g_scenario_dll, (LPCSTR)8);
	ScenarioEnterInn = GetProcAddress(g_scenario_dll, (LPCSTR)9);
	ScenarioLeaveInn = GetProcAddress(g_scenario_dll, (LPCSTR)10);
	ScenarioNewGame = GetProcAddress(g_scenario_dll, (LPCSTR)11);
	ScenarioSave = (void(__stdcall*)(CFile*))GetProcAddress(g_scenario_dll, (LPCSTR)12);
	ScenarioLoad = (void(__stdcall*)(CFile*))GetProcAddress(g_scenario_dll, (LPCSTR)13);
	ScenarioGetAvailableLocations = GetProcAddress(g_scenario_dll, (LPCSTR)14);
	ScenarioGetShopAssortment = GetProcAddress(g_scenario_dll, (LPCSTR)15);
	ScenarioIsTownAvailable = GetProcAddress(g_scenario_dll, (LPCSTR)16);
	ScenarioIsMissionAvailable = GetProcAddress(g_scenario_dll, (LPCSTR)17);
	ScenarioGetCurrentLocation = (Location * (__stdcall*)())GetProcAddress(g_scenario_dll, (LPCSTR)18);
	ScenarioGetAllLocations = GetProcAddress(g_scenario_dll, (LPCSTR)19);

	mwnd->field_0x45c = 0;

	PrecomputeExperienceLevelsTable();
	SetMessageColors(g_MessageColors);

	return TRUE;
}



int FUN_00497490()
{ //497490
	return ScenarioGetVar(0x300) > 0x77;
}


void SrandInit()
{ //5421e9
	srand(timeGetTime());
}


int32_t __cdecl AppGetWorkingDir(int maxchar, char* buf)
{ //4761dd
	int ln = strlen(g_WorkingDir);
	if (ln > maxchar)
		ln = maxchar;

	strncpy(buf, g_WorkingDir, ln);
	buf[ln] = 0;
	return ln;
}

int __cdecl AppHasAnySaveFile()
{ // 440d96
	char buf[260];
	AppGetWorkingDir(260 - 1, buf);

	strcat(buf, "\\game*.sav");

	_WIN32_FIND_DATAA fnd;
	HANDLE hFindFile = FindFirstFileA(buf, &fnd);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return 0;

	FindClose(hFindFile);
	return 1;
}

int __cdecl SaveListCompare(const void* data1, const void* data2)
{ // 43ea5b
	return CompareFileTime(&((WIN32_FIND_DATAA*)data1)->ftLastWriteTime, &((WIN32_FIND_DATAA*)data2)->ftLastWriteTime);
}

void __cdecl AppFindSavesList(CArray<WIN32_FIND_DATAA>* list, int p)
{ //43eaa0
	char buf[260];
	AppGetWorkingDir(260 - 1, buf);

	strcat(buf, "\\game*.sav");
	_WIN32_FIND_DATAA fnd;
	HANDLE hFindFile = FindFirstFileA(buf, &fnd);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return;

	while (true)
	{
		if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			if (p == 0)
			{
				if (strcmp(fnd.cFileName, "game9999.sav") != 0 && strcmp(fnd.cFileName, "game9998.sav") != 0)
					list->Add(fnd);
			}
			else
				list->Add(fnd);
		}

		if (FindNextFileA(hFindFile, &fnd) == 0)
			break;
	}
	FindClose(hFindFile);

	
	if (list->GetSize() != 0)
		qsort(list->GetData(), list->GetSize(), sizeof(WIN32_FIND_DATAA), SaveListCompare);

}