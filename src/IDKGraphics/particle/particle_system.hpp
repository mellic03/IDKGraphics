#pragma once

#include "particle.hpp"


namespace idk
{
    class RenderEngine;
    class ModelAllocator;
    class EngineAPI;
}


namespace idk::ParticleSystem
{
    struct ParticleDesc;
    struct EmitterDesc;
    struct Emitter;


    void init( idk::RenderEngine& );
    void render( idk::RenderEngine&, idk::ModelAllocator& );
    void update( idk::RenderEngine&, float dt, const glm::vec3 &view );

    int  createEmitter( const EmitterDesc&, const ParticleDesc& );

    void destroyEmitter( int id );
    Emitter &getEmitter( int id );

    void updateTransform( int id, const glm::vec3 &position, const glm::vec3 &velocity );

};



struct idk::ParticleSystem::EmitterDesc
{
    int       model_id  = -1;
    float     duration  = 8.0f;
};


struct idk::ParticleSystem::ParticleDesc
{
    uint32_t  count      = 32;

    glm::vec3 origin     = glm::vec3(0.0f);
    glm::vec3 origin_rng = glm::vec3(0.0f);

    glm::vec3 vel      = glm::vec3(0.0f);
    glm::vec3 vel_rng  = glm::vec3(0.0f);

    float scale_start  = 1.0f;
    float scale_end    = 1.0f;
    float scale_rng    = 0.0f;

    float duration     = 1.0f;
    float duration_rng = 0.0f;

    float spawn_time   = 0.0f;
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


public:
    ParticleDesc pdesc;
    EmitterDesc  edesc;

    Emitter( const ParticleDesc&, const EmitterDesc& );
    void update( float dt );
    // void update( float dt, const glm::vec3 &view, std::vector<glm::mat4>& );

    float getTimer() const { return m_timer; };
    bool  finished() const { return m_timer > edesc.duration; };

};



