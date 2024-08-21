#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;


void main()
{
    const int KERNEL_HW = 4;

    vec2 tsize  = 0.5 / textureSize(un_input, 0).xy;
    vec4 result = vec4(0.0);

    for(int x = -KERNEL_HW; x <= +KERNEL_HW; x++)
    {
        for(int y = -KERNEL_HW; y <= +KERNEL_HW; y++)
        {
            vec2 texcoord = fsin_texcoords + tsize*vec2(x, y);

            result += texture(un_input, texcoord);
        }    
    }

    result /= ((KERNEL_HW*2+1)*(KERNEL_HW*2+1));


    fsout_frag_color = result;
}