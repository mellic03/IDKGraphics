
struct Camera
{
    vec4 position;
    vec4 beg;
    vec4 aberration_rg;
    vec4 aberration_b;
    vec4 exposure;
};

layout (std140, binding = 2) uniform UBO_camera_data
{
    mat4 un_view;
    mat4 un_projection;
    vec3 un_viewpos;
    Camera un_camera;
    vec3 un_cam_beg;
    vec4 un_imagesize;
};




struct IDK_Camera
{
    vec4 position;
    mat4 V, P, PV;
    vec4 image_size;
    float exposure, gamma, shutter, padding;

    vec4 prev_position;
    mat4 prev_V, prev_P, prev_PV;
};

struct IDK_Dirlight
{
    vec4 direction;
    vec4 diffuse;
};

struct IDK_Pointlight
{
    vec4 position;
    vec4 diffuse;
    vec4 attenuation;
};

struct IDK_Spotlight
{
    vec4 position;
    vec4 direction;
    vec4 diffuse;
    vec4 attenuation;
    vec4 angle;
};



#define REE_MAX_CAMERAS     4
#define REE_MAX_DIRLIGHTS   1
#define REE_MAX_POINTLIGHTS 16
#define REE_MAX_SPOTLIGHTS  16

struct IDK_UBORenderData
{
    IDK_Camera      cameras     [REE_MAX_CAMERAS];
    IDK_Dirlight    dirlights   [REE_MAX_DIRLIGHTS];
    IDK_Pointlight  pointlights [REE_MAX_POINTLIGHTS];
    IDK_Spotlight   spotlights  [REE_MAX_SPOTLIGHTS];
};


layout (std140, binding = 0) uniform IDK_UBO_SyncData
{
    uint un_SyncData_index;
};


layout (std140, binding = 3) uniform IDK_UBO_RenderData
{
    IDK_UBORenderData un_RenderData[3];
};


IDK_Camera IDK_RenderData_GetCamera()
{
    return un_RenderData[un_SyncData_index].cameras[0];
}


IDK_Dirlight IDK_RenderData_GetDirlight( int idx )
{
    return un_RenderData[un_SyncData_index].dirlights[idx];
}

