#include "Shapes.h"
#include <limits.h>
#include <iostream>
#include <cmath>
#include "Renderer.h"
using namespace std;

/// 21/09/2017		Corrected errors in
// Vector2D Rectangle2D::CollisionNormal(const Circle2D& other) const


// ***************************************************************
// Member functions for IShape ***********************************
// ***************************************************************

// Virtual destructor for usual reasons
IShape2D::~IShape2D()
{

}


// ***************************************************************
// Member functions for Point2D **********************************
// ***************************************************************

Point2D::Point2D()
{
	this->mPosition.set(0,0);
}

Point2D::Point2D(float x, float y)
{
	this->mPosition.set(x,y);
}

Point2D::Point2D(const Vector2D &copy)
{
	this->mPosition=copy;
}

void Point2D::PlaceAt(const Vector2D &position)
{
	this->mPosition = position;
}

Vector2D Point2D::GetPosition() const
{
	return mPosition;
}

bool Point2D::Intersects(const Point2D &other) const
{
	return(this->mPosition == other.mPosition);
}

float Point2D::Distance(const Point2D &other) const
{
	return (this->mPosition - other.mPosition).magnitude();
}

Vector2D Point2D::Intersection(const Point2D &other) const
{	
	// Uhh... Closest point on a point IS the point.
	return this->mPosition;
}

bool Point2D::Intersects(const Segment2D &other) const
{
	double t1 =other.GetTFromX(this->mPosition.XValue);
	double t2 =other.GetTFromY(this->mPosition.YValue);
	if(t2==t1)	// On LINE
	{
		return (t2>=0 && t2<1.0);	// Within ends of segment
	}
	else
		return false;
}

bool Point2D::Intersects(const IShape2D& other) const
{
	// Is it a rectangle?
	const Rectangle2D* pRect = nullptr;
	pRect = dynamic_cast<const Rectangle2D*> (&other);
	if(pRect)
		return pRect->Intersects(*this);

	// Is it a circle?
	const Circle2D* pCirc = nullptr;
	pCirc = dynamic_cast<const Circle2D*> (&other);
	if(pCirc)
		return pCirc->Intersects(*this);

	// Is it a segment?
	const Segment2D* pSeg = nullptr;
	pSeg = dynamic_cast<const Segment2D*> (&other);
	if(pSeg)
		return pSeg->Intersects(*this);

	// Is it a point?
	const Point2D* ppt = nullptr;
	ppt = dynamic_cast<const Point2D*> (&other);
	if(ppt)
		return ppt->Intersects(*this);

	// Undefined shape
	return false;
}

float Point2D::Distance(const Segment2D &other) const
{
	// Project the point onto the line and find the parameter, t
	// using parametric equation.
	// Hmm. Should be ((C - S) . ( E - S ) ) / L^2

	return other.Distance(*this);
}

Vector2D Point2D::Intersection(const Segment2D &other) const
{
	return mPosition;
}

bool Point2D::Intersects(const Circle2D &other) const
{

	return ((mPosition-other.GetCentre()).magnitude()<other.GetRadius());
}

float Point2D::Distance(const Circle2D &other) const
{
	return ( (mPosition-other.GetCentre()).magnitude()-other.GetRadius() );
}

Vector2D Point2D::Intersection(const Circle2D &other) const
{
	// Uhh... Closest point on a point IS the point.
	return this->mPosition;
}

bool Point2D::Intersects(const Rectangle2D &other) const
{
	if( mPosition.XValue < other.GetCorner1().XValue
		|| mPosition.XValue > other.GetCorner2().XValue
		|| mPosition.YValue < other.GetCorner1().YValue
		|| mPosition.YValue > other.GetCorner2().YValue)
		return false;
	else
		return true;
}

float Point2D::Distance(const Rectangle2D &other) const
{
	// 9 possibilities
	if(mPosition.XValue< other.GetCorner1().XValue)
	{	// Left side
		if(mPosition.YValue<other.GetCorner1().YValue)
		{	// Top left
			return (mPosition-other.GetCorner1()).magnitude();
		}
		else if(mPosition.YValue>=other.GetCorner2().YValue)
		{	// Bottom left
			Vector2D bl(other.GetCorner1().XValue, other.GetCorner2().YValue);
			return (mPosition-bl).magnitude();
		}
		else
		{	// Middle left
			return other.GetCorner1().XValue-mPosition.XValue;
		}

	}
	else if(mPosition.XValue >= other.GetCorner2().XValue)
	{	// Right side
		if(mPosition.YValue<other.GetCorner1().YValue)
		{	// Top right
			Vector2D tr(other.GetCorner2().XValue, other.GetCorner1().YValue);
			return (mPosition-tr).magnitude();
		}
		else if(mPosition.YValue>=other.GetCorner2().YValue)
		{	// Bottom right
			return (mPosition-other.GetCorner2()).magnitude();
		}
		else
		{	// Middle right
			return mPosition.XValue-other.GetCorner2().XValue;
		}
	}
	else
	{	// Centre line
		if(mPosition.YValue<other.GetCorner1().YValue)
		{	// Top 
			return other.GetCorner1().YValue-mPosition.YValue;
		}
		else if(mPosition.YValue>=other.GetCorner2().YValue)
		{	// Bottom
			return mPosition.YValue-other.GetCorner2().YValue;
		}
		else
		{	// Inside rectangle  get distances to edges
			float t,b,l,r;
			t=mPosition.YValue-other.GetCorner1().YValue;
			b=other.GetCorner2().YValue-mPosition.YValue;
			l=mPosition.XValue-other.GetCorner1().XValue;
			r=other.GetCorner2().XValue-mPosition.XValue;

			float smallest = t;
			if(smallest>b)
				smallest =b;
			if(smallest>l)
				smallest = l;
			if(smallest>r)
				smallest =r;

			return -smallest;		// Negative, as inside the rectangle
		}
	}
}

Vector2D Point2D::Intersection(const Rectangle2D &other) const
{
	// Uhh... Closest point on a point IS the point.
	return this->mPosition;
}

// ****************************************************************
// Member functions for Segment2D *********************************
// ****************************************************************

Segment2D::Segment2D(): mStart(0,0), mEnd(0,0)
{

}

void Segment2D::PlaceAt(const Vector2D &start, const Vector2D &end)
{
	this->mStart=start;
	this->mEnd=end;
}

Vector2D Segment2D::GetStart() const
{
	return mStart;
}

Vector2D Segment2D::GetEnd() const
{
	return mEnd;
}

void Segment2D::ClipTo(float minT, float maxT)
{
	Vector2D end;
	end.set(mStart.XValue + maxT* (mEnd.XValue-mStart.XValue) , mStart.YValue + maxT* (mEnd.YValue-mStart.YValue) );

	SetStartTo(minT);
	mEnd = end;
}

float Segment2D::GetLength() const
{
	return (mStart-mEnd).magnitude();
}

bool Segment2D::Intersects(const Point2D &other) const
{

	return other.Intersects(*this);
}

float Segment2D::Distance(const Point2D &other) const
{

	return (other.GetPosition() - Intersection(other) ).magnitude();
}

Vector2D Segment2D::Intersection(const Point2D &other) const
{
	// Special case - line with length of zero
	if(mEnd==mStart) 
		return mEnd;
	else    // If line length is not zero
	{
		// Get unit direction vector from Start to End of the line
		Vector2D Direction = (mEnd - mStart).unitVector();

		// Projection of the point onto the line
		float dProjection = (other.GetPosition() - mStart)*Direction;

		float t = dProjection/GetLength();
			Vector2D vec = mStart + Direction*dProjection;
			vec=vec*2;
		if(t<0) 
			return mStart;
		else if(t>1.0)
			return mEnd;
		else
		{

			return mStart + Direction*dProjection;
		}
	}	// End if line length is not zero
}

bool Segment2D::Intersects(const IShape2D& other) const
{
	// Is it a rectangle?
	const Rectangle2D* pRect = nullptr;
	pRect = dynamic_cast<const Rectangle2D*> (&other);
	if(pRect)
		return pRect->Intersects(*this);

	// Is it a circle?
	const Circle2D* pCirc = nullptr;
	pCirc = dynamic_cast<const Circle2D*> (&other);
	if(pCirc)
		return pCirc->Intersects(*this);

	// Is it a segment?
	const Segment2D* pSeg = nullptr;
	pSeg = dynamic_cast<const Segment2D*> (&other);
	if(pSeg)
		return pSeg->Intersects(*this);

	// Is it a point?
	const Point2D* ppt = nullptr;
	ppt = dynamic_cast<const Point2D*> (&other);
	if(ppt)
		return ppt->Intersects(*this);

	// Undefined shape
	return false;
}

bool Segment2D::Intersects(const Segment2D &other) const
{
	// Check that lines are not parallel

	if(!ParallelTo(other))
	{
		//Find the t (parametric equation parameter) of the
		// point of intersection on this line

		// Find t. Big equation from
		// http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
		float t1 =(   (other.mEnd.XValue - other.mStart.XValue)*(this->mStart.YValue - other.mStart.YValue)
			- (other.mEnd.YValue - other.mStart.YValue) * (this->mStart.XValue - other.mStart.XValue)    )
			/ (    (other.mEnd.YValue - other.mStart.YValue) * (this->mEnd.XValue - this->mStart.XValue)
			- (other.mEnd.XValue - other.mStart.XValue) * (this->mEnd.YValue - this->mStart.YValue)  );

		Vector2D p1(this->PointFromT(t1));		// Point of intersection

		float t2 = other.GetTFromX(p1.XValue);

		if(t1>=0 && t1<1.0 && t2>=0 && t2<1.0)	// Intersection
			return true;
		else
			return false;
	}
	else		// Parallel lines
		return false;
}

float Segment2D::Distance(const Segment2D &other) const
{
	// Check that lines are not parallel

	if(!ParallelTo(other))
	{
		//Find the t (parametric equation parameter) of the
		// point of intersection on this line

		// Find t. Big equation from
		// http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
		float t1 =(   (other.mEnd.XValue - other.mStart.XValue)*(this->mStart.YValue - other.mStart.YValue)
			- (other.mEnd.YValue - other.mStart.YValue) * (this->mStart.XValue - other.mStart.XValue)    )
			/ (    (other.mEnd.YValue - other.mStart.YValue) * (this->mEnd.XValue - this->mStart.XValue)
			- (other.mEnd.XValue - other.mStart.XValue) * (this->mEnd.YValue - this->mStart.YValue)  );

		Vector2D p1(this->PointFromT(t1));		// Point of intersection

		float t2 = other.GetTFromX(p1.XValue);

		if(t1>=0 && t1<1.0 && t2>=0 && t2<1.0)	// Intersection
			return 0;							// Return zero separation
	}
		
	// Intersection not on segments or lines parallel
	float distances[4];				// Between four points
	distances[0] = other.Distance(Point2D(mStart));
	distances[1] = other.Distance(Point2D(mEnd));
	distances[2] = this->Distance(Point2D(other.mStart));
	distances[3] = this->Distance(Point2D(other.mEnd));

	// Find smallest
	int smallest = 0;
	for(int i=1;i<4;i++)
	{
		if(distances[i]<distances[smallest])
		smallest =i;
	}

	return distances[smallest];
}

bool Segment2D::ParallelTo(const Segment2D &other) const
{
	return ParallelTo(other.mEnd-other.mStart);
}

bool Segment2D::ParallelTo(const Vector2D &other) const
{
	Vector2D direction = mEnd -mStart;
	double dotProduct = direction.unitVector()*other.unitVector();
	if(dotProduct<1.00001 && dotProduct>0.99999)
		return true;
	if(dotProduct>-1.00001 && dotProduct<-0.99999)
		return true;
	else return false;
}

Vector2D Segment2D::Intersection(const Segment2D &other) const
{
	// Check that lines are not parallel

	if(!ParallelTo(other))
	{
		//Find the t (parametric equation parameter) of the
		// point of intersection on this line

		// Find t. Big equation from
		// http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
		float t1 =(   (other.mEnd.XValue - other.mStart.XValue)*(this->mStart.YValue - other.mStart.YValue)
			- (other.mEnd.YValue - other.mStart.YValue) * (this->mStart.XValue - other.mStart.XValue)    )
			/ (    (other.mEnd.YValue - other.mStart.YValue) * (this->mEnd.XValue - this->mStart.XValue)
			- (other.mEnd.XValue - other.mStart.XValue) * (this->mEnd.YValue - this->mStart.YValue)  );

		Vector2D p1(this->PointFromT(t1));		// Point of intersection

		float t2 = other.GetTFromX(p1.XValue);

		if(t1>=0 && t1<1.0 && t2>=0 && t2<1.0)	// Intersection
			return p1;							// Return the point
	}
		
	// Intersection not on segments or lines parallel
	double distances[4];				// Between four points
	distances[0] = other.Distance(Point2D(mStart));
	distances[1] = other.Distance(Point2D(mEnd));
	distances[2] = this->Distance(Point2D(other.mStart));
	distances[3] = this->Distance(Point2D(other.mEnd));

	// Find smallest
	int smallest = 0;
	for(int i=1;i<4;i++)
	{
		if(distances[i]<distances[smallest])
		smallest =i;
	}

	if(smallest == 0)			// my start closest to other segment
		return mStart;
	else if(smallest == 1)		// My end closest to other segment
		return mEnd;
	else if(smallest == 2)		// Other line's start closest to my segment
		return Intersection(Point2D(other.mStart));
	else						// other line's end closest to my segment
		return Intersection(Point2D(other.mEnd));
	
}

bool Segment2D::Intersects(const Circle2D &other) const
{
	return (Distance( other )<0);
}

float Segment2D::Distance(const Circle2D &other) const
{
	// Get distance to the centre and subtract the radius
	return (Distance( Point2D(other.GetCentre()))-other.GetRadius());;
}

Vector2D Segment2D::Intersection(const Circle2D &other) const
{

	return Intersection(Point2D(other.mCentre));
}

bool Segment2D::Intersects(const Rectangle2D &other) const
{
	if(GetLength()==0)	// Special case
	{
		return(other.Intersects(Point2D(mEnd)));
	}
	Segment2D clipped = other.Clip(*this);
	return (clipped.GetLength()>0);
}

float Segment2D::Distance(const Rectangle2D &other) const
{
	// Slow
	if(Intersects(other))
		return 0.0;
	return Distance(Point2D(Intersection(other)));
}

Vector2D Segment2D::FirstIntersection(const Rectangle2D &other) const
{
	Segment2D clipped = other.Clip(*this);
	if(clipped.GetLength()==0)	// No intersection
		return mEnd;
	else
		return clipped.mStart;
}




Vector2D Segment2D::Intersection(const Rectangle2D &other) const
{
	Segment2D clipped = other.Clip(*this);
	if(clipped.GetLength()>0)	// If intersects
		return (clipped.PointFromT(0.5)); // return mid-point of the clipped line

	// What if end of line is closest to an edge, not a corner?
	else						// Not intersects
	{
		double distances[6];	// From each corner to the line and
								// each line edge to the rectangle

		// Get distances - this is gonna be slow
		distances[0] = Distance(Point2D(other.GetTopLeft()));
		distances[1] = Distance(Point2D(other.GetTopRight()));
		distances[2] = Distance(Point2D(other.GetBottomLeft()));
		distances[3] = Distance(Point2D(other.GetBottomRight()));
		distances[4] = Point2D(mStart).Distance(other);
		distances[5] = Point2D(mEnd).Distance(other);

		// Find smallest
		int smallest = 0;
		for(int i=1;i<6;i++)
		{
			if(distances[i]<distances[smallest])
				smallest =1;
		}

		if(smallest == 0)	// Top Left
		{
			return Intersection(Point2D(other.GetTopLeft()));
		}
		else if(smallest == 1)	// Top right
		{
			return Intersection(Point2D(other.GetTopRight()));
		}
		else if(smallest == 2)	// Bottom left
		{
			return Intersection(Point2D(other.GetBottomLeft()));
		}
		else if(smallest == 3)	// Bottom right
		{
			return Intersection(Point2D(other.GetBottomRight()));
		}
		else if(smallest == 4)	// Start of line closest to an edge
		{
			return Point2D(mStart).Intersection(other);
		}
		else 	//End of line closest to an edge
		{
			return Point2D(mEnd).Intersection(other);
		}
	}
}

float Segment2D::GetTFromX(float x) const
{
	if(mEnd.XValue-mStart.XValue == 0.0)
	{
		if(x==mEnd.XValue)
			return 1.0;
		else
			return 0;
	}
	else
		return (x-mStart.XValue)/(mEnd.XValue-mStart.XValue);
}

float Segment2D::GetTFromY(float y) const
{
	if(mEnd.YValue-mStart.YValue == 0.0)
	{
		if(y==mEnd.YValue)
			return 1.0;
		else
			return 0;
	}
	else
		return (y-mStart.YValue)/(mEnd.YValue-mStart.YValue);
}

Vector2D Segment2D::PointFromT(float t) const
{
	return (mStart+(mEnd-mStart)*t);
}

void Segment2D::SetStartTo(float t)
{
	mStart.set(mStart.XValue + t* (mEnd.XValue-mStart.XValue) , mStart.YValue + t* (mEnd.YValue-mStart.YValue) );
}

void Segment2D::SetEndTo(float t)
{
	mEnd.set(mStart.XValue + t* (mEnd.XValue-mStart.XValue) , mStart.YValue + t* (mEnd.YValue-mStart.YValue) );
}

// *********************************************************************
// Member functions for Circle2D
// *********************************************************************

Circle2D::Circle2D(): mdRadius(0)
{
	this->mCentre.set(0,0);
}

Circle2D::Circle2D(const Vector2D &centre, float radius)
{
	this->mCentre=centre;
	if(radius>=0)
		this->mdRadius=radius;
	else
		this->mdRadius=-radius;
}


void Circle2D::PlaceAt(const Vector2D &centre, float radius)
{
	this->mCentre=centre;
	if(radius>=0)
		this->mdRadius=radius;
	else
		this->mdRadius=-radius;
}


Vector2D Circle2D::GetCentre() const
{

	return mCentre;
}

float Circle2D::GetRadius() const
{

	return mdRadius;
}
float Circle2D::GetArea() const
{
	return 3.141593f * mdRadius*mdRadius;
}

bool Circle2D::Intersects(const Point2D &other) const
{
	return((other.GetPosition()-mCentre).magnitude()<mdRadius);
}

float Circle2D::Distance(const Point2D &other) const
{
	return((other.GetPosition()-mCentre).magnitude()-mdRadius);
}

Vector2D Circle2D::Intersection(const Point2D &other) const
{
	Vector2D toPoint = other.mPosition - mCentre;
	toPoint = toPoint.unitVector();
	return mCentre+toPoint*mdRadius;
}

bool Circle2D::Intersects(const Segment2D &other) const
{
	return other.Intersects(*this);
}

bool Circle2D::Intersects(const IShape2D& other) const
{
	// Is it a rectangle?
	const Rectangle2D* pRect = nullptr;
	pRect = dynamic_cast<const Rectangle2D*> (&other);
	if(pRect)
		return pRect->Intersects(*this);

	// Is it a circle?
	const Circle2D* pCirc = nullptr;
	pCirc = dynamic_cast<const Circle2D*> (&other);
	if(pCirc)
		return pCirc->Intersects(*this);

	// Is it a segment?
	const Segment2D* pSeg = nullptr;
	pSeg = dynamic_cast<const Segment2D*> (&other);
	if(pSeg)
		return pSeg->Intersects(*this);

	// Is it a point?
	const Point2D* ppt = nullptr;
	ppt = dynamic_cast<const Point2D*> (&other);
	if(ppt)
		return ppt->Intersects(*this);

	// Undefined shape
	return false;
}

float Circle2D::Distance(const Segment2D &other) const
{
	return Distance(other.Intersection(mCentre));
}

Vector2D Circle2D::Intersection(const Segment2D &other) const
{
	return Intersection(other.Intersection(*this));	// Implicit cast to Point2D
}

bool Circle2D::Intersects(const Circle2D &other) const
{
	return( (other.GetCentre()-mCentre).magnitude() < (mdRadius+other.GetRadius()) );
}

float Circle2D::Distance(const Circle2D &other) const
{
	return( (other.GetCentre()-mCentre).magnitude() - (mdRadius+other.GetRadius()) );
}

Vector2D Circle2D::Intersection(const Circle2D &other) const
{
	return Intersection(Point2D(other.mCentre));
}

bool Circle2D::Intersects(const Rectangle2D &other) const
{
	Point2D c(this->mCentre);
	if(c.Distance(other)<this->mdRadius)
		return true;
	else
		return false;
}

float Circle2D::Distance(const Rectangle2D &other) const
{
	Point2D c(this->mCentre);
	return(c.Distance(other)-mdRadius);
}

Vector2D Circle2D::Intersection(const Rectangle2D &other) const
{
	Vector2D otherEdge = other.Intersection(*this)-mCentre;
	return (mCentre + otherEdge.unitVector()*mdRadius);
}

Vector2D Circle2D::CollisionNormal(const Point2D& other) const
{
	return (other.mPosition-mCentre).unitVector();
}

Vector2D Circle2D::CollisionNormal(const Circle2D& other) const
{
	return (other.mCentre-mCentre).unitVector();
}

Vector2D Circle2D::CollisionNormal(const Rectangle2D& other) const
{
	return (other.Intersection(*this)-mCentre).unitVector();
}

Vector2D Circle2D::CollisionNormal(const Segment2D& other) const
{
	return (other.Intersection(*this)-mCentre).unitVector();
}


// ********************************************************************
// Member functions for Rectangle2D
// ********************************************************************

Rectangle2D::Rectangle2D()
{
	this->mCorner1.set(0,0);
	this->mCorner2.set(0,0);
}

void Rectangle2D::PlaceAt(float top, float left, float bottom, float right)
{
	PlaceAt(Vector2D(left,top), Vector2D(right, bottom));
}

Vector2D Rectangle2D::GetCentre() const
{
	return (mCorner1+mCorner2)/2;
}

void Rectangle2D::PlaceAt(Vector2D bottomLeft, Vector2D topRight)
{
	// Verify corner 1 is bottom left and corner 2 is top right
	float top, left, bottom, right;
	if(bottomLeft.XValue<=topRight.XValue)
	{
		left = bottomLeft.XValue;
		right = topRight.XValue;
	}
	else
	{
		right = bottomLeft.XValue;
		left = topRight.XValue;
	}

	if(bottomLeft.YValue<=topRight.YValue)
	{
		top = bottomLeft.YValue;
		bottom = topRight.YValue;
	}
	else
	{
		bottom = bottomLeft.YValue;
		top = topRight.YValue;
	}
	
	mCorner1.set(left, top);
	mCorner2.set(right, bottom);
}

Vector2D Rectangle2D::GetCorner1() const
{
	return mCorner1;
}

Vector2D Rectangle2D::GetCorner2() const
{
	return mCorner2;
}

Vector2D Rectangle2D::GetBottomLeft() const
{
	return mCorner1;
}

Vector2D Rectangle2D::GetTopRight() const
{
	return mCorner2;
}

Vector2D Rectangle2D::GetTopLeft() const
{
	return Vector2D(mCorner1.XValue, mCorner2.YValue);
}

Vector2D Rectangle2D::GetBottomRight() const
{
	return Vector2D(mCorner2.XValue, mCorner1.YValue);
}

float Rectangle2D::GetArea() const
{
	float h = mCorner1.XValue - mCorner2.XValue;
	float w = mCorner1.YValue - mCorner2.YValue;

	float a = h* w;

	if(a<0) return -a;
	else return a;
}

bool Rectangle2D::Intersects(const Point2D &other) const
{
	return other.Intersects(*this);
}

Vector2D Rectangle2D::CollisionNormal(const Point2D &other)const
{
	Vector2D answer;
		double t,b,l,r;
		t=other.GetPosition().YValue-GetCorner1().YValue;
		b=GetCorner2().YValue-other.GetPosition().YValue;
		l=other.GetPosition().XValue-GetCorner1().XValue;
		r=GetCorner2().XValue-other.GetPosition().XValue;

		double smallest = t;
		{
			answer.set(0,1);
		}
		if(smallest>b)
		{
			answer.set(0,-1);
			smallest =b;
		}
		if(smallest>l)
		{
			answer.set(-1,0);
			smallest = l;
		}
		if(smallest>r)
		{
			answer.set(1,0);
			smallest =r;
		}
		if(smallest<0)
		{
			answer.set(0,0);
		}
		return answer;
}

float Rectangle2D::Distance(const Point2D &other) const
{
		
	return other.Distance(*this);
}

Vector2D Rectangle2D::Intersection(const Point2D &other) const
{
	// 9 possibilities
	if(other.mPosition.XValue< mCorner1.XValue)
	{	// Left side
		if(other.mPosition.YValue<mCorner1.YValue)
		{	// Bottom left
			return GetBottomLeft();
		}
		else if(other.mPosition.YValue>=GetCorner2().YValue)
		{	// Top left
			return GetTopLeft();
		}
		else
		{	// Middle left
			return Vector2D(mCorner1.XValue, other.mPosition.YValue);
		}

	}
	else if(other.mPosition.XValue >= mCorner2.XValue)
	{	// Right side
		if(other.mPosition.YValue<mCorner1.YValue)
		{	// Bottom right
			return GetBottomRight();
		}
		else if(other.mPosition.YValue>=mCorner2.YValue)
		{	// Top right
			return GetTopRight();
		}
		else
		{	// Middle right
			return Vector2D(mCorner2.XValue, other.mPosition.YValue);
		}
	}
	else
	{	// Centre line
		if(other.mPosition.YValue<mCorner1.YValue)
		{	// Top 
			return Vector2D( other.mPosition.XValue, mCorner1.YValue);
		}
		else if(other.mPosition.YValue>=mCorner2.YValue)
		{	// Bottom
			return Vector2D( other.mPosition.XValue, mCorner2.YValue);
		}
		else
		{	// Inside rectangle  get distances to edges
			double t,b,l,r;
			t=other.mPosition.YValue-mCorner1.YValue;
			b=mCorner2.YValue-other.mPosition.YValue;
			l=other.mPosition.XValue-mCorner1.XValue;
			r=mCorner2.XValue-other.mPosition.XValue;

			double smallest = t;
			if(smallest>b)
				smallest =b;
			if(smallest>l)
				smallest = l;
			if(smallest>r)
				smallest =r;

			if(smallest == t)
				return Vector2D( other.mPosition.XValue, mCorner1.YValue);
			else if(smallest == b)
				return Vector2D( other.mPosition.XValue, mCorner2.YValue);
			else if(smallest == l)
				return Vector2D(mCorner1.XValue, other.mPosition.YValue);
			else		// Smallest ==r
				return Vector2D(mCorner2.XValue, other.mPosition.YValue);
		}
	}

}

bool Rectangle2D::Intersects(const Segment2D &other) const
{
	return other.Intersects(*this);
}

bool Rectangle2D::Intersects(const IShape2D& other) const
{
	// Is it a rectangle?
	const Rectangle2D* pRect = nullptr;
	pRect = dynamic_cast<const Rectangle2D*> (&other);
	if(pRect)
		return pRect->Intersects(*this);

	// Is it a circle?
	const Circle2D* pCirc = nullptr;
	pCirc = dynamic_cast<const Circle2D*> (&other);
	if(pCirc)
		return pCirc->Intersects(*this);

	// Is it a segment?
	const Segment2D* pSeg = nullptr;
	pSeg = dynamic_cast<const Segment2D*> (&other);
	if(pSeg)
		return pSeg->Intersects(*this);

	// Is it a point?
	const Point2D* ppt = nullptr;
	ppt = dynamic_cast<const Point2D*> (&other);
	if(ppt)
		return ppt->Intersects(*this);

	// Undefined shape
	return false;
}



Vector2D Rectangle2D::Intersection(const Segment2D &other) const
{
	// Get the intersection point on the other segment
	// and return the point closest to it
	return Intersection(other.Intersection(*this));
}

bool Rectangle2D::Intersects(const Circle2D &other) const
{
	return other.Intersects(*this);
}

float Rectangle2D::Distance(const Circle2D &other) const
{

	return other.Distance(*this);
}

Vector2D Rectangle2D::Intersection(const Circle2D &other) const
{
		// Return the point closest to the centre of the circle
	return Intersection(other.mCentre);
}

Segment2D Rectangle2D::Clip(Segment2D other) const
{
	
	float top =this->mCorner1.YValue;
	float bottom =this->mCorner2.YValue;
	float left =this->mCorner1.XValue;
	float right =this->mCorner2.XValue;
	Vector2D p1 = other.GetStart();
	Vector2D p2 = other.GetEnd();

	if(p1.XValue<left && p2.XValue<left)
		return Segment2D();				// Both ends left of rectangle - empty segment
	if(p1.XValue>=right && p2.XValue>=right)
		return Segment2D();				// Both ends right of rectangle - empty segment
	if(p1.YValue>=bottom && p2.YValue>=bottom)
		return Segment2D();				// Both ends below rectangle - empty segment
	if(p1.YValue<top && p2.YValue<top)
		return Segment2D();				// Both ends above rectangle - empty segment

	if( this->Intersects(Point2D(p1)) && this->Intersects(Point2D(p2)) )
		return other;					// Entirely inside rectangle - return entire segment

	// Time to get serious

	float maxT=1.0;
	float minT=0.0;
	float tempT=0;

	// Check for clip at the left edge
	if(p1.XValue<left && p2.XValue>=left)
	{
		minT = other.GetTFromX(left);
	}

	if(p2.XValue<left && p1.XValue>=left)
	{
		maxT = other.GetTFromX(left);
	}

	// Check for clip at the right edge
	if(p1.XValue<=right && p2.XValue>=right)
	{
		tempT = other.GetTFromX(right);
		if(tempT<maxT) maxT=tempT;

	}

	if(p2.XValue<=right && p1.XValue>=right)
	{
		tempT = other.GetTFromX(right);
		if(tempT>minT) minT=tempT;

	}

	// Check for clip at the top edge
	if(p1.YValue<top && p2.YValue>=top)
	{
		tempT = other.GetTFromY(top);
		if(tempT>minT) minT = tempT;
	}

	if(p2.YValue<top && p1.YValue>=top)
	{
		tempT = other.GetTFromY(top);
		if(tempT<maxT) maxT=tempT;

	}

	// Check for clip at bottom edge
	if(p1.YValue<=bottom && p2.YValue>=bottom)
	{
		tempT = other.GetTFromY(bottom);
		if(tempT<maxT) maxT=tempT;

	}

	if(p2.YValue<=bottom && p1.YValue>=bottom)
	{
		tempT = other.GetTFromY(bottom);
		if(tempT>minT) minT=tempT;
		cout << "b";
	}

	// Check for mint and maxt overlap - means you missed the rectangle
	if(minT > maxT)
	{
		other.PlaceAt(Vector2D(0,0), Vector2D(0,0));
	}
	else
	{
		other.ClipTo(minT, maxT);
	}

	return other;
}

bool Rectangle2D::Intersects(const Rectangle2D &other) const
{
	if(other.mCorner1.XValue >= this->mCorner2.XValue
		|| other.mCorner2.XValue <= this->mCorner1.XValue
		|| other.mCorner1.YValue >= this->mCorner2.YValue
		|| other.mCorner2.YValue <= this->mCorner1.YValue)
		return false;
	else
		return true;
}

float Rectangle2D::Distance(const Rectangle2D &other) const
{
	Rectangle2D clipRect = other;		// A rectangle that describes the overlap
										// between the two rectangles
	// Clip the left side
	if(clipRect.mCorner1.XValue<this->mCorner1.XValue)
		clipRect.mCorner1.XValue=this->mCorner1.XValue;

	// Clip the right side
	if(clipRect.mCorner2.XValue>this->mCorner2.XValue)
		clipRect.mCorner2.XValue=this->mCorner2.XValue;

	// Clip the top side
	if(clipRect.mCorner1.YValue<this->mCorner1.YValue)
		clipRect.mCorner1.YValue=this->mCorner1.YValue;

	// Clip the bottom side
	if(clipRect.mCorner2.YValue>this->mCorner2.YValue)
		clipRect.mCorner2.YValue=this->mCorner2.YValue;

	// Check to see if the clip rect has been inverted horizontally
	if(clipRect.mCorner1.XValue< clipRect.mCorner2.XValue)
	{
		// If it HASN'T, flatten the rectangle horizontally
		clipRect.mCorner1.XValue = clipRect.mCorner2.XValue;
	}
	// Check to see if the clip rect has been inverted vertically
	if(clipRect.mCorner1.YValue< clipRect.mCorner2.YValue)
	{
		// If it HASN'T, flatten the rectangle vertically
		clipRect.mCorner1.YValue = clipRect.mCorner2.YValue;
	}

	// Return the diagonal of the resultant rectangle (this will be zero) if
	// there is overlap
	return (clipRect.mCorner1- clipRect.mCorner2).magnitude();
}

Vector2D Rectangle2D::Intersection(const Rectangle2D &other) const
{	
	Rectangle2D clipRect = other;		// A rectangle that describes the overlap
										// between the two rectangles
	// Clip the left side
	if(clipRect.mCorner1.XValue<this->mCorner1.XValue)
		clipRect.mCorner1.XValue=this->mCorner1.XValue;

	// Clip the right side
	if(clipRect.mCorner2.XValue>this->mCorner2.XValue)
		clipRect.mCorner2.XValue=this->mCorner2.XValue;

	// Clip the top side
	if(clipRect.mCorner1.YValue<this->mCorner1.YValue)
		clipRect.mCorner1.YValue=this->mCorner1.YValue;

	// Clip the bottom side
	if(clipRect.mCorner2.YValue>this->mCorner2.YValue)
		clipRect.mCorner2.YValue=this->mCorner2.YValue;

	// Note that clip rect may be "inverted" if there is no intersection
	// i.e. top > bottom or left>right
	// Not a problem.

	// Return the intersection point of the centre of the clipped rectangle

	return Intersection(clipRect.GetCentre());
}

float Rectangle2D::LengthThrough(const Segment2D &other) const
{
	Segment2D seg = this->Clip(other);
	return seg.GetLength();
}

Vector2D Rectangle2D::CollisionNormal(const Circle2D& other) const
{
	if(Intersects(Vector2D(other.mCentre)))		// Centre of circle inside rectangle
	{
		float t,b,l,r;
		t=other.mCentre.YValue-mCorner1.YValue;
		b=mCorner2.YValue-other.mCentre.YValue;
		l=other.mCentre.XValue-mCorner1.XValue;
		r=mCorner2.XValue-other.mCentre.XValue;

		float smallest = t;
		if(smallest>b)
			smallest =b;
		if(smallest>l)
			smallest = l;
		if(smallest>r)
			smallest =r;

		if(smallest == t)
			return Vector2D(0,1);
		else if(smallest == b)
			return Vector2D(0,-1);
		else if(smallest == l)
			return Vector2D(-1,0);
		else		// Smallest ==r
			return Vector2D(1,0);
	}
	else		// Centre of circle outside rectangle
	{
		Vector2D edge = Intersection(Point2D(other.mCentre));
		// Renderer::GetInstance()->DrawDot(edge, 6);
		Vector2D normal = other.mCentre - edge;
		normal = normal.unitVector() * (other.GetRadius() - normal.magnitude());
		return normal;
	}
}
Vector2D Rectangle2D::CollisionNormal(const Rectangle2D& other) const
{
	Vector2D answer(0,1);		// Assume an upwards collision
	// Get distance between this bottom and other top
	float distance = abs(this->mCorner1.YValue - other.mCorner2.YValue);
	
	// See if downwards collision is smaller
	if(distance > abs(this->mCorner2.YValue - other.mCorner1.YValue))
	{
		answer.set(0, -1);
		distance = abs(this->mCorner2.YValue - other.mCorner1.YValue);
	}

	// See if leftwards collision is smaller
	if(distance > abs(this->mCorner1.XValue - other.mCorner2.XValue))
	{
		answer.set(-1, 0);
		distance = abs(this->mCorner1.XValue - other.mCorner2.XValue);
	}

	// See if rightwards collision is smaller
	if(distance > abs(this->mCorner2.XValue - other.mCorner1.XValue))
	{
		answer.set(1, 0);
		distance = abs(this->mCorner2.XValue - other.mCorner1.XValue);
	}

	return answer;
}
Vector2D Rectangle2D::CollisionNormal(const Segment2D& other) const
{
	return CollisionNormal(other.Intersection(*this));
}