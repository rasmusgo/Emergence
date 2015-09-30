#include "world.hpp"
#include "entity.hpp"

#include "entities.hpp"

#include <map>
#include <math.h>

#include <iostream>
#include <string>
#include <sstream>

// TODO: Save to disk
// TODO: Load from disk

namespace WolfSheepServer
{
World::World() :
		auto_id(1), time(0)
{
	//srand(0);
	for (int i = 0; i < map_size; ++i)
		grass_map[i] = 100; //rand()%(5+g);

	factories["apple"]    = &World::create_entity_apple;
	factories["backpack"] = &World::create_entity_backpack;
	factories["door"]     = &World::create_entity_door;
	factories["human"]    = &World::create_entity_human;
	factories["key"]      = &World::create_entity_key;
	factories["rabbit"]   = &World::create_entity_rabbit;
	factories["rock"]     = &World::create_entity_rock;
	factories["sword"]    = &World::create_entity_sword;
}

World::~World()
{
	clear();
}

int World::add_entity(Entity *ent)
{
	ent->world = this;
	// Find a id to use
	for (unsigned int id = auto_id;; ++id)
	{
		// Make sure we don't replace any entity if auto_id wraps around
		Entity * &e = entities[id];
		if (e == 0)
		{
			e = ent;
			id_lookup[ent] = id;
			auto_id = id + 1;
			return id;
		}
	}
}

void World::remove_entity(Entity *ent)
{
	typeof(id_lookup.begin()) it = id_lookup.find(ent);

	// This should never happen, fail hard
	if (it == id_lookup.end())
		*(int*) (0) = 0;

	entities.erase(it->second);
	id_lookup.erase(it);
}

void World::delete_entity(Entity *ent)
{
	remove_entity(ent);
	delete ent;
}

void World::get_entities_on(int x, int y, std::vector<Entity*> &container) const
{
	for (typeof(entities.begin()) e = entities.begin(); e != entities.end(); ++e)
	{
		if (difference(e->second->get_x(), x) == 0
				&& difference(e->second->get_y(), y) == 0)
			container.push_back(e->second);
	}
}

bool World::is_blocked(int x, int y) const
{
	for (typeof(entities.begin()) e = entities.begin(); e != entities.end(); ++e)
	{
		if (difference(e->second->get_x(), x) == 0
				&& difference(e->second->get_y(), y) == 0
				&& e->second->is_blocking())
			return true;
	}
	return false;
}

void World::reset()
{
	// Clear grass
	for (int i = 0; i < map_size; ++i)
		grass_map[i] = 100;
}

void World::clear()
{
	// Delete all entities
	for (typeof(entities.begin()) e = entities.begin(); e != entities.end(); ++e)
		delete e->second;

	entities.clear();
	id_lookup.clear();
}

void World::tick()
{
	if (++time == 10)
	{
		time = 0;

		for (int i = 0; i < map_size; ++i)
		{
			unsigned char &g = grass_map[i];
			int odds = g < get_grasslimit() ? 31 : 63;
			int growth = (rand() & odds) == 0 ? 1 : 0; // chance of growth
			if (grass_map[i] + growth > 255)
				grass_map[i] = 255;
			else
				grass_map[i] += growth;
		}
	}

	// Make all entities think
	for (typeof(entities.begin()) e = entities.begin(); e != entities.end();
			++e)
		e->second->tick();
}

int World::difference(int a, int b)
{
	int c = (a - b) & map_mask; // Take the difference
	if (c & (1 << (map_bits - 1)))
		c = ~map_mask | c; // Fill with ones to the left if necessary
	return c;
}

int World::distance(int ax, int ay, int bx, int by)
{
	int dx = difference(ax, bx);
	int dy = difference(ay, by);

	if (dx < 0)
		dx = -dx;
	if (dy < 0)
		dy = -dy;

	return dx + dy;
}

unsigned char & World::grass(int x, int y)
{
	return grass_map[((y & map_mask) << map_bits) + (x & map_mask)];
}

unsigned char World::grass(int x, int y) const
{
	return grass_map[((y & map_mask) << map_bits) + (x & map_mask)];
}

void World::erode(int x, int y)
{
	unsigned char &g = grass(x, y);
	if (g > 0)
		--g;
}

inline int World::get_grasslimit() const
{
	static const int grasslimit = ceil(4 * (256 / 11.0));
	return grasslimit;
}

// Save to file
std::ostream& operator <<(std::ostream &os, World &w)
{
	// Header
	os << "version 1\n";
	os << w.entities.size() << std::endl;

	// Entities
	for (typeof(w.entities.begin()) e = w.entities.begin();
			e != w.entities.end(); ++e)
	{
		os << w.id_lookup[e->second] << " " << e->second->get_type() << " ";
		e->second->pack(os);
		os << std::endl;
	}

	// Grass
	for (int i = 0; i < w.map_size; ++i)
		os << (int) w.grass_map[i] << " ";

	return os;
}

// Load from file
std::istream& operator >>(std::istream &is, World &w)
{
	// Header
	std::string line;
	std::getline(is, line);
	if (line == "version 1\r")
		throw "invalid line endings, please convert using dos2unix";
	if (line != "version 1")
		throw "invalid file or version";

	// Entities
	// Remove all old entities
	w.clear();
	w.auto_id = 1;

	// Store string for second pass
	std::map<Entity*, std::string> secondpass;

	int num_ents = 0;
	is >> num_ents;
	for (int i = 0; i < num_ents; ++i)
	{
		unsigned int id;
		std::string type;
		is >> id >> type;
		std::getline(is, line);

		Entity *ent = w.create_entity(type, line);
		ent->world = &w;
		w.entities[id] = ent;
		w.id_lookup[ent] = id;
		secondpass[ent] = line;

		if (id >= w.auto_id)
			w.auto_id = id + 1;
	}

	// Grass
	for (int i = 0; i < w.map_size; ++i)
	{
		int g;
		is >> g;
		w.grass_map[i] = g;
	}

	// Make a second pass to allow entities to look up pointers from id
	for (typeof(secondpass.begin()) e = secondpass.begin();
			e != secondpass.end(); ++e)
	{
		std::stringstream ss(e->second);
		e->first->unpack(ss);
	}

	return is;
}

Entity* World::create_entity(std::string type, std::string line)
{
	typeof(factories.begin()) it = factories.find(type);
	if (it == factories.end())
		return 0;

	return (this->*(it->second))(line);
}

Entity* World::create_entity_apple(std::string line)
{
	int x;
	int y;
	int garbage;
	int color;
	std::stringstream ss(line);
	ss >> x >> y >> garbage >> garbage >> color;
	return new Apple(x, y, color);
}

Entity* World::create_entity_backpack(std::string line)
{
	int x;
	int y;
	int garbage;
	int capacity;
	std::stringstream ss(line);
	ss >> x >> y >> garbage >> garbage >> capacity;
	return new Backpack(x, y, capacity);
}

Entity* World::create_entity_door(std::string line)
{
	int x;
	int y;
	int garbage;
	bool locked;
	int lock_id;
	std::stringstream ss(line);
	ss >> x >> y >> garbage >> garbage >> locked >> lock_id;
	return new Door(x, y, locked, lock_id);
}

Entity* World::create_entity_human(std::string line)
{
	int x;
	int y;
	std::stringstream ss(line);
	ss >> x >> y;
	return new Human(x, y);
}

Entity* World::create_entity_key(std::string line)
{
	int x;
	int y;
	int garbage;
	int color;
	int key_id;
	std::stringstream ss(line);
	ss >> x >> y >> garbage >> garbage >> color >> key_id;
	return new Key(x, y, color, key_id);
}

Entity* World::create_entity_rabbit(std::string line)
{
	int x;
	int y;
	int garbage;
	bool male;
	std::stringstream ss(line);
	ss >> x >> y >> garbage >> garbage >> male;
	return new Rabbit(x, y, male);
}

Entity* World::create_entity_rock(std::string line)
{
	int x;
	int y;
	std::stringstream ss(line);
	ss >> x >> y;
	return new Rock(x, y);
}

Entity* World::create_entity_sword(std::string line)
{
	int x;
	int y;
	std::stringstream ss(line);
	ss >> x >> y;
	return new Sword(x, y);
}

}
