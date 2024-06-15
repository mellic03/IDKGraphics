#pragma once

#include <libidk/idk_glm.hpp>
#include <vector>


namespace idk
{
    class  ParticleEmitter;
    struct ParticleDesc;
}



struct idk::ParticleDesc
{
    int model_id;
    int count = 32;

    glm::vec3 velocity;
    glm::vec3 velocity_bias;
    glm::vec3 velocity_randomness;

    float scale;
    float scale_randomness;

    float duration;
    float duration_randomness;

};



struct idk::ParticleEmitter
{
private:
    struct Particle
    {
        glm::vec3 pos;
        glm::vec3 vel;
        float     scale;
        float     timer;
    };


    void _reset( Particle& );
    void _update( float dt, Particle& );

public:
    ParticleDesc m_desc;

    int       model_id;

    glm::vec3 origin;
    glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 velocity  = glm::vec3(0.0f);
    float scale = 1.0f;

    std::vector<Particle> m_particles;


    ParticleEmitter( const ParticleDesc &desc, const glm::vec3 &position,
                     const glm::vec3 &vel = glm::vec3(0.0f),
                     const glm::vec3 &dir = glm::vec3(0.0f, 1.0f, 0.0f) );

    void update( float dt );

    glm::mat4 getTransform( int i, const glm::vec3 &view );

};