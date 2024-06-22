#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"

layout (location = 0) in vec3 vsin_pos;
uniform mat4 un_P;

void main()
{
    const uint offset = IDK_SSBO_offsets[gl_DrawID];
    const mat4 model  = IDK_SSBO_transforms[offset + gl_InstanceID];

    gl_Position = un_P * model * vec4(vsin_pos, 1.0);
}  



