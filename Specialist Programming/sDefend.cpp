#include "dynamicObjects.h"
#include "staticmap.h"
#include "sDefend.h"
#include "sAttack.h"
#include "sCapture.h"

Defend* Defend::m_pInst = nullptr;

Defend* Defend::GetInstance()
{
	if (!m_pInst)
		m_pInst = new Defend;

	return m_pInst;
}

void Defend::Enter(Bot * pBot)
{
	// Sets domination point to defend
	pBot->m_behaviour->SetBehaviours(0, 0, 0, 0, 0, 1);
	SetTarget(pBot);


}

void Defend::Update(Bot * pBot)
{
	// First check distance
	CheckDistance(pBot);
	// Check if enemy is close
	CheckForEnemies(pBot);

	// Check if dom point is owed by bot still
	if (!CheckIfDPIsOwn(pBot))
		pBot->ChangeState(Capture::GetInstance());
	// AccumilateBehaviours
	pBot->SetAcceleration(pBot->m_behaviour->AccumilateBehaviours(pBot->GetLocation(), pBot->GetVelocity(),
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location, Vector2D(0, 0)));

}

void Defend::Exit(Bot * pBot)
{
}

void Defend::SetTarget(Bot * pBot)
{
	// Set the target
	// Gives the bot a target
	pBot->m_behaviour->SetDominationID(pBot->GetBotID() % 3);

	if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location))
	{
		pBot->m_behaviour->m_currentPathTarget =
			DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location;
	}
	else
	{
		pBot->m_behaviour->SetPath(DynamicObjects::GetInstance()->
			GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location, pBot->GetLocation());
	}
}

void Defend::CheckDistance(Bot * pBot)
{
	// Clears the path if target is in sight
	if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location))
	{
		if(((pBot->GetLocation() - 
			DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location)).magnitude() < 100)
			pBot->m_behaviour->SetBehaviours(0, 1, 0, 0, 0, 1);
		else
			pBot->m_behaviour->SetBehaviours(1, 0, 0, 0, 0, 1);


		// Clear path
		while (!pBot->m_behaviour->GetPath()->empty())
			pBot->m_behaviour->GetPath()->pop();
	}
}

void Defend::CheckForEnemies(Bot * pBot)
{
	// Check every enemy on the other team
	for (int i = 0; i < NUMBOTSPERTEAM; i++)
	{
		// First check for line of sight with another bot and if close
		if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
			DynamicObjects::GetInstance()->GetBot(1, i).GetLocation()))
		{
			if ((pBot->GetLocation() - DynamicObjects::GetInstance()->GetBot(1, i).
				GetLocation()).magnitude() < 400 && DynamicObjects::GetInstance()->GetBot(1, i).IsAlive())
			{
				pBot->ChangeState(Attack::GetInstance());
			}
		}
	}

}

bool Defend::CheckIfDPIsOwn(Bot * pBot)
{
	// Return true or false if DP is owned
	return (pBot->GetTeamID() == DynamicObjects::GetInstance()
		->GetDominationPoint(pBot->m_behaviour->m_dominationTarget).m_OwnerTeamNumber);
}

wchar_t * Defend::GetName()
{
	return L"DEFEND";
}
