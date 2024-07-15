#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"

layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in uint lightID;


uniform sampler2D un_previous;
uniform sampler2D un_noise;
uniform float     un_irrational;

uniform sampler2D un_fragdepth;
uniform sampler2DArray un_shadowmap;



float dirlight_shadow( IDK_Camera camera, IDK_Dirlight light, mat4 V, vec3 position )
{
    vec3 L = normalize(-light.direction.xyz);

    vec3 fragpos_viewspace = (V * vec4(position, 1.0)).xyz;
    vec4 cascade_depths = light.cascades;
    vec4 res     = step(cascade_depths, vec4(abs(fragpos_viewspace.z)));
    int  cascade = int(res.x + res.y + res.z + res.w);
         cascade = clamp(cascade, 0, 3);

    vec4  fragpos_lightspace = light.transforms[cascade] * vec4(position, 1.0);
    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = abs(projCoords.z);
    float bias = -0.001;

    const int KERNEL_HW = 1;
    vec2 texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;

    float ray_depth = texture(un_shadowmap, vec3(projCoords.xy, cascade)).r;
    float shadow    = (frag_depth - bias) > ray_depth ? 1.0 : 0.0;

    return 1.0 - shadow;
}



#define MAX_STEPS 16
#define SHAFT_INTENSITY 0.5


float IDK_PHG( float g, float cosTheta )
{
    const float Inv4Pi = 0.07957747154594766788;
    
    float gSq = g * g;
    float denomPreMul = 1 + gSq - (2.0 * g * cosTheta);
    return (1 - gSq) * Inv4Pi * inversesqrt(denomPreMul * denomPreMul * denomPreMul);
}

float IDK_MieScattering( float cosTheta )
{
    return mix(IDK_PHG(0.8, cosTheta), IDK_PHG(-0.5, cosTheta), 0.5);
}

float IDK_RayleighScattering( float cosTheta )
{
    return ((3.0*3.14159) / 16.0) * (1.0 + cosTheta*cosTheta);
}



void main()
{
    IDK_Camera   camera = IDK_UBO_cameras[0];
    IDK_Dirlight light  = IDK_UBO_dirlights[lightID];

    vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);
    ivec2 texel = ivec2( texcoord * vec2(camera.width, camera.height));
    vec2 noise_texcoord = vec2(texel) / textureSize(un_noise, 0);

    vec2 offset    = un_irrational * texture(un_noise, texcoord).rg;
         offset.r  = textureLod(un_noise, noise_texcoord, 0).r;

    vec3  ray_dir   = normalize(worldpos - camera.position.xyz);
    vec3  ray_pos   = camera.position.xyz + offset.r*ray_dir;
    float frag_dist = distance(ray_pos, worldpos);

    if (frag_dist > 0.9*camera.far)
    {
        vec3 result = SHAFT_INTENSITY * light.diffuse.rgb;
             result *= IDK_RayleighScattering(max(dot(ray_dir, -light.direction.xyz), 0.0));

        fsout_frag_color = vec4(result, 1.0);
        return;
    }

    const float step_size = frag_dist / MAX_STEPS;
    float volumetric = 0.0;

    for (int i=0; i<MAX_STEPS; i++)
    {
        ray_pos += step_size * ray_dir;
        volumetric += dirlight_shadow(camera, light, camera.V, ray_pos);
    }

    volumetric = SHAFT_INTENSITY * (volumetric / MAX_STEPS);
    volumetric *= IDK_RayleighScattering(max(dot(ray_dir, -light.direction.xyz), 0.0));

    // attenuate if too close to camera
    float cam_dist = distance(worldpos, camera.position.xyz);
          cam_dist = clamp(cam_dist, 0.0, 0.5);

    volumetric *= (cam_dist / 0.5);

    float A = 1.0 / 2.0;
    float B = (2.0 - 1.0) / 2.0;

    float prev = texture(un_previous, texcoord).r;
    vec3 result = A*volumetric*light.diffuse.rgb + B*prev;

    fsout_frag_color = vec4(result, SHAFT_INTENSITY);

}





