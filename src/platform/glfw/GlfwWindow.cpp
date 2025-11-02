#include "platform/glfw/GlfwWindow.hpp"
#include "core/Input.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdio>

namespace 
{ 
    int g_glfwRefCount = 0; 
}

GlfwWindow::GlfwWindow(const WindowProps& props) 
{
    if (g_glfwRefCount++ == 0) 
    {
        if (!glfwInit()) throw std::runtime_error("GLFW init failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    // Request 4x MSAA for smoother edges
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_Handle = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);

    if (!m_Handle) 
    {
        throw std::runtime_error("GLFW window creation failed");
    }

    glfwMakeContextCurrent(m_Handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        throw std::runtime_error("Failed to load OpenGL via GLAD (legacy)");
    }

    SetVSync(props.vsync);

    glfwSetWindowUserPointer(m_Handle, this);

    // keyboard
    glfwSetKeyCallback(m_Handle, [](GLFWwindow* win, int key, int sc, int action, int mods)
    {
        (void)win; (void)sc; (void)mods;
        if (action == GLFW_PRESS)
        {
            Input::SetKeyState(key, true);
        }   
        if (action == GLFW_RELEASE) 
        {
            Input::SetKeyState(key, false);
        }
    });

    // mouse buttons
    glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow* win, int button, int action, int mods)
    {
        (void)win; (void)mods;
        if (action == GLFW_PRESS)  
        {
            Input::SetMouseButton(button, true);
        } 
        if (action == GLFW_RELEASE) 
        {
            Input::SetMouseButton(button, false);
        }
    });

    // cursor position
    glfwSetCursorPosCallback(m_Handle, [](GLFWwindow* win, double x, double y)
    {
        (void)win; Input::SetMousePos(x, y);
    });

    // scroll
    glfwSetScrollCallback(m_Handle, [](GLFWwindow* win, double dx, double dy)
    {
        (void)win; Input::AddScroll(dx, dy);
    });

    glfwSetFramebufferSizeCallback(m_Handle, &GlfwWindow::FramebufferSizeCallback);
    glfwGetFramebufferSize(m_Handle, &m_FBWidth, &m_FBHeight);
}

GlfwWindow::~GlfwWindow() 
{
    if (m_Handle) 
    { 
        glfwDestroyWindow(m_Handle); m_Handle = nullptr; 
    }

    if (--g_glfwRefCount == 0) 
    {
        glfwTerminate();
    }
}

void GlfwWindow::PollEvents() 
{ 
    glfwPollEvents(); 
}

void GlfwWindow::SwapBuffers() 
{ 
    glfwSwapBuffers(m_Handle); 
}

bool GlfwWindow::ShouldClose() const 
{ 
    return glfwWindowShouldClose(m_Handle); 
}

void GlfwWindow::SetTitle(const std::string& title) 
{ 
    glfwSetWindowTitle(m_Handle, title.c_str()); 
}

void GlfwWindow::SetVSync(bool enabled) 
{ 
    m_VSync = enabled; 
    glfwSwapInterval(m_VSync ? 1 : 0); 
}

void GlfwWindow::FramebufferSizeCallback(GLFWwindow* win, int w, int h) 
{
    auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(win));
    self->m_FBWidth = w; 
    self->m_FBHeight = h;
}
