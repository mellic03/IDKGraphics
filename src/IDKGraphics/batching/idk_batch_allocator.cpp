#include "idk_batch_allocator.hpp"

#include <libidk/idk_assert.hpp>
#include <libidk/idk_vertex.hpp>
#include <libidk/GL/idk_glBindings.hpp>

#include <iostream>

idk::BatchAllocator::BatchAllocator( size_t nbytes, uint32_t vertexformat )
:
    m_vertexformat(vertexformat),
    VBO_nbytes(nbytes),
    IBO_nbytes(nbytes)
{
    auto desc = idk::getVertexFormatDescriptor(vertexformat);
    sizeof_vertex = desc.stride;

    GLuint     bindingindex = 0;
    GLbitfield buffer_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

    gl::createBuffers(1, &VBO);
    gl::namedBufferStorage(VBO, nbytes, nullptr, buffer_flags);
    VBO_ptr = gl::mapNamedBuffer(VBO, GL_WRITE_ONLY);
    IDK_ASSERT("Error mapping idk::BatchAllocator::VBO", VBO_ptr != nullptr);

    gl::createBuffers(1, &IBO);
    gl::namedBufferStorage(IBO, nbytes, nullptr, buffer_flags);
    IBO_ptr = gl::mapNamedBuffer(IBO, GL_WRITE_ONLY);
    IDK_ASSERT("Error mapping idk::BatchAllocator::IBO", IBO_ptr != nullptr);

    gl::createVertexArrays(1, &VAO);
    gl::vertexArrayVertexBuffer(VAO, bindingindex, VBO, 0, desc.stride);
    gl::vertexArrayElementBuffer(VAO, IBO);

    for (int i=0; i<desc.attribs; i++)
    {
        gl::enableVertexArrayAttrib(VAO, i);

        gl::vertexArrayAttribFormat(
            VAO,
            i,
            desc.attrib_size[i],
            desc.attrib_datatype[i],
            GL_FALSE,
            desc.attrib_offset[i]
        );

        gl::vertexArrayAttribBinding(VAO, i, bindingindex);
    }
}


idk::ModelHandle
idk::BatchAllocator::load( size_t nbytes_vertices, size_t nbytes_indices,
                           void *vertices, void *indices )
{
    void *vertices_ptr = allocVertices(nbytes_vertices);
    void *indices_ptr  = allocIndices(nbytes_indices);

    std::memcpy(vertices_ptr, vertices, nbytes_vertices);
    std::memcpy(indices_ptr,  indices,  nbytes_indices);

    size_t nvertices = nbytes_vertices / sizeof_vertex;
    size_t nindices  = nbytes_indices  / sizeof(uint32_t);

    idk::ModelHandle handle = {
        .vertexformat = m_vertexformat,
        .numindices   = nbytes_indices,
        .basevertex   = m_basevertex,
        .baseindex    = m_baseindex
    };

    m_basevertex += nvertices;
    m_baseindex  += nindices;

    return handle;
}



void *
idk::BatchAllocator::allocVertices( size_t nbytes )
{
    VBO_cursor += nbytes;

    IDK_ASSERT(
        "[BatchAllocator::allocVertices] Ran out of vertex storage",
        VBO_cursor < VBO_nbytes
    );

    return (uint8_t *)VBO_ptr + VBO_cursor;
}


void *
idk::BatchAllocator::allocIndices( size_t nbytes )
{
    IBO_cursor += nbytes;

    IDK_ASSERT(
        "[BatchAllocator::allocIndices] Ran out of index storage",
        IBO_cursor < IBO_nbytes
    );

    return (uint8_t *)IBO_ptr + IBO_cursor;
}


