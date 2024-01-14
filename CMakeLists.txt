cmake_minimum_required(VERSION 3.9)
project(IDKGraphics)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/animation/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/camera/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/batching/*.cpp
)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 17
)

# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
# endif()


target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    libidk
)

target_sources(
    ${PROJECT_NAME} PUBLIC FILE_SET HEADERS

    BASE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/

    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/animation/idk_animation_file.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/animation/idk_animation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/animation/IDKanimation.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/batching/idk_batch_allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/batching/idk_model_allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/batching/idk_model_handle.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/camera/idk_camera.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/camera/idk_frustum.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/idk_lightsource.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/idk_lightsystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/idk_shader_components.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/idk_shadergen.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/idk_shadowcascade.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/IDKlighting.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/filetypes/idk_idkvi.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/idk_model.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/idk_OBB.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/idk_vertex.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/IDKmodel.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/idk_drawmethods.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/idk_initflags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/idk_renderqueue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/idk_sdl_glew_init.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/idk_model_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/idk_noisegen.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/idk_renderengine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/idk_texture_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/IDKGraphics.hpp
)


install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_SOURCE_DIR}/build/include/IDKGraphics/
    FILE_SET HEADERS
)


FILE(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/IDKGE/
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/IDKGE/
)
