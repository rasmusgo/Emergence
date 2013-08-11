#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "SDL_net.h"

#include "graphics.h"
#include "oglconsole.h"
#include "sprite.h"
#include "client.h"

#include <list>
#include <map>
#include <math.h>

// TODO: network statistics

namespace WolfSheepClient
{
    Client::Client(const char *host, Uint16 port) :
		listener_thread(0),
		mutex(0),
		dx(0),
		dy(0),
		t(0)
    {
        for (int i = 0; i < 32*32; ++i)
            m_grass[i] = 0;

        // connect to localhost at port 9999 using TCP (client)
        IPaddress ip;
        if (SDLNet_ResolveHost(&ip, host, port) < 0)
        {
            printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            throw("Failed to resolve host");
        }

        sock = SDLNet_TCP_Open(&ip);
        if (!sock)
        {
            printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
            throw("Failed to open socket");
        }

        mutex = SDL_CreateMutex();
        listener_thread = SDL_CreateThread(listen, this);
    }

    void Client::send(Uint16 number)
    {
        char data[2];
        const int len = sizeof(data);
        SDLNet_Write16(number, (Uint16 *)data); // to network byte order

        int result = SDLNet_TCP_Send(sock, data, len);
        if (result < len)
        {
            printf("%s:%d SDLNet_TCP_Send: %s\n", __FILE__, __LINE__, SDLNet_GetError());
            // It may be good to disconnect sock because it is likely invalid now.
        }
    }

    void Client::recieve(void *data, int len)
    {
        int recieved = 0;
        while (recieved < len)
        {
            int result = SDLNet_TCP_Recv(sock, (unsigned char*)(data)+recieved, len-recieved);
            if (result <= 0)
            {
                if (result == 0)
                    printf("%s:%d SDLNet_TCP_Recv: Connection closed by peer\n", __FILE__, __LINE__);
                else
                    printf("%s:%d SDLNet_TCP_Recv: %s\n", __FILE__, __LINE__, SDLNet_GetError());
                throw "socket problem";
            }
            recieved += result;
        }
    }

    void Client::render()
    {
        SDL_LockMutex(mutex);

        static const int grasslimit = ceil(4*(256/11.0));
        int tiles[18*18];
        int i = 0;
        for (int y = 0; y < 18; ++y)
        {
            for (int x = 0; x < 18; ++x)
            {
                unsigned char a0 = grass(x,   y);
                unsigned char a1 = grass(x+1, y);
                unsigned char a2 = grass(x,   y+1);
                unsigned char a3 = grass(x+1, y+1);
                tiles[i++] = (a0 >= grasslimit ? 1 : 0) +
                             (a1 >= grasslimit ? 2 : 0) +
                             (a2 >= grasslimit ? 4 : 0) +
                             (a3 >= grasslimit ? 8 : 0);
            }
        }

        int overlay[17*17];
        i = 0;
        for (int y = 1; y < 18; ++y)
        {
            for (int x = 1; x < 18; ++x)
            {
                overlay[i++] = (int)(grass(x,y))*(11.0/256) + 100; // distribute over 11 images starting at index 100
            }
        }

        draw_quad(-512, -512, 1024, 1024, bgcolor_r, bgcolor_g, bgcolor_b);
        draw_tiles(-512-64 + dx*t, -512-64 + dy*t, 64, 64, tiles, 18, 18);
        draw_tiles(-512-32 + dx*t, -512-32 + dy*t, 64, 64, overlay, 17, 17);

        std::list<Sprite*> sprites;
        for (typeof(entities.begin()) it = entities.begin(); it != entities.end(); ++it)
            sprites.push_back(&(it->second));

        sprites.sort(Sprite::less);

        for (typeof(sprites.begin()) it = sprites.begin(); it != sprites.end(); ++it)
            (*it)->render();

        //for (typeof(entities.begin()) it = entities.begin(); it != entities.end(); ++it)
        //    it->second.render();


        t -= 0.125;
        if (t < 0)
            t = 0;

        SDL_UnlockMutex(mutex);
    }

    void Client::render_hud()
    {
        SDL_LockMutex(mutex);

        for (typeof(hud_sprites.begin()) it = hud_sprites.begin(); it != hud_sprites.end(); ++it)
            it->second.render();

        SDL_UnlockMutex(mutex);
    }

    Client::~Client()
    {
    	if (listener_thread != 0)
			SDL_KillThread(listener_thread);

        // close the connection on sock
        SDLNet_TCP_Close(sock);

        if (mutex != 0)
			SDL_DestroyMutex(mutex);
    }

    int Client::listen(void *data)
    {
    	Client *self = (Client*)data;
    	return self->listen();
    }

    int Client::listen()
    {
        // map av metod-pekare

        // Type of callback methods a pointer to a method of Client that recieves a char
        typedef void (Client::*msg_callback)(char msg);
        std::map<char, msg_callback> callbacks;
        //std::map<char, void (Client::*)(char)> callbacks;

        callbacks['g'] = &Client::msg_grass;
        callbacks['a'] = &Client::msg_add;
        callbacks['A'] = &Client::msg_add;
        callbacks['r'] = &Client::msg_remove;
        callbacks['R'] = &Client::msg_remove;
        callbacks['m'] = &Client::msg_move;
        callbacks['M'] = &Client::msg_move;
        callbacks['s'] = &Client::msg_scroll;
        callbacks['S'] = &Client::msg_scroll;
        callbacks['i'] = &Client::msg_change_image;
        callbacks['I'] = &Client::msg_change_image;
        callbacks['b'] = &Client::msg_background_color;
        callbacks['B'] = &Client::msg_background_color;
        callbacks['h'] = &Client::msg_hud;
        callbacks['c'] = &Client::msg_clear;

        char last_msg = ' ';
        try
        {
            for(;;)
            {
		        char msg;
		        recieve(&msg, 1);

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

                typeof(callbacks.begin()) it = callbacks.find(msg);
                if (it != callbacks.end())
                {
                    last_msg = msg;
                    (this->*(it->second))(msg);
                    //CALL_MEMBER_FN(*this, it->second)(msg);
                }
                else
                    printf("unknown packet type: %c (%d) after last known command: %c\n", msg, (int)((unsigned char)(msg)), last_msg);
		    }
		}
        catch (...)
        {
            return 0;
        }
    }

    void Client::msg_grass(char msg)
    {
        unsigned char data[17*17];

        // Recieve all tiles
        recieve(data, sizeof(data));

        SDL_LockMutex(mutex);
        // Extrapolate edges
        /*
        for (int x = 0; x < 17; ++x)
            grass[x + 1] = grass[19 + x + 1];
        for (int x = 0; x < 17; ++x)
            grass[18*19 + x + 1] = grass[17*19 + x + 1];

        for (int y = 0; y < 19; ++y)
            grass[y*19] = grass[y*19 + 1];
        for (int y = 0; y < 19; ++y)
            grass[y*19+18] = grass[y*19 + 17];
        */

        // Write new tiles
        int i = 0;
        for (int y = 1; y < 18; ++y)
        {
            for (int x = 1; x < 18; ++x)
            {
                grass(x, y) = data[i++];
            }
        }

        SDL_UnlockMutex(mutex);
    }

    void Client::msg_add(char msg)
    {
        Uint16 data[7];

        // read new entity
        recieve(data, sizeof(data));

        Uint16 id = SDLNet_Read16(data);
        Sprite s((Sint16) SDLNet_Read16(data+1),
                 (Sint16) SDLNet_Read16(data+2),
                 (Sint16) SDLNet_Read16(data+3),
                 (Sint16) SDLNet_Read16(data+4),
                 (Sint16) SDLNet_Read16(data+5),
                 SDLNet_Read16(data+6));

        SDL_LockMutex(mutex);
        entities[id] = s;
        SDL_UnlockMutex(mutex);

        //printf("entity %d with texture %d added on (%d,%d)\n", id, e.tex, e.x, e.y);
    }

    void Client::msg_remove(char msg)
    {
        Uint16 data[1];
        recieve(data, sizeof(data));

        SDL_LockMutex(mutex);
        entities.erase(SDLNet_Read16(data));
        SDL_UnlockMutex(mutex);

        //printf("entity %d with texture %d removed\n", (int)data[0], (int)data[5]);
    }

    void Client::msg_move(char msg)
    {
        Uint16 data[4];
        recieve(data, sizeof(data));

        Uint16 id = SDLNet_Read16(data);
        Sint16 x = (Sint16) SDLNet_Read16(data+1);
        Sint16 y = (Sint16) SDLNet_Read16(data+2);
        Sint16 z = (Sint16) SDLNet_Read16(data+3);

        SDL_LockMutex(mutex);
        entities[id].move(x, y, z, msg == 'M');
        SDL_UnlockMutex(mutex);
    }

    void Client::msg_scroll(char msg)
    {
        Uint16 data[2];
        recieve(data, sizeof(data));

        Sint16 x = (Sint16) SDLNet_Read16(data);
        Sint16 y = (Sint16) SDLNet_Read16(data+1);

        scrollx += x;
        scrolly += y;

        SDL_LockMutex(mutex);
        dx = dx*t + x*64;
        dy = dy*t + y*64;
        if (msg == 'S')
            t = 1;
        else
            t = 0;

        for (typeof(entities.begin()) it = entities.begin(); it != entities.end(); ++it)
            it->second.nudge(x*64, y*64, msg == 'S');

        SDL_UnlockMutex(mutex);
    }

    void Client::msg_change_image(char msg)
    {
        Uint16 data[4];
        recieve(data, sizeof(data));

        Uint16 id = SDLNet_Read16(data);
        Sint16 w = (Sint16) SDLNet_Read16(data+1);
        Sint16 h = (Sint16) SDLNet_Read16(data+2);
        Sint16 tex = (Sint16) SDLNet_Read16(data+3);

        SDL_LockMutex(mutex);
        entities[id].change_image(w, h, tex);
        SDL_UnlockMutex(mutex);
    }

    void Client::msg_background_color(char msg)
    {
        char data[3];
        recieve(data, sizeof(data));

        SDL_LockMutex(mutex);
        bgcolor_r = ((unsigned char)data[0])/255.0f;
        bgcolor_g = ((unsigned char)data[1])/255.0f;
        bgcolor_b = ((unsigned char)data[2])/255.0f;
        SDL_UnlockMutex(mutex);
    }

    void Client::msg_hud(char msg)
    {
        Uint16 data[7];
        recieve(data, sizeof(data));

        Uint16 id = SDLNet_Read16(data);
        Sprite s((Sint16) SDLNet_Read16(data+1),
                 (Sint16) SDLNet_Read16(data+2),
                 (Sint16) SDLNet_Read16(data+3),
                 (Sint16) SDLNet_Read16(data+4),
                 (Sint16) SDLNet_Read16(data+5),
                 SDLNet_Read16(data+6));

        SDL_LockMutex(mutex);
        hud_sprites[id] = s;
        SDL_UnlockMutex(mutex);

        //printf("entity %d with texture %d added on (%d,%d)\n", id, e.tex, e.x, e.y);
    }

    void Client::msg_clear(char msg)
    {
        SDL_LockMutex(mutex);
        entities.clear();
        hud_sprites.clear();
        SDL_UnlockMutex(mutex);
    }
}
