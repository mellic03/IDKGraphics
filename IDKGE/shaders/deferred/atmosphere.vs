#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out vec3 fsin_fragpos;
flat out int idk_AtmosphereID;

#include "../include/SSBO_indirect.glsl"
#include "../UBOs/UBOs.glsl"


void main()
{
    idk_AtmosphereID = gl_InstanceID;

    IDK_Camera camera = IDK_RenderData_GetCamera();
    IDK_Atmosphere atmosphere = un_RenderData.atmospheres[idk_AtmosphereID];

    vec4 position = atmosphere.transform * vec4(vsin_pos, 1.0);
    fsin_fragpos  = position.xyz;

    gl_Position = camera.P * camera.V * position;
}