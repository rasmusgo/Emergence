#include "entity.h"
#include "world.h"

#include <ostream>

namespace WolfSheepServer
{

const int Entity::dx[] = { 1, 0, -1, 0 };
const int Entity::dy[] = { 0, -1, 0, 1 };

Entity::Entity() :
		world(0),
		parent(0),
		x(0),
		y(0),
		direction(0),
		prepared(0)
{

}

Entity::~Entity()
{
	if (parent != 0)
		parent->remove_reference(this);
}

void Entity::prepare_action()
{
	++prepared;
}

void Entity::prepare_grab()
{
	++prepared;
}

void Entity::prepare_drop()
{
	++prepared;
}

void Entity::prepare_swap()
{
	++prepared;
}

void Entity::action()
{
	--prepared;
}

void Entity::grab()
{
	--prepared;
}

void Entity::drop()
{
	--prepared;
}

void Entity::swap()
{
	--prepared;
}

void Entity::right()
{
	if (parent != 0)
		return;

	if (!prepared && !world->is_blocked(get_x() + 1, get_y()))
		++x;

	direction = 0;

	// Make some erosion on the world when we walk
	world->erode(x, y);
}

void Entity::up()
{
	if (parent != 0)
		return;

	if (!prepared && !world->is_blocked(get_x(), get_y() - 1))
		--y;

	direction = 1;

	// Make some erosion on the world when we walk
	world->erode(x, y);
}

void Entity::left()
{
	if (parent != 0)
		return;

	if (!prepared && !world->is_blocked(get_x() - 1, get_y()))
		--x;

	direction = 2;

	// Make some erosion on the world when we walk
	world->erode(x, y);
}

void Entity::down()
{
	if (parent != 0)
		return;

	if (!prepared && !world->is_blocked(get_x(), get_y() + 1))
		++y;

	direction = 3;

	// Make some erosion on the world when we walk
	world->erode(x, y);
}

int Entity::get_x() const
{
	if (parent)
		return parent->get_x();
	return x;
}

int Entity::get_y() const
{
	if (parent)
		return parent->get_y();
	return y;
}

int Entity::get_z() const
{
	if (parent)
		return parent->get_z();
	return 0;
}

int Entity::get_offset_x(const Entity *ent) const
{
	return 0;
}

int Entity::get_offset_y(const Entity *ent) const
{
	return 0;
}

int Entity::get_offset_z(const Entity *ent) const
{
	return 0;
}

int Entity::get_offset_x() const
{
	int offset = (prepared > 0 ? dx[direction] * 5 : 0);
	if (parent)
		offset += parent->get_offset_x() + parent->get_offset_x(this);
	return offset;
}

int Entity::get_offset_y() const
{
	int offset = (prepared > 0 ? dy[direction] * 5 : 0);
	if (parent)
		offset += parent->get_offset_y() + parent->get_offset_y(this);
	return offset;
}

int Entity::get_offset_z() const
{
	if (parent)
		return parent->get_offset_z() + parent->get_offset_z(this);
	return 0;
}

int Entity::get_direction() const
{
	if (parent)
		return (parent->get_direction() + direction) & 3;
	return direction;
}

Entity* Entity::grab_me(Entity *grabber)
{
	if (parent != 0)
		return 0; // Fail to grab
	direction = (direction - grabber->get_direction()) & 3; // direction is now relative to grabber
	parent = grabber;
	return this;
}

void Entity::drop_me()
{
	world = parent->world;
	short pdir = parent->get_direction();
	x = parent->get_x() + dx[pdir];
	y = parent->get_y() + dy[pdir];
	direction = (direction + pdir) & 3; // direction is now absolute again
	parent = 0;
}

std::ostream& operator <<(std::ostream &os, const Entity &e)
{
	return os << e.x << " " << e.y << " " << e.direction << " "
			<< e.world->get_id_from_entity(e.parent);
}

void Entity::pack(std::ostream &os)
{
	os << x << " " << y << " " << direction << " "
			<< world->get_id_from_entity(parent);
}

void Entity::unpack(std::istream &is)
{
	unsigned int id = 0;
	is >> x >> y >> direction >> id;
	parent = world->get_entity_from_id(id);
}

} // WolfSheepServer
