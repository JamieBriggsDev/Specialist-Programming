#include "Vector2D.h"
#pragma once

// 11f - Added to engine
// 1/11/12 Fixed circle-segment collision functions

// Version 12.11 7/12/12
// Implemented Rectangle2D::CollisionNormal(Rectangle2D&)
// Implemented Rectangle2D::CollisionNormal(Segment2D&) 
// Implemented Rectangle2D::Intersection(Rectangle2D&) 
// Implemented Rectangle2D::Distance(Rectangle2D&) 

// Version 12.12  13/12/12
// Corrected error in Rectangle2D::Distance(Rectangle2D)

class IShape2D
{
public:
	virtual bool Intersects(const IShape2D& other) const=0;
	virtual ~IShape2D();
};

class Point2D;
class Segment2D;
class Circle2D;
class Rectangle2D;

class Point2D:public IShape2D
{
	friend Segment2D;
	friend Circle2D;
	friend Rectangle2D;
private:
	Vector2D mPosition;
public:
	// Constructor sets position to (0,0)
	Point2D();

	// Copy constructor
	Point2D(const Vector2D& copy);

	// Constructor. Remember that this is designed for screen geometry, so
	// y is inverted
	Point2D(float x, float y);

	// Places the point at the location specified by the vector
	void PlaceAt(const Vector2D &position);

	// Returns the vector location of the point
	Vector2D GetPosition() const;

	// Returns true if the other is identical to this
	// False otherwise
	bool Intersects(const Point2D &other) const;
	
	//Returns the distance between other and this
	float Distance(const Point2D &other) const;

	// Just returns the location of this Point2D
	Vector2D Intersection(const Point2D &other) const;

	// Returns true if this point lies on the 
	// Segment2D other.
	// False otherwise
	bool Intersects(const Segment2D &other) const;

	// Returns the distance from this point to the nearest 
	// point on the segment
	float Distance(const Segment2D &other) const;

	// Returns the location of this point
	Vector2D Intersection(const Segment2D &other) const;

	// Returns true if the point lies within the circle
	// False otherwise
	bool Intersects(const Circle2D &other) const;

	// Returns the distance from the point2D
	// to the edge of the circle
	// Note this will return a -ve value if
	// this point is inside the circle (-ve the
	// distance to the edge.)
	float Distance(const Circle2D &other) const;

	// Returns the location of this point
	Vector2D Intersection(const Circle2D &other) const;

	// Returns true if the point is within the rectangle
	// False otherwise
	// Note that Rectangle2D include the left and top edge,
	// but excludes the bottom and right edge
	bool Intersects(const Rectangle2D &other) const;

	// Returns true if the point intersects the specified shape
	bool Intersects(const IShape2D &other) const;

	// Returns the distance from this point to the closest 
	// point on the rectangle.
	// Returns a negative number if the point is within the rectangle
	//(-ve the distance to the nearest edge)
	float Distance(const Rectangle2D &other) const;

	// Returns the location of this point
	Vector2D Intersection(const Rectangle2D &other) const;
};

// A segment is a line with a start and an end
// I.e. not infinite length
class Segment2D:public IShape2D
{
	friend Point2D;
	friend Circle2D;
	friend Rectangle2D;
private:
	Vector2D mStart;
	Vector2D mEnd;
public:
	// Constructor - creates zero length segment at (0,0)
	Segment2D();

	// Places segment in position defined by the start and end
	// Start is part of the line, end isn't
	void PlaceAt(const Vector2D &start, const Vector2D &end);

	// Clips the segment to the t-values provided
	void ClipTo(float minT, float maxT);

	// Returns the position of the start of the segment
	Vector2D GetStart() const;

	// Returns the position of the end of the segment
	Vector2D GetEnd() const;

	// Returns the length of the segment
	float GetLength() const;

	// Returns true if other lies on the segment
	// False otherwise
	// Note that the point is not on the segment if it is beyond
	// the start or end
	bool Intersects(const Point2D &other) const;

	// Returns true if the segment intersects the specified shape
	bool Intersects(const IShape2D &other) const;

	// Distance from other to the closest point
	// on the segment
	float Distance(const Point2D &other) const;

	// Returns true if the segments are parallel (in either direction)
	// Allows for founding errors with a 0.01% tolerance
	bool ParallelTo(const Segment2D &other) const;

	// Returns true if the segment is parallel to a vector(in either direction)
	// Allows for founding errors with a 0.01% tolerance
	bool ParallelTo(const Vector2D &other) const;

	// Returns the closest point on the segment
	// to other
	Vector2D Intersection(const Point2D &other) const;

	// Returns true if the two segments intersect
	// Returns false otherwise
	bool Intersects(const Segment2D &other) const;

	// Returns the closest distance between the two segments
	float Distance(const Segment2D &other) const;

	// Returns the closest point on this segment to the
	// other segments (point of intersection if they intersect)
	Vector2D Intersection(const Segment2D &other) const;

	// Returns true if the Segment intersect the circle
	// False otherwise
	bool Intersects(const Circle2D &other) const;

	// Returns the closest distance from the segment to the circle
	// If the segment intersects the circle, this
	// returns -ve the distance from the deepest point on the
	// segment to the edge of the circle
	float Distance(const Circle2D &other) const;

	// Returns the closest point on the segment to the circle.
	// if the segement intersects the circle, this returns the
	// deepest point on the segment
	Vector2D Intersection(const Circle2D &other) const;

	// Returns true if the segment intersects the rectangle
	// False otherwise
	bool Intersects(const Rectangle2D &other) const;

	// Returns the distance from the Segment to the Rectangle
	// If they intersect, returns a negative number
	// Slow function
	float Distance(const Rectangle2D &other) const;

	// Returns the closest point on the Segment to the rectangle
	// If they intersect, returns the mid-point of the part of
	// the segment that intersects
	Vector2D Intersection(const Rectangle2D &other) const;

	// Returns the point of intersection with a rectangle.
	// If there are two points, returns the once closest to the
	// start.
	// If no intersection is found, returns the end of the segment
	Vector2D FirstIntersection(const Rectangle2D &other) const;

	/* Returns t, treating the line as a parametric equation.
	// Can return a t below 0 or above 1 if x is beyond the ends of the segment
	// If the line is horizontal, returns 1.0 if x is on the line, or
	 returns INT_MAX otherwise*/
	float GetTFromX(float x) const;

	// Returns t, treating the line as a parametric equation.
	// Can return a t below 0 or above 1 if y is beyond the ends of the segment
	// If the line is vertical, returns 1.0 if y is on the line, or
	// returns INT_MAX otherwise
	float GetTFromY(float y) const;

	// Returns the vector of a point on the line defined by t,
	// using parametric equations
	// x=x1 + t(x2-x1)
	// y=y1 + t(y2-y1)
	Vector2D PointFromT(float t) const;

	// Sets the start to the point on the line defined by t,
	// using parametric equations
	// x=x1 + t(x2-x1)
	// y=y1 + t(y2-y1)
	// Note, if t is less than 0, this will lengthen the line.
	// If t is greater than 1.0 this will reverse the direction of the line
	void SetStartTo(float t);

	// Sets the end to the point on the line defined by t,
	// using parametric equations
	// x=x1 + t(x2-x1)
	// y=y1 + t(y2-y1)
	// Note, if t is greater than 1.0, this will lengthen the line.
	// If t is less than 0.0 this will reverse the direction of the line
	void SetEndTo(float t);
};

class Circle2D:public IShape2D
{
	friend Point2D;
	friend Segment2D;
	friend Rectangle2D;
private:
	Vector2D mCentre;
	float mdRadius;
public:
	// Constructor. Sets centre to 0,0 and radius to zero
	Circle2D();

	// Constructor. Places the Circle2D at the specified location with specified radius.
	// A negative radius is converted to positive
	Circle2D(const Vector2D &centre, float radius);

	// Places the Circle2D at the specified location with specified radius.
	// A negative radius is converted to positive
	void PlaceAt(const Vector2D &centre, float radius);

	// Returns the location of the centre of the circle
	Vector2D GetCentre() const;

	// Returns the radius of the circle
	float GetRadius() const;

	// Returns the area of the circle
	float GetArea() const;

	// Returns true if the Point is inside the circle
	// False otherwise
	bool Intersects(const Point2D &other) const;

	// Returns true if the circle intersects the specified shape
	bool Intersects(const IShape2D &other) const;

	// Returns the distance from the point to the circle
	// If the point is inside the circle,
	// returns -ve the distance from the point to
	// the edge of the circle
	float Distance(const Point2D &other) const;

	// Returns the point on the edge of the circle closest
	// to the point (whether inside or outside)
	// Or returns the centre of this circle if the point
	// is exactly equal to the circle's centre
	Vector2D Intersection(const Point2D &other) const;

	// Returns true if the segment intersects the circle
	// False otherwise
	bool Intersects(const Segment2D &other) const;

	// Returns the closest distance from the circle
	// to the segment
	float Distance(const Segment2D &other) const;

	// If the segment intersects the  circle,
	// Returns the point on the surface of the circle
	// closest to the mid-point of the intersecting part of the segment.
	// otherwise returns the closest point on the edge of the 
	// circle to the segment
	Vector2D Intersection(const Segment2D &other) const;

	// Returns true if the two circles intersect
	// false otherwise
	bool Intersects(const Circle2D &other) const;

	// returns the distance between the two circles
	// This will be negative if the circles intersect
	float Distance(const Circle2D &other) const;

	// Returns the closest point on the edge of this circle
	// to the centre of the other circle. (Or the centre of
	// this circle if the two centres are equal)
	Vector2D Intersection(const Circle2D &other) const;

	// Returns true if the circle intersects the rectangle
	// false otherwise
	bool Intersects(const Rectangle2D &other) const;

	// Returns the distance from the edge of the circle
	// to the rectangle.
	// Returns a negative number if the circle and
	// the rectangle overlap
	float Distance(const Rectangle2D &other) const;

	// Returns the point on the surface of the circle
	// closest to the intersection (or the nearest point on
	// the rectangle if no intersection
	Vector2D Intersection(const Rectangle2D &other) const;

	// Returns the normal to the (presumed)
	// collision between the other point and
	// this circle
	Vector2D CollisionNormal(const Point2D& other) const;

	// Returns the normal to the (presumed)
	// collision between the other circle and
	// this circle
	Vector2D CollisionNormal(const Circle2D& other) const;

	// Returns the normal to the (presumed)
	// collision between the other rectangle and
	// this circle
	Vector2D CollisionNormal(const Rectangle2D& other) const;

	// Returns the normal to the (presumed)
	// collision between the other segment and
	// this circle
	Vector2D CollisionNormal(const Segment2D& other) const;
};

class Rectangle2D:public IShape2D
{
	friend Point2D;
	friend Segment2D;
	friend Circle2D;
private:
	Vector2D mCorner1;		// Bottom left
	Vector2D mCorner2;		// Top right
public:
	// Constructor. Sets all corners to (0,0)
	Rectangle2D();

	// Places the rectangle at the specified coordinates
	// the topRight should have larger X and Y values
	// than the bottomLeft.
	// If this is not right, x and y values are swapped by
	// this function as appropriate
	// The rectangle includes topLeft, but not bottomRight
	void PlaceAt(Vector2D bottomLeft, Vector2D topRight);

	// Places the rectangle at the specified location.
	// if top > bottom, these values are reversed
	// if left > right, these values are reversed
	// The rectangle includes the top and left edges,
	// but not the bottom or right
	void PlaceAt(float top, float left, float bottom, float right);

	// Returns the centre of the rectangle
	Vector2D GetCentre() const;

	// Returns the location of the top left corner
	Vector2D GetTopLeft() const;

	// Returns the location of the bottom right corner
	Vector2D GetBottomRight() const;

	// Returns the location of the bottom left corner
	Vector2D GetBottomLeft() const;

	// Returns the location of the top right corner
	Vector2D GetTopRight() const;

	// Returns the location of the top left corner
	Vector2D GetCorner1() const;

	// Returns the location of the bottom right corner
	Vector2D GetCorner2() const;

	// Returns the area of the rectangle
	float GetArea() const;

	// Returns true if the point intersects the rectangle
	// false otherwise
	bool Intersects(const Point2D &other) const;

	// Returns the distance from the point to the rectangle
	// If the point is inside the rectangle,
	// this returns -ve the distance to the closest edge
	float Distance(const Point2D &other) const;

	// Returns the point on the surface of the rectangle closest to the
	// point
	Vector2D Intersection(const Point2D &other) const;

	// Returns the normal to the surface of the rectangle closest
	// to the point of collision.
	// (1,0)  (-1,0)  (0,1)  or (0,-1)
	// If no collision, returns (0,0)
	Vector2D CollisionNormal(const Point2D& other) const;

	// Returns the normal of the probable collision between
	// the rectangle and the circle.
	// (If no collision returns a unit vector from the
	// closest point on the edge of the rectangle to
	// the centre of the circle
	Vector2D CollisionNormal(const Circle2D& other) const;

	// Returns the normal of the probable collision between
	// this rectangle and the passed rectangle. The normal
	// points away from this Rectangle, either (0,1) (0,-1)
	// (1,0) or (-1,0) depending on which is the closest edge.
	// (If there is no collision, it returns a vector pointing 
	// away from the closest edge on this rectangle
	Vector2D CollisionNormal(const Rectangle2D& other) const;

	// Not implemented  *************************************
	Vector2D CollisionNormal(const Segment2D& other) const;

	// Returns true if the segment intersects the
	// rectangle. False otherwise
	bool Intersects(const Segment2D &other) const;



	// Returns the length of the supplied segment that passes through the rectangle
	// Returns zero if the segment does not intersect the rectangle
	float LengthThrough(const Segment2D &other) const;

	// Returns the closest distance from the segment
	// to the rectangle. Returns a -ve value if the segment intersects
	// the rectangle
	float Distance(const Segment2D &other) const;

	// Returns the point on the edge of the rectangle
	// closest to segment (or the deepest part of the
	// segment if the segment intersects the rectangle)
	Vector2D Intersection(const Segment2D &other) const;

	// Returns true if the circle intersects the 
	// rectangle.
	// False otherwise
	bool Intersects(const Circle2D &other) const;

	// Returns the shortest distance from the circle 
	// to the rectangle.
	// Returns a negative number if the circle
	// intersects the rectangle
	float Distance(const Circle2D &other) const;

	// Returns the point on the edge of the rectangle
	// where intersection occured (or the closest
	// point to the circle if no intersection)
	Vector2D Intersection(const Circle2D &other) const;

	// Returns true if the rectangles intersect
	// false otherwise
	bool Intersects(const Rectangle2D &other) const;

	// Returns the distance between the two closest points 
	// on the two rectangles. Returns zero if the rectangles
	// overlap
	float Distance(const Rectangle2D &other) const;

	// Returns the point on this rectangle that is closest
	// to the point of intersection with the other rectangle.
	// If the two rectangles do not overlap, returns the
	// point on this rectangle that is closest to the other
	// rectangle
	Vector2D Intersection(const Rectangle2D &other) const;

	// Returns the supplied segment, clipped to fit inside the
	// rectangle.
	// Returns a zero length segment if the segment does not
	// intersect the rectangle
	Segment2D Clip(Segment2D other) const;

		// Returns true if the rectangle intersects the specified shape
	bool Intersects(const IShape2D &other) const;
};