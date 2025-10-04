cmake_minimum_required(VERSION 3.16)
project(ModelViewerBase CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------
# Options
# -------------------------------------------------------
option(BUILD_GLFW "Download & build GLFW" ON)

# -------------------------------------------------------
# Dependencies: GLFW via FetchContent (only GLFW)
# -------------------------------------------------------
include(FetchContent)

if(BUILD_GLFW)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
  set(GLFW_TARGET glfw)
else()
  find_package(glfw3 REQUIRED)
  set(GLFW_TARGET glfw)
endif()

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

add_executable(model_viewer
  src/main.cpp
  src/core/Application.cpp
  src/core/Camera.cpp
  src/core/OrbitCamera.cpp
  src/core/Input.cpp
  src/platform/glfw/GlfwWindow.cpp
  src/gfx/Shader.cpp
  src/scenes/CubeScene.cpp
  src/app/ModelViewerApp.cpp
)

target_include_directories(model_viewer PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/glad/include
)

if(MSVC)
  target_compile_definitions(model_viewer PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN GLFW_INCLUDE_NONE)
  add_compile_options(/MP)
endif()

target_link_libraries(model_viewer PRIVATE
  glad
  ${GLFW_TARGET}
  OpenGL::GL
  glm
)

if (MSVC)
  target_compile_options(model_viewer PRIVATE /W4 /permissive-)
else()
  target_compile_options(model_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
