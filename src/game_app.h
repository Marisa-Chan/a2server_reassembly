#ifndef GAME_APP_H
#define GAME_APP_H

#include "asm_mfc.h"


struct GameDataRes;
class PlayersList;
struct Server;
class NetStru1;




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
extern PlayersList* g_PlayersList; //6cdb24
extern Server* g_Server;   //642c2c
extern NetStru1 g_NetStru1_main;  //6c3a08

#endif