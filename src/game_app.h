#ifndef GAME_APP_H
#define GAME_APP_H

#include "asm_mfc.h"

class GameApp : public CWinApp
{
public:
	static GameApp theApp;
public:

	virtual BOOL InitInstance() override;
	virtual BOOL OnIdle(LONG lCount) override;
	virtual int ExitInstance() override;
};




#endif