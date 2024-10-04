#include "idk_ui.hpp"
#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>


static int BASE_DEPTH = 0;


idkui::UIRenderer::UIRenderer( const std::string &filepath, int size )
{
    TTF_Init();
    m_atlas = _renderFontAtlas(filepath, size);
    m_vertexbuffer_quads.resize(MAX_Z_DEPTH);
    _genQuadVAO();
}


void
idkui::UIRenderer::_genQuadVAO()
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
idkui::UIRenderer::_renderFontAtlas( const std::string &filepath, int size )
{
    TTF_Font *font = TTF_OpenFont(filepath.c_str(), size);

    int atlas_w = 512;
    int atlas_h = 512;
    int grid_w = atlas_w / size;
    int glyph_w = size;

    m_atlas_w = atlas_w;
    m_glyph_w = size;
    m_grid_w  = grid_w;

    SDL_Surface *atlas = SDL_CreateRGBSurface(0, atlas_w, atlas_h, 32, 0, 0, 0, 0);

    for (uint8_t current=' '; current<='}'; current++)
    {
        int x = size * ((current - ' ') % grid_w);
        int y = size * ((current - ' ') / grid_w);

        SDL_Surface *S = TTF_RenderGlyph32_Blended(font, (current), {255, 255, 255, 255});
    
        SDL_Rect src = {0, 0, S->w, S->h};
        SDL_Rect dst = {x, y, glyph_w, glyph_w};

        {
            float distance = float(glyph_w / 2) - float(S->w / 2);
            dst.x += distance;
        }

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


    if (std::filesystem::exists("./IDKGE/temp/ui/") == false)
    {
        std::filesystem::create_directory("./IDKGE/temp/ui/");
    }

    IMG_SavePNG(atlas, "./IDKGE/temp/ui/atlas.png");

    uint32_t texture = gltools::loadTexture2D(atlas->w, atlas->h, atlas->pixels, config);
    SDL_FreeSurface(atlas);

    return texture;
}





void
idkui::UIRenderer::renderImage( const Primitive &p, uint32_t texture )
{
    auto [xmin, ymin, xmax, ymax, w, h, z] = p.to_tuple();

    glm::vec3 positions[6] = {
        glm::vec3( xmax, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) ),
    };

    glm::vec2 texcoords[6] = {
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f)
    };

    glm::vec3 extents = glm::vec3(w, h, p.radius[0]);

    for (int i=0; i<6; i++)
    {
        Vertex vert = { positions[i], texcoords[i], extents, glm::vec4(0.0f) };    
        m_vertexbuffer_image.push_back(vert);
    }

    m_samplers.push_back(texture);
}



void
idkui::UIRenderer::renderPrimitive( const Primitive &p, const glm::vec4 &color )
{
    auto [xmin, ymin, xmax, ymax, w, h, z] = p.to_tuple();

    glm::vec3 positions[6] = {
        glm::vec3( xmax, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) ),
    };

    glm::vec2 texcoords[6] = {
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f)
    };

    glm::vec3 extents = glm::vec3(w, h, p.radius[0]);

    for (int i=0; i<6; i++)
    {
        Vertex vert = { positions[i], texcoords[i], extents, color };
        m_vertexbuffer_quads[BASE_DEPTH+z].push_back(vert);
    }
}



void
idkui::UIRenderer::renderText( const std::string &text, const Primitive &p,
                               const glm::vec4 &color )
{
    auto [xmin, ymin, xmax, ymax, w, h, z] = p.to_tuple();

    float label_w = float(text.length() * glyphWidth());
    float scale   = glm::min(label_w, w) / label_w;

    int x = int(xmin + 0.5f);
    int y = int(ymin + 0.5f*h + 0.5f);

    for (char c: text)
    {
        renderGlyph(x, y, z, c, color, scale);
        x += scale*m_glyph_w;
    }
}



void
idkui::UIRenderer::renderTextCentered( const std::string &text, const Primitive &p,
                                       const glm::vec4 &color )
{
    auto [xmin, ymin, xmax, ymax, w, h, z] = p.to_tuple();

    float label_w = float(text.length() * m_glyph_w);
    float width   = glm::min(label_w, w);
    float scale   = width / label_w;

    int x = xmin + 0.5f*w - 0.5f*width;
    int y = ymin + 0.5f*h - 0.5f*m_glyph_w;

    for (char c: text)
    {
        renderGlyph(x, y, z, c, color, scale);
        x += scale*m_glyph_w;
    }
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
idkui::UIRenderer::renderGlyph( int x, int y, int z, char c,
                                const glm::vec4 &color, float scale )
{
    float xmin = x;
    float xmax = x + m_glyph_w * scale;
    float ymin = y;
    float ymax = y + m_glyph_w * scale;

    glm::vec3 positions[6] = {
        glm::vec3( xmax, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) ),
        glm::vec3( xmax, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymin, float(BASE_DEPTH + z) ),
        glm::vec3( xmin, ymax, float(BASE_DEPTH + z) )
    };

    glm::vec2 texcoords[6] = {
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f)
    };


    glm::vec3 extents = scale * glm::vec3(m_glyph_w, m_glyph_w, 0.0f);

    uint32_t idx = uint32_t(c - ' ');
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
idkui::UIRenderer::_renderAllQuads( idk::RenderEngine &ren, const glm::mat4 &projection )
{
    auto &program = ren.getBindProgram("ui-quad");
    program.set_mat4("un_projection", projection);

    for (int i=0; i<m_vertexbuffer_quads.size(); i++)
    {
        if (m_vertexbuffer_quads[i].empty())
        {
            continue;
        }

        gl::namedBufferSubData(
            m_VBO, 0, m_vertexbuffer_quads[i].size()*sizeof(Vertex), m_vertexbuffer_quads[i].data()
        );

        gl::drawArrays(GL_TRIANGLES, 0, m_vertexbuffer_quads[i].size());
    }
}


void
idkui::UIRenderer::_renderAllText( idk::RenderEngine &ren, const glm::mat4 &projection )
{
    if (m_vertexbuffer_glyph.empty())
    {
        return;
    }

    gl::namedBufferSubData(
        m_VBO, 0, m_vertexbuffer_glyph.size()*sizeof(Vertex), m_vertexbuffer_glyph.data()
    );

    auto &program = ren.getBindProgram("ui-text");
    program.set_mat4("un_projection", projection);
    program.set_sampler2D("un_atlas", m_atlas);

    gl::drawArrays(GL_TRIANGLES, 0, m_vertexbuffer_glyph.size());
}


void
idkui::UIRenderer::_renderAllImage( idk::RenderEngine &ren, const glm::mat4 &projection )
{
    if (m_vertexbuffer_image.empty())
    {
        return;
    }

    gl::namedBufferSubData(
        m_VBO, 0, m_vertexbuffer_image.size()*sizeof(Vertex), m_vertexbuffer_image.data()
    );

    static bool first = true;

    if (first)
    {
        first = false;

        auto VS = idk::glShaderStage("IDKGE/shaders/ui-image.vs");
        auto FS = idk::glShaderStage("IDKGE/shaders/ui-image.fs");
        ren.createProgram("ui-image", idk::glShaderProgram(VS, FS));
    }

    auto &program = ren.getBindProgram("ui-image");
    program.set_mat4("un_projection", projection);

    for (int i=0; i<m_vertexbuffer_image.size(); i+=6)
    {
        program.set_sampler2D("un_texture", 0, m_samplers[i/6]);

        gl::drawArrays(GL_TRIANGLES, i, 6);
    }

    m_vertexbuffer_image.clear();
    m_samplers.clear();
}




static idk::EngineAPI *api_ptr = nullptr;


void
idkui::UIRenderer::_renderNode( Element *node )
{
    BASE_DEPTH += node->m_depth_offset;
    node->render(*this);
    _flush();

    for (auto *child: node->children)
    {
        if (child)
        {
            _renderNode(child);
        }
    }
    BASE_DEPTH -= node->m_depth_offset;
}


void
idkui::UIRenderer::renderNode( Element *node, int basedepth )
{
    BASE_DEPTH = basedepth;
    _renderNode(node);
    BASE_DEPTH = 0;
}


void
idkui::UIRenderer::clearTexture( idk::EngineAPI &api, GLbitfield mask )
{
    api_ptr = &api;

    auto &ren = api.getRenderer();
    float rw  = float(ren.winsize().x);
    float rh  = float(ren.winsize().y);

    idk::glFramebuffer &buffer = ren.getUIFrameBuffer();
    buffer.bind();
    buffer.clear(mask);
}




void
idkui::UIRenderer::_flush()
{
    if (api_ptr != nullptr)
    {
        renderTexture(*api_ptr);
    }
}



void
idkui::UIRenderer::renderTexture( idk::EngineAPI &api )
{
    api_ptr = &api;

    auto &ren = api.getRenderer();
    float rw  = float(ren.winsize().x);
    float rh  = float(ren.winsize().y);

    auto &buffer = ren.getUIFrameBuffer();
          buffer.bind();

    gl::bindVertexArray(m_VAO);
    gl::enable(GL_DEPTH_TEST);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 P = glm::ortho(0.0f, rw, rh, 0.0f, -MAX_Z_DEPTH, +MAX_Z_DEPTH);

    _renderAllQuads(ren, P);
    _renderAllImage(ren, P);
    _renderAllText(ren, P);

    // gl::disable(GL_DEPTH_TEST);
    // gl::enable(GL_DEPTH_TEST);

    gl::disable(GL_BLEND);


    for (int i=0; i<m_vertexbuffer_quads.size(); i++)
    {
        m_vertexbuffer_quads[i].clear();
    }

    m_vertexbuffer_quad.clear();
    m_vertexbuffer_glyph.clear();
}

