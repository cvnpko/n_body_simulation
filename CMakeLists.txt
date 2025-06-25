cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include_directories(external/glad/include)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(GLAD_SRC external/glad/src/glad.c)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    main.cpp
)

add_executable(MyProject ${SOURCES} ${GLAD_SRC})

target_link_libraries(MyProject
    ${OPENGL_LIBRARIES}
    glfw
)