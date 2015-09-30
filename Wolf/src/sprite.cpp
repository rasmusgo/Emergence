#include "SDL.h"

#include "graphics.hpp"
#include "sprite.hpp"

#include <map>
#include <math.h>

namespace WolfSheepClient
{
    Sprite::Sprite() :
            x(0),
            y(0),
            z(0),
            w(64),
            h(64),
            tex(0),
            x0(x),
            y0(y),
            t(0)
    {

    }

    Sprite::Sprite(const Sprite &s) :
            x(s.x),
            y(s.y),
            z(s.z),
            w(s.w),
            h(s.h),
            tex(s.tex),
            x0(s.x0),
            y0(s.y0),
            t(s.t)
    {

    }

    Sprite& Sprite::operator = (const Sprite &s)
    {
        x = s.x;
        y = s.y;
        z = s.z;
        w = s.w;
        h = s.h;
        tex = s.tex;
        x0 = s.x0;
        y0 = s.y0;
        t = s.t;
        return *this;
    }

    Sprite::Sprite(int x, int y, int z, int w, int h, unsigned int tex) :
            x(x),
            y(y),
            z(z),
            w(w),
            h(h),
            tex(tex),
            x0(x),
            y0(y),
            t(0)
    {

    }

    void Sprite::render()
    {
        char name[256];
        sprintf(name, "data/entity%03d.png", tex);
        draw_quad(get_x() - w/2, get_y() - h/2, w, h, get_texture(name));
        t -= 0.125;
        if (t < 0)
            t = 0;
    }

    void Sprite::move(int _x, int _y, int _z, bool smooth)
    {
        x0 = get_x();
        y0 = get_y();
        x = _x;
        y = _y;
        z = _z;
        t = smooth ? 1 : 0;
    }

    void Sprite::nudge(int _x, int _y, bool smooth)
    {
        x0 = get_x();
        y0 = get_y();
        x -= _x;
        y -= _y;
        t = smooth ? 1 : 0;
    }

    void Sprite::change_image(int _w, int _h, unsigned int _tex)
    {
        w = _w;
        h = _h;
        tex = _tex;
    }

    int Sprite::get_x() const
    {
        return floor(x*(1.0-t) + x0*t + 0.5);
    }

    int Sprite::get_y() const
    {
        return floor(y*(1.0-t) + y0*t + 0.5);
    }

    bool Sprite::operator < (const Sprite &b) const
    {
        return y+3*z < b.y+3*b.z;
    }
}
