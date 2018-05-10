#pragma once

/////////////////////////////////////////////////

#define ENEMY_TARGET_RANGE 550.0f

/////////////////////////////////////////////////

enum class StateNames{CAPTURE, ATTACK, RELOAD, DEFEND};

template<class EntityType>
class State
{
	friend class behaviour;
private:
	// Classes which use this class template will make singleton
	//  functions to get instance
protected:
	State(StateNames _name) : m_name(_name) {}
	StateNames m_name;
public:
	virtual void Enter(EntityType*) = 0;
	virtual void Update(EntityType*) = 0;
	virtual void Exit(EntityType*) = 0;
	virtual wchar_t* GetName() = 0;

};
