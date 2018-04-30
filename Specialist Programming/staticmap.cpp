#include "staticmap.h"
#include "renderer.h"
#include "errorlogger.h"

StaticMap* StaticMap::pInst=NULL;

StaticMap::StaticMap()
{

}

StaticMap::~StaticMap()
{

}

StaticMap* StaticMap::GetInstance()
{
	if(pInst == NULL)
	{
		pInst= new StaticMap();
	}

	return pInst;
}

void StaticMap::Release()		// Static
{
	if(pInst)
	{
		delete pInst;
		pInst = NULL;
	}
}

ErrorType StaticMap::Render()			// Static
{
	ErrorType answer = SUCCESS;

	// Render blocks
	std::vector<Block>::iterator bit = m_BlockList.begin();
	while(bit!=m_BlockList.end())
	{
		if(Renderer::GetInstance()->DrawBlock(bit->area)==FAILURE)
		{
			ErrorLogger::Writeln(L"Failed to render a block");
			answer=FAILURE;
		}
		++bit;
	}

  // Render supply points
  std::vector<SupplyPoint>::iterator bit2 = m_SupplyPointList.begin();
  while (bit2 != m_SupplyPointList.end())
  {
    if (Renderer::GetInstance()->DrawSupplyPoint(bit2->location) == FAILURE)
    {
      ErrorLogger::Writeln(L"Failed to render a supply point");
      answer = FAILURE;
    }
    ++bit2;
  }


	return answer;
}

void StaticMap::AddBlock(Rectangle2D area)
{
	Block nu;
	nu.area = area;
	m_BlockList.push_back(nu);
}

void StaticMap::AddSpawnPoint(Vector2D location)
{
	SpawnPoint nu;
	nu.location = location;
	m_SpawnPointList.push_back(nu);
}

void StaticMap::AddSupplyPoint(Vector2D location)
{
  SupplyPoint nu;
  nu.location = location;
  m_SupplyPointList.push_back(nu);
}

// Returns the number of supply points on the map
int StaticMap::GetNumSupplyPoints()
{
  return m_SupplyPointList.size();
}

// Returns the location of the specified resupply point
// Return (0,0) for an invalid number (<0 or > number of supply points
Vector2D StaticMap::GetResupplyLocation(int num)
{
  if (num<0 || num>GetNumSupplyPoints())
  {
    return Vector2D(0, 0);
  }
  else
  {
    return m_SupplyPointList[num].location;
  }
}

// Returns the location of the closest (Pythagorean distance)
// supply point. Returns (0,0) if there are no supply points.
Vector2D StaticMap::GetClosestResupplyLocation(Vector2D location)
{
  if (GetNumSupplyPoints()<=0)
  {
    return Vector2D(0, 0);
  }
  else
  {
    std::vector<SupplyPoint>::iterator it = m_SupplyPointList.begin();
    Vector2D closest = it->location;
    float dist2 = (closest - location).magnitudeSquared();
    while (it != m_SupplyPointList.end())
    {
      if ((it->location - location).magnitudeSquared() < dist2)
      {
        closest = it->location;
        dist2 = (closest - location).magnitudeSquared();
      }
      ++it;
    }
    return closest;
  }
}

	
// Returns true if there is line of sight between the two points
// False otherwise
bool StaticMap::IsLineOfSight(Vector2D location1, Vector2D location2) 
{
	Segment2D seg;
	seg.PlaceAt(location1, location2);

	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(seg))
			return false;
	}

	return true;
}

// Returns true if the specified point is inside a block
// False otherwise
bool StaticMap::IsInsideBlock(Vector2D location)
{
	Point2D p = location;
	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(p))
			return true;
	}
	return false;
}

// Returns true if the specified rectangle is partially or completely inside a block
// False otherwise
bool StaticMap::IsInsideBlock(Rectangle2D area) 
{
	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(area))
			return true;
	}

	return false;
}

// Returns true if the specified circle is partially or completely inside a block
// False otherwise
bool StaticMap::IsInsideBlock(Circle2D area) 
{
	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(area))
			return true;
	}

	return false;
}

// If the specified point is inside a block, Returns a vector normal to the closest
// edge with a magnitude equal to the shortest distance between the specified
// point and the edge.
// If the specified point is not inside a block, returns a vector (0,0);
Vector2D StaticMap::GetNormalToSurface(Vector2D location)
{
	Point2D p = location;
	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(p))
		{
			double distance = (it->area.Intersection(p)-location).magnitude();
			return (it->area.CollisionNormal(p)*float(distance));
		}
	}
	return Vector2D(0,0);
}

// If the specified circle intersects a block, Returns a vector normal to the closest
// edge with a magnitude equal to the shortest distance between the specified
// point and the edge.
// If the specified circle is not inside a block, returns a vector (0,0);
Vector2D StaticMap::GetNormalToSurface(Circle2D region)
{
	std::vector<Block>::iterator it = m_BlockList.begin();

	for(;it!=m_BlockList.end();++it)
	{
		if(it->area.Intersects(region))
		{
			return (it->area.CollisionNormal(region).unitVector());
		}
	}
	return Vector2D(0,0);
}


Vector2D StaticMap::GetSpawnPoint(int teamNumber)
{
	if(teamNumber<0 || teamNumber>=int(m_SpawnPointList.size()))
		return Vector2D(0,0);
	else
		return m_SpawnPointList[teamNumber].location;
}

void StaticMap::GetBlockList(Rectangle2D list[], int& size, int maxSize)
{
	size=0;

	std::vector<Block>::iterator it = m_BlockList.begin();
	while(size<maxSize && it!=m_BlockList.end())
	{
		list[size] = it->area;
		++size;
		++it;
	}
}