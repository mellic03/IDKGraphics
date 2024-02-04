#pragma once

#include <libidk/GL/common.hpp>
#include <libidk/idk_export.hpp>

#include "../model/IDKmodel.hpp"
#include "../idk_model_manager.hpp"
#include "../animation/IDKanimation.hpp"



namespace idk::drawmethods
{

    void    bind_material( const std::string &name, glShaderProgram &, Material & );
    void    bind_material( glShaderProgram &, Material & );


    void    dummy( glShaderProgram &, int, const glm::mat4 &, ModelSystem & );


    void    IDK_VISIBLE draw_textured     ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );
    void    IDK_VISIBLE draw_untextured   ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );
    void    IDK_VISIBLE draw_wireframe    ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );
    void    IDK_VISIBLE draw_heightmapped ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );
    void    IDK_VISIBLE draw_instanced    ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );
    void    IDK_VISIBLE draw_indirect     ( glShaderProgram &, int model_id, const glm::mat4 &, ModelSystem & );


    void    IDK_VISIBLE draw_animated( float dtime, glUBO &, int animator, int model,
                           glShaderProgram &, glm::mat4 &, ModelSystem & );


    // void    draw_wireframe( glShaderProgram &, Model &, glm::mat4 & );

};

