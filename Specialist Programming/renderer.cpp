#include "renderer.h"
#include "mydrawengine.h"
#define NULL 0

const float Shot::LIFETIME=0.1f;
const float Blood::LIFETIME = 1.0f; 
const float Pulse::LIFETIME = 0.25f;



Renderer::Renderer()
{
	m_iNextBlood=0;						// Next blood element to use
	m_iNextShot=0;
  m_iNextPulse = 0;
	m_SoundFX.LoadAssets();
}

Renderer::~Renderer()
{

}



Renderer* Renderer::pInst=NULL;

Renderer* Renderer::GetInstance()				// Static
{
	if(pInst == NULL)
	{
		pInst= new Renderer();
	}

	return pInst;
}

void Renderer::Release()
{
	pInst->m_SoundFX.Release();
	if(pInst)
	{
		delete pInst;
		pInst = NULL;
	}
}

ErrorType Renderer::ShowDominationPointClaimed(Vector2D location, int teamNumber)
{
  m_rgPulse[m_iNextPulse].location = location;
  m_rgPulse[m_iNextPulse].time = Pulse::LIFETIME;
  m_rgPulse[m_iNextPulse].colour = GetTeamColour(teamNumber);
  ++m_iNextPulse;
  if (m_iNextPulse >= NUMPULSES)
    m_iNextPulse = 0;
	return(m_SoundFX.PlayDomClaim());
}

ErrorType Renderer::ShowReload(Vector2D location)
{
  m_rgPulse[m_iNextPulse].location = location;
  m_rgPulse[m_iNextPulse].time = Pulse::LIFETIME;
  m_rgPulse[m_iNextPulse].colour = GetTeamColour(4);
  ++m_iNextPulse;
  if (m_iNextPulse >= NUMPULSES)
    m_iNextPulse = 0;
  return(m_SoundFX.PlayReload());
}

void Renderer::SetViewCentre(Vector2D centre)
{
	MyDrawEngine::GetInstance()->theCamera.PlaceAt(centre);
}
void Renderer::SetViewScale(float scale)
{
	MyDrawEngine::GetInstance()->theCamera.SetZoom(scale);
}

ErrorType Renderer::DrawBlock(Rectangle2D area)
{
	return MyDrawEngine::GetInstance()->FillRect(area, MyDrawEngine::WHITE);
}

bool Renderer::GetIsValid()
{
	return MyDrawEngine::GetInstance()->IsValid();
}

int Renderer::GetTeamColour(int teamNumber)
{
	int answer;
	switch(teamNumber)
	{
	case 0:
		answer = MyDrawEngine::GetInstance()->RED;
		break;
	case 1:
		answer = MyDrawEngine::GetInstance()->BLUE;
		break;
	case 2:
		answer = MyDrawEngine::GetInstance()->GREEN;
		break;
	case 3:
		answer = MyDrawEngine::GetInstance()->PURPLE;
		break;
	case 4:
		answer = MyDrawEngine::GetInstance()->CYAN;
		break;
	case 5:
		answer = MyDrawEngine::GetInstance()->YELLOW;
		break;
	case 6:
		answer = MyDrawEngine::GetInstance()->WHITE;
		break;
	case 7:
		answer = MyDrawEngine::GetInstance()->LIGHTRED;
		break;
	default:
		answer = MyDrawEngine::GetInstance()->GREY;
		break;	

	}
	return answer;
}

ErrorType Renderer::DrawMousePointer(Vector2D location)
{
	Rectangle2D destRect1;
	destRect1.PlaceAt(location-Vector2D(3,3), location+Vector2D(3,3));

	return MyDrawEngine::GetInstance()->FillRect(destRect1, MyDrawEngine::GREY);
}

ErrorType Renderer::DrawDominationPoint(Vector2D location, int teamNumber)
{
	return MyDrawEngine::GetInstance()->FillCircle(location, 10, GetTeamColour(teamNumber));
}

ErrorType Renderer::DrawBot(Vector2D location, float direction, int teamNumber)
{
	ErrorType answer = SUCCESS;

	answer = MyDrawEngine::GetInstance()->FillCircle(location, 12, GetTeamColour(teamNumber));
	if(MyDrawEngine::GetInstance()->FillCircle(location, 10, 0)==FAILURE)
		answer=FAILURE;

	Vector2D pointer;
	pointer.setBearing(float(direction), 16.0f);
	if(MyDrawEngine::GetInstance()->DrawLine(location, location+pointer, GetTeamColour(teamNumber))==FAILURE)
		answer=FAILURE;

	return answer;
}

ErrorType Renderer::DrawTextAt(Vector2D position, wchar_t* text)
{
  MyDrawEngine::GetInstance()->UseCamera(false);
	ErrorType Answer = MyDrawEngine::GetInstance()->WriteText(position, text, MyDrawEngine::WHITE);
  MyDrawEngine::GetInstance()->UseCamera(true);
  return Answer;
}

ErrorType Renderer::DrawNumberAt(Vector2D position, double number)
{
  MyDrawEngine::GetInstance()->UseCamera(false);
  ErrorType Answer = MyDrawEngine::GetInstance()->WriteDouble(position, number, MyDrawEngine::WHITE);
  MyDrawEngine::GetInstance()->UseCamera(true);
  return Answer;
}

ErrorType Renderer::DrawDeadBot(Vector2D location, int teamNumber)
{
	ErrorType answer = SUCCESS;

	answer = MyDrawEngine::GetInstance()->DrawLine(location+Vector2D(4,4), location-Vector2D(4,4), GetTeamColour(teamNumber));
	if(MyDrawEngine::GetInstance()->DrawLine(location+Vector2D(4,-4), location-Vector2D(4,-4), GetTeamColour(teamNumber))==FAILURE)
		answer=FAILURE;

	return answer;
}


ErrorType Renderer::DrawFlagPoint(Vector2D location, int teamNumber)
{

	return MyDrawEngine::GetInstance()->FillCircle(location, 8, GetTeamColour(teamNumber));
}

ErrorType Renderer::DrawSupplyPoint(Vector2D location)
{
  MyDrawEngine::GetInstance()->FillCircle(location, 10, GetTeamColour(3));
  return MyDrawEngine::GetInstance()->FillCircle(location, 6, GetTeamColour(4));
}

ErrorType Renderer::AddShot(Vector2D from, Vector2D to)
{
	m_SoundFX.PlayShot();
	m_rgShot[m_iNextShot].from=from;
	m_rgShot[m_iNextShot].to=to;
	m_rgShot[m_iNextShot].time=Shot::LIFETIME;
	++m_iNextShot;
	if(m_iNextShot>=NUMSHOTS)
		m_iNextShot=0;
	return SUCCESS;
}

ErrorType Renderer::DrawLine(Vector2D from, Vector2D to, int teamNumber)
{
	return MyDrawEngine::GetInstance()->DrawLine(from, to, GetTeamColour(teamNumber));
}

ErrorType Renderer::DrawDot(Vector2D position, int teamNumber)
{
	Rectangle2D dot;
	dot.PlaceAt(position - Vector2D(5,5), position + Vector2D(5,5));
	return MyDrawEngine::GetInstance()->FillRect(dot, GetTeamColour(teamNumber));
}

ErrorType Renderer::DrawFX(float frametime)
{
	// Shots
	for(int i=0;i<NUMSHOTS;++i)
	{
		if(m_rgShot[i].time>0)
		{
			double shade = m_rgShot[i].time/Shot::LIFETIME;
			int colour = _XRGB(int(255*shade), int(255*shade), int(255*shade));
			MyDrawEngine::GetInstance()->DrawLine(m_rgShot[i].from, m_rgShot[i].to, colour);
			m_rgShot[i].time-=frametime;	
		}
	}
	// Spray
	int detail = 1;//int(m_dScale)+1;			// Reduces how much blood is drawn at large scales
	for(int i=0;i<NUMBLOOD;i+=detail)
	{
		if(m_rgBlood[i].time>0)
		{
			MyDrawEngine::GetInstance()->DrawPoint(m_rgBlood[i].m_location, MyDrawEngine::RED);
			m_rgBlood[i].m_location+=m_rgBlood[i].velocity*frametime;
			m_rgBlood[i].velocity-=m_rgBlood[i].velocity*frametime/Blood::LIFETIME;
			m_rgBlood[i].time-=frametime;	
		}
	}

  // Pulses
  for (int i = 0; i<NUMPULSES; ++i)
  {
    if (m_rgPulse[i].time>0)
    {
      float size = 50 - 50 * m_rgPulse[i].time/Pulse::LIFETIME;
      MyDrawEngine::GetInstance()->FillCircle(m_rgPulse[i].location, size, m_rgPulse[i].colour);
      m_rgPulse[i].time -= frametime;
    }
  }

	return SUCCESS;
}


ErrorType Renderer::EndScene()
{
	//Flip and clear the back buffer
	MyDrawEngine::GetInstance()->Flip();
	MyDrawEngine::GetInstance()->ClearBackBuffer();
	return SUCCESS;
}

ErrorType Renderer::AddBloodSpray(Vector2D location, Vector2D direction, int size)
{
	const float BLOODSPEED = 200.0f;
	direction = direction.unitVector()*BLOODSPEED;
	for(int i=0;i<size;i++)
	{
		Vector2D spray;
		spray.setBearing(rand()*628/100.0f, rand()%900/10.0f);
		spray+=direction;
		m_rgBlood[m_iNextBlood].m_location=location;
		m_rgBlood[m_iNextBlood].velocity=spray;
		m_rgBlood[m_iNextBlood].time=Blood::LIFETIME;
		++m_iNextBlood;
		if(m_iNextBlood>=NUMBLOOD)
			m_iNextBlood=0;
	}
	return SUCCESS;

}