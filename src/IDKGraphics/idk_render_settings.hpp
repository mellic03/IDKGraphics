#pragma once

#include "idk_render_settings_field.hpp"


namespace idk
{
    struct RenderSettings;
    struct TAASettings;
    struct SSAOSettings;
    struct SSRSettings;
    struct SSGISettings;
    struct VolumetricSettings;
    struct EnvProbeSettings;
}


struct idk::TAASettings
{
    bool  enabled = true;
    int   factor  = 4;
    float scale   = 1.0f;
};


struct idk::SSAOSettings
{
    bool  enabled   = true;
    int   samples   = 4;
    float factor    = 4.0f;
    float intensity = 0.45f;
    float radius    = 0.5f;
    float bias      = -0.02f;
};


struct idk::SSRSettings
{
    bool  enabled     = true;
    int   blend_mode  = 0;
    int   samples     = 32;
    int   downsamples = 6;
};


struct idk::SSGISettings
{
    bool  enabled     = false;
    int   factor      = 4;
    float intensity   = 1.0f;
    int   samples     = 32;
};


struct idk::VolumetricSettings
{
    bool  enabled        = false;

    int   res_divisor    = 2;
    int   blend_mode     = 0;
    int   samples        = 16;
    float intensity      = 0.25f;

    int   samples_sun    = 4;
    float height_offset  = 0.0f;
    float height_falloff = 1.0f;
    float scatter_coeff  = 1.0f;
    float absorb_coeff   = 3.0f;

    float worley_amp     = 1.0f;
    float worley_wav     = 1.0f;
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
    SSGISettings       ssgi;
    VolumetricSettings volumetrics;
    EnvProbeSettings   envprobe;
};

