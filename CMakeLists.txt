cmake_minimum_required(VERSION 3.16)
project(ModelViewerBase CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Options
option(BUILD_GLFW "Download & build GLFW" ON)


# FetchContent for GLFW (portable)
include(FetchContent)
FetchContent_Declare(
glfw
GIT_REPOSITORY https://github.com/glfw/glfw.git
GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)


# Find OpenGL (platform-safe target OpenGL::GL)
find_package(OpenGL REQUIRED)


add_executable(model_viewer
    src/main.cpp
    src/core/Application.cpp
    src/platform/glfw/GlfwWindow.cpp
)


target_include_directories(model_viewer PRIVATE src)


target_link_libraries(model_viewer PRIVATE
glfw
OpenGL::GL
)


# Stricter warnings
if (MSVC)
target_compile_options(model_viewer PRIVATE /W4 /permissive-)
else()
target_compile_options(model_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()