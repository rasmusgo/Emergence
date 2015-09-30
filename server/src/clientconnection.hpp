#pragma once

#include "SDL.h"
#include "SDL_net.h"

#include "entity.hpp"
#include "binding.hpp"

#include <deque>
#include <string>
#include <map>
#include <set>

namespace WolfSheepServer
{
// Forward declaration (instead of include)
class ClientManager;

class ClientConnection
{
	// TODO: create or forbid copy-constructor and assignment operator
public:
	ClientConnection(TCPsocket sock, ClientManager *manager);
	~ClientConnection();

	void send(const void *data, int len);
	void send_scroll();
	void send_entity(Uint32, const Entity *ent);
	void send_entity_removals();
	void send_hud();

	static int send_thread_func(void *data)
	{
		return ((ClientConnection*)data)->send_thread_func();
	}

	int send_thread_func();

	/* Listen for client commands
	 */
	static int recieve_thread_func(void *data)
	{
		return ((ClientConnection*)data)->recieve_thread_func();
	}

	int recieve_thread_func();

	void process_messages();

	int get_status();

	int get_id() const { return id; }
	std::string get_name() const { return name; }
	int get_x() const;
	int get_y() const;
	Entity* get_entity() { return entity; }

	void update_entity_connection();

private:

	struct DataEntity
	{
		Uint16 id;
		Sint16 x;
		Sint16 y;
		Sint16 z;
		Sint16 w;
		Sint16 h;
		Uint16 tex;
		bool old;
	};

	struct DataPacket
	{
		int len;
		char *data;
	};

	struct ReceivedPacket
	{
		SDLKey key;
		bool released;
		Uint32 arrival_time;
	};

	ReceivedPacket pop_message();

	TCPsocket socket;
	int status; // 0 == OK	 -1 = bad
	SDL_Thread *recieve_thread;
	SDL_Thread *send_thread;
	ClientManager *manager;

	std::string name;
	Uint16 id;
	Entity *entity;
	int old_x;
	int old_y;

	std::map<Uint32, DataEntity> client_entities;
	std::map<SDLKey, Uint32> active_keys;
	std::map<SDLKey, Binding> bindings;

	SDL_mutex *send_mutex;
	SDL_mutex *messages_mutex;

	std::deque<DataPacket> data_to_send;
	std::deque<ReceivedPacket> messages;
};

} // WolfSheepServer
