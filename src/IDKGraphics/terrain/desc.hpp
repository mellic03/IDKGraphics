#pragma once

#include <libidk/idk_gl.hpp>
#include "../batching/idk_model_allocator.hpp"
#include "../storage/buffers.hpp"


namespace idk::TerrainRenderer
{
    struct NoiseFactor
    {
        float amp     = 0.5f;
        float wav     = 2.0f;
        float warp    = 0.0f;
        float octaves = 4.0f;
    };

    struct TerrainDesc
    {
        glm::vec4 clipmap_size = glm::vec4(32.0f, 5.0f, 1.0f, 1.0f);

        glm::vec4 texscale[1];
    
        glm::vec4 origin       = glm::vec4(0.0);
        glm::vec4 clamp_bounds = glm::vec4(0.25, 0.45, 0.85, 0.95);

        glm::vec4   water_color[4];
        glm::vec4   water_pos   = glm::vec4(0.0f);
        glm::vec4   water_scale = glm::vec4(1.0f);
        NoiseFactor water;

        NoiseFactor perlin;
        NoiseFactor voronoi;
        NoiseFactor vein;
        NoiseFactor exponent;
        NoiseFactor domainwarp;

        glm::mat4 transform    = glm::mat4(1.0f);
        glm::vec4 scale        = glm::vec4(1.0f, 0.2f, 0.5f, 1.0f);
        glm::vec4 slope_blend  = glm::vec4(0.6f, 0.75f, 0.1f, 0.5f);
        glm::vec4 height_blend = glm::vec4(0.6f, 0.75f, 0.1f, 0.5f);
    };
}