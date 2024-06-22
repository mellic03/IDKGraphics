#pragma once

#include <libidk/GL/idk_glXXBO.hpp>
#include <libidk/GL/idk_glSSBO.hpp>


namespace idk
{
    namespace storage_buffer
    {
        static constexpr uint32_t MAX_CAMERAS     = 1;
        static constexpr uint32_t MAX_DIRLIGHTS   = 1;
        static constexpr uint32_t MAX_POINTLIGHTS = 16;
        static constexpr uint32_t MAX_SPOTLIGHTS  = 8;

        static constexpr uint32_t MAX_TEXTURES   = 1024;
        static constexpr uint32_t MAX_TRANSFORMS = 1024;
        static constexpr uint32_t MAX_DRAW_CALLS = 512;

    };

    struct UBO_Buffer;
    struct SSBO_Buffer;

};



struct IDK_Camera
{
    glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 P = glm::mat4(1.0f);
    glm::mat4 V = glm::mat4(1.0f);

    float width, height, near=0.1f, far=128.0f;
    float exposure=1.0f, gamma=2.2f, shutter, pad0;
    float fov=90.0f, aspect=1.25f, bloom=0.01f, pad1;
};


struct IDK_Dirlight
{
    glm::mat4 transform;
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
};


struct IDK_Pointlight
{
    glm::vec4 position;
    glm::vec4 diffuse;
    glm::vec3 attenuation;
    float     radius;
};


struct IDK_Spotlight
{
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 diffuse;
    glm::vec4 attenuation;
    glm::vec3 angle;
    float     radius;
};



struct idk::SSBO_Buffer
{
    GLuint64    textures          [storage_buffer::MAX_TEXTURES];
    glm::mat4   transforms        [storage_buffer::MAX_TRANSFORMS];
    uint32_t    transform_offsets [storage_buffer::MAX_DRAW_CALLS];
    uint32_t    texture_offsets   [storage_buffer::MAX_DRAW_CALLS];
};


struct idk::UBO_Buffer
{
    IDK_Camera      cameras     [storage_buffer::MAX_CAMERAS];
    IDK_Dirlight    dirlights   [storage_buffer::MAX_DIRLIGHTS];
    IDK_Pointlight  pointlights [storage_buffer::MAX_POINTLIGHTS];
    IDK_Spotlight   spotlights  [storage_buffer::MAX_SPOTLIGHTS];
};
