#include "idk_ui.hpp"


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
