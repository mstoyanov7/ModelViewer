#pragma once
#include <memory>
#include <cstdint>

#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

using GLuint = unsigned int;

class TriangleScene 
{
public:
    TriangleScene() = default;
    ~TriangleScene();

    void init();    
    void render(); 
    void shutdown();

private:
    bool   initialized_ = false;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    std::unique_ptr<Shader> shader_;
};