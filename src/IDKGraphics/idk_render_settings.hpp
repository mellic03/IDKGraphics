#pragma once

#include "idk_render_settings_field.hpp"


namespace idk
{
    struct RenderSettings;
    struct TAASettings;
    struct SSAOSettings;
    struct SSRSettings;
    struct VolumetricSettings;
    struct EnvProbeSettings;
}


struct idk::TAASettings
{
    bool enabled = true;    uint32_t pad0[8];
    int  factor  = 16;      uint32_t pad1[7];
    float scale  = 1.0f;
};


struct idk::SSAOSettings
{
    bool  enabled   = false;
    int   unsharp   = 1;        uint32_t pad0[7];
    float factor    = 4.0f;
    float intensity = 0.45f;    uint32_t pad1[8];

    int   samples    = 9;
    int   iterations = 1;
    float radius     = 0.5f;
    float bias       = -0.02f;   uint32_t pad2[7];
};


struct idk::SSRSettings
{
    bool  enabled     = true;
    int   blend_mode  = 0;
    int   samples     = 32;
    int   downsamples = 6;
};


struct idk::VolumetricSettings
{
    bool  enabled        = true;    uint32_t pad0[8];

    int   res_divisor    = 2;
    int   blend_mode     = 0;
    int   samples        = 16;
    float intensity      = 0.25f;   uint32_t pad1[8];

    int   samples_sun    = 4;
    float height_offset  = 0.0f;
    float height_falloff = 1.0f;
    float scatter_coeff  = 1.0f;
    float absorb_coeff   = 3.0f;    uint32_t pad2[8];

    float worley_amp     = 1.0f;
    float worley_wav     = 1.0f;    uint32_t pad3[8];
};


struct idk::EnvProbeSettings
{
    bool enabled   = true;
    bool dirty     = true;
    bool visualize = false;

    glm::ivec3 grid_size = glm::ivec3(16, 1, 16);
    glm::vec3  cell_size = glm::vec3(4.0f, 1.0f, 4.0f);
    int        nprobes   = 16*1*16;
};



struct idk::RenderSettings
{
    TAASettings        taa;
    SSAOSettings       ssao;
    SSRSettings        ssr;
    VolumetricSettings volumetrics;
    EnvProbeSettings   envprobe;
};

