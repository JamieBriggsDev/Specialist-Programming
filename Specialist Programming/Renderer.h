#pragma once
#include "shapes.h"
#include "ErrorType.h"
#include "soundplayer.h"


struct Shot
{
	static const float LIFETIME;
	Vector2D from;
	Vector2D to;
	float time;

	Shot(): time(0)
	{}
};

struct Pulse
{
  static const float LIFETIME;
  Vector2D location;
  int colour;
  float time;
  Pulse() : time(0)
  {}
};

struct Blood
{
	static const float LIFETIME;
	Vector2D m_location;
	Vector2D velocity;
	float time;
	Blood(): time(0)
	{}
};

class Renderer
{
private:
	static const int NUMBLOOD=500;
	static const int NUMSHOTS=10;
  static const int NUMPULSES = 6;
	Renderer();
	~Renderer();
	Renderer(const Renderer& other);		// Disabled
	static Renderer* pInst;
	int m_iNextBlood;						// Next blood element to use
	Blood m_rgBlood[NUMBLOOD];	
	  int m_iNextPulse;						// Next pulse element to use		
  Pulse m_rgPulse[NUMPULSES];
	int m_iNextShot;			// Next shot element to use
	Shot m_rgShot[NUMSHOTS];
	SoundPlayer m_SoundFX;
	int GetTeamColour(int teamNumber);

public:

	static Renderer* GetInstance();				// Thread-safe singleton
	static void Release();
	void SetViewCentre(Vector2D centre);
	void SetViewScale(float scale);
	ErrorType DrawBlock(Rectangle2D area);
	ErrorType DrawDominationPoint(Vector2D location, int teamNumber);
	ErrorType DrawBot(Vector2D location, float direction, int teamNumber);
	ErrorType DrawDeadBot(Vector2D location, int teamNumber);
	ErrorType DrawMousePointer(Vector2D location);
	ErrorType DrawFlagPoint(Vector2D location, int teamNumber);
	ErrorType AddShot(Vector2D from, Vector2D to);
	ErrorType AddBloodSpray(Vector2D location, Vector2D direction, int size);
	ErrorType DrawFX(float frametime);
	ErrorType DrawTextAt(Vector2D position, wchar_t* text);
	ErrorType DrawNumberAt(Vector2D position, double number);
	ErrorType DrawLine(Vector2D from, Vector2D to, int teamNumber=-1);
	ErrorType DrawDot(Vector2D position, int teamNumber=-1);
	ErrorType ShowDominationPointClaimed(Vector2D location, int teamNumber);
  ErrorType ShowReload(Vector2D location);
	bool GetIsValid();			// Returns true if the game is renderable (not minimised, etc)
	// Flips and clears the back buffer
	ErrorType EndScene();
  ErrorType DrawSupplyPoint(Vector2D location);

};
