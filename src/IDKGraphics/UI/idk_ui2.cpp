#include "idk_ui2.hpp"

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>



static glm::vec2 mouse_screen  = glm::vec2(0.0f);
static glm::vec2 mouse_delta   = glm::vec2(0.0f);
static bool      mouse_down    = false;
static bool      mouse_clicked = false;


int focus_depth = 0;
idkui2::ElementBase *focus_element = nullptr;




void
idkui2::ElementBase::update( glm::vec2 corner, glm::vec2 span, int depth,
                              idkui2::UIRenderer &uiren )
{
    glm::vec4 margin = m_style.margin;

    m_bounds[0] = corner.x          + margin[0];
    m_bounds[1] = corner.x + span.x - margin[1];
    m_bounds[2] = corner.y          + margin[2];
    m_bounds[3] = corner.y + span.y - margin[3];

    auto [xmin, xmax, ymin, ymax] = m_bounds;

    m_focused = false;

    if (geometry::pointInRect(mouse_screen.x, mouse_screen.y, xmin, ymin, xmax-xmin, ymax-ymin))
    {
        m_focused = true;

        if (depth > focus_depth)
        {
            focus_element = this;
            focus_depth   = depth;
        }
    }

    _update(glm::vec2(xmin, ymin), glm::vec2(xmax-xmin, ymax-ymin), depth, uiren);

};









idkui2::Grid::Grid( const std::string &name, const ElementStyle &style, int rows, int cols )
:   ElementBase (name, style),
    m_rows      (rows),
    m_cols      (cols),
    m_children  (rows, std::vector<ElementBase*>(cols, nullptr))
{

}


void
idkui2::Grid::setChild( int row, int col, ElementBase *element )
{
    std::string msg = element->m_label + " (row, col) is out of range of " + this->m_label + ".children";

    while (row < 0)       row += m_rows;
    while (row >= m_rows) row -= m_rows;

    while (col < 0)       col += m_cols;
    while (col >= m_cols) col -= m_cols;

    IDK_ASSERT(
        msg.c_str(),
        (row < m_rows) && (col < m_cols)
    );

    m_children[row][col] = element;
}

void
idkui2::List::_update( glm::vec2 corner, glm::vec2 span, int depth,
                      idkui2::UIRenderer &uiren )
{
    auto [xmin, xmax, ymin, ymax] = m_bounds;


    if (m_style.invisible == false)
    {
        uiren.renderQuad(corner.x, corner.y, depth++, span.x, span.y, m_style.radius, m_style.bg);
    }


    glm::vec4 padding    = m_style.padding;
    glm::vec2 offset     = glm::vec2(0.0f, padding[3]);
    glm::vec2 child_span = glm::vec2(span.x, offset.y);

    for (ElementBase *child: m_children_front)
    {
        child->update(corner+offset, child_span, depth+1, uiren);
        offset.y += padding[3];
    }

    offset = glm::vec2(0.0f, span.y - padding[3]);

    for (ElementBase *child: m_children_back)
    {
        child->update(corner+offset, child_span, depth+1, uiren);
        offset.y -= padding[3];
    }

}



void
idkui2::Button::_update( glm::vec2 corner, glm::vec2 span, int depth,
                        idkui2::UIRenderer &uiren )
{
    auto [xmin, xmax, ymin, ymax] = m_bounds;

    glm::vec4 fg = m_style.fg;
    glm::vec4 bg = m_style.bg;

    if (m_focused)
    {
        std::swap(fg, bg);
    }

    uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);

    glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);
    uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg);

}


void
idkui2::Slider::on_down()
{
    auto [xmin, xmax, ymin, ymax] = m_bounds;

    float dist_min_data = m_data - m_min;
    float dist_min_max  = m_max - m_min;
    float alpha         = dist_min_data / dist_min_max;


    float w = ymax - ymin;

    float left  = xmin + w/2;
    float right = xmax - w/2;
    float mx    = mouse_screen.x;

    alpha = (mx - left) / (right - left);
    m_data = m_min + alpha*dist_min_max;

    if (m_step > 0.0f)
    {
        m_data = std::round(m_data / m_step) * m_step;
    }

    m_data = glm::clamp(m_data, m_min, m_max);
}


void
idkui2::Slider::_update( glm::vec2 corner, glm::vec2 span, int depth,
                       idkui2::UIRenderer &uiren )
{
    glm::vec4 fg = m_style.fg;
    glm::vec4 bg = m_style.bg;

    glm::vec4 fg2 = m_style.fg2;
    glm::vec4 bg2 = m_style.bg2;

    auto [xmin, xmax, ymin, ymax] = m_bounds;

    float w = span.y;
    float h = span.y;

    if (m_focused)
    {
        // std::swap(fg, bg);
    }

    uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);


    float alpha = (m_data - m_min) / (m_max - m_min);

    float x = (xmin + w/2) + alpha*(span.x - w);
    float y = ymin + span.y/2.0f;

    if (geometry::pointInRectCentered(mouse_screen.x, mouse_screen.y, x, y, w, h))
    {
        std::swap(fg2, bg2);
    }
    uiren.renderQuadCentered(x, y, depth++, w, h, m_style.radius, bg2);

}



void
idkui2::TextInput::_update( glm::vec2 corner, glm::vec2 span, int depth,
                           idkui2::UIRenderer &uiren )
{
    glm::vec4 margin = m_style.margin;

    float xmin = corner.x          + margin[0];
    float xmax = corner.x + span.x - margin[1];
    float ymin = corner.y          + margin[2];
    float ymax = corner.y + span.y - margin[3];

    corner = glm::vec2(xmin, ymin);
    span   = glm::vec2(xmax-xmin, ymax-ymin);

    glm::vec4 fg = m_style.fg;
    glm::vec4 bg = m_style.bg;

    glm::vec4 fg2 = m_style.fg2;
    glm::vec4 bg2 = m_style.bg2;


    if (m_focused)
    {
        std::swap(fg, bg);
    }

    uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);
    uiren.renderText(xmin, ymin, depth++, m_text, fg);

}



void
idkui2::Title::_update( glm::vec2 corner, glm::vec2 span, int depth,
                       idkui2::UIRenderer &uiren )
{
    auto [xmin, xmax, ymin, ymax] = m_bounds;

    corner = glm::vec2(xmin, ymin);
    span   = glm::vec2(xmax-xmin, ymax-ymin);

    glm::vec4 fg = m_style.fg;
    glm::vec4 bg = m_style.bg;

    glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);
    uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg);

}



void
idkui2::Grid::_update( glm::vec2 corner, glm::vec2 span, int depth,
                       idkui2::UIRenderer &uiren )
{
    if (m_visible == false)
    {
        return;
    }

    auto [xmin, xmax, ymin, ymax] = m_bounds;

    if (m_style.uniform_size)
    {
        span = glm::vec2(glm::min(span.x, span.y));
    }


    if (m_style.invisible == false)
    {
        float width  = xmax - xmin;
        float height = ymax - ymin;

        uiren.renderQuad(corner.x, corner.y, depth++, width, height, m_style.radius, m_style.bg);
        uiren.renderTextCenteredX((xmin+xmax)/2.0f, corner.y, depth++, m_label, m_style.fg);
    }


    glm::vec2 child_span = span / glm::vec2(m_cols, m_rows);
    float y = corner.y;


    for (std::vector<ElementBase *> &row: m_children)
    {
        float x = corner.x;

        for (ElementBase *child: row)
        {
            if (child != nullptr)
            {
                child->update(glm::vec2(x, y), child_span, depth+1, uiren);
            }
            x += child_span.x;
        }
        y += child_span.y;
    }

}



idkui2::Grid*
idkui2::LayoutManager::createRootPanel( int rows, int cols, const ElementStyle &style )
{
    m_root = new Grid("root", style, rows, cols);
    return m_root;
}



void
idkui2::LayoutManager::update( idk::EngineAPI &api, float dt )
{
    auto &ren    = api.getRenderer();
    auto &events = api.getEventSys();

    glm::vec2 size = glm::vec2(ren.resolution());

    if (events.mouseCaptured())
    {
        mouse_screen  = glm::vec2(-1024.0f);     
        mouse_delta   = glm::vec2(0.0f);
        mouse_clicked = false;
        mouse_down    = false;
    }

    else
    {
        mouse_screen  = events.mousePosition();
        mouse_delta   = events.mouseDelta();
        mouse_clicked = events.mouseClicked(MouseButton::LEFT);
        mouse_down    = events.mouseDown(MouseButton::LEFT);
    }


    {
        ElementBase *E = focus_element;

        if (E != nullptr)
        {
            E->on_focus();
    
            auto [xmin, xmax, ymin, ymax] = E->m_bounds;

            if (mouse_clicked)
            {
                E->on_click();
            }

            else if (mouse_down)
            {
                E->on_down();
            }
        }

        focus_depth   = 0;
        focus_element = nullptr;
    }


    m_root->update(glm::vec2(0.0f), size, 1, m_UIRenderer);
}



void
idkui2::LayoutManager::renderTexture( idk::EngineAPI &api )
{
    auto &ren = api.getRenderer();

    m_UIRenderer.renderTexture(api);

}
