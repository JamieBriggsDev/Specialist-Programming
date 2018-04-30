// GameTimer.cpp		Version 7			8/12/03	
// The definition file for the methods in "GameTimer", declared in gametimer.h

#include "GameTimer.h"


GameTimer::GameTimer()
{
	if(QueryPerformanceFrequency(&freq)==0)		// Find the timer frequency
	{
		freq.QuadPart=0;									// Set to zero if function failed

	}

	m_dGameRate = 1.0;
	m_dMinimumFrameTime=0.01;
	m_dFrameTime = 0.0;
  m_fFrameTime = 0.0f;
}


void GameTimer::mark()
{

	if (freq.QuadPart<=0)		// If frequency is zero (if QueryPerformanceCounter failed)
	{
		last.QuadPart=0;			// Set everything to zero
		m_dFrameTime=0;
	}
	else
	{
		LARGE_INTEGER now;

		m_dFrameTime=0.0;						// Set to zero, ready for loop
	//	while(m_dFrameTime<m_dMinimumFrameTime)	// Causes a delay until minimum frame time has elapsed
		{

			QueryPerformanceCounter(&now);		// Get current time
			m_dFrameTime=(now.QuadPart - last.QuadPart)/(double)freq.QuadPart;
											// Find time elapsed since last mark, and divide by
											// frequency to convert to seconds
		}

		m_dFrameTime = m_dFrameTime * m_dGameRate;
		last=now;						// Update mark time with current time

	}
  m_fFrameTime = float(m_dFrameTime);
}

void GameTimer::setMinimumFrameTime(double minTime)
{
	if(minTime>0.0)
	{
		m_dMinimumFrameTime = minTime;
	}
	else 
		m_dMinimumFrameTime=0.0;
}
