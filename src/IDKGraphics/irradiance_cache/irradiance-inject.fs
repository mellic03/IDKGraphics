#version 460 core
#extension GL_GOOGLE_include_directive: require
#include "irradiance-cache.glsl"


out vec4 fsout_color;
out vec2 fsin_texcoords;

uniform sampler2D un_fragcolor;
uniform sampler2D un_fragdepth;




vec3 sampleIrradiance( vec3 fragpos, vec3 dir )
{
    vec3 result = vec3(0.0);

    vec3 irradiance = IrradianceSampleWorld(fragpos);


    return result;
}



void main()
{
    IDK_Camera cam = IDK_GetCamera();

    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, fsin_texcoords, cam.P, cam.V);
    vec3 color    = texture(un_fragcolor, fsin_texcoords).rgb;

    addIrradiance(worldpos, color);
}



