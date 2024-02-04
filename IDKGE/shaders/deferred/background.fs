#version 460 core
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_albedo;

#include "../UBOs/UBOs.glsl"

in vec4 fsin_fragpos;
uniform samplerCube un_skybox;

struct DirLight
{
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
};

layout (std140, binding = 5) uniform UBO_dirlights
{
    DirLight    un_dirlights[10];
    mat4        un_cascade_matrices[4];
};


float PHG (float g, float cosTheta)
{
    const float Inv4Pi = 0.07957747154594766788;
    
    float gSq = g * g;
    float denomPreMul = 1 + gSq - (2.0 * g * cosTheta);
    return (1 - gSq) * Inv4Pi * inversesqrt(denomPreMul * denomPreMul * denomPreMul);
}

float miePhase (float cosTheta)
{
    return mix (PHG (0.8, cosTheta), PHG (-0.5, cosTheta), 0.5);
}


void main()
{
    vec3 L = normalize(-un_dirlights[0].direction.xyz);
    vec3 V = normalize(fsin_fragpos.xyz - un_camera.position.xyz);

    float mie = miePhase(max(dot(V, L), 0.0));
    float strength = 20.0 * smoothstep(0.0, 10.0, mie);
    vec3 sun_color = strength * vec3(0.97, 0.87, 0.87);

    vec3 color = texture(un_skybox, fsin_fragpos.xyz - un_viewpos).rgb;
    fsout_albedo = vec4(color, 1.0);

    // float frag_dist = distance(un_camera.position.xz, fsin_fragpos.xz);
    // float fog = clamp(frag_dist / 200.0, 0.0, 1.0);
    //     //   fog *= 1.0 - clamp(fsin_fragpos.y / 7.0, 0.0, 1.0);

    // vec3 result = sun_color + mix(color, un_dirlights[0].diffuse.rgb, fog);


    // fsout_albedo = vec4(result, 1.0);
}
