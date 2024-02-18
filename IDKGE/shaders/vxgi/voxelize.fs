#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;

#include "vxgi.glsl"
#include "../include/lightsource.glsl"
#include "../include/SSBO_material.glsl"
#include "../UBOs/UBOs.glsl"

layout (binding=0, rgba16f) writeonly uniform image3D un_voxel_radiance[6];
layout (binding=6, rgba16f) writeonly uniform image3D un_voxel_albedo;
layout (binding=7, rgba16f) writeonly uniform image3D un_voxel_normal;


// struct Camera
// {
//     vec4 position;
//     vec4 beg;
//     vec4 aberration_rg;
//     vec4 aberration_b;
//     vec4 exposure;
// };

// layout (std140, binding = 2) uniform UBO_camera_data
// {
//     mat4 un_view;
//     mat4 un_projection;
//     vec3 un_viewpos;
//     Camera un_camera;
//     vec3 un_cam_beg;
// };


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


    vec4  albedo = texture(un_ModelData.materials[material_id][0], fsin_texcoords).rgba;
    // vec3  normal = texture(un_ModelData.materials[material_id][1], texcoords).xyz * 2.0 - 1.0;
    vec3  ao_r_m = texture(un_ModelData.materials[material_id][2], fsin_texcoords).rgb;
    float emissv = texture(un_ModelData.materials[material_id][3], fsin_texcoords).r;

    // vec4 albedo   = texture(un_bindless_samplers[albedo_idx], fsin_texcoords);
    // vec3 emission = vec3(0.0); // texture(un_bindless_samplers[emissv_idx], fsin_texcoords).rgb;
    vec3 normal   = normalize(fsin_normal);

    DirLight light = un_dirlights[0];

    ivec3 texel = VXGI_WorldToTexel(fsin_fragpos, un_viewpos);

    imageStore(un_voxel_albedo, texel, albedo);
    imageStore(un_voxel_normal, texel, vec4(normal, 1.0));

    fsout_frag_color = albedo;
}

