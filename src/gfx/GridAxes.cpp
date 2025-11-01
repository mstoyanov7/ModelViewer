#include "gfx/GridAxes.hpp"
#include "gfx/Shader.hpp"
#include <vector>

GridAxes::~GridAxes()
{ 
    shutdown(); 
}

void GridAxes::init(int halfLines, float spacing) 
{
    // build grid on XZ plane (Y=0)
    std::vector<float> data;
    data.reserve((halfLines * 4 + 4 + 6) * 6);

    auto push = [&](float x, float y, float z, float r, float g, float b)
    {
        data.push_back(x); 
        data.push_back(y); 
        data.push_back(z);
        data.push_back(r); 
        data.push_back(g); 
        data.push_back(b);
    };

    float max = halfLines * spacing;

    // grid lines (gray, Z and X)
    for (int i=-halfLines; i<=halfLines; ++i) 
    {
        float k = i * spacing;
        // lines parallel to X (vary Z)
        push(-max, 0.f, k, 0.35f, 0.35f, 0.35f);
        push( max, 0.f, k, 0.35f, 0.35f, 0.35f);
        // lines parallel to Z (vary X)
        push(k, 0.f, -max, 0.35f, 0.35f, 0.35f);
        push(k, 0.f,  max, 0.35f, 0.35f, 0.35f);
    }

    // axes: X (red), Y (green), Z (blue)
    // X
    push(-max, 0, 0, 1, 0.2f, 0.2f); 
    push(max, 0, 0, 1, 0.2f, 0.2f);
    // Y
    push(0, -max, 0, 0.2f, 1, 0.2f); 
    push(0, max, 0, 0.2f, 1, 0.2f);
    // Z
    push(0, 0, -max, 0.2f ,0.4f ,1); 
    push(0, 0, max, 0.2f, 0.4f, 1);

    gridVertexCount_ = static_cast<GLsizei>(data.size() / 6);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    shader_ = Shader::FromFiles("assets/shaders/line.vert", "assets/shaders/line.frag");
}

void GridAxes::render(const Camera& cam) 
{
    if (!vao_) return;

    shader_->use();
    glUniformMatrix4fv(shader_->loc("uView"), 1, GL_FALSE, &cam.view()[0][0]);
    glUniformMatrix4fv(shader_->loc("uProj"), 1, GL_FALSE, &cam.proj()[0][0]);

    // Draw grid without writing depth to reduce z-fighting with the scene
    GLboolean depthMask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glDepthMask(GL_FALSE);
    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, gridVertexCount_);
    glBindVertexArray(0);
    glDepthMask(depthMask);
}

void GridAxes::shutdown() 
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
}
