#pragma once
#include "errortype.h"
#include "bot.h"
#include "navMesh.h"
#include "Network.h"

const int MAXTEAMS = 2;
const int MAXBOTSPERTEAM = 16;
const int MAXDOMINATIONPOINTS=16;



struct DominationPoint
{
	int m_OwnerTeamNumber;
	Vector2D m_Location;

	DominationPoint()
	{
		m_OwnerTeamNumber=-1;
	}
};

struct Team
{
	Bot m_rgBots[MAXBOTSPERTEAM];
	int m_iScore;

	void Reset()
	{
		m_iScore = 0;
	}

	Team()
	{
		Reset();
	}
};


class DynamicObjects
{
private:
	DynamicObjects();
	~DynamicObjects();
	DynamicObjects(const DynamicObjects & other);	//Disable
	Team m_rgTeams[MAXTEAMS];
	DominationPoint m_rgDominationPoints[MAXDOMINATIONPOINTS];
	static DynamicObjects* pInst;
	int m_iNumPlacedDominationPoints;
	float m_dNextScorePoint;						// Time until next points get added
	Graph m_graph;									// Map of nodes
public:
	static DynamicObjects* GetInstance();			// Thread-safe singleton

	// Deletes pInst
	static void Release();

	// Runs most of the game - tells each Bot to update
	// and checks for domination point capture
	ErrorType Update(float frametime);

	// Places all dynamic objects (Bots and dom points)
	// in the right location
	ErrorType Render();

	// Sets the score timer to zero and initialises the AI 
	// for each Bot
	void Initialise();

	// Returns a reference to the requested bot
	Bot& GetBot(int teamNumber, int botNumber);	

	// Returns the score for the specified team
	int GetScore(int teamNumber) const;

	// Returns the specified Domination point
	DominationPoint GetDominationPoint(int pointNumber) const;

	// Adds a new domination point at the specified location
	// (up to the maximum limit in MAXDOMINATIONPOINTS
	void PlaceDominationPoint(Vector2D location);

	// Returns a reference to the Graph
	Graph& GetGraph()
	{
		return m_graph;
	}

	Network::SendData m_data;
};