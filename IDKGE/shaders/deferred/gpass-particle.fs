#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/SSBO_indirect.glsl"
#include "../include/UBOs.glsl"
#include "../include/util.glsl"

layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec3 fsin_tangent;
in vec2 fsin_texcoords;
flat in int draw_id;


in vec3 TBN_viewpos;
in vec3 TBN_fragpos;
in mat3 TBN;
in mat3 TBNT;


void main()
{
    vec2 texcoords = fsin_texcoords;

    uint offset = un_IndirectDrawData.texture_offsets[draw_id];

    vec4  albedo = texture(un_IndirectDrawData.textures[offset+0], texcoords).rgba;
    vec3  normal = texture(un_IndirectDrawData.textures[offset+1], texcoords).xyz * 2.0 - 1.0;
    vec3  ao_r_m = texture(un_IndirectDrawData.textures[offset+2], texcoords).rgb;
    float noidea = texture(un_IndirectDrawData.textures[offset+3], texcoords).r;
    float emissv = texture(un_IndirectDrawData.textures[offset+4], texcoords).r;
    float ao        = ao_r_m.r;
    float roughness = ao_r_m.g;
    float metallic  = ao_r_m.b;

    if (albedo.a < 0.9)
    {
        // ivec2 texel = ivec2(fsin_texcoords * IDK_RenderData_GetCamera().image_size.xy);

        // int a = texel.x % 32;
        // int b = texel.y % 32;

        // if (a < 24 || b < 24)
        {
            discard;
        }
    }

    // vec3 N = normalize(TBN * normalize(normal)); // normalize(fsin_normal);
    //     //  N = normalize(mix(N, normalize(fsin_normal), 0.5));

    fsout_albedo = vec4(8.0*emissv*albedo.rgb, 1.0);
    // fsout_normal = IDK_PackNormal(N);
    fsout_pbr    = vec4(1.0, 0.0, 1.0, emissv);
}
