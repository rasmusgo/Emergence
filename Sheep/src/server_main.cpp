#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <string>

#include "SDL_net.h"
#include "SDL_endian.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"

#include "clientmanager.h"

namespace WolfSheepServer
{
    void quit()
    {
        SDLNet_Quit();
        SDL_Quit();
    }
}

using namespace WolfSheepServer;

int main ( int argc, char** argv )
{
    // initialize SDL
    if ( SDL_Init(0) < 0 )
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

    // create a client manager listening on port 9999 (server)
    ClientManager *client_manager = new ClientManager(9999);

    char *cptr;
    char buffer[256];
    while (true)
    {
        cptr = fgets(buffer, 256, stdin);
        if(cptr != NULL) {
            if (strcmp("quit\n", cptr) == 0)
                break;
            else if (strcmp("reset\n", cptr) == 0)
                client_manager->reset();
            else if (strcmp("status\n", cptr) == 0)
                client_manager->print_status();
            else if (strcmp("entities\n", cptr) == 0)
                client_manager->print_entities();
            else if (strcmp("clients\n", cptr) == 0)
                client_manager->print_clients();
            else if (strcmp("save\n", cptr) == 0)
                client_manager->save();
            else if (strcmp("load\n", cptr) == 0)
                client_manager->load();
            else if (strcmp("restart\n", cptr) == 0)
            {
                delete client_manager; // We are done, signal server-thread to finish
                client_manager = new ClientManager(9999);
            }
            else if (strcmp("\n", cptr) != 0)
                printf("Unknown command: %s", cptr);
        }
        else
            break;
    }

    delete client_manager; // We are done, signal server-thread to finish
    client_manager = 0;

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
