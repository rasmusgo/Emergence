#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "SDL_net.h"

#include "graphics.h"

#include <map>

namespace WolfSheepClient
{

class Sprite
{
public:
    Sprite();
    Sprite(const Sprite &s);
    Sprite & operator = (const Sprite &s);
    Sprite(int x, int y, int z, int w, int h, unsigned int tex);

    void render();
    void move(int x, int y, int z, bool smooth);
    void nudge(int x, int y, bool smooth);
    void change_image(int w, int h, unsigned int tex);

    int get_x() const;
    int get_y() const;

    bool operator < (const Sprite &b) const;
    static bool less(const Sprite *a, const Sprite *b)
    {
        return (*a) < (*b);
    }

protected:
    int x;
    int y;
    int z;
    int w;
    int h;
    unsigned int tex;
    int x0;
    int y0;
    float t;
};

}

#endif // SPRITE_H_INCLUDED
