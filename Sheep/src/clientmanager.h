#pragma once

#include "SDL_net.h"

#include "clientconnection.h"
#include "entity.h"
#include "world.h"

#include <vector>

namespace WolfSheepServer
{

class ClientManager
{
	// Forbid copy-constructor and assignment
	ClientManager(const ClientManager &cm);
	void operator =(const ClientManager &cm);

public:
	ClientManager(int port);
	~ClientManager();

	void reset();
	void print_status();
	void print_entities();
	void print_clients();
	void save();
	void load();

	Uint32 get_user_id(std::string name);
	Entity* get_entity_from_id(Uint32 id);

private:
	// Listen for client commands
	static int client_thread_func(void *data);

	// Listen for new clients
	static int server_thread_func(void *data);
	int server_thread_func();

	void add_client(TCPsocket sock);

	TCPsocket tcpsock; // Socket for accepting new clients
	SDL_Thread *thread; // Listening for new clients
	std::vector<ClientConnection*> clients;
	std::map<std::string, Uint32> users;
	bool done;

	World *world;

	bool need_to_save;
	bool need_to_load;
};

} // WolfSheepServer
