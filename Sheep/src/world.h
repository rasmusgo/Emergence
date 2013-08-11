#ifndef WORLD_H
#define WORLD_H

#include "entity.h"

#include <map>
#include <vector>

namespace WolfSheepServer
{
    class World
    {
    public:

        World();
        ~World();

        int add_entity(Entity *ent);
        void remove_entity(Entity *ent);
        void delete_entity(Entity *ent);
        void reset();
        void clear();
        void tick();
        static int difference(int a, int b);
        static int distance(int ax, int ay, int bx, int by);
        unsigned char & grass(int x, int y);
        unsigned char grass(int x, int y) const;
        void erode(int x, int y);
        int get_grasslimit() const;

        const std::map<int, Entity*> & get_entities() const { return entities; };
        void get_entities_on(int x, int y, std::vector<Entity*> &container) const;
        bool is_blocked(int x, int y) const;

        friend std::ostream& operator << (std::ostream &os, World &world); // Save to file
        friend std::istream& operator >> (std::istream &is, World &world); // Load from file

        unsigned int get_id_from_entity(Entity *ent) const
        {
            typeof(id_lookup.begin()) it = id_lookup.find(ent);
            if (it == id_lookup.end())
                return 0;
            return it->second;
        }

        Entity* get_entity_from_id(unsigned int id) const
        {
            typeof(entities.begin()) it = entities.find(id);
            if (it == entities.end())
                return 0;
            return it->second;
        }

    protected:
        static const int map_bits = 6; // 5 bits give 32x32 tiles on the map
        static const int map_mask = (1<<map_bits)-1;
        static const int map_width = 1 << map_bits;
        static const int map_size = 1 << (map_bits+map_bits); // 32*32
        unsigned char grass_map[map_size];

        std::map<int, Entity*> entities;
        std::map<Entity*, int> id_lookup;
        unsigned int auto_id;
        int time;

        std::map<std::string, Entity*(World::*)(std::string)> factories;

        Entity* create_entity(std::string type, std::string line);
        Entity* create_entity_apple(std::string line);
        Entity* create_entity_backpack(std::string line);
        Entity* create_entity_door(std::string line);
        Entity* create_entity_human(std::string line);
        Entity* create_entity_key(std::string line);
        Entity* create_entity_rabbit(std::string line);
        Entity* create_entity_rock(std::string line);
        Entity* create_entity_sword(std::string line);
    };
}

#endif
