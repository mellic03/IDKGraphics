#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"


layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;
uniform sampler2D un_bloom;


vec3 filmic(vec3 x, float gamma)
{
    vec3 X = max(vec3(0.0), x - 0.004);
    vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, vec3(1.0 / gamma));
}


vec3 aces(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}




void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();

    float exposure = camera.exposure;
    float gamma    = 2.2;

    vec3 hdr   = textureLod(un_input, fsin_texcoords, 0.0).rgb;
    vec3 blm   = texture(un_bloom, fsin_texcoords).rgb;

    hdr += camera.bloom * blm;

    vec3 sdr   = aces(exposure * hdr);
    vec3 color = pow(sdr, vec3(1.0 / gamma));
    // vec3 color = pow(sdr, vec3(1.0 / un_camera.beg.y));

    // vec3 color = filmic(exposure*hdr, un_camera.beg.y);
    // vec3 color = pow(hdr / (hdr + 1.0), vec3(1.0 / 2.2));


    fsout_frag_color = vec4(color, 1.0);
}
