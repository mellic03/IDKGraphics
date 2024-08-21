#pragma once


namespace idk
{
    struct RenderSettings;

    struct SSAOSettings;
    struct VolumetricSettings;
    struct EnvProbeSettings;
}



struct idk::SSAOSettings
{
    bool  enabled   = false;
    float factor    = 4.0f;
    float intensity = 0.45f;

    int   samples   = 9;
    float radius    = 0.5f;
    float bias      = -0.02f;
};


struct idk::VolumetricSettings
{
    bool  enabled     = true;
    float samples     = 16.0f;
    float attenuation = 1.0f;
    float intensity   = 4.0f;
    float factor      = 2.0f;

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
    bool dirlight_volumetrics = true;

    SSAOSettings       ssao;
    VolumetricSettings volumetrics;
    EnvProbeSettings   envprobe;
};

