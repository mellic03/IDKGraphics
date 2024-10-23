#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "./include/storage.glsl"

out vec4 fsout_frag_color;
in vec2 fsin_texcoords;

uniform float     un_alpha;
uniform sampler2D un_input;

void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec2  dst_size = vec2(camera.width, camera.height);
    vec2  src_size = textureSize(un_input, 0);

    ivec2 dst_texel    = ivec2(fsin_texcoords * dst_size);

    vec2 dst_center = dst_size / 2.0;
    vec2 delta      = (vec2(dst_texel) - dst_center) / src_size;

    vec2 src_texcoord = vec2(0.5) + delta;
         src_texcoord.y = 1.0 - src_texcoord.y;


    vec3 src = texture(un_input, src_texcoord).rgb;
    fsout_frag_color = vec4(src, un_alpha);

    // vec3 dst = imageLoad(un_output, dst_texel).rgb;

    // vec3 result = (1.0 - un_alpha)*dst + (un_alpha)*src;

    // imageStore(un_output, dst_texel, vec4(result, 1.0));
}
