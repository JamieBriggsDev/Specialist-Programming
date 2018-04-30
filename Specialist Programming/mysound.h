// Chris Rook

// version 10.1 29/7/05

// Version 10.1
// Added "GetDirectSoundBuffer" method for direct access to direct Sound.

// Version 10
// Inline functions corrected
// SetFrequency() added
// Added more debug messages and safer code.


#include <dsound.h>		// directX draw
#include "errortype.h"

#ifndef MYSOUND_H
#define MYSOUND_H
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")

class MySoundEngine;

class MySound
{
private:
	// This is a long pointer to the DSound buffer. DirectX
	// functions can be called on it, once it it initialised.
	LPDIRECTSOUNDBUFFER8 lpSoundBuffer;

public:
	// Constructor
	// Just sets the pointer to the directX buffer to NULL
	MySound();

	// Preconditions:
	//	The MySoundEngine has been started using Start()
	//  pszFilename points to a string containing the name
	//  of a PCM wave file.
	// Postcondition:
	//  A directX sound buffer is created, and theSound points to it.
	//  The wave data from the sound file is loaded into the buffer.
	ErrorType LoadWave(wchar_t* pszFilename);

	// Destructor
	// Calls shutDown, but no harm is calling it explicitly.
	~MySound();

	// Returns a pointer to the sound buffer encapsulated by the MySound object.
	// This may be used for advanced sound manipulation using DirectSound.
	LPDIRECTSOUNDBUFFER GetDirectSoundBuffer();

	// Precondition:
	//	A sound file has been correctly opened by the constructor.
	// Postconditions:
	//	The sound will start playing. If the flag has been set to 
	//	DSBPLAY_LOOPING, the sound will loop. Otherwise it will play once.
	//  If the sound is currently playing, it will restart.
	// Parameters:
	//	flag - a flag to specify whether or not to loop the sound
	//		Set this to DSBPLAY_LOOPING to make sound loop until stopped.
	//		Set to zero or leave unset to make sound play once.
	// Returns:
	//  SUCCESS if dsound played the sound correctly.
	//  FAILURE if the sound failed to play.	
	ErrorType Play(int flag=0);

	// Precondition:
	//	A sound file has been correctly loaded by the constructor.
	// Postconditions:
	//  The sound file will stop playing.
	// Returns:
	//  SUCCESS if DSound stopped the sound correctly.
	//  FAILURE if DSound reported an error
	ErrorType Stop();

	// Precondition:
	//  Sound file has been correctly loaded.
	//	lVolume is between -10000 and 0.
	// Postcondition:
	//  Volume of the sound is set to level indicated by lVolume.
	//  0 is normal loudness. -10000 is silent.
	// Returns:
	//  SUCCESS if DSound set the volume of the sound correctly.
	//  FAILURE if DSound reported an error
	ErrorType SetVolume(int lVolume);

	// Precondition:
	//  Sound file has been correctly loaded.
	//	lPan is between -10000 and +10000.
	// Postcondition:
	//  Pan of the sound is set to level indicated by lPan.
	//  0 is dead centre. -10000 is fully left. +10000 is fully right.
	//  Note that pan is on top of any stereo data stored in the .wav file
	// Returns:
	//  SUCCESS if DSound set the pan of the sound correctly.
	//  FAILURE if DSound reported an error
	ErrorType SetPan(int lPan);

	// Precondition:
	//  Sound file has been correctly loaded.
	//	lFrequenncy is between DSBFREQUENCY_MIN (100?) and DSBFREQUENCY_MAX (100,000)?
	// Postcondition:
	//  Frequency of the sound is set to level indicated by lFrequency.
	// Returns:
	//  SUCCESS if DSound set the frequency of the sound correctly.
	//  FAILURE if DSound reported an error
	ErrorType SetFrequency(int lFrequency);


	// Postcondition
	//  Releases memory and theSound
	//  theSound set to null
	// Returns:
	//  0 if theSound was initialised
	//  -1 if theSound was not initialised
	ErrorType Release();


};



#endif