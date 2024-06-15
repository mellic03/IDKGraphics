#include "particle.hpp"
#include <libidk/idk_random.hpp>




idk::ParticleEmitter::ParticleEmitter( const idk::ParticleDesc &desc, const glm::vec3 &position,
                                       const glm::vec3 &vel, const glm::vec3 &dir )
:   m_desc    (desc),
    origin    (position),
    velocity  (vel),
    direction (dir)
{
    m_particles.resize(64);

    for (int i=0; i<64; i++)
    {
        _reset(m_particles[i]);
    }

}


void
idk::ParticleEmitter::_reset( Particle &p )
{
    p.pos.x = idk::randf(-1.0f, +1.0f);
    p.pos.y = idk::randf(-1.0f, +1.0f);
    p.pos.z = idk::randf(-1.0f, +1.0f);
    p.pos = origin + glm::normalize(p.pos);
    p.pos = origin + glm::vec3(0.0f);


    glm::vec3 velA = m_desc.velocity;
    glm::vec3 velB = m_desc.velocity_randomness;
    // glm::vec3 velB = glm::vec3(idk::randf(-1.0f, 1.0f), idk::randf(-1.0f, 1.0f), idk::randf(-1.0f, 1.0f));

            //   velB = glm::length(velA) * glm::normalize(velB);

    glm::mat4 R = glm::lookAt(
        glm::vec3(0.0f),
        glm::normalize(direction),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    R = glm::inverse(R);

    velA.x += idk::randf(-1.0f, +1.0f) * velB.x;
    velA.y += idk::randf(-1.0f, +1.0f) * velB.y;
    velA.z += idk::randf(-1.0f, +1.0f) * velB.z;

    p.vel = glm::mat3(R) * velA;

    p.timer = glm::mix(m_desc.duration*idk::randf(0.0f, 1.0f), m_desc.duration, m_desc.duration_randomness);

    p.scale = glm::mix(idk::randf(0.0f, 1.0f), m_desc.scale, m_desc.scale_randomness);
}


void
idk::ParticleEmitter::_update( float dt, Particle &p )
{
    if (m_particles.size() != m_desc.count)
    {
        m_particles.resize(m_desc.count);

        for (int i=0; i<m_desc.count; i++)
        {
            _reset(m_particles[i]);
        }
    }

    p.pos += dt*p.vel;

    p.timer += dt;
    p.scale = 1.0f - (p.timer / m_desc.duration);

    if (p.timer >= m_desc.duration)
    {
        _reset(p);
    }

}


void
idk::ParticleEmitter::update( float dt )
{
    for (Particle &p: m_particles)
    {
        _update(dt, p);
    }
}


glm::mat4
idk::ParticleEmitter::getTransform( int i, const glm::vec3 &view )
{
    Particle &p = m_particles[i];

    glm::mat4 T = glm::translate(glm::mat4(1.0f), p.pos);

    glm::mat4 R = glm::lookAt(
        glm::vec3(0.0f),
        glm::normalize(p.pos - view),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    R = glm::inverse(R);

    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale * p.scale));

    return T*R*S;
}
