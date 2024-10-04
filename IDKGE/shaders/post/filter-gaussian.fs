#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_input;
uniform int un_horizontal = 1;
uniform int un_width = 3;


void main()
{
    vec2 tsize  = textureSize(un_input, 0);

    const float[5] weights = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    const vec2[2]  offsets = vec2[2](vec2(1.0, 0.0), vec2(0.0, 1.0));
    const vec2     offset  = offsets[un_horizontal] / tsize;


    vec4 result = vec4(0.0);

    // result += textureLod(un_input, fsin_texcoords, 0.0);
    result += weights[0]*textureLod(un_input, fsin_texcoords, 0.0);

    for (int i=1; i<5; i++)
    {
        vec2 uv_l = fsin_texcoords - float(i)*offset;
        vec2 uv_r = fsin_texcoords + float(i)*offset;

        result += weights[i] * textureLod(un_input, uv_l, 0.0);
        result += weights[i] * textureLod(un_input, uv_r, 0.0);
    }


    // #define HW 2
    // for (int i=-HW; i<=+HW; i++)
    // {
    //     vec2 uv = fsin_texcoords + float(i)*offset;
    //     result += textureLod(un_input, uv, 0.0);
    // }
    // result /= 2.0*HW + 1.0;


    fsout_frag_color = result;
}

