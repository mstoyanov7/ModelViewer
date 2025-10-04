#pragma once
#include <memory>
#include <cstdint>

#include "core/Camera.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

using GLuint = unsigned int;

class CubeScene 
{
public:
    CubeScene() = default;
    ~CubeScene();

    void init();
    void update(float dt);
    void render(const Camera& cam);
    void shutdown();

private:
    bool initialized_ = false;
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    std::unique_ptr<Shader> shader_;
    glm::mat4 model_{1.0f};
    float angle_ = 0.0f;
};