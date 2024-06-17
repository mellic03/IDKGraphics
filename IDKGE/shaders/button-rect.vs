// #version 460 core

// layout (location = 0) in vec3 vsin_pos;
// layout (location = 1) in vec2 vsin_texcoord;

// out vec2 fsin_texcoord;

// uniform mat4 un_projection;
// uniform vec4 un_bounds;

// void main()
// {
//     fsin_texcoord = vsin_texcoord;

//     float xmin = un_bounds[0];
//     float xmax = un_bounds[0] + un_bounds[2];

//     float ymin = un_bounds[1];
//     float ymax = un_bounds[1] + un_bounds[3];

//     vec2 position  = vsin_pos.xy + vec2(1.0, 1.0);
//          position *= (un_bounds.zw / 2.0);
//          position += un_bounds.xy;


//     gl_Position = un_projection * vec4(position, 0.0, 1.0);
// }


#version 460 core

out vec2 fsin_texcoord;

void main()
{
    const vec2 position  = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    const vec2 texcoords = (position + 1) * 0.5;

    fsin_texcoord = texcoords;
    gl_Position = vec4(position, 0.5, 1.0);
}


