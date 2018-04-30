// mysoundengine.cpp	Version 10		9/5/05
// The definition file for the methods in MySoundEngine, declared in mysoundengine.h


#include "mysoundengine.h"
#include "errorlogger.h"

MySoundEngine* MySoundEngine::instance=NULL;

MySoundEngine::MySoundEngine(HWND hwnd)
{
	// Initialise dsound
	HRESULT err;
	if (FAILED(DirectSoundCreate8(&DSDEVID_DefaultPlayback, &lpds, NULL)))
	{
		ErrorLogger::Writeln(L"Failed to create sound player");
		lpds=NULL;
	}

	// Set cooperative level and check for error
	err=lpds->SetCooperativeLevel(hwnd, DSSCL_NORMAL);

	if (FAILED(err))	// If failed to set cooperative level
	{
		ErrorLogger::Writeln(L"Failed to set sound cooperative level\n");
		lpds->Release();
		lpds=NULL;
	}
}

MySoundEngine::~MySoundEngine()
{
	Release();
}

ErrorType MySoundEngine::Release()
{
	if (lpds)			// If not already null
	{
		lpds->Release();
		lpds=NULL;
		return SUCCESS;
	}
	return FAILURE;
}

MySoundEngine* MySoundEngine::Start(HWND hwnd)
{
	if(instance)
	{
		instance->Terminate();
	}
	instance = new MySoundEngine(hwnd);
	return instance;
}

MySoundEngine* MySoundEngine::GetInstance()
{
	return instance;
}

ErrorType MySoundEngine::Terminate()
{
	if(instance)
	{
		delete instance;
		instance=NULL;
		return SUCCESS;
	}
	else
		return FAILURE;
}
