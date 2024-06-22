#version 460 core
#extension GL_GOOGLE_include_directive: require


#include "../include/storage.glsl"

out vec4 fsout_fragcolor;
in vec2 fsin_texcoords;

uniform samplerCube un_skybox;

void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();

    vec3 dir   = inverse(mat3(camera.P * camera.V)) * vec3(fsin_texcoords * 2.0 - 1.0, 1.0);
    vec3 color = texture(un_skybox, dir).rgb;

    color = clamp(color, 0.0, 2.0);

    if (color != color)
    {
        return;
    }

    fsout_fragcolor = vec4(color, 1.0);
}