#include <cstdlib>
#include <SDL.h>

#include "SDL_opengl.h"
#include "SDL_image.h"

#include "graphics.hpp"

#include <map>
#include <string>

namespace EmergenceClient
{
    GLuint nulltexture = 0;
    std::map<std::string, GLuint> textures;
    GLuint tile_textures[256];

    void init_opengl(int width, int height)
    {
        set_fullview(0, 0, width, height);

        glEnable( GL_TEXTURE_2D );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

        glClear( GL_COLOR_BUFFER_BIT );

        nulltexture = create_texture("data/null.png");
        char filename[128];
        for (int i = 0; i < 256; ++i)
        {
        	sprintf(filename, "data/grass%03d.png", i);
        	tile_textures[i] = create_texture(filename);
        	if (tile_textures[i] == 0)
                tile_textures[i] = nulltexture;
        }
    }

    void set_fullview(int x, int y, int width, int height)
    {
        glViewport( x, y, width, height );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    void set_worldview(int x, int y, int width, int height)
    {
        glViewport( x, y, width, height );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        glOrtho(-512.0f, 512.0f, 512.0f, -512.0f, -1.0f, 1.0f);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    void set_sideview(int x, int y, int width, int height)
    {
        glViewport( x, y, width, height );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        glOrtho(-100.0f, 100.0f, 300.0f, -300.0f, -1.0f, 1.0f);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    GLuint get_texture(std::string name)
    {
        // Is it loaded?
        typeof(textures.begin()) it = textures.find(name);
        if (it != textures.end())
            return it->second;

        // Not loaded, try to load it
        GLuint tex = create_texture(name.c_str());
        if (tex != 0)
            return textures[name] = tex;

        // Could set textures[name] to nulltexture to avoid repeated disk checks
        textures[name] = nulltexture;
        return nulltexture;
    }

    GLuint create_texture(const char *filename)
    {
        SDL_Surface *surface;	// This surface will tell us the details of the image
        //if ( (surface = SDL_LoadBMP(filename)) )
        if ( (surface = IMG_Load(filename)) )
        {
            GLuint tex = create_texture(surface);
            SDL_FreeSurface( surface );
            return tex;
        }
        else
        {
            fprintf(stderr, "SDL could not load %s: %s\n", filename, SDL_GetError());
            return 0;
        }
    }

    GLuint create_texture(SDL_Surface *surface)
    {
        GLuint texture;			// This is a handle to our texture object
        GLenum texture_format;
        GLint  nOfColors;

        // Check that the image's width is a power of 2
        if ( (surface->w & (surface->w - 1)) != 0 )
        {
            fprintf(stderr, "warning: image width is not a power of 2\n");
        }

        // Also check if the height is a power of 2
        if ( (surface->h & (surface->h - 1)) != 0 )
        {
            fprintf(stderr, "warning: image height is not a power of 2\n");
        }

        // get the number of channels in the SDL surface
        nOfColors = surface->format->BytesPerPixel;
        if (nOfColors == 4)     // contains an alpha channel
        {
            if (surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;
        }
        else if (nOfColors == 3)     // no alpha channel
        {
            if (surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;
        }
        else
        {
            fprintf(stderr, "warning: the image is not truecolor..  this will probably break\n");
            // this error should not go unhandled
            return 0;
        }

        // Have OpenGL generate a texture object handle for us
        glGenTextures( 1, &texture );

        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, texture );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        // Edit the texture object's image data using the information SDL_Surface gives us
        glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );

        return texture;
    }

    /* Draw a set of tiles, starting from x,y with tiles of size w,h
     * with rows*cols tiles with images specified in tiles.
     */
    void draw_tiles(int x, int y, int w, int h, int tiles[], int rows, int cols)
    {
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
            {
                //printf("index: %d textureID: %d\n", i*cols + j, tiles[i*cols + j]);
                draw_quad(x+w*j, y+i*h, w, h, tile_textures[tiles[i*cols + j]]);
            }
    }

    void draw_quad(int x, int y, int w, int h, GLuint texture)
    {
        glEnable( GL_TEXTURE_2D );

        // Bind the texture to which subsequent calls refer to
        glBindTexture( GL_TEXTURE_2D, texture );

        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin( GL_QUADS );
        //Bottom-left vertex (corner)
        glTexCoord2i( 0, 0 );
        glVertex3f( x, y, 0.0f );

        //Bottom-right vertex (corner)
        glTexCoord2i( 1, 0 );
        glVertex3f( x+w, y, 0.f );

        //Top-right vertex (corner)
        glTexCoord2i( 1, 1 );
        glVertex3f( x+w, y+h, 0.f );

        //Top-left vertex (corner)
        glTexCoord2i( 0, 1 );
        glVertex3f( x, y+h, 0.f );
        glEnd();
    }

    void draw_quad(int x, int y, int w, int h, float r, float g, float b)
    {
        glDisable( GL_TEXTURE_2D );

        glColor3f(r, g, b);

        glBegin( GL_QUADS );
        //Bottom-left vertex (corner)
        glVertex3f( x, y, 0.0f );

        //Bottom-right vertex (corner)
        glVertex3f( x+w, y, 0.f );

        //Top-right vertex (corner)
        glVertex3f( x+w, y+h, 0.f );

        //Top-left vertex (corner)
        glVertex3f( x, y+h, 0.f );
        glEnd();
    }

}

