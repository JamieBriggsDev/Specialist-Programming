#pragma once
#include <list>
#include <vector>
#include <stdlib.h>
#include <stack>
#include <algorithm>
//#include 

#include "rules.h"
#include "vector2D.h"
#include "Shapes.h"
#include "staticmap.h"

class Node;

class Edge
{
public:
	Node* m_fromNode;
	Node* m_toNode;
	float m_cost;
};

class Node
{
public:
	std::vector<Edge> m_edgeList;
	//std::vector<Node> m_nodeList;
	Node* m_parent;
	Vector2D m_position;
	//float h;
	float g;	
	float f;
};

class Graph
{
public:
	Graph()
	{
		AnalyseMap();
		for (int i = 0; i < NUMBOTSPERTEAM; i++)
		{
			m_pathReady[i] = false;
		}
	}
	static Graph* m_instance;
	static Graph* GetInstance()
	{
		if (!m_instance)
			m_instance = new Graph;
		return m_instance;
	}
	std::vector<Node> m_NodeVector;
	std::vector<Edge> m_EdgeVector;
	void AddNode(Vector2D _location);
	void AddEdge(Node *_from, Node *_to, float _cost);
	Node* GetClosestNode(Vector2D _position);
	Vector2D GetClosestNodePosition(Vector2D _position);
	float GetDistance(Node* _from, Node* _to);
	float GetDistance(Vector2D _first, Vector2D _second);
	// For Debug
	void DrawNodes();
	void DrawEdges();
	//void DrawPath();

	std::vector<Vector2D> m_drawNode;
	std::vector<Vector2D> m_drawEdgeStart;
	std::vector<Vector2D> m_drawEdgeFinish;

	// Methods hides variables
	bool IsPathReady(int _bot);
	std::stack<Vector2D> GetPath(int _bot);

	//void ComputeScores(Node* _currentNode, Node* _end);
	Node* GetRandomNode();
	//std::stack<Vector2D> PathFind(Vector2D _from, Vector2D _to);
	float findHeuristic(Node* _start, Node* _finish);
	void DrawLists();
	void AnalyseMap();
	static int m_totalNodes;
	void Partition(Rectangle2D _rect);
	void FindPaths();

	// Path Find method
	std::stack<Vector2D> PathFind(const Vector2D _from, const Vector2D _to, const int _bot);
	// Path Find threaded method
	void StartThreadedPathFind(const Vector2D _from, const Vector2D _to, const int _bot, 
		std::thread* _thread);
private:
	void ThreadedPathFind(const Vector2D _from, const Vector2D _to, const int _bot);
	// Used for A* algorithm
	std::vector<Node*> m_closedSet;
	std::vector<Node*> m_openSet;
	// For use with threading
	bool m_pathReady[NUMBOTSPERTEAM];
	// Paths for bots to use
	std::stack<Vector2D> m_path[NUMBOTSPERTEAM];


	Node* m_currentPtr;
	Node* m_nextPtr;
	Node* FindLowestFScoreNode(std::vector<Node*> _set);
	bool ElementInVector(std::vector<Node*> _set, Node* _element);
	void RemoveElementFromVector(std::vector<Node*> &_list, Node* _node);

};