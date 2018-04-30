#pragma once

#include "vector2D.h"

class Camera
{
private:
	Vector2D m_screenCentre;			// Coordinates of the centre of the screen (width/2, height/2)
	Vector2D m_worldPosition;			// World coordinates for the centre of the screen			
	float m_zoom;						// Scale factor for drawing. 
public:
	// Sets the camera with an intial position at 0,0 with 
	// a scale to make the screen have a height of 2000 using world coordinates.
	// So the top of the screen is 1000. Bottom of the screen is -1000.
	// Left and right will depend on screen aspect
	Camera();

	// Returns the screen coordinates from the given world coordinates
	// using the current camera settings
	Vector2D Transform(Vector2D objectPosition) const;

	// Returns the world coordinates from the given screen coordinates
	// using the current camera settings
	Vector2D ReverseTransform(Vector2D screenPosition) const;

	// Returns the product of the size and the current scale
	float Transform(float size) const;

	// Returns a rectangle on the screen from the given world rectangle
	// using the current camera settings
	Circle2D Transform(Circle2D circle) const;

	// Returns a circle on the screen from the given world circle
	// using the current camera settings
	Rectangle2D Transform(Rectangle2D rectangle) const;

	// Returns a segment on the screen from the given world segment
	// using the current camera settings
	Segment2D Transform(Segment2D size) const;

	// Sets the camera with an intial position at 0,0 with 
	// a scale to make the screen have a height of 2000 using world coordinates.
	// So the top of the screen is 1000. Bottom of the screen is -1000.
	// Left and right will depend on screen aspect
	void Reset();

	// Places the centre of the camera to the specified position
	void PlaceAt(Vector2D worldPosition);

	// Sets the camera zoom
	void SetZoom(float zoom);
};

