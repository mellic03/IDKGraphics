#include "idk_ui.hpp"
#include <libidk/idk_geometry.hpp>
#include <IDKIO/IDKIO.hpp>


idkui::Primitive
idkui::Element::computeOuterBounds( const glm::vec2 &corner, const glm::vec2 &span, int depth,
                                    const Style &style )
{
    float xmin, xmax, ymin, ymax, w, h;

    glm::vec2 pad = glm::vec2(style.padding);

    xmin = corner.x + pad.x;
    xmax = corner.x + span.x - pad.x;
    ymin = corner.y + pad.y;
    ymax = corner.y + span.y - pad.y;

    float cx = 0.5f * (xmin + xmax);
    float cy = 0.5f * (ymin + ymax);
    float hw = 0.5f * glm::clamp((xmax-xmin), style.minbounds.x, style.maxbounds.x);
    float hh = 0.5f * glm::clamp((ymax-ymin), style.minbounds.y, style.maxbounds.y);

    xmin = cx - hw;
    xmax = cx + hw;
    ymin = cy - hh;
    ymax = cy + hh;

    Primitive p0 = {
        glm::vec2(xmin, ymin), glm::vec2(xmax-xmin, ymax-ymin), depth+1, style.radius
    };

    return p0;
}


idkui::Primitive
idkui::Element::computeInnerBounds( const glm::vec2 &corner, const glm::vec2 &span, int depth,
                                    const Style &style )
{
    Primitive p0 = computeOuterBounds(corner, span, depth+1, style);

    p0.corner.x += style.margin.x;
    p0.corner.y += style.margin.y;

    p0.span.x -= 2.0f * style.margin.x;
    p0.span.y -= 2.0f * style.margin.y;

    m_corner = p0.corner;
    m_span   = p0.span;
    m_depth  = p0.depth;

    return p0;
}


bool
idkui::Element::mouseOver( const Primitive &p )
{
    auto [xmin, ymin, xmax, ymax, w, h, z] = p.to_tuple();
    auto mpos = idkio::mousePosition();
    return geometry::pointInRect(mpos.x, mpos.y, xmin, ymin, w, h);
}






void
idkui::List::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    int  rows  = children.size();
    auto cspan = m_span / glm::vec2(1, 12);
    auto tl    = m_corner;

    for (int row=0; row<rows; row++)
    {
        if (!children[row])
        {
            continue;
        }

        children[row]->updateLayout(tl, cspan, m_depth+1);
    
        tl.y += children[row]->m_outer_bounds.span.y;
    }

}



void
idkui::List::render( idkui::UIRenderer &ren )
{
    ren.renderPrimitive(m_outer_bounds, m_style.bg);
    ren.renderPrimitive(m_inner_bounds, m_style.fg);
}









void
idkui::Grid::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    int   rows = m_rows;
    int   cols = m_cols;
    auto  tl   = m_corner;

    for (int row=0; row<rows; row++)
    {
        tl.x = m_corner.x;

        for (int col=0; col<cols; col++)
        {
            glm::vec2 sp = m_span * glm::vec2(col_ratio[col], row_ratio[row]);

            if (children[cols*row + col])
            {
                children[cols*row + col]->updateLayout(tl, sp, depth+1);
            }

            tl.x += col_ratio[col] * m_span.x;
        }

        tl.y += row_ratio[row] * m_span.y;
    }

}


void
idkui::Grid::render( idkui::UIRenderer &ren )
{
    ren.renderPrimitive(m_outer_bounds, m_style.bg);
    ren.renderPrimitive(m_inner_bounds, m_style.fg);
};


idkui::Element*&
idkui::Grid::getChild( int row, int col )
{
    IDK_ASSERT(
        "Child index out of bounds",
        row >= 0 && row < m_rows && col >=0 && col < m_cols
    );

    return children[m_cols*row + col];
}







void
idkui::GridUniform::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    int   cols  = m_cols;
    auto  tl    = m_corner;
    auto  sp    = glm::vec2(m_span.x / float(cols));

    for (int i=0; i<children.size(); i++)
    {
        if (i != 0 && (i % cols) == 0)
        {
            tl.x  = m_corner.x;
            tl.y += sp.y;
        }

        if (children[i])
        {
            children[i]->updateLayout(tl, sp, depth+1);
        }

        tl.x += sp.x;
    }
}










void
idkui::Stack::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    auto tl = m_corner;
    auto sp = m_span;
    int  d  = m_depth;

    for (auto *child: children)
    {
        if (child)
        {
            child->updateLayout(tl, sp, d+1);
            // d += child->m_depth;
        }
    }
}




void
idkui::Text::render( idkui::UIRenderer &ren )
{
    ren.renderTextCentered(m_label, m_inner_bounds, m_style.fg);
}



void
idkui::TextAbsolute::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    // m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    // m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    // for (auto *child: children)
    // {
    //     child->updateLayout(m_corner, m_span, m_depth);
    // }

    m_outer_bounds = {m_position, glm::vec2(512.0f), depth+1, m_style.radius};
    m_inner_bounds = {m_position, glm::vec2(512.0f), depth+2, m_style.radius};

    m_corner = m_inner_bounds.corner;
    m_span   = m_inner_bounds.span;
    m_depth  = m_inner_bounds.depth;
}





void
idkui::Button::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth, m_style);

    for (auto *child: children)
    {
        child->updateLayout(m_corner, m_span, m_depth+1);
    }
}


void
idkui::Button::render( idkui::UIRenderer &ren )
{
    ren.renderPrimitive(m_outer_bounds, m_style.bg);
    ren.renderPrimitive(m_inner_bounds, m_style.fg);
}








void
idkui::Image::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(corner, span, depth+1, m_style);
    m_inner_bounds = computeInnerBounds(corner, span, depth+1, m_style);

}


void
idkui::Image::render( idkui::UIRenderer &ren )
{
    ren.renderImage(m_inner_bounds, m_texture);
}





void
idkui::ImageAbsolute::updateLayout( const glm::vec2 &corner, const glm::vec2 &span, int depth )
{
    m_outer_bounds = computeOuterBounds(m_position+m_offset, m_size, depth+1, m_style);
    m_inner_bounds = computeInnerBounds(m_position+m_offset, m_size, depth+1, m_style);

    m_offset = glm::mix(m_offset, glm::vec2(0.0f), 0.5f);

    m_style.padding = glm::mix(m_style.padding, m_style2.padding, 0.02f);
    m_style.margin  = glm::mix(m_style.margin,  m_style2.margin, 0.02f);

}


void
idkui::ImageAbsolute::render( idkui::UIRenderer &ren )
{
    Primitive p0 = m_inner_bounds;


    Primitive p1 = p0;
    p1.corner -= 0.5f * m_size;
    // p1.span    = m_size;

    ren.renderImage(p1, m_texture);

    for (auto &pos: m_positions)
    {
        p1 = p0;
        p1.corner -= 0.5f * m_size;
        // p1.span    = m_size;

        ren.renderImage(p1, m_texture);
    }

    m_positions.clear();
}



// void
// idkui::List2::update( const glm::vec2 &corner, const glm::vec2 &span, int depth )
// {
//     m_corner=corner; m_span=span; m_depth=depth;

//     glm::vec2 offset = glm::vec2(0.0f, 0.0f);
//     glm::vec2 cspan  = glm::vec2(span.x, 64.0f);

//     for (auto *C: children)
//     {
//         auto [minv, maxv] = computeInnerBounds(corner+offset, cspan, C->m_style);
//         C->update(minv, maxv, depth+1);
//         offset.y += 64.0f;
//     }
// }



// void
// idkui::List::render( idkui::Renderer &ren )
// {
//     // ren.rect(m_corner, m_span, m_style.bg, m_style.radius, m_depth);

//     for (auto *C: children)
//     {
//         C->render(ren);
//     }
// }

