#include "botController.h"
#include "bot.h"
#include "staticmap.h"
#include "navMesh.h"

botController::botController() : m_owner(nullptr), 
m_myStateMachine(nullptr)
{
}

void botController::Initialise(Bot * _owner)
{
	// If no owner, then set one
	if (_owner)
	{
		SetOwner(_owner);
	}
	// Process code once there is an owner, if statement for safety
	if (m_owner)
	{
		//SORT OUT STATE MACHINE ONCE WRITTEN!!!
	}


}

void botController::SetOwner(Bot * _owner)
{
	m_owner = _owner;
}

void botController::MoveTo(Vector2D _target)
{
	// First check if the bot can see the target, if not 
	//  then use A* to find the path to the target
	if (StaticMap::GetInstance()->
		IsLineOfSight(m_owner->GetLocation(), _target))
	{
		m_isSeekingTarget = true;
		m_isFollowingPath = false;
		m_seekTarget = _target;
	}
	else
	{
		m_path = Graph::GetInstance()->PathFind(Graph::GetInstance()->
			GetClosestNodePosition(m_owner->GetLocation()),
			Graph::GetInstance()->
			GetClosestNodePosition(_target));
		m_startTarget = _target;
		m_isFollowingPath = true;
		m_isSeekingTarget = false;
	}
	m_isWantingToStop = true;
}

void botController::StopMoving()
{
	m_isWantingToStop = true;
	m_isSeekingTarget = false;
	m_isFollowingPath = false;
}

bool botController::CanSeeBotToShoot()
{
	return StaticMap::GetInstance()->
		IsLineOfSight(m_owner->GetLocation(), m_botToShoot->GetLocation());

}

bool botController::CanSeePathToTargetLocation()
{
	return StaticMap::GetInstance()->
		IsLineOfSight(m_owner->GetLocation(), m_startTarget);

}

void botController::OnRespawn()
{
	// STATE MACHINE RESET WITH IF(m_myStateMachine)

	m_isWantingToStop = false;
}

void botController::SeekToTarget()
{
	// Makes owners acceleration to output of seek to target
	m_owner->m_Acceleration = behaviour::Seek(m_owner->m_Position,
		m_owner->m_Velocity, m_seekTarget);
	if (IsCloseToSeekTarget())
	{
		m_isSeekingTarget = false;
	}
}

void botController::OnTakeDamage()
{
	//RETALIATE CODE
}

Vector2D botController::GetSeekTarget()
{
	return m_seekTarget;
}

bool botController::IsCloseToSeekTarget(float _distance)
{
	if ((m_owner->GetLocation() - m_seekTarget).magnitude() <= _distance)
		return true;
	else
		return false;
}

void botController::FollowPath()
{
	if (!StaticMap::GetInstance()->IsLineOfSight(m_owner->m_Position, m_path.top))
	{
		MoveTo(m_startTarget);
	}
}



void botController::Update()
{
	//SORT OUT STATE MACHINE ONCE WRITTEN!!!

}

