#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;


#include "vxgi.glsl"
#include "../include/lightsource.glsl"

layout (binding=13,  rgba16f) uniform image3D un_voxel_albedo;
layout (binding=0,  rgba16f) uniform image3D un_voxel_propagation;
layout (binding=6,  rgba16f) uniform image3D un_voxel_radiance;
layout (binding=12, rgba16f) writeonly uniform image3D un_voxel_normal;


layout (std430, binding = 8) buffer SBO_bindless_textures
{
    sampler2D un_bindless_samplers[128];
};
uniform int un_material_id;


struct Camera
{
    vec4 position;
    vec4 beg;
    vec4 aberration_rg;
    vec4 aberration_b;
    vec4 exposure;
};

layout (std140, binding = 2) uniform UBO_camera_data
{
    mat4 un_view;
    mat4 un_projection;
    vec3 un_viewpos;
    Camera un_camera;
    vec3 un_cam_beg;
};


in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoords;

uniform sampler2DShadow un_depthmap;
uniform mat4 un_light_matrix;

void main()
{
    if (VXGI_in_bounds(fsin_fragpos, un_viewpos) == false)
    {
        return;
    }


    int albedo_idx = 4*un_material_id + 0;
    int normal_idx = 4*un_material_id + 1;
    int ao_r_m_idx = 4*un_material_id + 2;
    int emissv_idx = 4*un_material_id + 3;

    vec4 albedo   = texture(un_bindless_samplers[albedo_idx], fsin_texcoords);
    vec3 emission = texture(un_bindless_samplers[emissv_idx], fsin_texcoords).rgb;
    vec3 normal   = normalize(fsin_normal);


    DirLight light = un_dirlights[0];

    ivec3 texel = VXGI_WorldToTexel(fsin_fragpos, un_viewpos) - ivec3(1);

    vec3 radiance = albedo.rgb;
    radiance *= light.diffuse.rgb;
    radiance *= dirlight_shadow_2(0, un_depthmap, un_view, un_light_matrix, fsin_fragpos, normal);
    radiance *= max(dot(normal, -light.direction.xyz), 0.0);
    radiance += albedo.rgb * 10.0*emission.x;

    imageStore(un_voxel_albedo, texel, vec4(albedo.rgb, 1.0));
    imageStore(un_voxel_normal, texel, vec4(normal, 1.0));
    imageStore(un_voxel_propagation, texel, vec4(radiance, 1.0));


    // const vec3 directions[6] = vec3[]
    // (
    //     vec3(-1.0,  0.0,  0.0),
    //     vec3(+1.0,  0.0,  0.0),
    //     vec3( 0.0, -1.0,  0.0),
    //     vec3( 0.0, +1.0,  0.0),
    //     vec3( 0.0,  0.0, -1.0),
    //     vec3( 0.0,  0.0, +1.0)
    // );

    // for (int i=0; i<6; i++)
    // {
    //     float weight = dot(normal, directions[i]);
    //     weight = (weight > -0.01) ? 1.0/3.0 : 0.0;
    //     // weight = 1.0;
    //     vec4  rad = vec4(weight * radiance, 1.0);

    //     imageStore(un_voxel_propagation[i], texel, rad);
    // }


    fsout_frag_color = vec4(radiance.rgb, 1.0);
}

