#pragma once
#include "shapes.h"
#include <vector>
#include "ErrorType.h"

struct Block
{
	Rectangle2D area;
};


struct SpawnPoint
{
	Vector2D location;
};

struct SupplyPoint
{
  Vector2D location;
};


class StaticMap
{
private:
	StaticMap();
	~StaticMap();
	StaticMap(const StaticMap& other);			// Disabled
	static StaticMap* pInst;
	std::vector<Block> m_BlockList;
	std::vector<SpawnPoint> m_SpawnPointList;
  std::vector<SupplyPoint> m_SupplyPointList;

public:
	static StaticMap* GetInstance();
	static void Release();
	ErrorType Render();
	void AddBlock(Rectangle2D area);
	void AddSpawnPoint(Vector2D location);
  void AddSupplyPoint(Vector2D location);

  // Returns the number of supply points on the map
  int GetNumSupplyPoints();

  // Returns the location of the specified resupply point
  // Return (0,0) for an invalid number (<0 or > number of supply points
  Vector2D GetResupplyLocation(int num);

  // Returns the location of the closest (Pythagorean distance)
  // supply point. Returns (0,0) if there are no supply points.
  Vector2D GetClosestResupplyLocation(Vector2D location);
	
	// Returns true if there is line of sight between the two points
	// False otherwise
	bool IsLineOfSight(Vector2D location1, Vector2D location2)  ;

	// Returns true if the specified point is inside a block
	// False otherwise
	bool IsInsideBlock(Vector2D location)  ;

	// Returns true if the specified rectangle is partially or completely inside a block
	// False otherwise
	bool IsInsideBlock(Rectangle2D area)  ;

	// Returns true if the specified circle is partially or completely inside a block
	// False otherwise
	bool IsInsideBlock(Circle2D area) ;

	// If the specified point is inside a block, Returns a vector normal to the closest
	// edge with a magnitude equal to the shortest distance between the specified
	// point and the edge.
	// If the specified point is not inside a block, returns a vector (0,0);
	Vector2D GetNormalToSurface(Vector2D location) ;

	// If the specified circle is inside a block, Returns a unit vector normal to the closest
	// point and the edge.
	// If the specified circle is not inside a block, returns a vector (0,0);
	Vector2D GetNormalToSurface(Circle2D region) ;

	Vector2D GetSpawnPoint(int teamNumber) ;

	void GetBlockList(Rectangle2D list[], int& size, int maxSize) ;
};