#include "dynamicobjects.h"
#include "renderer.h"
#include "errorlogger.h"
#include "Network.h"

#define NULL 0

DynamicObjects::DynamicObjects()
{
	m_iNumPlacedDominationPoints =0;
	// Tell each bot its own team and number
	// (I hate this)
	for(int i=0;i<NUMTEAMS;i++)
	{
		for(int f=0;f<NUMBOTSPERTEAM;f++)
		{
			m_rgTeams[i].m_rgBots[f].SetOwnNumbers(i,f);
		}
	}



}

void DynamicObjects::Initialise()
{
	// Start the countdown for points
	m_dNextScorePoint=0.0;

	//Create graph
	m_graph.AnalyseMap();

	for(int i=0;i<NUMTEAMS;i++)
	{
		for(int f=0;f<NUMBOTSPERTEAM;f++)
		{
			// So the AI programmer can do any startup stuff
			m_rgTeams[i].m_rgBots[f].StartAI();	
		}
	}
}

DynamicObjects::~DynamicObjects()
{

}



DynamicObjects* DynamicObjects::pInst = NULL;

DynamicObjects* DynamicObjects::GetInstance()
{
	if(pInst == NULL)
	{
		pInst= new DynamicObjects();
	}

	return pInst;
}

void DynamicObjects::Release()		// Static
{
	if(pInst)
	{
		delete pInst;
		pInst = NULL;
	}
}

#define NetSpecificBot(i, j) l_data.m_team[i].m_bots[j]
#define SpecificBot(i, j) m_rgTeams[i].m_rgBots[j]

ErrorType DynamicObjects::Update(float frametime)
{
	if (!Network::GetInstance()->m_isHost)
	{
		if (Network::GetInstance()->Recieve())
		{
			// Used by network if needed
			Network::SendData l_data = Network::GetInstance()->m_data;
			// Set new score if changed to stop flickering
			if(l_data.m_team[0].m_teamScore > m_rgTeams[0].m_iScore)
				m_rgTeams[0].m_iScore = l_data.m_team[0].m_teamScore;
			if (l_data.m_team[1].m_teamScore > m_rgTeams[1].m_iScore)
				m_rgTeams[1].m_iScore = l_data.m_team[1].m_teamScore;
			// Go through bot teams
			for (int i = 0; i < NUMTEAMS; i++)
			{
				// Go through bots
				for (int j = 0; j < NUMBOTSPERTEAM; j++)
				{
					// Bot Data
					//Set Position
					SpecificBot(i, j).SetPosition(NetSpecificBot(i, j).m_posX,
						NetSpecificBot(i, j).m_posY);
					//Set Velocity
					SpecificBot(i, j).SetVelocity(NetSpecificBot(i, j).m_velX,
						NetSpecificBot(i, j).m_velY);
					//Set Directopm
					SpecificBot(i, j).SetDirection(NetSpecificBot(i, j).m_dir);
					//Set is alive
					SpecificBot(i, j).SetAlive(NetSpecificBot(i, j).m_isAlive);

					// Shoot Data
					SpecificBot(i, j).SetShootData(l_data.m_team[i].m_shootData[j].m_targetTeam,
						l_data.m_team[i].m_shootData[j].m_botNumber,
						l_data.m_team[i].m_shootData[j].m_damage,
						l_data.m_team[i].m_shootData[j].m_isFiring);

				}
			// Set position data
			}
		}
	}


	// Update all bots
	for(int i=0;i<NUMTEAMS;i++)
	{
		for(int f=0;f<NUMBOTSPERTEAM;f++)
		{
			m_rgTeams[i].m_rgBots[f].Update(frametime);
		}
	}



	// Check for alliance changes to Domination points
	// Cycle through each domination point
	for(int i=0;i<NUMDOMINATIONPOINTS;i++)
	{
		int numTeamsInRange =0;				// How many teams are close enough to claim the point
		int lastTeamFound = -1;				


		// Cycle through each team
		for(int f=0;f<NUMTEAMS;f++)
		{	
			double dominationRange = DOMINATIONRANGE;	// How close you need to be to claim the point	
			// If this team already owns the point
			if(f == m_rgDominationPoints[i].m_OwnerTeamNumber)
			{
				// ... it doesn't need to be as close to retain it.
				dominationRange*=4;	
			}


			// Cycle through each bot in the team
			for(int g=0;g<NUMBOTSPERTEAM;g++)
			{		
				// How far is this bot from the point under study?
				double range = (m_rgDominationPoints[i].m_Location - m_rgTeams[f].m_rgBots[g].GetLocation()).magnitude();
				// If it is close enough and the bot is still alive
				// add this team to the list of candidate teams
				if(range<dominationRange && m_rgTeams[f].m_rgBots[g].IsAlive())
				{
					numTeamsInRange++;
					lastTeamFound=f;
					break;
				}
			}
		}
		// If only one team can claim it, they claim it.
		// Otherwise, it does not change.
		if(numTeamsInRange==1)
		{
			if(m_rgDominationPoints[i].m_OwnerTeamNumber != lastTeamFound)
			{
				// Special FX
        Renderer::GetInstance()->ShowDominationPointClaimed(m_rgDominationPoints[i].m_Location, lastTeamFound);
				m_rgDominationPoints[i].m_OwnerTeamNumber = lastTeamFound;
			}
		}
	}

	// Add scores to each team for domination points
	m_dNextScorePoint+=frametime;
	// If it is time for the next points to be awarded
	if(m_dNextScorePoint>1.0)
	{
		// Cycle through each domination point
		for(int i=0;i<NUMDOMINATIONPOINTS;i++)
		{	

			int team = m_rgDominationPoints[i].m_OwnerTeamNumber;
			if(team>=0 )		// If the domination point is owned by someone
			{
				// Give a point to the team
				m_rgTeams[team].m_iScore++;
			}
		}
		// Next points will be awarded in one second
		m_dNextScorePoint-=1.0;
	}

	return SUCCESS;
}

int DynamicObjects::GetScore(int teamNumber) const
{
	return m_rgTeams[teamNumber].m_iScore;
}

ErrorType DynamicObjects::Render()
{
	ErrorType answer = SUCCESS;

	Renderer* pRenderer = Renderer::GetInstance();

	// Draw each Bot
	for(int i=0;i<NUMTEAMS;i++)
	{
		for(int f=0;f<NUMBOTSPERTEAM;f++)
		{
			Bot& currentBot = m_rgTeams[i].m_rgBots[f];
			// If it's alive, draw a circle and a line.
			if(currentBot.IsAlive())
			{
				if(pRenderer->DrawBot(currentBot.GetLocation(), currentBot.GetDirection() , i) == FAILURE)
				{
					ErrorLogger::Writeln(L"Renderer failed to draw a bot");
					answer=FAILURE;
				}
			}
			else	// Current Bot is dead
			{
				if(pRenderer->DrawDeadBot(currentBot.GetLocation() , i) == FAILURE)
				{
					ErrorLogger::Writeln(L"Renderer failed to draw a dead bot");
					// Non critical error
				}
			}
		}
	}

	// Draw domination points - just a circle with the right colour.
	for(int i=0;i<NUMDOMINATIONPOINTS;i++)
	{
		if(pRenderer->DrawDominationPoint( m_rgDominationPoints[i].m_Location, m_rgDominationPoints[i].m_OwnerTeamNumber)==FAILURE)
		{
				ErrorLogger::Writeln(L"Renderer failed to draw a domination point");
				answer=FAILURE;
		}
	}

	return SUCCESS;
}


Bot& DynamicObjects::GetBot(int teamNumber, int botNumber)
{
	teamNumber= teamNumber%NUMTEAMS;
	botNumber= botNumber%NUMBOTSPERTEAM;
	return m_rgTeams[teamNumber].m_rgBots[botNumber];
}

DominationPoint DynamicObjects::GetDominationPoint(int pointNumber) const
{
	return m_rgDominationPoints[pointNumber];
}

void DynamicObjects::PlaceDominationPoint(Vector2D location)
{
	if(m_iNumPlacedDominationPoints<NUMDOMINATIONPOINTS)
	{
		m_rgDominationPoints[m_iNumPlacedDominationPoints].m_Location=location;
		m_rgDominationPoints[m_iNumPlacedDominationPoints].m_OwnerTeamNumber=-1;
		++m_iNumPlacedDominationPoints;
	}
}