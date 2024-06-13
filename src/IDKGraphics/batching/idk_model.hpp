#pragma once

#include <libidk/GL/common.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>

#include <vector>
#include <string>
#include <fstream>


namespace idk
{
    static constexpr uint32_t IDK_MAX_MATERIALS = 128;
    static constexpr uint32_t IDK_TEXTURES_PER_MATERIAL = 5;

    struct ModelHandle
    {
        uint32_t vertexformat;
        uint32_t numindices;
        uint32_t basevertex;
        uint32_t baseindex;

        idk::glDrawCmd cmd;
    };


    struct TextureDescriptor
    {
        GLuint   texture;
        GLuint64 handle;
    };

    struct MaterialDescriptor
    {
        std::vector<int>      textures    = std::vector<int>(IDK_TEXTURES_PER_MATERIAL, -1);
        std::vector<float>    multipliers = std::vector<float>(IDK_TEXTURES_PER_MATERIAL, 1.0f);
    };


    using BindlessTextureBuffer  = std::vector<GLuint64>;


    struct MeshDescriptor
    {
        int    material_id;
        float  bounding_radius;

        uint32_t baseVertex;
        uint32_t firstIndex;

        uint32_t numVertices;
        uint32_t numIndices;
    };


    struct ModelDescriptor
    {
        std::vector<int> mesh_ids;
        std::vector<int> user_materials = std::vector<int>(IDK_TEXTURES_PER_MATERIAL, -1);
    };




    struct Vertex_P_N_T_UV
    {
        glm::vec3  position, normal, tangent;
        glm::vec2  texcoords;
    };


    struct Vertex_P_N_T_UV_SKINNED
    {
        glm::vec3  position, normal, tangent;
        glm::vec2  texcoords;

        glm::ivec4 bone_IDs;
        glm::vec4  bone_weights;
    };


    struct VertexFormatDescriptor
    {
        uint32_t stride;
        uint32_t attribs;

        uint32_t attrib_size[8];
        uint32_t attrib_nbytes[8];
        GLenum   attrib_datatype[8];
        uint32_t attrib_offset[8];
    };

    uint32_t VertexFormat_sizeof( uint32_t format );
    VertexFormatDescriptor VertexFormat_desc( uint32_t format );


    enum ModelVertexFormat: uint32_t
    {
        ModelVertexFormat_POS_NORM_TAN_UV         = 0,
        ModelVertexFormat_POS_NORM_TAN_UV_SKINNED = 1
    };



    static constexpr uint32_t IDK_MESHFILE_STRING_LENGTH = 128;

    enum MeshTextureIndex: uint32_t
    {
        MeshTextureIndex_ALBEDO = 0,
        MeshTextureIndex_NORMAL,
        MeshTextureIndex_AO_R_M,
        MeshTextureIndex_EMISSV
    };

    struct MeshFileHeader
    {
        float       bounding_radius;
        uint32_t    num_verts;
        uint32_t    num_indices;
        uint32_t    bitmask;
        std::string textures[IDK_TEXTURES_PER_MATERIAL];
    };

    struct ModelFileHeader
    {
        uint32_t major;
        uint32_t minor;
        uint32_t vertexformat;
        uint32_t num_meshes;
    };


    bool     MeshFile_hasTexture( uint32_t bitmask, int idx );
    uint32_t MeshFile_packBitmask( const idk::MeshFileHeader & );


    template <typename vertex_type>
    float MeshFile_computeBoundingSphere( uint32_t num_vertices, const vertex_type *vertices );

    idk::MeshFileHeader MeshFile_new();


    void MeshFile_write( std::ofstream          &stream,
                         uint32_t                vertexformat,
                         const MeshFileHeader   &header,
                         const void             *vertices,
                         const void             *indices );

    void MeshFile_read( std::ifstream   &stream,
                        uint32_t         vertexformat,
                        MeshFileHeader  &header,
                        void           *&vertices,
                        void           *&indices );

    void ModelFile_write( std::ofstream                     &stream,
                          const ModelFileHeader             &header,
                          const std::vector<MeshFileHeader> &meshes,
                          const std::vector<void *>         &vertices,
                          const std::vector<void *>         &indices );

    void ModelFile_read( const std::string           &filepath,
                         ModelFileHeader             &header,
                         std::vector<MeshFileHeader> &meshes,
                         std::vector<void *>         &vertices,
                         std::vector<void *>         &indices );

};



#include <iostream>

template <typename vertex_type>
float
idk::MeshFile_computeBoundingSphere( uint32_t num_vertices, const vertex_type *vertices )
{
    float radius = -1.0f;

    for (uint32_t i=0; i<num_vertices; i++)
    {
        glm::vec3 origin   = glm::vec3(0.0f);
        glm::vec3 position = vertices[i].position;

        radius = glm::max(radius, glm::distance(origin, position));
    }

    return radius;
}

