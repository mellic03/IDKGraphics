// #version 460 core
// #extension GL_GOOGLE_include_directive: require

// #include "./include/storage.glsl"

// out vec4 fsout_frag_color;
// in vec2 fsin_texcoord;

// uniform sampler2D un_text;
// uniform vec4      un_color;
// uniform vec4      un_bounds;


// void main()
// {
//     vec2 texel = fsin_texcoord * textureSize(un_text, 0);

//     float radius = 4.0;
//     vec2 ree = clamp(texel, vec2(radius), un_bounds.zw-radius);

//     if (distance(texel, ree) > radius)
//     {
//         discard;
//     }


//     vec4 color = texture(un_text, fsin_texcoord);
//         //  color += un_color;

//     fsout_frag_color = color;
// }



#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "./include/storage.glsl"

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoord;
uniform sampler2D un_input;


void main()
{
    // vec2  dst_size = IDK_RenderData_GetCamera().vec2(camera.near, camera.far);
    // vec2  src_size = textureSize(un_input, 0);

    // ivec2 dst_texel    = ivec2(dst_size * fsin_texcoord);


    // vec2 dst_center = dst_size / 2.0;
    // vec2 delta      = (vec2(dst_texel) - dst_center) / src_size;

    // vec2 src_texcoord = vec2(0.5) + delta;
    //      src_texcoord.y = 1.0 - src_texcoord.y;

    // if (src_texcoord.x < 0.0 || src_texcoord.x > 1.0)
    // {
    //     discard;
    // }

    // if (src_texcoord.y < 0.0 || src_texcoord.y > 1.0)
    // {
    //     discard;
    // }

    vec4 src = texture(un_input, fsin_texcoord);
         src.a = clamp(src.a, 0.0, 1.0);

    // if (src.a < 0.8)
    // {
    //     discard;
    // }
        //  src += vec4(0.25);
    // vec3 dst = imageLoad(un_output, dst_texel).rgb;

    // vec3 result = (1.0 - un_alpha)*dst + (un_alpha)*src;

    // imageStore(un_output, dst_texel, vec4(result, 1.0));


    fsout_frag_color = src;
}



