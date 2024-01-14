#include "idk_camera.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>


idk::Camera::Camera(float fov, float near, float far)
:   m_model(1.0f),
    m_projection(1.0f),
    m_view(1.0f),
    m_fov(fov),
    m_near(near),
    m_far(far)
{

}


idk::Camera::Camera():
Camera(80.0f, 0.1f, 100.0f)
{
    glm::vec3 pos;

    pos    = glm::vec3( 0.0f,  0.0f,  0.0f );
    _front = glm::vec3( 0.0f,  0.0f, -1.0f );
    _right = glm::vec3( 1.0f,  0.0f,  0.0f );
    _up    = glm::vec3( 0.0f,  1.0f,  0.0f );

    m_default_pos  = pos;
    _default_front = glm::vec4(_front, 0.0f);
    _default_right = glm::vec4(_right, 0.0f);
    _default_up    = glm::vec4(_up,    0.0f);

    m_view = glm::lookAt(
        pos,
        pos + _front,
        _up
    ); 

    m_projection = glm::perspective(glm::radians(m_fov), 1.0f, m_near, m_far);
}


void
idk::Camera::aspect(float w, float h)
{
    m_aspect = w / h;
    m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}


void
idk::Camera::setOffset(const glm::vec3 &v)
{
    m_offset = v;

    m_default_pos = v;
    m_default_pos.y = 0.0f;

    m_view = glm::lookAt(
        m_default_pos,
        m_default_pos + _front,
        _up
    );
}



void
idk::Camera::addOffset( const glm::vec3 &v )
{
    m_offset += v;

    m_default_pos += v;
    m_default_pos.y = 0.0f;

    m_view = glm::lookAt(
        m_default_pos,
        m_default_pos + _front,
        _up
    );
}


void
idk::Camera::translate( const glm::vec3 &v )
{
    m_model = glm::translate(glm::mat4(1.0f), v) * m_model;
}


void
idk::Camera::elevation( float f )
{
    m_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, f, 0.0f)) * m_model;
}


void
idk::Camera::pitch( float f )
{
    m_model = m_model * glm::rotate(f, glm::vec3(1.0f, 0.0f, 0.0f));
}


void
idk::Camera::roll( float f )
{
    m_view = glm::rotate(m_view, f, _front);

    if (_noroll == false)
    {
        _right = glm::inverse(m_view) * _default_right;
        _up = glm::inverse(m_view) * _default_up;
    }

}


void
idk::Camera::yaw( float f )
{
    glm::mat4 rot = glm::rotate(f, glm::inverse(glm::mat3(m_model)) * glm::vec3(0.0f, 1.0f, 0.0f));
    m_model = m_model * rot;
}


glm::vec3
idk::Camera::front()
{
    return m_model * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
}



glm::vec3
idk::Camera::right()
{
    return m_model * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}



glm::mat4
idk::Camera::view()
{
    static glm::mat4 model_mat;

    model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_offset.y, 0.0f)) * m_model;
    model_mat = glm::inverse(model_mat);

    return m_view * model_mat;
}

