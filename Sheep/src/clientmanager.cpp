#include "SDL_net.h"

#include <string>
#include <stdlib.h>
#include <fstream>

#include "clientmanager.h"
#include "entity.h"
#include "human.h"
#include "backpack.h"
#include "apple.h"
#include "rabbit.h"
#include "rock.h"
#include "key.h"
#include "door.h"
#include "sword.h"

namespace WolfSheepServer
{
    ClientManager::ClientManager(int port) :
            tcpsock(0),
            thread(0),
            done(false),
            need_to_save(false),
            need_to_load(false)
    {
        // Create dummy world
        world = new World();
        world->add_entity(new Sword(6, 0));
        world->add_entity(new Sword(6, 0));
        world->add_entity(new Sword(6, 0));
        world->add_entity(new Backpack(5, 0, 3));
        world->add_entity(new Backpack(5, 0, 3));
        world->add_entity(new Backpack(5, 0, 3));
        world->add_entity(new Apple(4, 2, 0));
        world->add_entity(new Apple(5, 2, 1));
        world->add_entity(new Apple(4, 3, 2));

        world->add_entity(new Key(4, -3, 0, 0xf00ba));
        world->add_entity(new Key(5, -3, 3, 0x7001));
        world->add_entity(new Door(4, -1, true, 0xf00ba));

        world->add_entity(new Rabbit(16, 12, true));
        world->add_entity(new Rabbit(19, 6, true));
        world->add_entity(new Rabbit(15, 6, false));
        world->add_entity(new Rabbit(8, 8, false));

        world->add_entity(new Rock(-5, -2));
        world->add_entity(new Rock(-3, -2));
        world->add_entity(new Rock(-4, -1));
        world->add_entity(new Rock(-3, -1));
        world->add_entity(new Rock(-4, 0));

        world->add_entity(new Rock(-3, 2));
        world->add_entity(new Rock(-4, 2));
        world->add_entity(new Rock(-3, 3));

        world->add_entity(new Rock(-5, 4));
        world->add_entity(new Rock(-5, 5));
        world->add_entity(new Rock(-4, 5));
        world->add_entity(new Rock(-5, 6));

        for (int i = 0; i < 100; ++i)
        {
            world->add_entity(new Rock(-3, -2));
        }

        // create a listening TCP socket on port 9999 (server)
        IPaddress ip;

        // Fill in the address and port
        if (SDLNet_ResolveHost(&ip, NULL, 9999) < 0)
        {
            printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
            exit(3);
        }

        // Open the socket
        tcpsock = SDLNet_TCP_Open(&ip);
        if (!tcpsock)
        {
            printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
            exit(4);
        }

        printf("Listening on port %d\n", SDL_SwapBE16(ip.port));

        thread = SDL_CreateThread(server_thread_func, this);
    }

    ClientManager::~ClientManager()
    {
        done = true;
        SDL_WaitThread(thread, NULL);
        delete world;
    }

    void ClientManager::reset()
    {
        world->reset();
    }

    void ClientManager::print_status()
    {
        printf("Status: %lu entities including %lu clients\n", world->get_entities().size(), clients.size());
    }

    void ClientManager::print_entities()
    {
        printf("Printing %lu entities:\n", world->get_entities().size());

        printf("   id   x   y   z\n");
        for (typeof(world->get_entities().begin()) e = world->get_entities().begin(); e != world->get_entities().end(); ++e)
        {
            printf("%5d %3d %3d %3d\n",
                e->first,
                e->second->world->difference(e->second->get_x(), 0),
                e->second->world->difference(e->second->get_y(), 0),
                e->second->get_offset_z());
        }
    }

    void ClientManager::print_clients()
    {
        printf("Printing %lu clients:\n", clients.size());

        printf("   id   x   y   name\n");
        for (typeof(clients.begin()) it = clients.begin(); it != clients.end(); ++it)
        {
            printf("%5d %3d %3d   %s\n",
                (*it)->get_id(),
                world->difference((*it)->get_x(), 0),
                world->difference((*it)->get_y(), 0),
                (*it)->get_name().c_str());
        }
    }

    /* Listen for new clients
     */
    int ClientManager::server_thread_func(void *data)
    {
        return ((ClientManager*)data)->server_thread_func();
    }

    int ClientManager::server_thread_func()
    {
        while (!done)
        {
            if (need_to_save)
            {
                need_to_save = false;
                std::ofstream out("savestate00.txt", std::ios::out);
                if(!out)
                    printf("Cannot open file.\n");
                else
                {
                    out << users.size() << std::endl;
                    for (typeof(users.begin()) it = users.begin(); it != users.end(); ++it)
                    {
                        out << it->second << " " << it->first << std::endl;
                    }
                    out << *world;
                    out.close();
                    printf("Saved.\n");
                }
            }

            if (need_to_load)
            {
                need_to_load = false;
                printf("trying to open file\n");
                std::ifstream in("savestate00.txt", std::ios::in);
                if(!in)
                    printf("Cannot open file.\n");
                else
                {
                    printf("clearing users\n");
                    users.clear();
                    int usercount=0;
                    printf("reading users\n");
                    // Read number of clients
                    in >> usercount;
                    // Read one line for each user
                    for (int i=0; i < usercount; ++i)
                    {
                        Uint32 id;
                        std::string name;
                        in >> id;
                        in.ignore(1);
                        std::getline(in, name);
                        users[name] = id;
                    }
                    printf("loading world\n");
                    try
                    {
                    	in >> *world;
                    }
                    catch (const char *e)
                    {
                    	printf("ERROR: '%s'\n", e);
                    	exit(-1);
                    }

                    printf("closing file\n");
                    in.close();

                    printf("trying to update clients\n");
                    for (typeof(clients.begin()) it = clients.begin(); it != clients.end(); ++it)
                    {
                        (*it)->update_entity_connection();
                    }

                    printf("Loaded.\n");
                }
            }

            // accept a connection coming in on server_tcpsock
            TCPsocket new_tcpsock;
            new_tcpsock = SDLNet_TCP_Accept(tcpsock);

            if (!new_tcpsock)
            {
                // No new client
                //printf("SDLNet_TCP_Accept: %s\n", SDLNet_GetError());
            }
            else
            {
                add_client(new_tcpsock);
            }

            // Loop through all clients and handle messages sent from the client
            for (typeof(clients.begin()) it = clients.begin(); it != clients.end(); ++it)
            {
                // Remove the client if the status is bad
                if ( (*it)->get_status() != 0 )
                {
                    printf("client %s disconnected\n", (*it)->get_name().c_str() );
                    delete *it;
                    it = clients.erase(it);
                    --it;
                    continue;
                }

                (*it)->process_messages();
            }

            // Send data to clients
            for (typeof(clients.begin()) it = clients.begin(); it != clients.end(); ++it)
            {
                // Scroll the world
                (*it)->send_scroll();

                // Send entities
                for (typeof(world->get_entities().begin()) e = world->get_entities().begin(); e != world->get_entities().end(); ++e)
                    (*it)->send_entity(e->first, e->second);

                // Remove those wich were sent before but not now
                (*it)->send_entity_removals();

                // Send hud
                (*it)->send_hud();

                // Send world info
                char data[1 + 17*17];
                const int len = sizeof(data);
                data[0] = 'g';
                int i = 1;

                Entity *ent = (*it)->get_entity();
                int player_x = ent->get_x();
                int player_y = ent->get_y();

                for (int y = -8; y <= 8; ++y)
                    for (int x = -8; x <= 8; ++x)
                        data[i++] = ent->world->grass(x+player_x, y+player_y);

                (*it)->send(data, len);
            }

            SDL_Delay(100); // Minimize cpu wastage

            world->tick();
        } // end main loop

        SDLNet_TCP_Close(tcpsock);

        // Close all sockets
        for (typeof(clients.begin()) it = clients.begin(); it != clients.end(); ++it)
            delete *it;
        clients.clear();
        return 0;
    }

    void ClientManager::add_client(TCPsocket new_tcpsock)
    {
        // New client
        IPaddress *client_address = SDLNet_TCP_GetPeerAddress(new_tcpsock);
        printf("Client connected from %s\n", SDLNet_ResolveIP(client_address));
        // communicate over new_tcpsock

        // Create new client
        ClientConnection *client_connection = new ClientConnection(new_tcpsock, this);
        clients.push_back(client_connection);
    }

    void ClientManager::save()
    {
        need_to_save = true;
    }

    void ClientManager::load()
    {
        need_to_load = true;
    }

    Uint32 ClientManager::get_user_id(std::string name)
    {
        Uint32 &id = users[name];
        if (world->get_entity_from_id(id) == 0)
        {
            // Create new user
            Entity *ent = new Human(0,0);
            id = world->add_entity(ent);
        }
        return id;
    }

    Entity* ClientManager::get_entity_from_id(Uint32 id)
    {
        return world->get_entity_from_id(id);
    }
}

