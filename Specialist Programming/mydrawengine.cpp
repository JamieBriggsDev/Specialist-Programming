// mydrawengine.cpp		Version 11		6/3/08
// The definition file for the methods in MyDrawEngine - declared in mydrawengine.h
// Version 10 -added error loggers and updated to IDirectDraw7
// Version 11
// Numerous new functions to support Vector2D object placement on screen
// Added bounds check for drawpoint.
// version 11b 3/4/08
// Changed constructor and start to support parameter for fullscreen/windowed



#include "mydrawengine.h"
#include <algorithm>			// Using find() in DeregisterPicture


MyDrawEngine* MyDrawEngine::instance=nullptr;

// *************************************************************
// Constructors and singleton management
// *************************************************************


ErrorType MyDrawEngine::Start(HWND hwnd, bool bFullScreen)
{
	if(instance)		// If an instance already exists
	{
		instance->Terminate();
	}
	 
	// Create the instance
	instance= new MyDrawEngine(hwnd);
	// Start the window for Windows
	if(FAILED(instance->StartWindow()))
	{
		ErrorLogger::Writeln(L"Failed to start the MyDrawEngine window");
		return FAILURE;
	}



	// Start a default font
	instance->AddFont(L"Ariel", 24, false, false);
	
	// If user has requested windowed mode

	instance->GoWindowed();

	// Clear everything
	instance->Flip();
	instance->ClearBackBuffer();

	return SUCCESS;
}		// Start

// **************************************************************

// Static method to return the instance (singleton pattern)
MyDrawEngine* MyDrawEngine::GetInstance()
{
	return instance;
}		// GetInstance

// **************************************************************

// Static method to delete the instance
ErrorType MyDrawEngine::Terminate()
{
	if(instance)
	{
		delete instance;
		instance=nullptr;
		return SUCCESS;
	}
	else
		return FAILURE;
}	// Terminate

// **************************************************************

// Constructor
// width - screen resolution width
// height  - screen resolution height
// hwnd - handle to the Window
MyDrawEngine::MyDrawEngine(HWND hwnd)
{
	// Set pointers to NULL
	m_lpD3D = nullptr;
	m_lpD3DDevice = nullptr;

	m_Hwnd = hwnd;				// Remember the window handle

	m_bFullScreen = true;		// Start off fullscreen

	m_bValid = false;			// Assume not valid until initialised

	// The first font to be created will be at position 0
	m_pNextFont = 0;

	// First image to be created will be at position 1
	m_NextPictureIndex=1;

	m_lpSprite = nullptr;

	m_CameraActive = true;

}		// Constructor

// *******************************************************************

MyDrawEngine::~MyDrawEngine()
{
	// Just make damn sure that directx is released
	Release();
}	// Destructor

// **************************************************************

// Starts the window
ErrorType MyDrawEngine::StartWindow()
{
	// Connect to direct 3D 9
	m_lpD3D=Direct3DCreate9(D3D_SDK_VERSION);
	if (m_lpD3D == nullptr)
	{
		ErrorLogger::Writeln(L"Could not connect to Direct3D");
		return FAILURE;		// No point going any further
	}

	// Get native resolution
	m_NativeScreenWidth=GetSystemMetrics(SM_CXSCREEN);
	m_NativeScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	m_ScreenHeight = m_NativeScreenHeight;
	m_ScreenWidth = m_NativeScreenWidth;

	// Set the presentation parameters options
	D3DPRESENT_PARAMETERS d3dpp;		// Order form for options
	ZeroMemory(&d3dpp, sizeof(d3dpp));	// Set it all to zero

	d3dpp.Windowed = false;				// Whatever the user requested
	d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;	// Slightly slower, but allows access to previous back buffer if ever needed.
	d3dpp.hDeviceWindow = m_Hwnd;			// Handle to the window
	d3dpp.BackBufferWidth = m_ScreenWidth;	// Requested screen width
	d3dpp.BackBufferHeight = m_ScreenHeight; // Requested screen height
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; // Back buffer formattaed to 32 bit XRGB

	// Create the device
	HRESULT err;			// To store the error result
	err=m_lpD3D->CreateDevice(D3DADAPTER_DEFAULT,		// Default graphics adapter
                      D3DDEVTYPE_HAL,			// Requesting hardware abstraction layer
                      m_Hwnd,					// Handle to the window. Again.
                      D3DCREATE_MIXED_VERTEXPROCESSING,	// Process vertices in software !!!!!!
                      &d3dpp,					// The presentation parameters
                      &m_lpD3DDevice);				// Pointer to the device

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create the device");
		ErrorLogger::Writeln(err);
		m_lpD3D->Release();
		m_lpD3D=nullptr;
		return FAILURE;		// No point going any further
	}

	err = D3DXCreateSprite(m_lpD3DDevice, &m_lpSprite );
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create sprite");
		ErrorLogger::Writeln(err);

		m_lpD3D->Release();
		m_lpD3D=nullptr;
		m_lpD3DDevice->Release();
		m_lpD3DDevice = nullptr;
		return FAILURE;		// No point going any further
	}

	m_bValid = true;		// Primary surface is ready for rendering
	return SUCCESS;
}		// Start window

// ********************************************************************

// Release the engine
ErrorType MyDrawEngine::Release()
{
	// Release fonts and bitmaps
	ReleaseBitmaps();
	ReleaseFonts();

	// Release the sprite
	if(m_lpSprite)
	{
		m_lpSprite->Release();
		m_lpSprite = nullptr;
	}

	// Release Direct3D interface
	if(m_lpD3D)
	{
		m_lpD3D->Release();
		m_lpD3D = nullptr;
	}

	// Release the graphics device
	if(m_lpD3DDevice)
	{
		m_lpD3DDevice->Release();
		m_lpD3DDevice = nullptr;
	}

	m_bValid = false;			// No longer a valid primary surface

	return SUCCESS;
}	// Release


// *******************************************************************
// Changing mode and device resets
// *******************************************************************

// Resets the device if it goes FUBAR or when switching fullscreen/windowed
ErrorType MyDrawEngine::ResetDevice()
{
// Set the presentation parameters options
	D3DPRESENT_PARAMETERS d3dpp;			// ORder form
	ZeroMemory(&d3dpp, sizeof(d3dpp));		// Set it all to zero

	d3dpp.Windowed = !m_bFullScreen;			// Whatever the user requested
	d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;	// Slightly slower, but allows access to previous back buffer if ever needed.
	d3dpp.hDeviceWindow = m_Hwnd;			// Handle to the window
	d3dpp.BackBufferWidth = m_ScreenWidth;	// Requested screen width
	d3dpp.BackBufferHeight = m_ScreenHeight;	// Requested screen height
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;	// Format is 32 bit XRGB

	// Need to release all bitmaps and fonts
	ReleaseBitmaps();
	ReleaseFonts();
	if(m_lpSprite)
		m_lpSprite->Release();
	m_lpSprite = nullptr;

	// Reset the device
	HRESULT err = m_lpD3DDevice->Reset(&d3dpp);

	// Did it work?
	if(FAILED(err))
	{
		// Big trouble.
		m_bValid = false;			// Probably already false
	}
	else
	{
		m_bValid = true;			// Device is now valid again.
		// Now can reload the bitmaps and fonts.
		ReloadBitmaps();
		ReloadFonts();

		HRESULT err2 = D3DXCreateSprite(m_lpD3DDevice, &m_lpSprite );
		if(FAILED(err2))
		{
			ErrorLogger::Writeln(L"Failed to create sprite on device reset");
			ErrorLogger::Writeln(err2);
		}

		// Position the window in case needed
		SetWindowPos(m_Hwnd, HWND_NOTOPMOST,
						 0, 0,
						 m_ScreenWidth,
						 m_ScreenHeight,
						 SWP_SHOWWINDOW);
	}

	if(FAILED(err))
		return FAILURE;
	else
		return SUCCESS;
}		// ResetDevice

// ****************************************************************


// Releases all bitmaps. Used on shutdown and when resetting device.
// Does not delete them from the map!
void MyDrawEngine::ReleaseBitmaps()
{
	// Start at the beginning
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.begin();

	// Loop through all bitmaps
	for(;picit!=m_MyPictureList.end();picit++)
	{
		if(picit->second.lpTheTexture)
			picit->second.lpTheTexture->Release();
		picit->second.lpTheTexture = nullptr;
	}
}	// Release bitmaps

// *******************************************************

// Reload all bitmaps. Needed after resetting the device
void MyDrawEngine::ReloadBitmaps()
{
	// Start at the beginning
	std::map<PictureIndex, MyPicture>::iterator it = m_MyPictureList.begin();

	// Loop through all bitmaps
	for(;it!=m_MyPictureList.end();it++)
	{
		// Could be faster by loading in-place, but file loading is much slower anyway
		ReloadPicture(it->first);
	}
}	// ReloadBitmaps

// ******************************************************

// Releases all fonts. Does not delete them from the map.
// Needed when resetting device or on shutdown
void MyDrawEngine::ReleaseFonts()
{
	// Start at the beginning
	std::map<FontIndex, MyFont>::iterator fit = m_MyFontList.begin();

	// Loop through all fonts
	for(;fit!=m_MyFontList.end();fit++)
	{
		if(fit->second.m_pFont)
			fit->second.m_pFont->Release();
		fit->second.m_pFont = nullptr;
	}
}	// ReleaseFonts

// *********************************************************


// Reload all fonts. Needed after resetting the device
void MyDrawEngine::ReloadFonts()
{
	// Start at the beginning
	std::map<FontIndex, MyFont>::iterator fit = m_MyFontList.begin();

	// Loop through fonts
	for(;fit!=m_MyFontList.end();fit++)
	{
		MyFont& currentFont = fit->second;

		// Set requested boldness
		UINT boldness =FW_MEDIUM;
		if(fit->second.m_bold == true)
			boldness = FW_BOLD;

		HRESULT err;

		// Create a font, using recorded parameters from when first loaded
		err = D3DXCreateFont(m_lpD3DDevice, currentFont.m_height, 0, boldness, 0, currentFont.m_italic, 
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
			DEFAULT_PITCH | FF_DONTCARE, currentFont.m_fontName.c_str(), &currentFont.m_pFont );

		if(FAILED(err))
		{
			ErrorLogger::Write(L"Failed to reload a font - ");
			ErrorLogger::Writeln(currentFont.m_fontName.c_str());
			ErrorLogger::Writeln(err);
		}
	}
}	// Reload fonts


// **************************************************************

// Switch to windowed mode
ErrorType MyDrawEngine::GoWindowed()
{
	if(m_bFullScreen == false)
	{
		// Already windowed
		return SUCCESS;			// That will be fifty quid, please
	}

	// remember it is now windowed
	m_bFullScreen = false;

	// Reset the device
	HRESULT err= ResetDevice();

	// Clear the screen
	Flip();
	ClearBackBuffer();

	if(FAILED(err))
	{
		return FAILURE;
	}
	else
	{
		Flip();
		ClearBackBuffer();
		return SUCCESS;
	}
}		// GoWindowed

// **************************************************************

bool MyDrawEngine::IsValid()
{
	return m_bValid;
}

// **************************************************************

// Switch to full screen
ErrorType MyDrawEngine::GoFullScreen()
{

	if(m_bFullScreen == true)
	{
		// Already full screen
		return SUCCESS;			// That will be fifty quid, please
	}

	// Remember you are now full screen
	m_bFullScreen=true;

	// Reset the graphics card
	HRESULT err= ResetDevice();

	// Clear screen
	Flip();
	ClearBackBuffer();

	if(FAILED(err))
		return FAILURE;
	else
		return SUCCESS;
}	// GoFullScreen

// ****************************************************************

bool MyDrawEngine::IsWindowFullScreen() const
{
	return m_bFullScreen;
}

// ****************************************************************

ErrorType MyDrawEngine::ChangeResolution(int width, int height)
{
	int oldWidth = m_ScreenWidth;
	int oldHeight = m_ScreenHeight;
	m_ScreenWidth = width;
	m_ScreenHeight = height;

	ErrorType err = ResetDevice();

	if(FAILED(err))
	{
		m_ScreenWidth = oldWidth;
		m_ScreenHeight = oldHeight;	
		ResetDevice();
		ErrorLogger::Writeln(L"Failed to change resolution. Reverting to previous.");
	}

	return err;
}

ErrorType MyDrawEngine::ChangeToNativeResolution()
{
	int oldWidth = m_ScreenWidth;
	int oldHeight = m_ScreenHeight;
	// Get native resolution
	m_ScreenWidth=m_NativeScreenWidth;
	m_ScreenHeight = m_NativeScreenHeight;

	ErrorType err = ResetDevice();

	if(FAILED(err))
	{
		m_ScreenWidth = oldWidth;
		m_ScreenHeight = oldHeight;	
		ResetDevice();
		ErrorLogger::Writeln(L"Failed to change resolution. Reverting to previous.");
	}

	return err;
}

// *******************************************************************

// Reloads a picture - used when resetting the device
ErrorType MyDrawEngine::ReloadPicture(PictureIndex pic)
{
	// Set the iterator to the row we are looking for in the list of pictures.
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.find(pic);

	// If could not find the picture requested.
	if(picit==m_MyPictureList.end())
	{
		ErrorLogger::Writeln(L"Attempting to reload an invalid PictureIndex in RelocadPicture.");
		return FAILURE;
	}

	// Release the old picture if it is present.
	if(picit->second.lpTheTexture)
	{
		picit->second.lpTheTexture->Release();
		picit->second.lpTheTexture = nullptr;
	}

	// Reload the picture using mostly default settings
	HRESULT err = D3DXCreateTextureFromFileEx(m_lpD3DDevice,
		(picit->second.m_SourceFileName).c_str(), D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
		D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
		0xff000000,				// Colour key is black
		NULL, NULL,
		&(picit->second.lpTheTexture) );

	if(FAILED(err))
	{
		ErrorLogger::Write(L"Failed to create texture from file in ReloadPicture: ");
		ErrorLogger::Writeln(picit->second.m_SourceFileName.c_str());
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	return SUCCESS;
}		// Reload picture

// ***********************************************************************

//Clears the back buffer
ErrorType MyDrawEngine::ClearBackBuffer()
{
	//Clear
	HRESULT err = m_lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0,1.0f,0);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Could not clear the back buffer.");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}
	return SUCCESS;
}	// ClearBackBuffer



// *****************************************************************

// Presents the Back Buffer and starts new scene
ErrorType MyDrawEngine::Flip()
{
	HRESULT err;		// To store error result

	// End the scene
	m_lpD3DDevice->EndScene();
	// Present
	err= m_lpD3DDevice->Present(NULL, NULL, NULL, NULL);

	if(FAILED(err))
	{
    if (IsValid())
    {
		  ErrorLogger::Writeln(L"Could not flip.");
		  ErrorLogger::Writeln(err);
      m_bValid = false;
    }
    if (ResetDevice()==FAILURE)
      return FAILURE;
	}

	// Start the next one
	m_lpD3DDevice->BeginScene();

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Could not start new scene after flip.");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}
	return SUCCESS;
}	// Flip

// **************************************************************
// Camera visibility information
// **************************************************************


// Returns true if the specified point is visible from the current camera position
bool MyDrawEngine::IsOnCamera(Vector2D point) const
{
	return GetViewport().Intersects(point);
}

// Returns true if the specified point is fully or partially visible from the current camera position
bool MyDrawEngine::IsOnCamera(const IShape2D& shape)const
{
	return shape.Intersects(GetViewport());
}

// Returns a rectangle describing the area of the world that is visible from the current camera position
Rectangle2D MyDrawEngine::GetViewport() const
{
	Vector2D tl(0.0f, 0.0f);
	Vector2D bq((float)m_ScreenHeight,(float)m_ScreenWidth);
	Rectangle2D screen;
	screen.PlaceAt(theCamera.Transform(tl), theCamera.Transform(bq));
	return screen;
}

void MyDrawEngine::UseCamera(bool activate)
{
	m_CameraActive = activate;
}


// **************************************************************
// Loading and deleting images
// **************************************************************

// Finds a previously loaded image
PictureIndex MyDrawEngine::FindPicture(wchar_t* filename)
{
	// Find the picture
	std::map<std::wstring, PictureIndex>::iterator picit = m_FilenameList.find(filename);

	// If not found
	if (picit == m_FilenameList.end())
	{
		return -1;
	}
	else
	{
		return picit->second;
	}
}

// Loading a picture
PictureIndex MyDrawEngine::LoadPicture(wchar_t* filename)
{
	// See if file with that name has already been loaded
	PictureIndex found = FindPicture(filename);
	if (found >= 0)			// Already loaded
	{
		return found;
	}
	else					// File not already loaded
	{
		MyPicture tempMyPicture;			// To store picture if it ever loads
		tempMyPicture.m_SourceFileName = filename;	// Remember the filename

		// Load the picture using mostly default settings
		HRESULT err = D3DXCreateTextureFromFileEx(m_lpD3DDevice,
			filename, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
			D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
			0xff000000,				// Colour key is black
			NULL, NULL,
			&tempMyPicture.lpTheTexture);

		if (FAILED(err))		// Probably a bad filename
		{
			ErrorLogger::Write(L"Failed to create texture from file: ");
			ErrorLogger::Writeln(filename);
			ErrorLogger::Writeln(err);
			return 0;			// Return zero if couldn't load
		}

		// Get information from the picture
		D3DSURFACE_DESC desc;
		tempMyPicture.lpTheTexture->GetLevelDesc(0, &desc);

		// Record the height and width
		tempMyPicture.m_height = desc.Height;
		tempMyPicture.m_width = desc.Width;

		// Set the default centre in the middle of the picture
		tempMyPicture.m_Centre.set(float(tempMyPicture.m_width / 2), float(tempMyPicture.m_height / 2));

		// Add the picture to the map
		m_MyPictureList.insert(std::pair<PictureIndex, MyPicture>(m_NextPictureIndex, tempMyPicture));

		m_FilenameList.insert(std::pair<std::wstring, PictureIndex>(filename, m_NextPictureIndex));

		// Increase the number of the next picture, and return the old number.
		return m_NextPictureIndex++;
	}

}		// LoadPicture

// ****************************************************************

// Request the size of a picture
void MyDrawEngine::GetDimensions(PictureIndex pic, int& height, int& width)
{
	// Find the picture
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.find(pic);

	// If not found
	if(picit==m_MyPictureList.end())
	{
		ErrorLogger::Writeln(L"Attempting to get an invalid PictureIndex in GetDimensions.");

	}
	else
	{
		// Set the return-by-reference data
		height = picit->second.m_height;
		width = picit->second.m_width;
	}
}	// GetDimensions



// ***********************************************************************



// Sets the centre of a MyPicture
ErrorType MyDrawEngine::SetCentre(PictureIndex pic, Vector2D centre)
{
	// Find the picture
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.find(pic);

	// If not found
	if(picit==m_MyPictureList.end())
	{
		ErrorLogger::Writeln(L"Attempting to set centre of an invalid PictureIndex in SetCentre.");
		return FAILURE;
	}
	else
	{
		// Set the centre
		picit->second.m_Centre = centre;
		return SUCCESS;
	}

}	// SetCentre



// **************************************************************

// Release a picture from memory
void MyDrawEngine::ReleasePicture(PictureIndex pic)
{
	// Find the picture in the map
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.find(pic);

	// If not found
	if(picit==m_MyPictureList.end())
	{
		ErrorLogger::Writeln(L"Attempting to release an invalid PictureIndex in ReleasePicture.");
		return;
	}

	// Release it and set to null
	if(picit->second.lpTheTexture)
		picit->second.lpTheTexture->Release();
	picit->second.lpTheTexture = nullptr;

	// Remove it from the map
	m_MyPictureList.erase(picit);

}		// ReleasePicture



// **************************************************************
// Writing text and numbers
// **************************************************************


// *****************************************************************

// Add a new font to the engine
FontIndex MyDrawEngine::AddFont(wchar_t* FontName, int height, bool bold, bool italic)
{
	MyFont temp;			// To hold the font being created
	// Record requested parameters
	temp.m_bold = bold;
	temp.m_fontName = FontName;
	temp.m_height = height;
	temp.m_italic = italic;

	// Set up the value of "THICKNESS" needed in D3DXCreateFont
	UINT boldness =FW_MEDIUM;
	if(bold == true)
		boldness = FW_BOLD;

	HRESULT err;

	// Create the font, as requested
	err = D3DXCreateFont(m_lpD3DDevice, height, 0, boldness, 0, italic, 
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_DONTCARE, FontName, &temp.m_pFont );

	if(FAILED(err))
	{
		ErrorLogger::Write(L"Failed to create font ");
		ErrorLogger::Writeln(FontName);
		ErrorLogger::Writeln(err);
		if(temp.m_pFont)
			temp.m_pFont->Release();
		return 0;
	}
	else
	{
		// Add it to the map
		m_MyFontList.insert(std::pair<FontIndex, MyFont>(m_pNextFont, temp));
		return m_pNextFont++;		// Return the number of the font
	}
}
 

// ********************************************************************************

// Writes text to the screen
ErrorType MyDrawEngine::WriteText(int x, int y, const wchar_t text[], int colour, FontIndex fontIndex )
{
	// Find the requested font
	std::map<FontIndex, MyFont>::iterator fit;	// Iterator to point to the font requested
	fit = m_MyFontList.find(fontIndex);				// Find the font
	if(fit == m_MyFontList.end())						// If font not found
	{
		ErrorLogger::Writeln(L"Write Text called with invalid FontIndex.");
		return FAILURE;
	}

	// Rect to draw the text inside
	RECT rect;
	rect.left =x;
	rect.top = y;
	rect.right =x+50;		// Will get expanded when the rect is calculated
	rect.bottom = y+50;

	// Nonna-Raymond call to DrawText. Calculates the rect, but does not draw
	fit->second.m_pFont->DrawText(NULL, text, -1, &rect, DT_CALCRECT , colour);

	// Now make the real call.
	HRESULT err;
	err = fit->second.m_pFont->DrawText(NULL, text, -1, &rect, 0 , colour);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"WriteText failed to draw text.");
		ErrorLogger::Writeln(err);
		return FAILURE;		
	}
	return SUCCESS;	

}	// End WriteText

// **********************************************************

// Writes a floating point number to the screen
ErrorType MyDrawEngine::WriteDouble(int x, int y, double num, int colour, FontIndex fontIndex )
{
	wchar_t buffer[32];			// To store the text when converted
	swprintf_s( buffer, 32, L"%.8g", num );	// Convert the number to text

	// Write it
	return WriteText(x,y, buffer, colour, fontIndex);
}	// WriteDouble

// **********************************************************
// Writes an integer point number to the screen
ErrorType MyDrawEngine::WriteInt(int x, int y, int num, int colour, FontIndex fontIndex )
{
	wchar_t buffer[32];
	swprintf_s( buffer,32, L"%i", num );

	return WriteText(x,y, buffer, colour, fontIndex);
}	// WriteInt

// *******************************************************************

ErrorType MyDrawEngine::WriteText(Vector2D position, const wchar_t text[], int colour, FontIndex fontIndex)
{
	if (m_CameraActive)
	{
		position = theCamera.Transform(position);
	}
	return WriteText(int(position.XValue), int(position.YValue), text, colour, fontIndex);
}

ErrorType MyDrawEngine::WriteInt(Vector2D position, int num, int colour, FontIndex fontIndex )
{
	if (m_CameraActive)
	{
		position = theCamera.Transform(position);
	}
	return WriteInt(int(position.XValue), int(position.YValue), num, colour, fontIndex);
}

ErrorType MyDrawEngine::WriteDouble(Vector2D position, double num, int colour, FontIndex fontIndex )
{
	if (m_CameraActive)
	{
		position = theCamera.Transform(position);
	}
	return WriteDouble(int(position.XValue), int(position.YValue), num, colour, fontIndex);
}



// **************************************************************
// Drawing Functions
// **************************************************************

// Draw a picture at the requested location
ErrorType MyDrawEngine::DrawAt(Vector2D position, PictureIndex pic, float scale, float angle, float transparency)
{
	// Find the picture
	std::map<PictureIndex, MyPicture>::iterator picit = m_MyPictureList.find(pic);

	if (m_CameraActive)
	{
		position = theCamera.Transform(position);
		scale = theCamera.Transform(scale);
	}

	// If not found
	if(picit==m_MyPictureList.end())
	{
		ErrorLogger::Writeln(L"Attempting to draw an invalid PictureIndex in DrawAt.");
		return FAILURE;	
	}
	

	MyPicture& thePicture = picit->second;		// Reference to the picture for easy coding

	// Check texture is loaded
	if(!thePicture.lpTheTexture)
	{
		ErrorLogger::Writeln(L"Cannot render MyPicture in DrawAt. MyPicture not initialised.");
		return FAILURE;
	}

	// Start drawing
	HRESULT err = m_lpSprite->Begin(D3DXSPRITE_ALPHABLEND);		// Alpha Blending requested
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to begin sprite render in DrawAt");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Specify the centre of the sprite - will be (height/2,width/2) unless user has asked for something 
	// different.
	D3DXVECTOR2 centre (thePicture.m_Centre.XValue, thePicture.m_Centre.YValue);

	// Create a transformation matrix for the requested scale, rotation and position.
	D3DXMATRIX transform;
	D3DXVECTOR2 scaling(scale, scale);
	D3DXVECTOR2 pos;
	pos.x = (position - thePicture.m_Centre).XValue;
	pos.y = (position - thePicture.m_Centre).YValue;
	D3DXMatrixTransformation2D(&transform, &centre, 0.0, &scaling, &centre, -angle, &pos);
	
	// Set the transformation matrix
	m_lpSprite->SetTransform(&transform);

	// Modulate the colour to add transparency
	unsigned int alpha = int(255-255*transparency)%256;
	unsigned int colour = 0xFFFFFF+(alpha<<24);

	// Draw the sprite
	err = m_lpSprite->Draw(thePicture.lpTheTexture, NULL, NULL, NULL, colour);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw sprite in DrawAt");
		ErrorLogger::Writeln(err);
		m_lpSprite->End();
		return FAILURE;
	}

	// Complete the sprite
	m_lpSprite->End();

	return SUCCESS;
}	// DrawAt



// **************************************************************

// Draws a line between the two coordinates
ErrorType MyDrawEngine::DrawLine( Vector2D start,  Vector2D end, unsigned int colour)
{
	if (m_CameraActive)
	{
		start = theCamera.Transform(start);
		end = theCamera.Transform(end);
	}

	// Vertex array with two vertices needed
		MYVERTEX Vertices[] = {
		{start.XValue, start.YValue, 0.0f, 1.0f, colour,},
		{end.XValue, end.YValue, 0.0f, 1.0f, colour,},
	};

	// Pointer to DirectX Vertex buffer
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;

	// Create the buffer using my default format
	HRESULT err =m_lpD3DDevice->CreateVertexBuffer(2*sizeof(MYVERTEX),
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in DrawLine");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 

	// Lock the buffer
	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);    
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock vertex buffer in DrawLine");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}

	// Copy vertices into the buffer
	memcpy(pBuff, Vertices, sizeof(Vertices));

	// Unlock the buffer
	err= vertexBuffer->Unlock();
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to unlock vertex buffer in DrawLine");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}

	// Set the vertex format
	err = m_lpD3DDevice->SetFVF(MYFVF);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to set FVF in DrawLine");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}
	// Set a default stream
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Draw, as a line list, starting at vertex 0.
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0,1);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw a primitive in DrawLine");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}
	// Release the buffer
	vertexBuffer->Release();

	return SUCCESS;
}	// DrawLine



// ******************************************************************

// Fill a circle
ErrorType MyDrawEngine::FillCircle( Vector2D centre, float radius, unsigned int colour)
{
	if (m_CameraActive)
	{
		centre = theCamera.Transform(centre);
		radius = theCamera.Transform(radius);
	}

	// Force a minimum radius
	if(radius<0.5) radius =0.5;

	// Calculate how may vertices will be needed to still make
	// the circle look round.
	int numVertices = int(radius * (6 / 10.0)+6);

	// The angle to rotate each vertex by to find the next vertex
	float angle = -6.285f / (numVertices-2);

	// Create an array of the vertex locations
	Vector2D* Points = new Vector2D[numVertices];

	// First vertex in the middle
	Points[0].XValue = centre.XValue;
	Points[0].YValue = centre.YValue;

	// Next vertex is directly below it, 2radius" pixels away
	Vector2D bottom = Vector2D(0, radius);
	// Find the remaining vertex locations - slow
	for(int i=1;i<numVertices;i++)
	{
		Points[i] =bottom.rotatedBy(-angle*i)+centre;
	}

	// Vertex buffer to hold the vertices
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	HRESULT err;
	// Create a vertex buffer using my format and default values
	err = m_lpD3DDevice->CreateVertexBuffer(sizeof(MYVERTEX)*numVertices,
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in DrawPointList");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 
	
	// Lock the buffer for writing
	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);  
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock the vertex buffer in DrawPointList");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Copy vertices into the buffer
	MYVERTEX* pVertices = (MYVERTEX*)pBuff;
	for( int i=0;i<numVertices;i++)
	{
		pVertices[i].x = Points[i].XValue;
		pVertices[i].y = Points[i].YValue;
		pVertices[i].z = 0.0f;
		pVertices[i].rhw=1.0f;
		pVertices[i].colour = colour;
	}

	// Writing finished - unlock
	vertexBuffer->Unlock();

	// Set my vertex format and a default stream source
	m_lpD3DDevice->SetFVF(MYFVF);
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Draw using a triangle fan
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0,numVertices-2);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw primitive in DrawPoint");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		delete[] Points;
		return FAILURE;
	}
	// Release the buffer
	vertexBuffer->Release();
	// Delete the array
	delete[] Points;
	return SUCCESS;
}	// FillCircle



//****************************************************************

// Fills a rectangle on the screen
ErrorType MyDrawEngine::FillRect(Rectangle2D destinationRect, unsigned int colour, float angle)
{

	if (m_CameraActive)
	{
		destinationRect = theCamera.Transform(destinationRect);
	}

	// Four corners
	Vector2D p1, p2, p3, p4;
	if (angle!=0.0f)		// Need to rotate due to angle
	{
		p1 = (destinationRect.GetBottomRight()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p2 = (destinationRect.GetTopRight()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p3 = (destinationRect.GetBottomLeft()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p4 = (destinationRect.GetTopLeft()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
	}
	else		// No rotation needed
	{
		p1 = destinationRect.GetBottomRight();
		p2 = destinationRect.GetTopRight();
		p3 = destinationRect.GetBottomLeft();
		p4 = destinationRect.GetTopLeft();
	}
	// Put corners into a vertex array - get the order right
	MYVERTEX Vertices[] = {
		{p1.XValue, p1.YValue, 0.0f, 1.0f, colour,},
		{p2.XValue, p2.YValue, 0.0f, 1.0f, colour,},
		{p3.XValue, p3.YValue, 0.0f, 1.0f, colour,},
		{p4.XValue, p4.YValue, 0.0f, 1.0f, colour,},
	};

	// Create a vertex buffer for drawing
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	HRESULT err;
	err = m_lpD3DDevice->CreateVertexBuffer(4*sizeof(MYVERTEX),
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in Fill Rect");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 

	// Lock buffer for writing
	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);  
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock the vertex buffer in FillRect");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Copy vertices into the buffer
	memcpy(pBuff, Vertices, sizeof(Vertices));

	// Writing complete - unlock
	vertexBuffer->Unlock();

	// Set my vertex format and a default stream
	m_lpD3DDevice->SetFVF(MYFVF);
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Draw using a triangle strip
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,2);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw primitive in FillRect");
    ErrorLogger::Writeln(err);
		return FAILURE;
	}
	// Release the buffer
	vertexBuffer->Release();

	return SUCCESS;
}

// ******************************************************************


ErrorType MyDrawEngine::BlendRect(Rectangle2D destinationRect, unsigned int colour, float transparency, float angle)
{

	if (m_CameraActive)
	{
		destinationRect = theCamera.Transform(destinationRect);
	}

	// Calculate transparency as an integer 0-255
	int trans = int(255 - 255 * transparency)%256;

	// Remove existing alpha component
	colour = colour & 16777215;

	// Insert requested alpha
	colour = colour + (trans<<24);

	// Four corners
	Vector2D p1, p2, p3, p4;
	if (angle!=0.0f)		// Rotate using angle
	{
		p1 = (destinationRect.GetTopRight()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p2 = (destinationRect.GetBottomRight()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p3 = (destinationRect.GetTopLeft()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
		p4 = (destinationRect.GetBottomLeft()-destinationRect.GetCentre()).rotatedBy(angle)+destinationRect.GetCentre();
	}
	else			// No need to rotate
	{
		p1 = destinationRect.GetTopRight();
		p2 = destinationRect.GetBottomRight();
		p3 = destinationRect.GetTopLeft();
		p4 = destinationRect.GetBottomLeft();
	}
	// Vertex array
	MYVERTEX Vertices[] = {
		{p1.XValue, p1.YValue, 0.0f, 1.0f, colour,},
		{p2.XValue, p2.YValue, 0.0f, 1.0f, colour,},
		{p3.XValue, p3.YValue, 0.0f, 1.0f, colour,},
		{p4.XValue, p4.YValue, 0.0f, 1.0f, colour,},
	};

	// Create a buffer
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	HRESULT err;
	err = m_lpD3DDevice->CreateVertexBuffer(4*sizeof(MYVERTEX),
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in BlendRect");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 

	// Lock buffer for writing
	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);  
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock the vertex buffer in BlendRect");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Copy vertices into the buffer

	memcpy(pBuff, Vertices, sizeof(Vertices));

	// Unlock, since writing is complete
	vertexBuffer->Unlock();

	// Set a vertex format and stream source
	m_lpD3DDevice->SetFVF(MYFVF);
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Enable alpha blending
	m_lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_lpD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Draw rectangle as a triangle strip - two triangles, starting at zero.
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,2);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw primitive in BlendRect");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}
	vertexBuffer->Release();

	return SUCCESS;
}	// Blend rectangle

// **************************************************************

// Draws a single dot
ErrorType MyDrawEngine::DrawPoint(Vector2D point, unsigned int colour)
{

	if (m_CameraActive)
	{
		point = theCamera.Transform(point);
	}
	// An "array" with one entry
	MYVERTEX Vertices[] = {
		{point.XValue, point.YValue, 0.0f, 1.0f, colour,},
	};

	// Create a vertex buffer
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	HRESULT err;
	err = m_lpD3DDevice->CreateVertexBuffer(sizeof(MYVERTEX),
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in DrawPoint");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 

	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);  
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock the vertex buffer in DrawPoint");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Copy vertices into the buffer

	memcpy(pBuff, Vertices, sizeof(Vertices));

	vertexBuffer->Unlock();

	// Set a vertex format and stream source
	m_lpD3DDevice->SetFVF(MYFVF);
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Draw a point list, with a single entry
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0,1);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw primitive in DrawPoint");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}
	vertexBuffer->Release();
	return SUCCESS;
}

// **************************************************************

ErrorType MyDrawEngine::DrawPointList(Vector2D points[], unsigned int colours[], unsigned int numPoints)
{
	if(numPoints<=0)
	{
		ErrorLogger::Writeln(L"Requested less than one point in DrawPointList.");
		return FAILURE;
	}

	// Create a vertex buffer
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	HRESULT err;
	err = m_lpD3DDevice->CreateVertexBuffer(sizeof(MYVERTEX)*numPoints,
							   0,
							   MYFVF,
							   D3DPOOL_MANAGED,
							   &vertexBuffer,
							   NULL);

	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to create a vertex buffer in DrawPointList");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	VOID* pBuff;		// Pointer to the locked buffer 

	// Lock buffer for writing
	err= vertexBuffer->Lock(0, 0, (void**)&pBuff, 0);  
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to lock the vertex buffer in DrawPointList");
		ErrorLogger::Writeln(err);
		return FAILURE;
	}

	// Copy vertices into the buffer
	MYVERTEX* pVertices = (MYVERTEX*)pBuff;
	for(unsigned int i=0;i<numPoints;i++)
	{
		Vector2D p = points[i];
		if (m_CameraActive)
		{
			p = theCamera.Transform(p);
		}
		pVertices[i].x = p.XValue;
		pVertices[i].y = p.YValue;
		pVertices[i].z = 0.0f;
		pVertices[i].rhw=1.0f;
		pVertices[i].colour = colours[i];
	}

	// Finished writing
	vertexBuffer->Unlock();

	// Set a vertex format and stream source
	m_lpD3DDevice->SetFVF(MYFVF);
	m_lpD3DDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MYVERTEX));

	// Draw using a point list
	err= m_lpD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0,numPoints);
	if(FAILED(err))
	{
		ErrorLogger::Writeln(L"Failed to draw primitive in DrawPoint");
		ErrorLogger::Writeln(err);
		vertexBuffer->Release();
		return FAILURE;
	}
	vertexBuffer->Release();
	return SUCCESS;
}	// DrawPointList




// **************************************************************
// Inner struct constructors
// **************************************************************

// MyFont constructor - just set pointer to NULL
MyDrawEngine::MyFont::MyFont()
{
	m_pFont = nullptr;
}

// **************************************************************

// MyPicture constructor -  set pointer to NULL and dimensions to zero
MyDrawEngine::MyPicture::MyPicture()
{
	lpTheTexture = nullptr;
	m_width =0;
	m_height=0;
}


