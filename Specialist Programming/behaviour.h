#pragma once

#include <stack>

#include "shapes.h"
#include "rules.h"
#include "bot.h"
#include "staticmap.h"
#include "rules.h"

// Predefines
class Bot;				// Tells behaviour there is a bot class
struct DominationPoint; // Tells behaviour there is a domination point
struct SupplyPoint;		// Tells behaviour there is a supply point

class behaviour
{
private:
	std::stack<Vector2D> m_path;
	Vector2D m_currentPathTarget;

	// STATES
	bool m_bSeekOn = false;
	bool m_bHideOn = false;
	bool m_bAvoidWallsOn = true;
	bool m_bPathFindOn = false;
	bool m_bArriveOn = false;
public:
	behaviour();

	//
	//Bot* m_bot = nullptr;	// Pointer to bot, gets initialised in bots StartAI
	//Bot* m_enemy = nullptr;	// Pointer to enemy bot

	// Behaviours
	static Vector2D Seek(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition);	//
	static Vector2D Arrive(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition); //
	static Vector2D Flee(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition); //
	static Vector2D Pursue(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime = 1.0f); //
	static Vector2D Evade(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime = 1.0f);

	// Wall and Circle evasion
	Vector2D AvoidWall(Circle2D targetPos);
	//Vector2D AvoidBots(Vector2D *locations);

	// Path Smoothing
	// If the bot can see the next path, then increase the node index by one
	//  until it hits its maximum index size
	void SetPath(std::stack<Vector2D> _path);
	void SetPath(Vector2D _goal, Vector2D _currentLocation);


	//Vector2D FollowPath();	// Follows the stored path in m_path
	//void PathSmoothing();	// Checks to see if the next node in the path
							//  can be seen and follows it if so
};