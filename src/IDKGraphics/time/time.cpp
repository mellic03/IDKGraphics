#include "time.hpp"
#include "../storage/bindings.hpp"
#include <libidk/GL/idk_glXXBO.hpp>


struct IDK_UBO_Time
{
    glm::vec4  time  = glm::vec4(0.0f, 1.0f, 1.61803398875f, 0.0f);
    glm::uvec4 frame = glm::uvec4(0);
    
    // float    time=0.0f, dtime=1.0f, irrational=1.61803398875f, pad0;
    // uint32_t frame=0, pad1, pad2, pad3;
};


namespace
{
    IDK_UBO_Time m_time;
}


void
idk::updateTime( float dt )
{
    static idk::glBufferObject<GL_UNIFORM_BUFFER> UBO_time(
        shader_bindings::UBO_Time, sizeof(IDK_UBO_Time), GL_DYNAMIC_COPY
    );

    static constexpr float pi = 3.14159f;

    m_time.time[1] = m_time.time[0];
    m_time.time[0] = glm::mod(m_time.time[0]+dt+pi, 2.0f*pi) - pi;

    m_time.time[2] += 1.61803398875f;
    m_time.time[2] = glm::mod(m_time.time[2]+1.0f, 2.0f) - 1.0f;

    m_time.frame[0] = (m_time.frame[0] + 1);

    UBO_time.bufferSubData(0, sizeof(IDK_UBO_Time), &m_time);
}


float
idk::getTime()
{
    return m_time.time[0];
}


float
idk::getDeltaTime()
{
    return m_time.time[0] - m_time.time[1];
}

uint32_t
idk::getFrame()
{
    return m_time.frame[0];
}