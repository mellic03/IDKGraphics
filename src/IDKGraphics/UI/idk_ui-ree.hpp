// #pragma once

// #include <vector>
// #include <sstream>

// #include <SDL2/SDL_ttf.h>
// #include <functional>

// #include <glm/glm.hpp>


// namespace idk { class EngineAPI; }


// // namespace idk::ui
// // {
// //     struct ElementStyle
// //     {
// //         glm::vec4 bg = glm::vec4(0.25f);
// //         glm::vec4 fg = glm::vec4(1.0f);
// //         float radius = 4.0f;
// //     };


// //     void init( const std::string &fontpath, int size );
// //     void shutdown();

// //     int createPanel( int direction, const ElementStyle &style );

// //     void openPanel( int panel );
// //     void closePanel( int panel );
// //     void togglePanel( int panel );

// //     void createButton( int panel, const std::string &text, std::function<void()> callback );

// //     void update( idk::EngineAPI& );
// //     void render( idk::EngineAPI& );
// // }




// namespace idkui
// {
//     class FontStream;
//     class TextManager;
// }



// class idkui::FontStream
// {
// private:
//     int m_x, m_y;
//     std::stringstream m_ss;

// public:
//     FontStream( int x, int y ): m_x(x), m_y(y) {  };

//     template <typename T>
//     FontStream &operator << ( const T &data )
//     {
//         m_ss << data;
//         return *this;
//     };

//     void writeToSurface( TTF_Font *font, SDL_Surface *surface );

// };



// class idkui::TextManager
// {
// private:
//     inline static TTF_Font *m_font    = nullptr;
//     inline static uint32_t *m_texture = nullptr;
//     inline static std::vector<idkui::FontStream> m_streams;

// public:
//     // TextManager( const std::string &fontpath, int size );
//     static void init( const std::string &fontpath, int size );
//     static void render( SDL_Surface* );

//     static idkui::FontStream &text( int x, int y );
// };


