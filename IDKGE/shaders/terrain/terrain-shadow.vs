#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./terrain.glsl"

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;



out TESC_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    mat3 TBN;
} vsout;



uniform uint un_light_id;
uniform uint un_cascade;

void main()
{
    vsout.fragpos  = vec3(IDK_SSBO_Terrain.transform * vec4(vsin_pos, 1.0));
    vsout.normal   = vec3(0.0, 1.0, 0.0);
    vsout.tangent  = vsin_tangent;
    vsout.texcoord = vsin_pos.xz + 0.5;

    vec3 N = vec3(0.0, 1.0, 0.0);
    vec3 T = vec3(1.0, 0.0, 0.0);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    B = normalize(B - dot(B, N) * N);
    vsout.TBN  = mat3(T, B, N);
}
