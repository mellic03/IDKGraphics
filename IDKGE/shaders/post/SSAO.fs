#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/time.glsl"
#include "../include/noise.glsl"


layout (location = 0) out float fsout_result;

in vec2 fsin_texcoords;

uniform sampler2D un_gNormal;
uniform sampler2D un_gDepth;

uniform int   un_samples     = 9;
uniform float un_intensity   = 1.0;
uniform float un_factor      = 32.0;
uniform float un_ssao_radius = 1.0;
uniform float un_ssao_bias   = -0.02;

#define KERNEL_HW     1
#define SSAO_RADIUS   un_ssao_radius
#define SSAO_BIAS     un_ssao_bias
#define DEPTH_CUTOFF  0.1



float DepthToViewZ( sampler2D depthtex, vec2 uv, mat4 P )
{
    float z = textureLod(depthtex, uv, 0.0).r * 2.0 - 1.0;

    vec4 pos  = vec4(uv * 2.0 - 1.0, z, 1.0);
         pos  = inverse(P) * vec4(pos.xyz, 1.0);
         pos /= pos.w;
    
    return pos.z;
}



vec2 rotateUV( vec2 uv, float rotation )
{
    float mid = 0.5;
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}





float SSAO( mat4 P, vec3 frag_vspace, vec3 norm_vspace, vec2 texcoord, vec2 image_size, float radius )
{
    float result = 0.0;

    vec2 sample_tex = texcoord * image_size;

    // vec3  wnoise = IDK_WhiteNoiseTexel(ivec2(texcoord*image_size)).rgb;
    // float theta  = 2.0 * 3.14159 * (wnoise.r);
    // vec2  offset = rotateUV(texcoord, theta) * image_size + IDK_Vec2Noise(int(IDK_GetFrame()));

    for (int i=0; i<un_samples; i++)
    {
        // offset += IDK_WhiteNoise(offset).rg;
        vec2 texel = image_size * (texcoord + (i+1)*IDK_GetIrrational());

        vec3  noise    = IDK_BlueNoiseTexel(ivec2(texel)).rgb * 2.0 - 1.0;
        vec3  samp_dir = normalize(noise);
              samp_dir = samp_dir * abs(noise) * sign(dot(samp_dir, norm_vspace));
    
        vec3 samp_pos = frag_vspace + radius*samp_dir;

        vec4 proj = P * vec4(samp_pos.xyz, 1.0);
             proj.xyz = (proj.xyz / proj.w) * 0.5 + 0.5;

        float real_z = samp_pos.z;
        float tex_z  = DepthToViewZ(un_gDepth, proj.xy, P);
        float delta  = (tex_z + SSAO_BIAS) - real_z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(tex_z - real_z));
        result += sign(max(delta, 0.0)) * rangeCheck;
    }

    return result;
}




void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 tsize = vec2(camera.width, camera.height);

    vec3 viewpos   = camera.position.xyz;
    vec2 texcoord  = fsin_texcoords;
    vec2 texel     = vec2(texcoord * tsize);


    if (textureLod(un_gDepth, texcoord, 0.0).r >= 1.0)
    {
        fsout_result = 1.0;
        return;
    }

    vec3 f_vspace = IDK_ViewFromDepth(un_gDepth, texcoord, camera.P);
    vec3 n_wspace = textureLod(un_gNormal, texcoord, 0.0).xyz;
    vec3 n_vspace = normalize((camera.V * vec4(n_wspace, 0.0)).xyz);

    float result = 0.0;

    float dsq    = abs(f_vspace.z) * abs(f_vspace.z);
    float radius = clamp(dsq/25.0, 0.1, 1.0);

    result += SSAO(camera.P, f_vspace, n_vspace, texcoord, tsize, radius);

    fsout_result = 1.0 - (result / float(un_samples));
}


