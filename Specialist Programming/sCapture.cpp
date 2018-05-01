#include "sCapture.h"
#include "dynamicObjects.h"

// static instance
Capture* Capture::m_pInst = nullptr;

Capture * Capture::GetInstance()
{
	// If instance hasn
	if (!m_pInst)
		m_pInst = new Capture;

	return m_pInst;
}

void Capture::Enter(Bot * pBot)
{
	// Sets target domination point and sets behaviour for bot
	SetTarget(pBot);
	pBot->m_behaviour->SetBehaviours(0, 0, 0, 0, 0, 1, 1);
}

void Capture::Update(Bot* pBot)
{ 
	CheckDistance(pBot);
	CheckForEnemy(pBot);
	CheckDPOwner(pBot);

	pBot->SetAcceleration(pBot->m_behaviour->AccumilateBehaviours(pBot->GetLocation(), pBot->GetVelocity(), 
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location, Vector2D(0, 0)));

} // Execute()

void Capture::Exit(Bot * pBot)
{
}

void Capture::SetTarget(Bot * pBot)
{
	// Gives the bot a target
	pBot->m_behaviour->SetDominationID(pBot->GetBotID() % 3);

	pBot->m_behaviour->SetPath(DynamicObjects::GetInstance()->
		GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location, pBot->GetLocation());
}

void Capture::CheckDistance(Bot* pBot)
{ 

	// Clears the path if target is in sight
	if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location))
	{
		pBot->m_behaviour->SetBehaviours(1, 0, 0, 0, 0, 1, 0);
		// Clear path
		while(!pBot->m_behaviour->GetPath()->empty())
			pBot->m_behaviour->GetPath()->pop();
	}

} 


void Capture::CheckForEnemy(Bot* pBot)
{ 

	// Loop through all enemy bots on the next team
	for (int i = 0; i < NUMBOTSPERTEAM; i++)
	{

		if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
			DynamicObjects::GetInstance()->GetBot(1, i).GetLocation()) &&
			(pBot->GetLocation() - DynamicObjects::GetInstance()->GetBot(1, i).
				GetLocation()).magnitude() <= 400 && DynamicObjects::GetInstance()->
			GetBot(1, i).IsAlive())
		{
			//pBot->ChangeState(Attack::GetInstance());
		}
	}

} 


void Capture::CheckDPOwner(Bot* pBot)
{ 

	if (DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID())
		.m_OwnerTeamNumber == pBot->GetTeamID())
	{
		//pBot->ChangeState(Defend::GetInstance());
	}

} 
