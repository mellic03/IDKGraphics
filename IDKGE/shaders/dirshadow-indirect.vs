#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "./include/SSBO_indirect.glsl"

layout (location = 0) in vec3 vsin_pos;
uniform mat4 un_lightspacematrix;

void main()
{
    const uint offset = un_IndirectDrawData.transform_offsets[gl_DrawID];
    const mat4 model  = un_IndirectDrawData.transforms[offset + gl_InstanceID];
    gl_Position = un_lightspacematrix * model * vec4(vsin_pos, 1.0);
}  



