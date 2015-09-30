#pragma once

/****************************************
 * This code is not in use right now
 ****************************************/

namespace WolfSheepServer
{

class World;

class WorldPosition
{
public:
	WorldPosition(int x, int y, World *world) :
			x(x), y(y), world(world)
	{
	}

	bool WorldPosition operator == (const WorldPosition &a, const WorldPosition &b)
	{
		if (a.world != b.world)
		return false;

		return (a.world->difference(a.x, b.x) == 0 && a.world->difference(a.y, b.y) == 0);
	}
protected:
	int x;
	int y;
	World *world;
private:
};

} // WolfSheepServer
