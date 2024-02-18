#pragma once

#include <libidk/GL/idk_glXXBO.hpp>


namespace idk
{
    struct UBORenderData;
    using StreamedUBO = glStreamedBufferObject<GL_UNIFORM_BUFFER, UBORenderData>;
};



struct IDK_Camera
{
    glm::vec4 position;
    glm::mat4 V, P, PV;
    glm::vec4 image_size;
    float exposure, gamma, shutter, padding;

    glm::vec4 prev_position;
    glm::mat4 prev_V, prev_P, prev_PV;
};

struct IDK_Dirlight
{
    glm::vec4 direction;
    glm::vec4 diffuse;
};

struct IDK_Pointlight
{
    glm::vec4 position;
    glm::vec4 diffuse;
    glm::vec4 attenuation;
};

struct IDK_Spotlight
{
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 diffuse;
    glm::vec4 attenuation;
    glm::vec4 angle;
};


#define REE_MAX_CAMERAS     4
#define REE_MAX_DIRLIGHTS   1
#define REE_MAX_POINTLIGHTS 16
#define REE_MAX_SPOTLIGHTS  16

struct idk::UBORenderData
{
    IDK_Camera      cameras     [REE_MAX_CAMERAS];
    IDK_Dirlight    dirlights   [REE_MAX_DIRLIGHTS];
    IDK_Pointlight  pointlights [REE_MAX_POINTLIGHTS];
    IDK_Spotlight   spotlights  [REE_MAX_SPOTLIGHTS];
};


