#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;



vec4 sample_vol( vec2 uv )
{
    vec4 result = textureLod(un_input, uv, 0.0);
        //  result.a = 1.0 - result.a;
        //  result.a = result.a; // * result.a * result.a * result.a;
        //  result.a = 1.0 - result.a;

    return result;
}


void main()
{
    const int KERNEL_HW = 1;

    vec2 tsize  = 0.5 / textureSize(un_input, 0).xy;
    vec4 result = vec4(0.0);
         result = sample_vol(fsin_texcoords);
        //  result.a = 1.0 - result.a;
        //  result.a = result.a * result.a * result.a * result.a;
        //  result.a = 1.0 - result.a;

    // for (int x = -KERNEL_HW; x <= +KERNEL_HW; x++)
    // {
    //     for(int y = -KERNEL_HW; y <= +KERNEL_HW; y++)
    //     {
    //         vec2 texcoord = fsin_texcoords + tsize*vec2(x, y);
    //         result += sample_vol(texcoord);
    //     }    
    // }

    // result /= ((KERNEL_HW*2+1)*(KERNEL_HW*2+1));

    fsout_frag_color = result;
}