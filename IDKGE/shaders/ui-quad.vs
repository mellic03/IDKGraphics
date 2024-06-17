#version 460 core

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec2 vsin_texcoord;


struct PanelQuad
{
    int x, y, w, h;
    vec4 color;
    vec4 radius;
};


layout (std140, binding = 2) uniform IDK_UBO_PanelQuad
{
    PanelQuad un_panel_quads[1024];
};


out vec2 fsin_texcoord;
flat out vec4  fsin_color;
flat out vec4  fsin_bounds;
flat out float fsin_radius;

uniform mat4 un_projection;


void main()
{
    fsin_texcoord = vsin_texcoord;

    PanelQuad desc = un_panel_quads[gl_DrawID + gl_InstanceID];

    fsin_color = desc.color;
    fsin_radius = desc.radius.x;
    fsin_bounds = vec4(desc.x, desc.y, desc.w, desc.h);

    vec2 position  = vsin_pos.xy + vec2(0.5);
         position *= vec2(desc.w, desc.h);
         position += vec2(desc.x, desc.y);

    gl_Position = un_projection * vec4(position, desc.radius.y, 1.0);
}

