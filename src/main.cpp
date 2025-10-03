#include "core/Application.hpp"
#include "platform/glfw/GlfwWindow.hpp"
#include "gfx/Shader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <memory>
#include <stdexcept>

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

class MyApp : public Application 
{
public:
    using Application::Application;

    ~MyApp() override 
    {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
    }

protected:
    void OnUpdate(double dt) override 
    {
        // Optional: animate title FPS
        accum_ += dt; frames_++;
        if (accum_ >= 0.3) 
        {
            char buf[128];
            const double fps = frames_ / accum_;
            snprintf(buf, sizeof(buf), "OpenGL — Triangle | %.1f FPS", fps);
            m_Window->SetTitle(buf);
            accum_ = 0.0; frames_ = 0;
        }

        if (!initialized_) 
        {
            initTriangle(); // lazy init after GL context exists
            initialized_ = true;
        }
    }

    void OnRender() override 
    {
        if (!initialized_) return;

        shader_->use();
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

private:
    void initTriangle() 
    {
        const float verts[] = {
            // pos.x pos.y   r    g    b
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
    }

private:
    bool initialized_ = false;
    std::unique_ptr<Shader> shader_;
    GLuint vao_ = 0, vbo_ = 0;

    double accum_ = 0.0;
    int frames_ = 0;
};

int main() {
    WindowProps props;
    props.title = "OpenGL — Triangle";
    props.width = 1280; props.height = 720;
    props.vsync = true;

    auto window = std::make_unique<GlfwWindow>(props);
    MyApp app(std::move(window));
    return app.Run();
}