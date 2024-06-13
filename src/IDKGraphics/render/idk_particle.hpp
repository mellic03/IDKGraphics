#pragma once

#include <libidk/idk_glm.hpp>
#include <vector>


namespace idk
{
    struct ParticleSystem;
    struct Particle;
}

struct idk::ParticleSystem
{
    float system_duration;
    float particle_duration;

    std::vector<float>     timers;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
};


struct idk::Particle
{
    float     timer;
    glm::vec3 position;
    glm::vec3 velocity;
};



