#pragma once

#include "entity.hpp"

namespace EmergenceServer
{

class Binding
{
public:
	Binding() :
			pressCallback(0), releaseCallback(0), repeat(0)
	{
	}
	Binding(EntityMemberFunction function, int repeat) :
			pressCallback(function), releaseCallback(0), repeat(repeat)
	{
	}
	Binding(const EntityMemberFunction &function) :
			pressCallback(function), releaseCallback(0), repeat(0)
	{
	}
	Binding(const EntityMemberFunction &onDown,
			const EntityMemberFunction &onUp) :
			pressCallback(onDown), releaseCallback(onUp), repeat(0)
	{
	}
	void onPressed(Entity *entity)
	{
		if (pressCallback)
			(entity->*(pressCallback))();
	}
	void onReleased(Entity *entity)
	{
		if (releaseCallback)
			(entity->*(releaseCallback))();
	}
	void onRepeat(Entity *entity, Uint32 timePressed)
	{
		if (pressCallback && repeat != 0 && timePressed >= repeat)
			(entity->*(pressCallback))();
	}
protected:
	EntityMemberFunction pressCallback;
	EntityMemberFunction releaseCallback;
	Uint32 repeat;
};

} // EmergenceServer
