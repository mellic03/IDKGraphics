#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/bindings.glsl"
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/taa.glsl"
#include "./prim.glsl"


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;
layout (location = 3) out vec4 fsout_vel;


in VS_out
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    mat3 TBN;
    flat uint DrawID;
    flat uint InstanceID;
    IDK_VelocityData vdata;

} fsin;



void main()
{
    IDK_Prim prim = UBO_Primitives[fsin.DrawID][fsin.InstanceID];

    vec4  albedo    = prim.albedo;
    float roughness = prim.roughness;
    float metallic  = prim.metallic;
    float emission  = prim.emission;

    // vec3 N = normalize(TBN * normalize(normal)); // normalize(fsin_normal);
        //  N = normalize(mix(N, normalize(fsin_normal), 0.5));

    fsout_albedo = vec4(albedo.rgb, 1.0);
    fsout_normal = fsin.TBN * normalize(fsin.normal);
    fsout_pbr    = vec4(roughness, metallic, 1.0, emission);
    fsout_vel    = PackVelocity(fsin.vdata);
}
