#include "behaviour.h"

behaviour::behaviour()
{
	
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

Vector2D behaviour::AvoidWall(Circle2D targetPos)
{
	if (StaticMap::GetInstance()->IsInsideBlock(targetPos))
		return (35.0f * StaticMap::GetInstance()->GetNormalToSurface(targetPos));
	else
		return Vector2D(0.0f, 0.0f);
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

//Vector2D behaviour::FollowPath(Vector2D _currentLocation)
//{
//	//Smoothing();
//	// Go to next location once bot gets too close or can see the next node
//	if (!m_path.empty())
//	{
//		if (m_bot->GetDistance(m_bot->GetLocation(), m_currentPathTarget) < 5.0f ||
//			StaticMap::GetInstance()->IsLineOfSight(m_bot->GetLocation(), m_path.top()))
//		{
//
//			m_currentPathTarget = m_path.top();
//			m_path.pop();
//		}
//
//	}
//
//	return Seek(m_currentPathTarget);
//}

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
