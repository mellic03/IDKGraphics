#version 460 core
#extension GL_GOOGLE_include_directive: require

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;

out vec3 fsin_fragpos;
out vec3 fsin_normal;
out vec2 fsin_texcoords;


#include "vxgi.glsl"


struct Camera
{
    vec4 position;
    vec4 beg;
    vec4 aberration_rg;
    vec4 aberration_b;
    vec4 exposure;
};

layout (std140, binding = 2) uniform UBO_camera_data
{
    mat4 un_view;
    mat4 un_projection;
    vec3 un_viewpos;
    Camera un_camera;
    vec3 un_cam_beg;
};


uniform mat4 un_P;
uniform mat4 un_model;


void main()
{
    vec4 position = un_model * vec4(vsin_pos, 1.0);
    vec4 normal   = un_model * vec4(vsin_normal, 0.0);

    fsin_fragpos   = position.xyz;
    fsin_normal    = normal.xyz;
    fsin_texcoords = vsin_texcoords;

    gl_Position = un_P * vec4(position.xyz - VXGI_truncateViewPosition(un_viewpos), 1.0);
}