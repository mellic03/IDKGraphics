#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "../include/taa.glsl"
#include "./water.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out FS_in
{
    vec3 fragpos;
    vec2 texcoord;
    float dist;
    float dy;

    IDK_VelocityData vdata;

} vsout;


const uint un_num_instances = 4;
// const float[2] CLIP_W = float[2]( 0.5*96.0, 128.0 );


layout (binding=0, r32f) readonly uniform image2D un_depth;



vec4 computeVertexPosition()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 cam_xz = camera.position.xz;

    const uint maxID = IDK_TERRAIN_NUM_CLIPS;
    uint clipID = gl_DrawID + gl_InstanceID;

    vec2 pos_xz = vsin_pos.xz;
    pos_xz *= 4.0;


    // if (clipID > 0)
    // {
    //     if (pos_xz.x == 0.5 || pos_xz.y == 0.5)
    //     {
    //         clipID = clamp(clipID + 1, 0, maxID);
    //         pos_xz *= 0.5;
    //     }
    // }

    float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
    float spacing     = 2.0 * clip_scale / IDK_TERRAIN_CLIP_W_VERTS;


    pos_xz = ((clip_scale * pos_xz) + cam_xz);
    pos_xz = floor(pos_xz / spacing) * spacing;

    return vec4(pos_xz, 0.0, 0.0);
}

// vec4 computeVertexPosition()
// {
//     IDK_Camera camera = IDK_UBO_cameras[0];
//     vec2 cam_xz = camera.position.xz;

//     const uint maxID = IDK_TERRAIN_NUM_CLIPS;
//     uint clipID = gl_DrawID + gl_InstanceID;

//     vec2 pos_xz = vsin_pos.xz;

//     // if (clipID > 0 && clipID < maxID)
//     // {
//     //     if (abs(vsin_pos.x) == 0.5 || abs(vsin_pos.z) == 0.5)
//     //     {
//     //         clipID = clamp(clipID + 1, 0, maxID);
//     //         pos_xz *= 0.5;
//     //     }
//     // }

//     float max_scale   = IDK_TERRAIN_CLIP_W * pow(2, maxID);
//     float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
//     float spacing     = clip_scale / 4.0;


//     pos_xz = (clip_scale * pos_xz) + cam_xz;
//     pos_xz = floor(pos_xz / spacing) * spacing;

//     return vec4(pos_xz, 0.0, 0.0);
// }




float TerrainComputeFinalHeight( float x, float z )
{
    mat4 M = IDK_SSBO_Terrain.transform;
    float xscale = length(vec3(M[0]));
    vec2 uv = (vec2(x, z) / xscale) + 0.5;

    float height  = textureLod(IDK_SSBO_Terrain.height, uv, 0.0).r;
          height *= IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.scale.y;
          height += IDK_SSBO_Terrain.transform[3].y;

    return height;
}



void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    const uint maxID = IDK_TERRAIN_NUM_CLIPS-1;
    float max_scale  = IDK_TERRAIN_CLIP_W * pow(2, maxID);

    vec2 local = computeVertexPosition().xy;
    vsout.fragpos = vec3(local.x, 0.0, local.y);
    vsout.texcoord = 2048.0 * TerrainWorldToUV(vsout.fragpos.xz);

    vec3 prev = vsout.fragpos;
    vec3 curr = vsout.fragpos;

    {
        float xscale = IDK_SSBO_Terrain.water_scale.x;
        float yscale = IDK_SSBO_Terrain.water_scale.y;
        float wscale = IDK_SSBO_Terrain.water_scale[3];

        float prev_height, curr_height;

        float t0 = IDK_GetTime() - IDK_GetDeltaTime();
        float t1 = IDK_GetTime();
        vec3  hpd = WaterComputeHeight(t0, vsout.texcoord.x, vsout.texcoord.y).xyz;

        vsout.texcoord -= hpd.yz;

        prev_height = hpd[0];
        curr_height = WaterComputeHeight(t1, vsout.texcoord.x, vsout.texcoord.y)[0];

        vsout.texcoord += hpd.yz;

        prev.y = IDK_SSBO_Terrain.water_pos.y + prev_height;
        curr.y = IDK_SSBO_Terrain.water_pos.y + curr_height;
        vsout.vdata = PackVData(camera, curr, prev);

        vsout.dy = curr.y - prev.y;
    }

    vsout.fragpos.y = IDK_SSBO_Terrain.water_pos.y + curr.y;

    // float theight = TerrainComputeFinalHeight(vsout.fragpos.x, vsout.fragpos.z);
    // float alpha   = clamp(theight/vsout.fragpos.y, 0.0, 1.0);

    // vsout.fragpos.y = mix(vsout.fragpos.y, IDK_SSBO_Terrain.water_pos.y, alpha);



    gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

}

