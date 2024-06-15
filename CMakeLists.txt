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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IDKGraphics/UI/*.cpp
)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
# endif()
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_ttf_INCLUDE_DIRS})


target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    SDL2_ttf
    libidk
)


FILE(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/IDKGE/
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/IDKGE/
)
