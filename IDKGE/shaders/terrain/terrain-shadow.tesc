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

} tsin[];


out TESE_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;

} tsout[];


const float TESS_LEVELS[6] = float[6](32.0, 24.0, 16.0, 12.0, 5.0, 2.0);


float GetTessLevel( float Distance0, float Distance1 )
{
    const float max_dist = 32.0;
    const float max_tess = 64.0;

    float avg_dist = (Distance0 + Distance1) / 2.0;

    float alpha = clamp(avg_dist / max_dist, 0.0, 1.0);
    uint  level = clamp(uint(5.0*alpha), 0, 5);

    float xscale = (mat3(IDK_SSBO_Terrain.transform) * vec3(1.0)).x;

    return TESS_LEVELS[level];
}


void main()
{
    // Set the control points of the output patch
    tsout[gl_InvocationID].texcoord = tsin[gl_InvocationID].texcoord;
    tsout[gl_InvocationID].normal   = tsin[gl_InvocationID].normal;
    tsout[gl_InvocationID].fragpos  = tsin[gl_InvocationID].fragpos;

    IDK_Camera camera = IDK_UBO_cameras[0];
    vec3 viewpos = camera.position.xyz;

    // Calculate the distance from the camera to the three control points
    float dist0 = distance(viewpos, tsout[0].fragpos);
    float dist1 = distance(viewpos, tsout[1].fragpos);
    float dist2 = distance(viewpos, tsout[2].fragpos);

    // Calculate the tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(dist1, dist2);
    gl_TessLevelOuter[1] = GetTessLevel(dist2, dist0);
    gl_TessLevelOuter[2] = GetTessLevel(dist0, dist1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}
