#include "behaviour.h"
#include "Renderer.h"
#include "navMesh.h"
#include "mydrawengine.h"


/// <summary>
/// </summary>
behaviour::behaviour() : m_owner(nullptr), m_botToShoot(nullptr), m_pathThread(nullptr)//, m_currentState(nullptr), m_previousState(nullptr)
{
	SetBehaviours(false, false, false, false, false, true, true);
	//m_pathThread = nullptr;
}

void behaviour::Initialise(Bot * _owner)
{
	// Sets behaviours owner bot
	SetOwner(_owner);
}

void behaviour::SetOwner(Bot * _owner)
{
	m_owner = _owner;
	m_ownerID = m_owner->GetBotID();
}

void behaviour::Update()
{
	//if(m_pathThread->)
	if (Graph::GetInstance()->IsPathReady(m_ownerID))
	{
		//m_isFollowingPath = true;
		if (m_pathThread != nullptr)
		{
			m_pathThread->join();
		}
		//if (m_pathThread != nullptr)
		//	delete m_pathThread;
		m_path = Graph::GetInstance()->GetPath(m_ownerID);
	}

}

bool behaviour::IsTargetAlive()
{
	return m_botToShoot->IsAlive();
}


void behaviour::SetBehaviours(bool _seek, bool _arrive, bool _pursuit,
bool _evade, bool flee, bool path, bool walls)
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

	//MyDrawEngine::GetInstance()->DrawLine(_currentPosition, _currentPosition + t_projection, MyDrawEngine::RED);


	// Vector2D for velocity
	Vector2D t_desiredVelocity;
	t_desiredVelocity.set(0, 0);

	// Checks to see if the big circle is inside a block in the play field
	if (StaticMap::GetInstance()->IsInsideBlock(t_bigCollisionCircle))
	{
		// Make desired velocity the normal to the surface
		t_desiredVelocity = StaticMap::GetInstance()->GetNormalToSurface(t_bigCollisionCircle);

		// If the small circle is inside the block, increase velocity
		if (StaticMap::GetInstance()->IsInsideBlock(t_smallCollisionCircle))
			t_desiredVelocity *= 12000;
		else
			t_desiredVelocity *= 6000;
	}

	return t_desiredVelocity;
}

Vector2D behaviour::AccumilateBehaviours(const Vector2D& _currentPosition, const Vector2D& _currentVelocity,
	const Vector2D& _targetPosition, const Vector2D& _targetVelocity)
{
	Vector2D t_tempAcceleration[7];
	Vector2D t_acceleration = Vector2D(0.0f, 0.0f);
	// ADD ALL SEVEN BEHAVIOUR STATES TO T_ACCELERATION AND RETURN
	if (m_isSeeking)
		t_tempAcceleration[0] = Seek(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isArriving)
		t_tempAcceleration[1] = Arrive(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isPursuiting)
		t_tempAcceleration[2] = Pursue(_currentPosition, _currentVelocity, _targetPosition, _targetVelocity);
	if (m_isEvading)
		t_tempAcceleration[3] = Evade(_currentPosition, _currentVelocity, _targetPosition, _targetVelocity);
	if (m_isFleeing)
		t_tempAcceleration[4] = Flee(_currentPosition, _currentVelocity, _targetPosition);
	if (m_isAvoidingWalls)
		t_tempAcceleration[5] = AvoidWall(_currentPosition, _currentVelocity);
	if (m_isFollowingPath)
		t_tempAcceleration[6] = FollowPath(_currentPosition, _currentVelocity);

	for (int i = 0; i < 7; i++)
	{
		t_acceleration += t_tempAcceleration[i];
	}

	if (m_owner->m_currentState->m_name == StateNames::ATTACK)
	{
		t_acceleration += Vector2D(0.0f, 0.0f);
	}

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

//void behaviour::SetPath(std::stack<Vector2D> _path)
//{
//	//for (int i = 1; i < _path.size(); i++)
//	//{
//	//	m_path.push(_path.at(_path.size() - i));
//	//}
//
//	//// Sets current target as the first node
//	////  then removes current target from list so the
//	////  next node can be seen for smoothing
//	//m_currentPathTarget = m_path.top();
//	//m_path.pop();
//	m_path = _path;
//}

void behaviour::SetPath(Vector2D _goal, Vector2D _currentLocation)
{
	Vector2D l_start = _currentLocation;
	std::stack<Vector2D> l_path;
	if (USE_THREADS)
	{
		//l_path.push(Graph::GetInstance()->GetClosestNode(_currentLocation)->m_position);
		Graph::GetInstance()->StartThreadedPathFind(l_start, _goal, m_ownerID, m_pathThread);
	}
	else
	{
		l_path = Graph::GetInstance()->PathFind(l_start,
			_goal, m_owner->GetBotID());

	}
	m_path.swap(l_path);

	m_currentPathTarget = _goal;
	//m_path = l_path;
}

Vector2D behaviour::FollowPath(const Vector2D& _currentPosition, const Vector2D& _currentVelocity)
{
	Vector2D t_nextNodeInPath;

	if (!m_path.size() == 0)
	{
		// Sets next node to follow the stop of the path stack
		t_nextNodeInPath = m_path.top();

		// Checks to see if the next node in the path can be seen
		if ((m_path.size() > 1))
		{
			Vector2D t_top = m_path.top();
			m_path.pop();
			Vector2D t_nextLocation = m_path.top();
			m_path.push(t_top);
			if (StaticMap::GetInstance()->IsLineOfSight(_currentPosition, t_nextLocation))
			{
				m_path.pop();
				t_nextNodeInPath = m_path.top();
			}

			// Uses a circle to check if bot is arriving at the next node
			Circle2D t_arriveCircle(_currentPosition, 40.0f);

			if (t_arriveCircle.Intersects((Point2D)t_nextNodeInPath))
			{
				if (m_ownerID == 0)
					m_path.pop();
				else
					m_path.pop();
				//m_path.top();
			}
		}
		return Seek(_currentPosition, _currentVelocity, t_nextNodeInPath);
	}
	else
	{
		return Seek(_currentPosition, _currentVelocity, m_currentPathTarget);
	}


}

