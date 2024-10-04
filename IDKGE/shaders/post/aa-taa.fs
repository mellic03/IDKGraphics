#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/taa.glsl"

out vec4 fsout_frag_color;
in  vec2 fsin_texcoords;


uniform float un_factor = 16.0;

uniform sampler2D un_curr_color;
uniform sampler2D un_prev_color;
uniform sampler2D un_prev_depth;
uniform sampler2D un_curr_depth;
uniform sampler2D un_velocity;
uniform sampler2D un_prev_velocity;



float Mitchell( float x )
{
    x = abs(x);
    float x2 = x*x;
    float x3 = x*x*x;
    
    const float B = 1.0 / 3.0;
    const float C = 1.0 / 3.0;

    if (x < 1.0)
    {
        return (12.0 - 9.0*B - 6.0*C)*x3 - (-18.0 + 12.0*B + 6.0*C)*x2;
    }

    else if (1.0 <= x && x < 2.0 )
    {
        return (-B - 6.0*C)*x3 + (6.0*B + 30.0*C)*x2 + (-12.0*B - 48.0*C)*x + (8.0*B + 24.0*C);
    }

    else
    {
        return 0.0;
    }
}


float blackman_harris( float x, float w )
{
    const float pi = 3.14159265358979;
    if (abs(x) > w)
        return 0.;
    return 0.35875 - 0.48829*cos(pi*x/w + pi) + 0.14128*cos(2.*pi*x/w) - 0.01168*cos(3.*pi*x/w + pi*3.);
}



float DepthToViewZ( sampler2D depthtex, vec2 uv, mat4 P )
{
    float z = textureLod(depthtex, uv, 0.0).r * 2.0 - 1.0;

    vec4 pos  = vec4(uv * 2.0 - 1.0, z, 1.0);
         pos  = inverse(P) * vec4(pos.xyz, 1.0);
         pos /= pos.w;
    
    return pos.z;
}



void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec2  isize = textureSize(un_curr_color, 0);
    vec2  tsize = 1.0 / isize;

    vec2 jitter   = camera.jitter.xy;

    vec4 vdata    = UnpackVelocity(textureLod(un_velocity, fsin_texcoords, 0.0));
    vec2 curr_vel = vdata.xy;
    vec2 curr_uv  = fsin_texcoords;
    vec2 prev_uv  = clamp(fsin_texcoords-curr_vel, vec2(0.0), vec2(1.0));

    vec3 curr_color = texture(un_curr_color, curr_uv).rgb;

    // Velocity based blur
    {
        float fps   = 1.0 / IDK_GetDeltaTime();
        float scale = 0.25 * (fps / 60.0);
        vec2  wvel  = scale*vdata.zw;

        // float vlength = length(wvel);
        float samples = 32.0; // clamp(vlength, 1, 64);

        vec2 rd  = wvel / samples;
        vec2 rp0 = curr_uv;
        vec2 rp1 = curr_uv;

        for (int i=0; i<int(samples); i++)
        {
            rp0 -= rd;
            rp1 += rd;

            curr_color += texture(un_curr_color, rp0).rgb;
            curr_color += texture(un_curr_color, rp1).rgb;
        }

        curr_color /= 2.0*samples + 1;
    }


    vec3  min_color  = min(vec3(+1000.0), curr_color);
    vec3  max_color  = max(vec3(-1000.0), curr_color);

    for (int i=-1; i<=1; i++)
    {
        for (int j=-1; j<=1; j++)
        {
            vec3 color = texture(un_curr_color, prev_uv + tsize*vec2(j, i)).rgb;
            min_color = min(min_color, color);
            max_color = max(max_color, color);
        }
    }

    vec3 prev_color = texture(un_prev_color, prev_uv).rgb;
         prev_color = clamp(prev_color, min_color, max_color);

    float alpha  = 1.0 / un_factor;
    vec3  result = mix(prev_color, curr_color, alpha);

    fsout_frag_color = vec4(result, 1.0);
}
