#include "scenes/TriangleScene.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const char* kVert = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aCol;
out vec3 vCol;
void main(){
    vCol = aCol;
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

static const char* kFrag = R"(#version 330 core
in vec3 vCol;
out vec4 FragColor;
void main(){
    FragColor = vec4(vCol, 1.0);
})";

TriangleScene::~TriangleScene() 
{ 
    shutdown(); 
}

void TriangleScene::init() 
{
    if (initialized_) return;

    const float verts[] = 
    {
         0.0f,  0.6f,   1.0f,0.3f,0.3f,
        -0.7f, -0.5f,   0.3f,1.0f,0.4f,
         0.7f, -0.5f,   0.3f,0.5f,1.0f
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shader_ = std::make_unique<Shader>(kVert, kFrag);
    initialized_ = true;
}

void TriangleScene::render() 
{
    if (!initialized_) return;
    shader_->use();
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void TriangleScene::shutdown() 
{
    if (vbo_) 
    { 
        glDeleteBuffers(1, &vbo_); 
        vbo_ = 0; 
    }

    if (vao_) 
    { 
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0; 
    }
    
    shader_.reset();
    initialized_ = false;
}
