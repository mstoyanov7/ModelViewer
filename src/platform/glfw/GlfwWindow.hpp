#pragma once
#include "core/Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

class GlfwWindow final : public IWindow 
{
public:
    explicit GlfwWindow(const WindowProps& props);
    ~GlfwWindow() override;

    void PollEvents() override;
    void SwapBuffers() override;
    bool ShouldClose() const override;

    void SetTitle(const std::string& title) override;
    void SetVSync(bool enabled) override;
    bool IsVSync() const override { return m_VSync; }

    void GetFramebufferSize(int& w, int& h) const override { w = m_FBWidth; h = m_FBHeight; }

    GLFWwindow* Handle() const { return m_Handle; }

private:
    static void FramebufferSizeCallback(GLFWwindow* win, int w, int h);

private:
    GLFWwindow* m_Handle = nullptr;
    bool m_VSync = true;
    int m_FBWidth = 0;
    int m_FBHeight = 0;
};