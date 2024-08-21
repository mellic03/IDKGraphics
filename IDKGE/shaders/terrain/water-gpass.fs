#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./water.glsl"




layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;


in FS_in
{
    vec3 fragpos;
    vec2 texcoord;
    float dist;
} fsin;




vec3 computeNormal()
{
    float x = fsin.texcoord.x;
    float z = fsin.texcoord.y;

    float t  = IDK_GetTime();

    vec2 pd = WaterComputeHeight(t, x, z).yz;
         pd /= IDK_SSBO_Terrain.water_scale[0];
         pd *= IDK_SSBO_Terrain.water_scale[1];

    float dX = pd[0];
    float dZ = pd[1];
 
    vec3 T = vec3(1.0, dX, 0.0);
    vec3 B = vec3(0.0, dZ, -1.0);
    vec3 N = normalize(cross(T, B));
         N = vec3(N.x, N.y, -N.z);

    return N;
}


vec3 computeNormal2()
{
    vec3 fdx = vec3(dFdx(fsin.fragpos.x), dFdx(fsin.fragpos.y), dFdx(fsin.fragpos.z));
    vec3 fdy = vec3(dFdy(fsin.fragpos.x), dFdy(fsin.fragpos.y), dFdy(fsin.fragpos.z));

    return normalize(cross(fdx, fdy));
}



layout (binding=0, r32f) readonly uniform image2D un_depth;

vec3 IDK_WorldToUV( vec3 world_position, mat4 PV )
{
    vec4 proj = PV * vec4(world_position, 1.0);
    proj.xy = (proj.xy / proj.w) * 0.5 + 0.5;

    return proj.xyz;
}


void main()
{
    IDK_Camera cam = IDK_UBO_cameras[0];

    vec2  texcoord = IDK_WorldToUV(fsin.fragpos, cam.P * cam.V).xy;
    ivec2 texel    = ivec2(texcoord * vec2(cam.width, cam.height));

    float terrain_dist = imageLoad(un_depth, texel).r;
    float water_dist   = distance(fsin.fragpos, cam.position.xyz);
    float water_depth  = terrain_dist - water_dist;
    // float alpha        = 1.0 - (1.0 / (4.0*water_depth + 1.0));

    // vec4 albedo = vec4(0.3, 0.38, 0.44, alpha);

    float max_depth = 8.0;
    float alpha = clamp(water_depth / max_depth, 0.0, 1.0);

    alpha = 1.0 / (1.0 + 0.04*water_depth*water_depth + 0.01*water_depth);

    vec4 shallow_color = IDK_SSBO_Terrain.water_color[0];
    vec4 deep_color    = IDK_SSBO_Terrain.water_color[1];

    vec4 albedo = mix(shallow_color, deep_color, 1.0 - alpha);
        //  albedo.a = alpha;

    vec3 normal;

    // if (texcoord.x < 0.5)
    {
        normal = computeNormal();
    }

    // float foam_factor = 1.0 - normal.y;
    // albedo.rgb = mix(albedo.rgb, vec3(1.0), foam_factor);

    // else 
    // {
    //     normal = computeNormal2();
    // }

    // albedo.rgb = vec3(fsin.dist);

    fsout_albedo = albedo;
    fsout_normal = normal;
    fsout_pbr    = IDK_SSBO_Terrain.water_color[3];
}
