#include "idk_rendernode.hpp"
#include "idk_renderengine.hpp"


void
idk::render_node( idk::RenderEngine &ren, idk::RenderNode *node, const glm::mat4 &parent )
{
    node->rotation = glm::normalize(node->rotation);

    glm::mat4 T = glm::translate(glm::mat4(1.0f), node->position);
    glm::mat4 R = glm::mat4_cast(node->rotation);
    glm::mat4 M = T*R;

    ren.drawModel(node->model, M);

    for (idk::RenderNode *child: node->children)
    {
        idk::render_node(ren, child, M);
    }
}
