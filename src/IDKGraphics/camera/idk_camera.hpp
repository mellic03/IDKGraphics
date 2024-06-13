#pragma once

#include <libidk/idk_transform.hpp>
#include <libidk/idk_glm.hpp>
#include <libidk/idk_export.hpp>

namespace idk { class Camera; };


class idk::Camera
{
private:

    float m_pitch = 0.0f;
    float m_yaw   = 0.0f;
    float m_roll  = 0.0f;


    glm::vec3 m_up    = glm::vec3(0.0f,  1.0f,  0.0f);
    glm::vec3 m_right = glm::vec3(1.0f,  0.0f,  0.0f);
    glm::vec3 m_front = glm::vec3(0.0f,  0.0f, -1.0f);

    glm::mat4 m_local_yaw   = glm::mat4(1.0f);
    glm::mat4 m_local_pitch = glm::mat4(1.0f);

    glm::mat4 m_local = glm::mat4(1.0f);
    glm::mat4 m_world = glm::mat4(1.0f);
    glm::mat4 m_model = glm::mat4(1.0f);
    glm::mat4 m_view  = glm::mat4(1.0f);

public:

    Camera();

    float near   = 0.1f;
    float far    = 100.0f;
    float aspect = 1.0f;
    float fov    = 80.0f;
    float bloom  = 0.0f;
    glm::vec2 chromatic_r;
    glm::vec2 chromatic_g;
    glm::vec2 chromatic_b;
    glm::vec4 chromatic_strength;

    glm::vec3 upvector    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 position    = glm::vec3(0.0f);
    glm::quat orientation = glm::quat(glm::vec3(0.0f));
    glm::quat surface_orientation = glm::quat(glm::vec3(0.0f));


    void updateParent( const glm::mat4 & );


    const glm::mat4 &getLocalMatrix() { return m_local; };
    const glm::mat4 &getWorldMatrix() { return m_world; };
    const glm::mat4 &getModelMatrix() { return m_model; };


    void      setModelMatrix( const glm::mat4 &M );

    glm::vec3 getUp();
    glm::vec3 getRight();
    glm::vec3 getFront();

    glm::vec3 getSurfaceUp();
    glm::vec3 getSurfaceRight();
    glm::vec3 getSurfaceFront();

    glm::mat4 V();
    glm::mat4 P();

    void pitch( float );
    void yaw( float );

};



// class IDK_VISIBLE idk::Camera
// {
// private:

//     glm::mat4           m_model;
//     glm::mat4           m_projection;
//     glm::mat4           m_view;
//     glm::mat4           m_rotation = glm::mat4(1.0f);

//     glm::vec3           _right;
//     glm::vec3           _front;
//     glm::vec3           m_up;

//     glm::vec3           m_default_pos;
//     glm::vec4           _default_right;
//     glm::vec4           _default_front;
//     glm::vec4           _default_up;

//     float               m_aspect;
//     float               m_fov;
//     float               m_near;
//     float               m_far;

//     bool                _ylock  = false;
//     bool                _noroll = false;


// public:

//     glm::vec4           m_bloom_gamma = glm::vec4(0.02f, 2.2f, 165.0f, 1.0f);
//     glm::vec4           m_exposure    = glm::vec4(1.0f,  0.0f, 10.0f, 1.0f);
//     glm::vec4           m_kmhs        = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

//     glm::vec2           aberration[3];

//     glm::vec2           m_r_abr = glm::vec2(0.0f);
//     glm::vec2           m_g_abr = glm::vec2(0.0f);
//     glm::vec2           m_b_abr = glm::vec2(0.0f);
//     float               m_abr_str = 0.0f;
//     glm::vec3           m_offset = glm::vec3(0.0f);

//                         Camera(float fov, float near, float far);
//                         Camera();

//     // const glm::vec3     position()       { return m_transform.position(); };
//     const glm::vec3 &   position()       { return *reinterpret_cast<glm::vec3 *>(&m_model[3]); };
//     const glm::vec3     renderPosition() { return glm::inverse(view())[3]; };

//     glm::mat4 &         model()         { return m_model; };
//     glm::mat4 &         projection()    { return m_projection; };
//     glm::mat4           view();
//     glm::mat4 &         viewref()       { return m_view; };

//     void                setUp( const glm::vec3 &v ) { m_up = v; };
//     glm::vec3           getUp() { return m_up; };


//     float               getFOV()    const { return m_fov;    };
//     float               getAspect() const { return m_aspect; };

//     void                setOffset(const glm::vec3 &v);
//     void                addOffset(const glm::vec3 &v);
//     const glm::vec3 &   getOffset() { return m_offset; };

    

//     void                translate(const glm::vec3 &v);
//     void                elevation(float f);

//     void                pitch(float f);
//     void                roll(float f);
//     void                yaw(float f);

//     glm::vec3           front();
//     glm::vec3           right();

//     void                aspect(float width, float height);
//     void                ylock(bool lock)  { _ylock = lock;  };
//     void                noroll(bool lock) { _noroll = lock; };

//     float               nearPlane() const { return m_near; };
//     float               farPlane()  const { return m_far;  };

// };

