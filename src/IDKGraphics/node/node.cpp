#include "node.hpp"



void
idk::NodeRenderer::render( idk::RenderNode *node) 
{
    glm::mat4 T = node->parent->transform * node->transform;

    if (node->model != -1)
    {
        // RQ.enqueue(node->model, T, node.prev_transform);
        node->prev_transform = T;
    }

    for (auto *child: node->children)
    {
        render(child);
    }
}


int
idk::RenderNode::giveChild( RenderNode *node )
{
    children.push_back(node);
    return children.size() - 1;
}


int
idk::RenderNode::removeChild( RenderNode *node )
{
    int idx = -1;

    for (int i=0; i<children.size(); i++)
    {
        if (children[i] == node)
        {
            idx = i;
            break;
        }
    }

    std::swap(children.back(), children[idx]);
    children.back()->parent = nullptr;
    children.pop_back();

    return idx;
}


