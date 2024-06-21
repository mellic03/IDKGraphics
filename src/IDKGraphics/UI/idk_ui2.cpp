#include "idk_ui2.hpp"

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKEvents/IDKEvents.hpp>
#include <libidk/idk_geometry.hpp>



static glm::vec2 mouse_screen  = glm::vec2(0.0f);
static bool      mouse_down    = false;
static bool      mouse_clicked = false;


idkui2::Panel::Panel( const std::string &name, const ElementStyle &style, int rows, int cols )
:   ElementBase (name, style),
    m_rows      (rows),
    m_cols      (cols),
    m_children  (rows, std::vector<ElementBase*>(cols, nullptr))
{

}


void
idkui2::Panel::giveChild( int row, int col, ElementBase *element )
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
idkui2::List::update( glm::vec2 corner, glm::vec2 span, int depth,
                      idkui2::UIRenderer &uiren )
{
    glm::vec4 margin = m_style.margin;

    float xmin = corner.x          + margin[0];
    float xmax = corner.x + span.x - margin[1];
    float ymin = corner.y          + margin[2];
    float ymax = corner.y + span.y - margin[3];

    corner = glm::vec2(xmin, ymin);
    span   = glm::vec2(xmax-xmin, ymax-ymin);


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
idkui2::Button::update( glm::vec2 corner, glm::vec2 span, int depth,
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

    if (idk::geometry::pointInRect(mouse_screen.x, mouse_screen.y, xmin, ymin, span.x, span.y))
    {
        std::swap(fg, bg);

        if (mouse_clicked)
        {
            m_callback();
        }

        else if (mouse_down)
        {
            std::swap(fg, bg);
        }
    }

    uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);

    glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);
    uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg);

}

void
idkui2::Title::update( glm::vec2 corner, glm::vec2 span, int depth,
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

    glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);
    uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg);

}



void
idkui2::Panel::update( glm::vec2 corner, glm::vec2 span, int depth,
                       idkui2::UIRenderer &uiren )
{
    if (m_visible == false)
    {
        return;
    }


    glm::vec4 margin = m_style.margin;

    float xmin = corner.x          + margin[0];
    float xmax = corner.x + span.x - margin[1];
    float ymin = corner.y          + margin[2];
    float ymax = corner.y + span.y - margin[3];

    corner = glm::vec2(xmin, ymin);
    span   = glm::vec2(xmax-xmin, ymax-ymin);


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



idkui2::Panel*
idkui2::LayoutManager::createRootPanel( int rows, int cols, const ElementStyle &style )
{
    m_root = new Panel("root", style, rows, cols);
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
        mouse_clicked = false;
        mouse_down    = false;
    }

    else
    {
        mouse_screen  = events.mousePosition();
        mouse_clicked = events.mouseClicked(MouseButton::LEFT);
        mouse_down    = events.mouseDown(MouseButton::LEFT);
    }

    m_root->update(glm::vec2(0.0f), size, 1, m_UIRenderer);
}



void
idkui2::LayoutManager::renderTexture( idk::EngineAPI &api )
{
    auto &ren = api.getRenderer();

    m_UIRenderer.renderTexture(api);

}
