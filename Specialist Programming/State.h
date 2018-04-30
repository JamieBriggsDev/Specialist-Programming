#pragma once

template<class EntityType>
class State
{
private:
	// Classes which use this class template will make singleton
	//  functions to get instance
public:
	virtual void Enter(EntityType*) = 0;
	virtual void Update(EntityType*) = 0;
	virtual void Exit(EntityType*) = 0;
};

