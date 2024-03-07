#pragma once

#include <libidk/idk_vector.hpp>


namespace idk::UI
{
    enum class ElementShape: uint32_t
    {
        RECT,
        TRIANGLE,
        CIRCLE
    };

    class Element;
    class DrawList;
};



class idk::UI::Element
{
    ElementShape shape;

    float pos[2];
    float size[2];

    Element( ElementShape s, float x, float y, float w, float h=0 )
    {
        shape   = s;
        pos[0]  = x;
        pos[1]  = y;
        size[0] = w;
        size[1] = h;
    };
};


class idk::UI::DrawList
{
private:
    idk::vector<Element> m_drawlist[10]; // Z-index can be 0-9 inclusive.

public:

    void pushElement( const UI::Element & );
    UI::Element popElement();


};

