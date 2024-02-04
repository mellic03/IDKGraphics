#version 460 core

layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec4 fsout_position_metallic;
layout (location = 2) out vec4 fsout_normal_ao;
layout (location = 3) out vec4 fsout_roughness_ref;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoords;

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
};

void main()
{
    vec3 normal = normalize(un_camera.position.xyz - fsin_fragpos);

    fsout_albedo            = vec4(1.0);
    fsout_position_metallic = vec4(fsin_fragpos, 0.01);
    fsout_normal_ao         = vec4(normal, 1.0);
    fsout_roughness_ref     = vec4(0.9, 0.0, 0.0, 0.0);
}
