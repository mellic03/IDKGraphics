#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/UBOs.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"



out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_input;

uniform sampler2D un_albedo;
uniform sampler2D un_normal;
uniform sampler2D un_pbr;
uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;
uniform samplerCube un_skybox;


#define RAY_OFFSET 0.02
#define RAY_STEP_SIZE 1.0
#define RAY_MAX_STEPS 128

#define MIPLEVEL_SPECULAR 5.0


void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();
    vec3 viewpos = camera.position.xyz;

    vec2  texcoord = fsin_texcoords;
    vec3  position = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load(
        camera,
        texcoord,
        un_fragdepth,
        un_albedo,
        un_normal,
        un_pbr,
        un_BRDF_LUT
    );

    if (texture(un_normal, texcoord).a > 1.0)
    {
        fsout_frag_color = textureLod(un_input, texcoord, 0.0);
        return;
    }

    // if (surface.roughness > 0.4)
    // {
    //     vec4 current = imageLoad(un_input, texel);
    //     imageStore(un_output, texel, current);
    //     return;
    // }

    if (dot(surface.R, surface.V) > 0.0)
    {
        fsout_frag_color = textureLod(un_input, texcoord, 0.0);
        return;
    }


    vec3 ray_pos = position + RAY_OFFSET*surface.N;
    vec3 cam_to_frag = normalize(ray_pos - viewpos);
    vec3 ray_dir = surface.R;

    vec3  result  = texture(un_skybox, ray_dir).rgb;
    int   count   = 0;
    float cumdist = 0.0;

    const mat4 PV = camera.PV;

    for (float i=0; i<RAY_MAX_STEPS; i++)
    {
        // Project ray into UV space
        // ---------------------------------------------------------------------------
        vec4 projected = PV * vec4(ray_pos, 1.0);
        projected.xy /= projected.w;
        projected.xy = projected.xy * 0.5 + 0.5;

        ivec2 tx = ivec2(projected.xy * camera.image_size.xy);
        vec2  uv = projected.xy;



        // float frag_depth = (PV * imageLoad(un_position, tx)).z;
        vec3 pos = IDK_WorldFromDepth(un_fragdepth, uv, camera.P, camera.V);

        float frag_depth = (PV * vec4(pos, 1.0)).z;
        float ray_depth  = IDK_WorldToUV(ray_pos, PV).z;
        // ---------------------------------------------------------------------------

        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        {
            result = vec3(0.0);
            break;
        }

        if (ray_depth >= frag_depth)
        {
            result = textureLod(un_input, uv, MIPLEVEL_SPECULAR*surface.roughness).rgb;
            break;
        }

        ray_pos += RAY_STEP_SIZE * ray_dir;
        cumdist += RAY_STEP_SIZE;
    }

    result *= fresnelSchlickR(surface.NdotV, surface.F0, surface.roughness);

    vec4 current = textureLod(un_input, fsin_texcoords, 0.0);
    fsout_frag_color = vec4(current.rgb + result, 1.0);
}

