#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;


#include "vxgi.glsl"
#include "../include/lightsource.glsl"

layout (binding=0, rgba16f) uniform image3D un_voxel_albedo;
layout (binding=1, rgba16f) uniform image3D un_voxel_normal;


layout (std430, binding = 8) buffer SBO_bindless_textures
{
    sampler2D un_bindless_samplers[128];
};
uniform int un_material_id;



#if VXGI_TEXTURE_FORMAT == FORMAT_RGBA8UI
    // void atomicAvgRGBA8( ivec3 uvt, vec3 color )
    // {
    //     imageAtomicMax(un_voxeldata, uvt, packUnorm4x8(vec4(color, 1.0 / 255.0)));

    //     uint next = packUnorm4x8(vec4(color, 1.0));
    //     uint prev = 0;
    //     uint curr = 0;

    //     while ((curr = imageAtomicCompSwap(un_voxeldata, uvt, prev, next)) != prev)
    //     {
    //         prev = curr;
    //         vec4 avg = unpackUnorm4x8(curr);
    //         uint count = uint(avg.a * 255.0);
    //         avg = vec4((avg.rgb * count * color) / float(count + 1), float(count + 1) / 255.0);
    //         next = packUnorm4x8(avg);
    //     }
    // }
#endif


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

#define ROLLING_AVG_FRAMES 60.0



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

    vec3 albedo   = texture(un_bindless_samplers[albedo_idx], fsin_texcoords).rgb;
    vec3 emission = texture(un_bindless_samplers[emissv_idx], fsin_texcoords).rgb;
    vec3 normal   = normalize(fsin_normal);


    DirLight light = un_dirlights[0];

    ivec3 texel = VXGI_WorldToTexel(fsin_fragpos, un_viewpos);

    vec4 radiance = vec4(albedo * light.diffuse.rgb, 1.0);
    // radiance *= dirlight_shadow_2(0, un_depthmap, un_view, un_light_matrix, fsin_fragpos, normal);
    // radiance *= max(dot(normal, -light.direction.xyz), 0.0);
    albedo = radiance.rgb;

    imageStore(un_voxel_albedo, texel, vec4(albedo.rgb, 1.0));
    imageStore(un_voxel_normal, texel, vec4(normal, 1.0));

    fsout_frag_color = vec4(albedo.rgb, 1.0);
}

