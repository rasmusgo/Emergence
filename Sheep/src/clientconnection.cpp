#include "SDL.h"
#include "SDL_net.h"

#include "clientconnection.h"
#include "clientmanager.h"
#include "entity.h"
#include "world.h"
#include "binding.h"

#include <deque>
#include <string>

namespace WolfSheepServer
{

ClientConnection::ClientConnection(TCPsocket sock, ClientManager *manager) :
		socket(sock), status(0), manager(manager)
{
	printf("ClientConnection()\n");

	bindings[SDLK_LEFT]  = bindings[SDLK_a] = Binding(&Entity::left, 200);
	bindings[SDLK_RIGHT] = bindings[SDLK_d] = Binding(&Entity::right, 200);
	bindings[SDLK_UP]    = bindings[SDLK_w] = Binding(&Entity::up, 200);
	bindings[SDLK_DOWN]  = bindings[SDLK_s] = Binding(&Entity::down, 200);

	bindings[SDLK_SPACE] = bindings[SDLK_e] = Binding(&Entity::prepare_action, &Entity::action);
	bindings[SDLK_LSHIFT]= bindings[SDLK_f] = Binding(&Entity::prepare_grab, &Entity::grab);
	bindings[SDLK_LCTRL] = bindings[SDLK_g] = Binding(&Entity::prepare_drop, &Entity::drop);
	bindings[SDLK_LALT]  = bindings[SDLK_q] = Binding(&Entity::prepare_swap, &Entity::swap);

	name = SDLNet_ResolveIP(SDLNet_TCP_GetPeerAddress(socket));
	update_entity_connection();

	old_x = get_x();
	old_y = get_y();

	send_mutex = SDL_CreateMutex();
	messages_mutex = SDL_CreateMutex();

	send_thread = SDL_CreateThread(send_thread_func, this);
	recieve_thread = SDL_CreateThread(recieve_thread_func, this);

	char data[] = { 'b', 151, 130, 42 }; // set background
	int len = sizeof(data);
	send(data, len);
}

ClientConnection::~ClientConnection()
{
	// Signal to stop
	status = -1;
	SDL_WaitThread(send_thread, NULL);

	// We don't need to lock because the send_thread is terminated
	while (data_to_send.size() > 0)
	{
		delete data_to_send.front().data;
		data_to_send.pop_front();
	}

	// make sure we don't trash messages when killing the thread.
	SDL_LockMutex(messages_mutex);
	SDL_KillThread(recieve_thread);
	SDL_UnlockMutex(messages_mutex);

	SDLNet_TCP_Close(socket);

	SDL_DestroyMutex(send_mutex);
	SDL_DestroyMutex(messages_mutex);

	// Remove this client from the world
	//entity->world->delete_entity(entity);
	//entity = 0;
}

void ClientConnection::update_entity_connection()
{
	id = manager->get_user_id(name);
	entity = manager->get_entity_from_id(id);
}

int ClientConnection::get_x() const
{
	return entity->get_x(); // + Entity::dx[entity->get_direction()];
}

int ClientConnection::get_y() const
{
	return entity->get_y(); // + Entity::dy[entity->get_direction()];
}

void ClientConnection::send(const void *data, int len)
{
	if (len <= 0 || status != 0)
		return;

	// Create a new packet and put it in the queue
	DataPacket packet =
	{ len, new char[len] };
	memcpy(packet.data, data, len);

	SDL_LockMutex(send_mutex);
	data_to_send.push_back(packet);
	SDL_UnlockMutex(send_mutex);
}

void ClientConnection::send_scroll()
{
	int x = get_x();
	int y = get_y();
	int dx = World::difference(x, old_x);
	int dy = World::difference(y, old_y);
	old_x = x;
	old_y = y;

	for (typeof(client_entities.begin()) it = client_entities.begin(); it != client_entities.end(); ++it)
	{
		it->second.x -= dx;
		it->second.y -= dy;
	}

	static const int len = 5;
	DataPacket packet =
	{ len, new char[len] };

	packet.data[0] = 'S';
	SDLNet_Write16(dx, packet.data + 1);
	SDLNet_Write16(dy, packet.data + 3);

	SDL_LockMutex(send_mutex);
	data_to_send.push_back(packet);
	SDL_UnlockMutex(send_mutex);
}

/* Sends messages to remove old entities which were not sent since last call
 */
void ClientConnection::send_entity_removals()
{
	std::vector<Uint32> old_entities;

	for (typeof(client_entities.begin()) it = client_entities.begin(); it != client_entities.end(); ++it)
	{
		if (it->second.old)
		{
			// This entity is old and should be removed
			old_entities.push_back(it->first);

			static const int len = 3;
			DataPacket packet =
			{ len, new char[len] };

			packet.data[0] = 'r';
			SDLNet_Write16(it->second.id, packet.data + 1);

			SDL_LockMutex(send_mutex);
			data_to_send.push_back(packet);
			SDL_UnlockMutex(send_mutex);

			continue;
		}
		else
		{
			// Next time this will be old if it hasn't been seen again
			it->second.old = true;
		}
	}

	// Forget the old entities
	for (typeof(old_entities.begin()) it = old_entities.begin(); it != old_entities.end(); ++it)
		client_entities.erase(*it);
}

void ClientConnection::send_entity(Uint32 eid, const Entity *ent)
{
	DataEntity dent = {
			eid, ent->world->difference(ent->get_x(), get_x()) * 64 + ent->get_offset_x(),
			ent->world->difference(ent->get_y(), get_y()) * 64 + ent->get_offset_y(),
			ent->get_z() + ent->get_offset_z(),
			ent->get_w(),
			ent->get_h(),
			ent->get_tex(),
			false
	};

	// Check if the entity exists allready
	typeof(client_entities.begin()) it = client_entities.find(eid);

	if (it != client_entities.end())
	{
		DataEntity &itde = it->second;

		// Check bounds
		if ((dent.x + dent.w < -1024 || dent.x > 1024 || dent.y + dent.h < -1024
				|| dent.y > 1024)
				&& (itde.x + itde.w < -1024 || itde.x > 1024
						|| itde.y + itde.h < -1024 || itde.y > 1024))
			return;
		// FIXME: If the entity moves across a corner, it will never be sent

		// If it exists send only modifications
		bool move = itde.x != dent.x || itde.y != dent.y || itde.z != dent.z;
		bool resize = itde.w != dent.w || itde.h != dent.h;
		bool change_tex = itde.tex != dent.tex;

		itde = dent;

		if (!move && !resize && !change_tex)
			return; // Nothing has changed, send nothing

		if (move && !resize && !change_tex)
		{
			// Only move the entity
			static const int len = 9;
			DataPacket packet =
			{ len, new char[len] };

			packet.data[0] = 'M';
			SDLNet_Write16(dent.id, packet.data + 1);
			SDLNet_Write16(dent.x, packet.data + 3);
			SDLNet_Write16(dent.y, packet.data + 5);
			SDLNet_Write16(dent.z, packet.data + 7);

			SDL_LockMutex(send_mutex);
			data_to_send.push_back(packet);
			SDL_UnlockMutex(send_mutex);
			return;
		}

		if (!move && (resize || change_tex))
		{
			// Only send width height and texture
			static const int len = 9;
			DataPacket packet =
			{ len, new char[len] };

			packet.data[0] = 'i';
			SDLNet_Write16(dent.id, packet.data + 1);
			SDLNet_Write16(dent.w, packet.data + 3);
			SDLNet_Write16(dent.h, packet.data + 5);
			SDLNet_Write16(dent.tex, packet.data + 7);

			SDL_LockMutex(send_mutex);
			data_to_send.push_back(packet);
			SDL_UnlockMutex(send_mutex);
			return;
		}

		{
			// Otherwise send everything
			static const int len = 15;
			DataPacket packet =
			{ len, new char[len] };

			packet.data[0] = 'a';
			SDLNet_Write16(dent.id, packet.data + 1);
			SDLNet_Write16(dent.x, packet.data + 3);
			SDLNet_Write16(dent.y, packet.data + 5);
			SDLNet_Write16(dent.z, packet.data + 7);
			SDLNet_Write16(dent.w, packet.data + 9);
			SDLNet_Write16(dent.h, packet.data + 11);
			SDLNet_Write16(dent.tex, packet.data + 13);

			SDL_LockMutex(send_mutex);
			data_to_send.push_back(packet);
			SDL_UnlockMutex(send_mutex);
		}
	}
	else
	{
		// If it's new, send it as a new entity
		// Check bounds
		if (dent.x + dent.w < -1024 || dent.x > 1024 || dent.y + dent.h < -1024 || dent.y > 1024)
			return;

		// Store it
		client_entities[eid] = dent;

		static const int len = 15;
		DataPacket packet =
		{ len, new char[len] };

		packet.data[0] = 'a';
		SDLNet_Write16(dent.id, packet.data + 1);
		SDLNet_Write16(dent.x, packet.data + 3);
		SDLNet_Write16(dent.y, packet.data + 5);
		SDLNet_Write16(dent.z, packet.data + 7);
		SDLNet_Write16(dent.w, packet.data + 9);
		SDLNet_Write16(dent.h, packet.data + 11);
		SDLNet_Write16(dent.tex, packet.data + 13);

		SDL_LockMutex(send_mutex);
		data_to_send.push_back(packet);
		SDL_UnlockMutex(send_mutex);
	}
}

/* Sends an entity with coordinates relative to our entity
 */
void ClientConnection::send_hud()
{
	std::vector<Entity::HudIcon> icons = entity->get_hud_icons();

	int i = 0;
	for (typeof(icons.begin()) it = icons.begin(); it != icons.end(); ++it)
	{
		static const int len = 15;
		DataPacket packet =
		{ len, new char[len] };

		packet.data[0] = 'h';
		SDLNet_Write16(i++, packet.data + 1); // id
		SDLNet_Write16(it->x, packet.data + 3); // x
		SDLNet_Write16(it->y, packet.data + 5); // y
		SDLNet_Write16(0, packet.data + 7); // z
		SDLNet_Write16(it->width, packet.data + 9); // width
		SDLNet_Write16(it->height, packet.data + 11); // height
		SDLNet_Write16(it->tex, packet.data + 13); // texture

		SDL_LockMutex(send_mutex);
		data_to_send.push_back(packet);
		SDL_UnlockMutex(send_mutex);
	}
}

int ClientConnection::send_thread_func()
{
	while (status == 0)
	{
		// Remove a packet from the queue or wait
		SDL_LockMutex(send_mutex);
		if (data_to_send.size() > 0)
		{
			int len = data_to_send.front().len;
			char * data = data_to_send.front().data;
			data_to_send.pop_front();
			SDL_UnlockMutex(send_mutex);

			// Send the packet
			int result = SDLNet_TCP_Send(socket, data, len);
			delete data;

			if (result < len)
			{
				printf("%s:%d SDLNet_TCP_Send: %s\n", __FILE__, __LINE__,
						SDLNet_GetError());
				// It may be good to disconnect sock because it is likely invalid now.
				status = -1;
				return 0; // terminate thread
			}
		}
		else
		{
			SDL_UnlockMutex(send_mutex);
			SDL_Delay(0);
		}
	}
	return 0;
}

/* Listen for client commands
 */
int ClientConnection::recieve_thread_func()
{
	while (status == 0)
	{
		Uint16 msg;
		int result = SDLNet_TCP_Recv(socket, &msg, 2);
		if (result == 1)
			result = SDLNet_TCP_Recv(socket, (char*) (&msg) + 1, 1); // Recieve the other half
		if (result <= 0)
		{
			if (result == 0)
				printf("%s:%d SDLNet_TCP_Recv: Connection closed by peer\n", __FILE__, __LINE__);
			else
				printf("%s:%d SDLNet_TCP_Recv: %s\n", __FILE__, __LINE__, SDLNet_GetError());
			status = -1;
			// An error may have occured, but sometimes you can just ignore it
			// It may be good to disconnect sock because it is likely invalid now.
			break;
		}

		Uint32 arrival_time = SDL_GetTicks();
		SDLKey key = (SDLKey) SDLNet_Read16(&msg);
		bool released = false;
		if (Sint16(key) < 0)
		{
			released = true;
			key = SDLKey(-Sint16(key));
		}

		ReceivedPacket packet =
		{ key, released, arrival_time };

		//printf("Recieved %s (%d) from %s\n", SDL_GetKeyName(key), key, name.c_str());

		SDL_LockMutex(messages_mutex);
		messages.push_back(packet);
		SDL_UnlockMutex(messages_mutex);

	}
	//SDLNet_TCP_Close(client_data->socket);
	return 0;
}

ClientConnection::ReceivedPacket ClientConnection::pop_message()
{
	ReceivedPacket packet = { SDLK_UNKNOWN };
	SDL_LockMutex(messages_mutex);
	if (messages.size() > 0)
	{
		packet = messages.front();
		messages.pop_front();
	}
	SDL_UnlockMutex(messages_mutex);

	return packet;
}

void ClientConnection::process_messages()
{
	Uint32 now = SDL_GetTicks();
	std::map<SDLKey, Uint32> releases;
	std::set<SDLKey> presses;
	std::map<SDLKey, Uint32> holds;
	for (ReceivedPacket packet = pop_message(); packet.key != SDLK_UNKNOWN; packet = pop_message())
	{
		if (packet.released)
			releases[packet.key] = packet.arrival_time;
		else
		{
			active_keys[packet.key] = packet.arrival_time;
			presses.insert(packet.key);
		}

		//printf("Processed %s (%d) from %s\n", SDL_GetKeyName(packet.key), packet.key, get_name().c_str());
	}

	Uint32 hold_time = 100; // ms
	for (typeof(active_keys.begin()) it = active_keys.begin(); it != active_keys.end(); ++it)
	{
		Uint32 t;
		typeof(releases.begin()) r = releases.find(it->first);
		if (r != releases.end())
		{
			t = r->second - it->second;
			printf("%d released after %d clocks, hold_time: %d\n", it->first, t,
					(int) (hold_time));
		}
		else
		{
			t = now - it->second;
			//printf("%d active for %d clocks, hold_time: %d\n", it->first, t, (int)(hold_time));
		}

		if (t >= hold_time)
			holds[it->first] = t;
	}

	// TODO: sort actions by arrival time

	for (typeof(holds.begin()) it = holds.begin(); it != holds.end(); ++it)
	{
		typeof(bindings.begin()) b = bindings.find(it->first);
		if (b != bindings.end() && presses.find(it->first) == presses.end())
		{
			// Call the member function on the entity
			b->second.onRepeat(entity, it->second);
		}
	}
	for (typeof(presses.begin()) it = presses.begin(); it != presses.end();
			++it)
	{
		typeof(bindings.begin()) b = bindings.find(*it);
		if (b != bindings.end())
		{
			// Call the member function on the entity
			b->second.onPressed(entity);
		}
	}

	for (typeof(releases.begin()) it = releases.begin(); it != releases.end();
			++it)
	{
		active_keys.erase(it->first);
		typeof(bindings.begin()) b = bindings.find(it->first);
		if (b != bindings.end())
		{
			// Call the member function on the entity
			b->second.onReleased(entity);
		}
	}
}

int ClientConnection::get_status()
{
	return status;
}
}

