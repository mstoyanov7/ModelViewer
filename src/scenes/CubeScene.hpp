#pragma once
#include <memory>
#include <cstdint>
#include <vector>

#include "core/Camera.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

using GLuint = unsigned int;

class CubeScene {
public:
    CubeScene() = default;
    ~CubeScene();

    void init();
    void update(float dt);
    void render(const Camera& cam);
    void shutdown();

    void setLighting(bool on) { lighting_ = on; }

private:
    bool initialized_ = false;
    bool lighting_    = true;       // <- toggle via 'L'
    GLuint vao_ = 0, vbo_ = 0;
    int vertexCount_ = 0;          // non-indexed draw (36 verts)
    std::unique_ptr<Shader> shader_;
    glm::mat4 model_{1.0f};
    float angle_ = 0.0f;
};