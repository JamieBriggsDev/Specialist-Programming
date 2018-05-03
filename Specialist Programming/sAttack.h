#pragma once
#include "State.h"
#include "bot.h"

class Attack :
	public State<Bot>
{
public:
	static Attack* m_pInst;
	Attack() : State(StateNames::ATTACK) {};
	// Gets an instance to the Attack class, and if an instance hasn't
	//  been made yet
	static Attack* GetInstance();

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

wchar_t * GetName();

// Set Target will:
// - Search all bots and finds which ones are in sight
// - Finds which bot is closest (biggest threat)
void SetTarget(Bot* pBot);
};
