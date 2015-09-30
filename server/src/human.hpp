#pragma once

#include "entity.hpp"

namespace WolfSheepServer
{

class Human: public Entity
{
public:
	Human(int x, int y);
	virtual ~Human();
	virtual void action();
	virtual void grab();
	virtual void drop();
	virtual void swap();
	virtual void use(Entity *user, Entity *target);
	virtual void tick();
	virtual int get_offset_x(const Entity *ent) const;
	virtual int get_offset_y(const Entity *ent) const;
	virtual int get_offset_z(const Entity *ent) const;
	virtual int get_w() const;
	virtual int get_h() const;
	virtual int get_tex() const;
	virtual int get_weight() const;
	virtual std::vector<HudIcon>& get_hud_icons();
	virtual void remove_reference(const Entity *ent);
	virtual std::string get_type()
	{
		return "human";
	}
	virtual void pack(std::ostream&);
	virtual void unpack(std::istream&);
protected:
	Entity *in_primary_hand;
	Entity *in_secondary_hand;
	Entity *on_back;
	int health;
	bool male;
	std::vector<Entity::HudIcon> hud_icons;
private:
};

} // WolfSheepServer
