#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "./particle.glsl"

layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;
layout (location = 3) out vec2 fsout_vel;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoord;
flat in uint drawID;
flat in uint idx;


uniform sampler2D un_albedo;


float linearDepth( float z, float near, float far )
{
    return near * far / (far + z * (near - far));
}


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    // uint  offset = IDK_SSBO_texture_offsets[drawID];
    // vec4  color = texture(IDK_SSBO_textures[offset+0], fsin_texcoord).rgba;

    vec4  color = texture(un_albedo, fsin_texcoord).rgba;
    float alpha = Particles[idx].col.a;

    if (color.a < 0.8)
    {
        discard;
    }

    fsout_albedo = vec4(4.0*color.rgb, alpha);
    fsout_normal = normalize(fsin_normal);
    fsout_pbr    = vec4(1.0, 0.0, 1.0, 4.0*alpha);

}