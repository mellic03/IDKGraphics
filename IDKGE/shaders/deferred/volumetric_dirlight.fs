#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;
uniform sampler2D un_texture_3;

struct DirLight
{
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
};

layout (std140, binding = 5) uniform UBO_dirlights
{
    DirLight    un_dirlights[10];
    mat4        un_cascade_matrices[4];
};


struct Camera
{
    vec4 position;
    vec4 beg;
    vec4 aberration_rg;
    vec4 aberration_b;
    vec4 exposure;
};

layout (std140, binding = 2) uniform UBO_camera_data
{
    mat4 un_view;
    mat4 un_projection;
    vec3 un_viewpos;
    Camera un_camera;
};


float PHG (float g, float cosTheta)
{
    const float Inv4Pi = 0.07957747154594766788;
    
    float gSq = g * g;
    float denomPreMul = 1 + gSq - (2.0 * g * cosTheta);
    return (1 - gSq) * Inv4Pi * inversesqrt(denomPreMul * denomPreMul * denomPreMul);
}


float miePhase (float cosTheta)
{
    return mix (PHG (0.8, cosTheta), PHG (-0.5, cosTheta), 0.5);
}


uniform sampler2DArrayShadow un_dirlight_depthmap;
uniform vec4 un_cascade_depths;

#define KERNEL_HW 2
#define BLEND_DIST 1.0
#define DIRLIGHT_BIAS 0.0

float sampleDepthMap( int layer, vec3 uv, float bias )
{
    vec2 texelSize = 0.5 / textureSize(un_dirlight_depthmap, 0).xy;

    float shadow = 0.0;

    for(int x = -KERNEL_HW; x <= KERNEL_HW; ++x)
    {
        for(int y = -KERNEL_HW; y <= KERNEL_HW; ++y)
        {
            vec2 sample_uv    = uv.xy + vec2(x, y) * texelSize;
            vec4 sample_coord = vec4(sample_uv, float(layer), uv.z - bias);

            shadow += texture(un_dirlight_depthmap, sample_coord); 
        }
    }

    return shadow / ((2*KERNEL_HW+1)*(2*KERNEL_HW+1));
}


float dirlight_shadow( int idx, vec3 position )
{
    vec3 L = normalize(-un_dirlights[idx].direction.xyz);

    vec3  fragpos_viewspace = (un_view * vec4(position, 1.0)).xyz;
    float frag_depth        = abs(fragpos_viewspace.z);

    vec4 res   = step(un_cascade_depths, vec4(frag_depth));
    int  layer = int(res.x + res.y + res.z + res.w);

    vec4 fragpos_lightspace = un_cascade_matrices[layer] * vec4(position, 1.0);
    vec3 projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // float bias = DIRLIGHT_BIAS * max(dot(N, L), 0.0);
    float bias = DIRLIGHT_BIAS;
    float shadow = sampleDepthMap(layer, projCoords, bias);

    return shadow;
}




#define MAX_STEPS 32
#define INTENSITY 0.01


void main()
{
    vec3  frag_pos   = texture(un_texture_1, fsin_texcoords).xyz;
    float frag_dist  = distance(un_viewpos, frag_pos) - 0.0001;

    vec3  ray_pos    = un_viewpos;
    vec3  ray_dir    = normalize(frag_pos - un_viewpos);
    float ray_length = 0.0;

    vec3  diffuse = un_dirlights[0].diffuse.xyz;
    vec3  accum   = vec3(0.0);

    const float step_size = frag_dist / MAX_STEPS;

    for (int i=0; i<MAX_STEPS; i++)
    {
        accum += diffuse * dirlight_shadow(0, ray_pos);

        ray_pos += step_size * ray_dir;
        ray_length += step_size;
    }

    vec3 dirlight_dir = normalize(un_dirlights[0].direction.xyz);
    float mie = miePhase(-dot(ray_dir, dirlight_dir));
    accum *= mie * INTENSITY * step_size;

    fsout_frag_color = vec4(accum, 1.0);
}

