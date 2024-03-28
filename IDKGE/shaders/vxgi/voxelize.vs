#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/SSBO_indirect.glsl"

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;

out vec3 fsin_fragpos;
out vec3 fsin_normal;
out vec2 fsin_texcoords;
flat out int material_id;


#include "vxgi.glsl"
#include "../include/UBOs.glsl"


uniform mat4 un_P;


void main()
{
    material_id = gl_DrawID;
    const uint offset = un_IndirectDrawData.offsets[gl_DrawID];
    const mat4 model  = un_IndirectDrawData.transforms[offset + gl_InstanceID];

    vec4 position = model * vec4(vsin_pos, 1.0);
    vec4 normal   = model * vec4(vsin_normal, 0.0);

    fsin_fragpos   = position.xyz;
    fsin_normal    = normal.xyz;
    fsin_texcoords = vsin_texcoords;

    vec3 cam_pos = IDK_RenderData_GetCamera().position.xyz;

    gl_Position = un_P * vec4(position.xyz - VXGI_truncateViewPosition(cam_pos), 1.0);
}