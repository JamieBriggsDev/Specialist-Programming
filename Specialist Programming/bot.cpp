#include <cmath>
#include <chrono>

#include "Debug.h"
#include "bot.h"
#include "dynamicobjects.h"
#include "staticmap.h"
#include "renderer.h"
#include "mydrawengine.h"
#include "navMesh.h"
#include "AllStates.h"
#include "Network.h"

			
bool Bot::m_DrawPath = false;
bool Bot::m_DrawStats = false;

float Bot::GetDistance(Vector2D _first, Vector2D _second)
{
	float x, y, answer;
	// Uses pythagorous
	x = _first.XValue - _second.XValue;
	y = _first.YValue - _second.YValue;
	answer = sqrt((x * x) + (y * y));
	return answer;
}

void Bot::Update(float frametime)
{
	if (Network::GetInstance()->m_isHost)
	{
		if (m_iOwnTeamNumber == 0)
			ProcessAI();
		else
			ProcessAIBadly();
	}
	else
	{
		if (m_DrawStats)
		{
			ClientDrawStats();
		}
	}

	

	// Check for respawn
	if(this->m_dTimeToRespawn>0)
	{
		m_dTimeToRespawn-=frametime;
		this->m_Velocity.set(0,0);
		if(m_dTimeToRespawn<=0)
		{
			PlaceAt(StaticMap::GetInstance()->GetSpawnPoint(m_iOwnTeamNumber));
			Reload();
		}
	}
	else
	{
		// Apply physics ***************************************************************
		
		// Disable movement for aiming bot
		if(this->m_bAiming == true)
		{
			m_Acceleration = -m_Velocity.unitVector()*MAXIMUMSPEED;
		}

		// Clamp acceleration to maximum
		if(m_Acceleration.magnitude()>MAXIMUMACCELERATION)
		{
			m_Acceleration=m_Acceleration.unitVector()*MAXIMUMACCELERATION;
		}

		// Accelerate
		m_Velocity+=m_Acceleration*frametime;

		// Clamp speed to maximum
		if(m_Velocity.magnitude()>MAXIMUMSPEED)
		{
			m_Velocity=m_Velocity.unitVector()*MAXIMUMSPEED;
		}

		if(m_Velocity.magnitude()>10.0)
		{
			m_dDirection = m_Velocity.angle();
		}

		// Move
		m_Position+=m_Velocity*frametime;

		// Check for collision
		Circle2D pos(m_Position, 8);

		if(StaticMap::GetInstance()->IsInsideBlock(pos))
		{
			// Collided. Move back to where you were.
			m_Position-=m_Velocity*frametime;
			m_Velocity.set(0,0);
		}

    // Check for resupply
    if ((StaticMap::GetInstance()->GetClosestResupplyLocation(m_Position) - m_Position).magnitude()<20.0f)
    {
      if (m_iAmmo <= 0 )
      {
        Renderer::GetInstance()->ShowReload(m_Position);
      }
      Reload();
    }

		// Handle shooting ***********************************************

		if(	m_dTimeToCoolDown>0)
			m_dTimeToCoolDown-=frametime;

		if(m_bAiming == true)
		{
			// If target is not valid
			if(m_iAimingAtTeam<0 ||m_iAimingAtTeam>=NUMTEAMS 
				||m_iAimingAtBot<0 ||m_iAimingAtBot>=NUMBOTSPERTEAM  )
			{
				m_bAiming = false;
				m_dAccuracy =0;
			}
			// else if target is dead
			else if(!DynamicObjects::GetInstance()->GetBot(m_iAimingAtTeam, m_iAimingAtBot).IsAlive())
			{
				m_bAiming = false;
				m_dAccuracy =0;
			}
      else if (m_dTimeToCoolDown>0 || m_iAmmo <= 0)
			{
				// Can't shoot. Waiting to cool down or no ammo
				Bot& targetBot = DynamicObjects::GetInstance()->
												GetBot(m_iAimingAtTeam, m_iAimingAtBot);
				if(m_Velocity.magnitude()<=10.0)
				{
					m_dDirection = (targetBot.m_Position-m_Position).angle();
				}
			}
			else		// Valid target
			{
				Bot& targetBot = DynamicObjects::GetInstance()->
												GetBot(m_iAimingAtTeam, m_iAimingAtBot);
				
				if(m_Velocity.magnitude()<=10.0)
				{
					m_dDirection = (targetBot.m_Position-m_Position).angle();
				}

				// Do we have line of sight?
				if(StaticMap::GetInstance()->IsLineOfSight(this->m_Position, targetBot.m_Position))
				{
					float range = (targetBot.m_Position-m_Position).magnitude();
					
					if(range<1.0)
						range = 1.0;		// Probably aiming at yourself
											// Otherwise suicide causes a divide by zero error.
											// That's the real reason why the church is against it.

					float peakAccuracy = sqrt(ACCURATERANGE / range);
					
					// Only gain accuracy if nearly stopped
					if(m_Velocity.magnitude()<MAXBOTSPEED/3)
					{
						m_dAccuracy += (peakAccuracy - m_dAccuracy)*frametime;
					}

					if(m_bFiring==true && m_dTimeToCoolDown<=0)
					{
						// Take the shot
						m_bFiring = false;
            m_iAmmo--;
						m_dTimeToCoolDown = TIMEBETWEENSHOTS;
						m_iShootDamage =0;

						while(m_iShootDamage<100 && (rand()%1000) < int(m_dAccuracy*1000))
						{
							m_iShootDamage +=20;
						}

						targetBot.TakeDamage(m_iShootDamage);

						m_dAccuracy/=2.0;

						Vector2D tgt = targetBot.m_Position;
						if(m_iShootDamage ==0)	// you missed
						{
							// Make it look like a miss
							tgt+=Vector2D(rand()%30-15.0f, rand()%30-15.0f);
						}
						Renderer::GetInstance()->AddShot(m_Position, tgt);
						Renderer::GetInstance()->AddBloodSpray(targetBot.m_Position, targetBot.m_Position-m_Position, m_iShootDamage /5);
					}

				}
				else		// No line of sight
				{
					m_bAiming = false;
					m_dAccuracy =0;				
				}

			}			// End valid target
		}
		else			// Not aiming
		{
			m_dAccuracy =0;
		}
	}
}

void Bot::Reload()
{
  m_iAmmo = MAXAMMO;
}

Vector2D Bot::GetLocation()
{
	return m_Position;
}

float Bot::GetDirection()
{
	return this->m_dDirection;
}

Vector2D Bot::GetVelocity()
{
	return m_Velocity;
}

void Bot::SetOwnNumbers(int teamNo, int botNo)
{
	m_iOwnTeamNumber = teamNo;
	m_iOwnBotNumber = botNo;
}

void Bot::PlaceAt(Vector2D position)
{
	m_Position = position;	// Current world coordinates
	m_Velocity.set(0,0);	// Current velocity
	m_dDirection=0;			// Direction. Mainly useful when stationary
	m_bAiming=false;		// If true, bot is aiming and cannot move
	m_dTimeToCoolDown=0;	// Countdown until the time the bot can shoot again
	m_dTimeToRespawn=0;		// Countdown until the bot can respawn. If zero or below, bot is alive
	m_Acceleration.set(0,0);
	m_bFiring=false;
	m_dAccuracy =0;
	m_iHealth=100;
}

bool Bot::IsAlive()
{
	if(m_dTimeToRespawn<=0)
		return true;
	else
		return false;
}

void Bot::ChangeState(State<Bot>* _state)
{
	//if (_state)
	//{
	//	m_myState->Exit(this);
	//	m_myState = _state;
	//	m_myState->Enter(this);
	//}

	// If there is a current state, exit it
	if (m_currentState != nullptr)
	{
		m_currentState->Exit(this);
		// First sets the previous state with the current state
		m_previousState = m_currentState;

	}
	// Makes the current state the new state
	m_currentState = _state;
	// Enters the current state if not nullptr
	if (_state != nullptr)
		m_currentState->Enter(this);
}

Bot::Bot()
{
	// I suggest you do nothing here.
	// Remember that the rest of the world may not have been created yet.
	PlaceAt(Vector2D(0,0));		// Places bot at a default location
	m_dTimeToRespawn=1;
}


int Bot::GetHealth()
{
	return m_iHealth;
}

double Bot::GetAccuracy()
{	
	if(m_bAiming== true)
		return m_dAccuracy;
	else
		return 0;
}

void Bot::SetTarget(int targetTeamNo, int targetBotNo)
{

	if(m_iAimingAtTeam!=targetTeamNo || m_iAimingAtBot!=targetBotNo)
	{

		m_iAimingAtTeam = targetTeamNo;

		m_iAimingAtBot = targetBotNo;

		m_dAccuracy=0;
	}

	m_bAiming = true;
}

// Stops the bot from aiming, so it can move again
void Bot::StopAiming()
{
	m_bAiming = false;
}

// Call this to set the bot to shoot, if it can.
// Once a shot it taken, the bot will not shoot again unless told to do so.
void Bot::Shoot()
{
	m_bFiring = true;
	m_netFiring = true;
	// Add bool here to network since it is used instantly
	//if (Network::GetInstance()->m_isHost)
	//{
	//	
		DynamicObjects::GetInstance()->
			m_data.m_team[0].m_shootData[m_iOwnBotNumber].m_isFiring = m_bFiring;
	//}
}

// Returns the number of the team of the bot being aimed at.
// Returns a negative number if no bot is being aimed at.
int Bot::GetTargetTeam()
{
	if(m_bAiming== true)
		return m_iAimingAtTeam;
	else
		return -1;
}

// Returns the number of the bot being aimed at.
// Returns a negative number if no bot is being aimed at.
int Bot::GetTargetBot()
{
	if(m_bAiming== true)
		return m_iAimingAtBot;
	else
		return -1;
}

void Bot::TakeDamage(int amount)
{
	m_iHealth-=amount;

	if(m_iHealth<=0 && m_dTimeToRespawn<=0)
	{
		m_dTimeToRespawn = RESPAWNTIME;
		ChangeState(Capture::GetInstance());
	}

	// Being shot at puts you off your aim. 
	// Even more so if you are hit

	if(amount>0)
		m_dAccuracy=0;
	else
		m_dAccuracy/=2;
}

// ****************************************************************************
// This is your function. Use it to set up any states at the beginning of the game
// and analyse the map.
// Remember that bots have not spawned yet, so will not be in their
// starting positions.
// Eventually, this will contain very little code - it just sets a starting state
// and calls methods to analyse the map
void Bot::StartAI()
{
	// Checks to see if the BOT is users team or not

	// Creates a new behaviour and gives it to the bot
	behaviour* myBehaviour = new behaviour;
	m_behaviour = myBehaviour;
	m_behaviour->SetOwner(this);
	
	// Sets states to nullptr
	m_currentState = nullptr;
	m_previousState = nullptr;

	m_msHighestUpdateTime = 0;
	m_msUpdateTime = 0;

	Vector2D start = Vector2D(-1400.0f, 0.0f);
	//Vector2D finish = Vector2D(0.0f, 0.0f);
	Vector2D finish = Graph::GetInstance()->GetRandomNode()->m_position;
	

	//m_behaviour->SetPath(Graph::GetInstance()->PathFind(start, finish));
	//m_currentPath = DynamicObjects::GetInstance()->GetGraph().PathFind(start, finish);
	

	//m_myGraph.AnalyseMap();

}

// This is your function. Use it to set the orders for the bot.
// Will be called once each frame from Update
// Remember this will be called even if the bot is currently dead
// Eventually, this will contain very little code - it just runs the state
void Bot::ProcessAI()
{
	//m_myPathFinder.Initialise(Graph::GetInstance());
	// DEBUGGING UPDATE TIME
	auto t_start = std::chrono::high_resolution_clock::now();
	//m_netFiring = false;
	// Update bots behaviour variables at the start
	if (IsAlive())
	{
		if (m_currentState == nullptr)
			ChangeState(Capture::GetInstance());

		m_currentState->Update(this);
		//m_behaviour->update(this);
		//m_Acceleration += m_behaviour->FollowPath();
		if(Network::GetInstance()->m_isHost)
			m_behaviour->Update();

		// Send is firing here
		DynamicObjects::GetInstance()->m_data.m_team[0].m_shootData[m_iOwnBotNumber].m_isFiring
			= m_bFiring;
		//	= m_bFiring;

	}
	else
		ChangeState(nullptr);


	//if (m_Velocity.magnitude() > MAXBOTSPEED)
	//{
	//	m_Velocity = m_Velocity.unitVector();
	//	m_Velocity *= MAXBOTSPEED;
	//}

	// Check for bot collision
	//m_Acceleration += m_behaviour->AvoidBots(TeamBotLocations);


	// UPDATE VELOCITY AT THE END
	//m_Velocity += m_Acceleration;

	// DEBUG TIME FINISHED
	auto t_finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> t_msUpdateTime = t_finish - t_start;

	// Add update time to recorded data untill 100 had been hit
	if (m_updateCycle < 100 && IsAlive())
	{
		m_recordedValue.push_back(t_msUpdateTime.count());
		m_updateCycle++;
	}
	//else if (m_updateCycle == 100)
	//{
		//string FileName = "Test1STBot" + std::to_string(m_iOwnBotNumber);
		//WriteUpdateTimeToFile(FileName);
		//m_updateCycle++;

	//}

	if(t_msUpdateTime.count() > m_msHighestUpdateTime)
		m_msHighestUpdateTime = t_msUpdateTime.count();

	m_msUpdateTime = t_msUpdateTime.count();


	// Debug info
	// DEBUG DRAWING :: COMMENT OUT WHERE NECASARY
	if (m_DrawPath == true)
	{
		DrawPaths();
	}
	if (m_DrawStats == true)
	{
		DrawStats();
	}
	///////////////////////////////////////////

}

void Bot::DrawPaths()
{

	stack<Vector2D> t_temp = *m_behaviour->GetPath();

	if(!t_temp.empty())
		MyDrawEngine::GetInstance()->DrawLine(m_Position, t_temp.top(), MyDrawEngine::PURPLE);
	else
		MyDrawEngine::GetInstance()->DrawLine(m_Position,m_behaviour->m_currentPathTarget, MyDrawEngine::PURPLE);

	while (t_temp.size() > 1)
	{
		Vector2D t_start = t_temp.top();
		t_temp.pop();
		Vector2D t_finish = t_temp.top();
		MyDrawEngine::GetInstance()->DrawLine(t_start, t_finish, MyDrawEngine::PURPLE);
	}
}

void Bot::DrawStats()
{
	Renderer* pTheRenderer = Renderer::GetInstance();


	// LEFT SIDE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
	// 20 spacing per line
	// SAY WHAT BOT
	pTheRenderer->DrawTextAt(Vector2D(10.0f, 60.0f + (m_iOwnBotNumber * 100)), L"Bot:");
	pTheRenderer->DrawNumberAt(Vector2D(100.0f, 60.0f + (m_iOwnBotNumber * 100)), m_iOwnBotNumber);
	// Check if alive
	if (IsAlive())
	{
		// POSITION DATA
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 80.0f + (m_iOwnBotNumber * 100)), L"Position:");
		pTheRenderer->DrawNumberAt(Vector2D(100.0f, 80.0f + (m_iOwnBotNumber * 100)), (int)m_Position.XValue);
		pTheRenderer->DrawNumberAt(Vector2D(160.0f, 80.0f + (m_iOwnBotNumber * 100)), (int)m_Position.YValue);
		// AMMO DATA
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 100.0f + (m_iOwnBotNumber * 100)), L"Ammo:");
		pTheRenderer->DrawNumberAt(Vector2D(100.0f, 100.0f + (m_iOwnBotNumber * 100)), m_iAmmo);
		// STATE DATA
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 120.0f + (m_iOwnBotNumber * 100)), L"State:");
		pTheRenderer->DrawTextAt(Vector2D(100.0f, 120.0f + (m_iOwnBotNumber * 100)), m_currentState->GetName());
		
	}
	else
	{
		// SAY IS DEAD
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 80.0f + (m_iOwnBotNumber * 100)), L"BOT DEAD!!!");
	}

	// ON BOT
	MyDrawEngine::GetInstance()->WriteInt(m_Position + Vector2D(0.0f, 50.0f), m_iOwnBotNumber, MyDrawEngine::WHITE);

	// RIGHT SIDE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
	if (IsAlive())
	{
		pTheRenderer->DrawTextAt(Vector2D(1500.0f, 80.0f + (m_iOwnBotNumber * 100)), L"       Update Time:");
		pTheRenderer->DrawNumberAt(Vector2D(1750.0f, 80.0f + (m_iOwnBotNumber * 100)), m_msUpdateTime);
		pTheRenderer->DrawTextAt(Vector2D(1500.0f, 100.0f + (m_iOwnBotNumber * 100)), L"Highest Update Time:");
		pTheRenderer->DrawNumberAt(Vector2D(1750.0f, 100.0f + (m_iOwnBotNumber * 100)), m_msHighestUpdateTime);
	}
}

void Bot::ClientDrawStats()
{
	Renderer* pTheRenderer = Renderer::GetInstance();
	// LEFT SIDE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
	// 20 spacing per line
	// SAY WHAT BOT
	pTheRenderer->DrawTextAt(Vector2D(10.0f, 60.0f + (m_iOwnBotNumber * 100)), L"Bot:");
	pTheRenderer->DrawNumberAt(Vector2D(100.0f, 60.0f + (m_iOwnBotNumber * 100)), m_iOwnBotNumber);
	// Check if alive
	if (IsAlive())
	{
		// POSITION DATA
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 80.0f + (m_iOwnBotNumber * 100)), L"Position:");
		pTheRenderer->DrawNumberAt(Vector2D(100.0f, 80.0f + (m_iOwnBotNumber * 100)), (int)m_Position.XValue);
		pTheRenderer->DrawNumberAt(Vector2D(160.0f, 80.0f + (m_iOwnBotNumber * 100)), (int)m_Position.YValue);
		// AMMO DATA
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 100.0f + (m_iOwnBotNumber * 100)), L"Ammo:");
		pTheRenderer->DrawNumberAt(Vector2D(100.0f, 100.0f + (m_iOwnBotNumber * 100)), m_iAmmo);
	}
	else
	{
		// SAY IS DEAD
		pTheRenderer->DrawTextAt(Vector2D(10.0f, 80.0f + (m_iOwnBotNumber * 100)), L"BOT DEAD!!!");
	}
}

void Bot::SetPosition(float _x, float _y)
{
	m_Position = Vector2D(_x, _y);
}

void Bot::SetVelocity(float _x, float _y)
{
	m_Velocity = Vector2D(_x, _y);
}

void Bot::SetDirection(float _r)
{
	m_dDirection = _r;
}

void Bot::SetAlive(bool _alive)
{
	if (_alive)
		m_dTimeToRespawn = 0.0f;
	else
		m_dTimeToRespawn = 1.0f;
}

void Bot::SetShootData(int _team, int _bot, int _damage, bool _isShooting)
{
	m_iAimingAtTeam = _team;
	m_iAimingAtBot = _bot;
	m_iShootDamage = _damage;
	m_bFiring = _isShooting;

	
	
}

//void Bot::WriteUpdateTimeToFile(string _name)
//{
//
//}

void Bot::ProcessAIBadly()
{
	// This is all placeholder code.
	// Delete it all and write your own

	DominationPoint targetDP = DynamicObjects::GetInstance()
		->GetDominationPoint(m_iOwnBotNumber%3);
	
	// Find closest enemy
	int closestEnemy = 0;
	double range = (DynamicObjects::GetInstance()->GetBot
		(1-m_iOwnTeamNumber, 0).m_Position - m_Position).magnitude();
  if (DynamicObjects::GetInstance()->GetBot(1 - m_iOwnTeamNumber, 0).m_iHealth < 0)
    range += 1000;

	for(int i=1;i<NUMBOTSPERTEAM;i++)
	{
		double nextRange = (DynamicObjects::GetInstance()->GetBot
			(1-m_iOwnTeamNumber, i).m_Position - m_Position).magnitude();
    if (DynamicObjects::GetInstance()->GetBot(1 - m_iOwnTeamNumber, 0).m_iHealth < 0)
      nextRange += 1000;
		if(nextRange<range)
		{
			closestEnemy = i;
			range=nextRange;
		}
	}

	Bot& targetEnemy = DynamicObjects::GetInstance()->
		GetBot(1-m_iOwnTeamNumber, closestEnemy);

  Circle2D loc;
  loc.PlaceAt(m_Position, 30);
  
  // Out of ammo?
  if (m_iAmmo<=0)
  {
    // Find closest resupply
    Vector2D closestSupply = StaticMap::GetInstance()->GetClosestResupplyLocation(m_Position);

    if (StaticMap::GetInstance()->IsLineOfSight(m_Position, closestSupply))
    {
      targetPoint = closestSupply;
    }
    else
    {
      // Pick a random point
      Vector2D randomPoint(rand() % 4000 - 2000.0f, rand() % 4000 - 2000.0f);

      // Is it better than current target point?
      double currentValue = (m_Position - targetPoint).magnitude()
        + (closestSupply - targetPoint).magnitude()*1.2;
      if (!StaticMap::GetInstance()->IsLineOfSight(m_Position, targetPoint))
      {
        currentValue += 2500;
      }
      if (!StaticMap::GetInstance()->IsLineOfSight(closestSupply, targetPoint))
      {
        currentValue += 1000;
      }
      // Is it too close to a block?
      Circle2D loc;
      loc.PlaceAt(randomPoint, 20);
      if (StaticMap::GetInstance()->IsInsideBlock(loc))
      {
        currentValue += 500;
      }

      double randomValue = (m_Position - randomPoint).magnitude()
        + (closestSupply - randomPoint).magnitude()*1.2;
      if (!StaticMap::GetInstance()->IsLineOfSight(m_Position, randomPoint))
      {
        randomValue += 1500;
      }
      if (!StaticMap::GetInstance()->IsLineOfSight(closestSupply, randomPoint))
      {
        randomValue += 1000;
      }

      if (randomValue < currentValue)
        // Set as target point
      {
        targetPoint = randomPoint;
      }
    }

    Vector2D desiredVelocity = (targetPoint - m_Position).unitVector() * MAXBOTSPEED;
    m_Acceleration = (desiredVelocity - m_Velocity);
//   	Renderer::GetInstance()->DrawDot(targetPoint);
    StopAiming();
    // Bounce off walls

    Circle2D bounds(m_Position, 50);

    if (StaticMap::GetInstance()->IsInsideBlock(bounds))
    {
      m_Acceleration = StaticMap::GetInstance()->GetNormalToSurface(bounds)*MAXIMUMACCELERATION;
    }
  }
	// Closest enemy within range?
	else if( StaticMap::GetInstance()->IsLineOfSight( m_Position, targetEnemy.m_Position)
		&& targetEnemy.IsAlive() && range<400)
	{
		// Kill it
		SetTarget(1-m_iOwnTeamNumber, closestEnemy);

    if (m_dAccuracy>0.7 || (targetEnemy.m_bAiming && targetEnemy.m_dTimeToCoolDown<0.1 &&m_dAccuracy>0.3))
		{
			Shoot();
		}
	}
  else if (StaticMap::GetInstance()->IsInsideBlock(loc))    // Too close to a wall
  {
    loc.PlaceAt(targetPoint, 30);
    if ((m_Position - targetPoint).magnitude()<45
      || StaticMap::GetInstance()->IsInsideBlock(loc))
    {
      targetPoint = m_Position + Vector2D(rand() % 60 - 30.0f, rand() % 60 - 30.0f);
    }
  }
	else    // Go to domination point
	{
    // If dom point is visible, within 400 units and owned
    if (StaticMap::GetInstance()->IsLineOfSight(m_Position, targetDP.m_Location)
      && (m_Position-targetDP.m_Location).magnitude()<400 &&
      targetDP.m_OwnerTeamNumber == m_iOwnTeamNumber)
    {
      // Stand and kill nearest target
      m_Acceleration = -m_Velocity;
      SetTarget(1 - m_iOwnTeamNumber, closestEnemy);
      if (m_dAccuracy>0.5)
      {
        Shoot();
      }
    }
    // If dom point is visible
		// Dock
		else if(StaticMap::GetInstance()->IsLineOfSight( m_Position, targetDP.m_Location))
		{
			// Target is dom point
      targetPoint = targetDP.m_Location;
      StopAiming();
		}
    else // Navigate to it
		{
			// Pick a random point
			Vector2D randomPoint(rand()%4000-2000.0f, rand()%4000-2000.0f);

			// Is it better than current target point?
			double currentValue = (m_Position - targetPoint).magnitude()
				+ (targetDP.m_Location - targetPoint).magnitude()*1.2;
			if(!StaticMap::GetInstance()->IsLineOfSight(m_Position, targetPoint))
			{
				currentValue+=2500;
			}
			if(!StaticMap::GetInstance()->IsLineOfSight(targetDP.m_Location, targetPoint))
			{
				currentValue+=1000;
			}
      // Is it too close to a block?
      Circle2D loc;
      loc.PlaceAt(randomPoint, 20);
      if (StaticMap::GetInstance()->IsInsideBlock(loc))
      {
        currentValue += 500;
      }

			double randomValue = (m_Position - randomPoint).magnitude()
					+ (targetDP.m_Location - randomPoint).magnitude()*1.2;
			if(!StaticMap::GetInstance()->IsLineOfSight(m_Position, randomPoint))
			{
				randomValue+=1500;
			}
			if(!StaticMap::GetInstance()->IsLineOfSight(targetDP.m_Location, randomPoint))
			{
				randomValue+=1000;
			}

			if(randomValue< currentValue)
			// Set as target point
			{
				targetPoint = randomPoint;
			}
      StopAiming();
		}

		// Head for target point
	//	Renderer::GetInstance()->DrawDot(targetPoint);
		Vector2D desiredVelocity = (targetPoint-m_Position).unitVector() * MAXBOTSPEED;
		m_Acceleration = (desiredVelocity-m_Velocity);

		// Bounce off walls
		Circle2D bounds(m_Position, 50);
		if(StaticMap::GetInstance()->IsInsideBlock(bounds))
		{
			m_Acceleration= StaticMap::GetInstance()->GetNormalToSurface(bounds)*MAXIMUMACCELERATION;
		}

	}
}

