#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/bindings.glsl"
#include "../include/noise.glsl"
#include "../include/rotate.glsl"
#include "../include/time.glsl"
#include "../include/taa.glsl"
#include "terrain.glsl"


layout (std430, binding = IDK_BINDING_SSBO_Grass) readonly buffer IDKGrass
{
    vec2 GrassPositions[128*4096];
};


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out VS_out
{
    vec3 fragpos;
    vec2 texcoord;
    vec3 normal;
    vec3 color;
    float ao;
    float layer;

    IDK_VelocityData vdata;

} vsout;


mat4 lookAt( vec3 eye, vec3 center, vec3 up )
{
    vec3 zaxis = normalize(center - eye);
    vec3 xaxis = normalize(cross(up, zaxis));
    vec3 yaxis = cross(zaxis, xaxis);

    mat4 V = mat4(1.0);

    V[0][0] = xaxis.x;
    V[0][1] = yaxis.x;
    V[0][2] = zaxis.x;

    V[1][0] = xaxis.y;
    V[1][1] = yaxis.y;
    V[1][2] = zaxis.y;

    V[2][0] = xaxis.z;
    V[2][1] = yaxis.z;
    V[2][2] = zaxis.z;

    V[3][0] = dot(xaxis, -eye);
    V[3][1] = dot(yaxis, -eye);
    V[3][2] = dot(zaxis, -eye);

    V[3][3] = 1.0;

    return V;
}


float SOS( float t, float x )
{
    float result = 0.0;

    float a = 1.0;
    float w = 0.25;

    for (int i=0; i<4; i++)
    {
        result += a * sin(w*x + t);

        a *= 0.5;
        w *= 2.0;
    }

    return result;
}


const vec3 color_base = vec3(0.35, 0.37, 0.27);
const vec3 color_tip  = vec3(0.76, 0.78, 0.64);



vec2 animate( vec3 pos )
{
    vec2 result = vec2(0.0);

    float t = IDK_GetTime();
    result += sin(pos.x + pos.z + t);

    float a = 1.0;
    float w = 8.0;

    for (int i=0; i<4; i++)
    {
        result.x += a * IDK_PerlinNoise(pos.xz/w, 0).r;
        result.y += a * IDK_PerlinNoise(pos.xz/w, 1).r;
    
        a *= 0.5;
        w *= 2.0;
    }

    return result;
}




struct Material
{
    vec4 diff;
    vec3 norm;
    vec3 arm;
};


Material packMaterial( uint idx, vec2 uv, vec3 N, float scale )
{
    Material mat;

    mat.diff = texture(IDK_SSBO_Terrain.diff, vec3(scale*uv, idx));
    mat.norm = texture(IDK_SSBO_Terrain.norm, vec3(scale*uv, idx)).rgb * 2.0 - 1.0;
    mat.arm  = texture(IDK_SSBO_Terrain.arm,  vec3(scale*uv, idx)).rgb;

    return mat;
}


Material mixMaterial( Material A, Material B, float alpha )
{
    Material AB;

    AB.diff = mix(A.diff, B.diff, alpha);
    AB.norm = mix(A.norm, B.norm, alpha);
    AB.arm  = mix(A.arm,  B.arm,  alpha);

    return AB;
}


Material Bruh( int idx, vec2 uv, vec3 N )
{
    float scale     = 2.0;
    float scale4[4] = float[4]( 0.25, 0.1, 0.05, 0.01 );
    float theta4[4] = float[4]( 0.25, 0.5, 0.75, 1.01 );

    return packMaterial(idx, uv, N, 0.5);
}



vec3 TerrainComputeColor( vec2 uv )
{
    vec3 result = vec3(0.0);

    vec3 N = textureLod(IDK_SSBO_Terrain.nmap, uv, 0.0).rgb;

    Material grass = Bruh(0, uv, N);
    Material rock2 = Bruh(2, uv, N);

    {
        float lo = IDK_SSBO_Terrain.slope_blend[0];
        float hi = IDK_SSBO_Terrain.slope_blend[1];

        float alpha = smoothstep(lo, hi, 1.0 - abs(N.y));

        result = mixMaterial(grass, rock2, alpha).diff.rgb;
    }

    return result;
}





void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec2  pos_sc = GrassPositions[gl_InstanceID].xy;
    vec3  pos    = vec3(pos_sc.x, 0.0, pos_sc.y);
    vec2  uv     = TerrainWorldToUV(pos.x, pos.z);
    vec3  color  = TerrainComputeColor(uv);

    vec2  hs = textureLod(IDK_SSBO_Terrain.height, uv, 0.0).rg;
    vec3  N  = textureLod(IDK_SSBO_Terrain.nmap, uv, 0.0).rgb;
    float h  = (IDK_PerlinNoise(pos.xz / 512.0, 0).r + 1.0) * 0.75;

    pos.y = hs.r * IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.scale.y + IDK_SSBO_Terrain.transform[3].y;

    float scale  = 0.75 + 0.5;
    vec3  scale3 = vec3(scale * 1.0, h, scale * 1.0);

    
    vec2 dir = normalize(camera.position.xz - pos.xz);
         dir = vec2(dir.y, -dir.x);

    dir.x = float(int((16.0*(pos.x+64.0))) % 16) / 16.0 - 0.574159;
    dir.y = float(int((16.0*(pos.z+64.0))) % 16) / 16.0 - 0.574159;

    float theta = atan(dir.y, dir.x);
    float ct = cos(theta);
    float st = sin(theta);

    mat4 R = mat4(1.0);
         R[0] = vec4( ct, 0.0, st, 0.0);
         R[2] = vec4(-st, 0.0, ct, 0.0);

    // mat4 R = lookAt(pos, camera.position.xyz, vec3(0.0, 1.0, 0.0));
    //      R = mat4(inverse(mat3(R)));

    // mat4 R = mat4(inverse(mat3(camera.V)));

    mat4 T = mat4(1.0) * R;
         T[3] = vec4(pos, 1.0);

    vsout.fragpos = (scale3 * vsin_pos); // - vec3(0.0, 0.5, 0.0);
    vsout.fragpos = (T * vec4(vsout.fragpos, 1.0)).xyz;


    {
        float t1 = IDK_GetTime();
        float t0 = t1 - IDK_GetDeltaTime();

        vec3 prev = vsout.fragpos;
        vec3 curr = vsout.fragpos;

        prev.x += 0.35 * 1.0 * vsin_pos.y * SOS(t0, vsout.fragpos.x);
        prev.z += 0.35 * 1.0 * vsin_pos.y * SOS(t0, vsout.fragpos.z);

        curr.x += 0.35 * 1.0 * vsin_pos.y * SOS(t1, vsout.fragpos.x);
        curr.z += 0.35 * 1.0 * vsin_pos.y * SOS(t1, vsout.fragpos.z);

        vsout.vdata = PackVData(camera, curr, prev);
        vsout.fragpos = curr;
    }

    vsout.texcoord = vsin_texcoords;
    vsout.normal   = normalize(N);
    vsout.ao       = mix(0.0, 1.0, vsin_pos.y);
    vsout.color    = color;

    float noise = hs.g - 0.5;
    vsout.layer = 5.0 * clamp(noise, 0.0, 1.0);

    gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

}