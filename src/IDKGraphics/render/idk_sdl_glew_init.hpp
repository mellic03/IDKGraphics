#pragma once

#include <libidk/idk_sdl2.hpp>

namespace idk::internal { class SDL2_WindowSystem; };

class idk::internal::SDL2_WindowSystem
{
private:
    int             m_gl_minor;
    int             m_gl_major;

    SDL_Window *    m_window;
    SDL_GLContext   m_glcontext;

public:

    SDL2_WindowSystem( const char *windowname, int w, int h, int gl_major,
                       int gl_minor, uint32_t flags );


    SDL_Window *createWindow  ( const char *name, uint32_t flags, int w, int h );
    void        destroyWindow ( SDL_Window * );

    SDL_Window   *getMainWindow() { return m_window;    };
    SDL_GLContext getGlContext()  { return m_glcontext; };

    // int getGLVersionMinor() {};
    // int getGLVersionMajor() {};

};
