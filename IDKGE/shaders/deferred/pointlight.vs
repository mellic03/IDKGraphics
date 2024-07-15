#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out vec3 fsin_fragpos;
flat out int lightID;


void main()
{
    lightID = gl_InstanceID;

    IDK_Camera camera = IDK_UBO_cameras[0];
    IDK_Pointlight light = IDK_UBO_pointlights[lightID];
    fsin_fragpos = (2.0 * light.radius * vsin_pos) + light.position.xyz;

    gl_Position = camera.P * camera.V * vec4(fsin_fragpos, 1.0);
}