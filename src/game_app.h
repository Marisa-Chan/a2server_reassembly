#ifndef GAME_APP_H
#define GAME_APP_H

#include "asm_mfc.h"
#include "table.h"

class GameApp : public CWinApp
{
public:
	static GameApp theApp;
public:

	virtual BOOL InitInstance() override;
	virtual BOOL OnIdle(LONG lCount) override;
	virtual int ExitInstance() override;
};


/* in asm */
void LogMessage(CString message); //43AA23


extern GameDataRes g_GameDataRes; //6d0668

#endif