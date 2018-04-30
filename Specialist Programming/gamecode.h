//  *********************************************************************************
#include "errortype.h"
// Gamecode.h		Version 11		6/3/08
// Header file to declare the three game loop functions
#pragma once

ErrorType GameInit(bool bFullScreen);

ErrorType GameMain();

void GameShutdown();

// For reading keyboard
#define KEYPRESSED(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

#define SCREENWIDTH 1024
#define SCREENHEIGHT 768
#define COLOURDEPTH 32

