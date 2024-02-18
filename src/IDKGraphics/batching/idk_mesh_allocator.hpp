#pragma once

#include <libidk/idk_memory.hpp>
#include "idk_model.hpp"


namespace idk
{
    class MeshAllocator;
    struct MeshPage;
};


struct idk::MeshPage
{
    size_t size;
    uint32_t basevertex;
    uint32_t baseindex;
};


class idk::MeshAllocator
{
private:
    uint32_t m_vertexformat;
    uint32_t sizeof_vertex;
    uint32_t m_basevertex = 0;
    uint32_t m_baseindex  = 0;

    uint8_t *VBO_ptr;
    uint8_t *IBO_ptr;

    size_t   VBO_nbytes;
    size_t   IBO_nbytes;

    size_t   VBO_cursor = 0;
    size_t   IBO_cursor = 0;

    
    void   *vertex_alloc( size_t nbytes );
    void   *index_alloc( size_t nbytes );


public:
    GLuint              VAO;
    GLuint              VBO;
    GLuint              IBO;

                        MeshAllocator( size_t nbytes, uint32_t vertexformat );

    idk::MeshDescriptor loadMesh( size_t nbytes_vertices, size_t nbytes_indices,
                                  void *vertices, void *indices );

    void    clear();


    void   *mapVBO( GLenum access );
    void   *mapIBO( GLenum access );

    void    unmapVBO();
    void    unmapIBO();

};