#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"



layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in int lightID;


uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;

uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;





void main()
{
    IDK_Camera     camera = IDK_UBO_cameras[0];
    IDK_Pointlight light  = IDK_UBO_pointlights[lightID];

    vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load(
        camera,
        texcoord,
        un_fragdepth,
        un_texture_0, 
        un_texture_1,
        un_texture_2,
        un_BRDF_LUT
    );

    vec3 result = IDK_PBR_Pointlight(light, surface, worldpos);
    result = (result == result) ? result : vec3(0.0);

    fsout_frag_color = vec4(result, surface.alpha);
}
