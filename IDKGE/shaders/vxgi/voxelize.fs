#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;

#include "vxgi.glsl"
#include "../include/lightsource.glsl"
#include "../include/SSBO_indirect.glsl"
#include "../include/UBOs.glsl"

layout (binding=0, rgba16f) writeonly uniform image3D un_voxel_radiance[6];
layout (binding=6, rgba8ui) writeonly uniform uimage3D un_voxel_albedo;
layout (binding=7, rgba8ui) writeonly uniform uimage3D un_voxel_normal;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoords;
flat in int material_id;

uniform sampler2DShadow un_depthmap;
uniform mat4 un_light_matrix;

#define ROLLING_AVG_FRAMES 64.0



void main()
{
    if (VXGI_in_bounds(fsin_fragpos, IDK_RenderData_GetCamera().position.xyz) == false)
    {
        return;
    }


    vec4  albedo = texture(un_IndirectDrawData.materials[material_id][0], fsin_texcoords).rgba;
    // vec3  normal = texture(un_IndirectDrawData.materials[material_id][1], texcoords).xyz * 2.0 - 1.0;
    vec3  ao_r_m = texture(un_IndirectDrawData.materials[material_id][2], fsin_texcoords).rgb;
    float emissv = texture(un_IndirectDrawData.materials[material_id][3], fsin_texcoords).r;

    vec3 normal   = normalize(fsin_normal);

    ivec3 texel = VXGI_WorldToTexel(fsin_fragpos, un_viewpos);

    imageStore(un_voxel_albedo, texel, ivec4(255.0*albedo));
    imageStore(un_voxel_normal, texel, ivec4(VXGI_PackNormal(normal), 0.0));

    fsout_frag_color = albedo;
}

