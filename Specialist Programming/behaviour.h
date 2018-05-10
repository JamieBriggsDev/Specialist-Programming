#pragma once

#define USE_THREADS true

#include <stack>
#include <thread>

#include "shapes.h"
#include "rules.h"
#include "bot.h"
#include "staticmap.h"
#include "rules.h"
#include "State.h"

// Predefines
class Bot;				// Tells behaviour there is a bot class
struct DominationPoint; // Tells behaviour there is a domination point
struct SupplyPoint;		// Tells behaviour there is a supply point

class behaviour
{
private:

	// Path find thread
	std::thread *m_pathThread;
	std::stack<Vector2D> m_path;
	//Vector2D m_currentPathTarget;
	//Vector2D m_startTarget;

	// Behaviour Switches
	bool m_isSeeking = false;
	bool m_isArriving = false;
	bool m_isPursuiting = false;
	bool m_isEvading = false;
	bool m_isFleeing = false;
	bool m_isAvoidingWalls = false;
	bool m_isFollowingPath = false;
	//bool m_bHideOn = false;
	//bool m_bAvoidWallsOn = true;
	//bool m_bArriveOn = false;
	bool m_isWantingToStop = false;
	bool m_isAttemptingShotAtTarget = false;


public:
	// IDs to bot and target
	int m_ownerID;
	int m_botTarget;
	int m_dominationTarget;
	// Owner and target bot
	class Bot* m_owner;
	class Bot* m_botToShoot;
	Vector2D m_startTarget;
	Vector2D m_currentPathTarget;
	// Constructor (Sets bot pointers to nullptr)
	behaviour();
	~behaviour()
	{
		if (USE_THREADS)
			m_pathThread->join();
	}
	// Initialise function which sets the bot
	void Initialise(class Bot* _owner = nullptr);
	// Sets the owner bot, called in initialise
	void SetOwner(class Bot* _owner);
	// Update function
	void Update();

	// Get Owner Bot
	class Bot* GetOwnerBot() { return m_owner; }

	bool IsTargetAlive();
	// Get Current State
	//State<Bot>* GetCurrentState() { return m_currentState; }
	// Set new state
	//void SetCurrentState(State<Bot>* _newState);
	// Set domination target
	void SetDominationID(int _id) { m_dominationTarget = _id; }
	// Get domination target
	int GetDominationID() { return m_dominationTarget; }

	//	// ------------- AI
	//// Moves owner to target location
	//void MoveTo(Vector2D _target);
	//// Stops the owner from moving
	//void StopMoving();
	//// Can see bot wanting to shoot
	//bool CanSeeBotToShoot();
	//// Can see path to Target location
	//bool CanSeePathToTargetLocation();
	//// On Respawn initialise
	//void OnRespawn();
	//// Seeks to the target set in Vector2D m_seekTarget
	//void SeekToTarget();
	//// Process what happens when damage is taken
	//void OnTakeDamage();
	//// Finds Vector2D of m_seekTarget
	//Vector2D GetSeekTarget() { return m_currentPathTarget; }
	//// Checks to see bot is close to the m_seekTarget
	//bool IsCloseToSeekTarget(float _distance = 50.0f);
	
	//Bot* m_bot = nullptr;	// Pointer to bot, gets initialised in bots StartAI
	//Bot* m_enemy = nullptr;	// Pointer to enemy bot
	void SetPath(std::stack<Vector2D> _path) { m_path = _path; }
	void SetPath(Vector2D _goal, Vector2D _currentLocation);
	std::stack<Vector2D>* GetPath() { return &m_path; }

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//				THE SEVEN BEHAVIOURS
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// Set ALL of the behaviours
	void SetBehaviours(bool _seek, bool _arrive, bool _pursuit,
		bool _evade, bool flee, bool path, bool walls = true);
	void SetSeek(bool _toggle) { m_isSeeking = _toggle; }
	void SetArrive(bool _toggle) { m_isArriving = _toggle; }
	void SetPursuit(bool _toggle) { m_isPursuiting = _toggle; }
	void SetEvade(bool _toggle) { m_isEvading = _toggle; }
	void SetFlee(bool _toggle) { m_isFleeing = _toggle; }
	void SetFollowingPath(bool _toggle) { m_isFollowingPath = _toggle; }
	void SetWalls(bool _toggle) { m_isAvoidingWalls = _toggle; }

	// The seven behaviours, all return a Vector2D which is to be added to the bot acceleration
	Vector2D Seek(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, 
		const Vector2D& _targetPosition);	
	Vector2D Arrive(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, 
		const Vector2D& _targetPosition); 
	Vector2D Pursue(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, 
		const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime = 1.0f); 
	Vector2D Evade(const Vector2D& _currentPosition, const Vector2D& _currentVelocity, 
		const Vector2D& _targetPosition, const Vector2D& _targetVelocity, const float& _leadPursuitTime = 1.0f);
	Vector2D Flee(const Vector2D& _currentPosition, const Vector2D& _currentVelocity,
		const Vector2D& _targetPosition); 
	Vector2D FollowPath(const Vector2D& _currentPosition, const Vector2D& _currentVelocity);
	Vector2D AvoidWall(const Vector2D& _currentPosition, const Vector2D& _currentVelocity);

	// Returns a Vector2D which accumilates all 7 different behaviours and uses the 7 switches
	Vector2D AccumilateBehaviours(const Vector2D& _currentPosition, const Vector2D& _currentVelocity,
		const Vector2D& _targetPosition, const Vector2D& _targetVelocity);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//				END OF MAIN BEHAVIOURS
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Vector2D AvoidBots(Vector2D *locations);


	//void PathSmoothing();	// Checks to see if the next node in the path
							//  can be seen and follows it if so
};