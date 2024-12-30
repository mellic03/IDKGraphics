#pragma once

#include <vector>
#include <libidk/idk_transform.hpp>


namespace idk
{
    class RenderEngine;
    struct RenderNode;
    void render_node( idk::RenderEngine&, idk::RenderNode*, const glm::mat4& );
}


struct idk::RenderNode
{
private:
    glm::vec4 prev_position;
    glm::quat prev_rotation;

public:
    int model;
    float radSq;
    glm::vec3 position;
    glm::quat rotation;
    std::vector<RenderNode *> children;

    RenderNode( int model_id = -1, float radius_sq = 500.0f )
    :   model(model_id),
        radSq(radius_sq)
    {

    }

};



