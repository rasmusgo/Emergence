#include "rabbit.hpp"
#include "world.hpp"

#include <stdlib.h>
#include <sstream>

// TODO: Make adult male rabbits dislike each other
// TODO: Increase the number of babies born each time
// TODO: Let the female rabbits decide if they want to mate

// Reference on rabbits: http://www.welshrabbitry.com/birth.html

namespace EmergenceServer
{
Rabbit::Rabbit(int x, int y, bool male) :
		health(50),
		male(male),
		stomach(0),
		age(0),
		fear(0),
		weight(25),
		pregnant(0)
{
	this->x = x;
	this->y = y;

	mood = &Rabbit::calm;
}

Rabbit::~Rabbit()
{
	//dtor
}

void Rabbit::damage(int dmg, Entity *user, Entity *weapon)
{
	if (dmg > 0)
		health -= dmg;

	if (dmg > health / 4)
		fear += 50;
	else
		fear += 25;
}

Entity* Rabbit::grab_me(Entity *grabber)
{
	// It's scary if someone tries to pick me up
	fear += 25;
	return Entity::grab_me(grabber);
}

void Rabbit::drop_me()
{
	// It's a little scary to be dropped
	fear += 5;
	Entity::drop_me();
}

void Rabbit::prepare_action()
{
	// Don't increment prepared because it is not decremented in action
}

void Rabbit::action()
{
	// Don't do anything if picked up
	if (parent != 0)
		return;

	unsigned char &grass = world->grass(get_x(), get_y());
	if (grass > 0)
	{
		// Long grass gives more food
		if (grass >= world->get_grasslimit())
		{
			stomach += 2;
			grass -= 2;
		}
		else
		{
			stomach += 1;
			grass -= 1;
		}
	}
}

void Rabbit::tick()
{
	++age;

	if (mood == &Rabbit::dead)
	{
		dead();
		return;
	}

	// Health cannot be higher than weight
	if (health > weight)
		health = weight;

	if (age % 10 == 0)
	{
		if (pregnant > 0)
		{
			++pregnant;
			--stomach;
			// It's fifty-fifty to give birth this second
			// expected time of pregnancy is 20 + 0.25 + 2*0.125 + 3*0.0775 + ... =
			if (pregnant > 20 && rand() & 1)
			{
				// Give birth
				bool boy = rand() & 1;
				Rabbit *child = new Rabbit(get_x(), get_y(), boy);
				child->weight = pregnant;
				world->add_entity(child);
				pregnant = 0;

				if (boy)
					printf("a cute little rabbit boy was born\n");
				else
					printf("a cute little rabbit girl was born\n");
			}
		}

		// Calm down
		if (fear > 0)
			--fear;
	}

	if (age % 20 == 0)
	{
		// Digest food
		if (stomach > 0)
		{
			--stomach;
			++health;
		}
		else
		{
			--health;
		}
	}

	if (health <= 0)
	{
		mood = &Rabbit::dead;
		return;
	}

	// Can't do anything if picked up
	if (parent != 0)
		return;

	// Panic if fear is high
	if (fear >= 20)
		mood = &Rabbit::panic;

	(this->*mood)();
}

void Rabbit::calm()
{
	// TODO: Look for company
	if (stomach < 25)
		mood = &Rabbit::hungry;

	if (age % 20 == 0)
	{
		if (male && parent == 0)
		{
			std::vector<Entity*> container;
			world->get_entities_on(get_x(), get_y(), container);
			for (typeof(container.begin()) it = container.begin(); it != container.end(); ++it)
			{
				if (*it == this)
					continue;

				Rabbit *rabbit = dynamic_cast<Rabbit*>(*it);
				// Is this a fertile female rabbit?
				if (rabbit != 0 && rabbit->mood == &Rabbit::calm
						&& rabbit->male == false && rabbit->pregnant == 0
						&& rabbit->age > 100)
				{
					// Make sweet love, fast
					rabbit->pregnant = 1;
					printf("a rabbit got pregnant\n");
					return;
				}
			}
		}

		static const ai_callback actions[] =
		{ &Rabbit::up, &Rabbit::left, &Rabbit::down, &Rabbit::right,
				&Rabbit::action, 0, 0, 0 };

		// Do something, or not
		int a = rand() & 7; // same as modulo 8
		if (actions[a] != 0)
			(this->*actions[a])();
	}
}

void Rabbit::hungry()
{
	// TODO: Look for food
	if (stomach >= 50)
		mood = &Rabbit::calm;

	if (age % 5 == 0)
	{
		static const ai_callback actions[] = {
				&Rabbit::up,
				&Rabbit::left,
				&Rabbit::down,
				&Rabbit::right,
				&Rabbit::action,
				&Rabbit::action,
				&Rabbit::action,
				&Rabbit::action,
		};

		// Eat or move
		int a = rand() & 7; // same as modulo 8
		(this->*actions[a])();
	}
}

/* Run fast!
 */
void Rabbit::panic()
{
	// TODO: Run away from danger
	if (fear <= 10)
	{
		mood = &Rabbit::calm;
		return;
	}

	// These are the avalible directions we can run
	static const ai_callback moves[] = {
			&Rabbit::right,
			&Rabbit::up,
			&Rabbit::left,
			&Rabbit::down,
	};

	// We can turn or keep running forward
	// the chance of running forward is greater than turning
	const ai_callback actions[] = {
			moves[(direction - 1) & 3], // Turn left
			moves[(direction + 1) & 3], // Turn right
			moves[(direction + 2) & 3], // Turn around
			moves[direction], // Continue forward
			moves[direction],
			moves[direction],
			moves[direction],
			moves[direction],
	};

	// Choose direction
	(this->*actions[rand() & 7])();

	// It is calming to run
	--fear;
}

/* Decay slowly when dead and in time remove from world
 */
void Rabbit::dead()
{
	// Decay
	int decayrate = 10;
	if (parent != 0)
		decayrate = 20;

	if (weight > 0 && age % decayrate == 0)
		--weight;

	// Remove if decayed completely
	if (weight <= 0)
	{
		// Remove the entity from the world and delete it
		world->delete_entity(this);
		return;
	}
}

int Rabbit::get_w() const
{
	if (age > 100)
		return 48 + pregnant; // (body_fat + length) *0.01 * 64;
	else
		return 16 + (32 / 100.0) * age;
}

int Rabbit::get_h() const
{
	if (age > 100)
		return 48 + pregnant;
	else
		return 16 + (32 / 100.0) * age;
}

int Rabbit::get_tex() const
{
	if (mood == &Rabbit::dead)
		return 1;
	return 0;
}

int Rabbit::get_weight() const
{
	return weight + pregnant;
}

void Rabbit::pack(std::ostream &os)
{
	Entity::pack(os);
	os << " " << health << " "
			<< male << " "
			<< stomach << " "
			<< age << " "
			<< fear << " "
			<< weight << " "
			<< pregnant << " ";

	if (mood == &Rabbit::calm)
		os << 1;
	else if (mood == &Rabbit::hungry)
		os << 2;
	else if (mood == &Rabbit::panic)
		os << 3;
	else if (mood == &Rabbit::dead)
		os << 4;
	else
		os << 0;
}

void Rabbit::unpack(std::istream &is)
{
	Entity::unpack(is);
	int mood_id;
	is >> health >> male >> stomach >> age >> fear >> weight >> pregnant >> mood_id;

	ai_callback moods[] =
	{ 0, &Rabbit::calm, &Rabbit::hungry, &Rabbit::panic, &Rabbit::dead };

	mood = moods[mood_id];
}

}
