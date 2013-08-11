#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "SDL_net.h"

#include "graphics.h"
#include "sprite.h"

#include <map>

namespace WolfSheepClient
{

class Client
{
    // Forbid copying
    Client(const Client &c);
    void operator = (const Client &c);

public:
    Client(const char *host, Uint16 port);
    ~ Client();

    void send(Uint16 number);
    void recieve(void *data, int len);
    void render();
    void render_hud();

private:
    static int listen(void *data);
    int listen();

    void msg_grass(char msg);
    void msg_add(char msg);
    void msg_remove(char msg);
    void msg_move(char msg);
    void msg_scroll(char msg);
    void msg_change_image(char msg);
    void msg_background_color(char msg);
    void msg_hud(char msg);
    void msg_clear(char msg);

    TCPsocket sock;
    SDL_Thread *listener_thread;
    SDL_mutex *mutex;

    // The following members are protected by mutex
    int m_grass[32*32];
    int & grass(int x, int y)
    {
        unsigned xi = x + scrollx;
        unsigned yi = y + scrolly;
        return m_grass[((yi&31)*32) + (xi&31)];
    }
    unsigned int scrollx;
    unsigned int scrolly;
    std::map<unsigned int, Sprite> entities;
    std::map<unsigned int, Sprite> hud_sprites;
    float bgcolor_r;
    float bgcolor_g;
    float bgcolor_b;
    int dx;
    int dy;
    float t;
};

}

#endif // CLIENT_H_INCLUDED
