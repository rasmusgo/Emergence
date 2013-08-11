#ifndef TEMP_SPRITE_H
#define TEMP_SPRITE_H

#include "entity.h"

#include <sstream>

namespace WolfSheepServer
{
	class TempSprite : public Entity
	{
		public:
			TempSprite(int x, int y, int w, int h, int texture, int time) :
                width(w),
                height(h),
                texture(texture),
                time(time)
			{
                this->x = x;
                this->y = y;
			}
			virtual int get_w() const { return width; }
			virtual int get_h() const { return height; }
			virtual int get_tex() const { return texture; }
			virtual int get_weight() const { return 10; } // 0.01 kg

            virtual std::string get_type()
            {
                return "temp_sprite";
            }

            virtual void pack(std::ostream &os)
            {
                Entity::pack(os);
                os << " " << width
                   << " " << width
                   << " " << height
                   << " " << texture
                   << " " << time;
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

#endif // TEMP_SPRITE_H


