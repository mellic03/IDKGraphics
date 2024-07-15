#pragma once


namespace idk
{
    struct RenderSettings;
    struct EnvProbeSettings;
}


struct idk::EnvProbeSettings
{
    bool enabled   = true;
    bool dirty     = true;
    bool visualize = false;

    glm::ivec3 grid_size = glm::ivec3(16, 1, 16);
    glm::vec3  cell_size = glm::vec3(4.0f, 1.0f, 4.0f);
    int        nprobes   = 16*1*16;

    // inline static glm::vec3 PROBE_CELL_SPACING = glm::vec3(2.0, 3.0, 2.0);

    // inline static uint32_t PROBE_GRID_X = 8;
    // inline static uint32_t PROBE_GRID_Y = 3;
    // inline static uint32_t PROBE_GRID_Z = 8;
    // inline static uint32_t PROBE_GRID_NPROBES = PROBE_GRID_X * PROBE_GRID_Y * PROBE_GRID_Z;

    // inline static auto PROBE_GRID_SIZE  = glm::ivec3(PROBE_GRID_X, PROBE_GRID_Y, PROBE_GRID_Z);
    // inline static auto PROBE_GRID_HSIZE = PROBE_GRID_SIZE / 2;

};



struct idk::RenderSettings
{
    bool dirlight_volumetrics = true;

    EnvProbeSettings envprobe;
};

