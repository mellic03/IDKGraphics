#pragma once

#include <cstddef>
#include <cstdint>

namespace idk
{
    void updateTime( float dt );
    float getTime();
    float getDeltaTime();
    uint32_t getFrame();
}



