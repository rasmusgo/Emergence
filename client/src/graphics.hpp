#pragma once

#include "SDL.h"
#include "SDL_opengl.h"

#include <string>

namespace EmergenceClient
{

	void init_opengl(int width, int height);
	void set_fullview(int x, int y, int width, int height);
	void set_worldview(int x, int y, int width, int height);
	void set_sideview(int x, int y, int width, int height);

	GLuint create_texture(SDL_Surface *surface);

	GLuint create_texture(const char *filename);

	GLuint get_texture(std::string);

    /* Draw a set of tiles, starting from x,y with tiles of size w,h
     * with rows*cols tiles with images specified in tiles.
     */
    void draw_tiles(int x, int y, int w, int h, int tiles[], int rows, int cols);

	void draw_quad(int x, int y, int w, int h, GLuint texture);

	void draw_quad(int x, int y, int w, int h, float r, float g, float b);

} // EmergenceClient
