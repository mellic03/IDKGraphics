#pragma once

#include <libidk/GL/idk_glXXBO.hpp>


namespace idk
{
    namespace deferred_lighting
    {
        static constexpr size_t MAX_DIRLIGHTS   = 1;
        static constexpr size_t MAX_POINTLIGHTS = 256;
        static constexpr size_t MAX_SPOTLIGHTS  = 32;
        static constexpr size_t MAX_ATMOSPHERES = 16;
    };


    struct UBORenderData;
    using StreamedUBO = glStreamedBufferObject<GL_UNIFORM_BUFFER, UBORenderData>;
};



struct IDK_Camera
{
    glm::vec4 position;
    glm::mat4 V, P, PV, P_far, PV_far;
    glm::vec4 image_size;
    glm::vec4 image_plane;
    float exposure = 1.0f, gamma, shutter, bloom = 0.2f;

    glm::vec2 chromatic_r = glm::vec2(0.0f);
    glm::vec2 chromatic_g = glm::vec2(0.0f);
    glm::vec2 chromatic_b = glm::vec2(0.0f);
    glm::vec2 chromatic_pad = glm::vec2(0.0f);
    glm::vec4 chromatic_strength = glm::vec4(0.0f);

    glm::vec4 prev_position;
    glm::mat4 prev_V, prev_P, prev_PV;
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
    glm::mat4 transform;
    glm::vec4 position;
    glm::vec4 diffuse;
    glm::vec3 attenuation;
    float     radius;
};


struct IDK_Spotlight
{
    glm::mat4 transform;
    glm::vec4 position;
    glm::quat orientation;
    glm::vec4 direction;
    glm::vec4 diffuse;
    glm::vec4 attenuation;
    glm::vec3 angle;
    float     radius;
};



struct IDK_Atmosphere
{
    glm::mat4   transform        = glm::mat4(1.0f);
    glm::vec4   position         = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4   sun_position     = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4   wavelengths      = glm::vec4(700.0f, 530.0f, 440.0f, 0.0f);
    // float       sealevel         = 20.0f;
    float       radius           = 100.0f;
    float       density_falloff  = 1.01f;
    float       scatter_strength = 2.00f;
    float       atmosphere_scale = 1.25f;
};





#define REE_MAX_CAMERAS 4

struct idk::UBORenderData
{
    IDK_Camera      cameras     [REE_MAX_CAMERAS];
    IDK_Dirlight    dirlights   [idk::deferred_lighting::MAX_DIRLIGHTS];
    IDK_Pointlight  pointlights [idk::deferred_lighting::MAX_POINTLIGHTS];
    IDK_Spotlight   spotlights  [idk::deferred_lighting::MAX_SPOTLIGHTS];
    IDK_Atmosphere  atmospheres [idk::deferred_lighting::MAX_ATMOSPHERES];
};


