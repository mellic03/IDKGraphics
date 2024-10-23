cmake_minimum_required(VERSION 3.9)
project(IDKGraphics)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/lighting/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/env_probe/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/animation/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/camera/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/render/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/noise/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/time/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/particle/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/terrain/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/renderstage/*.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/storage/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/batching/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/UI/*.cpp
)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

if (IDK_DEBUG)
    target_compile_options(${PROJECT_NAME} PUBLIC -Werror -fsanitize=undefined -lstdc++fs -g)
else()
    target_compile_options(${PROJECT_NAME} PUBLIC -Ofast)
endif ()



target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    # SDL2_ttf
    libidk
)


FILE(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/IDKGE/
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/IDKGE/
)

FILE(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/
)
