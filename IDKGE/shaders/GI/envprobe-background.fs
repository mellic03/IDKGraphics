#version 460 core

out vec4 fsout_fragcolor;
in vec3 fsin_fragpos;

uniform samplerCube un_skybox;

void main()
{
    vec3 N = normalize(fsin_fragpos);
    vec3 result = textureLod(un_skybox, N, 0).rgb;
    fsout_fragcolor = vec4(result, 1.0);
}
