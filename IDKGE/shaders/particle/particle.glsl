#include "../include/bindings.glsl"

#define EMITTER_MAX_PARTICLES 64
#define MAX_EMITTERS 128


struct IDK_ParticleDesc
{
    ivec4 count;

    vec4 origin;
    vec4 origin_rng;

    vec4 velocity;
    vec4 velocity_rng;

    vec4 scale;
    vec4 scale_factor;
    vec4 scale_rng;

    vec4 timer;
    vec4 duration;
    vec4 duration_rng;
};



struct IDK_Particle
{
    vec4 timer;
    vec4 scale;
    vec4 pos;
    vec4 vel;
    vec4 rot;
    vec4 color;
};


layout (std430, binding = IDK_BINDING_SSBO_ParticleDesc) readonly buffer InputParticleDesc
{
    IDK_ParticleDesc IDK_SSBO_ParticleDescriptors[MAX_EMITTERS];
};


layout (std430, binding = IDK_BINDING_SSBO_Particles) buffer InputParticles
{
    IDK_Particle Particles[EMITTER_MAX_PARTICLES * MAX_EMITTERS];
};

