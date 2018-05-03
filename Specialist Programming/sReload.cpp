#include "sReload.h"
#include "sAttack.h"
#include "sCapture.h"
#include "dynamicObjects.h"
#include "bot.h"

// static instance
Reload* Reload::m_pInst = nullptr;

Reload * Reload::GetInstance()
{
	// If instance hasn
	if (!m_pInst)
		m_pInst = new Reload;

	return m_pInst;
}

void Reload::Enter(Bot * pBot)
{
	// Sets target domination point and sets behaviour for bot
	SetTarget(pBot);
	pBot->m_behaviour->SetBehaviours(0, 0, 0, 0, 0, 1);
}

void Reload::Update(Bot* pBot)
{ 
	CheckDistance(pBot);

	pBot->SetAcceleration(pBot->m_behaviour->AccumilateBehaviours(pBot->GetLocation(), pBot->GetVelocity(), 
		DynamicObjects::GetInstance()->GetDominationPoint(pBot->m_behaviour->GetDominationID()).m_Location, Vector2D(0, 0)));

} // Execute()

void Reload::Exit(Bot * pBot)
{
}

void Reload::SetTarget(Bot * pBot)
{
	// Gives the bot a target
	pBot->m_behaviour->m_currentPathTarget = 
		StaticMap::GetInstance()->GetClosestResupplyLocation(pBot->GetLocation());

	pBot->m_behaviour->SetPath(pBot->m_behaviour->m_currentPathTarget, pBot->GetLocation());
}

void Reload::CheckDistance(Bot* pBot)
{ 
	// Clears the path if target is in sight
	if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
		pBot->m_behaviour->m_currentPathTarget))
	{
		pBot->m_behaviour->SetBehaviours(1, 0, 0, 0, 0, 1, 0);
		// Clear path
		while(!pBot->m_behaviour->GetPath()->empty())
			pBot->m_behaviour->GetPath()->pop();
	}

	if ((pBot->GetLocation() - pBot->m_behaviour->m_currentPathTarget).magnitude() < 10.0f)
	{
		pBot->m_behaviour->SetBehaviours(0, 0, 0, 0, 0, 0, 1);
		if(pBot->GetAmmo())
		{
			pBot->ChangeState(Capture::GetInstance());
		}
	}

} 


wchar_t* Reload::GetName()
{
	return L"Reload";
}
