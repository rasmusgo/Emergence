#pragma once

#include "entity.hpp"

namespace WolfSheepServer
{

class Container
{
public:
	virtual bool push(Entity *) = 0;
	virtual Entity* pop() = 0;
};

} // WolfSheepServer
