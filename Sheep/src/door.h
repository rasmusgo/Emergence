#pragma once

#include "entity.h"

namespace WolfSheepServer
{

class Door: public Entity
{
public:
	Door(int x, int y, bool locked, int lock_id) :
			locked(locked), lock_id(lock_id)
	{
		this->x = x;
		this->y = y;
	}
	virtual int get_tex() const
	{
		if (locked)
			return 70;
		return 71;
	}
	virtual int get_weight() const
	{
		return 10000; // 10 kg
	}
	virtual bool is_blocking() const
	{
		return locked;
	}
	virtual Entity* grab_me(Entity *grabber)
	{
		if (locked)
			return 0;
		return Entity::grab_me(grabber);
	}

	virtual bool lock(int key_id)
	{
		if (key_id == lock_id)
		{
			locked = !locked;
			return true;
		}
		return false;
	}

	virtual std::string get_type()
	{
		return "door";
	}

	virtual void pack(std::ostream &os)
	{
		Entity::pack(os);
		os << " " << locked << " " << lock_id;
	}

	virtual void unpack(std::istream &is)
	{
		Entity::unpack(is);
		is >> locked >> lock_id;
	}
protected:
	bool locked;
	int lock_id;
private:
};

} // WolfSheepServer
