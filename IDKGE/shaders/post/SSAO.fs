#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/noise.glsl"


layout (location = 0) out float fsout_result;

in vec2 fsin_texcoords;

uniform sampler2D un_gNormal;
uniform sampler2D un_gDepth;

uniform sampler2D un_prev;
uniform float     un_irrational;


uniform int   un_samples     = 9;
uniform float un_intensity   = 1.0;
uniform float un_factor      = 32.0;
uniform float un_ssao_radius = 1.0;
uniform float un_ssao_bias   = -0.02;

#define KERNEL_HW     1
#define SSAO_RADIUS   un_ssao_radius
#define SSAO_BIAS     un_ssao_bias
#define DEPTH_CUTOFF  0.001


float linearDepth( float z, float near, float far )
{
    return near * far / (far + z * (near - far));
}


float SSAO( vec2 texcoord, vec3 frag_pos, vec3 N, mat4 P, mat4 V )
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 image_size = vec2(camera.width, camera.height);

    vec2 texel    = texcoord * image_size;
    // vec2 noise_uv = texel / textureSize(un_noise, 0);
    vec2 offset   = un_irrational * IDK_WhiteNoise(texcoord/2.0).rg;

    vec3 sample_dir = IDK_WhiteNoise(texcoord/2.0 + offset).rgb;
         sample_dir = sample_dir * 2.0 - 1.0;
         sample_dir = normalize(sample_dir);
         sample_dir *= sign(dot(sample_dir, N));

    vec3 sample_pos = frag_pos + 0.1*sample_dir;

    vec4 projected = P * vec4(sample_pos, 1.0);
    projected.xyz /= projected.w;
    projected.xyz = projected.xyz * 0.5 + 0.5;


    float sample_depth = texture(un_gDepth, projected.xy).r * 2.0 - 1.0;
    vec3  sample_world = IDK_WorldFromDepth(un_gDepth, projected.xy, P, V);
    vec3  sample_view  = (V * vec4(sample_world, 1.0)).xyz;
          sample_depth = sample_view.z;


    float rangeCheck = smoothstep(0.0, 1.0, SSAO_RADIUS / abs(frag_pos.z - sample_depth));
    return (sample_depth >= sample_pos.z + SSAO_BIAS ? 1.0 : 0.0) * rangeCheck;

}


float convolve_prev( vec2 texcoord )
{
    vec2  size   = 1.0 / textureSize(un_prev, 0);
    float result = 0.0;

    const vec2 offsets[9] = vec2[9]
    (
        vec2( -1.0, -1.0 ),
        vec2( -1.0,  0.0 ),
        vec2( -1.0, +1.0 ),
        vec2(  0.0, -1.0 ),
        vec2(  0.0,  0.0 ),
        vec2(  0.0, +1.0 ),
        vec2( +1.0, -1.0 ),
        vec2( +1.0,  0.0 ),
        vec2( +1.0, +1.0 )
    );

    for (int i=0; i<9; i++)
    {
        result += textureLod(un_prev, texcoord + size*offsets[i], 0.0).r;
    }

    return result / 9.0;
}



void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 image_size = vec2(camera.width, camera.height);

    vec3 viewpos   = camera.position.xyz;
    vec2 texcoord  = fsin_texcoords;

    vec3 world_pos = IDK_WorldFromDepth(un_gDepth, texcoord, camera.P, camera.V);
    vec3 frag_pos  = (camera.V * vec4(world_pos, 1.0)).xyz;

    vec3 normal = texture(un_gNormal, texcoord).rgb;
         normal = (camera.V * vec4(normal, 0.0)).xyz;
         normal = normalize(normal);


    float result = 0.0;

    for (int y=-1; y<=+1; y+=2)
    {
        for (int x=-1; x<=+1; x+=2)
        {
            vec2 offset = 0.5 * vec2(x, y) / image_size;
            vec2 sample_uv = texcoord + offset;

            result += SSAO(sample_uv, frag_pos, normal, camera.P,  camera.V);
        }
    }

    result /= 4.0; // ((2*KERNEL_HW+1)*(2*KERNEL_HW+1));
    result = 1.0 - (un_intensity * result);

    vec4 prev_uv = camera.P * camera.prev_V * vec4(world_pos, 1.0);
         prev_uv.xy = (prev_uv.xy / prev_uv.w) * 0.5 + 0.5;

    float A = 1.0; // 1.0 / un_factor;
    float B = 0.0; // (un_factor - 1.0) / un_factor;

    float prev = 0.0;

    if (prev_uv.xy == clamp(prev_uv.xy, 0.0, 1.0))
    {
        A = 1.0 / un_factor;
        B = (un_factor - 1.0) / un_factor;
        prev = convolve_prev(prev_uv.xy); // texture(un_prev, prev_uv.xy).r;
    }


    result = A*result + B*prev;

    fsout_result = result;

}

