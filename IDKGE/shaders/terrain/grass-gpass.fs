#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/noise.glsl"


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;


in VS_out
{
    vec3 fragpos;
    vec2 texcoord;
    vec3 normal;
    vec3 color;
    float layer;

} fsin;


uniform sampler2D un_albedo;
uniform sampler2DArray un_diff;


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec4  albedo = texture(un_diff, vec3(fsin.texcoord, fsin.layer));
    float noise  = IDK_BlueNoise(8.0 * fsin.texcoord).r;

    if (noise > albedo.a)
    {
        discard;
    }

    fsout_albedo = vec4(fsin.color * albedo.rgb, 1.0);
    fsout_normal = normalize(fsin.normal);
    fsout_pbr    = vec4(1.0, 0.0, 1.0, 0.0);

}
