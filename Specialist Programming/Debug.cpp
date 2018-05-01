#include "Debug.h"
#include "mydrawengine.h"

Debug* Debug::m_inst = nullptr;

Debug * Debug::GetInstance()
{
	if (m_inst == nullptr)
		m_inst = new Debug;

	return m_inst;
}

void Debug::DrawPath(stack<Vector2D> *_path, Vector2D _currentPosition)
{
	if (_path->size() > 0)
	{
		std::stack<Vector2D> l_temp = *_path;
		Vector2D previousPoint = l_temp.top();
		l_temp.pop();
		Vector2D nextPoint = l_temp.top();

		// Bot to first point
		MyDrawEngine::GetInstance()->DrawLine(_currentPosition, previousPoint, MyDrawEngine::DARKGREEN);

		// Rest of path
		while (l_temp.size() > 1)
		{
			MyDrawEngine::GetInstance()->DrawLine(previousPoint, nextPoint, MyDrawEngine::GREEN);

			previousPoint = nextPoint;
			l_temp.pop();
			nextPoint = l_temp.top();
		}
		MyDrawEngine::GetInstance()->DrawLine(previousPoint, nextPoint, MyDrawEngine::LIGHTGREEN);

	}

}
