#pragma once
#include "errortype.h"
#include "gametimer.h"
#define NULL 0
class Game
{
private:
	static Game* pInst;
	Game();
	~Game();						
	Game(const Game& other);		// Disabled
	enum GameState{RUNNING, PAUSED, MINIMISED};
	GameState m_State;
	GameTimer m_timer;
public:
	static Game* GetInstance();		// Thread-safe singleton
	static void Release();
	ErrorType Start();
	ErrorType InitialiseScript();
	ErrorType Update();
	ErrorType RunInterface();		// Love the identifier. You just know this is going to have cohesion problems
	ErrorType End();
};