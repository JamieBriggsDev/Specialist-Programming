#include <time.h>

#include "game.h"
#include "staticmap.h"
#include "Renderer.h"
#include "errorlogger.h"
#include "myinputs.h"
#include "dynamicobjects.h"
#include "Network.h"


#define NetSpecificBot(i, j) l_data.m_team[i].m_bots[j]
#define SpecificBot(i, j) GetBot(i, j)

Game* Game::pInst = NULL;

Game* Game::GetInstance()
{
	if(pInst == NULL)
	{
		pInst= new Game();
	}

	return pInst;
}


void Game::Release()		// Static
{
	if(pInst)
	{
		delete pInst;
		pInst = NULL;
	}
}


Game::Game()
{
	InitialiseScript();
	Network::GetInstance()->SetupServer();
}

Game::~Game()
{
	
}

ErrorType Game::Start()
{

	m_timer.mark();
	m_timer.mark();
	m_State = RUNNING;

	return SUCCESS;
}

ErrorType Game::RunInterface()
{
	ErrorType answer=SUCCESS;

	Renderer* pTheRenderer = Renderer::GetInstance();

	// Handle mouse
	static int mouseX = 0;
	static int mouseY = 0;
	static float screenCentreX = 0.0f;
	static float screenCentreY = 0.0f;
	static float zoom = 0.5f;
	MyInputs* pInputs = MyInputs::GetInstance();
	pInputs->SampleMouse();

	mouseX += int(pInputs->GetMouseDX()/zoom);
	mouseY -= int(pInputs->GetMouseDY()/zoom);

	if(mouseX<screenCentreX-200/zoom)
		screenCentreX-=((screenCentreX-200/zoom)-mouseX)/(50*zoom);
	if(mouseX>screenCentreX+200/zoom)
		screenCentreX+=(mouseX-(screenCentreX+200/zoom))/(50*zoom);
	if(mouseY>screenCentreY+200/zoom)
    screenCentreY += (mouseY - (screenCentreY+200/zoom)) / (50 * zoom);
	if(mouseY<screenCentreY-200/zoom)
    screenCentreY -= ((screenCentreY-200/zoom)-mouseY) / (50 * zoom);

	// Handle viewport
	pTheRenderer->SetViewScale(zoom);
	pTheRenderer->SetViewCentre(Vector2D(screenCentreX,screenCentreY));

	pTheRenderer->DrawMousePointer( Vector2D(float(mouseX), float(mouseY)));

	if(pInputs->GetMouseDZ()>0)
		zoom*=1.05f;
	if(pInputs->GetMouseDZ()<0)
		zoom/=1.05f;

	// Display the scores
	for(int i=0;i<NUMTEAMS;i++)
	{
		int score=(DynamicObjects::GetInstance()->GetScore(i));
		Vector2D pos(10.0f, i*25.0f+10.0f);
		pTheRenderer->DrawTextAt(pos, L"Team ");
		pos.set(75.0f, i*25.0f+10.0f);
		pTheRenderer->DrawNumberAt(pos, i+1);
		pos.set(105.0f, i*25.0f+10.0f);
		pTheRenderer->DrawNumberAt(pos, score);
	}

	return answer;
}

ErrorType Game::Update()
{
	// DEBUG
	MyInputs::GetInstance()->SampleKeyboard();

	m_timer.mark();
	// Cap the frame time
	if(m_timer.m_fFrameTime>0.05f)
		m_timer.m_fFrameTime=0.05f;
	ErrorType answer=SUCCESS;

	Renderer* pTheRenderer = Renderer::GetInstance();

	// Used by network if needed
	Network::SendData l_data;

	// If not paused or minimised
	if(m_State == RUNNING)
	// Update Dynamic objects
	{
		DynamicObjects::GetInstance()->Update(m_timer.m_fFrameTime);


		if (Network::GetInstance()->m_isHost)
		{
			MyInputs* l_Input = MyInputs::GetInstance();
			l_Input->SampleKeyboard();
			// Connect to client when num 9 is pressed
			if (l_Input->KeyPressed(DIK_NUMPAD9))
			{
				Network::Release();
				Network::GetInstance()->m_isHost = false;
				if (!Network::GetInstance()->ConnectToServer())
				{
					// if cant connect, clean up and become host
					Network::GetInstance()->Release();
					Network::GetInstance()->SetupServer();
				}
			}
			else
			{

				Network::GetInstance()->CheckNewClients();
				// pack data here to get sent if theres at least one client
				if (Network::GetInstance()->GetNumberOfClients() > 0)
				{
					// Team Score
					l_data.m_team[0].m_teamScore = DynamicObjects::GetInstance()->GetScore(0);
					l_data.m_team[1].m_teamScore = DynamicObjects::GetInstance()->GetScore(1);
					// loop through both teams
					for (int i = 0; i < NUMTEAMS; i++)
					{
						// loop through bots in a team
						for (int j = 0; j < NUMBOTSPERTEAM; j++)
						{


							//Position data
							NetSpecificBot(i, j).m_posX = (double)DynamicObjects::GetInstance()->
								SpecificBot(i, j).GetLocation().XValue;
							NetSpecificBot(i, j).m_posY = (double)DynamicObjects::GetInstance()->
								SpecificBot(i, j).GetLocation().YValue;
							//Velocity data
							NetSpecificBot(i, j).m_velX = (double)DynamicObjects::GetInstance()->
								SpecificBot(i, j).GetVelocity().XValue;
							NetSpecificBot(i, j).m_velY = (double)DynamicObjects::GetInstance()->
								SpecificBot(i, j).GetVelocity().YValue;
							//Rotation
							NetSpecificBot(i, j).m_dir = (double)DynamicObjects::GetInstance()->
								SpecificBot(i, j).GetLocation().YValue;
							// Is Alive
							NetSpecificBot(i, j).m_isAlive = DynamicObjects::GetInstance()->
								SpecificBot(i, j).IsAlive();
							// Shoot data
							l_data.m_team[i].m_shootData[j].m_targetTeam = DynamicObjects::GetInstance()->SpecificBot(i, j).GetTargetTeam();
							l_data.m_team[i].m_shootData[j].m_botNumber = DynamicObjects::GetInstance()->SpecificBot(i, j).GetTargetBot();
							l_data.m_team[i].m_shootData[j].m_damage = DynamicObjects::GetInstance()->SpecificBot(i, j).GetDamage();
							l_data.m_team[i].m_shootData[j].m_isFiring = DynamicObjects::GetInstance()->SpecificBot(i, j).GetIsFiring();

						}
					}
				}
				// End of packing

			}
		}

		if (Network::GetInstance()->m_isActive &&
			!Network::GetInstance()->m_isHost)
		{
			Network::GetInstance()->Recieve();
			//pTheRenderer->DrawNumberAt(Vector2D(0.0f, 0.0f), Network::GetInstance()->m_dataRecieved);
		}


		// DEBUG
		if (MyInputs::GetInstance()->KeyPressed(DIK_NUMPAD0))
		{
			//Graph::GetInstance()->DrawEdges();
			Bot::m_DrawPath = true;
			Graph::GetInstance()->DrawNodes();
			Bot::m_DrawStats = true;

		}
		else
		{
			Bot::m_DrawPath = false; Bot::m_DrawStats = false;

			if (MyInputs::GetInstance()->KeyPressed(DIK_NUMPAD2))
				Graph::GetInstance()->DrawEdges();
			if (MyInputs::GetInstance()->KeyPressed(DIK_NUMPAD3))
				Bot::m_DrawPath = true;
			else
				Bot::m_DrawPath = false;
			if (MyInputs::GetInstance()->KeyPressed(DIK_NUMPAD1))
				Graph::GetInstance()->DrawNodes();
			if (MyInputs::GetInstance()->KeyPressed(DIK_NUMPAD4))
				Bot::m_DrawStats = true;
			else
				Bot::m_DrawStats = false;
		}


	}

	// Render
	if(pTheRenderer->GetIsValid()==false)
	{
		m_State = MINIMISED;			// Or some other reason why invalid
	}
	else
	{
		if(m_State == MINIMISED)
			m_State = PAUSED;			// No longer minimised. Return to pause state.
		if(StaticMap::GetInstance()->Render()==FAILURE)
		{
			ErrorLogger::Writeln(L"Static map failed to render. Terminating.");
			answer = FAILURE;
		}
		if(DynamicObjects::GetInstance()->Render()==FAILURE)
		{
			ErrorLogger::Writeln(L"Dynamic objects failed to render. Terminating.");
			answer = FAILURE;
		}

		if(pTheRenderer->DrawFX(m_timer.m_fFrameTime)==FAILURE)
		{
			ErrorLogger::Writeln(L"Render failed to draw FX.");
			// Continue. Non-critical error.
		}
		if(pTheRenderer->EndScene()==FAILURE)
		{
			ErrorLogger::Writeln(L"Render failed to end scene. Terminating.");
			answer = FAILURE;
		}
	}

	if(RunInterface()==FAILURE)
	{
		ErrorLogger::Writeln(L"Interface reports failure.");
		// Non-critical error
	}

	if (Network::GetInstance()->m_isHost)
	{
		if (Network::GetInstance()->GetNumberOfClients() > 0)
			Network::GetInstance()->Send(l_data);
	}

	return answer;
}

ErrorType Game::End()
{
	Renderer::Release();
	StaticMap::Release();
	DynamicObjects::Release();
	return SUCCESS;
}


ErrorType Game::InitialiseScript()
{
	// Hard code for now
	StaticMap* pMap = StaticMap::GetInstance();

	// Add some rectangles
	Rectangle2D r;
	r.PlaceAt(1000, -1000, 700, -700);
	pMap->AddBlock(r);
  r.PlaceAt(-700, 700, -1000, 1000);
  pMap->AddBlock(r);

  r.PlaceAt(1000, -700, 900, 0);
  pMap->AddBlock(r);
  r.PlaceAt(-900, 0, -1000, 700);
  pMap->AddBlock(r);

  r.PlaceAt(-700, -1000, -1400, -700);
  pMap->AddBlock(r);
  r.PlaceAt(1400, 700, 700, 1000);
  pMap->AddBlock(r);

  r.PlaceAt(300, -1000, -300, -850);
  pMap->AddBlock(r);
  r.PlaceAt(300, 850, -300, 1000);
  pMap->AddBlock(r);

  r.PlaceAt(500, -450, 200, -150);
  pMap->AddBlock(r);
  r.PlaceAt(-200, 150, -500, 450);
  pMap->AddBlock(r);

  r.PlaceAt(-200, -500, -750, -400);
  pMap->AddBlock(r);
  r.PlaceAt(750, 400, 200, 500);
  pMap->AddBlock(r);

  r.PlaceAt(700, -200, 600, 200);
  pMap->AddBlock(r);
  r.PlaceAt(-600, -200, -700, 200);
  pMap->AddBlock(r);


	// Add two spawn points
	pMap->AddSpawnPoint(Vector2D(-1250, 0));
	pMap->AddSpawnPoint(Vector2D(1250, 00));

  // Add four supply points
  pMap->AddSupplyPoint(Vector2D(900, 500));
  pMap->AddSupplyPoint(Vector2D(900, -500));
  pMap->AddSupplyPoint(Vector2D(-900, 500));
  pMap->AddSupplyPoint(Vector2D(-900, -500));

	// Place the domination points
	DynamicObjects* pDynObjects = DynamicObjects::GetInstance();

	pDynObjects->PlaceDominationPoint(Vector2D( -200,-850 ));
	pDynObjects->PlaceDominationPoint(Vector2D( 0,0 ));
	pDynObjects->PlaceDominationPoint(Vector2D( 200,850 ));

	pDynObjects->Initialise();

	return SUCCESS;
}