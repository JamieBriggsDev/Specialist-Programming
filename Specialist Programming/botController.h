#pragma once
#include <stack>
#include "vector2D.h"

class botController
{
private:
	class Bot* m_owner;
	class Bot* m_botToShoot;
	class StateMachine* m_myStateMachine;

	Vector2D m_startTarget;
	Vector2D m_seekTarget;
	std::stack<Vector2D> m_path;

	//BOT CONTROLLER STATES
	bool m_isFollowingPath = false;
	bool m_isSeekingTarget = false;
	bool m_isAttemptingShotAtTarget = false;
	bool m_isWantingToStop = true;
public:

	//Methods
	// Constructor
	botController();
	// Initialise function of bot controller
	void Initialise(class Bot* _owner = nullptr);
	// Update function of bot controller
	void Update();
	// Sets the owner bot of bot controller
	void SetOwner(class Bot* _owner);
	// Gets the state machine of bot controller
	class StateMachine* GetStateMachine() { return m_myStateMachine; }
	// Gets the ownder bot of bot controller
	class Bot* GetOwner() { return m_owner; }

	// ------------- AI
	// Moves owner to target location
	void MoveTo(Vector2D _target);
	// Stops the owner from moving
	void StopMoving();
	// Can see bot wanting to shoot
	bool CanSeeBotToShoot();
	// Can see path to Target location
	bool CanSeePathToTargetLocation();
	// On Respawn initialise
	void OnRespawn();
	// Seeks to the target set in Vector2D m_seekTarget
	void SeekToTarget();
	// Process what happens when damage is taken
	void OnTakeDamage();
	// Finds Vector2D of m_seekTarget
	Vector2D GetSeekTarget();
	// Checks to see bot is close to the m_seekTarget
	bool IsCloseToSeekTarget(float _distance = 50.0f);

private:
	// Follows path set in list<Vector2D> m_path
	void FollowPath();
};