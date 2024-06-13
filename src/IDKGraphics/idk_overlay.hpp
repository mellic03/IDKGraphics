#pragma once

#include <cstddef>
#include <cstdint>

#include <libidk/GL/idk_glTexture.hpp>
#include <libidk/idk_glm.hpp>


namespace idk
{
    struct RenderOverlay;
    struct RenderOverlayFill;
}


struct idk::RenderOverlay
{
    idk::TextureRef texture;

    int      state = 0;

    float    display_duration;
    float    fadein_duration;
    float    fadeout_duration;

    float    display_timer;
    float    fadein_timer;
    float    fadeout_timer;
    float    alpha;

    glm::vec3 color;

    RenderOverlay( const idk::TextureRef &tex, float fadein, float display, float fadeout );

    void advance( float dt );
    bool finished();

};


struct idk::RenderOverlayFill
{
    glm::vec3 fill;

    float     display_duration;
    float     fadein_duration;
    float     fadeout_duration;

    float     display_timer;
    float     fadein_timer;
    float     fadeout_timer;
    float     alpha;

    glm::vec3 color;

    RenderOverlayFill( const glm::vec3 &fillcolor, float fadein, float display, float fadeout );

    void advance( float dt );
    bool finished();

};



