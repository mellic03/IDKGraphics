#version 460 core

#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;

#include "../include/storage.glsl"
#include "../vxgi/vxgi.glsl"
#include "../include/storage.glsl"

#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/lightsource.glsl"



in vec2 fsin_texcoords;
uniform sampler2D un_fragdepth;


#define MAX_STEPS 32
#define SHAFT_INTENSITY 0.0005




void main()
{
    vec4 result     = vec4(0.0);
    // vec4 diffuse    = vec4(un_dirlights[0].diffuse.xyz, 1.0);

    // IDK_Camera camera = IDK_RenderData_GetCamera();

    // vec3  viewpos    = camera.position.xyz;
    // vec3  frag_pos   = IDK_WorldFromDepth(un_fragdepth, fsin_texcoords, camera.P, camera.V);
    // float frag_dist  = distance(camera.position.xyz, frag_pos);
    // vec3  ray_dir    = normalize(frag_pos - camera.position.xyz);

    // const float step_size = frag_dist / MAX_STEPS;


    // for (int i=0; i<MAX_STEPS; i++)
    // {
    //     vec3 ray_pos = camera.position.xyz + float(i) * step_size * ray_dir;
    //     result += SHAFT_INTENSITY * diffuse * dirlight_shadow_NoSlopeBias(0, camera.V, ray_pos);
    // }

    // result.a = (frag_dist < camera.image_plane[1]) ? 1.0 : 0.0;

    fsout_frag_color = result;
}

