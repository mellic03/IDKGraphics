#pragma once


namespace idk
{
    class RenderEngine;
    class Framebuffer;
    class RenderStage;
}


class idk::RenderStage
{
private:

public:
    virtual void init( RenderEngine& ) = 0;
    virtual void update( RenderEngine&, Framebuffer& ) = 0;

};

