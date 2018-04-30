// GameCode.cpp		Version 11			5/3/08	
// These three functions form the basis of a game loop in the window created in the
// wincode.cpp file

#include "gamecode.h"
#include "mysoundengine.h"
#include "mydrawengine.h"
#include "mysound.h"
#include "myinputs.h"
#include <time.h>
#include "gametimer.h"
#include "errorlogger.h"
#include <math.h>
#include "Game.h"

MySoundEngine* pTheSoundEngine;	// A pointer to the sound engine
MyInputs* pTheInputs;

extern HWND gHwnd;				// A handle to the main window application (created and declared in wincode.cpp).
extern HINSTANCE g_hinstance;	// A handle to the program instance

// Your global objects ***********************************************************************


// Your global functions *********************************************************************



// The game !!! *********************************************************************************

ErrorType GameInit(bool bFullScreen)
// Called once before entering game loop. 
// Use this function to set up the program
// gHwnd is the handle to the window and is required by the constructors for the engines
{
	// Create the engines - this should be done before creating other DDraw objects
	pTheSoundEngine = MySoundEngine::Start(gHwnd);
	pTheInputs = MyInputs::Start(g_hinstance, gHwnd);


	MyDrawEngine::Start(gHwnd, bFullScreen);



	if(Game::GetInstance()->Start()==FAILURE)
	{
		ErrorLogger::Writeln(L"Game failed to start");
		return FAILURE;
	}

	return (SUCCESS);
}

// ******************************************************************

#include "bot.h"
ErrorType GameMain()
// Called repeatedly - the game loop
{

	if(	Game::GetInstance()->Update() == FAILURE)
	{
		ErrorLogger::Writeln(L"Game update reports failure. Terminating program.");
		PostMessage(gHwnd,WM_CLOSE,0,0);
	}

	return SUCCESS;
}

// ***********************************************************

void GameShutdown()
// called after the game loop is finished
{
	if(Game::GetInstance()->End()==FAILURE)
	{
		ErrorLogger::Writeln(L"Game reports failure on end");
	}
	Game::Release();

	// (engines must be terminated last)
	MyDrawEngine::Terminate();
	MySoundEngine::Terminate();
	MyInputs::Terminate();
}



