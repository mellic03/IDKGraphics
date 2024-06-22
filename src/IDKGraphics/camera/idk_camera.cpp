// #include "idk_camera.hpp"

// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/matrix_decompose.hpp>
// #include <glm/gtx/rotate_vector.hpp>


// idk::Camera::Camera()
// {
//     near   = 0.2f;
//     far    = 500.0f;
//     aspect = 1.0f;
//     fov    = 80.0f;
// }


// glm::vec3
// idk::Camera::getUp()
// {
//     return glm::normalize(glm::mat3(m_model) * glm::vec3(0.0f, 1.0f, 0.0f));
// }

// glm::vec3
// idk::Camera::getRight()
// {
//     return glm::normalize(glm::mat3(m_model) * glm::vec3(1.0f, 0.0f, 0.0f));
// }


// glm::vec3
// idk::Camera::getFront()
// {
//     return glm::normalize(glm::mat3(m_model) * glm::vec3(0.0f, 0.0f, -1.0f));
// }



// glm::vec3
// idk::Camera::getSurfaceUp()
// {
//     return m_up;
// }


// glm::vec3
// idk::Camera::getSurfaceRight()
// {
//     glm::vec3 right;
//               right = glm::normalize(glm::cross(getFront(), getSurfaceUp()));
//             //   right = glm::normalize(glm::mat3(glm::inverse(m_parent)) * right);

//     return right;
// }


// glm::vec3
// idk::Camera::getSurfaceFront()
// {
//     glm::vec3 front;
//               front = glm::normalize(glm::cross(getSurfaceUp(), getRight()));
//             //   front = glm::normalize(glm::mat3(glm::inverse(m_parent)) * front);

//     return front;
// }


// glm::mat4
// idk::Camera::V()
// {
//     // m_up    = glm::normalize(upvector);
//     // m_right = glm::normalize(glm::cross(m_front, m_up));
//     // m_front = glm::normalize(glm::cross(m_up, m_right));

//     // glm::quat qYaw   = glm::angleAxis(m_yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
//     // glm::quat qPitch = glm::angleAxis(m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));

//     // m_local_pitch = glm::mat4_cast(qPitch);
//     // m_local_yaw   = glm::mat4_cast(qYaw);

//     // m_local = glm::inverse(glm::lookAt(glm::vec3(0.0f), m_front, m_up)) * m_local_yaw * m_local_pitch;
//     // m_model = m_world * m_local;
//     // m_view  = glm::inverse(m_model);

//     return glm::inverse(m_model);
// }


// glm::mat4
// idk::Camera::P()
// {
//    return glm::perspective(glm::radians(fov), aspect, near, far);
// }



// void
// idk::Camera::updateParent( const glm::mat4 &parent )
// {
//     // m_world  = parent;
//     // m_view   = V();
//     // position = glm::vec3(m_model[3]);
// }



// void
// idk::Camera::setModelMatrix( const glm::mat4 &M )
// {
//     m_model  = M;
//     position = glm::vec3(M[3]);
// }



// void
// idk::Camera::pitch( float f )
// {
//     m_pitch += glm::radians(f);
// }


// void
// idk::Camera::yaw( float f )
// {
//     m_yaw += glm::radians(f);
// }

