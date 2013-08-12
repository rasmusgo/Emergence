#ifndef ENTITY_H
#define ENTITY_H

#include "SDL.h"

#include <iostream>
#include <string>
#include <vector>

namespace WolfSheepServer
{
class World;

class Entity
{
public:
	Entity();
	virtual ~Entity();
	virtual void prepare_action(); // triggered before action (pressing down SPACE)
	virtual void prepare_grab(); // triggered before grab (pressing down SHIFT)
	virtual void prepare_drop(); // triggered before drop (pressing down CTRL)
	virtual void prepare_swap(); // triggered before swap (pressing down ALT)
	virtual void action(); // triggered by SPACE
	virtual void grab(); // triggered by SHIFT
	virtual void drop(); // triggered by CTRL
	virtual void swap(); // triggered by ALT
	virtual void use(Entity *user, Entity *target) {}
	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void tick() {}
	//virtual std::string name() = 0;
	virtual int get_x() const;
	virtual int get_y() const;
	virtual int get_z() const;
	// The first offset is how far ent is offseted by this
	virtual int get_offset_x(const Entity *ent) const;
	virtual int get_offset_y(const Entity *ent) const;
	virtual int get_offset_z(const Entity *ent) const;
	// The second offset is accumulated offset for this
	virtual int get_offset_x() const;
	virtual int get_offset_y() const;
	virtual int get_offset_z() const;
	virtual int get_w() const
	{
		return 64;
	}
	virtual int get_h() const
	{
		return 64;
	}
	virtual int get_tex() const = 0;
	virtual int get_weight() const = 0; // weight in grams, including any carried entities
	virtual int get_direction() const;
	virtual bool is_blocking() const
	{
		return false;
	}

	// damage gets called when an entity is getting hurt by another entity
	virtual void damage(int dmg, Entity *user, Entity *weapon) {}

	/* The grabber calls grab_me on the grabbed entity
	 * when the grabber picks it up
	 * The grabbed entity can return itself, another entity or null
	 */
	virtual Entity* grab_me(Entity *grabber);
	virtual void drop_me();

	/* mount is a request to sit on an entity
	 * eg. a human wants to put a backpack on his back
	 * In human: backpack->mount(human)
	 * returns true if the mount was successful
	 */
	virtual bool mount(Entity *ent)
	{
		return false;
	}
	virtual bool unmount()
	{
		return true;
	}

	/* The parent remove all references to the entity
	 * so it can be removed from the world.
	 * All entities that keep referenses to other entities
	 * must implement this.
	 */
	virtual void remove_reference(const Entity *ent)
	{
	}

	struct HudIcon
	{
		int x;
		int y;
		int width;
		int height;
		int tex;
	};

	struct HudText
	{
		int x;
		int y;
		int width;
		int height;
		char text[64];
	};

	virtual std::vector<HudIcon>& get_hud_icons()
	{
		static std::vector<HudIcon> hud_icons;
		hud_icons.clear();

		HudIcon icon = { 0, 0, 64, 64, get_tex() };
		hud_icons.push_back(icon);

		return hud_icons;
	}

	virtual std::string get_type() = 0;
	virtual void pack(std::ostream &os);
	virtual void unpack(std::istream &is);

	// forward direction
	static const int dx[];
	static const int dy[];

	World *world;
protected:
	Entity *parent;
	Uint16 x;
	Uint16 y;
	short direction;
	short prepared;

	friend std::ostream& operator <<(std::ostream &os, const Entity &e);
private:
};

typedef void (Entity::*EntityMemberFunction)();
}

#endif // ENTITY_H
