#ifndef IDK_TERRAIN
#define IDK_TERRAIN


#include "../include/bindings.glsl"
#include "../include/noise.glsl"
#include "../include/time.glsl"
#include "../generative/simplex.glsl"


#define IDK_TERRAIN_TEX_W 1024
#define IDK_TERRAIN_CLIP_W IDK_SSBO_Terrain.clipmap_size[0]
#define IDK_TERRAIN_NUM_CLIPS uint(IDK_SSBO_Terrain.clipmap_size[1])
#define IDK_TERRAIN_CLIP_W_VERTS 64


struct NoiseFactor
{
    float amp;
    float wav;
    float warp;
    float octaves;
};


struct SSBO_Terrain
{
    sampler2D      height;
    sampler2D      nmap;
    sampler2DArray diff;
    sampler2DArray norm;
    sampler2DArray arm;
    sampler2DArray disp;

    vec4 clipmap_size;

    vec4 texscale[1];

    vec4 origin;
    vec4 clamp_bounds;

    vec4 water_color[4];
    vec4 water_pos;
    vec4 water_scale;
    NoiseFactor water;

    NoiseFactor perlin;
    NoiseFactor voronoi;
    NoiseFactor vein;
    NoiseFactor exponent;
    NoiseFactor domainwarp;

    mat4 transform;
    vec4 scale;
    vec4 slope_blend;
    vec4 height_blend;

};


layout (std430, binding = IDK_BINDING_SSBO_Terrain) readonly buffer IDKTerrain
{
    SSBO_Terrain IDK_SSBO_Terrain;
};



vec2 TerrainDomainWarp( float w, vec2 texcoord )
{
    vec2 xmagic = vec2(12.34, 34.56);
    vec2 zmagic = vec2(56.78, 78.90);

    float dx = (IDK_PerlinNoise(w * (texcoord + xmagic), 1).r * 2.0 - 1.0);
    float dz = (IDK_PerlinNoise(w * (texcoord + zmagic), 0).r * 2.0 - 1.0);

    return w * vec2(dx, dz);
}




#define TerrainFractalNoise( _result, _texcoord, _NF, _texture ) \
{ \
    float _a = 1.0; \
    float _w = 1.0; \
    \
    for (int i=0; i<int(_NF.octaves); i++) \
    { \
        float _noise  = textureLod(_texture, vec3(_w * _texcoord, i%8), 0.0).r; \
        \
        _result[0] += _a * _noise; \
        _result[1] += (_a); \
        \
        _a *= _NF.amp; \
        _w *= _NF.wav; \
    } \
    \
    _result[1] = max(_result[1], 0.0001); \
}




vec2 rotateUV(vec2 uv, float rotation)
{
    float mid = 0.5;
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}


float MakeRidgeNoise( float noise )
{
    return 2.0 * (0.5 - abs(0.5 - noise));
}


float TerrainFractalPerlin( vec2 texcoord )
{
    NoiseFactor NF = IDK_SSBO_Terrain.perlin;

    float height   = 0.0;
    float weight   = 0.0;
    vec2  gradient = vec2(0.0);

    float a = 1.0;
    float w = 1.0;
    float k = IDK_SSBO_Terrain.scale.z;

    for (int i=0; i<int(NF.octaves); i++)
    {
        float xnoise = IDK_PerlinNoiseOffset(w*(texcoord), i%8, ivec2(1, 0)).r;
        float znoise = IDK_PerlinNoiseOffset(w*(texcoord), i%8, ivec2(0, 1)).r;
        float noise  = IDK_PerlinNoise(w*texcoord, i%8).r;

        gradient += a * vec2(xnoise-noise, znoise-noise);

        height += (a * noise) / (1.0 + k*length(gradient));
        weight += a;

        a *= NF.amp;
        w *= NF.wav;
    }


    return height / (weight);
}




float squareGradient( vec2 texcoord )
{
    vec2 dxz = (texcoord - vec2(0.0));
         dxz = dxz*dxz;
    float d = sqrt(dxz.x + dxz.y);

    return 1.0 - d;

}


float terrace_height( float height, vec2 bounds )
{
    float lo = bounds[0];
    float hi = bounds[1];

    height = clamp(height, lo, hi);

    if (lo <= height && height <= hi)
    {
        float alpha = smoothstep(lo, hi, height);

        height = mix(height, 0.5*(lo+hi), alpha);
    }

    return height;
}



float smoothmin( float a, float b, float k )
{
    k *= log(2.0);
    float x = b-a;
    return a + x / (1.0 - exp2(x/k));
}


float smoothmax( float a, float b, float k )
{
    k *= log(2.0);
    float x = b-a;
    return b - x / (1.0 - exp2(x/k));
}


float smoothterrace( float h )
{
    float lo = floor(h / 0.1) * 0.1;
    float hi = ceil(h / 0.1) * 0.1;

    float a = smoothstep(lo, hi, h);

    return mix(lo, hi, a);
}





float fbm( vec2 x, int octaves )
{
	float v = 0.0;
	float a = 0.5;
	vec2 shift = vec2(100);
	// Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));

	for (int i = 0; i < octaves; ++i)
    {
		v += a * snoise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}


float TerrainComputeHeight( vec2 texcoord )
{
    float frequency = IDK_SSBO_Terrain.scale.x;
    float yscale    = IDK_SSBO_Terrain.scale.y;

    vec2 uv = (texcoord + IDK_SSBO_Terrain.origin.xy) / frequency - 0.5;

    // vec4 voronoi = vec4(0.0);
    // TerrainFractalNoise(voronoi, uv, IDK_SSBO_Terrain.voronoi, IDK_SSBO_VoronoiNoise);
    // voronoi[0] = MakeRidgeNoise(voronoi[0] / voronoi[1]);

    float height = TerrainFractalPerlin(uv);
    // float height = TerrainFractalPerlin(uv);
        //   height = MakeRidgeNoise(height);

    float island = squareGradient(texcoord-0.5);

    // height = smoothmax(height, IDK_SSBO_Terrain.clamp_bounds[0], 0.01);
    // height = smoothmin(height, IDK_SSBO_Terrain.clamp_bounds[1], 0.01);

    // height = smoothterrace(height);
    // height *= island*island*island*island*island;

    return height;
}









// vec3 TerrainComputeLocalPosition( uint drawID, uint instanceID, vec3 vert_pos, vec2 cam_xz, float yscale )
// {
//     float xscale = IDK_TERRAIN_CLIP_W;
//     float max_xscale = xscale;
//     float ratio = pow(2, float(IDK_TERRAIN_NUM_CLIPS-1)) / xscale;

//     if (drawID == 1)
//     {
//         xscale = pow(2, float(instanceID)) / ratio;
//     }

//     else
//     {
//         xscale = 1.0 / ratio;
//     }

//     float spacing = xscale / IDK_TERRAIN_CLIP_W[drawID];
//     vec2  min_xz  = max_xscale * vec2(-0.5);
//     vec2  max_xz  = max_xscale * vec2(+0.5);
//     vec2  pos_xz  = xscale * vert_pos.xz + round(cam_xz / spacing) * spacing;

//     vec2 texcoord = (pos_xz - min_xz) / (max_xz - min_xz);
//          texcoord = clamp(texcoord, 0.0, 1.0);

//     return vec3(texcoord.x, 0.0, texcoord.y);
// }



vec3 TerrainSampleHeight( vec2 texcoord )
{
    texcoord += 0.5 / IDK_TERRAIN_TEX_W;

    float height  = textureLod(IDK_SSBO_Terrain.height, texcoord, 0.0).r;
          height *= IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.scale.y;

    return vec3(0.0, height, 0.0);
}


vec3 TerrainComputeWorldOffset( uint drawID, vec2 cam_xz, float xscale )
{
    mat4 model = IDK_SSBO_Terrain.transform;

    return vec3(0.0, model[3].y, 0.0);
}



#endif