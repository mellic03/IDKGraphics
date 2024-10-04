#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./particle.glsl"

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;

out vec3 fsin_fragpos;
out vec3 fsin_normal;
out vec2 fsin_texcoord;
flat out uint idx;
flat out uint drawID;
uniform uint un_offset;



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


void main()
{
    drawID = gl_DrawID + un_offset;
    idx = un_offset + gl_InstanceID;

    IDK_Camera camera = IDK_UBO_cameras[0];

    mat4 model    = inverse(camera.V);
         model[3] = vec4(Particles[idx].pos.xyz, 1.0);

    mat4 T = mat4(1.0);
         T[3] = vec4(Particles[idx].pos.xyz, 1.0);

    mat4 R = lookAt(vec3(0.0), normalize(camera.position.xyz - Particles[idx].pos.xyz), vec3(0.0, 1.0, 0.0));
         R = mat4(inverse(mat3(R)));

    model = T * R;
    vec3 pos = vsin_pos;

    vec3 position = vec3(model * vec4(pos * Particles[idx].sc[0], 1.0));
    vec3 normal   = vec3(model * vec4(vsin_normal,  0.0));

    fsin_fragpos   = position;
    fsin_normal    = normalize(normal);
    fsin_texcoord  = vsin_texcoords;

    gl_Position = camera.P * camera.V * vec4(position, 1.0);
}