#pragma once
#include "mysound.h"
#include "errorlogger.h"

// Just a class to play sound effects.
// ONly shots, reloads right now. May add more
//	- claim a domination point
class SoundPlayer
{
private:
	static const int NUMSHOTS=6;
	MySound* m_pShots[NUMSHOTS];
	int m_iNextShot;
  MySound* m_pReloads[NUMSHOTS];
  int m_iNextReload;
	MySound* m_DomClaim;
public:
	SoundPlayer()
	{
		for(int i=0;i<NUMSHOTS;i++)
		{
			m_pShots[i] = new MySound();
      m_pReloads[i] = new MySound();
		}
		m_iNextShot=0;
    m_iNextReload = 0;
		m_DomClaim = new MySound();
	}

	void Release()
	{
		for(int i=0;i<NUMSHOTS;i++)
		{
			delete m_pShots[i];
			m_pShots[i]=NULL;
      delete m_pReloads[i];
      m_pReloads[i] = NULL;
		}
		delete m_DomClaim;
		m_DomClaim = NULL;
	}

	~SoundPlayer()
	{
		Release();
	}

	ErrorType LoadAssets()
	{
		for(int i=0;i<NUMSHOTS;i++)
		{
			if(FAILED(m_pShots[i]->LoadWave(L"shot.wav")))
			{
				ErrorLogger::Writeln(L"Failed to load shot.wav");
				return FAILURE;
			}
		}
    for (int i = 0; i < NUMSHOTS; i++)
    {
      if (FAILED(m_pReloads[i]->LoadWave(L"reload.wav")))
      {
        ErrorLogger::Writeln(L"Failed to load shot.wav");
        return FAILURE;
      }
    }
		if(FAILED(m_DomClaim->LoadWave(L"grab.wav")))
		{
			ErrorLogger::Writeln(L"Failed to load grab.wav");
			return FAILURE;
		}
		return SUCCESS;
	}

	ErrorType PlayShot()
	{

		if(FAILED(m_pShots[m_iNextShot++]->Play()))
			{
				ErrorLogger::Writeln(L"Failed to play shot sound effect");
				return FAILURE;
			}
		if(m_iNextShot>=NUMSHOTS)
			m_iNextShot =0;
		return SUCCESS;
	}

  ErrorType PlayReload()
  {

    if (FAILED(m_pReloads[m_iNextReload++]->Play()))
    {
      ErrorLogger::Writeln(L"Failed to play reload sound effect");
      return FAILURE;
    }
    if (m_iNextReload >= NUMSHOTS)
      m_iNextReload = 0;
    return SUCCESS;
  }

	ErrorType PlayDomClaim()
	{

		if(FAILED(m_DomClaim->Play()))
			{
				ErrorLogger::Writeln(L"Failed to play domination claim sound effect");
				return FAILURE;
			}
		return SUCCESS;
	}
};