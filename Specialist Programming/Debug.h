#pragma once

#include <stack>

#include "vector2D.h"

using namespace std;

class Debug
{
private:
	static Debug *m_inst;

public:
	static Debug *GetInstance();

	static void DrawPath(stack<Vector2D> *_path, Vector2D _currentPosition);
};