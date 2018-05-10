#pragma once

// This state gets a bot to go to a domination point and capture

#include "State.h"
#include "bot.h"

// The capture state 
class Capture : public State<Bot>
{
private:
	static Capture* m_pInst;
  Capture() : State(StateNames::CAPTURE) {}

public:
	// Gets an instance to the Capture class, and if an instance hasn't
	//  been made yet
	static Capture* GetInstance();

	// Enters the Capture state. Set Target is then called which does:
	// - Finds a domination point
	// - Finds a path to the domination point
	// - Sets behaviours for the state
	void Enter(Bot* pBot);

	// Checks how far away the bot is from the targeted domination point while
	// checking to see if there are any enemies around us to attack. Then checks
	// to see if the domination point hasn't already been captured by another
	// member of the team
	void Update(Bot* pBot);

	//Tidies up after the state has completed the activities
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
	void CheckDPOwner(Bot* pBot);

	wchar_t * GetName();

};