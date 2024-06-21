#include "idk_ui.hpp"

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>



// /*
//     Try batching UI elements instead, it will probably be faster.

//     struct Buffer
//     {
//         glm::vec4 bounds[MAX_ELEMENTS];
//         uint64_t  textures[MAX_ELEMENTS];
//     };

// */


// // Make this an SSBO, use gl_DrawID as index into bounds and handles.
// struct UI_RectBuffer
// {
//     glm::vec4 bounds[128];
//     uint64_t  handles[128];
// };


// struct UI_Button
// {
//     float x, y;
//     float w, h;
    
//     std::string text;
//     std::function<void()> callback;

//     glm::vec4 background = glm::vec4(0.0f);

//     float clicked = 0.0f;
//     bool hovered = false;
//     bool dirty   = true;
// };


// struct UI_Panel
// {
//     float x, y;
//     float w, h;

//     bool hidden = false;

//     std::vector<UI_Button> buttons;
//     idk::ui::ElementStyle style;
// };



// static const idk::glTextureConfig texture_config = {
//     .internalformat = GL_RGBA8,
//     .format         = GL_RGBA,
//     .minfilter      = GL_LINEAR,
//     .magfilter      = GL_LINEAR,
//     .datatype       = GL_UNSIGNED_BYTE,
//     .genmipmap      = GL_FALSE
// };


// namespace
// {
//     idk::Allocator<UI_Panel> m_panels;
//     TTF_Font *m_font;
//     SDL_Surface *m_surface = nullptr;
// }



// void
// idk::ui::init( const std::string &fontpath, int size )
// {
//     // TTF_Init();
//     m_font = TTF_OpenFont(fontpath.c_str(), size);
//     m_surface = SDL_CreateRGBSurface(0, 32, 1024, 1024, 0, 0, 0, 0);
// }

// void
// idk::ui::shutdown()
// {
//     TTF_CloseFont(m_font);

//     // for (UI_Button &b: m_buttons)
//     // {
//     //     gl::deleteTextures(1, &b.texture);
//     //     SDL_FreeSurface(b.surface);
//     // }
// }



// int
// idk::ui::createPanel( int direction, const ElementStyle &style )
// {
//     UI_Panel panel;

//     panel.x = (direction == 0) ? 10 : 990;
//     panel.y = 150;

//     panel.w = 100;
//     panel.h = 400;

//     panel.style = style;

//     return m_panels.create(panel);
// }


// void
// idk::ui::openPanel( int panel )
// {
//     m_panels.get(panel).hidden = false;
// }


// void
// idk::ui::closePanel( int panel )
// {
//     m_panels.get(panel).hidden = true;
// }


// void
// idk::ui::togglePanel( int panel )
// {
//     bool hidden = m_panels.get(panel).hidden;
//     m_panels.get(panel).hidden = !hidden;
// }






// void
// idk::ui::createButton( int p, const std::string &text, std::function<void()> callback )
// {
//     UI_Panel &panel = m_panels.get(p);

//     std::cout << "X: " << panel.x + 10 << "\n";

//     UI_Button button = {
//         .x = panel.x + 10,
//         .y = panel.y + panel.buttons.size()*(64 + 10),
//         .w = 128,
//         .h = 32,
//         .text = text,
//         .callback = callback
//     };

//     panel.buttons.push_back(button);
// }



// void
// _renderButton( UI_Button &button, const idk::ui::ElementStyle &es )
// {
//     glm::u8vec4 a = glm::u8vec4(255.0f * es.fg);
//     glm::u8vec4 b = glm::u8vec4(255.0f * es.bg);

//     SDL_Color fg = {a.x, a.y, a.z, a.a};
//     SDL_Color bg = {b.x, b.y, b.z, b.a};

//     if (button.clicked <= 0.0f && button.hovered == true)
//     {
//         std::swap(fg, bg);
//     }

//     // Draw background rect
//     // -----------------------------------------------------------------------------------------
//     glm::ivec4 bb = glm::ivec4(button.x, button.y, button.w, button.h);
//     SDL_Rect brect = { bb.x, bb.y, bb.z, bb.w };
//     SDL_FillRect(m_surface, &brect, (bg.a << 24) + (bg.r << 16) + (bg.g << 8) + bg.b);
//     // -----------------------------------------------------------------------------------------


//     // Draw text
//     // -----------------------------------------------------------------------------------------
//     SDL_Surface *S0 = TTF_RenderText_Blended_Wrapped(m_font, button.text.c_str(), fg, 32);
//     SDL_Rect src = {0, 0, S0->w, S0->h};
//     SDL_Rect dst = {button.x, button.y, S0->w, S0->h};

//     SDL_BlitScaled(S0, &src, m_surface, &dst);
//     // -----------------------------------------------------------------------------------------

//     SDL_FreeSurface(S0);
// }



// void
// _renderPanel( UI_Panel &panel, const idk::ui::ElementStyle &es )
// {
//     glm::u8vec4 a = glm::u8vec4(255.0f * es.fg);
//     glm::u8vec4 b = glm::u8vec4(255.0f * es.bg);

//     SDL_Color bg = {b.x, b.y, b.z, b.a};

//     // Draw background rect
//     // -----------------------------------------------------------------------------------------
//     glm::ivec4 bb = glm::ivec4(panel.x, panel.y, panel.w, panel.h);
//     SDL_Rect brect = { bb.x, bb.y, bb.z, bb.w };
//     SDL_FillRect(m_surface, &brect, (bg.a << 24) + (bg.r << 16) + (bg.g << 8) + bg.b);
//     // -----------------------------------------------------------------------------------------

// }



// void
// idk::ui::update( idk::EngineAPI &api )
// {
//     float dt = api.getEngine().deltaTime();
//     auto &ren = api.getRenderer();
//     auto &events = api.getEventSys();

//     // if (events.mouseDown(idk::MouseButton::LEFT) == false)
//     // {
//     //     return;
//     // }

//     glm::vec2 mouse = events.mousePosition();
//     float mx = mouse.x;
//     float my = mouse.y;

//     idkui::TextManager::text(mx, my) << "(" << mx << ", " << my << ")";

//     for (UI_Panel &panel: m_panels)
//     {
//         for (UI_Button &b: panel.buttons)
//         {
//             b.clicked -= dt;
//             b.hovered = false;

//             if (geometry::pointInRect(mx, my, b.x, b.y, b.w, b.h))
//             {
//                 b.hovered = true;

//                 if (events.mouseClicked(idk::MouseButton::LEFT))
//                 {
//                     b.clicked = 0.05f;
//                     b.callback();
//                 }
//             }
//         }
//     }

// }


// void
// idk::ui::render( idk::EngineAPI &api )
// {
//     auto &ren = api.getRenderer();


//     static uint32_t texture;

//     if (m_surface == nullptr)
//     {
//         m_surface = SDL_CreateRGBSurface(0, ren.width(), ren.height(), 32, 0, 0, 0, 0);
//         SDL_SetSurfaceBlendMode(m_surface, SDL_BLENDMODE_ADD);

//         texture = gltools::loadTexture2D(m_surface->w, m_surface->h, m_surface->pixels, texture_config);
//     }

//     int rw = ren.width();
//     int rh = ren.height();
//     int sw = m_surface->w;
//     int sh = m_surface->h;


//     if (rw != sw || rh != sh)
//     {
//         SDL_FreeSurface(m_surface);
//         m_surface = SDL_CreateRGBSurface(0, ren.width(), ren.height(), 32, 0, 0, 0, 0);

//         gl::deleteTextures(1, &texture);
//         texture = gltools::loadTexture2D(m_surface->w, m_surface->h, m_surface->pixels, texture_config);
//     }

//     SDL_Rect rect = { 0, 0, m_surface->w, m_surface->h };
//     SDL_FillRect(m_surface, &rect, 0);

//     for (UI_Panel &panel: m_panels)
//     {
//         if (panel.hidden)
//         {
//             continue;
//         }

//         _renderPanel(panel, panel.style);

//         for (UI_Button &b: panel.buttons)
//         {
//             _renderButton(b, panel.style);
//         }
//     }

//     gl::deleteTextures(1, &texture);
//     texture = gltools::loadTexture2D(m_surface->w, m_surface->h, m_surface->pixels, texture_config);
    
//     // gl::textureSubImage2D(
//     //     texture, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_surface->pixels
//     // );

//     ren.drawTextureOverlay(texture);
// }






















void
idkui::TextManager::init( const std::string &fontpath, int size )
{
    TTF_Init();
    m_font = TTF_OpenFont(fontpath.c_str(), size);
}


void
idkui::TextManager::render( SDL_Surface *dst )
{
    for (auto &f: m_streams)
    {
        f.writeToSurface(m_font, dst);
    }

    m_streams.clear();
}


idkui::FontStream &
idkui::TextManager::text( int x, int y )
{
    m_streams.push_back(FontStream(x, y));
    return m_streams.back();
}


void
idkui::FontStream::writeToSurface( TTF_Font *font, SDL_Surface *surface )
{
    SDL_Color fg = {150, 150, 150, 0};
    SDL_Color bg = {255, 255, 255, 0};

    SDL_Surface *S = TTF_RenderUTF8_Blended(font, m_ss.str().c_str(), fg);

    SDL_Rect src, dst;

    src.x = 0;
    src.y = 0;
    src.w = S->w;
    src.h = S->h;

    dst.x = m_x;
    dst.y = m_y;
    dst.w = S->w;
    dst.h = S->h;

    SDL_BlitScaled(S, &src, surface, &dst);

    SDL_FreeSurface(S);
};
