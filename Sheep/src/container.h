#ifndef CONTAINER_H
#define CONTAINER_H

#include "entity.h"

namespace WolfSheepServer
{

class Container
{
public:
	virtual bool push(Entity *) = 0;
	virtual Entity* pop() = 0;
};

}
#endif // CONTAINER_H
