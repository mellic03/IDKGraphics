#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/noise.glsl"
#include "../include/time.glsl"



out vec4 fsout_frag_color;
in vec2 fsin_texcoords;

uniform sampler2D un_normal;
uniform sampler2D un_fragdepth;

#define RAY_OFFSET 0.02
#define RAY_STEP_SIZE 1.0
#define RAY_MAX_STEPS 128


float getFadeFactor()
{
    vec2 texcoord = gl_FragCoord.xy / textureSize(un_fragdepth, 0);

    float edgeThickness = 0.05;

    float edgeFadeFactor = smoothstep(0.0, edgeThickness, texcoord.x) * 
                           smoothstep(0.0, edgeThickness, texcoord.y) *
                           smoothstep(1.0, 1.0 - edgeThickness, texcoord.x) * 
                           smoothstep(1.0, 1.0 - edgeThickness, texcoord.y);

    return edgeFadeFactor;
}


float DepthToViewZ( sampler2D depthtex, vec2 uv, mat4 P )
{
    float z = textureLod(depthtex, uv, 0.0).r * 2.0 - 1.0;

    vec4 pos  = vec4(uv * 2.0 - 1.0, z, 1.0);
         pos  = inverse(P) * vec4(pos.xyz, 1.0);
         pos /= pos.w;
    
    return pos.z;
}



void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();
    vec3 viewpos = camera.position.xyz;

    vec2 texcoord = fsin_texcoords;
    vec3 fragpos  = vec3(0.0);

    {
        float depth = textureLod(un_fragdepth, texcoord, 0.0).r;

        vec4 ndc;
             ndc.xy = texcoord * 2.0 - 1.0;
             ndc.z  = depth * 2.0 - 1.0;
             ndc.w  = 1.0;

        vec4 view = inverse(camera.V) * inverse(camera.P) * ndc;

        fragpos = (view.xyz / view.w);
    }


    vec3 N = normalize(textureLod(un_normal, texcoord, 0.0).xyz);
    vec3 L = -IDK_UBO_dirlights[0].direction.xyz;

    vec2  tsize = textureSize(un_fragdepth, 0);
    vec2  texel = tsize * (texcoord + IDK_GetIrrational());
    vec2  bnoise = IDK_BlueNoiseTexel(ivec2(texel)).rg;
    float n0 = bnoise[0] * 0.25;
    float n1 = bnoise[1] + 0.5;


    if (textureLod(un_fragdepth, texcoord, 0.0).r >= 1.0)
    {
        fsout_frag_color = vec4(1.0);
        return;
    }

    #define MAXDIST    0.75
    #define STEPS      32
    #define STEP_SIZE (MAXDIST / STEPS)
    #define THICKNESS  -0.05

    float occlusion = 0.0;

    vec3 ro = (camera.V * vec4(fragpos, 1.0)).xyz;
    vec3 rd = normalize((camera.V * vec4(L, 0.0)).xyz);
    vec3 rn = normalize((camera.V * vec4(N, 0.0)).xyz);
    vec3 rp = ro;

    for (int i=0; i<STEPS; i++)
    {
        rp += STEP_SIZE*rd;

        vec4 proj = camera.P * vec4(rp.xyz, 1.0);
        vec2 uv   = (proj.xy / proj.w) * 0.5 + 0.5;

        if (uv.x <= 0.0 || uv.x >= 1.0 || uv.y <= 0.0 || uv.y >= 1.0)
        {
            break;
        }

        float tex_z = DepthToViewZ(un_fragdepth, uv, camera.P);
        float delta = (rp.z) - (tex_z);

        if ((delta < 0.0) && (delta > THICKNESS))
        {
            occlusion = 1.0;
            break;
        }
    }


    fsout_frag_color = vec4(0.0);
}



