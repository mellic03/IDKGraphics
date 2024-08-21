// #include "particle.hpp"
// #include <libidk/idk_random.hpp>




// idk::ParticleEmitter::ParticleEmitter( const idk::ParticleDesc &desc, const glm::vec3 &position,
//                                        const glm::vec3 &vel, const glm::vec3 &dir )
// :   m_desc    (desc),
//     model_id  (desc.model_id),
//     origin    (position),
//     velocity  (vel),
//     direction (glm::normalize(dir))
// {
//     m_particles.resize(64);

//     for (int i=0; i<64; i++)
//     {
//         _reset(m_particles[i]);
//     }

// }


// #include <libidk/idk_log.hpp>

// void
// idk::ParticleEmitter::_reset( Particle &p )
// {
//     p.pos = origin;

//     glm::vec3 velA = m_desc.velocity;
//     glm::vec3 velB = m_desc.velocity_randomness;

//     // glm::mat4 R = glm::lookAt(
//     //     glm::vec3(0.0f),
//     //     glm::normalize(direction),
//     //     glm::vec3(0.0f, 1.0f, 0.0f)
//     // );

//     // R = glm::mat4(glm::mat3(glm::inverse(R)));


//     velA.x += idk::randf(-1.0f, +1.0f) * velB.x;
//     velA.y += idk::randf(-1.0f, +1.0f) * velB.y;
//     velA.z += idk::randf(-1.0f, +1.0f) * velB.z;

//     // p.vel = glm::mat3(R) * velA;
//     p.vel = velA;

//     p.timer = idk::randf(-0.1f, +0.1f);
//     p.scale = m_desc.scale; // + m_desc.scale_randomness * idk::randf(-1.0f, +1.0f);
// }


// void
// idk::ParticleEmitter::_update( float dt, Particle &p )
// {
//     if (m_particles.size() != m_desc.count)
//     {
//         m_particles.resize(m_desc.count);

//         for (int i=0; i<m_desc.count; i++)
//         {
//             _reset(m_particles[i]);
//         }
//     }

//     p.pos += dt*p.vel;

//     p.timer += dt;
//     p.scale = 1.0f - (p.timer / m_desc.duration);
//     p.scale = glm::clamp(p.scale, 0.001f, m_desc.scale);

//     if (p.timer >= m_desc.duration)
//     {
//         _reset(p);
//     }
// }


// void
// idk::ParticleEmitter::update( float dt )
// {
//     for (Particle &p: m_particles)
//     {
//         _update(dt, p);
//     }

//     if (m_duration >= 0.0f)
//     {
//         m_timer += dt;
//     }
// }


// glm::mat4
// idk::ParticleEmitter::getTransform( int i, const glm::vec3 &view )
// {
//     Particle &p = m_particles[i];

//     glm::mat4 T = glm::translate(glm::mat4(1.0f), p.pos);

//     glm::mat4 R = glm::lookAt(
//         glm::vec3(0.0f),
//         glm::normalize(p.pos - view),
//         glm::vec3(0.0f, 1.0f, 0.0f)
//     );

//     R = glm::inverse(R);

//     glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(p.scale));

//     return T*R*S;
// }
