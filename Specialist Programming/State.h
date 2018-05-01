#pragma once


enum class StateNames{CAPTURE};

template<class EntityType>
class State
{
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

	StateNames GetStateName() { return m_name; }

};

