#include "idk_ui.hpp"
#include <libidk/idk_geometry.hpp>

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKIO/IDKIO.hpp>



// static glm::vec2 mouse_screen  = glm::vec2(0.0f);
// static glm::vec2 mouse_delta   = glm::vec2(0.0f);
// static bool      mouse_down    = false;
// static bool      mouse_clicked = false;


// int focus_depth = 0;
// idkui::ElementBase *focus_element = nullptr;



// void
// idkui::ElementBase::update( glm::vec2 corner, glm::vec2 span, int depth,
//                               idkui::UIRenderer &uiren )
// {
//     {
//         const glm::vec4 &margin = m_style.margin;

//         m_bounds[0] = corner.x          + margin[0];
//         m_bounds[1] = corner.x + span.x - margin[1];
//         m_bounds[2] = corner.y          + margin[2];
//         m_bounds[3] = corner.y + span.y - margin[3];

//         auto [xmin, xmax, ymin, ymax] = m_bounds;

//         if (m_style.invisible == false)
//         {
//             uiren.renderQuad(xmin, ymin, depth++, xmax-xmin, ymax-ymin, m_style.radius, m_style.bg);
//         }
//     }


//     const glm::vec4 &padding = m_style.padding;

//     m_bounds[0] -= padding[0];
//     m_bounds[1] -= padding[1];
//     m_bounds[2] += padding[2];
//     m_bounds[3] -= padding[3];

//     auto [xmin, xmax, ymin, ymax] = m_bounds;


//     m_focused = false;

//     if (geometry::pointInRect(mouse_screen.x, mouse_screen.y, xmin, ymin, xmax-xmin, ymax-ymin))
//     {
//         m_focused = true;

//         if (depth > focus_depth)
//         {
//             focus_element = this;
//             focus_depth   = depth;
//         }
//     }

//     _update(glm::vec2(xmin, ymin), glm::vec2(xmax-xmin, ymax-ymin), depth, uiren);

// };





// idkui::Grid::Grid( const std::string &name, const ElementStyle &style, int rows, int cols )
// :   ElementBase       (name, style),
//     m_rows            (rows),
//     m_cols            (cols),
//     m_children        (rows, std::vector<ElementBase*>(cols, nullptr)),
//     m_row_proportions (rows, 1.0f / rows),
//     m_col_proportions (cols, 1.0f / cols)
// {

// }



// idkui::Split::Split( const std::string &name, const ElementStyle &style, float ratio,
//                       ElementBase *left, ElementBase *right )
// :   Grid    (name, style, 1, 2),
//     m_ratio (ratio)
// {
//     m_children[0][0] = left;
//     m_children[0][1] = right;
//     setColProportions({ratio, 1.0f - ratio});
// }



// void
// idkui::Grid::setChild( int row, int col, ElementBase *element )
// {
//     std::string msg = element->m_label + " (row, col) is out of range of " + this->m_label + ".children";

//     while (row < 0)       row += m_rows;
//     while (row >= m_rows) row -= m_rows;

//     while (col < 0)       col += m_cols;
//     while (col >= m_cols) col -= m_cols;

//     IDK_ASSERT(
//         msg.c_str(),
//         (row < m_rows) && (col < m_cols)
//     );

//     m_children[row][col] = element;
// }


// idkui::ElementBase*
// idkui::Grid::getChild( int row, int col )
// {
//     return m_children[row][col];
// }


// void
// idkui::Grid::setRowProportions( const std::vector<float> &proportions )
// {
//     m_row_proportions = proportions;

//     float length = 0.0f;
//     for (int i=0; i<m_rows; i++)
//     {
//         length += m_row_proportions[i];
//     }

//     IDK_ASSERT("Cannot normalize proportions", length > 0.0001f);

//     for (int i=0; i<m_rows; i++)
//     {
//         m_row_proportions[i] /= length;
//     }
// }


// void
// idkui::Grid::setColProportions( const std::vector<float> &proportions )
// {
//     m_col_proportions = proportions;

//     float length = 0.0f;
//     for (int i=0; i<m_cols; i++)
//     {
//         length += m_col_proportions[i];
//     }

//     IDK_ASSERT("Cannot normalize proportions", length > 0.0001f);

//     for (int i=0; i<m_cols; i++)
//     {
//         m_col_proportions[i] /= length;
//     }
// }





// void
// idkui::List::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                       idkui::UIRenderer &uiren )
// {
//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     if (m_style.invisible == false)
//     {
//         uiren.renderQuad(corner.x, corner.y, depth++, span.x, span.y, m_style.radius, m_style.bg);
//     }

//     glm::vec2 offset     = glm::vec2(0.0f, 0.0f);
//     glm::vec2 child_span = glm::vec2(span.x, 64.0f);

//     for (ElementBase *child: m_children_front)
//     {
//         child->update(corner+offset, child_span, depth+1, uiren);
//         offset.y += 64.0f;
//     }


//     offset.y = span.y - 64.0f;

//     for (ElementBase *child: m_children_back)
//     {
//         child->update(corner+offset, child_span, depth+1, uiren);
//         offset.y -= 64.0f;
//     }
// }



// void
// idkui::Button::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                         idkui::UIRenderer &uiren )
// {
//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     glm::vec4 fg = m_style.fg;
//     glm::vec4 bg = m_style.bg;

//     if (m_focused)
//     {
//         std::swap(fg, bg);
//     }


//     uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);

//     glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);

//     float label_w = float(m_label.length() * uiren.glyphWidth());
//     float scale   = glm::min(label_w, span.x) / label_w;

//     uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg, scale);

// }


// void
// idkui::Slider::on_down()
// {
//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     float dist_min_data = m_data - m_min;
//     float dist_min_max  = m_max - m_min;
//     float alpha         = dist_min_data / dist_min_max;


//     float w = ymax - ymin;

//     float left  = xmin + w/2;
//     float right = xmax - w/2;
//     float mx    = mouse_screen.x;

//     alpha = (mx - left) / (right - left);

//     float prev = m_data;
//     m_data = m_min + alpha*dist_min_max;

//     if (m_step > 0.0f)
//     {
//         m_data = std::round(m_data / m_step) * m_step;
//     }

//     if (fabs(m_data - prev) > m_step)
//     {
//         m_callback(m_data);
//     }

//     m_data = glm::clamp(m_data, m_min, m_max);
// }


// void
// idkui::Slider::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                        idkui::UIRenderer &uiren )
// {
//     glm::vec4 fg = m_style.fg;
//     glm::vec4 bg = m_style.bg;

//     glm::vec4 fg2 = m_style.fg2;
//     glm::vec4 bg2 = m_style.bg2;

//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     float w = span.y;
//     float h = span.y;

//     if (m_focused)
//     {
//         // std::swap(fg, bg);
//     }

//     if (geometry::pointInRect(mouse_screen.x, mouse_screen.y, xmin, ymin, span.x, span.y))
//     {
//         if (idkio::mouseDown(idkio::LMOUSE))
//         {
//             on_down();
//         }
//     }

//     uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);


//     float alpha = (m_data - m_min) / (m_max - m_min);

//     float x = (xmin + w/2) + alpha*(span.x - w);
//     float y = ymin + span.y/2.0f;

//     if (geometry::pointInRectCentered(mouse_screen.x, mouse_screen.y, x, y, w, h))
//     {
//         std::swap(fg2, bg2);
//     }

//     uiren.renderQuadCentered(x, y, depth++, w, h, m_style.radius, bg2);

// }



// void
// idkui::TextInput::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                            idkui::UIRenderer &uiren )
// {
//     glm::vec4 margin = m_style.margin;

//     float xmin = corner.x          + margin[0];
//     float xmax = corner.x + span.x - margin[1];
//     float ymin = corner.y          + margin[2];
//     float ymax = corner.y + span.y - margin[3];

//     corner = glm::vec2(xmin, ymin);
//     span   = glm::vec2(xmax-xmin, ymax-ymin);

//     glm::vec4 fg = m_style.fg;
//     glm::vec4 bg = m_style.bg;

//     // glm::vec4 fg2 = m_style.fg2;
//     // glm::vec4 bg2 = m_style.bg2;

//     // if (m_focused)
//     // {
//     //     std::swap(fg, bg);
//     // }


//     uiren.renderQuad(xmin, ymin, depth++, span.x, span.y, m_style.radius, bg);

//     if (m_text.length() == 0)
//     {
//         return;
//     }

//     float label_w = float(m_text.length() * uiren.glyphWidth());
//     float new_w   = glm::min(label_w, span.x);
//     float scale   =  new_w / label_w;

//     uiren.renderText(xmin, ymin, depth++, m_text, fg, scale);


//     float xcursor = xmin + label_w;
//     uiren.renderQuad(xcursor, ymin+span.y, depth++, 32, 8, 0.0f, fg);

// }


// void
// idkui::TextInput::on_focus( idk::EngineAPI &api )
// {
//     if (!m_text.empty() && idk::IO::keyTapped(Keycode::BACKSPACE))
//     {
//         m_text.pop_back();
//     }

//     if (idk::IO::keyTapped(Keycode::PERIOD))
//     {
//         m_text.push_back('.');
//     }

//     if (idk::IO::keyTapped(Keycode::N0))
//     {
//         m_text.push_back('0');
//     }

//     for (uint32_t k=Keycode::N1; k<=Keycode::N9; k++)
//     {
//         if (idk::IO::keyTapped((Keycode)(k)))
//         {
//             m_text += char(k - Keycode::N1 + '0');
//         }
//     }

// }



// void
// idkui::Title::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                        idkui::UIRenderer &uiren )
// {
//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     corner = glm::vec2(xmin, ymin);
//     span   = glm::vec2(xmax-xmin, ymax-ymin);

//     glm::vec4 fg = m_style.fg;
//     glm::vec4 bg = m_style.bg;

//     glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);

//     uiren.renderQuadCentered(center.x, center.y, depth++, span.x, span.y, m_style.radius, m_style.bg);
//     uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg);

// }




// void
// idkui::Label::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                         idkui::UIRenderer &uiren )
// {
//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     corner = glm::vec2(xmin, ymin);
//     span   = glm::vec2(xmax-xmin, ymax-ymin);

//     glm::vec4 fg = m_style.fg;
//     glm::vec4 bg = m_style.bg;

//     glm::vec2 center = glm::vec2(xmin+span.x/2.0f, ymin+span.y/2.0f);


//     int   label_w = m_label.length() * uiren.glyphWidth();
//     int   quad_w  = span.x;
//     int   width   = glm::min(label_w, quad_w);
//     float scale   = float(width) / label_w;

//     uiren.renderQuadCentered(center.x, center.y, depth++, span.x, span.y, m_style.radius, m_style.bg);
//     uiren.renderTextCentered(center.x, center.y, depth++, m_label, fg, scale);

// }



// void
// idkui::Grid::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                        idkui::UIRenderer &uiren )
// {
//     if (m_visible == false)
//     {
//         return;
//     }

//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     if (m_style.uniform_size)
//     {
//         span = glm::vec2(glm::min(span.x, span.y));
//     }


//     if (m_style.invisible == false)
//     {
//         float width  = xmax - xmin;
//         float height = ymax - ymin;
//         // float label_w = float(m_label.length() * uiren.glyphWidth());
//         // float scale   = glm::min(label_w, span.x) / label_w;


//         uiren.renderQuad(corner.x, corner.y, depth++, width, height, m_style.radius, m_style.bg);
//         // uiren.renderTextCenteredX((xmin+xmax)/2.0f, corner.y, depth++, m_label, m_style.fg, scale);
//     }

    
//     glm::vec2 child_corner = corner;
//     // glm::vec2 child_span   = span / glm::vec2(m_cols, m_rows);

//     float x, y, w, h;

//     y = corner.y;

//     for (int row=0; row<m_rows; row++)
//     {
//         x = corner.x;
//         h = span.y * m_row_proportions[row];

//         for (int col=0; col<m_cols; col++)
//         {
//             w = span.x * m_col_proportions[col];
            
//             if (m_children[row][col])
//             {
//                 m_children[row][col]->update(glm::vec2(x, y), glm::vec2(w, h), depth+1, uiren);
//             }

//             x += w;
//         }
    
//         y += h;
//     }
// }



// void
// idkui::SubMenu::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                           idkui::UIRenderer &uiren )
// {
//     if (m_visible == false)
//     {
//         return;
//     }

//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     const auto &fg = m_style.fg;
//     const auto &bg = m_style.bg;
//     const float radius = m_style.radius;

//     // Background
//     // -----------------------------------------------------------------------------------------
//     {
//         int w = xmax-xmin;
//         int h = ymax-ymin;
//         uiren.renderQuad(xmin, ymin, depth++, w, h, radius, bg);
//     }
//     // -----------------------------------------------------------------------------------------

//     // Title bar
//     // -----------------------------------------------------------------------------------------
//     float label_w = float(m_label.length() * uiren.glyphWidth());
//     float label_h = float(uiren.glyphWidth());
//     float scale   = glm::min(label_w, span.x) / label_w;

//     float w = span.x;
//     float h = 2.0f * label_h;

//     float cx = xmin + 0.5f*w;
//     float cy = ymin + 0.5f*h;

//     uiren.renderQuadCentered(cx, cy, depth++, w, h, radius, bg);
//     uiren.renderTextCentered(cx, cy, depth++, m_label, fg, scale);
//     // -----------------------------------------------------------------------------------------

//     corner.y += h;
//     span.y   -= h;

//     m_bounds[2] += h;
//     m_bounds[3] -= h;

//     idkui::Grid::_update(corner, span, depth+1, uiren);

// }




// void
// idkui::Split::_update( glm::vec2 corner, glm::vec2 span, int depth,
//                         idkui::UIRenderer &uiren )
// {
//     if (m_visible == false)
//     {
//         return;
//     }

//     auto [xmin, xmax, ymin, ymax] = m_bounds;

//     const auto &fg = m_style.fg;
//     const auto &bg = m_style.bg;
//     const float radius = m_style.radius;

//     float width  = xmax - xmin;
//     float height = ymax - ymin;

//     float xmin_L = xmin;
//     float xmax_L = xmin_L + m_ratio*width;

//     float xmin_R = xmax_L;
//     float xmax_R = xmin_R + (1.0f - m_ratio)*width;

//     glm::vec2 child_corner, child_span;

//     // Left child
//     child_corner = glm::vec2(xmin_L, ymin);
//     child_span   = glm::vec2(xmax_L-xmin_L, span.y);
//     m_children[0][0]->update(child_corner, child_span, depth+1, uiren);

//     // Right child
//     child_corner = glm::vec2(xmin_R, ymin);
//     child_span   = glm::vec2(xmax_R-xmin_R, span.y);
//     m_children[0][1]->update(child_corner, child_span, depth+1, uiren);
// }



// void
// idkui::LayoutManager::updateNode( ElementBase *root, ElementBase *node, const glm::vec2 &span )
// {
//     if (node == nullptr)
//     {
//         return;
//     }

//     node->update(glm::vec2(0.0f), span, 1, m_UIRenderer);
// }


// void
// idkui::LayoutManager::renderTexture( idk::EngineAPI &api, ElementBase *root )
// {
//     if (root == nullptr)
//     {
//         return;
//     }

//     auto &ren = api.getRenderer();
//     auto size = glm::vec2(ren.winsize());

//     root->update(glm::vec2(0.0f), size, 1, m_UIRenderer);


//     if (idkio::mouseCaptured())
//     {
//         mouse_screen  = glm::vec2(-1024.0f);     
//         mouse_delta   = glm::vec2(0.0f);
//         mouse_clicked = false;
//         mouse_down    = false;
//     }

//     else
//     {
//         mouse_screen  = idkio::mousePosition();
//         mouse_delta   = idkio::mouseDelta();
//         mouse_clicked = idkio::mouseClicked(idkio::LMOUSE);
//         mouse_down    = idkio::mouseDown(idkio::RMOUSE);
//     }


//     {
//         ElementBase *E = focus_element;

//         if (E != nullptr)
//         {
//             E->on_focus(api);
    
//             auto [xmin, xmax, ymin, ymax] = E->m_bounds;

//             if (mouse_clicked)
//             {
//                 E->on_click();
//             }

//             else if (mouse_down)
//             {
//                 E->on_down();
//             }
//         }

//         focus_depth   = 0;
//         focus_element = nullptr;
//     }



//     m_UIRenderer.renderTexture(api);

// }


void
idkui::LayoutManager::updateInput( Element *node )
{   
    if (node == nullptr)
    {
        return;
    }

    if (Element::mouseOver(node->m_outer_bounds))
    {
        if (idkio::mouseClicked(idkio::LMOUSE))
        {
            node->onClick();
        }

        else
        {
            node->onHover();
        }
    }

    else
    {
        node->offHover();
    }

    for (auto *child: node->children)
    {
        updateInput(child);
    }
}
