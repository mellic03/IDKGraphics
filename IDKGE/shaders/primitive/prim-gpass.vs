#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/taa.glsl"
#include "./prim.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;



out VS_out
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    mat3 TBN;
    flat uint DrawID;
    flat uint InstanceID;
    IDK_VelocityData vdata;

} vsout;




void main()
{
    vsout.DrawID = gl_DrawID;
    vsout.InstanceID = gl_InstanceID;

    IDK_Camera camera = IDK_UBO_cameras[0];

    const mat4 model  = UBO_Primitives[gl_DrawID][gl_InstanceID].T;
    const mat4 prev_T = UBO_Primitives[gl_DrawID][gl_InstanceID].prev_T;

    vec4 position = model * vec4(vsin_pos,     1.0);
    vec4 normal   = model * vec4(vsin_normal,  0.0);
    vec4 tangent  = model * vec4(vsin_tangent, 0.0);

    vec3 curr = (model * vec4(vsin_pos, 1.0)).xyz;
    vec3 prev = (prev_T * vec4(vsin_pos, 1.0)).xyz;
    vsout.vdata = PackVData(camera, curr, prev);

    vec3 N = normalize(mat3(model) * normalize(vsin_normal));
    vec3 T = normalize(mat3(model) * normalize(vsin_tangent));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    B = normalize(B - dot(B, N) * N);

    vsout.fragpos  = position.xyz;
    vsout.normal   = N;
    vsout.tangent  = T;
    vsout.texcoord = vsin_texcoords;
    vsout.TBN  = mat3(T, B, N);

    gl_Position = camera.P * camera.V * position;
}