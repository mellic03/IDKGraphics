#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "./include/storage.glsl"

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;


uniform uint un_light_id;
uniform uint un_draw_offset;
uniform uint un_cascade;


void main()
{
    IDK_Dirlight light = IDK_UBO_dirlights[un_light_id];

    const uint drawID = gl_DrawID + un_draw_offset;
    const uint offset = IDK_SSBO_transform_offsets[drawID];
    const mat4 model  = IDK_SSBO_transforms[offset + gl_InstanceID];

    mat4 transform = (un_cascade <= 3) ? light.transforms[un_cascade] : light.transform;

    gl_Position = transform * model * vec4(vsin_pos, 1.0);
}  



