#pragma once

#include "entity.hpp"

namespace EmergenceServer
{

class Container
{
public:
	virtual bool push(Entity *) = 0;
	virtual Entity* pop() = 0;
};

} // EmergenceServer
