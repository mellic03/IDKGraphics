#include "idk_overlay.hpp"
#include <libidk/idk_log.hpp>


idk::RenderOverlay::RenderOverlay( const idk::TextureRef &tex,
                                   float fadein, float display, float fadeout )
{
    texture = tex;

    display_duration = display;
    fadein_duration  = fadein;
    fadeout_duration = fadeout;

    display_timer    = 0.0f;
    fadein_timer     = 0.0f;
    fadeout_timer    = 0.0f;

    alpha            = 0.0f;
}


void
idk::RenderOverlay::advance( float dt )
{
    if (state == 0)
    {
        if (fadein_timer < fadein_duration)
        {
            alpha = fadein_timer / fadein_duration;
            fadein_timer += dt;
        }
    
        else
        {
            state = 1;
        }
    }

    if (state == 1)
    {
        if (display_duration > 0.0f && display_timer < display_duration)
        {

            alpha = 1.0f;
            display_timer += dt;
        }

        else
        {
            state = 2;
        }
    }

    if (state == 2)
    {
        if (fadeout_duration > 0.0f && fadeout_timer < fadeout_duration)
        {
            alpha = 1.0f - (fadeout_timer / fadeout_duration);
            fadeout_timer += dt;
        }

        else
        {
            state = 3;
        }
    }

}


bool
idk::RenderOverlay::finished()
{
    return state >= 3;
}







idk::RenderOverlayFill::RenderOverlayFill( const glm::vec3 &fillcolor,
                                           float fadein, float display, float fadeout )
{
    fill = fillcolor;

    display_duration = display;
    fadein_duration  = fadein;
    fadeout_duration = fadeout;

    display_timer    = 0.0f;
    fadein_timer     = 0.0f;
    fadeout_timer    = 0.0f;

    alpha            = 1.0f;
}


void
idk::RenderOverlayFill::advance( float dt )
{
    if (fadein_duration > 0.0f && fadein_timer < fadein_duration)
    {
        alpha = fadein_timer / fadein_duration;
        fadein_timer += dt;
    }

    else if (display_duration > 0.0f && display_timer < display_duration)
    {
        alpha = 1.0f;
        display_timer += dt;
    }

    else if (fadeout_duration > 0.0f && fadeout_timer < fadeout_duration)
    {
        alpha = 1.0f - (fadeout_timer / fadeout_duration);
        fadeout_timer += dt;
    }
}


bool
idk::RenderOverlayFill::finished()
{
    bool a = fadein_timer  >= fadein_duration;
    bool b = display_timer >= display_duration;
    bool c = fadeout_timer >= fadeout_duration;

    return a && b && c;
}
