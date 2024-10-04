#pragma once

#include <libidk/idk_allocator.hpp>



namespace idk
{
    class Volume;
};



class idk::Volume
{
public:
    struct VolumeDesc
    {
        
    };

private:
    inline static idk::Allocator<VolumeDesc> m_volumes;

public:


};

