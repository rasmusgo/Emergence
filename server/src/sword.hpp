#pragma once

#include "entity.hpp"

namespace EmergenceServer
{

class Sword: public Entity
{
public:
    Sword(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
    virtual int get_w() const
    {
        return 48;
    }
    virtual int get_h() const
    {
        return 48;
    }
    virtual int get_tex() const
    {
        return 40;
    }
    virtual int get_weight() const
    {
        return 2000;
    } // 2 kg

    virtual void use(Entity *user, Entity *target)
    {
        if (target != 0 && user->world == target->world)
        {
            int distance = this->world->distance(this->get_x(), this->get_y(),
                    target->get_x(), target->get_y());
            if (distance <= 1)
                target->damage(10, user, this);
        }
    }

    virtual std::string get_type()
    {
        return "sword";
    }
protected:
private:
};

} // EmergenceServer
