cmake_minimum_required(VERSION 3.10)
project(NBodySimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include_directories(external/glad/include)
include_directories(external/imgui/include)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(GLAD_SRC external/glad/src/glad.c)

file(GLOB_RECURSE IMGUISOURCES
    external/imgui/src/imgui.cpp
    external/imgui/src/imgui_draw.cpp
    external/imgui/src/imgui_widgets.cpp
    external/imgui/src/imgui_tables.cpp
    external/imgui/src/imgui_impl_glfw.cpp
    external/imgui/src/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    main.cpp
)

add_executable(NBodySimulation ${SOURCES} ${IMGUISOURCES} ${GLAD_SRC})

target_link_libraries(NBodySimulation
    ${OPENGL_LIBRARIES}
    glfw
)