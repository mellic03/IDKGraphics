#include "idk_ui2.hpp"

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>


static glm::vec2 m_screen_size = glm::vec2(512.0f);
static glm::vec2 m_mouse = glm::vec2(512.0f);
static bool  m_clicked = false;
static float m_zindex = 1.0f;



idkui2::Panel::Panel( const std::string &name, const ElementAlignment &alignment,
                      const ElementStyle &style, Panel *parent )
:   m_parent    (parent),
    m_name      (name),
    m_alignment (alignment),
    m_style     (style)
{

}


void
idkui2::Panel::giveChild( const std::string &name, ElementAlignment alignment, const ElementStyle &style )
{
    m_children.push_back(new Panel(name, alignment, style, this));
}



idkui2::LayoutManager::LayoutManager( const std::string &filepath, int size )
{
    m_atlas = _renderFontAtlas(filepath, size);

    _genQuadVAO();

    m_quad_UBO.init(2, 1024);
    m_quad_buffer.resize(1024);

    m_text_UBO.init(1, 1024);
    m_text_buffer.resize(1024);

    m_cmdbuffer.init();
    m_cmdbuffer.bufferData(1 * sizeof(idk::glDrawCmd), nullptr, GL_DYNAMIC_COPY);

}


void
idkui2::LayoutManager::_genQuadVAO()
{
    float vertices[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  


    size_t vert_nbytes = 5 * sizeof(float); // x, y, u, v
    size_t vbo_nbytes  = 6 * vert_nbytes;
    size_t ibo_nbytes  = 6 * sizeof(uint32_t);

    gl::createBuffers(1, &m_VBO);
    gl::namedBufferData(m_VBO, vbo_nbytes, vertices, GL_STATIC_DRAW);

    gl::createBuffers(1, &m_IBO);
    gl::namedBufferData(m_IBO, ibo_nbytes, indices, GL_STATIC_DRAW);

    gl::createVertexArrays(1, &m_VAO);
    gl::vertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, vert_nbytes);
    gl::vertexArrayElementBuffer(m_VAO, m_IBO);

    gl::enableVertexArrayAttrib(m_VAO, 0);
    gl::vertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    gl::vertexArrayAttribBinding(m_VAO, 0, 0);

    gl::enableVertexArrayAttrib(m_VAO, 1);
    gl::vertexArrayAttribFormat(m_VAO, 1, 2, GL_FLOAT, GL_FALSE, 3*sizeof(float));
    gl::vertexArrayAttribBinding(m_VAO, 1, 0);
}




uint32_t
idkui2::LayoutManager::_renderFontAtlas( const std::string &filepath, int size )
{
    TTF_Font *font = TTF_OpenFont(filepath.c_str(), size);

    int atlas_w = 512;
    int atlas_h = 512;
    int grid_w = atlas_w / size;
    int glyph_w = size;

    m_atlas_w = 512;
    m_glyph_w = size;
    m_grid_w  = grid_w;

    SDL_Surface *atlas = SDL_CreateRGBSurface(0, atlas_w, atlas_h, 32, 0, 0, 0, 0);

    for (uint8_t current='A'; current<='z'; current++)
    {
        int x = size * ((current-'A') % grid_w);
        int y = size * ((current-'A') / grid_w);

        SDL_Surface *S = TTF_RenderGlyph32_Blended(font, (current), {255, 255, 255, 255});

        SDL_Rect src = {0, 0, S->w, S->h};
        SDL_Rect dst = {x, y, glyph_w, glyph_w};

        SDL_BlitSurface(S, &src, atlas, &dst);
        SDL_FreeSurface(S);
    }

    idk::glTextureConfig config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_NEAREST,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    uint32_t texture = gltools::loadTexture2D(atlas->w, atlas->h, atlas->pixels, config);
    SDL_FreeSurface(atlas);

    return texture;
}



void
idkui2::LayoutManager::_renderQuad( int x, int y, int w, int h, const glm::vec4 &color,
                                    ElementAlignment alignment )
{
    if (alignment & ALIGN_CENTER)
    {
        x -= w/2;
        y -= h/2;
    }


    PanelQuad quad;
    quad.x = x;
    quad.y = y;
    quad.w = w;
    quad.h = h;
    quad.color = color;
    quad.pad   = glm::vec4(16.0f, m_zindex, 0.0f, 0.0f);

    m_quad_buffer.push_back(quad);
}


void
idkui2::LayoutManager::_renderText( int x, int y, const std::string &text, ElementAlignment alignment )
{
    int width = m_glyph_w * text.length();
    glm::ivec2 offset = glm::ivec2(0.0f);

    if (alignment & ALIGN_CENTER)
    {
        offset = glm::ivec2(-width/2.0f, 0.0f);
    }

    for (char c: text)
    {
        int32_t idx = int32_t(c - 'A');

        m_text_buffer.push_back({x+offset.x, y+offset.y, m_zindex, idx});
        x += m_glyph_w;
    }
}


void
idkui2::LayoutManager::createPanel( const std::string &label, uint32_t alignment,
                                    const ElementStyle &style )
{
    Panel *P = new idkui2::Panel(label, ElementAlignment(alignment), style);
           P->m_visible = false;

    m_panels[label] = P;
    m_root_panels[label] = P;
}


void
idkui2::LayoutManager::createSubPanel( const std::string &panel, const std::string &label,
                                       ElementAlignment alignment, const ElementStyle &style )
{
    if (m_panels.contains(panel) == false)
    {
        LOG_ERROR() << "Panel \"" << panel << "\" does not exist.";
        return;
    }

    m_panels[panel]->giveChild(label, alignment, style);
    m_panels[label] = m_panels[panel]->m_children.back();
    m_panels[panel]->m_children.back()->m_style.bg += 0.2f;
}



void
idkui2::LayoutManager::openPanel( const std::string &panel )
{
    m_panels[panel]->m_visible = true;
}


void
idkui2::LayoutManager::closePanel( const std::string &panel )
{
    m_panels[panel]->m_visible = false;
}


void
idkui2::LayoutManager::togglePanel( const std::string &panel )
{
    if (m_panels.contains(panel) == false)
    {
        LOG_ERROR() << "Panel \"" << panel << "\" does not exist.";
        return;
    }

    bool b = m_panels[panel]->m_visible;
    m_panels[panel]->m_visible = !b;
}





void
idkui2::LayoutManager::createButton( const std::string &panel, const std::string &label,
                                     const ElementStyle &style, std::function<void()> callback )
{
    if (m_panels.contains(panel) == false)
    {
        return;
    }

    m_panels[panel]->m_buttons.push_back({label, style, callback});
}


void
idkui2::LayoutManager::disableButton( const std::string &panel, const std::string &button )
{
    if (m_panels.contains(panel) == false)
    {
        return;
    }

    for (Button &b: m_panels[panel]->m_buttons)
    {
        if (b.text == button)
        {
            b.enabled = false;
        }
    }
}




void
idkui2::LayoutManager::_renderButton( const idkui2::Button &button, const glm::vec2 &center,
                                      const glm::vec2 &span )
{
    if (button.enabled == false)
    {
        return;
    }


    glm::vec2 pos = center;

    float width  = m_glyph_w * button.text.length();
    float height = m_glyph_w;

    int x = int(pos.x);
    int y = int(pos.y);
    int w = int(width);
    int h = int(height);

    // x -= w/2;
    // y -= h/2;

    ElementStyle style0 = button.style;
    ElementStyle style1 = button.style;
    std::swap(style1.fg, style1.bg);

    m_zindex += 0.01f;

    if (geometry::pointInsideRect(m_mouse.x, m_mouse.y, x, y, w, h))
    {
        if (m_clicked)
        {
            button.callback();
        }

        std::swap(style0, style1);
    }


    _renderQuad(x, y, w+4, h+4, 2.0f*style0.bg, ALIGN_LEFT);
    m_zindex += 0.01f;
    _renderQuad(x, y, w, h, style0.bg, ALIGN_LEFT);


    m_zindex += 0.01f;

    _renderText(x, y, button.text, ALIGN_LEFT);

    m_zindex += 0.01f;

}



void
idkui2::LayoutManager::_renderPanel( idkui2::Panel *P, const glm::vec2 &center, const glm::vec2 &span )
{
    if (P->m_visible == false)
    {
        return;
    }

    glm::vec2 size   = 0.5f * span;
    glm::vec2 delta  = glm::vec2(0.0f);

    if (P->m_alignment & ALIGN_LEFT)
    {
        delta.x -= 2.0f * (size.x / 3.0f);
    }

    if (P->m_alignment & ALIGN_RIGHT)
    {
        delta.x += 2.0f * (size.x / 3.0f);
    }

    if (P->m_alignment & ALIGN_TOP)
    {
        delta.y -= size.y/2;
    }

    if (P->m_alignment & ALIGN_BOTTOM)
    {
        delta.y += size.y/2;
    }

    glm::vec2 new_center = center + delta;
    glm::vec2 pos = new_center;

    m_zindex += 0.01f;

    _renderQuad(pos.x, pos.y, size.x/2.0f + 4, size.y/2.0f + 4, 2.0f*P->m_style.bg);
    m_zindex += 0.01f;

    _renderQuad(pos.x, pos.y, size.x/2.0f, size.y/2.0f, P->m_style.bg);
    m_zindex += 0.01f;

    glm::vec2 offset = glm::vec2(-size/4.0f) + 16.0f;

    for (Button &b: P->m_buttons)
    {
        _renderButton(b, new_center + offset, size/4.0f);
        offset.y += 2*m_glyph_w;
    }

    m_zindex += 0.01f;

    for (Panel *child: P->m_children)
    {
        _renderPanel(child, new_center, size);
    }

}


void
idkui2::LayoutManager::update( idk::EngineAPI &api, float dt )
{
    m_quad_buffer.clear();
    m_text_buffer.clear();

    m_zindex = -0.9f;

    for (auto &[key, P]: m_root_panels)
    {
        _renderPanel(P, m_screen_size/2.0f, m_screen_size);
    }

}


void
idkui2::LayoutManager::_renderAllQuads( idk::RenderEngine &ren )
{
    if (m_quad_buffer.empty())
    {
        return;
    }

    idk::glDrawCmd cmd = {
        .count           = 6,
        .instanceCount   = uint32_t(m_quad_buffer.size()),
        .firstIndex      = 0,
        .baseVertex      = 0,
        .baseInstance    = 0
    };


    m_cmdbuffer.bufferSubData(0, sizeof(idk::glDrawCmd), (const void *)(&cmd));
    m_quad_UBO.update(m_quad_buffer.data());

    auto &program = ren.getProgram("ui-quad");
    program.bind();

    glm::mat4 P = glm::ortho(0.0f, m_screen_size.x, m_screen_size.y, 0.0f, -1.0f, +1.0f);
    program.set_mat4("un_projection", P);

    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_cmdbuffer.ID());

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );

}


void
idkui2::LayoutManager::_renderAllText( idk::RenderEngine &ren )
{
    if (m_text_buffer.empty())
    {
        return;
    }

    idk::glDrawCmd cmd = {
        .count           = 6,
        .instanceCount   = uint32_t(m_text_buffer.size()),
        .firstIndex      = 0,
        .baseVertex      = 0,
        .baseInstance    = 0
    };


    m_cmdbuffer.bufferSubData(0, sizeof(idk::glDrawCmd), (const void *)(&cmd));
    m_text_UBO.update(m_text_buffer.data());

    auto &program = ren.getProgram("ui-text");
    program.bind();

    glm::mat4 P = glm::ortho(0.0f, m_screen_size.x, m_screen_size.y, 0.0f, -1.0f, +1.0f);
    program.set_mat4("un_projection", P);

    program.set_sampler2D("un_atlas", m_atlas);
    program.set_int("un_grid_w",  m_grid_w);
    program.set_int("un_glyph_w", m_glyph_w);

    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_cmdbuffer.ID());

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );

}



void
idkui2::LayoutManager::renderTexture( idk::EngineAPI &api )
{
    auto &ren = api.getRenderer();

    int rw = ren.width();
    int rh = ren.height();

    m_screen_size = glm::vec2(rw, rh);
    m_mouse   = api.getEventSys().mousePosition();
    m_clicked = api.getEventSys().mouseClicked(idk::MouseButton::LEFT);

    gl::bindVertexArray(m_VAO);
    gl::enable(GL_DEPTH_TEST, GL_BLEND);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    idk::glFramebuffer &buffer = ren.getUIFrameBuffer();
    buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    buffer.bind();

    _renderAllQuads(ren);
    _renderAllText(ren);

    gl::disable(GL_BLEND);

}

