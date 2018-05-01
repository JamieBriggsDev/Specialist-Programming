#include "behaviour.h"

behaviour::behaviour() : m_owner(nullptr), m_botToShoot(nullptr), m_currentState(nullptr), m_previousState(nullptr)
{
	SetBehaviours(false, false, false, false, false, true, true);
}

void behaviour::Initialise(Bot * _owner)
{
	// Sets behaviours owner bot
	SetOwner(_owner);
}

void behaviour::SetOwner(Bot * _owner)
{
	m_owner = _owner;
}

void behaviour::Update()
{
}

void behaviour::SetCurrentState(State<Bot>* _newState)
{
	// If there is a current state, exit it
	if (m_currentState)
	{
		m_currentState->Exit(m_owner);
		// First sets the previous state with the current state
		m_previousState = m_currentState;

	}
	// Makes the current state the new state
	m_currentState = _newState;
	// Enters the current state if not nullptr
	if(_newState != nullptr)
		m_currentState->Enter(m_owner);
}

//void behaviour::MoveTo(Vector2D _target)
//{
//	// First check if the bot can see the target, if not 
//	//  then use A* to find the path to the target
//	if (StaticMap::GetInstance()->
//		IsLineOfSight(m_owner->GetLocation(), _target))
//	{
//		m_isSeeking = true;
//		m_isFollowingPath = false;
//		m_currentPathTarget = _target;
//	}
//	else
//	{
//		m_path = Graph::GetInstance()->PathFind(Graph::GetInstance()->
//			GetClosestNodePosition(m_owner->GetLocation()),
//			Graph::GetInstance()->
//			GetClosestNodePosition(_target));
//		m_startTarget = _target;
//		m_isFollowingPath = true;
//		m_isSeeking = false;
//	}
//	m_isWantingToStop = true;
//}
//
//void behaviour::StopMoving()
//{
//	m_isWantingToStop = true;
//	m_isSeeking = false;
//	m_isFollowingPath = false;
//}
//
//bool behaviour::CanSeeBotToShoot()
//{
//	return StaticMap::GetInstance()->
//		IsLineOfSight(m_owner->GetLocation(), m_botToShoot->GetLocation());
//}
//
//bool behaviour::CanSeePathToTargetLocation()
//{
//	return StaticMap::GetInstance()->
//		IsLineOfSight(m_owner->GetLocation(), m_startTarget);
//}
//
//void behaviour::OnRespawn()
//{
//	// STATE MACHINE RESET WITH IF(m_myStateMachine)
//
//	m_isWantingToStop = false;
//}
//
//void behaviour::SeekToTarget()
//{
//	// Makes owners acceleration to output of seek to target
//	m_owner->m_Acceleration = Seek(m_owner->m_Position,
//		m_owner->m_Velocity, m_currentPathTarget);
//	if (IsCloseToSeekTarget())
//	{
//		m_isSeeking = false;
//	}
//}
//
//void behaviour::OnTakeDamage()
//{
//	//RETALIATE CODE
//}
//
//bool behaviour::IsCloseToSeekTarget(float _distance)
//{
//	if ((m_owner->GetLocation() - m_currentPathTarget).magnitude() <= _distance)
//		return true;
//	else
//		return false;
//}


void behaviour::SetBehaviours(bool _seek, bool _arrive, bool _pursuit, bool _evade, bool flee, bool walls, bool path)
{
	m_isSeeking = _seek;
	m_isArriving = _arrive;
	m_isPursuiting = _pursuit;
	m_isEvading = _evade;
	m_isFleeing = flee;
	m_isAvoidingWalls = walls;
	m_isFollowingPath = path;
}

Vector2D behaviour::Seek(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition)
{
	Vector2D desiredVelocity = (_targetPosition - _currentPosition).unitVector() * MAXBOTSPEED;

	Vector2D behaviourAccn = (desiredVelocity - _currentVelocity).unitVector() * MAXBOTACCELERATION;

	return behaviourAccn;
}

Vector2D behaviour::Arrive(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition)
{
	float distance = (_targetPosition - _currentPosition).magnitude();
	Vector2D desiredVelocity = ((_targetPosition - _currentPosition).unitVector() * (distance / 2.0f));

	Vector2D behaviourAccn = desiredVelocity - _currentVelocity;

	return behaviourAccn;
}

Vector2D behaviour::Flee(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition)
{
	Vector2D desiredVelocity = (_currentPosition - _targetPosition).unitVector() * MAXBOTSPEED;

	Vector2D behaviourAccn = desiredVelocity - _currentVelocity;

	return behaviourAccn;
}


Vector2D behaviour::Pursue(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime)
{
	float time = (_targetPosition - _currentPosition).magnitude() / MAXBOTSPEED;
	Vector2D PursueTarget = _targetPosition + _targetVelocity * (time * _leadPursuitTime);
	return Seek(_currentPosition, _currentVelocity, PursueTarget);
}

Vector2D behaviour::Evade(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime)
{
	float distance = (_targetPosition - _currentPosition).magnitude();

	float time = distance / MAXBOTSPEED;

	Vector2D PursueTarget = _targetPosition + _targetVelocity * time;

	return Flee(_currentPosition, _currentVelocity, PursueTarget);
}

Vector2D behaviour::AvoidWall(const Vector2D& _currentPosition, const Vector2D& _currentVelocity)
{
	Vector2D t_projection = _currentVelocity.unitVector() * 50;
	// Make collision circles, small for complete collision, big for avoiding collision
	Circle2D t_smallCollisionCircle(_currentPosition + t_projection, 30.0f);
	Circle2D t_bigCollisionCircle(_currentPosition + t_projection, 50.0f);

	// Create a Vector2D for velocity and init to 0
	Vector2D t_desiredVelocity;
	t_desiredVelocity.set(0, 0);

	// Checks to see if the big circle is inside a block in the play field
	if (StaticMap::GetInstance()->IsInsideBlock(t_bigCollisionCircle))
	{
		// Make desired velocity the normal to the surface
		t_desiredVelocity = StaticMap::GetInstance()->GetNormalToSurface(t_bigCollisionCircle);

		// If the small circle is inside the block, increase velocity
		if (StaticMap::GetInstance()->IsInsideBlock(t_smallCollisionCircle))
			t_desiredVelocity *= 10000;
		else
			t_desiredVelocity *= 5000;
	}

	return t_desiredVelocity;
}

Vector2D behaviour::AccumilateBehaviours(const Vector2D& _currentPosition, const Vector2D& _currentVelocity,
	const Vector2D& _targetPosition, const Vector2D& _targetVelocity)
{
	Vector2D t_acceleration;
	// ADD ALL SEVEN BEHAVIOUR STATES TO T_ACCELERATION AND RETURN
	if(m_isSeeking)
		t_acceleration += Seek(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isArriving)
		t_acceleration += Arrive(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isPursuiting)
		t_acceleration += Pursue(_currentPosition, _currentVelocity, _targetPosition, _targetVelocity);
	if (m_isEvading)
		t_acceleration += Evade(_currentPosition, _currentVelocity, _targetPosition, _targetVelocity);
	if (m_isFleeing)
		t_acceleration += Flee(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isAvoidingWalls)
		t_acceleration += AvoidWall(_currentPosition, _currentVelocity);
	if (m_isSeeking)
		t_acceleration += FollowPath(_currentPosition, _currentVelocity);


	return t_acceleration;
}

//Vector2D behaviour::AvoidBots(Vector2D *locations)
//{
//	Vector2D l_BotLocalPositions[NUMBOTSPERTEAM];
//
//	bool l_discarded[NUMBOTSPERTEAM];
//
//	/* Get a list of all friends local positions and sets all discarded*/
//	for (int i = 0; i < (NUMBOTSPERTEAM); i++)
//	{
//		l_BotLocalPositions[i] = m_bot->GetLocation() - locations[i];
//		l_discarded[i] = false;
//	}
//
//	// Combine loops below
//	// Discard Own Bot
//	l_discarded[m_bot->GetOwnBotNumber()] = true;
//
//	// Discard Bots beyond range
//	for (int i = 0; i < (NUMBOTSPERTEAM); i++)
//	{
//		// Wont process if the bot is already discarded
//		if (l_discarded[i] == false)
//		{
//			if (sqrt(l_BotLocalPositions[i].XValue * l_BotLocalPositions[i].XValue +
//				l_BotLocalPositions[i].YValue * l_BotLocalPositions[i].YValue) > 2.0f)
//			{
//				l_discarded[i] = true;
//			}
//		}
//	}
//
//	//// Discard Bots behind
//	//for (int i = 0; i < (NUMBOTSPERTEAM); i++)
//	//{
//	//	// Wont process if the bot is already discarded
//	//	if (discarded[i] == false)
//	//	{
//	//		if (sqrt(BotLocalPositions[i].XValue * BotLocalPositions[i].XValue +
//	//			BotLocalPositions[i].YValue * BotLocalPositions[i].YValue) > 5)
//	//		{
//	//			discarded[i] = true;
//	//		}
//	//	}
//	//}
//
//	// Discard Bots with Y Value > Radius + PlayerRadius (15 is Bot Radius)
//	for (int i = 0; i < (NUMBOTSPERTEAM); i++)
//	{
//		// Wont process if the bot is already discarded
//		if (l_discarded[i] == false)
//		{
//			if (locations[i].YValue > (5 + 5))
//			{
//				l_discarded[i] = true;
//			}
//		}
//	}
//
//	// Get First valid bot and if no valid bot, return Vector(0.0f, 0.0f)
//	int l_closestBot = 0;
//	while (l_discarded[l_closestBot] = false)
//	{
//		l_closestBot++;
//		if (l_closestBot > NUMBOTSPERTEAM)
//		{
//			// No valid bot so return 0.0f
//			return Vector2D(0.0f, 0.0f);
//		}
//	}
//	// First valid bot distance
//	float l_closestBotDistance = sqrt(l_BotLocalPositions[l_closestBot].XValue * l_BotLocalPositions[l_closestBot].XValue +
//		l_BotLocalPositions[l_closestBot].YValue * l_BotLocalPositions[l_closestBot].YValue);
//
//	// Find closest valid bot starting with bot after first valid bot
//	for (int i = (l_closestBot + 1); i < (NUMBOTSPERTEAM); i++)
//	{
//		// Wont process if the bot is already discarded
//		if (l_discarded[i] == false)
//		{
//			float l_check = sqrt(l_BotLocalPositions[i].XValue * l_BotLocalPositions[i].XValue +
//				l_BotLocalPositions[i].YValue * l_BotLocalPositions[i].YValue);
//			if (l_check < l_closestBotDistance)
//			{
//				l_closestBot = i;
//				l_closestBotDistance = l_check;
//			}
//		}
//	}
//
//	// Work out evasion calculations
//	Vector2D l_output = Vector2D(-l_closestBotDistance, 1000 * (float)m_bot->GetOwnBotNumber());
//
//	return l_output;
//}

void behaviour::SetPath(std::stack<Vector2D> _path)
{
	//for (int i = 1; i < _path.size(); i++)
	//{
	//	m_path.push(_path.at(_path.size() - i));
	//}

	//// Sets current target as the first node
	////  then removes current target from list so the
	////  next node can be seen for smoothing
	//m_currentPathTarget = m_path.top();
	//m_path.pop();
	m_path = _path;
}

void behaviour::SetPath(Vector2D _goal, Vector2D _currentLocation)
{
	Vector2D l_start = _currentLocation;
	std::stack<Vector2D> l_path = Graph::GetInstance()->PathFind(l_start,
		_goal);

	m_path = l_path;
}

Vector2D behaviour::FollowPath(const Vector2D& _currentPosition, const Vector2D& _currentVelocity)
{
	Vector2D t_nextNodeInPath;

	if (!m_path.size() == 0)
	{
		// Sets next node to follow the stop of the path stack
		t_nextNodeInPath = m_path.top();

		// Checks to see if the next node in the path can be seen
		if (!(m_path.size() > 1))
		{
			Vector2D t_top = m_path.top();
			m_path.pop();
			Vector2D t_nextLocation = m_path.top();
			m_path.push(t_top);
			if (StaticMap::GetInstance()->IsLineOfSight(_currentPosition, t_nextLocation))
			{
				m_path.pop();
			}

			// Uses a circle to check if bot is arriving at the next node
			Circle2D t_arriveCircle(_currentPosition, 40.0f);

			if (t_arriveCircle.Intersects((Point2D)t_nextNodeInPath))
			{
				m_path.pop();
				m_path.top();
			}
		}
	}

	return Seek(_currentPosition, _currentVelocity, t_nextNodeInPath);


	//if (!StaticMap::GetInstance()->IsLineOfSight(m_owner->m_Position, m_path.top()))
	//{
	//	MoveTo(m_startTarget);
	//}

	//// Follow path, if end of path then arrive
	//if (!CanSeePathToTargetLocation())
	//{
	//	m_owner->m_Acceleration = behaviour::Seek(m_owner->m_Position, m_owner->m_Velocity, m_path.top());
	//}
	//else
	//{
	//	m_owner->m_Acceleration = behaviour::Arrive(m_owner->m_Position, m_owner->m_Velocity, m_startTarget);
	//}

	//// If next node in path can be seen, pop top of the stack and follow next path but 
	////  only if more then 1 positions. If only one position, stop when close to target
	//if (m_path.size() > 1)
	//{
	//	Vector2D top = m_path.top();
	//	m_path.pop();
	//	Vector2D nextLocation = m_path.top();
	//	m_path.push(top);
	//	if (StaticMap::GetInstance()->IsLineOfSight(m_owner->m_Position, nextLocation))
	//	{
	//		m_path.pop();
	//	}
	//}
	//else if ((m_startTarget - m_owner->m_Position).magnitude() < 20.0f)
	//{
	//	m_isFollowingPath = false;
	//	StopMoving();
	//}

}

//void behaviour::PathSmoothing()
//{
//	// Returns true or false if it can see the next node in a path
//	if (!m_path.empty())
//	{
//		if (StaticMap::GetInstance()->IsLineOfSight(m_bot->GetLocation(), m_path.top()))
//		{
//			m_currentPathTarget = m_path.top();
//			m_path.pop();
//		}
//	}
//}
