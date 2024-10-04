#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/taa.glsl"
#include "../include/util.glsl"
#include "../include/noise.glsl"

// layout (location = 0) out vec4 fsout_frag_color;
layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;
layout (location = 3) out vec4 fsout_vel;

in VS_out
{
    vec3 fragpos;
    vec2 texcoord;
    vec3 normal;
    vec3 color;
    float ao;
    float layer;

    IDK_VelocityData vdata;

} fsin;


uniform sampler2D un_albedo;
uniform sampler2DArray un_diff;


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec4  albedo = texture(un_diff, vec3(fsin.texcoord, fsin.layer));
    // float noise  = IDK_BlueNoise(4.0*fsin.texcoord).r;

    // if (noise > 1.25*albedo.a)
    // {
        // discard;
    // }

    fsout_vel = PackVelocity(fsin.vdata);


    if (albedo.a < 0.05)
    {
        discard;
    }

    albedo.rgb = mix(fsin.color, albedo.rgb, fsin.ao);


    IDK_Dirlight light = IDK_UBO_dirlights[0];

    // float NdotL = max(dot(fsin.normal, -normalize(light.direction.xyz)), 0.01);
    // vec3 diff   = light.diffuse.rgb * fsin.color * albedo.rgb * NdotL;
    // fsout_frag_color = vec4(diff, 1.0);

    float ao = 1.0 - fsin.ao;
          ao = 1.0 - (ao*ao*ao*ao);

    fsout_albedo = vec4(albedo.rgb, 1.0);
    fsout_normal = normalize(fsin.normal);
    fsout_pbr    = vec4(0.95, 0.0, ao, 0.0);

}
