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

#define IDK_WATER_OCTAVES 512


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


struct IDK_TerrainDesc
{
    sampler2D height;
    sampler2D nmap;
};

struct IDK_WaterDesc
{
    vec4 position;
    vec4 scale;
    vec4 noisefactor;
    vec4 directions[IDK_WATER_OCTAVES];
};



layout (std430, binding = IDK_BINDING_SSBO_Terrain) readonly buffer IDKTerrain
{
    SSBO_Terrain IDK_SSBO_Terrain;
    vec2 IDK_SSBO_Water_dirs[IDK_WATER_OCTAVES];
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
        float _noise = textureLod(_texture, vec3(_w * _texcoord, i%8), 0.0).r; \
        \
        _result[0] += _a * _noise; \
        _result[1] += _a; \
        \
        _a *= _NF.amp; \
        _w *= _NF.wav; \
    } \
    \
    _result[1] = max(_result[1], 0.0001); \
    _result[0] *= _result[1]; \
}



#define TerrainFractalNoise1( _result, _texcoord, _NF, _texture ) \
{ \
    float _a = 1.0; \
    float _w = 1.0; \
    float _weight = 1.0; \
    \
    for (int i=0; i<int(_NF.octaves); i++) \
    { \
        float _noise = (1.0 - textureLod(_texture, vec3(_w * _texcoord, i%8), 0.0).r); \
        \
        _result[0] += _a * _noise; \
        _result[1] += _a; \
        \
        _a *= _NF.amp; \
        _w *= _NF.wav; \
        _weight += _a; \
    } \
    \
    _result[0] /= _weight; \
}



#define TerrainFractalNoise2( _result, _texcoord, _NF, _texture ) \
{ \
    float _a = 1.0; \
    float _w = 1.0; \
    \
    for (int i=0; i<int(_NF.octaves); i++) \
    { \
        float _noise = (1.0 - textureLod(_texture, vec3(_w * _texcoord, i%8), 0.0).r); \
        \
        _result[0] += _a * _noise; \
        _result[1] += _a; \
        \
        _a *= _NF.amp; \
        _w *= _NF.wav; \
    } \
    \
    _result[1] = max(_result[1], 0.0001); \
    _result[0] *= _result[1]; \
}




vec2 rotateUV(vec2 uv, float rotation)
{
    float mid = 0.5;
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}



float squareGradient( vec2 texcoord )
{
    float d = clamp(distance(texcoord, vec2(0.5)), 0.0, 1.0);
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




float MakeRidgeNoise( float noise )
{
    return 2.0 * (0.5 - abs(0.5 - noise));
}


float sigmoid( float x )
{
    return 1.0 / (1.0 + exp(-x));
}


float TerrainFractalPerlin( vec2 texcoord )
{
    NoiseFactor NF = IDK_SSBO_Terrain.perlin;

    float height = 0.0;
    float weight = 0.0;

    float a = 1.0;
    float w = 1.0;

    for (int i=0; i<int(NF.octaves); i++)
    {
        float noise = IDK_PerlinNoise(w*texcoord, i%8).r * 0.5;

        height += a * noise;
        weight += a;

        a *= NF.amp;
        w *= NF.wav;
    }

    return height;
}


float TerrainFractalVoronoi( vec2 texcoord )
{
    NoiseFactor NF = IDK_SSBO_Terrain.voronoi;

    float height   = 0.0;
    float weight   = 0.0;
    vec2  gradient = vec2(0.0);

    float a = 1.0;
    float w = 1.0;
    float k = IDK_SSBO_Terrain.scale.z;

    for (int i=0; i<int(NF.octaves); i++)
    {
        // float xnoise = (1.0 - IDK_VoronoiNoiseOffset(w*texcoord, i%8, ivec2(1, 0)).r);
        // float znoise = (1.0 - IDK_VoronoiNoiseOffset(w*texcoord, i%8, ivec2(0, 1)).r);
        float noise  = (1.0 - IDK_VoronoiNoise(w*texcoord, i%8).r);

        // gradient += vec2(noise-xnoise, noise-znoise);
        // height += (a*noise) / (1.0 + k*length(gradient));

        height += a*noise;
        weight += a;

        a *= NF.amp;
        w *= NF.wav;
    }

    return height;
}



float islandGradient( vec2 uv )
{
    float d = clamp(distance(uv, vec2(0.5)), 0.0, 1.0);
          d = 1.0 - (d);
        //   d = d * exp(-d);

    return d - 0.5;
    // return clamp(d - 0.5, 0.0, 1.0);
    // return sigmoid(d) - 0.5;
}


float TerrainComputeHeight( vec2 texcoord )
{
    float frequency = IDK_SSBO_Terrain.scale.x;
    float yscale    = IDK_SSBO_Terrain.scale.y;

    vec2 uv = (texcoord + 0.5 + IDK_SSBO_Terrain.origin.xy) / frequency;

    float a = IDK_SSBO_Terrain.clamp_bounds[0];

    float P = 1.5 * TerrainFractalPerlin(uv);
          P = P*P;

    float V = 1.25 * TerrainFractalVoronoi(uv);
          V = V*V;
          V = smoothmax(V, 3.0, 0.75);

    float height = mix(P, V, a);
    // float height = smoothmax(P, V, 0.7);
        //   height = MakeRidgeNoise(height);
        //   height = smoothmax(P, V*V, 0.1);

    height += IDK_SSBO_Terrain.clamp_bounds[1];
    height *= islandGradient(texcoord);

    return height;
}


float TerrainComputeDetail( vec2 world_xz )
{
    vec4 result = vec4(0.0);

    NoiseFactor NF  = IDK_SSBO_Terrain.vein;
    NoiseFactor NF2 = IDK_SSBO_Terrain.vein;
    // TerrainFractalNoise1(result, 64.0*texcoord, NF, IDK_SSBO_VoronoiNoise);

    float h1 = (IDK_VoronoiNoise(0.1 * world_xz, 3).r);
    float h2 = 1.0 - pow(h1, 2.0);
          h2 = min(h2, 0.9);

    float height = h2 - 0.5*(1.0 - h1);

    return 1.0 * height;
}


float TerrainComputeGrassness( vec2 texcoord )
{
    vec4 result = vec4(0.0);

    NoiseFactor NF = { 0.5, 2.0, 0.0, 8.0 };
    TerrainFractalNoise(result, 0.1*texcoord, NF, IDK_SSBO_PerlinNoise);

    result[0] /= 4.0;


    float grass = clamp(result[0], 0.0, 1.5);
          grass = clamp(grass, 0.0, 1.5);

    return grass;
}



vec2 TerrainWorldToUV( float x, float z )
{
    float xscale = length(vec3(IDK_SSBO_Terrain.transform[0]));
    float yscale = IDK_SSBO_Terrain.scale.y;

    vec3 minv  = xscale * vec3(-0.5, 0.0, -0.5);
    vec3 maxv  = xscale * vec3(+0.5, 0.0, +0.5);

    float u = (x - minv.x) / (maxv.x - minv.x);
    float v = (z - minv.z) / (maxv.z - minv.z);

    return vec2(u, v);
}

vec2 TerrainWorldToUV( vec2 uv )
{
    return TerrainWorldToUV(uv.x, uv.y);
}


vec3 TerrainComputeWorldOffset( uint drawID, vec2 cam_xz, float xscale )
{
    mat4 model = IDK_SSBO_Terrain.transform;

    return vec3(0.0, model[3].y, 0.0);
}



#endif