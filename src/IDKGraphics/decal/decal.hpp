#pragma once




namespace idk
{
    class RenderStage;
    class glFramebuffer;
}


class idk::RenderStage
{
public:
    virtual void init() = 0;
    virtual void render( idk::glFramebuffer &buffer_out ) = 0;
};


