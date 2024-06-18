#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/UBOs.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"



out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_fragdepth;

#define DEPTH_CUTOFF 0.001


void main()
{
    // IDK_Camera camera = IDK_RenderData_GetCamera();
    // vec3 viewpos = camera.position.xyz;

    vec2  texcoord = fsin_texcoords;

    float depth = texture(un_fragdepth, texcoord).r;

    vec3 result = vec3(0.0);

    fsout_frag_color = vec4(in_color.rgb + result.rgb, in_color.a);
}

