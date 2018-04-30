#include "vector2D.h"
#include "math.h"


Vector2D::Vector2D(): XValue(0), YValue(0)
{
}

Vector2D::Vector2D(float x, float y): XValue(x), YValue(y)
{
}

Vector2D::Vector2D(const Vector2D& copyVector)
{
	XValue=copyVector.XValue;
	YValue=copyVector.YValue;
}

void Vector2D::set(float x, float y)
{
	XValue=x;
	YValue=y;
}

void Vector2D::set(const Vector2D& copyVector)
{
	XValue=copyVector.XValue;
	YValue=copyVector.YValue;
}

float Vector2D::magnitude() const
{
	return sqrt(XValue*XValue + YValue*YValue);
}

	// Returns the square of the magnitude of the vector
	// (hence avoiding a square root.)
float Vector2D::magnitudeSquared() const
{
	return (XValue*XValue + YValue*YValue);
}


Vector2D Vector2D::unitVector() const
{
	float divideBy = magnitude();

	if(divideBy==0)
	{
		return Vector2D(0,0);
	}
		else
	return Vector2D(XValue/divideBy, YValue/divideBy);
}

Vector2D Vector2D::operator+(const Vector2D& addVector) const
{
	return Vector2D (XValue+addVector.XValue,YValue+addVector.YValue);
}

void Vector2D::operator+=(const Vector2D& addVector)
{
	XValue+=addVector.XValue;
	YValue+=addVector.YValue;
}

void Vector2D::operator-=(const Vector2D& addVector)
{
	XValue-=addVector.XValue;
	YValue-=addVector.YValue;
}

void Vector2D::operator/=(float divisor)
{
	XValue/=divisor;
	YValue/=divisor;
}

void Vector2D::operator*=(float multiplier)
{
	XValue*=multiplier;
	YValue*=multiplier;
}

void Vector2D::operator*=(int multiplier)
{
	XValue*=multiplier;
	YValue*=multiplier;
}


Vector2D Vector2D::operator-(const Vector2D& addVector) const
{
	return Vector2D (XValue-addVector.XValue,YValue-addVector.YValue);
}

Vector2D Vector2D::operator -() const
{
	return Vector2D (-this->XValue,-this->YValue);
}

Vector2D Vector2D::operator*(float multiplier) const
{
	return Vector2D(XValue*multiplier,YValue*multiplier);
}

Vector2D Vector2D::operator*(int multiplier) const
{
	return Vector2D(XValue*multiplier,YValue*multiplier);
}

float Vector2D::operator*(const Vector2D& otherVector) const
{
	return XValue*otherVector.XValue+YValue*otherVector.YValue;
}

bool Vector2D::operator==(const Vector2D& otherVector) const
{
	return (XValue == otherVector.XValue && YValue == otherVector.YValue);
}

Vector2D Vector2D::operator/(float divisor) const
{
	return Vector2D(XValue/divisor,YValue/divisor);
}

float Vector2D::angle() const
// Working in radians
{
	float answer;

	answer=atan2(XValue, YValue);

	while(answer<0) answer+=6.28319f;
	while(answer>=6.28319f) answer-=6.28319f;

	return answer;
}

void Vector2D::setBearing(float angle, float magnitude)
{
// Working in radians
	XValue = magnitude*sin(angle);
	YValue = magnitude*cos(angle);
}

bool Vector2D::isParallelTo(const Vector2D &otherVector) const
{
	if(this->unitVector() == otherVector.unitVector()) return true;
	if(this->unitVector() == -otherVector.unitVector()) return true;
	else return false;
}

bool Vector2D::isPerpendicularTo(const Vector2D &otherVector) const
{
	return((*this*otherVector)==0);
}

Vector2D Vector2D::perpendicularVector() const
{
	return Vector2D(this->YValue, -this->XValue);
}

Vector2D Vector2D::rotatedBy(float angle) const
{
	float s = sin(angle);
	float c = cos(angle);
	return Vector2D(this->XValue * c - this->YValue*s, this->XValue * s + this->YValue * c);
	
}

Vector2D operator*(float multiplier, Vector2D v) 
{
	return v*multiplier;
}