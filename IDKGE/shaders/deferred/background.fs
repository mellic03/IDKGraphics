#version 460 core
#extension GL_GOOGLE_include_directive: require


#include "../include/UBOs.glsl"
#include "../include/util.glsl"

out vec4 fsout_fragcolor;
in vec2 fsin_texcoords;

uniform sampler2D   un_fragdepth;
uniform samplerCube un_skybox;

void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();

    vec3 position = IDK_WorldFromDepth(
        un_fragdepth,
        fsin_texcoords,
        camera.P,
        camera.V
    );

    vec3 dir   = normalize(position - camera.position.xyz);
    vec3 color = texture(un_skybox, dir).rgb;

    fsout_fragcolor = vec4(color, 1.0);
}