cmake_minimum_required(VERSION 3.16)

project(BouncyLabs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(BouncyLabs src/main.cpp)

include_directories(dependencies/imgui dependencies/imgui/backends dependencies/glad/include)

set(IMGUI_SOURCES
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_demo.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

set(GLAD_SOURCES
    dependencies/glad/src/glad.c
)

target_sources(BouncyLabs PRIVATE ${IMGUI_SOURCES} ${GLAD_SOURCES})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(BouncyLabs OpenGL::GL glfw)