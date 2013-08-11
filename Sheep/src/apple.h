#ifndef APPLE_H
#define APPLE_H

#include "entity.h"

#include <sstream>

namespace WolfSheepServer
{
	class Apple : public Entity
	{
		public:
			Apple(int x, int y, int color) :
                color(color)
			{
                this->x = x;
                this->y = y;
			}
			virtual int get_w() const { return 24; }
			virtual int get_h() const { return 24; }
			virtual int get_tex() const { return 20 + color; }
			virtual int get_weight() const { return 150; } // 0.15 kg

            virtual std::string get_type()
            {
                return "apple";
            }

            virtual void pack(std::ostream &os)
            {
                Entity::pack(os);
                os << " " << color;
            }

            virtual void unpack(std::istream &is)
            {
                Entity::unpack(is);
                is >> color;
            }

		protected:
            int color;
        private:
	};
}

#endif // APPLE_H

