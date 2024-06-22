#include "idk_ui2.hpp"
#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>



idkui2::UIRenderer::UIRenderer( const std::string &filepath, int size )
{
    m_atlas = _renderFontAtlas(filepath, size);

    _genQuadVAO();

}



void
idkui2::UIRenderer::_genQuadVAO()
{
    size_t MAX_QUADS = 512;

    size_t vert_nbytes = sizeof(Vertex);
    size_t vbo_nbytes  = MAX_QUADS * (vert_nbytes / sizeof(float)) * vert_nbytes;

    gl::createBuffers(1, &m_VBO);
    gl::namedBufferStorage(m_VBO, vbo_nbytes, nullptr, GL_DYNAMIC_STORAGE_BIT);

    gl::createVertexArrays(1, &m_VAO);
    gl::vertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, vert_nbytes);

    gl::enableVertexArrayAttrib(m_VAO, 0);
    gl::enableVertexArrayAttrib(m_VAO, 1);
    gl::enableVertexArrayAttrib(m_VAO, 2);
    gl::enableVertexArrayAttrib(m_VAO, 3);

    gl::vertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    gl::vertexArrayAttribFormat(m_VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texcoord));
    gl::vertexArrayAttribFormat(m_VAO, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, extents));
    gl::vertexArrayAttribFormat(m_VAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));

    gl::vertexArrayAttribBinding(m_VAO, 0, 0);
    gl::vertexArrayAttribBinding(m_VAO, 1, 0);
    gl::vertexArrayAttribBinding(m_VAO, 2, 0);
    gl::vertexArrayAttribBinding(m_VAO, 3, 0);
}



uint32_t
idkui2::UIRenderer::_renderFontAtlas( const std::string &filepath, int size )
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
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_TRUE
    };

    uint32_t texture = gltools::loadTexture2D(atlas->w, atlas->h, atlas->pixels, config);
    SDL_FreeSurface(atlas);

    return texture;
}




void
idkui2::UIRenderer::renderQuad( int x, int y, int z, int w, int h, float r, const glm::vec4 &color )
{
    float xmin = x;
    float xmax = x+w;
    float ymin = y;
    float ymax = y+h;


    glm::vec3 positions[6] = {
        glm::vec3( xmax, ymax, float(z) ),
        glm::vec3( xmax, ymin, float(z) ),
        glm::vec3( xmin, ymax, float(z) ),
        glm::vec3( xmax, ymin, float(z) ),
        glm::vec3( xmin, ymin, float(z) ),
        glm::vec3( xmin, ymax, float(z) ),
    };

    glm::vec2 texcoords[6] = {
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f)
    };

    glm::vec3 extents = glm::vec3(w, h, r);

    for (int i=0; i<6; i++)
    {
        Vertex vert = { positions[i], texcoords[i], extents, color };
        m_vertexbuffer_quad.push_back(vert);
    }
}


void
idkui2::UIRenderer::renderQuadCentered( int x, int y, int z, int w, int h, float r,
                                        const glm::vec4 &color )
{
    renderQuad(x-w/2, y-h/2, z, w, h, r, color);
}




glm::vec2 atlas_uv( int row, int col, int glyph_w, int atlas_w, glm::vec2 texcoord )
{
    glm::ivec2 texel = glm::ivec2(float(atlas_w) * texcoord);
               texel /= atlas_w / glyph_w;
               texel += glyph_w * glm::ivec2(col, row);
               texel.x -= glyph_w / 8;
               texel.y += glyph_w / 8;

    return glm::vec2(texel) / float(atlas_w);
}


void
idkui2::UIRenderer::renderGlyph( int x, int y, int z, char c, const glm::vec4 &color )
{
    float xmin = x;
    float xmax = x + m_glyph_w;
    float ymin = y;
    float ymax = y + m_glyph_w;

    glm::vec3 positions[6] = {
        glm::vec3( xmax, ymax, float(z)),
        glm::vec3( xmax, ymin, float(z)),
        glm::vec3( xmin, ymax, float(z)),
        glm::vec3( xmax, ymin, float(z)),
        glm::vec3( xmin, ymin, float(z)),
        glm::vec3( xmin, ymax, float(z)),
    };

    glm::vec2 texcoords[6] = {
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f)
    };


    glm::vec3 extents = glm::vec3(m_glyph_w, m_glyph_w, 0.0f);

    uint32_t idx = uint32_t(c - 'A');
    uint32_t row = idx / m_grid_w;
    uint32_t col = idx % m_grid_w;

    for (int i=0; i<6; i++)
    {
        glm::ivec2 texel = m_atlas_w * glm::ivec2(texcoords[i]);
                   texel /= m_grid_w;
                   texel.x += col;
                   texel.y += row;

        glm::vec2 texcoord = atlas_uv(row, col, m_glyph_w, m_atlas_w, texcoords[i]);

        Vertex vert = { positions[i], texcoord, extents, color };
        m_vertexbuffer_glyph.push_back(vert);
    }
}



void
idkui2::UIRenderer::renderText( int x, int y, int z, const std::string &text,
                                const glm::vec4 &color )
{
    for (char c: text)
    {
        renderGlyph(x, y, z, c, color);
        x += m_glyph_w;
    }
}


void
idkui2::UIRenderer::renderTextCentered( int x, int y, int z, const std::string &text,
                                        const glm::vec4 &color )
{
    x -= 0.5f * (m_glyph_w * text.length());
    y -= 0.5f * m_glyph_w;

    renderText(x, y, z, text, color);
}



void
idkui2::UIRenderer::renderTextCenteredX( int x, int y, int z, const std::string &text,
                                         const glm::vec4 &color )
{
    x -= 0.5f * (m_glyph_w * text.length());
    renderText(x, y, z, text, color);
}


// void
// idkui2::UIRenderer::renderButton( int x, int y, int z, const std::string &text,
//                                   const glm::vec3 &fg, const glm::vec3 &bg )
// {
//     int w = m_glyph_w * text.length();
//     int h = m_glyph_w;

//     renderQuad(x, y, z+0, w, h, bg);
//     renderText(x, y, z+1, text, fg);
// }


// void
// idkui2::UIRenderer::renderButtonCentered( int x, int y, int z, const std::string &text,
//                                           const glm::vec3 &fg, const glm::vec3 &bg )
// {
//     int w = m_glyph_w * text.length();
//     int h = m_glyph_w;

//     x -= w/2;
//     y -= h/2;

//     renderButton(x, y, z, text, fg, bg);
// }



// void
// idkui2::UIRenderer::renderButtonCenteredX( int x, int y, int z, const std::string &text,
//                                            const glm::vec3 &fg, const glm::vec3 &bg )
// {
//     x -= 0.5f * m_glyph_w * text.length();
//     renderButton(x, y, z, text, fg, bg);
// }






void
idkui2::UIRenderer::_renderAllQuads( idk::RenderEngine &ren, const glm::mat4 &projection )
{
    if (m_vertexbuffer_quad.empty())
    {
        return;
    }
  
    gl::namedBufferSubData(
        m_VBO, 0, m_vertexbuffer_quad.size()*sizeof(Vertex), m_vertexbuffer_quad.data()
    );

    auto &program = ren.getProgram("ui-quad");
    program.bind();
    program.set_mat4("un_projection", projection);

    gl::drawArrays(GL_TRIANGLES, 0, m_vertexbuffer_quad.size());

}


void
idkui2::UIRenderer::_renderAllText( idk::RenderEngine &ren, const glm::mat4 &projection )
{
    if (m_vertexbuffer_glyph.empty())
    {
        return;
    }
  
    gl::namedBufferSubData(
        m_VBO, 0, m_vertexbuffer_glyph.size()*sizeof(Vertex), m_vertexbuffer_glyph.data()
    );

    auto &program = ren.getProgram("ui-text");
    program.bind();

    program.set_mat4("un_projection", projection);
    program.set_sampler2D("un_atlas", m_atlas);

    gl::drawArrays(GL_TRIANGLES, 0, m_vertexbuffer_glyph.size());
}



void
idkui2::UIRenderer::renderTexture( idk::EngineAPI &api )
{
    auto &ren = api.getRenderer();

    float rw = float(ren.width());
    float rh = float(ren.height());

    glm::mat4 projection = glm::ortho(0.0f, rw, rh, 0.0f, -MAX_Z_DEPTH, +MAX_Z_DEPTH);


    idk::glFramebuffer &buffer = ren.getUIFrameBuffer();
    buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    buffer.bind();


    gl::bindVertexArray(m_VAO);

    gl::enable(GL_DEPTH_TEST, GL_BLEND);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _renderAllQuads(ren, projection);
    _renderAllText(ren, projection);

    gl::disable(GL_BLEND);


    m_vertexbuffer_quad.clear();
    m_vertexbuffer_glyph.clear();
}

