#pragma once

namespace idk::shader_bindings
{
    static constexpr int UBO_Lightsources  = 0;
    static constexpr int UBO_GBuffer       = 1;
    static constexpr int SSBO_Indirect     = 0;

    static constexpr int UBO_EnvProbes     = 2;
    static constexpr int SSBO_Irradiance   = 1;

    static constexpr int UBO_Primitives    = 3;

    static constexpr int SSBO_ParticleDesc = 3;
    static constexpr int SSBO_Particles    = 4;

    static constexpr int SSBO_Terrain      = 5;
    static constexpr int SSBO_Water        = 6;
    static constexpr int SSBO_Grass        = 7;

    static constexpr int SSBO_Noise        = 8;
    static constexpr int SSBO_NoiseGen     = 9;

    static constexpr int UBO_Time          = 8;

};

