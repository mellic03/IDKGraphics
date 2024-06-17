#pragma once

#include <libidk/GL/idk_glSSBO.hpp>


namespace idk
{
    namespace indirect_draw
    {
        static constexpr uint32_t MAX_TEXTURES   = 1024;
        static constexpr uint32_t MAX_TRANSFORMS = 1024;
        static constexpr uint32_t MAX_DRAW_CALLS = 512;

    };

    struct DrawIndirectData;
};



struct idk::DrawIndirectData
{
    GLuint64    textures          [idk::indirect_draw::MAX_TEXTURES];
    glm::mat4   transforms        [idk::indirect_draw::MAX_TRANSFORMS];
    uint32_t    transform_offsets [idk::indirect_draw::MAX_DRAW_CALLS];
    uint32_t    texture_offsets   [idk::indirect_draw::MAX_DRAW_CALLS];
};



