#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./terrain.glsl"


layout (vertices = 3) out;

uniform vec3 gEyeWorldPos;


in TESC_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    mat3 TBN;

} tsin[];


out TESE_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float tesslevel;
    mat3 TBN;

} tsout[];


const float TESS_LEVELS[6] = float[6](24.0, 16.0, 12.0, 6.0, 3.0, 2.0);


float GetTessLevel( float dist )
{
    const float max_dist = 32.0;
    const float max_tess = 64.0;

    float alpha = clamp(dist / max_dist, 0.0, 1.0);
    uint  level = clamp(uint(5.0*alpha), 0, 5);

    float xscale = (mat3(IDK_SSBO_Terrain.transform) * vec3(1.0)).x;

    return TESS_LEVELS[level];
}


void main()
{
    // Set the control points of the output patch
    tsout[gl_InvocationID].texcoord = tsin[gl_InvocationID].texcoord;
    tsout[gl_InvocationID].normal   = tsin[gl_InvocationID].normal;
    tsout[gl_InvocationID].tangent  = tsin[gl_InvocationID].tangent;
    tsout[gl_InvocationID].fragpos  = tsin[gl_InvocationID].fragpos;
    tsout[gl_InvocationID].TBN      = tsin[gl_InvocationID].TBN;

    IDK_Camera camera = IDK_UBO_cameras[0];
    vec3 viewpos = camera.position.xyz;

    vec3 pos0 = tsout[0].fragpos;
    vec3 pos1 = tsout[1].fragpos;
    vec3 pos2 = tsout[2].fragpos;

    // Calculate the distance from the camera to the three control points
    float dist0 = distance(viewpos.xz, pos0.xz);
    float dist1 = distance(viewpos.xz, pos1.xz);
    float dist2 = distance(viewpos.xz, pos2.xz);

    float min_dist = min(dist0, min(dist1, dist2));

    // Calculate the tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(min_dist);
    gl_TessLevelOuter[1] = GetTessLevel(min_dist);
    gl_TessLevelOuter[2] = GetTessLevel(min_dist);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];

    tsout[gl_InvocationID].tesslevel = gl_TessLevelInner[0];

}
