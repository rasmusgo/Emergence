#ifndef ROCK_H
#define ROCK_H

#include "entity.h"

namespace WolfSheepServer
{

class Rock: public Entity
{
public:
	Rock(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	virtual int get_tex() const
	{
		return 50;
	}
	virtual int get_weight() const
	{
		return 100000;
	} // 100 kg
	virtual bool is_blocking() const
	{
		return true;
	}

	virtual std::string get_type()
	{
		return "rock";
	}
protected:
private:
};

}
#endif // ROCK_H
