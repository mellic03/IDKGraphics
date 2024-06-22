#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"


layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in int idk_LightID;


uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;

uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;



float PHG( float g, float cosTheta )
{
    const float Inv4Pi = 0.07957747154594766788;
    
    float gSq = g * g;
    float denomPreMul = 1 + gSq - (2.0 * g * cosTheta);
    return (1 - gSq) * Inv4Pi * inversesqrt(denomPreMul * denomPreMul * denomPreMul);
}


float miePhase( float cosTheta )
{
    return mix(PHG(0.8, cosTheta), PHG(-0.5, cosTheta), 0.5);
}


float rayleigh( float cosTheta )
{
    return ((3.0*3.14159) / 16.0) * (1.0 + cosTheta*cosTheta);
}




vec3 IDK_Spotlight_Volumetrics( IDK_Spotlight light, vec3 viewpos, vec3 worldpos )
{
    vec3 result  = vec3(0.0);
    vec3 diffuse = light.diffuse.rgb;

    float frag_dist = distance(viewpos, worldpos);
    vec3  ray_pos   = viewpos;
    vec3  ray_dir   = normalize(worldpos - ray_pos);


    const float step_size = frag_dist / 32;

    for (int i=0; i<32; i++)
    {
        result += diffuse;
        ray_pos += step_size * ray_dir;
    }

    vec3 L = normalize(light.position.xyz - viewpos);

    float costheta = dot(ray_dir, L);
    float mie  = miePhase(costheta);
    float rayl = rayleigh(costheta);

    vec3 A = vec3(220, 165, 18) / 255.0;
    vec3 B = vec3(116, 136, 162) / 255.0;

    result *= 0.01 * step_size * (mie*A + rayl*B);

    return result;
}



void main()
{
    IDK_Camera    camera = IDK_RenderData_GetCamera();
    IDK_Spotlight light  = IDK_RenderData_GetSpotlight(idk_LightID);

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

    vec3 result  = IDK_PBR_Spotlight(light, surface, worldpos);

    fsout_frag_color = vec4(result, surface.alpha);
}
