#include "sAttack.h"
#include "sCapture.h"
#include "dynamicObjects.h"
#include "bot.h"

// static instance
Attack* Attack::m_pInst = nullptr;

Attack * Attack::GetInstance()
{
	// If instance hasn
	if (!m_pInst)
		m_pInst = new Attack;

	return m_pInst;
}

void Attack::Enter(Bot * pBot)
{
	// Sets target domination point and sets behaviour for bot
	SetTarget(pBot);
	pBot->m_behaviour->SetBehaviours(0, 0, 1, 0, 0, 0);
}

void Attack::Update(Bot * pBot)
{
	// Check ammo first (RELOAD)
	//if (pBot->GetAmmo() < 1)
	//{
	//	pBot->ChangeState(Capture::GetInstance());
	//}
	// Check if target is alive or out of sight
	if (!pBot->m_behaviour->IsTargetAlive() ||
		!StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
			pBot->m_behaviour->m_botToShoot->GetLocation()))
	{
		pBot->ChangeState(Capture::GetInstance());
	}
	// Check if target is too far away (CHASE)
	else if ((pBot->GetLocation() - pBot->m_behaviour->m_botToShoot->GetLocation()).magnitude() >
		ENEMY_TARGET_RANGE - 150.0f)
	{
		pBot->SetAcceleration(pBot->m_behaviour->AccumilateBehaviours(
			pBot->m_behaviour->m_botToShoot->GetLocation(), 
			pBot->m_behaviour->m_botToShoot->GetVelocity(), 
			pBot->GetLocation(),
			pBot->GetVelocity()));
	}
	else if (pBot->GetTargetTeam() != 1)
	{
		pBot->SetTarget(1, pBot->m_behaviour->m_botToShoot->m_iOwnBotNumber);
	}

	// SHOOT CODE
	if ((pBot->GetLocation() - pBot->m_behaviour->m_botToShoot->GetLocation()).magnitude()
		< ENEMY_TARGET_RANGE)
	{
		if (pBot->GetAccuracy() >= 0.7)
			pBot->Shoot();
	}
	else
	{
		if (pBot->GetAccuracy() >= 0.6)
			pBot->Shoot();
	}
	//else
	//{
	//	if (pBot->GetAccuracy() >= 0.6)
	//		pBot->Shoot();
	//}
	// ELSE

}

void Attack::SetTarget(Bot * pBot)
{
	float l_closestDistance = FLT_MAX;

	for (int i = 0; i < NUMBOTSPERTEAM; i++)
	{
		float t_distance = (pBot->GetLocation() -
			DynamicObjects::GetInstance()->GetBot(1, i).GetLocation()).magnitude();
		// First check if in range and closest so far
		if (t_distance <= ENEMY_TARGET_RANGE && t_distance < l_closestDistance)
		{
			// Second check for line of sight if the enemy bot is shootable
			if (StaticMap::GetInstance()->IsLineOfSight(pBot->GetLocation(),
				DynamicObjects::GetInstance()->GetBot(1, i).GetLocation()))
			{
				// If so, set as target bot
				l_closestDistance = t_distance;
				pBot->m_behaviour->m_botToShoot = &DynamicObjects::GetInstance()->GetBot(1, i);
			}
		}
	}
}


void Attack::Exit(Bot* pBot)
{ // Tidies up after the states activities, getting ready for the next state

	pBot->StopAiming();
} // Exit()

wchar_t* Attack::GetName()
{
	return L"ATTACK";
}
