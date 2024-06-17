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


#define RAY_OFFSET 0.02
#define RAY_STEP_SIZE 1.0
#define RAY_MAX_STEPS 128

#define MIPLEVEL_SPECULAR 4.0


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

    vec4 in_color = texture(un_input, texcoord);

    if (surface.alpha < 1.0)
    {
        fsout_frag_color = in_color;
        return;
    }

    if (dot(surface.R, surface.V) > 0.0)
    {
        fsout_frag_color = in_color;
        return;
    }


    vec3 ray_pos = position + RAY_OFFSET*surface.N;
    float initial_depth = IDK_WorldToUV(ray_pos, camera.PV).z;

    vec3 view_dir = inverse(mat3(camera.PV)) * vec3(fsin_texcoords * 2.0 - 1.0, 1.0);
    vec3 ray_dir  = surface.R;

    if (dot(view_dir, ray_dir) < 0.3)
    {
        fsout_frag_color = in_color;
        return;
    }



    vec4  result  = vec4(0.0, 0.0, 0.0, 1.0);
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
            fsout_frag_color = in_color;
            return;
        }

        else if (ray_depth >= frag_depth && frag_depth > initial_depth)
        {
            result = textureLod(un_input, uv, MIPLEVEL_SPECULAR*surface.roughness);
            break;
        }

        ray_pos += RAY_STEP_SIZE * ray_dir;
        cumdist += RAY_STEP_SIZE;
    }

    result.rgb *= fresnelSchlickR(surface.NdotV, surface.F0, surface.roughness);

    fsout_frag_color = vec4(in_color.rgb + result.rgb, in_color.a);
}

