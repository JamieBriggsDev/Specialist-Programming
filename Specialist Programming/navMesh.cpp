#include <time.h>
#include <mutex>

#include "navMesh.h"
#include "mydrawengine.h"
#include "rules.h"

#define add(x) push_back(x)

Graph* Graph::m_instance = nullptr;

std::mutex g_mutex;

#define THREADLOCK g_mutex.lock()
#define THREADUNLOCK g_mutex.unlock()


void Graph::AddNode(Vector2D _location)
{
	Node newNode;
	newNode.m_position = _location;
	m_NodeVector.push_back(newNode);
}

void Graph::AddEdge(Node *_from, Node *_to, float _cost)
{
	Edge newEdge;
	newEdge.m_fromNode = _from;
	newEdge.m_toNode = _to;
	newEdge.m_cost = _cost;


	// For the way back, passed into the _to->edgelist
	Edge invertedEdge;
	invertedEdge.m_fromNode = _to;
	invertedEdge.m_toNode = _from;
	invertedEdge.m_cost = _cost;


	// Stored in edge vector so drawing lines is easier
	m_EdgeVector.push_back(newEdge);

	_from->m_edgeList.push_back(newEdge);
	_to->m_edgeList.push_back(invertedEdge);

	// Add Nodes to Node lists in Node
	//_from->m_nodeList.push_back(_to);
	//_to->m_nodeList.push_back(_from);
}

float Graph::GetDistance(Node* _from, Node* _to)
{
	float x, y, answer;
	// Uses pythagorous
	x = _from->m_position.XValue - _to->m_position.XValue;
	y = _from->m_position.YValue - _to->m_position.YValue;
	answer = sqrt((x * x) + (y * y));
	return answer;
}

float Graph::GetDistance(Vector2D _first, Vector2D _second)
{
	float x, y, answer;
	// Uses pythagorous
	x = _first.XValue - _second.XValue;
	y = _first.YValue - _second.YValue;
	answer = sqrt((x * x) + (y * y));
	return answer;
}

Node* Graph::GetClosestNode(Vector2D _position)
{
	// Get first nodes distance
	float l_shortestDistance = GetDistance(_position, m_NodeVector.at(0).m_position);
	int l_closestIndex = 0;
	for (int i = 1; i < (int)m_NodeVector.size(); i++)
	{
		if (l_shortestDistance > GetDistance(_position, m_NodeVector.at(i).m_position))
		{
			l_shortestDistance = GetDistance(_position, m_NodeVector.at(i).m_position);
			l_closestIndex = i;
		}
	}
	return &m_NodeVector.at(l_closestIndex);
}

Vector2D Graph::GetClosestNodePosition(Vector2D _position)
{
	return GetClosestNode(_position)->m_position;
}

void Graph::DrawNodes()
{
	// Draw a list of nodes
	MyDrawEngine::GetInstance()->DrawPointList(m_drawNode.data(), MyDrawEngine::WHITE, m_NodeVector.size());
}

void Graph::DrawEdges()
{ 
	// Draws a list of edges
	MyDrawEngine::GetInstance()->DrawLineList(m_drawEdgeStart.data(), m_drawEdgeFinish.data(), MyDrawEngine::BLACK, m_EdgeVector.size());
		//MyDrawEngine::GetInstance()->WriteInt(Vector2D(0.0f, 0.0f), m_EdgeVector.size(), MyDrawEngine::RED);
}

void Graph::AnalyseMap()
{
	// In this function, the map will be generated
	Rectangle2D l_map;
	l_map.PlaceAt(-Vector2D(NAVMESHLENGTH, NAVMESHHEIGHT), Vector2D(NAVMESHLENGTH, NAVMESHHEIGHT));
	Partition(l_map);
	FindPaths();
}

int Graph::m_totalNodes = 0;

void Graph::Partition(Rectangle2D _rect)
{
	// This partition method works by sticking a node inside a 
	//  rectangle sent in, however if the rectangle is colliding
	//  with a block, it will then be split into 4, in which, the smaller
	//  rectangles will be passed through into this partition method again through
	//  this one. Think of it as Partition(){Partition(){Partition()}}; These methods will
	//  stop once the rectangle is too small.
	//--------------------------------------------------
	//|							|						|
	//|							|						|
	//|				O			|			O			|
	//|							|						|
	//|							|						|
	//--------------------------------------------------
	//|				|			|			|			|
	//|		O		|	  O		|	  O		|	  O		|
	//|				|			|			|			|
	//|-------------|-----------|-----------|-----------|
	//|#############|			|			|			|
	//|#############|	  O  	|	  O		|	  O		|
	//|#############|			|			|			|
	//--------------------------------------------------

	
	// Makes sure that once the area of the rectangle
	//  gets too small, it stops partitioning.
	if (_rect.GetArea() < 1367)
	{
		m_totalNodes += 1;
		return;
	}
	// Otherwise, let the partitioning begin!
	//  Firstly if the rectangle intercepts into the map, split
	//		the rectangle into 4 other rectangles.
	else if (StaticMap::GetInstance()->IsInsideBlock(_rect))
	{
		// Create 4 different rectangles
		// R1-------R2
		//	|		|
		// R3-------R4
		// R1 and R4 are made differently through 4 floats
		// R2 and R4 are made through 2 Vectors (Bottom left and Bottom Rigt)
		Rectangle2D r1, r2, r3, r4;
		r1.PlaceAt(_rect.GetTopLeft().YValue,
			_rect.GetTopLeft().XValue,
			_rect.GetCentre().YValue, 
			_rect.GetCentre().XValue);
		r2.PlaceAt(_rect.GetCentre(), _rect.GetTopRight());
		r3.PlaceAt(_rect.GetBottomLeft(), _rect.GetCentre());
		r4.PlaceAt(_rect.GetCentre().YValue, 
			_rect.GetCentre().XValue,
			_rect.GetBottomRight().YValue, 
			_rect.GetBottomRight().XValue);

		// Partition the new rectangles again, in which if they need to,
		//  they will split into another 4 rectangles if needs be
		Partition(r1);
		Partition(r2);
		Partition(r3);
		Partition(r4);
		//Partition
	}
	else
	{
		AddNode(_rect.GetCentre());
	}
}

void Graph::FindPaths()
{

	StaticMap* pStaticMap = StaticMap::GetInstance();

	int l_NodeVectorSize = m_NodeVector.size();
	for (int i = 0; i < l_NodeVectorSize; i++)
	{
		// Start from after the node we're checking
		for (int j = i + 1; j < l_NodeVectorSize; j++)
		{
			// Makes sure not to compare the node with itself
			if ( StaticMap::GetInstance()->
				IsLineOfSight(m_NodeVector.at(i).m_position, m_NodeVector.at(j).m_position)
				&& GetDistance(&m_NodeVector.at(i), &m_NodeVector.at(j)) < 350.0f
				&& GetDistance(&m_NodeVector.at(i), &m_NodeVector.at(j)) > 50.0f)
			{
				float l_distance = (m_NodeVector.at(i).m_position - m_NodeVector.at(j).m_position).magnitude();
				AddEdge(&m_NodeVector.at(i), &m_NodeVector.at(j), l_distance);
			}
			
		}
	}
	//for (Node i : m_NodeVector)
	//{
	//	for (Node j : m_NodeVector)
	//	{
	//		if (pStaticMap->IsLineOfSight(i.m_position, j.m_position))
	//		{ // If there is a line of sight, create and store the edge
	//			float l_distance = (i.m_position - j.m_position).magnitude();
	//			AddEdge(&i, &j, l_distance);

	//			//Edge anEdge;
	//			//anEdge.edgeTo = j.nodeID;
	//			//anEdge.cost = (i.m_position - j.m_position).magnitude();
	//			//i.edges.push_back(anEdge);
	//		}
	//	}
	//}
	m_drawNode.clear();
	m_drawEdgeStart.clear();
	m_drawEdgeFinish.clear();

	for (Edge e : m_EdgeVector)
	{
		m_drawEdgeStart.push_back(e.m_fromNode->m_position);
		m_drawEdgeFinish.push_back(e.m_toNode->m_position);
	}


	for (Node n : m_NodeVector)
	{
		m_drawNode.push_back(n.m_position);
	}

}


std::stack<Vector2D> Graph::PathFind(const Vector2D _from, const Vector2D _to, const int _bot)
{
	// #0	-	Initialise starting variables
	// Temp variable for nodes neighbours
	std::vector<Node*> l_neighbors;
	// Temp g score variable
	float l_tentativeGScore;

	while (!m_path[_bot].empty())
		m_path[_bot].pop();

	// Makes sure sets are all empty
	m_closedSet.clear();
	m_openSet.clear();

	// Gets start and finish nodes
	Node* l_start = GetClosestNode(_from);
	Node* l_goal = GetClosestNode(_to);

	// #1	-	Put start node into the open list with variables initialised
	l_start->m_parent = nullptr;
	l_start->g = 0.0f;
	l_start->f = findHeuristic(l_start, l_goal);
	m_openSet.add(l_start);

	// #2 Start loop
	while (!m_openSet.empty())
	{
		// #3	-	Find node with smalled f value in open list;
		m_currentPtr = FindLowestFScoreNode(m_openSet);

		// #FINSIH	-	 This is the finish point which will
		// recreate the path
		if (m_currentPtr->m_position == l_goal->m_position)
		{
			m_path[_bot].push(_to);
			while (m_currentPtr)
			{
				m_path[_bot].push(m_currentPtr->m_position);
				if (m_currentPtr->m_parent != nullptr)
					m_currentPtr = m_currentPtr->m_parent;
				else return m_path[_bot];
			}

			break;
		}

		// #4	-	Remove current node from open set and add to closed set
		m_closedSet.add(m_currentPtr);

		///////////
		RemoveElementFromVector(m_openSet, m_currentPtr);

		// #5 - Get nodes neighbours
		for (int i = 0; i < m_currentPtr->m_edgeList.size(); i++)
		{
			l_neighbors.push_back(m_currentPtr->m_edgeList[i].m_toNode);
		}

		// #6	-	Go through all neighbours. 
		//			- If its in the closed set, ignore it
		//			- If the node isn't in the open set, add it with variables
		//			- if its in the open set, check to see if it f value is bigger then this path,
		//				and if it is, change it
		for (int i = 0; i < (int)l_neighbors.size(); i++)
		{
			// if node is within the closed set, skip the node
			if (ElementInVector(m_closedSet, l_neighbors.at(i)))
			{
				continue;
			}
			else
			{
				//// The distance from start to a neighbor
				l_tentativeGScore = m_currentPtr->g +
					GetDistance(m_currentPtr, l_neighbors.at(i));

				// If the node is not in the open set or the distance traveled is shorted then
				//  its set G value
				//if (!ElementInVector(l_openSet, l_neighbors.at(i)) ||
				//	l_tentativeGScore < l_neighbors.at(i).g)
				//{
				//	l_neighbors.at(i).g = l_tentativeGScore;
				//	l_neighbors.at(i).f = l_neighbors.at(i).g +
				//		findHeuristic(l_neighbors.at(i), l_goal);
				//	l_neighbors.at(i).m_parent = m_currentPtr;
				//	// If its not in the open list, add it
				//	if (!ElementInVector(l_openSet, l_neighbors.at(i)))
				//	{
				//		l_openSet.push_back(l_neighbors.at(i));
				//	}
				//}

				// Isnt in open set
				if (!ElementInVector(m_openSet, l_neighbors[i]) ||
					l_tentativeGScore < l_neighbors[i]->g)
				{
					l_neighbors.at(i)->m_parent = m_currentPtr;
					l_neighbors.at(i)->g = l_tentativeGScore;
					l_neighbors.at(i)->f = l_neighbors.at(i)->g +
						findHeuristic(l_neighbors.at(i), l_goal);
					m_openSet.add(l_neighbors.at(i));
				}
				// Is in open set
				else
				{
					if (l_tentativeGScore + findHeuristic(l_neighbors.at(i), l_goal) <
						l_neighbors.at(i)->f)
					{
						l_neighbors.at(i)->g = l_tentativeGScore;
						l_neighbors.at(i)->f = l_neighbors.at(i)->g +
							findHeuristic(l_neighbors.at(i), l_goal);
						l_neighbors.at(i)->m_parent = m_currentPtr;
					}
				}

			}// if not in closed set
		}// for loop
	}// while loop

	 // if fails, return l_path with one position = 0.0f, 0.0f
	if (!m_path[_bot].empty())
	{
		return m_path[_bot];
	}
	else
	{
		//m_path[_bot].push(Vector2D(0.0f, 0.0f));
		return m_path[_bot];
	}
}

void Graph::StartThreadedPathFind(const Vector2D _from, const Vector2D _to, const int _bot, std::thread * _thread)
{
	_thread = new std::thread(&Graph::ThreadedPathFind, this, _from, _to, _bot);
}


void Graph::ThreadedPathFind(const Vector2D _from, const Vector2D _to, const int _bot)
{
	THREADLOCK;
	// #0	-	Initialise starting variables
	// Temp variable for nodes neighbours
	std::vector<Node*> l_neighbors;
	// Temp g score variable
	float l_tentativeGScore;

	while (!m_path[_bot].empty())
		m_path[_bot].pop();

	// Makes sure sets are all empty
	m_closedSet.clear();
	m_openSet.clear();

	// Gets start and finish nodes
	Node* l_start = GetClosestNode(_from);
	Node* l_goal = GetClosestNode(_to);

	// #1	-	Put start node into the open list with variables initialised
	l_start->m_parent = nullptr;
	l_start->g = 0.0f;
	l_start->f = findHeuristic(l_start, l_goal);
	m_openSet.add(l_start);

	// #2 Start loop
	while (!m_openSet.empty())
	{
		// #3	-	Find node with smalled f value in open list;
		m_currentPtr = FindLowestFScoreNode(m_openSet);

		// #FINSIH	-	 This is the finish point which will
		// recreate the path
		if (m_currentPtr->m_position == l_goal->m_position)
		{
			m_path[_bot].push(_to);
			while (m_currentPtr)
			{
				m_path[_bot].push(m_currentPtr->m_position);
				if (m_currentPtr->m_parent != nullptr)
					m_currentPtr = m_currentPtr->m_parent;
				else 
				{
					m_pathReady[_bot] = true;
					THREADUNLOCK;
					return;
					//return m_path[_bot];
				}
			}

			break;
		}

		// #4	-	Remove current node from open set and add to closed set
		m_closedSet.add(m_currentPtr);

		///////////
		RemoveElementFromVector(m_openSet, m_currentPtr);

		// #5 - Get nodes neighbours
		for (int i = 0; i < m_currentPtr->m_edgeList.size(); i++)
		{
			l_neighbors.push_back(m_currentPtr->m_edgeList[i].m_toNode);
		}

		// #6	-	Go through all neighbours. 
		//			- If its in the closed set, ignore it
		//			- If the node isn't in the open set, add it with variables
		//			- if its in the open set, check to see if it f value is bigger then this path,
		//				and if it is, change it
		for (int i = 0; i < (int)l_neighbors.size(); i++)
		{
			// if node is within the closed set, skip the node
			if (ElementInVector(m_closedSet, l_neighbors.at(i)))
			{
				continue;
			}
			else
			{
				//// The distance from start to a neighbor
				l_tentativeGScore = m_currentPtr->g +
					GetDistance(m_currentPtr, l_neighbors.at(i));

				// If the node is not in the open set or the distance traveled is shorted then
				//  its set G value
				//if (!ElementInVector(l_openSet, l_neighbors.at(i)) ||
				//	l_tentativeGScore < l_neighbors.at(i).g)
				//{
				//	l_neighbors.at(i).g = l_tentativeGScore;
				//	l_neighbors.at(i).f = l_neighbors.at(i).g +
				//		findHeuristic(l_neighbors.at(i), l_goal);
				//	l_neighbors.at(i).m_parent = m_currentPtr;
				//	// If its not in the open list, add it
				//	if (!ElementInVector(l_openSet, l_neighbors.at(i)))
				//	{
				//		l_openSet.push_back(l_neighbors.at(i));
				//	}
				//}

				// Isnt in open set
				if (!ElementInVector(m_openSet, l_neighbors[i]) ||
					l_tentativeGScore < l_neighbors[i]->g)
				{
					l_neighbors.at(i)->m_parent = m_currentPtr;
					l_neighbors.at(i)->g = l_tentativeGScore;
					l_neighbors.at(i)->f = l_neighbors.at(i)->g +
						findHeuristic(l_neighbors.at(i), l_goal);
					m_openSet.add(l_neighbors.at(i));
				}
				// Is in open set
				else
				{
					if (l_tentativeGScore + findHeuristic(l_neighbors.at(i), l_goal) <
						l_neighbors.at(i)->f)
					{
						l_neighbors.at(i)->g = l_tentativeGScore;
						l_neighbors.at(i)->f = l_neighbors.at(i)->g +
							findHeuristic(l_neighbors.at(i), l_goal);
						l_neighbors.at(i)->m_parent = m_currentPtr;
					}
				}

			}// if not in closed set
		}// for loop
	}// while loop

	 // if fails, return l_path with one position = 0.0f, 0.0f
	if (!m_path[_bot].empty())
	{
		m_pathReady[_bot] = false;
		THREADUNLOCK;
		return;
	}
	else
	{
		//m_path[_bot].push(Vector2D(0.0f, 0.0f));
		m_pathReady[_bot] = false;
		THREADUNLOCK;
		return;
	}
}

//std::stack<Vector2D> Graph::PathFind(Vector2D _from, Vector2D _to)
//{
//	// #0	-	Initialise starting variables
//	// Temp variable for nodes neighbours
//	std::vector<Node*> l_neighbors;
//	// Temp g score variable
//	float l_tentativeGScore;
//
//	// Makes sure sets are all empty
//	m_closedSet.clear();
//	m_openSet.clear();
//
//	// Gets start and finish nodes
//	Node* l_start = GetClosestNode(_from);
//	Node* l_goal = GetClosestNode(_to);
//
//	// #1	-	Put start node into the open list with variables initialised
//	l_start->m_parent = nullptr;
//	l_start->g = 0.0f;
//	l_start->f = findHeuristic(l_start, l_goal);
//	m_openSet.push_back(l_start);
//
//	// #2 Start loop
//	while (!m_openSet.empty())
//	{
//		// #3	-	Find node with smalled f value in open list;
//		m_currentPtr = FindLowestFScoreNode(m_openSet);
//
//		// #FINSIH	-	 This is the finish point which will
//		// recreate the path
//		if (m_currentPtr->m_position == l_goal->m_position)
//		{
//			while (m_currentPtr)
//			{
//				m_path[_bot].push(m_currentPtr->m_position);
//				if (m_currentPtr->m_parent != nullptr)
//					m_currentPtr = m_currentPtr->m_parent;
//				else return m_path[_bot];
//			}
//
//			break;
//		}
//
//		// #4	-	Remove current node from open set and add to closed set
//		m_closedSet.push_back(m_currentPtr);
//
//		///////////
//		RemoveElementFromVector(m_openSet, m_currentPtr);
//
//		// #5	-	 Go through each neighbour in l_currentNode. If Node is in
//		//  the closed list, then ignore it.
//		// If node is not in the open list, add it and find out its score
//		// If the node is already in the open list, check the G score if its lower
//		// then the current generated path to get there. IF it is, update its scores and
//		// update its parent as well
//		//for (std::vector<Node*>::iterator iter = l_current->m_nodeList.begin(); 
//		//	iter != l_current->m_nodeList.end(); ++iter)
//
//		// # 5	-	Find all neighbors to the current node
//		l_neighbors.clear();
//		for (int i = 0; i < (int)m_NodeVector.size(); i++)
//		{
//			if (m_NodeVector.at(i).m_position == m_currentPtr->m_position)
//			{
//				continue;
//			}
//			if (ElementInVector(m_closedSet, &m_NodeVector.at(i)))
//			{
//				continue;
//			}
//			if (StaticMap::GetInstance()->IsLineOfSight(m_currentPtr->m_position,
//				m_NodeVector.at(i).m_position))
//			{
//				l_neighbors.push_back(&m_NodeVector.at(i));
//			}
//		}
//
//		// #6	-	Go through all neighbours. 
//		//			- If its in the closed set, ignore it
//		//			- If the node isn't in the open set, add it with variables
//		//			- if its in the open set, check to see if it f value is bigger then this path,
//		//				and if it is, change it
//		for (int i = 0; i < (int)l_neighbors.size(); i++)
//		{
//			// if node is within the closed set, skip the node
//			if (!ElementInVector(m_closedSet, l_neighbors.at(i)))
//			{
//				//// The distance from start to a neighbor
//				l_tentativeGScore = m_currentPtr->g +
//					GetDistance(m_currentPtr, l_neighbors.at(i));
//
//				// If the node is not in the open set or the distance traveled is shorted then
//				//  its set G value
//				//if (!ElementInVector(l_openSet, l_neighbors.at(i)) ||
//				//	l_tentativeGScore < l_neighbors.at(i).g)
//				//{
//				//	l_neighbors.at(i).g = l_tentativeGScore;
//				//	l_neighbors.at(i).f = l_neighbors.at(i).g +
//				//		findHeuristic(l_neighbors.at(i), l_goal);
//				//	l_neighbors.at(i).m_parent = m_currentPtr;
//				//	// If its not in the open list, add it
//				//	if (!ElementInVector(l_openSet, l_neighbors.at(i)))
//				//	{
//				//		l_openSet.push_back(l_neighbors.at(i));
//				//	}
//				//}
//
//				// Isnt in open set
//				if (!ElementInVector(m_openSet, l_neighbors[i]) ||
//						l_tentativeGScore < l_neighbors[i]->g)
//				{
//					l_neighbors.at(i)->g = l_tentativeGScore;
//					l_neighbors.at(i)->f = l_neighbors.at(i)->g +
//						findHeuristic(l_neighbors.at(i), l_goal);
//					l_neighbors.at(i)->m_parent = m_currentPtr;
//					m_openSet.push_back(l_neighbors.at(i));
//				}
//				// Is in open set
//				else
//				{
//					if (l_tentativeGScore + findHeuristic(l_neighbors.at(i), l_goal) <
//						l_neighbors.at(i)->f)
//					{
//						l_neighbors.at(i)->g = l_tentativeGScore;
//						l_neighbors.at(i)->f = l_neighbors.at(i)->g +
//							findHeuristic(l_neighbors.at(i), l_goal);
//						l_neighbors.at(i)->m_parent = m_currentPtr;
//					}
//				}
//				
//			}// if not in closed set
//		}// for loop
//	}// while loop
//
//	 // if fails, return l_path with one position = 0.0f, 0.0f
//	if (!m_path[_bot].empty())
//	{
//		return m_path[_bot];
//	}
//	else
//	{
//		//m_path[_bot].push(Vector2D(0.0f, 0.0f));
//		return m_path[_bot];
//	}
//}

// Manhatten distance
float Graph::findHeuristic(Node* _start, Node* _finish)
{
	//X
	//|	
	//|	  
	//|			
	//|       
	//|			
	//|			  
	//|				
	//+--------------Y
	///////////////////
	//float dx = _start->m_position.XValue - _finish->m_position.XValue;
	//if (dx < 0)
	//	dx = dx * -1;

	//float dy = _start->m_position.YValue - _finish->m_position.YValue;
	//if (dy < 0)
	//	dy = dy * -1;


	//return (dx + dy);
	return (_start->m_position - _finish->m_position).magnitude();
}

void Graph::DrawLists()
{
	// Open Set
	for (int i = 0; i < (int)m_openSet.size(); i++)
	{
		MyDrawEngine::GetInstance()->FillCircle(m_openSet.at(i)->m_position, 2.5f, MyDrawEngine::BLUE);
	}

	// Closed Set
	for (int i = 0; i < (int)m_closedSet.size(); i++)
	{
		MyDrawEngine::GetInstance()->FillCircle(m_closedSet.at(i)->m_position, 1.5f, MyDrawEngine::RED);
	}
}



bool Graph::IsPathReady(int _bot)
{
	return m_pathReady[_bot];
}

std::stack<Vector2D> Graph::GetPath(int _bot)
{
	// Safety
	if (m_pathReady[_bot])
	{
		m_pathReady[_bot] = false;
		return m_path[_bot];
	}
}

Node* Graph::FindLowestFScoreNode(std::vector<Node*> _set)
{
	int l_lowestIndex = 0;
	float l_lowestScore = _set.at(0)->f;
	// Goes through all nodes in list and if F is smaller then
	//  l_lowestScore then set l_lowestScore to the nodes f.
	//  Then loop more until the end of the array
	for (int i = 1; i < (int)_set.size(); i++)
	{
		if (ElementInVector(m_closedSet, _set.at(i)))
		{
			continue;
		}
		if (_set.at(i)->f < l_lowestScore)
		{
			l_lowestScore = _set.at(i)->f;
			l_lowestIndex = i;
		}
	}

	return _set.at(l_lowestIndex);

}

bool Graph::ElementInVector(std::vector<Node*> _set, Node* _element)
{
	// Removes element from vector by looking at position
	//  because no two nodes share the same position
	for (int i = 0; i < (int)_set.size(); i++)
	{
		if (_element->m_position == _set.at(i)->m_position)
		{
			return true;
		}
	}

	return false;
}


Node *Graph::GetRandomNode()
{	
	// Finds a random Node
	int random = rand() % m_NodeVector.size();
	return &m_NodeVector.at(random);
}

void Graph::RemoveElementFromVector(std::vector<Node*> &_list, Node* _node)
{
	//for (std::vector<Node>::iterator iter = _list.begin(); iter != _list.end(); ++iter)
	//{
	//	if (iter == _node);
	//	{
	//		_list.erase(iter);
	//		break;
	//	}
	//}
	//for (int i = 0; i < _list.size(); i++)
	//{
	//	if (_node.m_position == _list.at(i).m_position)
	//	{
	//		_list.erase(_list.begin() + i);
	//		break;
	//	}
	//}
	Vector2D l_nodeLocation = _node->m_position;
	for (int i = 0; i < (int)_list.size(); i++)
	{
		if (l_nodeLocation == _list.at(i)->m_position)
		{
			_list.erase(_list.begin() + i);
			break;
		}
	}
	//_list.erase(std::remove(_list.begin(), _list.end(), _node), _list.end());
	//auto it = std::find(_list.begin(), _list.end(), _node);
	//// If elements isnt at the end, swap the end node with the erased node
	//if (it != _list.end())
	//{
	//	using std::swap;

	//	swap(*it, _list.back());
	//	_list.pop_back();
	//}
}
