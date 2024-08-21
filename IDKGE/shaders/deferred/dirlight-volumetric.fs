#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/noise.glsl"
#include "../include/pbr.glsl"

layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in uint lightID;


uniform float un_samples     = 16.0;
uniform float un_attenuation = 0.01;
uniform float un_intensity   = 1.0;
uniform float un_factor      = 32.0;

#define NUM_SAMPLES   un_samples
#define RAY_INTENSITY un_intensity
#define BLEND_FACTOR  un_factor

// #define NUM_SAMPLES   16.0
// #define RAY_INTENSITY 1.0
// #define BLEND_FACTOR  4.0


// uniform sampler2D un_previous;
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
    float bias = -0.0001;

    const int KERNEL_HW = 1;
    vec2 tsize = 1.0 / textureSize(un_shadowmap, 0).xy;

    float shadow = 0.0;

    for (int y=-1; y<=+1; y++)
    {
        for (int x=-1; x<=+1; x++)
        {
            vec2 uv = projCoords.xy + vec2(x, y) / tsize;
            float ray_depth = texture(un_shadowmap, vec3(uv, cascade)).r;
            shadow += (frag_depth - bias) > ray_depth ? 1.0 : 0.0;
        }
    }

    return 1.0 - (shadow / 9.0);
}



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



#define V_color  vec3(0.05, 0.13, 0.22)
#define H_color1 vec3(0.59, 0.4, 0.17)
#define H_color2 vec3(0.7, 0.76, 0.66)


vec3 bad_atmospherics( vec3 ray_dir, vec3 L, IDK_Dirlight light )
{
    vec3 result = vec3(0.0);

    float VdotL    = dot(ray_dir, -L) * 0.5 + 0.5;
    float vertical = pow(1.0 - abs(ray_dir.y), 5);

    vec3 hoz = mix(H_color1, H_color2, pow(VdotL, 2));
    result = mix(V_color, hoz, vertical);

    VdotL = clamp(dot(ray_dir, -L), 0.0, 1.0);
    result += light.diffuse.rgb * IDK_MieScattering(VdotL);

    return light.diffuse.a * result;
}




void main()
{
    IDK_Camera   camera = IDK_UBO_cameras[0];
    IDK_Dirlight light  = IDK_UBO_dirlights[lightID];

    vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
    vec2 texel    = texcoord * vec2(camera.width, camera.height);

    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);
    float offset  = IDK_BlueNoise((texel / 512.0) + un_irrational).r;

    vec3  ray_dir   = normalize(worldpos - camera.position.xyz);
    vec3  ray_pos   = camera.position.xyz;

    float frag_dist = min(distance(ray_pos, worldpos), 256.0);
    float step_size = frag_dist / NUM_SAMPLES;

    ray_pos += 4.0*offset * step_size * ray_dir;
    frag_dist = min(distance(ray_pos, worldpos), 256.0);
    step_size = frag_dist / NUM_SAMPLES;

    float intensity  = un_intensity * IDK_MieScattering(max(dot(ray_dir, -light.direction.xyz), 0.0));
          intensity /= NUM_SAMPLES;

    float volumetric = 0.0;

    if (textureLod(un_fragdepth, texcoord, 0.0).r >= 0.999)
    {
        fsout_frag_color = vec4(light.diffuse.rgb, intensity);
        return;
    }


    for (int i=0; i<NUM_SAMPLES; i++)
    {
        ray_pos += step_size * ray_dir;
        volumetric += dirlight_shadow(camera, light, camera.V, ray_pos);
    }


    volumetric *= intensity;

    fsout_frag_color = vec4(light.diffuse.rgb, volumetric);


    // vec4 prev_uv = camera.P * camera.prev_V * vec4(worldpos, 1.0);
    //      prev_uv.xy = (prev_uv.xy / prev_uv.w) * 0.5 + 0.5;

    // float A = 1.0;
    // float B = 0.0;

    // vec4 curr = vec4(light.diffuse.rgb, volumetric);
    // // vec4 prev = vec4(0.0);

    // // if (prev_uv.xy == clamp(prev_uv.xy, 0.0, 1.0))
    // // {
    // //     A = 1.0 / BLEND_FACTOR;
    // //     B = (BLEND_FACTOR - 1.0) / BLEND_FACTOR;

    // //     // prev = textureLod(un_previous, prev_uv.xy, 0.0).rgba;

    // //     // for (int row=-1; row<1; row++)
    // //     // {
    // //     //     for (int col=-1; col<1; col++)
    // //     //     {
    // //     //         vec2 offset = vec2(col, row) / textureSize(un_previous, 0);
    // //     //         prev += textureLod(un_previous, prev_uv.xy+offset, 0.0);
    // //     //     }
    // //     // }

    // //     // prev /= 9.0;
    // // }

    // // fsout_frag_color = A*curr + B*prev;

}





