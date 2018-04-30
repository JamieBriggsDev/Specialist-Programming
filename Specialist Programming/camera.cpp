#pragma once

#include "mydrawengine.h"
#include "camera.h"

Camera::Camera()
{
	Reset();
}

Vector2D Camera::Transform(Vector2D objectPosition) const
{
  objectPosition = objectPosition - m_worldPosition;
  objectPosition.YValue = -objectPosition.YValue;
	objectPosition = m_zoom*(objectPosition) + m_screenCentre;

	return objectPosition;
}

Vector2D Camera::ReverseTransform(Vector2D screenPosition) const
{

	screenPosition = m_worldPosition + (screenPosition) / m_zoom;
  screenPosition.YValue = -screenPosition.YValue;
  screenPosition = screenPosition + m_worldPosition;
	return screenPosition;
}

float Camera::Transform(float size) const
{
	return size*m_zoom;
}

Circle2D Camera::Transform(Circle2D circle) const
{
	return Circle2D(Transform(circle.GetCentre()), Transform(circle.GetRadius()));
}

Rectangle2D Camera::Transform(Rectangle2D rectangle) const
{
	Rectangle2D answer;
	answer.PlaceAt(Transform(rectangle.GetBottomLeft()), Transform(rectangle.GetTopRight()));
	return answer;
}

Segment2D Camera::Transform(Segment2D segment) const
{
	Segment2D answer;
	answer.PlaceAt(Transform(segment.GetStart()), Transform(segment.GetEnd()));
	return answer;
}

void Camera::Reset()
{
	MyDrawEngine* pDrawEngine = MyDrawEngine::GetInstance();
	int height = 0;
	int width =0;

	if(pDrawEngine)			// Draw engine is initialised
	{
		height = pDrawEngine->GetScreenHeight();
		width = pDrawEngine->GetScreenWidth();	
	}
	else					// Use windows instead
	{
		width=GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
	}

	m_screenCentre.set(width/2.0f, height/2.0f);
	m_zoom = height/2000.0f;
}


void Camera::PlaceAt(Vector2D worldPosition)
{
	m_worldPosition = worldPosition;
}

void Camera::SetZoom(float zoom)
{
	m_zoom = zoom;
}