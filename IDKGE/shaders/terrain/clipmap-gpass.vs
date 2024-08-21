#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./terrain.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float clipID;
} vsout;


uniform uint un_num_instances;



vec4 computeVertexPosition()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 cam_xz = camera.position.xz;

    const uint maxID = IDK_TERRAIN_NUM_CLIPS;
    uint clipID = gl_DrawID + gl_InstanceID;
    vsout.clipID = clipID;

    vec2 pos_xz = vsin_pos.xz;


    float max_scale   = IDK_TERRAIN_CLIP_W * pow(2, maxID);
    float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
    float spacing     = clip_scale / IDK_TERRAIN_CLIP_W_VERTS;


    pos_xz = (clip_scale * pos_xz) + cam_xz;
    pos_xz = floor(pos_xz / spacing) * spacing;

    return vec4(pos_xz, 0.0, 0.0);
}


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    const uint maxID = IDK_TERRAIN_NUM_CLIPS-1;
    float max_scale  = IDK_TERRAIN_CLIP_W * pow(2, maxID);

    vec4 local = computeVertexPosition();
    vsout.fragpos = vec3(local.x, 0.0, local.y);
    vsout.texcoord = (vsout.fragpos.xz / max_scale) + 0.5;

    vsout.fragpos.y = TerrainSampleHeight(vsout.texcoord).y;
    vsout.fragpos.y += IDK_SSBO_Terrain.transform[3].y;

    // vsout.normal   = texture(IDK_SSBO_Terrain.nmap, vsout.texcoord).rgb;
    vsout.tangent  = vsin_tangent;

    gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

}
