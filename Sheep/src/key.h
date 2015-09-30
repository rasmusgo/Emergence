#pragma once

#include "entity.h"
#include "door.h"

namespace WolfSheepServer
{

class Key: public Entity
{
public:
	Key(int x, int y, int color, int key_id) :
			color(color),
			key_id(key_id)
	{
		this->x = x;
		this->y = y;
	}
	virtual int get_w() const
	{
		return 24;
	}
	virtual int get_h() const
	{
		return 24;
	}
	virtual int get_tex() const
	{
		return 60 + color;
	}
	virtual int get_weight() const
	{
		return 50;
	} // 0.05 kg

	virtual int get_key_id() const
	{
		return key_id;
	}

	virtual void use(Entity *user, Entity *target)
	{
		Door *door = dynamic_cast<Door*>(target);
		if (door != 0 && this->world == door->world)
		{
			// Make sure the key is close enough to the door
			int distance = this->world->distance(this->get_x(), this->get_y(), door->get_x(), door->get_y());
			if (distance <= 1)
			{
				door->lock(key_id);
			}
		}
	}

	virtual std::string get_type()
	{
		return "key";
	}

	virtual void pack(std::ostream &os)
	{
		Entity::pack(os);
		os << " " << color << " " << key_id;
	}

	virtual void unpack(std::istream &is)
	{
		Entity::unpack(is);
		is >> color >> key_id;
	}

protected:
	int color;
	int key_id;
private:
};

} // WolfSheepServer
