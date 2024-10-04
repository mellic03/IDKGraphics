#include "../include/bindings.glsl"

#ifndef IDK_PARTICLE
#define IDK_PARTICLE

#define EMITTER_MAX_PARTICLES 512
#define MAX_EMITTERS 128


struct IDK_ParticleDesc
{
    uvec4 count;

    vec4 pos, posRNG;
    vec4 vel, velRNG;
    vec4 scA, scB, scRNG;
    vec4 dur, durRNG;
    vec4 stime;
};

struct IDK_Particle
{
    vec4 t;
    vec4 sc;
    vec4 pos;
    vec4 vel;
    vec4 col;
};


layout (std430, binding = IDK_BINDING_SSBO_ParticleDesc) readonly buffer InputParticleDesc
{
    IDK_ParticleDesc IDK_SSBO_ParticleDescriptors[MAX_EMITTERS];
};


layout (std430, binding = IDK_BINDING_SSBO_Particles) buffer InputParticles
{
    IDK_Particle Particles[EMITTER_MAX_PARTICLES * MAX_EMITTERS];
};



#endif