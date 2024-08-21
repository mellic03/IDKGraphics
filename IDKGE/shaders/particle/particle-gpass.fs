#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "./particle.glsl"

layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoord;
flat in uint idx;


uniform sampler2D un_albedo;


float linearDepth( float z, float near, float far )
{
    return near * far / (far + z * (near - far));
}


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec4  color = texture(un_albedo, fsin_texcoord).rgba;
    float alpha = Particles[idx].color.a;

    fsout_albedo = vec4(4.0*color.rgb, alpha);
    // fsout_normal = normalize(fsin_normal);
    fsout_pbr    = vec4(1.0, 0.05, 1.0, alpha);
}
