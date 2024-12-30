#pragma once

#include <glm/glm.hpp>
#include <libidk/idk_transform.hpp>

#include <vector>


namespace idk
{
    class RenderNode;
    class NodeRenderer;
}



class idk::NodeRenderer
{
private:

public:
    void render( idk::RenderNode *node );

};



class idk::RenderNode
{
private:
    friend class idk::NodeRenderer;
    glm::mat4 prev_transform;    

public:
    int   model = -1;
    float radSq = 64.0f;
    glm::mat4 transform;

    RenderNode *parent;
    std::vector<RenderNode*> children;

    int giveChild(RenderNode*);
    int removeChild(RenderNode*);

};

