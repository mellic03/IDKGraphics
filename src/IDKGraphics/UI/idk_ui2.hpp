#pragma once

#include <vector>
#include <sstream>

#include <SDL2/SDL_ttf.h>
#include <functional>

#include <glm/glm.hpp>
#include <libidk/GL/idk_glXXBO.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>



template <GLenum gl_target, typename T>
class idk_glTemplatedBufferObject_test
{
private:
    GLuint m_buffer;
    size_t m_nbytes;

public:
    void init( GLuint index, size_t num_elements )
    {
        m_nbytes = num_elements * sizeof(T);

        idk::gl::createBuffers(1, &m_buffer);
        idk::gl::bindBufferBase(gl_target, index, m_buffer);
        idk::gl::namedBufferData(m_buffer, m_nbytes, nullptr, GL_DYNAMIC_COPY);
    };


    void update( void *data )
    {
        idk::gl::namedBufferSubData(m_buffer, 0, m_nbytes, data);
    };

};







namespace idk { class EngineAPI; class RenderEngine; }


namespace idk::ui
{
    enum ElementAlignment: uint32_t
    {
        ALIGN_LEFT   = 1<<1,
        ALIGN_RIGHT  = 1<<2,
        ALIGN_TOP    = 1<<3,
        ALIGN_BOTTOM = 1<<4,
        ALIGN_CENTER = 1<<5
    };

    // struct ElementStyle
    // {
    //     glm::vec4 bg = glm::vec4(0.25f);
    //     glm::vec4 fg = glm::vec4(1.0f);
    //     float radius = 4.0f;
    // };

    struct ElementStyle
    {
        glm::vec4 fg = glm::vec4(1.0f);
        glm::vec4 bg = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float radius;
    };

    class Panel;
    class Button;
    class LayoutManager;

}


namespace idkui2 = idk::ui;


class idkui2::Panel
{
private:

public:
    Panel           *m_parent;
    std::string      m_name;
    ElementAlignment m_alignment;

    int x, y, w, h;
    idkui2::ElementStyle m_style;
    bool m_visible = true;

    std::vector<Button>  m_buttons;
    std::vector<Panel *> m_children;


    Panel( const std::string &name, const ElementAlignment&, const ElementStyle&, Panel *parent = nullptr );

    void giveChild( const std::string &name, ElementAlignment, const ElementStyle& );

    void update( idk::EngineAPI &api, float dt );

};






class idkui2::Button
{
public:
    std::string text = "Blank";
    idkui2::ElementStyle style;
    std::function<void()> callback;

};







class idkui2::LayoutManager
{
private:

    struct PanelQuad
    {
        int32_t x, y, w, h;
        glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);
        glm::vec4 pad = glm::vec4(0.0f);
    };

    struct TextQuad
    {
        int32_t x, y;
        float z;
        int32_t glyph_idx;
    };

    using CmdBuf_type = idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>;
    using PanelUBO_type = idk_glTemplatedBufferObject_test<GL_UNIFORM_BUFFER, PanelQuad>;
    using TextUBO_type  = idk_glTemplatedBufferObject_test<GL_UNIFORM_BUFFER, TextQuad>;


    PanelUBO_type               m_quad_UBO;
    std::vector<PanelQuad>      m_quad_buffer;


    TextUBO_type                m_text_UBO;
    std::vector<TextQuad>       m_text_buffer;

    idk::glDrawCmd              m_drawcmd;
    CmdBuf_type                 m_cmdbuffer;

    uint32_t                    m_VAO, m_VBO, m_IBO;

    std::map<std::string, Panel*>   m_root_panels;
    std::map<std::string, Panel*>   m_panels;
    std::vector<Button>             m_buttons;


    uint32_t _renderFontAtlas( const std::string &filepath, int size );

    void     _genQuadVAO();
    void     _genCommandBuffer();

    void     _renderQuad( int x, int y, int w, int h, const glm::vec4 &color, ElementAlignment algn = ALIGN_CENTER );
    void     _renderText( int x, int y, const std::string&, ElementAlignment algn = ALIGN_CENTER );

    void    _renderPanel( idkui2::Panel*, const glm::vec2&, const glm::vec2& );
    void    _renderButton( const idkui2::Button&, const glm::vec2&, const glm::vec2& );

    void    _renderAllQuads( idk::RenderEngine& );
    void    _renderAllText( idk::RenderEngine& );


public:
    uint32_t m_atlas;
    int m_atlas_w;
    int m_glyph_w;
    int m_grid_w;

    LayoutManager( const std::string &font, int size );

    void update( idk::EngineAPI &api, float dt );


    void renderTexture( idk::EngineAPI &api );


    void createPanel( const std::string &label, uint32_t, const ElementStyle& );

    void createSubPanel( const std::string &panel, const std::string &label,
                         ElementAlignment, const ElementStyle& );

    void openPanel( const std::string &panel );
    void closePanel( const std::string &panel );
    void togglePanel( const std::string &panel );


    void createButton( const std::string &panel, const std::string &label,
                       const ElementStyle&, std::function<void()> );

    void createCheckbox( const std::string &panel, const std::string &label, bool& );

};