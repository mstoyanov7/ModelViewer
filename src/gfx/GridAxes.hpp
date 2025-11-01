#pragma once
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "core/Camera.hpp"


class GridAxes {
public:
    GridAxes() = default;
    ~GridAxes();

    void init(int halfLines = 10, float spacing = 1.0f);
    void render(const Camera& cam);
    void shutdown();

private:
    GLuint vao_ = 0, vbo_ = 0;
    GLsizei gridVertexCount_ = 0;

    std::unique_ptr<Shader> shader_;
};
