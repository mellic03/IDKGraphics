#version 460 core

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec2 vsin_texcoord;
layout (location = 2) in vec3 vsin_extents;
layout (location = 3) in vec4 vsin_color;


out vec2 fsin_texcoord;
flat out vec3 fsin_extents;
flat out vec4 fsin_color;

uniform mat4 un_projection;


// vec2 atlas_uv( int row, int col, int glyph_w, int atlas_w, vec2 texcoord )
// {
//     ivec2 texel = ivec2(textureSize(un_atlas, 0) * texcoord);
//           texel /= (atlas_w / glyph_w);
//           texel += glyph_w * ivec2(row, col);
//           texel.x -= glyph_w / 8;
//           texel.y += glyph_w / 8;

//     return vec2(texel) / textureSize(un_atlas, 0);
// }



void main()
{
    fsin_texcoord = vsin_texcoord;
    fsin_extents  = vsin_extents;
    fsin_color    = vsin_color;

    gl_Position = un_projection * vec4(vsin_pos, 1.0);
}

