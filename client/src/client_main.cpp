#include <cstdlib>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_net.h>

#include "client.hpp"
#include "graphics.hpp"

#include "oglconsole.h"

namespace WolfSheepClient
{
    static bool done = false;
    static Client *client = 0;

	void quit()
	{
        // Disconnect client
        delete client;
        client = 0;

	    OGLCONSOLE_Quit();
		SDLNet_Quit();
		SDL_Quit();
	}

	void cmdCB(OGLCONSOLE_Console console, char *cmd)
    {
        // TODO: Here would be a good place to use a map with member function pointers
        if (!strncmp(cmd, "quit", 4))
        {
            done = true;
            return;
        }
        else if (!strncmp(cmd, "connect", 7))
        {
            char address[256] = "localhost";
            int port = 9999;
            if (sscanf(cmd, "connect %255s %i", address, &port) >= 0)
            {
                printf("connecting to %s:%i\n", address, port);
                OGLCONSOLE_Output(console, "connecting to %s %i\n", address, port);
                try
                {
                    delete client;
                    client = 0;
                    client = new Client(address, port);
                    OGLCONSOLE_SetVisibility(0);
                }
                catch (...)
                {
                    printf("failed to connect to %s:%i\n", address, port);
                    OGLCONSOLE_Output(console, "failed to connect to %s:%i\n", address, port);
                    delete client;
                    client = 0;
                }
                return;
            }

            OGLCONSOLE_Output(console, "usage: connect [address [port]]\n");
            return;
        }
        else if (!strncmp(cmd, "add", 3))
        {
            int a, b;
            if (sscanf(cmd, "add %i %i", &a, &b) == 2)
            {
                OGLCONSOLE_Output(console, "%i + %i = %i\n", a, b, a+b);
                return;
            }

            OGLCONSOLE_Output(console, "usage: add INT INT\n");
            return;
        }
        else if (!strncmp(cmd, "disconnect", 10))
        {
            delete client;
            client = 0;
            return;
        }

        OGLCONSOLE_Output(console, "I don't understand this command: %s\n", cmd);
    }
}

using namespace WolfSheepClient;

extern "C"
{
    void OGLCONSOLE_AddHistory(_OGLCONSOLE_Console *console, const char *s);
}

int main ( int argc, char** argv )
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // initialize SDL net
    if ( SDLNet_Init() < 0 )
    {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        return 2;
    }

    // Undo SDLs redirection
    #ifdef WIN32
    freopen("CON", "w", stdout); // redirects stdout
    freopen("CON", "w", stderr); // redirects stderr
    #endif

    // make sure SDL cleans up before exit
    atexit(quit);

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // create a new window
    int width = 800;
    int height = 600;
    int bpp = 16;
    SDL_Surface* screen = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL);
    if ( !screen )
    {
        printf("Unable to set %ix%i video: %s\n", width, height, SDL_GetError());
        return 1;
    }

    SDL_WM_SetCaption("Wolf Client","Wolf Client");

    init_opengl(width, height);

    // Initialize OGLCONSOLE
    OGLCONSOLE_Console console = OGLCONSOLE_Create();
    OGLCONSOLE_EnterKey(cmdCB);
    OGLCONSOLE_SetVisibility(1);
    OGLCONSOLE_AddHistory(console, "connect localhost");

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    SDL_GL_SwapBuffers();

    // Measure the time elapsed
    Uint32 t1 = SDL_GetTicks();
    Uint32 t0 = t1;

    // program main loop
    while (!done)
    {
        // DRAWING STARTS HERE

        // clear screen
        //SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        set_worldview(0, 0, width-200, height);

        // draw game
        if (client != 0)
            client->render();

        set_sideview(width-200, 0, 200, height);

        // draw hud
        if (client != 0)
            client->render_hud();

        // Set full view for console
        set_fullview(0, 0, width, height);

        // Render our console
        OGLCONSOLE_Draw();

        // draw status square
        static int r=0,g=0,b=0, d = 1;
        draw_quad(0,0,2,2, r/255.0,g,b);
        if (r <= 0)
            d = 1;
        if (r >= 255)
            d = -1;
        r += d;

        // DRAWING ENDS HERE

        // update the screen
        SDL_GL_SwapBuffers();

        SDL_Delay(10);
        // message processing loop
        t0 = t1;
        while (true)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                // Give the console first dibs on event processing
                if (OGLCONSOLE_SDLEvent(&event) == 0) switch( event.type )
                {
                    // exit if the window is closed
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_VIDEORESIZE:
                    // What now?
                    width = event.resize.w;
                    height = event.resize.h;
                    SDL_SetVideoMode(width, height, bpp, SDL_OPENGL | SDL_RESIZABLE); // Create new window
                    init_opengl(width, height);
                    break;
                    // check for keypresses
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    {
                        if (client != 0)
                        {
                            // Release is represented with negation
                            int key = event.key.state == SDL_PRESSED ? event.key.keysym.sym : -event.key.keysym.sym;
                            client->send(key);
                        }
                        break;
                    }
                } // end switch
            } // end of message processing

            if ( (t1 = SDL_GetTicks())-t0 < 15 )
                SDL_Delay(1);
            else
                break;
        }
    } // end main loop

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}

