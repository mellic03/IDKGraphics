#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/SSBO_material.glsl"

layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec4 fsout_position;
layout (location = 2) out vec4 fsout_normal;
layout (location = 3) out vec4 fsout_pbr;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec3 fsin_tangent;
in vec2 fsin_texcoords;
flat in int material_id;


in vec3 TBN_viewpos;
in vec3 TBN_fragpos;
in mat3 TBN;
in mat3 TBNT;


void main()
{
    vec2 texcoords = fsin_texcoords;

    vec4  albedo = texture(un_ModelData.materials[material_id][0], texcoords).rgba;
    vec3  normal = texture(un_ModelData.materials[material_id][1], texcoords).xyz * 2.0 - 1.0;
    vec3  ao_r_m = texture(un_ModelData.materials[material_id][2], texcoords).rgb;
    float emissv = texture(un_ModelData.materials[material_id][3], texcoords).r;

    if (albedo.a < 0.8)
        discard;

    float ao        = ao_r_m.r;
    float roughness = ao_r_m.g;
    float metallic  = ao_r_m.b;

    vec3 N = normalize(TBN * normal);

    fsout_albedo   = vec4(albedo.rgb, 1.0);
    fsout_position = vec4(fsin_fragpos, 1.0);
    fsout_normal   = vec4(N, 1.0);
    fsout_pbr      = vec4(roughness, metallic, ao, emissv.x);
}
