#pragma once

#include <libidk/GL/common.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>

#include <vector>
#include <string>
#include <fstream>


namespace idk
{

    struct ModelHandle
    {
        uint32_t vertexformat;
        uint32_t numindices;
        uint32_t basevertex;
        uint32_t baseindex;

        idk::glDrawElementsIndirectCommand cmd;
    };


    struct TextureDescriptor
    {
        GLuint   texture;
        GLuint64 handle;
    };

    struct MaterialDescriptor
    {
        std::vector<int>      textures    = std::vector<int>(4, -1);
        std::vector<float>    multipliers = std::vector<float>(4, 1.0f);
    };

    static constexpr uint32_t IDK_MAX_MATERIALS = 128;
    static constexpr uint32_t IDK_TEXTURES_PER_MATERIAL = 4;

    using BindlessTextureBuffer  = std::vector<GLuint64>;


    struct MeshDescriptor
    {
        int material_id;

        uint32_t baseVertex;
        uint32_t baseIndex;

        uint32_t numVertices;
        uint32_t numIndices;
    };


    struct ModelDescriptor
    {
        std::vector<int> mesh_ids;
    };






    struct Vertex_P_N_T_UV
    {
        glm::vec3 position, normal, tangent;
        glm::vec2 texcoords;
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
        ModelVertexFormat_POS_NORM_TAN_UV = 0
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

    void ModelFile_write( const std::string                 &filepath,
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
