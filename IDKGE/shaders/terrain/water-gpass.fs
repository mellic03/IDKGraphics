#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/taa.glsl"
#include "./water.glsl"


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;
layout (location = 3) out vec4 fsout_vel;


in FS_in
{
    vec3 fragpos;
    vec2 texcoord;
    float dist;
    float dy;

    IDK_VelocityData vdata;
} fsin;



vec3 computeNormal()
{
    float x = fsin.texcoord.x;
    float z = fsin.texcoord.y;

    float xscale = IDK_SSBO_Terrain.water_scale.x;
    float yscale = IDK_SSBO_Terrain.water_scale.y;
    float wscale = IDK_SSBO_Terrain.water_scale[3];

    float t0 = IDK_GetTime() - IDK_GetDeltaTime();
    float t1 = IDK_GetTime();
    vec2  pd = WaterComputeHeight(t0, x, z).yz;

    x -= pd[0];
    z -= pd[1];

    pd = WaterComputeHeight(t1, x, z).yz;

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



uniform sampler2D un_tmp_depth;

void main()
{
    IDK_Camera cam = IDK_UBO_cameras[0];

    vec2  texcoord = IDK_WorldToUV(fsin.fragpos, cam.P * cam.V).xy;
    ivec2 texel    = ivec2(texcoord * vec2(cam.width, cam.height));

    float prev_z = IDK_ViewFromDepth(un_tmp_depth, texcoord, cam.P).z;
    float curr_z = (cam.V * vec4(fsin.fragpos, 1.0)).z;
    float wdepth = prev_z - curr_z;
    
    float A = IDK_SSBO_Terrain.water_color[2].r / 255.0;
    float B = IDK_SSBO_Terrain.water_color[2].g / 255.0;
    float C = IDK_SSBO_Terrain.water_color[2].b / 255.0;
    float alpha  = 1.0 / (1.0 + B*wdepth + C*wdepth*wdepth);


    vec4 shallow_color = IDK_SSBO_Terrain.water_color[0];
    vec4 deep_color    = IDK_SSBO_Terrain.water_color[1];

    vec4 albedo = mix(shallow_color, deep_color, 1.0-alpha);
    vec3 normal = computeNormal();

    IDK_Dirlight light = IDK_UBO_dirlights[0];


    vec4 diffuse = light.diffuse;
    vec3 V = normalize(fsin.fragpos - cam.position.xyz);
    vec3 L = normalize(-light.direction.xyz);

    float transmittance = max(dot(V, L), 0.0);
          transmittance = pow(transmittance, 8.0);
    // float transmittance = max(dot(normal, light.direction.xyz), 0.0);
    //       transmittance *= max(dot(normal, V), 0.0);

    albedo = mix(albedo, diffuse*shallow_color, transmittance);

    // // if (fsin.dy < -0.5)
    // {
    //     float a = clamp(-fsin.dy, 0.0, 1.0);
    //     albedo = mix(albedo, vec4(1.0), a);
    // }


    // float foam = abs(normal.y);
    //       foam = 1.0 - pow(foam, 4);

    // albedo = mix(albedo, vec4(1.0), foam);

    // if (texcoord.x > 0.5)
    // {
    //     normal = computeNormal2();
    // }


    fsout_albedo = albedo;
    fsout_normal = normal;
    fsout_pbr    = IDK_SSBO_Terrain.water_color[3];
    fsout_vel    = PackVelocity(fsin.vdata);

}
