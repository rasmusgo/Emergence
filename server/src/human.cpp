#include "human.hpp"
#include "world.hpp"
#include "container.hpp"

#include <sstream>

namespace WolfSheepServer
{
Human::Human(int x, int y) :
		in_primary_hand(0),
		in_secondary_hand(0),
		on_back(0),
		health(100),
		male(true)
{
	this->x = x;
	this->y = y;

	Entity::HudIcon icon = { 0, 0, 64, 64, 10 };
	hud_icons.push_back(icon);
	icon.x = -48;
	icon.width = 32;
	icon.height = 32;
	hud_icons.push_back(icon);
	icon.x = 48;
	hud_icons.push_back(icon);
}

Human::~Human()
{
	// Drop everything
	if (in_primary_hand)
		in_primary_hand->drop_me();
	if (in_secondary_hand)
		in_secondary_hand->drop_me();
	if (on_back)
		on_back->drop_me();
}

void Human::action()
{
	--prepared;

	if (in_primary_hand == 0)
		return;

	// Find something in the world to interact with
	std::vector<Entity*> container;
	world->get_entities_on(get_x() + dx[direction], get_y() + dy[direction],
			container);

	for (typeof(container.begin()) it = container.begin();
			it != container.end(); ++it)
	{
		in_primary_hand->use(this, (*it));
		return;
	}

	// If we don't have anything in front of us, use it on empty air
	in_primary_hand->use(this, 0);
}

void Human::grab()
{
	--prepared;

	if (in_primary_hand == 0)
	{
		// Pick up from world
		std::vector<Entity*> container;
		world->get_entities_on(get_x() + dx[direction], get_y() + dy[direction],
				container);

		for (typeof(container.begin()) it = container.begin();
				it != container.end(); ++it)
		{
			// Try to pick up something
			if (*it != this)
			{
				in_primary_hand = (*it)->grab_me(this);
				if (in_primary_hand != 0)
					return;
			}
		}
	}
	else
	{
		// Try to put on back or in container on back
		if (on_back == 0)
		{
			// Try to carry on back
			if (in_primary_hand->mount(this))
			{
				on_back = in_primary_hand;
				in_primary_hand = 0;
			}
		}
		else
		{
			// Try to put inside thing on back
			Container *c = dynamic_cast<Container*>(on_back);
			if (c != 0)
			{
				// Drop into container on back
				in_primary_hand->drop_me();
				if (c->push(in_primary_hand))
					in_primary_hand = 0;
				else
				{
					// Pick it up again if the bag can't have it
					in_primary_hand = in_primary_hand->grab_me(this);
					;
				}
			}
		}
	}
}

void Human::drop()
{
	--prepared;

	if (in_primary_hand != 0)
	{
		// Drop into world
		in_primary_hand->drop_me();
		in_primary_hand = 0;
	}
	else
	{
		// Pick something from back or container on back
		if (on_back != 0)
		{
			// Try pick from container on back
			Container *c = dynamic_cast<Container*>(on_back);
			if (c != 0)
			{
				Entity *ent = c->pop(); // null if empty

				// Try to grab the entity from the container
				if (ent != 0)
				{
					if (ent->grab_me(this))
						in_primary_hand = ent;
				}
			}

			// Remove entity from back if we failed to pick from container
			if (in_primary_hand == 0)
			{
				if (on_back->unmount())
				{
					in_primary_hand = on_back;
					on_back = 0;
				}
			}

		}
	}
}

void Human::swap()
{
	--prepared;

	Entity *tmp = in_secondary_hand;
	in_secondary_hand = in_primary_hand;
	in_primary_hand = tmp;
}

void Human::remove_reference(const Entity *ent)
{
	if (in_primary_hand == ent)
		in_primary_hand = 0;
	if (in_secondary_hand == ent)
		in_secondary_hand = 0;
	if (on_back == ent)
		on_back = 0;
}

void Human::use(Entity *user, Entity *target)
{
}

void Human::tick()
{
}

int Human::get_offset_x(const Entity *ent) const
{
	if (ent == in_primary_hand)
		return dx[get_direction()] * 24;
	else if (ent == in_secondary_hand)
		return dx[(get_direction() + 1) & 3] * 24;
	return dx[get_direction()] * -24;
}

int Human::get_offset_y(const Entity *ent) const
{
	// 24 from the body, 8 above the ground
	if (ent == in_primary_hand)
		return dy[get_direction()] * 24 - 8;
	else if (ent == in_secondary_hand)
		return dy[(get_direction() + 1) & 3] * 24 - 8;
	return dy[get_direction()] * -24 - 8;
}

int Human::get_offset_z(const Entity *ent) const
{
	// 24 from the body, 8 above the ground
	if (ent == in_primary_hand)
		return 8;
	else if (ent == in_secondary_hand)
		return 8;
	return 8;
}

int Human::get_w() const
{
	return 64; // (body_fat + length) *0.01 * 64;
}

int Human::get_h() const
{
	return 64; //(length) *0.01 * 64;
}

int Human::get_tex() const
{
	return 10; //(length) *0.01 * 64;
}

int Human::get_weight() const
{
	int weight = 70000;
	if (in_primary_hand != 0)
		weight += in_primary_hand->get_weight();
	if (in_secondary_hand != 0)
		weight += in_secondary_hand->get_weight();
	if (on_back != 0)
		weight += on_back->get_weight();
	return weight;
}

std::vector<Entity::HudIcon>& Human::get_hud_icons()
{
	if (in_primary_hand != 0)
		hud_icons[0].tex = in_primary_hand->get_tex();
	else
		hud_icons[0].tex = 10;

	if (in_secondary_hand != 0)
		hud_icons[1].tex = in_secondary_hand->get_tex();
	else
		hud_icons[1].tex = 10;

	if (on_back != 0)
		hud_icons[2].tex = on_back->get_tex();
	else
		hud_icons[2].tex = 10;

	return hud_icons;
}

void Human::pack(std::ostream &os)
{
	Entity::pack(os);
	os << " " << health << " "
			<< male << " "
			<< world->get_id_from_entity(in_primary_hand) << " "
			<< world->get_id_from_entity(in_secondary_hand) << " "
			<< world->get_id_from_entity(on_back);
}

void Human::unpack(std::istream &is)
{
	Entity::unpack(is);
	unsigned int id1, id2, id3;
	is >> health >> male >> id1 >> id2 >> id3;

	in_primary_hand = world->get_entity_from_id(id1);
	in_secondary_hand = world->get_entity_from_id(id2);
	on_back = world->get_entity_from_id(id3);
}

}
