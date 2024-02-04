#version 460 core
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;


#include "../UBOs/UBOs.glsl"
#include "./vxgi.glsl"


uniform sampler3D un_voxeldata;

in vec4 fsin_fragpos;

#define MAX_STEPS 512
#define STEP_SIZE 0.05
#define INTENSITY 0.08

#define APERTURE 1.0
#define ZMIPLEVEL 0.0

void main()
{
    vec3 pos = un_viewpos;
    vec3 dir = normalize(fsin_fragpos.xyz - pos);

    vec3 down = VXGI_truncatePosition(pos);
    vec3 up   = down + VXGI_VOXEL_SIZE;

    vec3 vstep = sign(dir);
    vec3 tm = (up - pos) / dir;
    vec3 td = vstep * (VXGI_VOXEL_SIZE / dir);

    float xstep = vstep.x; // sign(dir.x);
    float ystep = vstep.y; // sign(dir.y);
    float zstep = vstep.z; // sign(dir.z);


    float tmx = tm.x;
    float tmy = tm.y;
    float tmz = tm.z;

    float tdx = td.x;
    float tdy = td.y;
    float tdz = td.z;


    vec4  accum = vec4(0.0);
    // accum = VXGI_TraceCone(pos, dir, radians(APERTURE), un_viewpos, un_voxeldata).rgb;


    for (int i=0; i<MAX_STEPS; i++)
    {
        vec3 texcoord = VXGI_WorldToTexCoord(pos, un_viewpos);
        accum += textureLod(un_voxeldata, texcoord, ZMIPLEVEL) * pow(2, ZMIPLEVEL);

        if (accum.a > 0.0)
        {
            // accum.rgb = accum.rgb * 0.5 + 0.5;
            break;
        }

        pos += STEP_SIZE * dir;

    }


    // vec3 texel = VXGI_WorldToTexel(pos, un_viewpos);

    // for (int i=0; i<MAX_STEPS; i++)
    // {
    //     accum += textureLod(un_voxeldata, texel / VXGI_TEXTURE_SIZE, ZMIPLEVEL);

    //     if (accum.a > 0.0)
    //     {
    //         fsout_frag_color = vec4(accum.rgb, 1.0);
    //         return;
    //     }

    //     if (tmx < tmy)
    //     {
    //         if (tmx < tmz)
    //         {
    //             tmx += tdx;
    //             texel.x += xstep;
    //         }

    //         else
    //         {
    //             tmz += tdz;
    //             texel.z += zstep;
    //         }
    //     }

    //     else
    //     {
    //         if (tmy < tmz)
    //         {
    //             tmy += tdy;
    //             texel.y += ystep;
    //         }

    //         else
    //         {
    //             tmz += tdz;
    //             texel.z += zstep;
    //         }
    //     }

    // }

    fsout_frag_color = vec4(accum.rgb, 1.0);
}


