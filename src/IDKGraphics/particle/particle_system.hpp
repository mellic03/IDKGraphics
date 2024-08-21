#pragma once

#include "particle.hpp"


namespace idk
{
    class RenderQueue;
    class ModelAllocator;
    class EngineAPI;
}


namespace idk::ParticleSystem
{
    struct ParticleDesc;
    struct EmitterDesc;
    struct Emitter;


    void init();
    void render( idk::ModelAllocator& );

    void update( float dt, const glm::vec3 &view, idk::RenderQueue& );

    // int  createEmitter( const glm::mat4&, const EmitterDesc&, const ParticleDesc& );
    // int  createEmitter( const glm::vec3&, const EmitterDesc&, const ParticleDesc& );
    int  createEmitter( const glm::vec3&, const glm::vec3&,
                        const EmitterDesc&, const ParticleDesc& );

    void destroyEmitter( int id );
    Emitter &getEmitter( int id );

};



struct idk::ParticleSystem::ParticleDesc
{
    glm::vec3 origin_rng;

    glm::vec3 velocity;
    glm::vec3 velocity_rng;

    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 scale_factor = glm::vec3(0.0f, 1.0f, 0.0f);
    float scale_rng;

    float duration;
    float duration_rng;
};


struct idk::ParticleSystem::EmitterDesc
{
    int   model_id  = -1;
    int   particles = 32;
    float duration  = -1.0f;
};


struct idk::ParticleSystem::Emitter
{
private:
    struct Particle
    {
        float timer;
        float scale;
        glm::vec3 pos;
        glm::vec3 vel;
    };

    float                 m_timer;
    std::vector<Particle> m_particles;

    void _reset_particle( Particle& );


public:
    glm::mat4    transform;
    glm::vec3    position;
    glm::vec3    direction;

    ParticleDesc pdesc;
    EmitterDesc  edesc;

    Emitter( const glm::vec3&, const glm::vec3&, const ParticleDesc&, const EmitterDesc& );
    void update( float dt );
    // void update( float dt, const glm::vec3 &view, std::vector<glm::mat4>& );

    float getTimer() const { return m_timer; };
    bool  finished() const { return m_timer > edesc.duration; };

};



