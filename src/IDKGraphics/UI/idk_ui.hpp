#pragma once

#include <vector>
#include <sstream>

#include <SDL2/SDL_ttf.h>



namespace idkui
{
    class FontStream;
    class TextManager;
};



class idkui::FontStream
{
private:
    int m_x, m_y;
    std::stringstream m_ss;

public:
    FontStream( int x, int y ): m_x(x), m_y(y) {  };

    template <typename T>
    FontStream &operator << ( const T &data )
    {
        m_ss << data;
        return *this;
    };

    void writeToSurface( TTF_Font *font, SDL_Surface *surface );

};



class idkui::TextManager
{
private:
    inline static TTF_Font *m_font    = nullptr;
    inline static uint32_t *m_texture = nullptr;
    inline static std::vector<idkui::FontStream> m_streams;

public:
    // TextManager( const std::string &fontpath, int size );
    static void init( const std::string &fontpath, int size );
    static void render( SDL_Surface* );

    static idkui::FontStream &text( int x, int y );
};


