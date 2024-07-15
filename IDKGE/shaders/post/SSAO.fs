#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"


layout (location = 0) out float fsout_result;

in vec2 fsin_texcoords;

uniform sampler2D un_gNormal;
uniform sampler2D un_gDepth;

uniform sampler2D un_prev;

uniform sampler2D un_noise;
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
    vec2 offset = un_irrational * texture(un_noise, texcoord).rg;

    vec3 sample_dir = texture(un_noise, texcoord + offset).rgb;
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


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 image_size = vec2(camera.width, camera.height);;

    vec3  viewpos    = camera.position.xyz;
    vec2  texcoord   = fsin_texcoords;

    vec3 frag_pos = IDK_WorldFromDepth(un_gDepth, texcoord, camera.P, camera.V);
         frag_pos = (camera.V * vec4(frag_pos, 1.0)).xyz;

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

    float A = 1.0 / un_factor;
    float B = (un_factor - 1.0) / un_factor;

    float prev = texture(un_prev, texcoord).r;
    result = A*result + B*prev;

    fsout_result = result;

}

