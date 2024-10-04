#ifndef IDK_WATER
#define IDK_WATER

#include "./terrain.glsl"
#include "../include/time.glsl"


vec3 WaterComputeHeight( float time, float x, float z )
{
    NoiseFactor NF = IDK_SSBO_Terrain.water;

    float xscale = IDK_SSBO_Terrain.water_scale[0];
    float yscale = IDK_SSBO_Terrain.water_scale[1];
    float tscale = IDK_SSBO_Terrain.water_scale[2];
    float wscale = IDK_SSBO_Terrain.water_scale[3];

    float t        = tscale * time;
    float height   = 0.0;
    vec2  gradient = vec2(0.0);
    float a        = yscale;
    float w        = 1.0 / xscale;

    // int waves = clamp(int(NF.octaves), 1, IDK_WATER_OCTAVES);

    int waves = int(NF.octaves);

    for (int i=0; i<waves; i++)
    {
        vec2 dir = IDK_SSBO_Water_dirs[(i+32)%IDK_WATER_OCTAVES];
        float xz = dot(vec2(x, z), dir);

        height   += a * sin(w*xz + t);
        gradient += a * w * dir * cos(w*xz + t);

        a *= NF.amp;
        w *= NF.wav;
    }

    return vec3(height, gradient);
}



#endif