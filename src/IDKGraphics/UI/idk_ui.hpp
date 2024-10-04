#pragma once

#include <vector>
#include <sstream>

#include <SDL2/SDL_ttf.h>
#include <array>
#include <functional>

#include <glm/glm.hpp>
#include <libidk/GL/idk_glXXBO.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>
#include <libidk/idk_allocator.hpp>


namespace idk { class EngineAPI; class RenderEngine; }


namespace idk::ui
{
    struct Style
    {
        enum Alignment: uint32_t { LEFT, RIGHT, TOP, BOTTOM, CENTER };

        glm::vec4 margin    = glm::vec4(0.0f);
        glm::vec4 padding   = glm::vec4(0.0f);
        glm::vec4 radius    = glm::vec4(0.0f);
        glm::vec4 fg        = glm::vec4(1.0f);
        glm::vec4 bg        = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 border    = glm::vec4(0.0f);
        Alignment align     = CENTER;
        glm::vec2 minbounds = glm::vec2(0.0f);
        glm::vec2 maxbounds = glm::vec2(+INFINITY);
    };


    struct Primitive;
    struct Element;
    struct Composite;
    struct List;
    struct Grid;
    struct GridUniform;
    struct Stack;
    struct Text;
    struct TextAbsolute;
    struct Button;
    struct Image;
    struct ImageAbsolute;


    class LayoutManager;
    class UIRenderer;

}

namespace idkui = idk::ui;





inline std::tuple<float, float> to_tuple( const glm::vec2 &v )
{
    return std::make_tuple(v.x, v.y);
}

inline std::tuple<float, float, float> to_tuple( const glm::vec3 &v )
{
    return std::make_tuple(v.x, v.y, v.z);
}


struct idkui::Primitive
{
    glm::vec2 corner = glm::vec2(0.0f);
    glm::vec2 span   = glm::vec2(0.0f);
    int       depth  = 1;
    glm::vec4 radius = glm::vec4(0.0f);

    std::tuple<float, float, float, float, float, float, int>
    to_tuple() const
    {
        return std::make_tuple(
            corner.x,        corner.y,
            corner.x+span.x, corner.y+span.y,
            span.x,          span.y,
            depth
        );
    }
};





struct idkui::Element
{
    std::string  m_label = "Element";
    glm::vec2    m_corner = glm::vec2(0.0f);
    glm::vec2    m_span   = glm::vec2(0.0f);
    int          m_depth  = 1;
    int          m_depth_offset = 0;

    idkui::Style m_style;
    idkui::Style m_style2;
    Primitive    m_inner_bounds;
    Primitive    m_outer_bounds;

    std::vector<Element*> children;

    Element( const Style &style )
    :   m_style(style), m_style2(style)
    {

    }

    Element( const Style &style, const std::string &label )
    :   m_style(style), m_style2(style), m_label(label)
    {

    }

    Element( const Style &style, const std::initializer_list<Element*> &args )
    :   m_style(style), m_style2(style), children(args)
    {

    }

    Element( const std::initializer_list<Element*> &args )
    :  children(args)
    {

    }

    Primitive computeOuterBounds( const glm::vec2&, const glm::vec2&, int, const Style& );
    Primitive computeInnerBounds( const glm::vec2&, const glm::vec2&, int, const Style& );

    static bool mouseOver( const Primitive& );

    virtual void updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
    {
        m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
        m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

        for (auto *child: children)
        {
            if (child)
            {
                child->updateLayout(m_corner, m_span, m_depth);
            }
        }
    }

    void setDepthOffset( int depth ) { m_depth_offset = depth; }

    virtual void render( idkui::UIRenderer& ) {  };
    virtual void onHover()  { };
    virtual void offHover() { };
    virtual void onClick()  { };
    virtual void onChange() { };
};



struct idkui::Composite: public Element
{
    Composite( const std::initializer_list<Element*> &args )
    :   Element(args)
    {

    }

    // virtual void updateLayout( const glm::vec2&, const glm::vec2&, int );
    // virtual void render( idkui::UIRenderer& );
};




struct idkui::List: public Element
{
    List( int rows, const Style &style )
    :   Element(style)
    {
        children = std::vector<Element*>(rows, nullptr);
    }

    List( const Style &style, std::initializer_list<Element*> args )
    :   Element(style, args)
    {

    }

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int );
    virtual void render( idkui::UIRenderer& );
};




struct idkui::Grid: public Element
{
    int m_rows, m_cols;
    std::vector<float> row_ratio;
    std::vector<float> col_ratio;

    Grid( int rows, int cols, const Style &style )
    :   Element(style),
        m_rows(rows),
        m_cols(cols),
        row_ratio(rows, 1.0f / float(rows)),
        col_ratio(cols, 1.0f / float(cols))
    {
        children = std::vector<Element*>(rows*cols, nullptr);
    }

    Grid( const Style &style, std::vector<std::vector<Element*>> args )
    :   Element(style),
        m_rows(args.size()),
        m_cols(args[0].size()),
        row_ratio(m_rows, 1.0f / float(m_rows)),
        col_ratio(m_cols, 1.0f / float(m_cols))
    {
        children = std::vector<Element*>(m_rows*m_cols, nullptr);

        for (int row=0; row<m_rows; row++)
        {
            for (int col=0; col<m_cols; col++)
            {
                children[m_cols*row + col] = args[row][col];
            }
        }
    }

    void setRowRatios( const std::vector<float> &ratios ) { row_ratio = ratios; }

    void setColRatios( const std::vector<float> &ratios )
    {
        col_ratio = ratios;

        float length = 0.0f;
        for (int i=0; i<m_cols; i++)
        {
            length += col_ratio[i];
        }

        IDK_ASSERT("Cannot normalize proportions", length > 0.0001f);

        for (int i=0; i<m_cols; i++)
        {
            col_ratio[i] /= length;
        }
    }

    Element *&getChild( int row, int col );

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int );
    virtual void render( idkui::UIRenderer& ) override;
};




struct idkui::GridUniform: public Grid
{
    GridUniform( int total, int cols, const Style &style )
    :   Grid(cols, cols, style)
    {
        m_cols = cols;
        children.resize(total, nullptr);
    }

    GridUniform( int cols, const Style &style, const std::vector<std::vector<Element*>> &args )
    :   Grid(style, args)
    {
        m_cols = cols;
    }

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int );
};



struct idkui::Stack: public Element
{
private:
    struct StackNode: public Element
    {
        StackNode *prev = nullptr;

        StackNode( const Style &style, Element *node )
        :   Element(style, {node}) {  };
    
        virtual void render( idkui::UIRenderer& ) {  };
    };

    StackNode *m_top;
    std::vector<Element*> m_stack;


public:
    Stack( const Style &style )
    :   Element(style)
    {
        m_top = new StackNode(style, nullptr);
        children.push_back(m_top);
    };

    void push( Element *node )
    {
        StackNode *prev = m_top;
        m_top = new StackNode(m_style2, node);
        m_top->prev = prev;

        children[0] = m_top;

        // if (children.empty())
        // {
        //     children.push_back(node);
        // }
        // else
        // {
        //     m_stack.push_back(children[0]);
        //     children[0] = node;
        // }
    }

    void pop()
    {
        if (m_top->prev != nullptr)
        {
            StackNode *top = m_top;
            m_top = top->prev;

            children[0] = m_top;
        
            delete top;
        }

        // if (children.empty())
        // {
        //     return;
        // }

        // children.pop_back();

        // if (m_stack.size() > 0)
        // {
        //     children.push_back(m_stack.back());
        //     m_stack.pop_back();
        // }
    }

    Element *top()
    {
        if (m_top->children.empty())
        {
            return nullptr;
        }

        return m_top->children[0];
        // return (children.empty()) ? nullptr : children.back();
    }

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int ) override;
    virtual void render( idkui::UIRenderer& ) {  }
};



struct idkui::Text: public Element
{
    Text( const std::string &label, const Style &style )
    :   Element(style, label) {  }

    virtual void render( idkui::UIRenderer &ren ) override;
};



struct idkui::TextAbsolute: public Text
{
private:
    glm::vec2 m_position = glm::vec2(0.0f);

public:
    TextAbsolute( const std::string &label, const Style &style )
    :   Text(label, style) {  }

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int ) override;
    void setPosition( float x, float y ) { m_position = glm::vec2(x, y); }
};



struct idkui::Button: public Element
{
    std::function<void()> callback;

    Button( const std::string &label, const Style &buttonstyle, const Style &textstyle,
            const std::function<void()> &onclick = [](){} )
    :   Element(buttonstyle, {new idkui::Text(label, textstyle)}),
        callback(onclick)
    {
        
    }

    virtual void updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth );
    virtual void render( idkui::UIRenderer &ren );

    virtual void onHover() override
    {
        m_style.fg = glm::mix(m_style.fg, m_style2.bg, 0.95f);
        m_style.bg = glm::mix(m_style.bg, m_style2.fg, 0.95f);
    };

    virtual void offHover() override
    {
        m_style.fg = glm::mix(m_style.fg, m_style2.fg, 0.95f);
        m_style.bg = glm::mix(m_style.bg, m_style2.bg, 0.95f);
    };

    virtual void onClick() override
    {
        this->callback();
    }

};




struct idkui::Image: public Element
{
    uint32_t m_texture;

    Image( uint32_t texture, const Style &style )
    :   Element(style),
        m_texture(texture)
    {
        
    }

    virtual void updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth );
    virtual void render( idkui::UIRenderer &ren );

    virtual void onHover() override
    {
        m_style.fg = glm::mix(m_style.fg, m_style2.bg, 0.95f);
        m_style.bg = glm::mix(m_style.bg, m_style2.fg, 0.95f);
    };

    virtual void offHover() override
    {
        m_style.fg = glm::mix(m_style.fg, m_style2.fg, 0.95f);
        m_style.bg = glm::mix(m_style.bg, m_style2.bg, 0.95f);
    };
};





struct idkui::ImageAbsolute: public Image
{
private:
    std::vector<glm::vec2> m_positions;
    glm::vec2 m_position = glm::vec2(0.0f);
    glm::vec2 m_offset   = glm::vec2(0.0f);
    glm::vec2 m_size     = glm::vec2(128.0f);

public:

    ImageAbsolute( uint32_t texture, const Style &style )
    :   Image(texture, style)
    {
        
    }

    void setPosition  ( const glm::vec2 &p ) { m_position = p; }
    void pushPosition ( const glm::vec2 &p ) { m_positions.push_back(p); }
    void addOffset    ( const glm::vec2 &v ) { m_offset += v; }

    void expand()
    {
        m_style.margin  *= 0.0f;
        m_style.padding *= 0.0f;
    }

    virtual void updateLayout( const glm::vec2&, const glm::vec2&, int );
    virtual void render( idkui::UIRenderer &ren );
};




class idkui::UIRenderer
{
private:

    static constexpr float MAX_Z_DEPTH = 128.0f;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;
        glm::vec3 extents;
        glm::vec4 color;
    };

    struct QuadDesc
    {
        glm::vec3 position;
        glm::vec3 extents;
        glm::vec3 fg, bg;
    };


    uint32_t                    m_VAO, m_VBO, m_IBO;
    // std::vector<Vertex>         m_vertexbuffer;
    // std::vector<uint32_t>       m_indexbuffer;

    std::vector<Vertex>              m_vertexbuffer_quad;
    std::vector<std::vector<Vertex>> m_vertexbuffer_quads;
    std::vector<Vertex>              m_vertexbuffer_glyph;

    std::vector<uint32_t>            m_samplers;
    std::vector<Vertex>              m_vertexbuffer_image;

    std::vector<uint32_t>            m_indexbuffer;

    uint32_t m_atlas;
    int      m_atlas_w;
    int      m_glyph_w;
    int      m_glyph_h;
    int      m_grid_w;

    std::vector<int> glyph_sizes;


    uint32_t _renderFontAtlas( const std::string &filepath, int size );

    void     _genQuadVAO();
    void     _renderAllQuads( idk::RenderEngine&, const glm::mat4& );
    void     _renderAllText( idk::RenderEngine&, const glm::mat4& );
    void     _renderAllImage( idk::RenderEngine&, const glm::mat4& );
    void     _renderNode( Element* );
    void     _flush();


public:

    UIRenderer( const std::string &font, int size );

    int glyphWidth() const { return m_glyph_w; };

    void renderGlyph( int, int, int, char, const glm::vec4&, float );
    void renderText( const std::string&, const Primitive&, const glm::vec4& );
    void renderTextCentered( const std::string&, const Primitive&, const glm::vec4& );

    void renderPrimitive( const Primitive&, const glm::vec4& );
    void renderImage( const Primitive&, uint32_t texture );


    void renderNode( Element*, int basedepth=0 );
    void renderTexture( idk::EngineAPI &api );
    void clearTexture( idk::EngineAPI&, GLbitfield );

};



class idkui::LayoutManager
{
private:
    friend class ElementBase;
    friend class Grid;
    friend class Button;

    UIRenderer    m_UIRenderer;

public:
    LayoutManager( const std::string &font, int size ): m_UIRenderer(font, size) {  };

    void updateInput( Element* );
    void renderNode( Element *n, int basedepth=0 )       { m_UIRenderer.renderNode(n, basedepth); }
    void renderTexture ( idk::EngineAPI &api )           { m_UIRenderer.renderTexture(api); };
    void clearTexture( idk::EngineAPI &a, GLbitfield m ) { m_UIRenderer.clearTexture(a, m); };

    // void updateNode( ElementBase *root, ElementBase *node, const glm::vec2 &span );
    // void renderTexture( idk::EngineAPI&, ElementBase* );


};


