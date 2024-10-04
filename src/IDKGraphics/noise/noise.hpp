#pragma once

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>


namespace idk::internal
{
    void upload_noise();
}


namespace idk::noise
{
    glm::vec2 BlueRG( float u, float v );
    glm::vec2 Randvec2( int i );
}



