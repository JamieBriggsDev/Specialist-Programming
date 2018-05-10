#pragma once

#include "State.h"
#include "bot.h"

class Defend :
	public State<Bot>
{
private:
private:
	static Defend* m_pInst;
	Defend() : State(StateNames::DEFEND) {}
public:
	// Gets an instance to the Defend class, and if an instance hasn't
	//  been made yet, create a new one
	static Defend* GetInstance();

	// Enters the Defend state. Sets domination Target by:
	// - Finds a domination point
	// - Finds a path to the domination point
	// - Sets behaviours for the state
	void Enter(Bot* pBot);

	// Does three things::
	// - Checks if close to domination point
	// - Check if any enemy bots are aroundf
  // - Checks if the domination point is still owned by the bots team
	void Update(Bot* pBot);

	// Does no clean up as it does not have to
	void Exit(Bot* pBot);

	// Set Target will:
	// - Finds a domination point
	// - Finds a path to the domination point
	void SetTarget(Bot* pBot);

	// Checks distance from bot to target
	void CheckDistance(Bot* pBot);

	// Checks if an enemy is close
	void CheckForEnemies(Bot* pBot);

	// Check who owns domination point
	bool CheckIfDPIsOwn(Bot* pBot);

	wchar_t * GetName();
	
};

