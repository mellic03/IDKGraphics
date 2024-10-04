#include "irradiance_cache.hpp"
#include "../storage/bindings.hpp"
#include "../idk_renderengine.hpp"

using namespace idk;


struct SSBO_IrradianceCache
{
    glm::vec4 data[128][128][128];
};


namespace
{
    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO;

}



void
IrradianceCache::init( idk::RenderEngine &ren )
{
    m_SSBO.init(shader_bindings::SSBO_Irradiance);
    m_SSBO.bufferData(sizeof(SSBO_IrradianceCache, nullptr, GL_DYNAMIC_DRAW));

    
}

