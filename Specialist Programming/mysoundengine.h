// MySoundEngine.h

// Chris Rook

// version 2 29/1/02
// version 10 9/5/05
// Updated to directsound 8
// Added more debug messages
// Using errorlogger
// Safer code
// Converted to singleton

// Inline functions corrected

#pragma once

#include <dsound.h>		// directX draw
#include "mysound.h"
#include "errortype.h"



#pragma comment(lib, "dsound.lib")

class MySoundEngine
{
	friend MySound;		// For fast access to DSound in MySound

private:
	// Simply creates a MySoundEngine
	// hwnd is the handle of the main window
	// Precondition:
	//	DirectSound is installed on the computer
	MySoundEngine(HWND hwnd);

	// The destructor for the MyInstrument. Calls Release().
	~MySoundEngine();

	// Releases the memory and COM objects=. Needs to be called
	// before closing the program. (Gets called by the constructor,
	// but best to call it explicitly as well.)
	// Returns FAILURE if the COM object was not NULL already
	// Returns SUCCESS otherwise
	ErrorType Release();


	IDirectSound8 *lpds;
	static MySoundEngine* instance;

public:
	// Creates the static singleton MySoundEngine instance
	// hwnd is the handle of the main window
	// Precondition:
	//	DirectSound is installed on the computer
	// Note this function should be called once at the start of the game before using 
	// "MySoundEngine::Start()"
	// before using any other methods.
	static MySoundEngine* Start(HWND hwnd);

		// Postcondition:	A pointer to the instance of MyDrawEngine has been returned.
	// Call this using "MySoundEngine enginePtr = MySoundEngine::GetInstance();"
	static MySoundEngine* GetInstance();

	// Postcondition:	The instance of MySoundEngine has been terminated.
	// Returns:			SUCCESS If the instance of MySoundEngine had been started using Start()
	//					FAILURE if the instance of MySoundEngine had not been started.
	// Note that you should call this at the end of your game to avoid a memory leak.
	static ErrorType Terminate();

	// Returns a string describing the directDraw error for most HRESULTs sent to it
	static char* ErrorString(HRESULT err);
};




