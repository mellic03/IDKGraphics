#pragma once

#include "../idk_render_settings.hpp"

namespace idk
{
    struct EnvProbeSettings;
}


namespace idk::env_probe
{
    // inline static glm::vec3 PROBE_CELL_SPACING = glm::vec3(2.0, 3.0, 2.0);

    // inline static uint32_t PROBE_GRID_X = 8;
    // inline static uint32_t PROBE_GRID_Y = 3;
    // inline static uint32_t PROBE_GRID_Z = 8;
    // inline static uint32_t PROBE_GRID_NPROBES = PROBE_GRID_X * PROBE_GRID_Y * PROBE_GRID_Z;

    // inline static auto PROBE_GRID_SIZE  = glm::ivec3(PROBE_GRID_X, PROBE_GRID_Y, PROBE_GRID_Z);
    // inline static auto PROBE_GRID_HSIZE = PROBE_GRID_SIZE / 2;

    glm::vec3 layerToWorld( const glm::vec3 &viewpos, uint32_t layer, const EnvProbeSettings& );
    uint32_t  worldToLayer( const glm::vec3 &viewpos, glm::vec3 world, const EnvProbeSettings& );

}


namespace idk
{
    struct LightProbeSettings
    {
        
    };
}