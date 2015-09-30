#pragma once

#include "entity.h"

namespace WolfSheepServer
{

class Container
{
public:
	virtual bool push(Entity *) = 0;
	virtual Entity* pop() = 0;
};

} // WolfSheepServer
