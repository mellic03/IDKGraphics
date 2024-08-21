#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/bindings.glsl"
#include "../include/noise.glsl"
#include "../include/rotate.glsl"


layout (std430, binding = IDK_BINDING_SSBO_Grass) readonly buffer IDKGrass
{
    vec4 GrassPositions[4*2048];
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
    float layer;

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
    float w = 0.5;

    for (int i=0; i<4; i++)
    {
        result += a * sin(w*x + t);

        a *= 0.5;
        w *= 2.0;
    }

    return result;
}


uniform float un_time;

const vec3 color_base = vec3(0.35, 0.37, 0.27);
const vec3 color_tip  = vec3(0.76, 0.78, 0.64);



vec2 animate( vec3 pos )
{
    vec2 result = vec2(0.0);

    result += sin(pos.x + pos.z + un_time);

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


void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec3  pos   = GrassPositions[gl_InstanceID].xyz;
    float scale = GrassPositions[gl_InstanceID].w;

    mat4 R = mat4(1.0); // mat4(inverse(mat3(camera.V)));

    mat4 T = mat4(1.0);
         T[3] = vec4(pos, 1.0);

    vsout.fragpos = (T * vec4(scale * vsin_pos, 1.0)).xyz;

    vsout.fragpos.x += scale * vsin_pos.y * SOS(un_time, vsout.fragpos.x);
    vsout.fragpos.z += scale * vsin_pos.y * SOS(un_time, vsout.fragpos.z);

    // vsout.fragpos.xz += scale * vsin_pos.y * animate(vsout.fragpos);

    vsout.texcoord = vsin_texcoords;
    vsout.normal   = vec3(0.0, 1.0, 0.0); // vsin_normal;
    vsout.color    = mix(vec3(0.0), vec3(1.0), vsin_pos.y);

    float noise = IDK_PerlinNoise(pos.xz / 6.0, 0).r;
    vsout.layer = 5.0 * noise;

    gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

}