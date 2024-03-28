#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/UBOs.glsl"
#include "../include/util.glsl"



layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_texture_0;


void main()
{
    vec2 texel_size = 2.0 / textureSize(un_texture_0, 0);
    vec2 texcoord   = fsin_texcoords;

    float x = texel_size.x;
    float y = texel_size.y;

    vec3 a = texture(un_texture_0, vec2(texcoord.x - 2*x, texcoord.y + 2*y)).rgb;
    vec3 b = texture(un_texture_0, vec2(texcoord.x,       texcoord.y + 2*y)).rgb;
    vec3 c = texture(un_texture_0, vec2(texcoord.x + 2*x, texcoord.y + 2*y)).rgb;

    vec3 d = texture(un_texture_0, vec2(texcoord.x - 2*x, texcoord.y)).rgb;
    vec3 e = texture(un_texture_0, vec2(texcoord.x,       texcoord.y)).rgb;
    vec3 f = texture(un_texture_0, vec2(texcoord.x + 2*x, texcoord.y)).rgb;

    vec3 g = texture(un_texture_0, vec2(texcoord.x - 2*x, texcoord.y - 2*y)).rgb;
    vec3 h = texture(un_texture_0, vec2(texcoord.x,       texcoord.y - 2*y)).rgb;
    vec3 i = texture(un_texture_0, vec2(texcoord.x + 2*x, texcoord.y - 2*y)).rgb;

    vec3 j = texture(un_texture_0, vec2(texcoord.x - x, texcoord.y + y)).rgb;
    vec3 k = texture(un_texture_0, vec2(texcoord.x + x, texcoord.y + y)).rgb;
    vec3 l = texture(un_texture_0, vec2(texcoord.x - x, texcoord.y - y)).rgb;
    vec3 m = texture(un_texture_0, vec2(texcoord.x + x, texcoord.y - y)).rgb;

    vec3 result  = 0.125 * e;
         result += 0.03125 * (a + c + g + i);
         result += 0.06250 * (b + d + f + h);
         result += 0.12500 * (j + k + l + m);


    fsout_frag_color = vec4(result, 1.0);

}
