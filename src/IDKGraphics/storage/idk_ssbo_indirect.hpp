#pragma once

#include <libidk/GL/idk_glSSBO.hpp>


namespace idk
{
    namespace indirect_draw
    {
        static constexpr uint32_t MAX_MATERIALS  = 2048*5;
        static constexpr uint32_t MAX_TRANSFORMS = 2048;
        static constexpr uint32_t MAX_DRAW_CALLS = 512;
    };

    struct DrawIndirectData;

};



struct idk::DrawIndirectData
{
    GLuint64    materials       [idk::indirect_draw::MAX_MATERIALS];
    GLuint64    user_materials  [idk::indirect_draw::MAX_MATERIALS];
    glm::mat4   transforms      [idk::indirect_draw::MAX_TRANSFORMS];
    uint32_t    offsets         [idk::indirect_draw::MAX_DRAW_CALLS];
};



