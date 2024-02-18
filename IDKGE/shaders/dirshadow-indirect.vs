#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "./include/SSBO_material.glsl"

layout (location = 0) in vec3 vsin_pos;
uniform mat4 un_lightspacematrix;

void main()
{
    const mat4 model = un_ModelData.transforms[gl_DrawID];
    gl_Position = un_lightspacematrix * model * vec4(vsin_pos, 1.0);
}  



