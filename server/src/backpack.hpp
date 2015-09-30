#pragma once

#include "entity.hpp"
#include "container.hpp"

#include <stdlib.h>
#include <sstream>

namespace WolfSheepServer
{

class Backpack: public Entity, public Container
{
public:
	Backpack(int x, int y, int capacity) :
			capacity(capacity),
			size(0),
			slots(new Entity*[capacity])
	{
		this->x = x;
		this->y = y;
	}
	virtual ~Backpack()
	{
		while (pop() != 0)
			;
		delete[] slots;
	}
	virtual bool push(Entity *ent)
	{
		if (size >= capacity)
			return false;

		ent = ent->grab_me(this);

		if (ent == 0)
			return false;

		slots[size++] = ent;
		return true;
	}
	virtual Entity* pop()
	{
		if (size <= 0)
			return 0;

		int i = rand() % size;
		Entity *ent = slots[i];
		slots[i] = slots[--size];
		ent->drop_me();
		return ent;
	}
	virtual bool mount(Entity *ent)
	{
		return true;
	}
	virtual bool unmount()
	{
		return true;
	}

	virtual void remove_reference(const Entity *ent)
	{
		// This goes through the whole backpack
		// but will not remove multiple references
		for (int i = 0; i < size; ++i)
		{
			if (slots[i] == ent)
			{
				slots[i] = slots[--size];
				return;
			}
		}
	}

	virtual int get_w() const
	{
		return 40;
	}
	virtual int get_h() const
	{
		return 40;
	}
	virtual int get_tex() const
	{
		return 30;
	}
	virtual int get_weight() const
	{
		int weight = 1000; // 1 kg for the backpack
		for (int i = 0; i < size; ++i)
			weight += slots[i]->get_weight();
		return weight;
	}

	virtual Entity* grab_me(Entity *grabber)
	{
		// If we already have a parent, none is allowed to grab us
		if (parent != 0)
			return 0;

		// If the backpack contains items, try to pick them up from the backpack
		Entity *ent = pop();
		if (ent != 0)
			return ent->grab_me(grabber);

		// If the backpack is empty, let the grabber pick up the whole backpack
		return Entity::grab_me(grabber);
	}

	virtual std::string get_type()
	{
		return "backpack";
	}

	virtual void pack(std::ostream &os)
	{
		Entity::pack(os);
		os << " " << capacity << " " << size;
		for (int i = 0; i < size; ++i)
			os << " " << world->get_id_from_entity(slots[i]);
	}

	virtual void unpack(std::istream &is)
	{
		Entity::unpack(is);
		int garbage;
		is >> garbage >> size;
		for (int i = 0; i < size; ++i)
		{
			int id;
			is >> id;
			slots[i] = world->get_entity_from_id(id);
		}
	}

protected:
	const int capacity;
	int size;
	Entity **slots;
private:
};

} // WolfSheepServer
