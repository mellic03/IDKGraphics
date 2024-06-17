#version 460 core

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec2 vsin_texcoord;


struct TextQuad
{
    int   x;
    int   y;
    float z;
    int   glyph_idx;
};




layout (std140, binding = 1) uniform IDK_UBO_TextQuad
{
    TextQuad un_text_quads[1024];
};


out vec2 fsin_texcoord;

uniform mat4 un_projection;
uniform sampler2D un_atlas;

uniform int un_grid_w;
uniform int un_glyph_w;



vec2 atlas_uv( int row, int col, int glyph_w, int atlas_w, vec2 texcoord )
{
    ivec2 texel = ivec2(textureSize(un_atlas, 0) * texcoord);
          texel /= (atlas_w / glyph_w);
          texel += glyph_w * ivec2(row, col);
          texel.x -= glyph_w / 8;
          texel.y += glyph_w / 8;

    return vec2(texel) / textureSize(un_atlas, 0);
}



void main()
{
    TextQuad desc = un_text_quads[gl_DrawID + gl_InstanceID];

    vec2 src_size = textureSize(un_atlas, 0);

    int idx = desc.glyph_idx;
    int row = (idx / un_grid_w);
    int col = (idx % un_grid_w);

    int atlas_w = int(src_size.x);

    fsin_texcoord = atlas_uv(col, row, un_glyph_w, atlas_w, vsin_texcoord);

    vec2 position  = vsin_pos.xy + vec2(0.5);
         position *= un_glyph_w;
         position += vec2(desc.x, desc.y);

    gl_Position = un_projection * vec4(position, desc.z, 1.0);
}

