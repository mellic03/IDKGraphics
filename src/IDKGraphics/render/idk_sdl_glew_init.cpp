#include "idk_sdl_glew_init.hpp"
#include "idk_initflags.hpp"

#include <libidk/GL/common.hpp>
#include <iostream>

using namespace idk::internal;


SDL2_WindowSystem::SDL2_WindowSystem( const char *windowname, int w, int h,
                                            int gl_major, int gl_minor, uint32_t flags )
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Error creating window\n";
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


    Uint32 sdl_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
    if (flags & idk::InitFlag::INIT_HEADLESS) sdl_flags |= SDL_WINDOW_HIDDEN;

    m_window = SDL_CreateWindow(
        windowname,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        sdl_flags
    );


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);


    m_glcontext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glcontext);
    SDL_GL_SetSwapInterval(0);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error initializing glew\n";
        exit(1);
    }

    std::cout
        << "OpenGL version "
        << gl_major << "." << gl_minor
        << "\n";

}



SDL_Window *
SDL2_WindowSystem::createWindow( const char *name,
                                 uint32_t flags  = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
                                 int      width  = 512,
                                 int      height = 512 )
{ 
    SDL_Window *window = SDL_CreateWindow(
        name,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
    );

    return window;
}


void
SDL2_WindowSystem::destroyWindow( SDL_Window *window )
{
    SDL_DestroyWindow(window);
}
