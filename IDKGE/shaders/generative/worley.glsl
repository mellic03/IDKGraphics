
// contributors: ["Patricio Gonzalez Vivo", "David Hoskins", "Inigo Quilez"]
// description: Pass a value and get some random normalize value between 0 and 1
// use: float random[2|3](<float|vec2|vec3> value)
// options:
//     - RANDOM_HIGHER_RANGE: for working with a range over 0 and 1
//     - RANDOM_SINLESS: Use sin-less random, which tolerates bigger values before producing pattern. From https://www.shadertoy.com/view/4djSRW
//     - RANDOM_SCALE: by default this scale if for number with a big range. For producing good random between 0 and 1 use bigger range
// examples:
//     - /shaders/generative_random.frag
// license:
//     - MIT License (MIT) Copyright 2014, David Hoskins


#ifndef RANDOM_SCALE
#ifdef RANDOM_HIGHER_RANGE
#define RANDOM_SCALE vec4(.1031, .1030, .0973, .1099)
#else
#define RANDOM_SCALE vec4(443.897, 441.423, .0973, .1099)
#endif
#endif

#ifndef FNC_RANDOM
#define FNC_RANDOM
float random(in float x) {
#ifdef RANDOM_SINLESS
    x = fract(x * RANDOM_SCALE.x);
    x *= x + 33.33;
    x *= x + x;
    return fract(x);
#else
    return fract(sin(x) * 43758.5453);
#endif
}

float random(in vec2 st) {
#ifdef RANDOM_SINLESS
    vec3 p3  = fract(vec3(st.xyx) * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
#else
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453);
#endif
}

float random(in vec3 pos) {
#ifdef RANDOM_SINLESS
    pos  = fract(pos * RANDOM_SCALE.xyz);
    pos += dot(pos, pos.zyx + 31.32);
    return fract((pos.x + pos.y) * pos.z);
#else
    return fract(sin(dot(pos.xyz, vec3(70.9898, 78.233, 32.4355))) * 43758.5453123);
#endif
}

float random(in vec4 pos) {
#ifdef RANDOM_SINLESS
    pos = fract(pos * RANDOM_SCALE);
    pos += dot(pos, pos.wzxy + 33.33);
    return fract((pos.x + pos.y) * (pos.z + pos.w));
#else
    float dot_product = dot(pos, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
#endif
}

vec2 random2(float p) {
    vec3 p3 = fract(vec3(p) * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

vec2 random2(vec2 p) {
    vec3 p3 = fract(p.xyx * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

vec2 random2(vec3 p3) {
    p3 = fract(p3 * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

vec3 random3(float p) {
    vec3 p3 = fract(vec3(p) * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xxy + p3.yzz) * p3.zyx); 
}

vec3 random3(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * RANDOM_SCALE.xyz);
    p3 += dot(p3, p3.yxz + 19.19);
    return fract((p3.xxy + p3.yzz) * p3.zyx);
}

vec3 random3(vec3 p) {
    p = fract(p * RANDOM_SCALE.xyz);
    p += dot(p, p.yxz + 19.19);
    return fract((p.xxy + p.yzz) * p.zyx);
}

vec4 random4(float p) {
    vec4 p4 = fract(p * RANDOM_SCALE);
    p4 += dot(p4, p4.wzxy + 19.19);
    return fract((p4.xxyz + p4.yzzw) * p4.zywx);   
}

vec4 random4(vec2 p) {
    vec4 p4 = fract(p.xyxy * RANDOM_SCALE);
    p4 += dot(p4, p4.wzxy + 19.19);
    return fract((p4.xxyz + p4.yzzw) * p4.zywx);
}

vec4 random4(vec3 p) {
    vec4 p4 = fract(p.xyzx * RANDOM_SCALE);
    p4 += dot(p4, p4.wzxy + 19.19);
    return fract((p4.xxyz + p4.yzzw) * p4.zywx);
}

vec4 random4(vec4 p4) {
    p4 = fract(p4  * RANDOM_SCALE);
    p4 += dot(p4, p4.wzxy + 19.19);
    return fract((p4.xxyz + p4.yzzw) * p4.zywx);
}
#endif


// contributors: Shadi El Hajj
// description: Commonly used distance functions.
// options:
// - DIST_FNC: change the distance function, currently implemented are distEuclidean, distManhattan, distChebychev and distMinkowski
// - DIST_MINKOWSKI_P: the power of the Minkowski distance function (1.0 Manhattan, 2.0 Euclidean, Infinity Chebychev)
// license: MIT License (MIT) Copyright (c) 2024 Shadi EL Hajj


#ifndef DIST_FNC
#define DIST_FNC distEuclidean
#endif

#ifndef DIST_MINKOWSKI_P
#define DIST_MINKOWSKI_P 2.0 // 1: Manhattan, 2: Euclidean, Infinity: Chebychev
#endif

#ifndef FNC_DIST
#define FNC_DIST

float distEuclidean(vec2 a, vec2 b) { return distance(a, b); }
float distEuclidean(vec3 a, vec3 b) { return distance(a, b); }
float distEuclidean(vec4 a, vec4 b) { return distance(a, b); }

// https://en.wikipedia.org/wiki/Taxicab_geometry
float distManhattan(vec2 a, vec2 b) { return abs(a.x - b.x) + abs(a.y - b.y); }
float distManhattan(vec3 a, vec3 b) { return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z); }
float distManhattan(vec4 a, vec4 b) { return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z) + abs(a.w - b.w); }

// https://en.wikipedia.org/wiki/Chebyshev_distance
float distChebychev(vec2 a, vec2 b) { return max(abs(a.x - b.x), abs(a.y - b.y)); }
float distChebychev(vec3 a, vec3 b) { return max(abs(a.x - b.x), max(abs(a.y - b.y), abs(a.z - b.z))); }
float distChebychev(vec4 a, vec4 b) { return max(abs(a.x - b.x), max(abs(a.y - b.y), max(abs(a.z - b.z), abs(a.w - b.w) ))); }

// https://en.wikipedia.org/wiki/Minkowski_distance
float distMinkowski(vec2 a, vec2 b) { return  pow(pow(abs(a.x - b.x), DIST_MINKOWSKI_P) + pow(abs(a.y - b.y), DIST_MINKOWSKI_P), 1.0 / DIST_MINKOWSKI_P); }
float distMinkowski(vec3 a, vec3 b) { return  pow(pow(abs(a.x - b.x), DIST_MINKOWSKI_P) + pow(abs(a.y - b.y), DIST_MINKOWSKI_P) + pow(abs(a.z - b.z), DIST_MINKOWSKI_P), 1.0 / DIST_MINKOWSKI_P); }
float distMinkowski(vec4 a, vec4 b) { return  pow(pow(abs(a.x - b.x), DIST_MINKOWSKI_P) + pow(abs(a.y - b.y), DIST_MINKOWSKI_P) + pow(abs(a.z - b.z), DIST_MINKOWSKI_P) + pow(abs(a.w - b.w), DIST_MINKOWSKI_P), 1.0 / DIST_MINKOWSKI_P); }

float dist(vec2 a, vec2 b) { return DIST_FNC(a, b); }
float dist(vec3 a, vec3 b) { return DIST_FNC(a, b); }
float dist(vec4 a, vec4 b) { return DIST_FNC(a, b); }

#endif


// contributors: Patricio Gonzalez Vivo
// description: Worley noise. Returns vec2(F1, F2)
// use: <vec2> worley2(<vec2|vec3> pos)
// options:
//     - WORLEY_JITTER: amount of pattern randomness. With 1.0 being the default and 0.0 resulting in a perfectly symmetrical pattern.
//     - WORLEY_DIST_FNC: change the distance function, currently implemented are distEuclidean, distManhattan, distChebychev and distMinkowski
// examples:
//     - https://raw.githubusercontent.com/patriciogonzalezvivo/lygia_examples/main/generative_worley.frag
// license:
//     - Copyright (c) 2021 Patricio Gonzalez Vivo under Prosperity License - https://prosperitylicense.com/versions/3.0.0
//     - Copyright (c) 2021 Patricio Gonzalez Vivo under Patron License - https://lygia.xyz/license


#ifndef FNC_WORLEY
#define FNC_WORLEY

#ifndef WORLEY_JITTER
#define WORLEY_JITTER 1.0
#endif

#ifndef WORLEY_DIST_FNC
#define WORLEY_DIST_FNC distEuclidean
#endif

vec2 worley2(vec2 p){
    vec2 n = floor( p );
    vec2 f = fract( p );

    float distF1 = 1.0;
    float distF2 = 1.0;
    vec2 off1 = vec2(0.0); 
    vec2 pos1 = vec2(0.0);
    vec2 off2 = vec2(0.0);
    vec2 pos2 = vec2(0.0);
    for( int j= -1; j <= 1; j++ ) {
        for( int i=-1; i <= 1; i++ ) {	
            vec2  g = vec2(i,j);
            vec2  o = random2( n + g ) * WORLEY_JITTER;
            vec2  p = g + o;
            float d = WORLEY_DIST_FNC(p, f);
            if (d < distF1) {
                distF2 = distF1;
                distF1 = d;
                off2 = off1;
                off1 = g;
                pos2 = pos1;
                pos1 = p;
            }
            else if (d < distF2) {
                distF2 = d;
                off2 = g;
                pos2 = p;
            }
        }
    }

    return vec2(distF1, distF2);
}

float worley(vec2 p){ return 1.0-worley2(p).x; }

vec2 worley2(vec3 p) {
    vec3 n = floor( p );
    vec3 f = fract( p );

    float distF1 = 1.0;
    float distF2 = 1.0;
    vec3 off1 = vec3(0.0);
    vec3 pos1 = vec3(0.0);
    vec3 off2 = vec3(0.0);
    vec3 pos2 = vec3(0.0);
    for( int k = -1; k <= 1; k++ ) {
        for( int j= -1; j <= 1; j++ ) {
            for( int i=-1; i <= 1; i++ ) {	
                vec3  g = vec3(i,j,k);
                vec3  o = random3( n + g ) * WORLEY_JITTER;
                vec3  p = g + o;
                float d = WORLEY_DIST_FNC(p, f);
                if (d < distF1) {
                    distF2 = distF1;
                    distF1 = d;
                    off2 = off1;
                    off1 = g;
                    pos2 = pos1;
                    pos1 = p;
                }
                else if (d < distF2) {
                    distF2 = d;
                    off2 = g;
                    pos2 = p;
                }
            }
        }
    }

    return vec2(distF1, distF2);
}

float worley(vec3 p){ return 1.0-worley2(p).x; }

#endif