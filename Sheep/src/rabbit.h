#ifndef RABBIT_H
#define RABBIT_H

#include "entity.h"

namespace WolfSheepServer
{
class Rabbit: public Entity
{
	typedef void (Rabbit::*ai_callback)();
public:
	Rabbit(int x, int y, bool male);
	virtual ~Rabbit();
	virtual void prepare_action();
	virtual void action();
	virtual void tick();
	virtual int get_w() const;
	virtual int get_h() const;
	virtual int get_tex() const;
	virtual int get_weight() const;

	virtual void damage(int dmg, Entity *user, Entity *weapon);

	// Called when another entity picks this entity up
	virtual Entity* grab_me(Entity *grabber);
	virtual void drop_me();

	virtual std::string get_type()
	{
		return "rabbit";
	}
	virtual void pack(std::ostream&);
	virtual void unpack(std::istream&);
protected:
	int health;
	bool male;
	int stomach;
	int age;
	int fear;
	int weight;
	int pregnant;

	ai_callback mood;
	void calm();
	void hungry();
	void panic();
	void dead();
private:
};

}
#endif // RABBIT_H
