// mysound.cpp	Version 10.1		18/7/05
// The definition file for the methods in MySound, declared in mysound.h

// Removed wave loading from constructor into separate LoadWave() method



#include "mysound.h"
#include "mysoundengine.h"
#include "errorlogger.h"

#define DSBCAPS_CTRLDEFAULT 0x000000E0

MySound::MySound()
{
	lpSoundBuffer=NULL;
}


ErrorType MySound::LoadWave(wchar_t* pszFilename)
{
	// Get a pointer to the sound engine
	MySoundEngine* theSoundEngine = MySoundEngine::GetInstance();

	if(theSoundEngine==NULL)
	{
		ErrorLogger::Writeln(L"Cannot load a sound wave - MySoundEngine has not been started.");
		return FAILURE;
	}

	if(lpSoundBuffer!=NULL)
	{
		lpSoundBuffer->Release();
		lpSoundBuffer=NULL;
	}

	DSBUFFERDESC dsbd;			// "Order form" for the sound
	WAVEFORMATEX formatdesc;	// Description of the format	
	HMMIO hWaveFile;		// Handle to the wave file
	MMCKINFO parent;		// A parent chunk (wav file data chunks)
	MMCKINFO child;			// A child chunk (wav file data chunks)

	UCHAR *tempBuffer;		// Pointer to a buffer to temporarily store sound
	UCHAR *tempPtr1;		// Pointer to first part of sound buffer
	UCHAR *tempPtr2;		// Pointer to second part of sound buffer
	DWORD length1;			// Length of first part of sound buffer
	DWORD length2;			// Length of second part of sound buffer

	// ***************************************************************
	// Most of what follows is some fairly complicated bits that
	// open a pcm wave file, and read the contents into the
	// directX buffer.

	// Chunk info initialised
 
	parent.ckid			= (FOURCC)0;
	parent.cksize		= 0;
	parent.fccType		= (FOURCC)0;
	parent.dwDataOffset	= 0;
	parent.dwFlags		= 0;

	child=parent;

	// Open the wav file

	hWaveFile = mmioOpen(pszFilename, NULL, MMIO_READ|MMIO_ALLOCBUF);	

	if (hWaveFile==NULL)			// If file could not open
	{
		ErrorLogger::Write(L"Failed to open sound file ");
		ErrorLogger::Writeln(pszFilename);
		return FAILURE;
	}

	// Find the wave section
	// What is it with sound engineers? Is is because they are musicians
	// that they like using meaningless terms like "descend"?
	parent.fccType=mmioFOURCC('W','A','V','E');

	if (mmioDescend(hWaveFile, &parent, NULL, MMIO_FINDRIFF))
	{
		ErrorLogger::Write(L"Couldn't find wave section in wave file ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);			// Error - close the wave file
		return FAILURE;
	}

	// Find the format section
	child.ckid=mmioFOURCC('f','m','t',' ');
	if (mmioDescend(hWaveFile, &child,&parent, 0)!=MMSYSERR_NOERROR)
	{
		ErrorLogger::Write(L"Couldn't find format section in wave file ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);			// Error - close the wave file
		return FAILURE;
	}

	// Read out the format data
	if (mmioRead(hWaveFile, (char *)&formatdesc, sizeof(formatdesc))!=sizeof(formatdesc))
	{
		ErrorLogger::Write(L"Error in wave format of ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);
		return FAILURE;

	}

	// Check this is a pcm format (a standard wav format)
	if (formatdesc.wFormatTag!=WAVE_FORMAT_PCM)
	{
		ErrorLogger::Write(L"Error in wave format of ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);
		return FAILURE;

	}

	// Pop upstairs so we can then get down to data chunk
	if (mmioAscend(hWaveFile, &child, 0)!=MMSYSERR_NOERROR )
	{
		ErrorLogger::Write(L"Couldn't ascend to data chunk of ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);
		return FAILURE;

	}

	// Now drop into data chunk
	child.ckid=mmioFOURCC('d','a','t','a');

	if (mmioDescend(hWaveFile, &child,&parent, MMIO_FINDCHUNK)!=MMSYSERR_NOERROR)
	{
		ErrorLogger::Write(L"Couldn't find data section in wave file ");
		ErrorLogger::Writeln(pszFilename);

		mmioClose(hWaveFile,0);			// Error - close the wave file
		return FAILURE;
	}


	// *************************************************************
	// Now that the info from the file has been stored, it is possible to
	// Create a sound buffer ready to hold the data, so it back to directX

	// Set the descriptor for the sound to be created.

	memset(&dsbd,0,sizeof(dsbd));
	dsbd.dwSize=sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRLDEFAULT;				// Default features
	dsbd.dwBufferBytes=child.cksize;				// Set bytes needed to store
	dsbd.lpwfxFormat=&formatdesc;					// The format descriptor (got earlier from the file)

  LPDIRECTSOUNDBUFFER lpTempSB = NULL;  // Temporary pointer to the IDirectSoundBuffer interface

  // Get the directSoundBuffer interface
  HRESULT err = theSoundEngine->lpds->CreateSoundBuffer(&dsbd, &lpTempSB, NULL);
	if (FAILED(err))
	{
		lpSoundBuffer=NULL;
		ErrorLogger::Write(L"Could not create a sound buffer for ");
    ErrorLogger::Writeln(pszFilename);
		return FAILURE;
	}
  else   // Successfully got the interface
  {
    // Upgrade the interface to IDirectSoundBuffer8
    err = lpTempSB->QueryInterface(IID_IDirectSoundBuffer8, (void**)&lpSoundBuffer);
    if (FAILED(err))
    {
      lpSoundBuffer = NULL;
      ErrorLogger::Write(L"Could not find IDirectSoundBuffer8 interface for: ");
      ErrorLogger::Writeln(pszFilename);
      return FAILURE;
    }
    lpTempSB->Release();    // No longer need the old interface
  }

	// ************************************************************
	// The file is open, the buffer is created. Now to read all the data in.

	// Load data into a buffer
	tempBuffer = (UCHAR *)malloc(child.cksize);
	mmioRead(hWaveFile, (char*)tempBuffer, child.cksize);

	// Close the file
	mmioClose(hWaveFile,0);

	// Locking the Dsound buffer

	err = lpSoundBuffer->Lock(0, child.cksize, (void**) &tempPtr1,
							&length1, (void**) &tempPtr2,
							&length2, DSBLOCK_FROMWRITECURSOR);
	if(FAILED(err))
	{
		ErrorLogger::Write(L"Couldn't lock the sound buffer for ");
    ErrorLogger::Writeln(pszFilename);
		free(tempBuffer);
		lpSoundBuffer->Release();
		lpSoundBuffer=NULL;
		return FAILURE;
	}

	// Copy the two bits of the buffer
	memcpy(tempPtr1, tempBuffer, length1);
	memcpy(tempPtr2, tempBuffer+length1, length2);

	// Unlock the Dsound buffer
	err = lpSoundBuffer->Unlock(tempPtr1,
							length1, tempPtr2,
							length2);
	if(FAILED(err))
	{
		ErrorLogger::Write(L"Couldn't unlock the sound buffer for");
    ErrorLogger::Writeln(pszFilename);
		free(tempBuffer);
		lpSoundBuffer->Release();
		lpSoundBuffer=NULL;
		return FAILURE;
	}

	free(tempBuffer);

	return SUCCESS;
}	// End constructor




MySound::~MySound()
{
	Release();
}

ErrorType MySound::SetPan(int lPan)
{
	if(!lpSoundBuffer)
	{
		ErrorLogger::Writeln(L"Can't pan. Sound buffer not created.");
		return FAILURE;
	}
	HRESULT err = lpSoundBuffer->SetPan(lPan);
	if (FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to pan a sound.");
    if (err == DSERR_CONTROLUNAVAIL)
      ErrorLogger::Writeln(L"Because control is not available.");
    else if (err == DSERR_INVALIDPARAM)
      ErrorLogger::Writeln(L"Because parameter is not valid.");
    else if (err == DSERR_PRIOLEVELNEEDED)
      ErrorLogger::Writeln(L"Because it does not have sufficient priority level.");
    else
      ErrorLogger::Writeln(L"For unknown reasons.");
		return FAILURE;
	}
	return SUCCESS;
}

ErrorType MySound::Release()
{
	if (lpSoundBuffer)				// If lpSoundBuffer is not null
	{
		lpSoundBuffer->Release();	// Attempt to release it

		lpSoundBuffer=NULL;

		return SUCCESS;
	}
	return FAILURE;
}

ErrorType MySound::Play(int flag)
{
	// The first two numbers in the Play() functions below
	// are always zero. The third controls whether to loop,
	// or just play once.

	if(!lpSoundBuffer)
	{
		ErrorLogger::Writeln(L"Cannot play sound. Sound buffer not created.");
		return FAILURE;
	}

	HRESULT err= lpSoundBuffer->Play(0,0,flag);

	if (FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to play a sound");
    if (err == DSERR_INVALIDCALL)
      ErrorLogger::Writeln(L"Because call to Play is invalid.");
    else if (err == DSERR_INVALIDPARAM)
      ErrorLogger::Writeln(L"Because parameter is not valid.");
    else if (err == DSERR_PRIOLEVELNEEDED)
      ErrorLogger::Writeln(L"Because it does not have sufficient priority level.");
    else if (err == DSERR_BUFFERLOST)
      ErrorLogger::Writeln(L"Because the buffer is lost.");
    else
      ErrorLogger::Writeln(L"For unknown reasons.");
		return FAILURE;
	}

	return SUCCESS;
}

ErrorType MySound::Stop()
{
	if(!lpSoundBuffer)
	{
		ErrorLogger::Writeln(L"Sound buffer not created.");
		return FAILURE;
	}

	HRESULT err = lpSoundBuffer->Stop();
	if (FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to stop a sound");
		return FAILURE;
	}

	return SUCCESS;
}

ErrorType MySound::SetVolume(int lVolume)
{
	if(!lpSoundBuffer)
	{
		ErrorLogger::Writeln(L"Sound buffer not created.");
		return FAILURE;
	}
	HRESULT err = lpSoundBuffer->SetVolume(lVolume);
	if (FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to set volume for a sound");
    if (err == DSERR_CONTROLUNAVAIL)
      ErrorLogger::Writeln(L"Because control is unavailable.");
    else if (err == DSERR_INVALIDPARAM)
      ErrorLogger::Writeln(L"Because parameter is not valid.");
    else if (err == DSERR_PRIOLEVELNEEDED)
      ErrorLogger::Writeln(L"Because it does not have sufficient priority level.");
    else if (err == DSERR_BUFFERLOST)
      ErrorLogger::Writeln(L"Because the buffer is lost.");
    else
      ErrorLogger::Writeln(L"For unknown reasons.");
		return FAILURE;
	}
	return SUCCESS;
}


ErrorType MySound::SetFrequency(int lFrequency)
{
	if(!lpSoundBuffer)
	{
		ErrorLogger::Writeln(L"Sound buffer not created.");
		return FAILURE;
	}
	HRESULT err = lpSoundBuffer->SetFrequency(lFrequency);
	if (FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to set frequency for a sound");
    if (err == DSERR_CONTROLUNAVAIL)
      ErrorLogger::Writeln(L"Because control is unavailable.");
    else if (err == DSERR_INVALIDPARAM)
      ErrorLogger::Writeln(L"Because parameter is not valid.");
    else if (err == DSERR_PRIOLEVELNEEDED)
      ErrorLogger::Writeln(L"Because it does not have sufficient priority level.");
    else if (err == DSERR_BUFFERLOST)
      ErrorLogger::Writeln(L"Because the buffer is lost.");
    else
      ErrorLogger::Writeln(L"For unknown reasons.");
		return FAILURE;
	}
	return SUCCESS;
}

LPDIRECTSOUNDBUFFER MySound::GetDirectSoundBuffer()
{
	return lpSoundBuffer;
}