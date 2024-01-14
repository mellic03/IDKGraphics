#pragma once

#include <libidk/GL/common.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>


namespace idk
{
    struct ModelHandle
    {
        uint32_t vertexformat;
        uint32_t numindices;
        uint32_t basevertex;
        uint32_t baseindex;

        idk::glDrawElementsIndirectCommand cmd;

    };
};
